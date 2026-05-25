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

#include "dm_auth_pincode_state_3rd.h"

#include <sys/time.h>
#include <cstring>
#include <securec.h>

#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_info_3rd.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_pincode_message_processor_3rd.h"
#include "dm_auth_pincode_state_machine_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"
#include "hichain_auth_connector_3rd.h"
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

constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;

int32_t DmAuthPincodeState3rd::GetTaskTimeout(std::shared_ptr<DmAuthPincodeContext> context, const char* taskName,
    int32_t taskTimeOut)
{
    LOGI("GetTaskTimeout, taskName: %{public}s", taskName);
    auto timeout = TASK_TIME_OUT_MAP.find(std::string(taskName));
    if (timeout != TASK_TIME_OUT_MAP.end()) {
        return timeout->second;
    }
    return taskTimeOut;
}

void DmAuthPincodeState3rd::HandleAuthenticateTimeout(std::shared_ptr<DmAuthPincodeContext> context,
    const std::string &name)
{
    LOGI("DmAuthPincodeContext::HandleAuthenticateTimeout start timer name %{public}s", name.c_str());
    context->timer->DeleteTimer(name);
    context->reason = ERR_DM_TIME_OUT;
    context->authPinStateMac3rd->NotifyEventFinish(DmEventType::ON_FAIL);
    LOGI("DmAuthPincodeContext::HandleAuthenticateTimeout complete");
}

void DmAuthPincodeState3rd::BuildResultContent(std::shared_ptr<DmAuthPincodeContext> context,
    std::vector<TrustDeviceInfo3rd> &deviceInfos)
{
    CHECK_NULL_VOID(context);
    if (context->reason != DM_OK) {
        LOGE("context->reason: %{public}d is not DM_OK", context->reason);
        return;
    }
    const DmPincodeAccess &selfAccess =
        (context->direction == DM_AUTH_PINCODE_SOURCE) ? context->accesser : context->accessee;
    const DmPincodeAccess &remoteAccess =
        (context->direction == DM_AUTH_PINCODE_SOURCE) ? context->accessee : context->accesser;
    TrustDeviceInfo3rd deviceInfo;
    deviceInfo.trustDeviceId = remoteAccess.deviceIdHash;
    deviceInfo.businessName = selfAccess.businessName;

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

void DmAuthPincodeState3rd::SourceFinish(std::shared_ptr<DmAuthPincodeContext> context)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->listener);
    CHECK_NULL_VOID(context->timer);
    LOGI("SourceFinish reason:%{public}d, state:%{public}d", context->reason, context->state);
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    BuildResultContent(context, deviceInfos);
    context->listener->OnAuthResult(context->processInfo, context->reason, context->state, deviceInfos, "");
    context->timer->DeleteAll();
}

void DmAuthPincodeState3rd::SinkFinish(std::shared_ptr<DmAuthPincodeContext> context)
{
    CHECK_NULL_VOID(context);
    CHECK_NULL_VOID(context->listener);
    CHECK_NULL_VOID(context->timer);
    CHECK_NULL_VOID(context->authPinMsgProc3rd);
    LOGI("SinkFinish reason:%{public}d, state:%{public}d", context->reason, context->state);
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    BuildResultContent(context, deviceInfos);
    context->listener->OnAuthResult(context->processInfo, context->reason, context->state, deviceInfos, "");
    context->timer->DeleteAll();
    context->authPinMsgProc3rd->CreateAndSendMsg(DmPincodeMessageType::AUTH_PINCODE_RESP_FINISH, context);
}
} // namespace DistributedHardware
} // namespace OHOS