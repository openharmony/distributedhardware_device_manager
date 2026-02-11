/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "softbus_session.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_dfx_constants.h"
#include "dm_hitrace.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "multiple_user_connector.h"
#include "json_object.h"
#include "softbus_connector.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<ISoftbusSessionCallback> SoftbusSession::sessionCallback_ = nullptr;
constexpr const char* DM_HITRACE_AUTH_TO_OPPEN_SESSION = "DM_HITRACE_AUTH_TO_OPPEN_SESSION";
constexpr int32_t MAX_DATA_LEN = 65535;

//LCOV_EXCL_START
SoftbusSession::SoftbusSession()
{
    LOGD("SoftbusSession constructor.");
}

SoftbusSession::~SoftbusSession()
{
    LOGD("SoftbusSession destructor.");
}

int32_t SoftbusSession::RegisterSessionCallback(std::shared_ptr<ISoftbusSessionCallback> callback)
{
    sessionCallback_ = callback;
    return DM_OK;
}

int32_t SoftbusSession::UnRegisterSessionCallback()
{
    sessionCallback_ = nullptr;
    return DM_OK;
}
//LCOV_EXCL_STOP

int32_t SoftbusSession::OpenAuthSession(const std::string &deviceId)
{
    DmTraceStart(std::string(DM_HITRACE_AUTH_TO_OPPEN_SESSION));
    int32_t sessionId = -1;
    std::string connectAddr;
    auto addrInfo = SoftbusConnector::GetConnectAddr(deviceId, connectAddr);
    if (addrInfo == nullptr) {
        LOGE("[SOFTBUS]addrInfo is nullptr. sessionId: %{public}d.", sessionId);
        return sessionId;
    }
    sessionId = ::OpenAuthSession(DM_SESSION_NAME, addrInfo.get(), 1, nullptr);
    if (sessionId < 0) {
        LOGE("[SOFTBUS]open session error, sessionId: %{public}d.", sessionId);
        return sessionId;
    }
    DmTraceEnd();
    LOGI("OpenAuthSession success. sessionId: %{public}d.", sessionId);
    return sessionId;
}

int32_t SoftbusSession::CloseAuthSession(int32_t sessionId)
{
    LOGI("CloseAuthSession.");
    ::CloseSession(sessionId);
    return DM_OK;
}

int32_t SoftbusSession::GetPeerDeviceId(int32_t sessionId, std::string &peerDevId)
{
    char peerDeviceId[DEVICE_UUID_LENGTH] = {0};
    int32_t ret = ::GetPeerDeviceId(sessionId, &peerDeviceId[0], DEVICE_UUID_LENGTH);
    if (ret == DM_OK) {
        peerDevId = peerDeviceId;
        LOGI("[SOFTBUS]GetPeerDeviceId success for session: %{public}d, peerDeviceId: %{public}s.", sessionId,
            GetAnonyString(peerDevId).c_str());
        return DM_OK;
    }
    LOGE("[SOFTBUS]GetPeerDeviceId failed for session: %{public}d, ret: %{public}d.", sessionId, ret);
    peerDevId = "";
    return ret;
}

int32_t SoftbusSession::SendData(int32_t sessionId, std::string &message)
{
    if (message.size() > MAX_DATA_LEN) {
        LOGE("SendData size is %{public}zu too long.", message.size());
        return ERR_DM_FAILED;
    }

    int32_t ret = SendBytes(sessionId, message.c_str(), strlen(message.c_str()));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed.");
        return ret;
    }
    return DM_OK;
}

int32_t SoftbusSession::SendHeartbeatData(int32_t sessionId, std::string &message)
{
    if (message.size() > MAX_DATA_LEN) {
        LOGE("SendHeartbeatData size is %{public}zu too long.", message.size());
        return ERR_DM_FAILED;
    }

    int32_t ret = SendBytes(sessionId, message.c_str(), strlen(message.c_str()));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendHeartbeatData failed.");
        return ret;
    }
    return DM_OK;
}

int SoftbusSession::OnSessionOpened(int sessionId, int result)
{
    LOGI("OnSessionOpened, success, sessionId: %{public}d, result: %{public}d.", sessionId, result);
    if (sessionCallback_ == nullptr) {
        LOGI("Session callback is not registered.");
        return DM_OK;
    }
    CHECK_NULL_RETURN(sessionCallback_, ERR_DM_POINT_NULL);
    int32_t sessionSide = GetSessionSide(sessionId);
    sessionCallback_->OnSessionOpened(sessionId, sessionSide, result);
    return DM_OK;
}

void SoftbusSession::OnSessionClosed(int sessionId)
{
    LOGI("OnSessionClosed, sessionId: %{public}d.", sessionId);
    CHECK_NULL_VOID(sessionCallback_);
    sessionCallback_->OnSessionClosed(sessionId);
    return;
}
} // namespace DistributedHardware
} // namespace OHOS
