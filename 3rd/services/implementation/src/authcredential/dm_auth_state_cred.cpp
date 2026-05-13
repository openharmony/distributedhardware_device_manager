/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <sys/time.h>
#include <cstring>
#include <securec.h>

#include "access_control_profile.h"
#include "dm_auth_state_cred.h"
#include "dm_auth_context_cred.h"
#include "dm_auth_info_3rd.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_message_processor_cred.h"
#include "dm_auth_state_machine_cred.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "deviceprofile_connector_3rd.h"
#include "hichain_auth_connector_3rd.h"
#include "multiple_user_connector_3rd.h"

namespace OHOS {
namespace DistributedHardware {
const char* const DM_FIELD_CRED_OWNER_ID = "ownerId";
const char* const DM_FIELD_OPEN_ID_HASH = "openIdHash";
const char* const DM_FIELD_DEVICE_ID = "deviceId";
const char* const DM_FIELD_USER_ID = "userId";
const char* const DM_FIELD_DEVICE_ID_HASH = "deviceIdHash";
const char* const DM_FIELD_PEER_USER_SPACE_ID = "peerUserSpaceId";
const char* const DM_FIELD_CRED_ID = "credId";
const char* const DM_FIELD_CRED_TYPE = "credType";
const char* const DM_FIELD_AUTHORIZED_APP_LIST = "authorizedAppList";
const char* const DM_FIELD_EXTEND_INFO_ID = "extendInfo";
const char* const DM_FIELD_OPEN_ID = "openId";
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;
// clone task timeout map
const std::map<std::string, int32_t> TASK_TIME_OUT_MAP = {
    { std::string(AUTHENTICATE_TIMEOUT_TASK), CLONE_AUTHENTICATE_TIMEOUT },
    { std::string(NEGOTIATE_TIMEOUT_TASK), CLONE_NEGOTIATE_TIMEOUT },
    { std::string(CONFIRM_TIMEOUT_TASK), CLONE_CONFIRM_TIMEOUT },
    { std::string(ADD_TIMEOUT_TASK), CLONE_ADD_TIMEOUT },
    { std::string(WAIT_NEGOTIATE_TIMEOUT_TASK), CLONE_WAIT_NEGOTIATE_TIMEOUT },
    { std::string(WAIT_REQUEST_TIMEOUT_TASK), CLONE_WAIT_REQUEST_TIMEOUT },
    { std::string(WAIT_PIN_AUTH_TIMEOUT_TASK), CLONE_PIN_AUTH_TIMEOUT },
    { std::string(SESSION_HEARTBEAT_TIMEOUT_TASK), CLONE_SESSION_HEARTBEAT_TIMEOUT }
};

constexpr uint16_t ONBINDRESULT_MAPPING_NUM = 2;
constexpr int32_t MS_PER_SECOND = 1000;
constexpr int32_t US_PER_MSECOND = 1000;
constexpr int32_t GET_SYSTEMTIME_MAX_NUM = 3;
constexpr int32_t PIN_CODE_COUNT_MAX_NUM = 10;

int32_t DmAuthStateCred::GetTaskTimeout(std::shared_ptr<DmAuthCredContext> context, const char* taskName,
    int32_t taskTimeOut)
{
    CHECK_NULL_RETURN(taskName, ERR_DM_POINT_NULL);
    LOGI("GetTaskTimeout, taskName: %{public}s", taskName);
    auto timeout = TASK_TIME_OUT_MAP.find(std::string(taskName));
    if (timeout != TASK_TIME_OUT_MAP.end()) {
        return timeout->second;
    }
    return taskTimeOut;
}

void DmAuthStateCred::HandleAuthenticateTimeout(std::shared_ptr<DmAuthCredContext> context, const std::string &name)
{
    LOGI("DmAuthCredContext::HandleAuthenticateTimeout start timer name %{public}s", name.c_str());
    CHECK_NULL_VOID(context);
    context->timer->DeleteTimer(name);
    context->reason = ERR_DM_TIME_OUT;
    context->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    LOGI("DmAuthCredContext::HandleAuthenticateTimeout complete");
}

void DmAuthStateCred::SourceFinish(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->listener);
    CHECK_NULL_VOID(context->timer);
    LOGI("SourceFinish reason:%{public}d, state:%{public}d", context->reason, context->state);
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    BuildTrustDeviceInfos(context, deviceInfos);
    context->listener->OnAuthResult(context->processInfo, context->reason, context->state, deviceInfos, "");
    context->timer->DeleteAll();
}

void DmAuthStateCred::SinkFinish(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->listener);
    CHECK_NULL_VOID(context->timer);
    CHECK_NULL_VOID(context->authMessageProcessor);
    LOGI("SinkFinish reason:%{public}d, state:%{public}d", context->reason, context->state);
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    BuildTrustDeviceInfos(context, deviceInfos);
    context->listener->OnAuthResult(context->processInfo, context->reason, context->state, deviceInfos, "");
    context->timer->DeleteAll();
    context->authMessageProcessor->CreateAndSendMsg(DmCredMessageType::CRED_RESP_FINISH, context);
}

uint64_t DmAuthStateCred::GetSysTimeMs()
{
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;
    int32_t retryNum = 0;
    while (retryNum < GET_SYSTEMTIME_MAX_NUM) {
        if (gettimeofday(&time, nullptr) != 0) {
            retryNum++;
            LOGE("GetSysTimeMs failed. retryNum: %{public}d", retryNum);
            continue;
        }
        return (uint64_t) time.tv_sec * MS_PER_SECOND + (uint64_t)time.tv_usec / US_PER_MSECOND;
    }
    return 0;
}

int32_t DmAuthStateCred::QueryCredential(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    if (context->accesser.credType == DM_AUTH_CREDENTIAL_ACCOUNT_UNRELATED) {
        return QueryP2pCredential(context);
    }
    DmCredAccess &selfAccess = (context->direction == DM_AUTH_CRED_SOURCE) ? context->accesser : context->accessee;
    std::string accountName = MultipleUserConnector3rd::GetOhosAccountNameByUserId(selfAccess.userId);
    if (accountName.empty()) {
        LOGE("accountName is empty");
        return ERR_DM_QUERY_CREDENTIAL_FAILED;
    }
    JsonObject credInfo;
    JsonObject queryParams;
    queryParams[DM_FIELD_DEVICE_ID] = selfAccess.deviceId;
    queryParams[DM_FIELD_USER_ID] = accountName;
    queryParams[DM_FIELD_CRED_TYPE] = DM_AUTH_CREDENTIAL_ACCOUNT_RELATED;
    int32_t ret = context->hiChainAuthConnector->QueryCredentialInfo(selfAccess.userId, queryParams, credInfo);
    if (ret != DM_OK) {
        LOGE("QueryCredentialInfo failed ret %{public}d.", ret);
        return ERR_DM_QUERY_CREDENTIAL_FAILED;
    }
    for (const auto &item : credInfo.Items()) {
        if (!item.Contains(DM_FIELD_CRED_ID) || !item[DM_FIELD_CRED_ID].IsString()) {
            continue;
        }
        if (!CheckOpenId(context, item)) {
            continue;
        }
        selfAccess.transmitCredentialId = item[DM_FIELD_CRED_ID].Get<std::string>();
        return DM_OK;
    }
    return ERR_DM_QUERY_CREDENTIAL_FAILED;
}

bool DmAuthStateCred::CheckOpenId(std::shared_ptr<DmAuthCredContext> context, const JsonItemObject &item)
{
    CHECK_NULL_RETURN(context, false);
    if (context->accesser.openIdHash.empty()) {
        return true;
    }
    if (!IsString(item, DM_FIELD_EXTEND_INFO_ID)) {
        LOGE("extendInfo is empty.");
        return false;
    }
    std::string extendInfo = item[DM_FIELD_EXTEND_INFO_ID].Get<std::string>();
    JsonObject extendInfoJson;
    extendInfoJson.Parse(extendInfo);
    if (extendInfoJson.IsDiscarded() || !IsString(extendInfoJson, DM_FIELD_OPEN_ID)) {
        LOGE("extendInfo invalid.");
        return false;
    }
    std::string openId = extendInfoJson[DM_FIELD_OPEN_ID].Get<std::string>();
    if (context->accesser.openIdHash == Crypto3rd::Sha256(openId).substr(0, DM_OPENID_HASH_LEN)) {
        return true;
    }
    return false;
}

int32_t DmAuthStateCred::QueryP2pCredential(std::shared_ptr<DmAuthCredContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->hiChainAuthConnector, ERR_DM_POINT_NULL);
    DmCredAccess &selfAccess = (context->direction == DM_AUTH_CRED_SOURCE) ? context->accesser : context->accessee;
    DmCredAccess &remoteAccess = (context->direction == DM_AUTH_CRED_SOURCE) ? context->accessee : context->accesser;
    JsonObject credInfo;
    JsonObject queryParams;
    queryParams[DM_FIELD_DEVICE_ID_HASH] = remoteAccess.deviceIdHash;
    queryParams[DM_FIELD_PEER_USER_SPACE_ID] = std::to_string(remoteAccess.userId);
    queryParams[DM_FIELD_CRED_TYPE] = DM_AUTH_CREDENTIAL_ACCOUNT_UNRELATED;
    queryParams[FIELD_CRED_OWNER] = "DM";
    int32_t ret = context->hiChainAuthConnector->QueryCredentialInfo(selfAccess.userId, queryParams, credInfo);
    if (ret != DM_OK) {
        LOGE("QueryCredentialInfo failed ret %{public}d.", ret);
        return ERR_DM_QUERY_CREDENTIAL_FAILED;
    }
    for (const auto &item : credInfo.Items()) {
        if (!item.Contains(DM_FIELD_CRED_ID) || !item[DM_FIELD_CRED_ID].IsString() ||
            !item.Contains(DM_FIELD_AUTHORIZED_APP_LIST)) {
            continue;
        }
        std::vector<std::string> appList;
        item[DM_FIELD_AUTHORIZED_APP_LIST].Get(appList);
        auto erIt = std::find(appList.begin(), appList.end(), std::to_string(selfAccess.tokenId));
        if (erIt != appList.end()) {
            selfAccess.transmitCredentialId = item[DM_FIELD_CRED_ID].Get<std::string>();
            return DM_OK;
        }
    }
    return ERR_DM_QUERY_CREDENTIAL_FAILED;
}

void DmAuthStateCred::BuildTrustDeviceInfos(std::shared_ptr<DmAuthCredContext> context,
    std::vector<TrustDeviceInfo3rd> &deviceInfos)
{
    CHECK_NULL_VOID(context);
    if (context->reason != DM_OK) {
        return ;
    }
    const DmCredAccess &selfAccess =
        (context->direction == DM_AUTH_CRED_SOURCE) ? context->accesser : context->accessee;
    const DmCredAccess &remoteAccess =
        (context->direction == DM_AUTH_CRED_SOURCE) ? context->accessee : context->accesser;
    TrustDeviceInfo3rd deviceInfo;
    deviceInfo.trustDeviceId = remoteAccess.deviceIdHash;
    deviceInfo.businessName = selfAccess.businessName;
    deviceInfo.userId = remoteAccess.userId;
    
    uint32_t keyLen = selfAccess.sessionKey.size();
    if (keyLen == 0 || keyLen > MAX_SESSION_KEY_LENGTH) {
        LOGE("SessionKey len invalid, len: %{public}d", keyLen);
        return;
    }
    deviceInfo.sessionKey.key = (uint8_t*)calloc(keyLen, sizeof(uint8_t));
    if (deviceInfo.sessionKey.key == nullptr) {
        LOGE("calloc failed.");
        return;
    }
    if (memcpy_s(deviceInfo.sessionKey.key, keyLen, selfAccess.sessionKey.data(), keyLen) != DM_OK) {
        LOGE("memcpy_s failed.");
        (void)memset_s(deviceInfo.sessionKey.key, keyLen, 0, keyLen);
        free(deviceInfo.sessionKey.key);
        deviceInfo.sessionKey.key = nullptr;
        deviceInfo.sessionKey.keyLen = 0;
        return;
    }
    deviceInfo.sessionKey.keyLen = keyLen;
    deviceInfos.push_back(deviceInfo);
}
} // namespace DistributedHardware
} // namespace OHOS