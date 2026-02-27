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

#include <cstdlib>
#include <map>

#include "accesstoken_kit.h"
#include "access_control_profile.h"
#include "accesser.h"
#include "accessee.h"
#include "auth_manager.h"
#include "app_manager.h"
#include "business_event.h"
#include "distributed_device_profile_client.h"
#include "dm_auth_cert.h"
#include "dm_auth_attest_common.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "dm_timer.h"
#include "dm_radar_helper.h"
#include "dm_language_manager.h"
#include "dm_constants.h"
#include "dm_anonymous.h"
#include "dm_random.h"
#include "dm_auth_context.h"
#include "dm_auth_state.h"
#include "dm_freeze_process.h"
#include "deviceprofile_connector.h"
#include "distributed_device_profile_errors.h"
#include "device_auth.h"
#include "hap_token_info.h"
#include "json_object.h"
#include "multiple_user_connector.h"
#include "os_account_manager.h"
#include "parameter.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {
namespace {
    const char* DM_DISTURBANCE_EVENT_KEY = "business_id_cast+_disturbance_event";
    const char* DM_ANTI_DISTURBANCE_MODE = "is_in_anti_disturbance_mode";
    const int64_t DM_MIN_RANDOM = 1;
    const int64_t DM_MAX_RANDOM_INT64 = INT64_MAX;
    const int32_t CAR_CENTRAL_CONTROL_SCREEN_DISPLAYID = 0;
    constexpr int8_t SERVICE_UNPUBLISHED_STATE = 0;
    const int32_t INVALID_USERID = -1;
}

DmAuthStateType AuthSrcStartState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_START_STATE;
}

int32_t AuthSrcStartState::Action(std::shared_ptr<DmAuthContext> context)
{
    return DM_OK;
}

DmAuthStateType AuthSrcNegotiateStateMachine::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_NEGOTIATE_STATE;
}

std::string AuthSrcNegotiateStateMachine::GetAccountGroupIdHash(std::shared_ptr<DmAuthContext> context)
{
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP;
    std::string queryParams = jsonObj.Dump();

    int32_t osAccountUserId = context->accesser.userId;
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return "";
    }
    std::vector<GroupInfo> groupList;
    if (!context->hiChainConnector->GetGroupInfo(osAccountUserId, queryParams, groupList)) {
        return "";
    }
    JsonObject jsonAccountObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    for (auto &groupInfo : groupList) {
        jsonAccountObj.PushBack(Crypto::GetGroupIdHash(groupInfo.groupId));
    }
    return jsonAccountObj.Dump();
}

int32_t AuthSrcNegotiateStateMachine::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcNegotiateStateMachine::Action sessionId %{public}d.", context->sessionId);

    context->reply = ERR_DM_AUTH_REJECT;
    context->accessee.dmVersion = "";

    // Calculate the hash value
    context->accesser.deviceIdHash = Crypto::GetUdidHash(context->accesser.deviceId);
    context->accesser.accountIdHash = Crypto::GetAccountIdHash16(context->accesser.accountId);
    context->accesser.tokenIdHash = Crypto::GetTokenIdHash(std::to_string(context->accesser.tokenId));

    // Create old message for compatible
    context->accesser.accountGroupIdHash = GetAccountGroupIdHash(context);

    std::string message = context->authMessageProcessor->CreateMessage(MSG_TYPE_REQ_ACL_NEGOTIATE, context);
    context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
    if (context->timer != nullptr) {
        context->timer->StartTimer(std::string(NEGOTIATE_TIMEOUT_TASK),
            DmAuthState::GetTaskTimeout(context, NEGOTIATE_TIMEOUT_TASK, NEGOTIATE_TIMEOUT),
            [this, context] (std::string name) {
                DmAuthState::HandleAuthenticateTimeout(context, name);
            });
    }

    return DM_OK;
}

DmAuthStateType AuthSinkNegotiateStateMachine::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_NEGOTIATE_STATE;
}

int32_t AuthSinkNegotiateStateMachine::RespQueryAcceseeIds(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    // 1. Get deviceId
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    context->accessee.deviceId = std::string(localDeviceId);
    context->accessee.deviceIdHash = Crypto::GetUdidHash(context->accessee.deviceId);
    // 2. Get userId
    int32_t deviceType = context->softbusConnector->GetLocalDeviceTypeId();
    context->accessee.userId = GetSinkUserIdByDeviceType(context,
        static_cast<DmDeviceType> (deviceType));
    if (context->accessee.userId < 0) {
        LOGE("get accessee userId failed.");
        return ERR_DM_GET_LOCAL_USERID_FAILED;
    }
    // 3. Get accountId
    context->accessee.accountId = MultipleUserConnector::GetOhosAccountIdByUserId(context->accessee.userId);
    context->accessee.accountIdHash = Crypto::GetAccountIdHash16(context->accessee.accountId);
    // 4. Get tokenId
    if (AppManager::GetInstance().GetNativeTokenIdByName(context->accessee.bundleName,
        context->accessee.tokenId) == DM_OK) {
        context->accessee.bindLevel = DmRole::DM_ROLE_SA;
    } else if (AppManager::GetInstance().GetHapTokenIdByName(context->accessee.userId, context->accessee.bundleName, 0,
        context->accessee.tokenId) == DM_OK) {
        context->accessee.bindLevel = DmRole::DM_ROLE_FA;
    } else {
        LOGE("sink not contain the bundlename %{public}s.", context->accessee.bundleName.c_str());
        return ERR_DM_GET_TOKENID_FAILED;
    }
    if (!context->IsProxyBind && AuthManagerBase::CheckProcessNameInWhiteList(context->accessee.bundleName)) {
        context->accessee.bindLevel = DmRole::DM_ROLE_USER;
    }
    context->accessee.tokenIdHash = Crypto::GetTokenIdHash(std::to_string(context->accessee.tokenId));
    context->accesser.isOnline = context->softbusConnector->CheckIsOnline(context->accesser.deviceIdHash, true);
    context->accessee.language = DmLanguageManager::GetInstance().GetSystemLanguage();
    context->accessee.deviceName = context->listener->GetLocalDisplayDeviceName();
    context->accessee.networkId = context->softbusConnector->GetLocalDeviceNetworkId();
    return RespQueryProxyAcceseeIds(context);
}

int32_t AuthSinkNegotiateStateMachine::GetSinkUserIdByDeviceType(std::shared_ptr<DmAuthContext> context,
    DmDeviceType deviceType)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    LOGI("displayId = %{public}d", context->accessee.displayId);
    if (deviceType == DmDeviceType::DEVICE_TYPE_CAR) {
        return GetSinkCarUserId(context);
    }
    return MultipleUserConnector::GetUserIdByDisplayId(context->accessee.displayId);
}

int32_t AuthSinkNegotiateStateMachine::GetSinkCarUserId(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    LOGI("GetSinkCarUserId start.");
    int32_t mainScreenUserId = MultipleUserConnector::GetUserIdByDisplayId(CAR_CENTRAL_CONTROL_SCREEN_DISPLAYID);
    if (mainScreenUserId < 0) {
        LOGE("mainScreenUserId = %{public}d is invalid.", mainScreenUserId);
        return INVALID_USERID;
    }
    if (AppManager::GetInstance().GetNativeTokenIdByName(context->accessee.bundleName,
        context->accessee.tokenId) == DM_OK) {
        if (context->accessee.displayId == -1) {
            LOGI("not transmit peer displayId, return mainScreenUserId");
            return mainScreenUserId;
        }
        if (context->accessee.displayId != CAR_CENTRAL_CONTROL_SCREEN_DISPLAYID) {
            LOGE("accessee.displayId = %{public}d is not control screen.",
                context->accessee.displayId);
            return INVALID_USERID;
        }
    }
    return mainScreenUserId;
}

int32_t AuthSinkNegotiateStateMachine::RespQueryProxyAcceseeIds(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind) {
        return DM_OK;
    }
    if (context->subjectProxyOnes.empty()) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end(); ++item) {
        if (AppManager::GetInstance().GetNativeTokenIdByName(item->proxyAccessee.bundleName,
            item->proxyAccessee.tokenId) == DM_OK) {
            item->proxyAccessee.bindLevel = DmRole::DM_ROLE_SA;
        } else if (AppManager::GetInstance().GetHapTokenIdByName(context->accessee.userId,
            item->proxyAccessee.bundleName, 0, item->proxyAccessee.tokenId) == DM_OK) {
            item->proxyAccessee.bindLevel = DmRole::DM_ROLE_FA;
        } else {
            LOGE("sink not contain the bundlename %{public}s.", item->proxyAccessee.bundleName.c_str());
            return ERR_DM_GET_TOKENID_FAILED;
        }
        item->proxyAccessee.tokenIdHash = Crypto::GetTokenIdHash(std::to_string(item->proxyAccessee.tokenId));
    }
    return DM_OK;
}

int32_t AuthSinkNegotiateStateMachine::ProcRespNegotiate5_1_0(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    int32_t ret = RespQueryAcceseeIds(context);
    if (ret != DM_OK) {
        LOGE("DmAuthManager::ProcRespNegotiate5_1_0 fail to get all id.");
        return ret;
    }
    DeleteInvalidCredAndAcl(context);
    JsonObject credInfo;
    GetSinkCredentialInfo(context, credInfo);
    JsonObject aclTypeJson;
    GetSinkAclInfo(context, credInfo, aclTypeJson);
    context->accessee.aclTypeList = aclTypeJson.Dump();
    JsonObject credTypeJson;
    GetSinkCredType(context, credInfo, aclTypeJson, credTypeJson);
    context->accessee.credTypeList = credTypeJson.Dump();
    if (context->isServiceBind) {
        ret = SinkNegotiateService(context);
        return ret;
    }
    return DM_OK;
}

int32_t AuthSinkNegotiateStateMachine::SinkNegotiateService(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    int64_t serviceId = context->accessee.serviceId;
    if (serviceId == 0) {
        LOGE("service id invalid.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    ServiceInfoProfile serviceInfoProfile;
    int32_t ret = DeviceProfileConnector::GetInstance().GetServiceInfoProfileByServiceId(serviceId,
        serviceInfoProfile);
    if (ret != DM_OK) {
        LOGE("GetServiceInfoProfileByServiceId failed, ret %{public}d.", ret);
        return ret;
    }
    if (serviceInfoProfile.publishState == SERVICE_UNPUBLISHED_STATE) {
        LOGE("service id not publish.");
        return ERR_DM_SERVICE_BIND_PEER_SERVICE_ID_UNPUBLISH;
    }
    return DM_OK;
}

int32_t AuthSinkNegotiateStateMachine::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkNegotiateStateMachine::Action sessionid %{public}d", context->sessionId);
    if (IsAntiDisturbanceMode(context->businessId)) {
        LOGI("Sink is AntiDisturbMode.");
        context->reason = ERR_DM_ANTI_DISTURB_MODE;
        return ERR_DM_ANTI_DISTURB_MODE;
    }
    int64_t remainingFrozenTime = 0;
    if (FreezeProcess::GetInstance().IsFrozen(remainingFrozenTime)) {
        LOGE("Device is Frozen");
        context->remainingFrozenTime = remainingFrozenTime;
        return ERR_DM_DEVICE_FROZEN;
    }
    // 1. Create an authorization timer
    if (context->timer != nullptr) {
        context->timer->StartTimer(std::string(AUTHENTICATE_TIMEOUT_TASK),
            AUTHENTICATE_TIMEOUT,
            [this, context] (std::string name) {
                DmAuthState::HandleAuthenticateTimeout(context, name);
        });
    }

    // To be compatible with historical versions, use ConvertSrcVersion to get the actual version on the source side.
    std::string preVersion = std::string(DM_VERSION_5_0_OLD_MAX);
    LOGI("AuthSinkNegotiateStateMachine::Action start version compare %{public}s to %{public}s",
        context->accesser.dmVersion.c_str(), preVersion.c_str());
    if (CompareVersion(context->accesser.dmVersion, preVersion) == false) {
        LOGE("AuthSinkNegotiateStateMachine::Action incompatible version");
        context->reason = ERR_DM_VERSION_INCOMPATIBLE;
        return ERR_DM_VERSION_INCOMPATIBLE;
    }
    SetIsProxyBind(context);
    int32_t ret = ProcRespNegotiate5_1_0(context);
    if (ret != DM_OK) {
        LOGE("AuthSinkNegotiateStateMachine::Action proc response negotiate failed");
        context->reason = ret;
        return ret;
    }
    context->accessee.certRandom = static_cast<uint64_t>(GenRandLongLong(DM_MIN_RANDOM, DM_MAX_RANDOM_INT64));
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_ACL_NEGOTIATE, context);
    context->timer->StartTimer(std::string(WAIT_REQUEST_TIMEOUT_TASK),
        DmAuthState::GetTaskTimeout(context, WAIT_REQUEST_TIMEOUT_TASK, WAIT_REQUEST_TIMEOUT),
        [this, context] (std::string name) {
            DmAuthState::HandleAuthenticateTimeout(context, name);
        });
    return DM_OK;
}

void AuthSinkNegotiateStateMachine::SetIsProxyBind(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind) {
        return;
    }
    if (AuthManagerBase::CheckProcessNameInProxyAdaptationList(context->accessee.pkgName)) {
        LOGI("%{public}s in proxy adaptation list", context->accessee.pkgName.c_str());
        return;
    }
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    auto ret = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(
        context->accessee.pkgName, context->authType, srvInfo);
    if (ret != OHOS::DistributedDeviceProfile::DP_SUCCESS) {
        context->IsProxyBind = false;
        context->subjectProxyOnes.clear();
        LOGI("ReadServiceInfo not found");
        return;
    }
    std::string srvExtarInfo = srvInfo.GetExtraInfo();
    if (srvExtarInfo.empty()) {
        LOGI("no srvExtarInfo data");
        context->IsProxyBind = false;
        context->subjectProxyOnes.clear();
        return;
    }
    JsonObject jsonObj;
    jsonObj.Parse(srvExtarInfo);
    if (jsonObj.IsDiscarded() || !jsonObj.Contains(APP_USER_DATA) ||
        !IsArray(jsonObj, APP_USER_DATA)) {
        context->IsProxyBind = false;
        context->subjectProxyOnes.clear();
        LOGE("no subject proxy data");
        return;
    }
}

void AuthSinkNegotiateStateMachine::GetSinkCredType(std::shared_ptr<DmAuthContext> context,
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
            DirectlyDeleteCredential(context, context->accessee.userId, item);
            continue;
        }
        int32_t credType = item[FILED_CRED_TYPE].Get<int32_t>();
        LOGI("credType %{public}d.", credType);
        switch (credType) {
            case DM_IDENTICAL_ACCOUNT:
                credTypeJson["identicalCredType"] = credType;
                context->accessee.credentialInfos[credType] = item.Dump();
                break;
            case DM_SHARE:
                credTypeJson["shareCredType"] = credType;
                context->accessee.credentialInfos[credType] = item.Dump();
                break;
            case DM_POINT_TO_POINT:
                GetSinkCredTypeForP2P(context, item, aclInfo, credTypeJson, credType, deleteCredInfo);
                break;
            case DM_LNN:
                if (!aclInfo.Contains("lnnAcl") ||
                    (context->accessee.aclProfiles[DM_LNN].GetAccessee().GetAccesseeCredentialIdStr() !=
                    item[FILED_CRED_ID].Get<std::string>() &&
                    context->accessee.aclProfiles[DM_LNN].GetAccesser().GetAccesserCredentialIdStr() !=
                    item[FILED_CRED_ID].Get<std::string>())) {
                    deleteCredInfo.push_back(item[FILED_CRED_ID].Get<std::string>());
                    DirectlyDeleteCredential(context, context->accessee.userId, item);
                } else {
                    credTypeJson["lnnCredType"] = credType;
                    context->accessee.credentialInfos[credType] = item.Dump();
                }
                break;
            default:
                LOGE("invalid credType %{public}d.", credType);
                break;
        }
    }
    GetSinkProxyCredTypeForP2P(context, deleteCredInfo);
    for (const auto &item : deleteCredInfo) {
        credInfo.Erase(item);
    }
}

void AuthSinkNegotiateStateMachine::GetSinkCredTypeForP2P(std::shared_ptr<DmAuthContext> context,
    const JsonItemObject &credObj, JsonObject &aclInfo, JsonObject &credTypeJson,
    int32_t credType, std::vector<std::string> &deleteCredInfo)
{
    CHECK_NULL_VOID(context);
    if (aclInfo.Contains("pointTopointAcl")) {
        credTypeJson["pointTopointCredType"] = credType;
        context->accessee.credentialInfos[credType] = credObj.Dump();
    }
}

void AuthSinkNegotiateStateMachine::GetSinkProxyCredTypeForP2P(std::shared_ptr<DmAuthContext> context,
    std::vector<std::string> &deleteCredInfo)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end(); ++item) {
        JsonObject credInfoJson;
        if (!item->proxyAccessee.credInfoJson.empty()) {
            credInfoJson.Parse(item->proxyAccessee.credInfoJson);
        }
        for (const auto &credItem : credInfoJson.Items()) {
            if (!credItem.Contains(FILED_CRED_ID) || !credItem[FILED_CRED_ID].IsString()) {
                continue;
            }
            if (!credItem.Contains(FILED_CRED_TYPE) || !credItem[FILED_CRED_TYPE].IsNumberInteger()) {
                deleteCredInfo.push_back(credItem[FILED_CRED_ID].Get<std::string>());
                DirectlyDeleteCredential(context, context->accessee.userId, credItem);
                continue;
            }
            int32_t credType = credItem[FILED_CRED_TYPE].Get<int32_t>();
            if (credType != static_cast<int32_t>(DM_POINT_TO_POINT)) {
                continue;
            }
            std::string credId = credItem[FILED_CRED_ID].Get<std::string>();
            JsonObject aclTypeJson;
            if (!item->proxyAccessee.aclTypeList.empty()) {
                aclTypeJson.Parse(item->proxyAccessee.aclTypeList);
            }
            if (!aclTypeJson.Contains("pointTopointAcl") ||
                item->proxyAccessee.aclProfiles.find(DM_POINT_TO_POINT) == item->proxyAccessee.aclProfiles.end() ||
                (item->proxyAccessee.aclProfiles[DM_POINT_TO_POINT].GetAccessee().GetAccesseeCredentialIdStr() !=
                    credItem[FILED_CRED_ID].Get<std::string>() &&
                item->proxyAccessee.aclProfiles[DM_POINT_TO_POINT].GetAccesser().GetAccesserCredentialIdStr() !=
                credItem[FILED_CRED_ID].Get<std::string>())) {
                deleteCredInfo.push_back(credItem[FILED_CRED_ID].Get<std::string>());
                DeleteCredential(context, context->accessee.userId, credItem,
                    item->proxyAccessee.aclProfiles[DM_POINT_TO_POINT]);
                continue;
            }
            JsonObject validCredInfoJson;
            if (!item->proxyAccessee.credTypeList.empty()) {
                validCredInfoJson.Parse(item->proxyAccessee.credTypeList);
            }
            validCredInfoJson["pointTopointCredType"] = credType;
            item->proxyAccessee.credTypeList = validCredInfoJson.Dump();
            item->proxyAccessee.credentialInfos[credType] = credItem.Dump();
        }
    }
}

void AuthSinkNegotiateStateMachine::GetSinkAclInfo(std::shared_ptr<DmAuthContext> context,
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
        if (trustDeviceIdHash != context->accesser.deviceIdHash &&
            trustDeviceIdHash != context->accessee.deviceIdHash) {
            LOGE("devId %{public}s hash %{public}s, er devId %{public}s.", GetAnonyString(trustDeviceId).c_str(),
                GetAnonyString(trustDeviceIdHash).c_str(), GetAnonyString(context->accesser.deviceIdHash).c_str());
            continue;
        }
        bindLevel = item.GetBindLevel();
        switch (item.GetBindType()) {
            case DM_IDENTICAL_ACCOUNT:
                if (context->accessee.accountIdHash != context->accesser.accountIdHash ||
                    context->accessee.accountId != item.GetAccesser().GetAccesserAccountId()) {
                    DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.GetAccessControlId());
                    break;
                }
                if (IdenticalAccountAclCompare(context, item.GetAccesser(), item.GetAccessee())) {
                    aclInfo["identicalAcl"] = DM_IDENTICAL_ACCOUNT;
                    context->accessee.aclProfiles[DM_IDENTICAL_ACCOUNT] = item;
                }
                break;
            case DM_SHARE:
                if (ShareAclCompare(context, item.GetAccesser(), item.GetAccessee()) &&
                    CheckCredIdInAcl(context, item, credInfo, DM_SHARE)) {
                    aclInfo["shareAcl"] = DM_SHARE;
                    context->accessee.aclProfiles[DM_SHARE] = item;
                }
                break;
            case DM_POINT_TO_POINT:
                GetSinkAclInfoForP2P(context, item, credInfo, aclInfo);
                break;
            default:
                LOGE("invalid bindType %{public}d.", item.GetBindType());
                break;
        }
    }
    if (aclInfo.Contains("pointTopointAcl") && !aclInfo.Contains("lnnAcl") && bindLevel != USER) {
        aclInfo.Erase("pointTopointAcl");
        DeleteAcl(context, context->accessee.aclProfiles[DM_POINT_TO_POINT]);
    }
}

void AuthSinkNegotiateStateMachine::GetSinkAclInfoForP2P(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, JsonObject &aclInfo)
{
    CHECK_NULL_VOID(context);
    if (Point2PointAclCompare(context, profile.GetAccesser(), profile.GetAccessee()) &&
        CheckCredIdInAcl(context, profile, credInfo, DM_POINT_TO_POINT)) {
        aclInfo["pointTopointAcl"] = DM_POINT_TO_POINT;
        context->accessee.aclProfiles[DM_POINT_TO_POINT] = profile;
    }
    if (LnnAclCompare(context, profile.GetAccesser(), profile.GetAccessee()) &&
        CheckCredIdInAcl(context, profile, credInfo, DM_LNN) && profile.GetBindLevel() == USER) {
        aclInfo["lnnAcl"] = DM_LNN;
        context->accessee.aclProfiles[DM_LNN] = profile;
    }
    GetSinkProxyAclInfoForP2P(context, profile);
}

void AuthSinkNegotiateStateMachine::GetSinkProxyAclInfoForP2P(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::AccessControlProfile &profile)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    for (auto &app : context->subjectProxyOnes) {
        if ((profile.GetAccessee().GetAccesseeTokenId() == app.proxyAccessee.tokenId &&
            Crypto::GetTokenIdHash(std::to_string(profile.GetAccesser().GetAccesserTokenId())) ==
            app.proxyAccesser.tokenIdHash) ||
            (profile.GetAccesser().GetAccesserTokenId() == app.proxyAccessee.tokenId &&
            Crypto::GetTokenIdHash(std::to_string(profile.GetAccessee().GetAccesseeTokenId())) ==
            app.proxyAccesser.tokenIdHash)) {
            std::string credId;
            if (!IsAclHasCredential(profile, app.proxyAccessee.credInfoJson, credId)) {
                DeleteAclAndSk(context, profile);
                continue;
            }
            std::vector<std::string> appList;
            JsonObject credInfoJsonObj;
            if (!app.proxyAccessee.credInfoJson.empty()) {
                credInfoJsonObj.Parse(app.proxyAccessee.credInfoJson);
            }
            credInfoJsonObj[credId][FILED_AUTHORIZED_APP_LIST].Get(appList);
            const size_t APP_LIST_SIZE = 2;
            if (appList.size() < APP_LIST_SIZE ||
                std::find(appList.begin(), appList.end(),
                    std::to_string(profile.GetAccesser().GetAccesserTokenId())) == appList.end() ||
                std::find(appList.begin(), appList.end(),
                    std::to_string(profile.GetAccessee().GetAccesseeTokenId())) == appList.end()) {
                DeleteAclAndSk(context, profile);
                continue;
            }
            JsonObject aclTypeJson;
            if (!app.proxyAccessee.aclTypeList.empty()) {
                aclTypeJson.Parse(app.proxyAccessee.aclTypeList);
            }
            aclTypeJson["pointTopointAcl"] = DM_POINT_TO_POINT;
            app.proxyAccessee.aclTypeList = aclTypeJson.Dump();
            app.proxyAccessee.aclProfiles[DM_POINT_TO_POINT] = profile;
        }
    }
}

bool AuthSinkNegotiateStateMachine::CheckCredIdInAcl(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, uint32_t bindType)
{
    std::string credId = profile.GetAccessee().GetAccesseeCredentialIdStr();
    if (credInfo.Contains(credId) && (!credInfo[credId].IsObject() || !credInfo[credId].Contains(FILED_CRED_TYPE) ||
        !credInfo[credId][FILED_CRED_TYPE].IsNumberInteger())) {
        LOGE("credId %{public}s contain credInfoJson invalid.", credId.c_str());
        credInfo.Erase(credId);
        DeleteAcl(context, profile);
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

void AuthSinkNegotiateStateMachine::CheckCredIdInAclForP2P(std::shared_ptr<DmAuthContext> context,
    std::string &credId, const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo,
    uint32_t bindType, bool &checkResult)
{
    if (!DmAuthState::IsMatchCredentialAndP2pACL(credInfo, credId, profile)) {
        LOGE("acl bindlevel and credential authorizedScope not match");
        DeleteAcl(context, profile);
        credInfo.Erase(credId);
        return;
    }
    if (!credInfo[credId].Contains(FILED_CRED_TYPE) ||
        !credInfo[credId][FILED_CRED_TYPE].IsNumberInteger()) {
        return;
    }
    if (credInfo[credId][FILED_CRED_TYPE].Get<uint32_t>() == bindType) {
        std::vector<std::string> appList;
        credInfo[credId][FILED_AUTHORIZED_APP_LIST].Get(appList);
        const size_t APP_LIST_SIZE = 2;
        if (appList.size() >= APP_LIST_SIZE &&
            std::find(appList.begin(), appList.end(),
                std::to_string(profile.GetAccesser().GetAccesserTokenId())) != appList.end() &&
            std::find(appList.begin(), appList.end(),
                std::to_string(profile.GetAccessee().GetAccesseeTokenId())) != appList.end()) {
            checkResult = true;
        } else {
            DeleteAclAndSk(context, profile);
        }
    } else {
        DeleteAcl(context, profile);
    }
}

bool AuthSinkNegotiateStateMachine::IdenticalAccountAclCompare(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee)
{
    LOGI("start");
    return accesser.GetAccesserDeviceId() == context->accessee.deviceId &&
        accesser.GetAccesserUserId() == context->accessee.userId &&
        Crypto::GetUdidHash(accessee.GetAccesseeDeviceId()) == context->accesser.deviceIdHash;
}

bool AuthSinkNegotiateStateMachine::ShareAclCompare(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee)
{
    LOGI("start");
    return accessee.GetAccesseeDeviceId() == context->accessee.deviceId &&
        accessee.GetAccesseeUserId() == context->accessee.userId &&
        Crypto::GetUdidHash(accesser.GetAccesserDeviceId()) == context->accesser.deviceIdHash;
}

bool AuthSinkNegotiateStateMachine::Point2PointAclCompare(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee)
{
    LOGI("start");
    return (accessee.GetAccesseeDeviceId() == context->accessee.deviceId &&
        accessee.GetAccesseeUserId() == context->accessee.userId &&
        accessee.GetAccesseeTokenId() == context->accessee.tokenId &&
        Crypto::GetUdidHash(accesser.GetAccesserDeviceId()) == context->accesser.deviceIdHash &&
        Crypto::GetTokenIdHash(std::to_string(accesser.GetAccesserTokenId())) == context->accesser.tokenIdHash) ||
        (accesser.GetAccesserDeviceId() == context->accessee.deviceId &&
        accesser.GetAccesserUserId() == context->accessee.userId &&
        accesser.GetAccesserTokenId() == context->accessee.tokenId &&
        Crypto::GetUdidHash(accessee.GetAccesseeDeviceId()) == context->accesser.deviceIdHash &&
        Crypto::GetTokenIdHash(std::to_string(accessee.GetAccesseeTokenId())) == context->accesser.tokenIdHash);
}

bool AuthSinkNegotiateStateMachine::LnnAclCompare(std::shared_ptr<DmAuthContext> context,
    const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee)
{
    LOGI("start");
    return ((accessee.GetAccesseeDeviceId() == context->accessee.deviceId &&
        accessee.GetAccesseeUserId() == context->accessee.userId) ||
        (accesser.GetAccesserDeviceId() == context->accessee.deviceId &&
        accesser.GetAccesserUserId() == context->accessee.userId)) &&
        accessee.GetAccesseeTokenId() == 0 && accessee.GetAccesseeBundleName() == "" &&
        (Crypto::GetUdidHash(accesser.GetAccesserDeviceId()) == context->accesser.deviceIdHash ||
        Crypto::GetUdidHash(accessee.GetAccesseeDeviceId()) == context->accesser.deviceIdHash) &&
        accesser.GetAccesserTokenId() == 0 && accesser.GetAccesserBundleName() == "";
}

void AuthSinkNegotiateStateMachine::GetSinkCredentialInfo(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo)
{
    CHECK_NULL_VOID(context);
    // get identical credential
    if (context->accesser.accountIdHash == context->accessee.accountIdHash) {
        GetIdenticalCredentialInfo(context, credInfo);
        GetP2PCredentialInfo(context, credInfo);
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
    for (auto& item : credInfo.Items()) { // id1:json1, id2:json2, id3:json3
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

void AuthSinkNegotiateStateMachine::GetIdenticalCredentialInfo(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo)
{
    CHECK_NULL_VOID(context);
    JsonObject queryParams;
    queryParams[FILED_DEVICE_ID] = context->accessee.deviceId;
    queryParams[FILED_USER_ID] = MultipleUserConnector::GetOhosAccountNameByUserId(context->accessee.userId);
    queryParams[FILED_CRED_TYPE] = DM_AUTH_CREDENTIAL_ACCOUNT_RELATED;
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    if (context->hiChainAuthConnector->QueryCredentialInfo(context->accessee.userId, queryParams, credInfo) != DM_OK) {
        LOGE("QueryCredentialInfo failed credInfo %{public}s.", credInfo.Dump().c_str());
    }
}

void AuthSinkNegotiateStateMachine::GetShareCredentialInfo(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo)
{
    CHECK_NULL_VOID(context);
    JsonObject queryParams;
    queryParams[FILED_DEVICE_ID_HASH] = context->accesser.deviceIdHash;
    queryParams[FILED_PEER_USER_SPACE_ID] = std::to_string(context->accesser.userId);
    queryParams[FILED_CRED_TYPE] = DM_AUTH_CREDENTIAL_ACCOUNT_ACROSS;
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    if (context->hiChainAuthConnector->QueryCredentialInfo(context->accessee.userId, queryParams, credInfo) != DM_OK) {
        LOGE("QueryCredentialInfo failed credInfo %{public}s.", credInfo.Dump().c_str());
    }
}

void AuthSinkNegotiateStateMachine::GetP2PCredentialInfo(std::shared_ptr<DmAuthContext> context,
    JsonObject &credInfo)
{
    CHECK_NULL_VOID(context);
    JsonObject queryParams;
    queryParams[FILED_DEVICE_ID_HASH] = context->accesser.deviceIdHash;
    queryParams[FILED_PEER_USER_SPACE_ID] = std::to_string(context->accesser.userId);
    queryParams[FILED_CRED_TYPE] = DM_AUTH_CREDENTIAL_ACCOUNT_UNRELATED;
    queryParams[FILED_CRED_OWNER] = "DM";
    CHECK_NULL_VOID(context->hiChainAuthConnector);
    if (context->hiChainAuthConnector->QueryCredentialInfo(context->accessee.userId, queryParams, credInfo) != DM_OK) {
        LOGE("QueryCredentialInfo failed credInfo %{public}s.", credInfo.Dump().c_str());
    }
}

bool AuthSinkNegotiateStateMachine::IsAntiDisturbanceMode(const std::string &businessId)
{
    LOGI("AuthManager::IsAntiDisturbMode start.");
    if (businessId.empty()) {
        LOGE("AuthManager::IsAntiDisturbMode businessId is empty.");
        return false;
    }

    DistributedDeviceProfile::BusinessEvent event;
    event.SetBusinessKey(DM_DISTURBANCE_EVENT_KEY);
    int32_t ret = DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance().GetBusinessEvent(event);
    if (ret != DM_OK) {
        LOGE("GetBusinessEvent failed to get event, ret: %{public}d", ret);
        return false;
    }
    std::string businessValue = event.GetBusinessValue();
    if (businessValue.empty()) {
        LOGE("AuthManager::IsAntiDisturbMode failed: businessValue is empty.");
        return false;
    }
    return ParseAndCheckAntiDisturbanceMode(businessId, businessValue);
}

bool AuthSinkNegotiateStateMachine::ParseAndCheckAntiDisturbanceMode(const std::string &businessId,
    const std::string &businessValue)
{
    JsonObject jsonObject(businessValue);
    if (jsonObject.IsDiscarded()) {
        LOGE("AuthManager::IsAntiDisturbMode failed: invalid JSON format in businessValue.");
        return false;
    }
    if (!IsString(jsonObject, DM_BUSINESS_ID)) {
        LOGE("AuthManager::IsAntiDisturbMode failed: 'business_id' field is missing or invalid.");
        return false;
    }
    std::string parsedBusinessId = jsonObject[DM_BUSINESS_ID].Get<std::string>();
    if (parsedBusinessId != businessId) {
        LOGE("AuthManager::IsAntiDisturbMode failed: businessId mismatch. Expected: %{public}s, Found: %{public}s",
            businessId.c_str(), parsedBusinessId.c_str());
        return false;
    }
    if (!jsonObject.Contains(DM_ANTI_DISTURBANCE_MODE) || !jsonObject[DM_ANTI_DISTURBANCE_MODE].IsBoolean()) {
        LOGE("AuthManager::IsAntiDisturbMode failed: 'is_in_anti_disturbance_mode' field is missing or invalid.");
        return false;
    }
    bool isInAntiDisturbanceMode = jsonObject[DM_ANTI_DISTURBANCE_MODE].Get<bool>();
    LOGI("AuthManager::IsAntiDisturbMode result: %{public}s", isInAntiDisturbanceMode ? "true" : "false");

    return isInAntiDisturbanceMode;
}
} // namespace DistributedHardware
} // namespace OHOS