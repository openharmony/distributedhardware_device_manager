/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "auth_manager.h"
#include "access_control_profile.h"
#include "deviceprofile_connector.h"
#include "distributed_device_profile_errors.h"
#include "dm_anonymous.h"
#include "dm_auth_context.h"
#include "dm_auth_state.h"
#include "dm_auth_state_machine.h"
#include "dm_crypto.h"
#include "dm_dialog_manager.h"
#include "dm_language_manager.h"
#include "dm_log.h"
#include "dm_negotiate_process.h"
#include "dm_softbus_cache.h"
#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {

constexpr const char* TAG_CRED_ID = "credId";
constexpr const char* TAG_CUSTOM_DESCRIPTION = "CUSTOMDESC";
constexpr const char* TAG_LOCAL_DEVICE_TYPE = "LOCALDEVICETYPE";
constexpr const char* TAG_REQUESTER = "REQUESTER";
constexpr const char* UNVALID_CREDTID = "invalidCredId";
// authType fallback table
using FallBackKey = std::pair<std::string, DmAuthType>; // accessee.bundleName, authType
static std::map<FallBackKey, DmAuthType> g_pinAuthTypeFallBackMap = {
    {{"cast_engine_service", DmAuthType::AUTH_TYPE_NFC}, DmAuthType::AUTH_TYPE_PIN},
};
// Maximum number of recursive lookups
constexpr size_t MAX_FALLBACK_LOOPKUP_TIMES = 2;

DmAuthStateType AuthSrcConfirmState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_CONFIRM_STATE;
}

void AuthSrcConfirmState::NegotiateCredential(std::shared_ptr<DmAuthContext> context, JsonObject &credTypeNegoResult)
{
    CHECK_NULL_VOID(context);
    JsonObject accesseeCredTypeList;
    accesseeCredTypeList.Parse(context->accessee.credTypeList);
    JsonObject accesserCredTypeList;
    accesserCredTypeList.Parse(context->accesser.credTypeList);
    if (accesseeCredTypeList.IsDiscarded() || accesserCredTypeList.IsDiscarded()) {
        LOGE("CredTypeList invalid.");
        return;
    }
    if (accesseeCredTypeList.Contains("identicalCredType") && accesserCredTypeList.Contains("identicalCredType")) {
        LOGI("have identical credential.");
        credTypeNegoResult["identicalCredType"] = DM_IDENTICAL_ACCOUNT;
        context->accesser.isGenerateLnnCredential = false;
    }
    if (accesseeCredTypeList.Contains("shareCredType") && accesserCredTypeList.Contains("shareCredType")) {
        LOGI("have share credential.");
        credTypeNegoResult["shareCredType"] = DM_SHARE;
        context->accesser.isGenerateLnnCredential = false;
    }
    if (accesseeCredTypeList.Contains("pointTopointCredType") &&
        accesserCredTypeList.Contains("pointTopointCredType")) {
        LOGI("have point_to_point credential.");
        credTypeNegoResult["pointTopointCredType"] = DM_POINT_TO_POINT;
    }
    if (accesseeCredTypeList.Contains("lnnCredType") && accesserCredTypeList.Contains("lnnCredType")) {
        LOGI("have lnn credential.");
        credTypeNegoResult["lnnCredType"] = DM_LNN;
        context->accesser.isGenerateLnnCredential = false;
    }
}

void AuthSrcConfirmState::NegotiateAcl(std::shared_ptr<DmAuthContext> context, JsonObject &aclNegoResult)
{
    CHECK_NULL_VOID(context);
    JsonObject accesseeAclList;
    accesseeAclList.Parse(context->accessee.aclTypeList);
    JsonObject accesserAclList;
    accesserAclList.Parse(context->accesser.aclTypeList);
    if (accesseeAclList.IsDiscarded() || accesserAclList.IsDiscarded()) {
        LOGE("aclList invalid.");
        return;
    }
    if (accesseeAclList.Contains("identicalAcl") && accesserAclList.Contains("identicalAcl")) {
        LOGI("have identical acl.");
        aclNegoResult["identicalAcl"] = DM_IDENTICAL_ACCOUNT;
        context->accesser.isAuthed = true;
        context->accesser.isPutLnnAcl = false;
    }
    if (accesseeAclList.Contains("shareAcl") && accesserAclList.Contains("shareAcl")) {
        LOGI("have share acl.");
        aclNegoResult["shareAcl"] = DM_SHARE;
        context->accesser.isAuthed = true;
        context->accesser.isPutLnnAcl = false;
    }
    if (accesseeAclList.Contains("pointTopointAcl") && accesserAclList.Contains("pointTopointAcl")) {
        LOGI("have point_to_point acl.");
        aclNegoResult["pointTopointAcl"] = DM_POINT_TO_POINT;
        context->accesser.isAuthed = true;
    }
    if (accesseeAclList.Contains("lnnAcl") && accesserAclList.Contains("lnnAcl")) {
        LOGI("have lnn acl.");
        aclNegoResult["lnnAcl"] = DM_LNN;
        context->accesser.isPutLnnAcl = false;
    }
}

void AuthSrcConfirmState::GetSrcCredType(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo, JsonObject &aclInfo, JsonObject &credTypeJson)
{
    CHECK_NULL_VOID(context);
    std::vector<std::string> deleteCredInfo;
    for (const auto &item : credInfo.Items()) {
        if (!item.Contains(FILED_CRED_TYPE) || !item[FILED_CRED_TYPE].IsNumberInteger() ||
            !item.Contains(FILED_CRED_ID) || !item[FILED_CRED_ID].IsString()) {
            deleteCredInfo.push_back(item[FILED_CRED_ID].Get<std::string>());
            continue;
        }
        int32_t credType = item[FILED_CRED_TYPE].Get<int32_t>();
        LOGI("credType %{public}d.", credType);
        switch (credType) {
            case DM_IDENTICAL_ACCOUNT:
                credTypeJson["identicalCredType"] = credType;
                context->accesser.credentialInfos[credType] = item.Dump();
                break;
            case DM_SHARE:
                credTypeJson["shareCredType"] = credType;
                context->accesser.credentialInfos[credType] = item.Dump();
                break;
            case DM_POINT_TO_POINT:
                GetSrcCredTypeForP2P(context, item, aclInfo, credTypeJson, credType, deleteCredInfo);
                break;
            case DM_LNN:
                if (!aclInfo.Contains("lnnAcl") ||
                    (context->accesser.aclProfiles[DM_LNN].GetAccesser().GetAccesserCredentialIdStr() !=
                    item[FILED_CRED_ID].Get<std::string>() &&
                    context->accesser.aclProfiles[DM_LNN].GetAccessee().GetAccesseeCredentialIdStr() !=
                    item[FILED_CRED_ID].Get<std::string>())) {
                    deleteCredInfo.push_back(item[FILED_CRED_ID].Get<std::string>());
                } else {
                    credTypeJson["lnnCredType"] = credType;
                    context->accesser.credentialInfos[credType] = item.Dump();
                }
                break;
            default:
                LOGE("invalid credType %{public}d.", credType);
                break;
        }
    }
    for (const auto &item : deleteCredInfo) {
        credInfo.Erase(item);
        context->hiChainAuthConnector->DeleteCredential(context->accesser.userId, item);
    }
}

void AuthSrcConfirmState::GetSrcCredTypeForP2P(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credObj,
    JsonObject &aclInfo, JsonObject &credTypeJson, int32_t credType, std::vector<std::string> &deleteCredInfo)
{
    CHECK_NULL_VOID(context);
    if (!aclInfo.Contains("pointTopointAcl") ||
        (context->accesser.aclProfiles[DM_POINT_TO_POINT].GetAccesser().GetAccesserCredentialIdStr() !=
        credObj[FILED_CRED_ID].Get<std::string>() &&
        context->accesser.aclProfiles[DM_POINT_TO_POINT].GetAccessee().GetAccesseeCredentialIdStr() !=
        credObj[FILED_CRED_ID].Get<std::string>())) {
        deleteCredInfo.push_back(credObj[FILED_CRED_ID].Get<std::string>());
    } else {
        credTypeJson["pointTopointCredType"] = credType;
        context->accesser.credentialInfos[credType] = credObj.Dump();
    }
}

void AuthSrcConfirmState::GetSrcAclInfo(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo, JsonObject &aclInfo)
{
    CHECK_NULL_VOID(context);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAllAclIncludeLnnAcl();
    FilterProfilesByContext(profiles, context);
    uint32_t bindLevel = DM_INVALIED_TYPE;
    for (const auto &item : profiles) {
        std::string trustDeviceId = item.GetTrustDeviceId();
        std::string trustDeviceIdHash = Crypto::GetUdidHash(trustDeviceId);
        if ((trustDeviceIdHash != context->accessee.deviceIdHash &&
            trustDeviceIdHash != context->accesser.deviceIdHash)) {
            LOGE("devId %{public}s hash %{public}s, accesser devId %{public}s.", GetAnonyString(trustDeviceId).c_str(),
                GetAnonyString(trustDeviceIdHash).c_str(), GetAnonyString(context->accesser.deviceIdHash).c_str());
            continue;
        }
        bindLevel = item.GetBindLevel();
        switch (item.GetBindType()) {
            case DM_IDENTICAL_ACCOUNT:
                if (IdenticalAccountAclCompare(context, item.GetAccesser(), item.GetAccessee())) {
                    aclInfo["identicalAcl"] = DM_IDENTICAL_ACCOUNT;
                    context->accesser.aclProfiles[DM_IDENTICAL_ACCOUNT] = item;
                }
                break;
            case DM_SHARE:
                if (ShareAclCompare(context, item.GetAccesser(), item.GetAccessee()) &&
                    CheckCredIdInAcl(context, item, credInfo, DM_SHARE)) {
                    aclInfo["shareAcl"] = DM_SHARE;
                    context->accesser.aclProfiles[DM_SHARE] = item;
                }
                break;
            case DM_POINT_TO_POINT:
                GetSrcAclInfoForP2P(context, item, credInfo, aclInfo);
                break;
            default:
                LOGE("invalid bindType %{public}d.", item.GetBindType());
                break;
        }
    }
    if (aclInfo.Contains("pointTopointAcl") && !aclInfo.Contains("lnnAcl") && bindLevel != USER) {
        aclInfo.Erase("pointTopointAcl");
        DeleteAcl(context, context->accesser.aclProfiles[DM_POINT_TO_POINT]);
    }
}

void AuthSrcConfirmState::GetSrcAclInfoForP2P(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, JsonObject &aclInfo)
{
    CHECK_NULL_VOID(context);
    if (Point2PointAclCompare(context, profile.GetAccesser(), profile.GetAccessee()) &&
        CheckCredIdInAcl(context, profile, credInfo, DM_POINT_TO_POINT)) {
        aclInfo["pointTopointAcl"] = DM_POINT_TO_POINT;
        context->accesser.aclProfiles[DM_POINT_TO_POINT] = profile;
    }
    if (LnnAclCompare(context, profile.GetAccesser(), profile.GetAccessee()) &&
        CheckCredIdInAcl(context, profile, credInfo, DM_LNN) && profile.GetBindLevel() == USER) {
        aclInfo["lnnAcl"] = DM_LNN;
        context->accesser.aclProfiles[DM_LNN] = profile;
    }
}

bool AuthSrcConfirmState::CheckCredIdInAcl(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, uint32_t bindType)
{
    LOGI("start.");
    std::string credId = profile.GetAccesser().GetAccesserCredentialIdStr();
    if (!credInfo.Contains(credId)) {
        credId = profile.GetAccessee().GetAccesseeCredentialIdStr();
        if (!credInfo.Contains(credId)) {
            LOGE("credInfoJson not contain credId %{public}s.", GetAnonyString(credId).c_str());
            DeleteAcl(context, profile);
            return false;
        }
    }
    if (credInfo.Contains(credId) && (!credInfo[credId].IsObject() || !credInfo[credId].Contains(FILED_CRED_TYPE) ||
        !credInfo[credId][FILED_CRED_TYPE].IsNumberInteger())) {
        LOGE("credId %{public}s contain credInfoJson invalid.", credId.c_str());
        DeleteAcl(context, profile);
        credInfo.Erase(credId);
        return false;
    }
    bool checkResult = false;
    switch (bindType) {
        LOGI("bindType %{public}d.", bindType);
        case DM_IDENTICAL_ACCOUNT:
        case DM_SHARE:
        case DM_LNN:
            if (credInfo[credId][FILED_CRED_TYPE].Get<uint32_t>() == bindType) {
                checkResult = true;
            } else {
                DeleteAcl(context, profile);
            }
            break;
        case DM_POINT_TO_POINT:
            CheckCredIdInAclForP2P(context, credId, profile, credInfo, bindType, checkResult);
            break;
        default:
            break;
    }
    return checkResult;
}

void AuthSrcConfirmState::CheckCredIdInAclForP2P(std::shared_ptr<DmAuthContext> context, std::string &credId,
    const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, uint32_t bindType,
    bool &checkResult)
{
    if (credInfo[credId][FILED_CRED_TYPE].Get<uint32_t>() == bindType) {
        std::vector<std::string> appList;
        credInfo[credId][FILED_AUTHORIZED_APP_LIST].Get(appList);
        const size_t APP_LIST_SIZE = 2;
        if (appList.size() >= APP_LIST_SIZE &&
            ((std::to_string(profile.GetAccesser().GetAccesserTokenId()) == appList[0] &&
            std::to_string(profile.GetAccessee().GetAccesseeTokenId()) == appList[1]) ||
            (std::to_string(profile.GetAccesser().GetAccesserTokenId()) == appList[1] &&
            std::to_string(profile.GetAccessee().GetAccesseeTokenId()) == appList[0]))) {
            checkResult = true;
        } else {
            DeleteAcl(context, profile);
        }
    }
}

bool AuthSrcConfirmState::IdenticalAccountAclCompare(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee)
{
    LOGI("start.");
    return accesser.GetAccesserDeviceId() == context->accesser.deviceId &&
        accesser.GetAccesserUserId() == context->accesser.userId &&
        Crypto::GetUdidHash(accessee.GetAccesseeDeviceId()) == context->accessee.deviceIdHash;
}

bool AuthSrcConfirmState::ShareAclCompare(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee)
{
    LOGI("start.");
    return accesser.GetAccesserDeviceId() == context->accesser.deviceId &&
        accesser.GetAccesserUserId() == context->accesser.userId &&
        Crypto::GetUdidHash(accessee.GetAccesseeDeviceId()) == context->accessee.deviceIdHash;
}

bool AuthSrcConfirmState::Point2PointAclCompare(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee)
{
    LOGI("start.");
    return (accesser.GetAccesserDeviceId() == context->accesser.deviceId &&
        accesser.GetAccesserUserId() == context->accesser.userId &&
        accesser.GetAccesserTokenId() == context->accesser.tokenId &&
        Crypto::GetUdidHash(accessee.GetAccesseeDeviceId()) == context->accessee.deviceIdHash &&
        Crypto::GetTokenIdHash(std::to_string(accessee.GetAccesseeTokenId())) == context->accessee.tokenIdHash) ||
        (accessee.GetAccesseeDeviceId() == context->accesser.deviceId &&
        accessee.GetAccesseeUserId() == context->accesser.userId &&
        accessee.GetAccesseeTokenId() == context->accesser.tokenId &&
        Crypto::GetUdidHash(accesser.GetAccesserDeviceId()) == context->accessee.deviceIdHash &&
        Crypto::GetTokenIdHash(std::to_string(accesser.GetAccesserTokenId())) == context->accessee.tokenIdHash);
}

bool AuthSrcConfirmState::LnnAclCompare(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee)
{
    LOGI("start.");
    return ((accesser.GetAccesserDeviceId() == context->accesser.deviceId &&
        accesser.GetAccesserUserId() == context->accesser.userId) ||
        (accessee.GetAccesseeDeviceId() == context->accesser.deviceId &&
        accessee.GetAccesseeUserId() == context->accesser.userId)) &&
        accesser.GetAccesserTokenId() == 0 && accesser.GetAccesserBundleName() == "" &&
        (Crypto::GetUdidHash(accessee.GetAccesseeDeviceId()) == context->accessee.deviceIdHash ||
        Crypto::GetUdidHash(accesser.GetAccesserDeviceId()) == context->accessee.deviceIdHash) &&
        accessee.GetAccesseeTokenId() == 0 && accessee.GetAccesseeBundleName() == "";
}

void AuthSrcConfirmState::GetSrcCredentialInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo)
{
    LOGI("start.");
    CHECK_NULL_VOID(context);
    // get identical credential
    if (context->accesser.accountIdHash == context->accessee.accountIdHash) {
        GetIdenticalCredentialInfo(context, credInfo);
    }
    // get share credential
    if (context->accesser.accountIdHash != context->accessee.accountIdHash &&
        context->accesser.accountIdHash != Crypto::GetAccountIdHash16("ohosAnonymousUid") &&
        context->accessee.accountIdHash != Crypto::GetAccountIdHash16("ohosAnonymousUid")) {
        GetShareCredentialInfo(context, credInfo);
        GetP2PCredentialInfo(context, credInfo);
    }
    // get point_to_point credential
    if (context->accesser.accountIdHash == Crypto::GetAccountIdHash16("ohosAnonymousUid") ||
        context->accessee.accountIdHash == Crypto::GetAccountIdHash16("ohosAnonymousUid")) {
        GetP2PCredentialInfo(context, credInfo);
    }
    std::vector<std::string> deleteCredInfo;
    for (auto &item : credInfo.Items()) { // id1:json1, id2:json2, id3:json3
        uint32_t credType = DmAuthState::GetCredentialType(context, item);
        if (credType == DM_INVALIED_TYPE || !item.Contains(FILED_CRED_TYPE) ||
            !item[FILED_CRED_TYPE].IsNumberInteger() || !item.Contains(FILED_CRED_ID) ||
            !item[FILED_CRED_ID].IsString()) {
            deleteCredInfo.push_back(item[FILED_CRED_ID].Get<std::string>());
            continue;
        }
        item[FILED_CRED_TYPE] = credType;
    }
    for (const auto &item : deleteCredInfo) {
        credInfo.Erase(item);
    }
}

void AuthSrcConfirmState::GetIdenticalCredentialInfo(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo)
{
    LOGI("start.");
    CHECK_NULL_VOID(context);
    JsonObject queryParams;
    queryParams[FILED_DEVICE_ID] = context->accesser.deviceId;
    queryParams[FILED_USER_ID] = MultipleUserConnector::GetOhosAccountNameByUserId(context->accesser.userId);
    queryParams[FILED_CRED_TYPE] = DM_AUTH_CREDENTIAL_ACCOUNT_RELATED;
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    if (context->hiChainAuthConnector->QueryCredentialInfo(context->accesser.userId, queryParams, credInfo) != DM_OK) {
        LOGE("QueryCredentialInfo failed credInfo %{public}s.", credInfo.Dump().c_str());
    }
}

void AuthSrcConfirmState::GetShareCredentialInfo(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo)
{
    LOGI("start.");
    CHECK_NULL_VOID(context);
    JsonObject queryParams;
    queryParams[FILED_DEVICE_ID_HASH] = context->accessee.deviceIdHash;
    queryParams[FILED_PEER_USER_SPACE_ID] = std::to_string(context->accessee.userId);
    queryParams[FILED_CRED_TYPE] = DM_AUTH_CREDENTIAL_ACCOUNT_ACROSS;
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    if (context->hiChainAuthConnector->QueryCredentialInfo(context->accesser.userId, queryParams, credInfo) != DM_OK) {
        LOGE("QueryCredentialInfo failed credInfo %{public}s.", credInfo.Dump().c_str());
    }
}

void AuthSrcConfirmState::GetP2PCredentialInfo(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo)
{
    LOGI("start.");
    CHECK_NULL_VOID(context);
    JsonObject queryParams;
    queryParams[FILED_DEVICE_ID_HASH] = context->accessee.deviceIdHash;
    queryParams[FILED_PEER_USER_SPACE_ID] = std::to_string(context->accessee.userId);
    queryParams[FILED_CRED_TYPE] = DM_AUTH_CREDENTIAL_ACCOUNT_UNRELATED;
    queryParams[FILED_CRED_OWNER] = "DM";
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    if (context->hiChainAuthConnector->QueryCredentialInfo(context->accesser.userId, queryParams, credInfo) != DM_OK) {
        LOGE("QueryCredentialInfo failed credInfo %{public}s.", credInfo.Dump().c_str());
    }
}

int32_t AuthSrcConfirmState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("start.");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    context->timer->DeleteTimer(std::string(NEGOTIATE_TIMEOUT_TASK));
    GetCustomDescBySinkLanguage(context);
    context->accessee.isOnline = SoftbusCache::GetInstance().CheckIsOnline(context->accessee.deviceIdHash);
    JsonObject credInfo;
    GetSrcCredentialInfo(context, credInfo);
    JsonObject aclInfo;
    GetSrcAclInfo(context, credInfo, aclInfo);
    context->accesser.aclTypeList = aclInfo.Dump();
    JsonObject credTypeJson;
    GetSrcCredType(context, credInfo, aclInfo, credTypeJson);
    context->accesser.credTypeList = credTypeJson.Dump();
    // update credType negotiate result
    JsonObject credTypeNegoResult;
    NegotiateCredential(context, credTypeNegoResult);
    context->accesser.credTypeList = credTypeNegoResult.Dump();
    // update acl negotiate result
    JsonObject aclNegoResult;
    NegotiateAcl(context, aclNegoResult);
    context->accesser.aclTypeList = aclNegoResult.Dump();

    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REQ_USER_CONFIRM, context);
    context->listener->OnAuthResult(context->processInfo, context->peerTargetId.deviceId, context->accessee.tokenIdHash,
        STATUS_DM_SHOW_AUTHORIZE_UI, DM_OK);
    context->listener->OnBindResult(context->processInfo, context->peerTargetId,
        DM_OK, STATUS_DM_SHOW_AUTHORIZE_UI, "");
    context->timer->StartTimer(std::string(CONFIRM_TIMEOUT_TASK),
        DmAuthState::GetTaskTimeout(context, CONFIRM_TIMEOUT_TASK, CONFIRM_TIMEOUT),
        [context] (std::string name) {
            HandleAuthenticateTimeout(context, name);
        });
    return DM_OK;
}

void AuthSrcConfirmState::GetCustomDescBySinkLanguage(std::shared_ptr<DmAuthContext> context)
{
    if (context == nullptr || context->customData.empty()) {
        LOGI("customDesc is empty.");
        return;
    }
    context->customData = DmLanguageManager::GetInstance().GetTextBySystemLanguage(context->customData,
        context->accessee.language);
}

DmAuthStateType AuthSinkConfirmState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_CONFIRM_STATE;
}

int32_t AuthSinkConfirmState::ShowConfigDialog(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkConfirmState::ShowConfigDialog start");

    if (context->authType == AUTH_TYPE_PIN_ULTRASONIC &&
        context->ultrasonicInfo == DmUltrasonicInfo::DM_Ultrasonic_Invalid) {
        LOGE("AuthSinkConfirmState::ShowConfigDialog ultrasonicInfo invalid.");
        return STOP_BIND;
    }

    NodeBasicInfo nodeBasicInfo;
    int32_t result = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (result != SOFTBUS_OK) {
        LOGE("GetLocalNodeDeviceInfo from dsofbus fail, result=%{public}d", result);
        return STOP_BIND;
    }
 
    if (nodeBasicInfo.deviceTypeId == TYPE_TV_ID) {
        int32_t ret = AuthManagerBase::EndDream();
        if (ret != DM_OK) {
            LOGE("fail to end dream, err:%{public}d", ret);
            return STOP_BIND;
        }
    } else if (IsScreenLocked()) {
        LOGE("AuthSinkConfirmState::ShowStartAuthDialog screen is locked.");
        context->reason = ERR_DM_BIND_USER_CANCEL;
        context->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
        return STOP_BIND;
    }

    JsonObject jsonObj;
    jsonObj[TAG_CUSTOM_DESCRIPTION] = context->customData;
    jsonObj[TAG_LOCAL_DEVICE_TYPE] = context->accesser.deviceType;
    jsonObj[TAG_REQUESTER] = context->accesser.deviceName;
    jsonObj[TAG_USER_ID] = context->accessee.userId;    // Reserved
    jsonObj[TAG_HOST_PKGLABEL] = context->pkgLabel;

    const std::string params = jsonObj.Dump();
    DmDialogManager::GetInstance().ShowConfirmDialog(params);

    LOGI("AuthSinkConfirmState::ShowConfigDialog end");
    return DM_OK;
}

void AuthSinkConfirmState::NegotiateCredential(std::shared_ptr<DmAuthContext> context, JsonObject &credTypeNegoResult)
{
    CHECK_NULL_VOID(context);
    JsonObject accesseeCredTypeList;
    accesseeCredTypeList.Parse(context->accessee.credTypeList);
    JsonObject accesserCredTypeList;
    accesserCredTypeList.Parse(context->accesser.credTypeList);
    if (accesseeCredTypeList.IsDiscarded() || accesserCredTypeList.IsDiscarded()) {
        LOGE("CredTypeList invalid.");
        return;
    }
    if (accesseeCredTypeList.Contains("identicalCredType") && accesserCredTypeList.Contains("identicalCredType")) {
        LOGI("have identical credential.");
        credTypeNegoResult["identicalCredType"] = DM_IDENTICAL_ACCOUNT;
        context->accessee.isGenerateLnnCredential = false;
    }
    if (accesseeCredTypeList.Contains("shareCredType") && accesserCredTypeList.Contains("shareCredType")) {
        LOGI("have share credential.");
        credTypeNegoResult["shareCredType"] = DM_SHARE;
        context->accessee.isGenerateLnnCredential = false;
    }
    if (accesseeCredTypeList.Contains("pointTopointCredType") &&
        accesserCredTypeList.Contains("pointTopointCredType")) {
        LOGI("have point_to_point credential.");
        credTypeNegoResult["pointTopointCredType"] = DM_POINT_TO_POINT;
    }
    if (accesseeCredTypeList.Contains("lnnCredType") && accesserCredTypeList.Contains("lnnCredType")) {
        LOGI("have lnn credential.");
        credTypeNegoResult["lnnCredType"] = DM_LNN;
        context->accessee.isGenerateLnnCredential = false;
    }
    return;
}

void AuthSinkConfirmState::NegotiateAcl(std::shared_ptr<DmAuthContext> context, JsonObject &aclNegoResult)
{
    CHECK_NULL_VOID(context);
    JsonObject accesseeAclList;
    accesseeAclList.Parse(context->accessee.aclTypeList);
    JsonObject accesserAclList;
    accesserAclList.Parse(context->accesser.aclTypeList);
    if (accesseeAclList.IsDiscarded() || accesserAclList.IsDiscarded()) {
        LOGE("aclList invalid.");
        return;
    }
    if (accesseeAclList.Contains("identicalAcl") && accesserAclList.Contains("identicalAcl")) {
        LOGI("have identical acl.");
        aclNegoResult["identicalAcl"] = DM_IDENTICAL_ACCOUNT;
        context->accessee.isPutLnnAcl = false;
        context->accessee.isAuthed = true;
    }
    if (accesseeAclList.Contains("shareCredType") && accesserAclList.Contains("shareCredType")) {
        LOGI("have share acl.");
        aclNegoResult["shareAcl"] = DM_SHARE;
        context->accessee.isPutLnnAcl = false;
        context->accessee.isAuthed = true;
    }
    if (accesseeAclList.Contains("pointTopointAcl") && accesserAclList.Contains("pointTopointAcl")) {
        LOGI("have point_to_point acl.");
        aclNegoResult["pointTopointAcl"] = DM_POINT_TO_POINT;
        context->accessee.isAuthed = true;
    }
    if (accesseeAclList.Contains("lnnAcl") && accesserAclList.Contains("lnnAcl")) {
        LOGI("have lnn acl.");
        aclNegoResult["lnnAcl"] = DM_LNN;
        context->accessee.isPutLnnAcl = false;
    }
}

void AuthSinkConfirmState::MatchFallBackCandidateList(
    std::shared_ptr<DmAuthContext> context, DmAuthType authType)
{
    for (size_t i = 0; i < MAX_FALLBACK_LOOPKUP_TIMES; i++) {
        auto it = g_pinAuthTypeFallBackMap.find({context->accessee.bundleName, authType});
        if (it != g_pinAuthTypeFallBackMap.end()) {
            authType = it->second;
            context->authTypeList.push_back(authType);
        } else {
            break;
        }
    }
}

void AuthSinkConfirmState::ReadServiceInfo(std::shared_ptr<DmAuthContext> context)
{
    // query ServiceInfo by accessee.pkgName and authType from client
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    auto ret = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(
        context->accessee.pkgName, context->authType, srvInfo);
    if (ret == OHOS::DistributedDeviceProfile::DP_SUCCESS) {
        LOGI("AuthSinkConfirmState::ReadServiceInfo found");
        // ServiceInfo found
        context->serviceInfoFound = true;
        // read authBoxType
        context->authBoxType = static_cast<DMLocalServiceInfoAuthBoxType>(srvInfo.GetAuthBoxType());
        if (DmAuthState::IsImportAuthCodeCompatibility(context->authType)) {
            std::string pinCode = srvInfo.GetPinCode(); // read pincode
            if (AuthSinkStatePinAuthComm::IsPinCodeValid(pinCode)) {
                context->pinCode = pinCode;
            }
            srvInfo.SetPinCode("******");
            DeviceProfileConnector::GetInstance().UpdateLocalServiceInfo(srvInfo);
        }
        if (context->authBoxType == DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM) { // no authorization box
            int32_t confirmOperation = srvInfo.GetAuthType(); // read confirmOperation
            if (confirmOperation == static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ONETIME)) {
                context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH;
            } else if (confirmOperation == static_cast<int32_t>(DMLocalServiceInfoAuthType::CANCEL)) {
                context->confirmOperation = UiAction::USER_OPERATION_TYPE_CANCEL_AUTH;
            } else if (confirmOperation == static_cast<int32_t>(DMLocalServiceInfoAuthType::TRUST_ALWAYS)) {
                context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
            } else {
                context->confirmOperation = UiAction::USER_OPERATION_TYPE_CANCEL_AUTH;
            }
        }
        context->customData = srvInfo.GetDescription(); // read customData
    } else if (DmAuthState::IsImportAuthCodeCompatibility(context->authType) &&
        AuthSinkStatePinAuthComm::IsAuthCodeReady(context)) {
        // only special scenarios can import pincode
        context->authBoxType = DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM; // no authorization box
    } else {
        // not special scenarios, reset confirmOperation to cancel
        context->confirmOperation = UiAction::USER_OPERATION_TYPE_CANCEL_AUTH;
        context->authBoxType = DMLocalServiceInfoAuthBoxType::STATE3; // default: tristate box
    }
}

int32_t AuthSinkConfirmState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("start.");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    JsonObject credTypeNegoResult;
    JsonObject aclNegoResult;
    NegotiateCredential(context, credTypeNegoResult);
    context->accessee.credTypeList = credTypeNegoResult.Dump();
    NegotiateAcl(context, aclNegoResult);
    context->accessee.aclTypeList = aclNegoResult.Dump();
    if (credTypeNegoResult.Dump() != context->accesser.credTypeList ||
        aclNegoResult.Dump() != context->accesser.aclTypeList) {
        LOGE("compability negotiate not match.");
        context->reason = ERR_DM_CAPABILITY_NEGOTIATE_FAILED;
        return ERR_DM_FAILED;
    }
    int32_t ret = NegotiateProcess::GetInstance().HandleNegotiateResult(context);
    if (ret != DM_OK) {
        LOGE("HandleNegotiateResult failed ret %{public}d.", ret);
        context->reason = ERR_DM_CAPABILITY_NEGOTIATE_FAILED;
        return ret;
    }
    if (context->needBind) {
        return ProcessBindAuthorize(context);
    } else {
        return ProcessNoBindAuthorize(context);
    }
}

int32_t AuthSinkConfirmState::ProcessBindAuthorize(std::shared_ptr<DmAuthContext> context)
{
    LOGI("start.");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    ReadServiceInfo(context);
    context->authTypeList.clear();
    context->authTypeList.push_back(context->authType);
    if (context->authType == AUTH_TYPE_PIN_ULTRASONIC) {
        context->authTypeList.push_back(AUTH_TYPE_PIN);
    } else {
        MatchFallBackCandidateList(context, context->authType);
    }
    if (DmAuthState::IsImportAuthCodeCompatibility(context->authType) &&
        (context->serviceInfoFound || AuthSinkStatePinAuthComm::IsAuthCodeReady(context)) &&
        context->authBoxType == DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM) {
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_USER_CONFIRM, context);
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
        return DM_OK;
    }
    if ((context->authType == DmAuthType::AUTH_TYPE_PIN || context->authType == DmAuthType::AUTH_TYPE_NFC ||
        context->authType == DmAuthType::AUTH_TYPE_PIN_ULTRASONIC) &&
        context->authBoxType == DMLocalServiceInfoAuthBoxType::STATE3) {
        context->timer->DeleteTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK));
        if (ShowConfigDialog(context) != DM_OK) {
            LOGE("ShowConfigDialog failed");
            context->reason = ERR_DM_SHOW_CONFIRM_FAILED;
            return ERR_DM_FAILED;
        }
        if (DmEventType::ON_USER_OPERATION !=
            context->authStateMachine->WaitExpectEvent(DmEventType::ON_USER_OPERATION)) {
            LOGE("AuthSinkConfirmState::Action ON_USER_OPERATION err");
            return ERR_DM_FAILED;
        }
        if (context->confirmOperation == USER_OPERATION_TYPE_CANCEL_AUTH) {
            LOGE("AuthSinkConfirmState::Action USER_OPERATION_TYPE_CANCEL_AUTH");
            context->reason = ERR_DM_BIND_USER_CANCEL;
            return ERR_DM_FAILED;
        }
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_USER_CONFIRM, context);
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
        return DM_OK;
    }
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_CANCEL_AUTH;
    return ERR_DM_FAILED;
}

int32_t AuthSinkConfirmState::ProcessNoBindAuthorize(std::shared_ptr<DmAuthContext> context)
{
    LOGI("start.");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    JsonObject accesseeCredTypeList;
    accesseeCredTypeList.Parse(context->accessee.credTypeList);
    if (accesseeCredTypeList.IsDiscarded()) {
        LOGE("CredTypeList invalid.");
        context->reason = ERR_DM_CAPABILITY_NEGOTIATE_FAILED;
        return ERR_DM_FAILED;
    }
    if (accesseeCredTypeList.Contains("identicalCredType")) {
        context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
        context->accessee.transmitCredentialId = GetCredIdByCredType(context, DM_IDENTICAL_ACCOUNT);
    } else if (accesseeCredTypeList.Contains("shareCredType")) {
        context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
        context->accessee.transmitCredentialId = GetCredIdByCredType(context, DM_SHARE);
    } else if (accesseeCredTypeList.Contains("pointTopointCredType")) {
        context->accessee.transmitCredentialId = GetCredIdByCredType(context, DM_POINT_TO_POINT);
    } else if (accesseeCredTypeList.Contains("lnnCredType")) {
        context->accessee.lnnCredentialId = GetCredIdByCredType(context, DM_LNN);
    } else {
        LOGE("credTypeList invalid.");
        context->reason = ERR_DM_CAPABILITY_NEGOTIATE_FAILED;
        return ERR_DM_FAILED;
    }
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_USER_CONFIRM, context);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkCredentialAuthStartState>());
    return DM_OK;
}

std::string AuthSinkConfirmState::GetCredIdByCredType(std::shared_ptr<DmAuthContext> context, int32_t credType)
{
    LOGI("credType %{public}d.", credType);
    CHECK_NULL_RETURN(context, UNVALID_CREDTID);
    if (context->accessee.credentialInfos.find(credType) != context->accessee.credentialInfos.end()) {
        LOGE("invalid credType.");
        return UNVALID_CREDTID;
    }
    std::string credInfoStr = context->accessee.credentialInfos[credType];
    JsonObject credInfoJson;
    credInfoJson.Parse(credInfoStr);
    if (credInfoJson.IsDiscarded() || !credInfoJson.Contains(FILED_CRED_ID) ||
        !credInfoJson[FILED_CRED_ID].IsNumberInteger()) {
        LOGE("credInfoStr invalid.");
        return UNVALID_CREDTID;
    }
    return credInfoJson[FILED_CRED_ID].Get<std::string>();
}
} // namespace DistributedHardware
} // namespace OHOS