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
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "auth_manager.h"
#include "access_control_profile.h"
#include "deviceprofile_connector.h"
#include "distributed_device_profile_errors.h"
#include "dm_anonymous.h"
#include "dm_auth_attest_common.h"
#include "dm_auth_cert.h"
#include "dm_auth_context.h"
#include "dm_auth_state.h"
#include "dm_auth_state_machine.h"
#include "dm_crypto.h"
#include "dm_dialog_manager.h"
#include "dm_language_manager.h"
#include "dm_log.h"
#include "dm_negotiate_process.h"
#include "dm_softbus_cache.h"
#include "ffrt.h"
#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {

constexpr const char* TAG_CRED_ID = "credId";
constexpr const char* TAG_CUSTOM_DESCRIPTION = "CUSTOMDESC";
constexpr const char* TAG_LOCAL_DEVICE_TYPE = "LOCALDEVICETYPE";
constexpr const char* TAG_REQUESTER = "REQUESTER";
constexpr const char* UNVALID_CREDTID = "invalidCredId";
constexpr const char* TAG_IS_SUPPORT_ULTRASONIC = "isSupportUltrasonic";
// authType fallback table
using FallBackKey = std::pair<std::string, DmAuthType>; // accessee.bundleName, authType
static std::map<FallBackKey, DmAuthType> g_pinAuthTypeFallBackMap = {
    {{"cast_engine_service", DmAuthType::AUTH_TYPE_NFC}, DmAuthType::AUTH_TYPE_PIN},
};
// Maximum number of recursive lookups
constexpr size_t MAX_FALLBACK_LOOPKUP_TIMES = 2;

AuthSrcConfirmState::~AuthSrcConfirmState()
{
    LOGI("AuthSrcConfirmState destructor.");
}

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

void AuthSrcConfirmState::NegotiateProxyCredential(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind) {
        return;
    }
    if (context->subjectProxyOnes.empty()) {
        return;
    }
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end(); ++item) {
        JsonObject credTypeNegoResult;
        JsonObject accesseeCredTypeList;
        accesseeCredTypeList.Parse(item->proxyAccessee.credTypeList);
        JsonObject accesserCredTypeList;
        accesserCredTypeList.Parse(item->proxyAccesser.credTypeList);
        if (accesseeCredTypeList.IsDiscarded() || accesserCredTypeList.IsDiscarded()) {
            item->proxyAccesser.credTypeList = credTypeNegoResult.Dump();
            continue;
        }
        if (accesseeCredTypeList.Contains("pointTopointCredType") &&
            accesserCredTypeList.Contains("pointTopointCredType")) {
            credTypeNegoResult["pointTopointCredType"] = DM_POINT_TO_POINT;
            item->proxyAccesser.credTypeList = credTypeNegoResult.Dump();
            continue;
        }
        item->proxyAccesser.credTypeList = credTypeNegoResult.Dump();
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
        context->accesser.isUserLevelAuthed = true;
        context->accesser.isPutLnnAcl = false;
    }
    if (accesseeAclList.Contains("shareAcl") && accesserAclList.Contains("shareAcl")) {
        LOGI("have share acl.");
        aclNegoResult["shareAcl"] = DM_SHARE;
        context->accesser.isAuthed = true;
        context->accesser.isUserLevelAuthed = true;
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

void AuthSrcConfirmState::NegotiateProxyAcl(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind) {
        return;
    }
    if (context->subjectProxyOnes.empty()) {
        return;
    }
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end(); ++item) {
        JsonObject aclNegoResult;
        JsonObject accesseeAclList;
        if (!item->proxyAccessee.aclTypeList.empty()) {
            accesseeAclList.Parse(item->proxyAccessee.aclTypeList);
        }
        JsonObject accesserAclList;
        if (!item->proxyAccesser.aclTypeList.empty()) {
            accesserAclList.Parse(item->proxyAccesser.aclTypeList);
        }
        if (accesseeAclList.IsDiscarded() || accesserAclList.IsDiscarded()) {
            item->proxyAccesser.aclTypeList = aclNegoResult.Dump();
            continue;
        }
        if (accesseeAclList.Contains("pointTopointAcl") && accesserAclList.Contains("pointTopointAcl")) {
            LOGI("have point_to_point acl bundleName: %{public}s.", item->proxyAccesser.bundleName.c_str());
            aclNegoResult["pointTopointAcl"] = DM_POINT_TO_POINT;
            item->proxyAccesser.aclTypeList = aclNegoResult.Dump();
            item->proxyAccesser.isAuthed = true;
            continue;
        }
        item->proxyAccesser.aclTypeList = aclNegoResult.Dump();
    }
}

void AuthSrcConfirmState::GetSrcCredType(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo, JsonObject &aclInfo, JsonObject &credTypeJson)
{
    CHECK_NULL_VOID(context);
    std::vector<std::string> deleteCredInfo;
    for (const auto &item : credInfo.Items()) {
        if (!item.Contains(FILED_CRED_ID) || !item[FILED_CRED_ID].IsString()) {
            continue;
        }
        if (!item.Contains(FILED_CRED_TYPE) || !item[FILED_CRED_TYPE].IsNumberInteger()) {
            deleteCredInfo.push_back(item[FILED_CRED_ID].Get<std::string>());
            DirectlyDeleteCredential(context, context->accesser.userId, item);
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
                    DirectlyDeleteCredential(context, context->accesser.userId, item);
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
    GetSrcProxyCredTypeForP2P(context, deleteCredInfo);
    for (const auto &item : deleteCredInfo) {
        credInfo.Erase(item);
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
        DeleteCredential(context, context->accesser.userId, credObj, context->accesser.aclProfiles[DM_POINT_TO_POINT]);
    } else {
        credTypeJson["pointTopointCredType"] = credType;
        context->accesser.credentialInfos[credType] = credObj.Dump();
    }
}

void AuthSrcConfirmState::GetSrcProxyCredTypeForP2P(std::shared_ptr<DmAuthContext> context,
    std::vector<std::string> &deleteCredInfo)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end(); ++item) {
        JsonObject credInfoJson;
        if (!item->proxyAccesser.credInfoJson.empty()) {
            credInfoJson.Parse(item->proxyAccesser.credInfoJson);
        }
        for (const auto &credItem : credInfoJson.Items()) {
            if (!credItem.Contains(FILED_CRED_ID) || !credItem[FILED_CRED_ID].IsString()) {
                continue;
            }
            if (!credItem.Contains(FILED_CRED_TYPE) || !credItem[FILED_CRED_TYPE].IsNumberInteger()) {
                deleteCredInfo.push_back(credItem[FILED_CRED_ID].Get<std::string>());
                DirectlyDeleteCredential(context, context->accesser.userId, credItem);
                continue;
            }
            int32_t credType = credItem[FILED_CRED_TYPE].Get<int32_t>();
            if (credType != static_cast<int32_t>(DM_POINT_TO_POINT)) {
                continue;
            }
            std::string credId = credItem[FILED_CRED_ID].Get<std::string>();
            JsonObject aclTypeJson;
            if (!item->proxyAccesser.aclTypeList.empty()) {
                aclTypeJson.Parse(item->proxyAccesser.aclTypeList);
            }
            if (!aclTypeJson.Contains("pointTopointAcl") ||
                item->proxyAccesser.aclProfiles.find(DM_POINT_TO_POINT) == item->proxyAccesser.aclProfiles.end() ||
                (item->proxyAccesser.aclProfiles[DM_POINT_TO_POINT].GetAccessee().GetAccesseeCredentialIdStr() !=
                    credItem[FILED_CRED_ID].Get<std::string>() &&
                item->proxyAccesser.aclProfiles[DM_POINT_TO_POINT].GetAccesser().GetAccesserCredentialIdStr() !=
                credItem[FILED_CRED_ID].Get<std::string>())) {
                deleteCredInfo.push_back(credItem[FILED_CRED_ID].Get<std::string>());
                DeleteCredential(context, context->accesser.userId, credItem,
                    item->proxyAccesser.aclProfiles[DM_POINT_TO_POINT]);
                continue;
            }
            JsonObject validCredInfoJson;
            if (!item->proxyAccesser.credTypeList.empty()) {
                validCredInfoJson.Parse(item->proxyAccesser.credTypeList);
            }
            validCredInfoJson["pointTopointCredType"] = credType;
            item->proxyAccesser.credTypeList = validCredInfoJson.Dump();
            item->proxyAccesser.credentialInfos[credType] = credItem.Dump();
        }
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
                if (context->accessee.accountIdHash != context->accesser.accountIdHash ||
                    context->accesser.accountId != item.GetAccesser().GetAccesserAccountId()) {
                    DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.GetAccessControlId());
                    break;
                }
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
    GetSrcProxyAclInfoForP2P(context, profile);
}

void AuthSrcConfirmState::GetSrcProxyAclInfoForP2P(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::AccessControlProfile &profile)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    for (auto &app : context->subjectProxyOnes) {
        if ((profile.GetAccesser().GetAccesserTokenId() == app.proxyAccesser.tokenId &&
            Crypto::GetTokenIdHash(std::to_string(profile.GetAccessee().GetAccesseeTokenId())) ==
            app.proxyAccessee.tokenIdHash) ||
            (profile.GetAccessee().GetAccesseeTokenId() == app.proxyAccesser.tokenId &&
            Crypto::GetTokenIdHash(std::to_string(profile.GetAccesser().GetAccesserTokenId())) ==
            app.proxyAccessee.tokenIdHash)) {
            std::string credId;
            if (!IsAclHasCredential(profile, app.proxyAccesser.credInfoJson, credId)) {
                continue;
            }
            std::vector<std::string> appList;
            JsonObject credInfoJsonObj;
            if (!app.proxyAccesser.credInfoJson.empty()) {
                credInfoJsonObj.Parse(app.proxyAccesser.credInfoJson);
            }
            credInfoJsonObj[credId][FILED_AUTHORIZED_APP_LIST].Get(appList);
            const size_t APP_LIST_SIZE = 2;
            if (appList.size() < APP_LIST_SIZE ||
                std::find(appList.begin(), appList.end(),
                    std::to_string(profile.GetAccesser().GetAccesserTokenId())) == appList.end() ||
                std::find(appList.begin(), appList.end(),
                    std::to_string(profile.GetAccessee().GetAccesseeTokenId())) == appList.end()) {
                continue;
            }
            JsonObject aclTypeJson;
            if (!app.proxyAccesser.aclTypeList.empty()) {
                aclTypeJson.Parse(app.proxyAccesser.aclTypeList);
            }
            aclTypeJson["pointTopointAcl"] = DM_POINT_TO_POINT;
            app.proxyAccesser.aclTypeList = aclTypeJson.Dump();
            app.proxyAccesser.aclProfiles[DM_POINT_TO_POINT] = profile;
        }
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
            DeleteAclAndSk(context, profile);
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
    if (!DmAuthState::IsMatchCredentialAndP2pACL(credInfo, credId, profile)) {
        LOGE("acl bindlevel and credential authorizedScope not match");
        DeleteAcl(context, profile);
        credInfo.Erase(credId);
        return;
    }
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
            DeleteAclAndSk(context, profile);
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
        if (!item.Contains(FILED_CRED_ID) || !item[FILED_CRED_ID].IsString()) {
            continue;
        }
        uint32_t credType = DmAuthState::GetCredentialType(context, item);
        if (credType == DM_INVALIED_TYPE || !item.Contains(FILED_CRED_TYPE) ||
            !item[FILED_CRED_TYPE].IsNumberInteger()) {
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
    int32_t ret = context->hiChainAuthConnector->QueryCredentialInfo(context->accesser.userId, queryParams, credInfo);
    if (ret != DM_OK) {
        LOGE("QueryCredentialInfo failed ret %{public}d.", ret);
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
    int32_t ret = context->hiChainAuthConnector->QueryCredentialInfo(context->accesser.userId, queryParams, credInfo);
    if (ret != DM_OK) {
        LOGE("QueryCredentialInfo failed ret %{public}d.", ret);
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
    int32_t ret = context->hiChainAuthConnector->QueryCredentialInfo(context->accesser.userId, queryParams, credInfo);
    if (ret != DM_OK) {
        LOGE("QueryCredentialInfo failed ret %{public}d.", ret);
    }
}

void AuthSrcConfirmState::GenerateCertificate(std::shared_ptr<DmAuthContext> context)
{
    if (context == nullptr) {
        LOGE("context is nullptr!");
        return;
    }
#ifdef DEVICE_MANAGER_COMMON_FLAG
    context->accesser.isCommonFlag = true;
    LOGI("open device do not generate cert!");
    context->accesser.cert = "common";
#else
    DmCertChain dmCertChain;
    int32_t certRet = -1;
    if (CompareVersion(context->accessee.dmVersion, DM_VERSION_5_1_3)) {
        certRet = AuthCert::GetInstance().GenerateCertificateV2(dmCertChain, context->accessee.certRandom);
    } else {
        certRet = AuthCert::GetInstance().GenerateCertificate(dmCertChain);
    }
    if (certRet != DM_OK) {
        LOGE("generate cert fail, certRet = %{public}d", certRet);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(context->certMtx_);
        context->accesser.cert = AuthAttestCommon::GetInstance().SerializeDmCertChain(&dmCertChain);
    }
    context->certCV_.notify_all();
    AuthAttestCommon::GetInstance().FreeDmCertChain(dmCertChain);
#endif
    return;
}

int32_t AuthSrcConfirmState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcConfirmState start.");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    context->timer->DeleteTimer(std::string(NEGOTIATE_TIMEOUT_TASK));
    ResetBindLevel(context);
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
    NegotiateProxyCredential(context);
    // update acl negotiate result
    JsonObject aclNegoResult;
    NegotiateAcl(context, aclNegoResult);
    context->accesser.aclTypeList = aclNegoResult.Dump();
    NegotiateProxyAcl(context);
    NegotiateUltrasonic(context);
    uint32_t credType = 0;
    uint32_t aclType = 0;
    if (IsUint32(credTypeNegoResult, "identicalCredType")) {
        credType = credTypeNegoResult["identicalCredType"].Get<uint32_t>();
    }
    if (IsUint32(aclNegoResult, "identicalAcl")) {
        aclType = aclNegoResult["identicalAcl"].Get<uint32_t>();
    }
    if (credType == DM_IDENTICAL_ACCOUNT && aclType != DM_IDENTICAL_ACCOUNT) {
        context->softbusConnector->JoinLnn(context->accessee.addr, true);
        context->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
        return DM_OK;
    }
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_REQ_USER_CONFIRM, context);
    // generate cert sync
    ffrt::submit([=]() { GenerateCertificate(context);});
    context->listener->OnAuthResult(context->processInfo, context->peerTargetId.deviceId, context->accessee.tokenIdHash,
        static_cast<int32_t>(STATUS_DM_SHOW_AUTHORIZE_UI), DM_OK);
    context->listener->OnBindResult(context->processInfo, context->peerTargetId,
        DM_OK, static_cast<int32_t>(STATUS_DM_SHOW_AUTHORIZE_UI), "");
    context->timer->StartTimer(std::string(CONFIRM_TIMEOUT_TASK),
        DmAuthState::GetTaskTimeout(context, CONFIRM_TIMEOUT_TASK, CONFIRM_TIMEOUT),
        [context] (std::string name) {
            HandleAuthenticateTimeout(context, name);
        });
    return DM_OK;
}

void AuthSrcConfirmState::NegotiateUltrasonic(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (context->authType != AUTH_TYPE_PIN_ULTRASONIC) {
        LOGE("auth type not ultrasonic.");
        return;
    }
    if (context->accessee.extraInfo.empty()) {
        LOGE("extraInfo empty.");
        return;
    }
    JsonObject json;
    json.Parse(context->accessee.extraInfo);
    if (json.IsDiscarded()) {
        LOGE("extraInfo invalid.");
        return;
    }
    bool isSupportUltrasonic = true;
    if (IsBool(json, TAG_IS_SUPPORT_ULTRASONIC)) {
        isSupportUltrasonic = json[TAG_IS_SUPPORT_ULTRASONIC].Get<bool>();
    }
    if (!isSupportUltrasonic) {
        context->authType = AUTH_TYPE_PIN;
    }
}

void AuthSrcConfirmState::ResetBindLevel(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (context->accesser.bindLevel != DmRole::DM_ROLE_USER || !context->IsProxyBind) {
        return;
    }
    context->accesser.bindLevel = DmRole::DM_ROLE_SA;
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
    GetBundleLabel(context);
    CreateProxyData(context, jsonObj);
    const std::string params = jsonObj.Dump();
    DmDialogManager::GetInstance().ShowConfirmDialog(params);

    LOGI("AuthSinkConfirmState::ShowConfigDialog end");
    return DM_OK;
}

void AuthSinkConfirmState::GetBundleLabel(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Get ability manager failed");
        return;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (object == nullptr) {
        LOGE("object is NULL.");
        return;
    }

    sptr<OHOS::AppExecFwk::IBundleMgr> bms = iface_cast<OHOS::AppExecFwk::IBundleMgr>(object);
    if (bms == nullptr) {
        LOGE("bundle manager service is NULL.");
        return;
    }

    auto bundleResourceProxy = bms->GetBundleResourceProxy();
    if (bundleResourceProxy == nullptr) {
        LOGE("GetBundleResourceProxy fail");
        return;
    }
    for (auto &app : context->subjectProxyOnes) {
        AppExecFwk::BundleResourceInfo resourceInfo;
        auto result = bundleResourceProxy->GetBundleResourceInfo(app.proxyAccessee.bundleName,
            static_cast<uint32_t>(OHOS::AppExecFwk::ResourceFlag::GET_RESOURCE_INFO_ALL), resourceInfo);
        if (result == ERR_OK) {
            app.proxyAccessee.pkgLabel = resourceInfo.label;
        }
    }
}

int32_t AuthSinkConfirmState::CreateProxyData(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObj)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        jsonObj[PARAM_KEY_IS_PROXY_BIND] = false;
        return DM_OK;
    }
    jsonObj[PARAM_KEY_IS_PROXY_BIND] = context->IsProxyBind;
    JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    for (const auto &app : context->subjectProxyOnes) {
        JsonObject object;
        object[TAG_HOST_PKGLABEL] = !app.proxyAccessee.pkgLabel.empty() ? app.proxyAccessee.pkgLabel :
            app.proxyAccessee.bundleName;
        object[TAG_BUNDLE_NAME] = app.proxyAccessee.bundleName;
        object[BUNDLE_INFO] = app.proxyAccessee.bundleInfo;
        allProxyObj.PushBack(object);
    }
    jsonObj[APP_USER_DATA] = allProxyObj.Dump();
    jsonObj[TITLE] = context->title;
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

void AuthSinkConfirmState::NegotiateProxyCredential(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end(); ++item) {
        JsonObject credTypeNegoResult;
        JsonObject accesseeCredTypeList;
        accesseeCredTypeList.Parse(item->proxyAccessee.credTypeList);
        JsonObject accesserCredTypeList;
        accesserCredTypeList.Parse(item->proxyAccesser.credTypeList);
        if (accesseeCredTypeList.IsDiscarded() || accesserCredTypeList.IsDiscarded()) {
            item->proxyAccessee.credTypeList = credTypeNegoResult.Dump();
            continue;
        }
        if (accesseeCredTypeList.Contains("pointTopointCredType") &&
            accesserCredTypeList.Contains("pointTopointCredType")) {
            credTypeNegoResult["pointTopointCredType"] = DM_POINT_TO_POINT;
            item->proxyAccessee.credTypeList = credTypeNegoResult.Dump();
            continue;
        }
        item->proxyAccessee.credTypeList = credTypeNegoResult.Dump();
    }
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
        context->accessee.isUserLevelAuthed = true;
    }
    if (accesseeAclList.Contains("shareCredType") && accesserAclList.Contains("shareCredType")) {
        LOGI("have share acl.");
        aclNegoResult["shareAcl"] = DM_SHARE;
        context->accessee.isPutLnnAcl = false;
        context->accessee.isAuthed = true;
        context->accessee.isUserLevelAuthed = true;
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

void AuthSinkConfirmState::NegotiateProxyAcl(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end(); ++item) {
        JsonObject aclNegoResult;
        JsonObject accesseeAclList;
        if (!item->proxyAccessee.aclTypeList.empty()) {
            accesseeAclList.Parse(item->proxyAccessee.aclTypeList);
        }
        JsonObject accesserAclList;
        if (!item->proxyAccesser.aclTypeList.empty()) {
            accesserAclList.Parse(item->proxyAccesser.aclTypeList);
        }
        if (accesseeAclList.IsDiscarded() || accesserAclList.IsDiscarded()) {
            item->proxyAccessee.aclTypeList = aclNegoResult.Dump();
            continue;
        }
        if (accesseeAclList.Contains("pointTopointAcl") && accesserAclList.Contains("pointTopointAcl")) {
            LOGI("have point_to_point acl bundleName: %{public}s.", item->proxyAccesser.bundleName.c_str());
            aclNegoResult["pointTopointAcl"] = DM_POINT_TO_POINT;
            item->proxyAccessee.aclTypeList = aclNegoResult.Dump();
            item->proxyAccessee.isAuthed = true;
            continue;
        }
        item->proxyAccessee.aclTypeList = aclNegoResult.Dump();
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
        context->srvExtarInfo = srvInfo.GetExtraInfo();
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
    NegotiateProxyCredential(context);
    NegotiateAcl(context, aclNegoResult);
    context->accessee.aclTypeList = aclNegoResult.Dump();
    NegotiateProxyAcl(context);
    if (credTypeNegoResult.Dump() != context->accesser.credTypeList ||
        aclNegoResult.Dump() != context->accesser.aclTypeList) {
        LOGE("compability negotiate not match.");
        context->reason = ERR_DM_CAPABILITY_NEGOTIATE_FAILED;
        return ERR_DM_FAILED;
    }
    if (!ProcessServerAuthorize(context)) {
        LOGE("no srvExtarInfo");
        context->reason = ERR_DM_AUTH_PEER_REJECT;
        return ERR_DM_FAILED;
    }
    int32_t ret = NegotiateProcess::GetInstance().HandleNegotiateResult(context);
    if (ret != DM_OK) {
        LOGE("HandleNegotiateResult failed ret %{public}d.", ret);
        context->reason = ret;
        return ret;
    }
    if (IsNeedBind(context)) {
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
    if ((DmAuthState::IsImportAuthCodeCompatibility(context->authType) ||
        context->authType == DmAuthType::AUTH_TYPE_PIN_ULTRASONIC) &&
        (context->serviceInfoFound || AuthSinkStatePinAuthComm::IsAuthCodeReady(context)) &&
        context->authBoxType == DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_USER_CONFIRM, context);
        return DM_OK;
    }
    if ((context->authType == DmAuthType::AUTH_TYPE_PIN || context->authType == DmAuthType::AUTH_TYPE_NFC ||
        context->authType == DmAuthType::AUTH_TYPE_PIN_ULTRASONIC) &&
        context->authBoxType == DMLocalServiceInfoAuthBoxType::STATE3) {
        return ProcessUserAuthorize(context);
    }
    context->confirmOperation = UiAction::USER_OPERATION_TYPE_CANCEL_AUTH;
    return ERR_DM_FAILED;
}

int32_t AuthSinkConfirmState::ProcessUserAuthorize(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
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
        context->reason = ERR_DM_AUTH_PEER_REJECT;
        return ERR_DM_FAILED;
    }
    if (!ProcessUserOption(context, context->userOperationParam)) {
        LOGE("user reject");
        context->reason = ERR_DM_AUTH_PEER_REJECT;
        return ERR_DM_FAILED;
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_USER_CONFIRM, context);
    return DM_OK;
}

bool AuthSinkConfirmState::ProcessUserOption(std::shared_ptr<DmAuthContext> context,
    const std::string &authorizeInfo)
{
    if (context == nullptr) {
        return false;
    }
    if (!context->IsProxyBind) {
        return true;
    }
    if (authorizeInfo.empty() && !context->IsCallingProxyAsSubject) {
        LOGE("no proxy data");
        context->subjectProxyOnes.clear();
        return false;
    }
    JsonObject jsonObj;
    jsonObj.Parse(authorizeInfo);
    if (jsonObj.IsDiscarded() || !jsonObj.Contains(APP_USER_DATA) ||
        !IsArray(jsonObj, APP_USER_DATA)) {
        context->subjectProxyOnes.clear();
        LOGE("no subject proxy data");
        return context->IsCallingProxyAsSubject;
    }
    JsonObject appDataObj;
    std::string appDataStr = jsonObj[APP_USER_DATA].Dump();
    appDataObj.Parse(appDataStr);
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end();) {
        if (IsUserAuthorize(appDataObj, item->proxyAccessee)) {
            item++;
        } else {
            item = context->subjectProxyOnes.erase(item);
        }
    }
    return context->subjectProxyOnes.size() > 0 || context->IsCallingProxyAsSubject;
}

bool AuthSinkConfirmState::ProcessServerAuthorize(std::shared_ptr<DmAuthContext> context)
{
    if (context == nullptr) {
        return false;
    }
    if (!context->IsProxyBind) {
        return true;
    }
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    auto ret = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(
        context->accessee.pkgName, context->authType, srvInfo);
    if (ret != OHOS::DistributedDeviceProfile::DP_SUCCESS) {
        LOGE("ReadServiceInfo not found");
        return false;
    }
    std::string srvExtarInfo = srvInfo.GetExtraInfo();
    if (srvExtarInfo.empty()) {
        LOGE("no proxy data");
        context->subjectProxyOnes.clear();
        return false;
    }
    JsonObject jsonObj;
    jsonObj.Parse(srvExtarInfo);
    if (jsonObj.IsDiscarded() || !jsonObj.Contains(APP_USER_DATA) ||
        !IsArray(jsonObj, APP_USER_DATA)) {
        context->subjectProxyOnes.clear();
        LOGE("no subject proxy data");
        return false;
    }
    if (IsString(jsonObj, TITLE)) {
        context->title = jsonObj[TITLE].Get<std::string>();
    }
    JsonObject appDataObj;
    std::string appDataStr = jsonObj[APP_USER_DATA].Dump();
    appDataObj.Parse(appDataStr);
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end();) {
        if (IsUserAuthorize(appDataObj, item->proxyAccessee)) {
            item++;
        } else {
            item = context->subjectProxyOnes.erase(item);
        }
    }
    return context->subjectProxyOnes.size() > 0;
}

bool AuthSinkConfirmState::IsUserAuthorize(JsonObject &paramObj, DmProxyAccess &access)
{
    if (paramObj.IsDiscarded()) {
        return false;
    }
    for (auto const &item : paramObj.Items()) {
        if (!IsString(item, TAG_BUNDLE_NAME)) {
            continue;
        }
        if (access.bundleName == item[TAG_BUNDLE_NAME].Get<std::string>()) {
            if (IsString(item, BUNDLE_INFO)) {
                access.bundleInfo = item[BUNDLE_INFO].Get<std::string>();
            }
            return true;
        }
    }
    return false;
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
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkCredentialAuthStartState>());
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_USER_CONFIRM, context);
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