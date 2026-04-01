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

#include "access_control_profile.h"
#include "dm_auth_state_3rd.h"
#include "dm_auth_context_3rd.h"
#include "dm_auth_info_3rd.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_message_processor_3rd.h"
#include "dm_auth_state_machine_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "hichain_auth_connector_3rd.h"
#include "kv_adapter_manager_3rd.h"
#include "multiple_user_connector_3rd.h"

namespace OHOS {
namespace DistributedHardware {
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
constexpr const static char* ONBINDRESULT_MAPPING_LIST[ONBINDRESULT_MAPPING_NUM] = {
    "CollaborationFwk",
    "cast_engine_service",
};

constexpr const static char* FLAG_WHITE_LIST[] = {
    "wear_link_service",
};
constexpr int32_t FLAG_WHITE_LIST_NUM = std::size(FLAG_WHITE_LIST);

int32_t DmAuthState3rd::GetTaskTimeout(std::shared_ptr<DmAuthContext> context, const char* taskName,
    int32_t taskTimeOut)
{
    LOGI("GetTaskTimeout, taskName: %{public}s", taskName);
    auto timeout = TASK_TIME_OUT_MAP.find(std::string(taskName));
    if (timeout != TASK_TIME_OUT_MAP.end()) {
        return timeout->second;
    }
    return taskTimeOut;
}

void DmAuthState3rd::HandleAuthenticateTimeout(std::shared_ptr<DmAuthContext> context, const std::string &name)
{
    LOGI("DmAuthContext::HandleAuthenticateTimeout start timer name %{public}s", name.c_str());
    context->timer->DeleteTimer(name);
    context->reason = ERR_DM_TIME_OUT;
    context->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    LOGI("DmAuthContext::HandleAuthenticateTimeout complete");
}

std::string DmAuthState3rd::BuildResultContent(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, "");
    const DmAccess &selfAccess = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    JsonObject contentObj;
    JsonObject skJsonArr(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    JsonObject skObj;
    skObj[TAG_BUSINESS_NAME] = selfAccess.businessName;
    std::string keyStr(selfAccess.sessionKey.begin(), selfAccess.sessionKey.end());
    skObj[TAG_SESSIONKEY] = keyStr;
    skJsonArr.PushBack(skObj);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        contentObj[TAG_SESSIONKEYS] = skJsonArr.Dump();
        return contentObj.Dump();
    }
    for (const auto &app : context->subjectProxyOnes) {
        const DmProxyAccess &selfProxyAccess =
            (context->direction == DM_AUTH_SOURCE) ? app.proxyAccesser : app.proxyAccessee;
        JsonObject proxySkObj;
        proxySkObj[TAG_BUSINESS_NAME] = selfProxyAccess.businessName;
        std::string proxyKeyStr(selfProxyAccess.sessionKey.begin(), selfProxyAccess.sessionKey.end());
        proxySkObj[TAG_SESSIONKEY] = proxyKeyStr;
        skJsonArr.PushBack(proxySkObj);
    }
    contentObj[TAG_SESSIONKEYS] = skJsonArr.Dump();
    return contentObj.Dump();
}

void DmAuthState3rd::SourceFinish(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->listener);
    CHECK_NULL_VOID(context->timer);
    LOGI("SourceFinish reason:%{public}d, state:%{public}d", context->reason, context->state);
    context->listener->OnAuthResult(context->processInfo, context->reason, context->state, BuildResultContent(context));
    context->timer->DeleteAll();
}

void DmAuthState3rd::SinkFinish(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->listener);
    CHECK_NULL_VOID(context->timer);
    CHECK_NULL_VOID(context->authMessageProcessor);
    LOGI("SinkFinish reason:%{public}d, state:%{public}d", context->reason, context->state);
    context->listener->OnAuthResult(context->processInfo, context->reason, context->state,
        BuildResultContent(context));
    context->timer->DeleteAll();
    context->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_RESP_FINISH, context);
}

void DmAuthState3rd::SaveAcl(std::shared_ptr<DmAuthContext> context)
{
    LOGI("start");
    CHECK_NULL_VOID(context);
    DmAccess &selfAccess = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    DmAccess &remoteAccess = (context->direction == DM_AUTH_SOURCE) ? context->accessee : context->accesser;
    AccessControl3rd accessControl3rd;
    accessControl3rd.trustDeviceId = remoteAccess.deviceId;
    accessControl3rd.sessionKeyId = selfAccess.transmitSessionKeyId;
    accessControl3rd.createTime = static_cast<int64_t>(GetSysTimeMs());
    accessControl3rd.extra = "";
    accessControl3rd.accesser.deviceId = context->accesser.deviceId;
    accessControl3rd.accesser.userId = context->accesser.userId;
    accessControl3rd.accesser.accountId = context->accesser.accountId;
    accessControl3rd.accesser.tokenId = context->accesser.tokenId;
    accessControl3rd.accesser.processName = context->accesser.processName;
    accessControl3rd.accesser.uid = context->accesser.uid;
    accessControl3rd.accesser.businessName = context->accesser.businessName;
    accessControl3rd.accesser.version = context->accesser.dmVersion;

    accessControl3rd.accessee.deviceId = context->accessee.deviceId;
    accessControl3rd.accessee.userId = context->accessee.userId;
    accessControl3rd.accessee.accountId = context->accessee.accountId;
    accessControl3rd.accessee.tokenId = context->accessee.tokenId;
    accessControl3rd.accessee.processName = context->accessee.processName;
    accessControl3rd.accessee.uid = context->accessee.uid;
    accessControl3rd.accessee.businessName = context->accessee.businessName;
    accessControl3rd.accessee.version = context->accessee.dmVersion;
    SaveAclToDb(context, accessControl3rd);
    SaveProxyAcl(context);
}

void DmAuthState3rd::SaveProxyAcl(std::shared_ptr<DmAuthContext> context)
{
    LOGI("start");
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    DmAccess &remoteAccess = (context->direction == DM_AUTH_SOURCE) ? context->accessee : context->accesser;
    DmAccess &selfAccess = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    for (auto &app : context->subjectProxyOnes) {
        DmProxyAccess &selfProxyAccess = (context->direction == DM_AUTH_SOURCE) ? app.proxyAccesser : app.proxyAccessee;
        AccessControl3rd accessControl3rd;
        accessControl3rd.trustDeviceId = remoteAccess.deviceId;
        accessControl3rd.sessionKeyId = selfProxyAccess.transmitSessionKeyId;
        accessControl3rd.createTime = static_cast<int64_t>(GetSysTimeMs());
        JsonObject proxyObj;
        proxyObj[TAG_PROXY_TOKEN_ID] = selfAccess.tokenId;
        proxyObj[TAG_PROXY_PROCESS_NAME] = selfAccess.processName;
        proxyObj[TAG_PROXY_UID] = selfAccess.uid;
        accessControl3rd.extra = proxyObj.Dump();

        accessControl3rd.accesser.deviceId = context->accesser.deviceId;
        accessControl3rd.accesser.userId = context->accesser.userId;
        accessControl3rd.accesser.accountId = context->accesser.accountId;
        accessControl3rd.accesser.version = context->accesser.dmVersion;
        accessControl3rd.accesser.tokenId = app.proxyAccesser.tokenId;
        accessControl3rd.accesser.processName = app.proxyAccesser.processName;
        accessControl3rd.accesser.uid = app.proxyAccesser.uid;
        accessControl3rd.accesser.businessName = app.proxyAccesser.businessName;

        accessControl3rd.accessee.deviceId = context->accessee.deviceId;
        accessControl3rd.accessee.userId = context->accessee.userId;
        accessControl3rd.accessee.accountId = context->accessee.accountId;
        accessControl3rd.accessee.version = context->accessee.dmVersion;
        accessControl3rd.accessee.tokenId = app.proxyAccessee.tokenId;
        accessControl3rd.accessee.processName = app.proxyAccessee.processName;
        accessControl3rd.accessee.uid = app.proxyAccessee.uid;
        accessControl3rd.accessee.businessName = app.proxyAccessee.businessName;
        SaveAclToDb(context, accessControl3rd);
    }
}

void DmAuthState3rd::SaveAclToDb(std::shared_ptr<DmAuthContext> context, const AccessControl3rd &accessControl3rd)
{
    CHECK_NULL_VOID(context);
    std::string key = ACL_PREFIX + accessControl3rd.accesser.deviceId +
        std::to_string(accessControl3rd.accesser.userId) +
        std::to_string(accessControl3rd.accesser.tokenId) + accessControl3rd.accesser.processName +
        std::to_string(accessControl3rd.accesser.uid) +
        accessControl3rd.accesser.businessName + accessControl3rd.accessee.deviceId +
        std::to_string(accessControl3rd.accessee.userId) + accessControl3rd.accessee.processName +
        accessControl3rd.accessee.businessName;
    std::string valueStr = "";
    int32_t ret = KVAdapterManager3rd::GetInstance().Get(key, valueStr);
    if (ret == DM_OK && !valueStr.empty()) {
        JsonObject json(valueStr);
        if (!json.IsDiscarded() && context->authMessageProcessor != nullptr) {
            AccessControl3rd accessControlTemp = json.Get<AccessControl3rd>();
            int32_t userId = context->direction == DM_AUTH_SOURCE ? accessControlTemp.accesser.userId :
                accessControlTemp.accessee.userId;
            context->authMessageProcessor->DeleteSessionKeyToDP(userId, accessControlTemp.sessionKeyId);
        }
    }
    JsonObject aclJsonObj{};
    aclJsonObj = accessControl3rd;
    std::string aclStr = aclJsonObj.Dump();
    ret = KVAdapterManager3rd::GetInstance().PutByKey(key, aclStr);
    LOGI("result: %{public}d", ret);
}

uint64_t DmAuthState3rd::GetSysTimeMs()
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
} // namespace DistributedHardware
} // namespace OHOS