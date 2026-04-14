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

#ifndef OHOS_DM_AUTH_ADAPTER_3RD_H
#define OHOS_DM_AUTH_ADAPTER_3RD_H

#include <functional>
#include <string>
#include <memory>
#include <map>
#include <cstdint>

#include "device_manager_data_struct_3rd.h"
#include "hichain_connector_callback_3rd.h"
#include "softbus_session_callback_3rd.h"

namespace OHOS {
namespace DistributedHardware {
extern const char* TAG_DM_VERSION;
extern const char* TAG_DM_VERSION_DJI;           // compatible for new and old protocol

extern const char* AUTHENTICATE_TIMEOUT_TASK;
extern const char* NEGOTIATE_TIMEOUT_TASK;
extern const char* CONFIRM_TIMEOUT_TASK;
extern const char* ADD_TIMEOUT_TASK;
extern const char* WAIT_NEGOTIATE_TIMEOUT_TASK;
extern const char* WAIT_REQUEST_TIMEOUT_TASK;
extern const char* AUTH_DEVICE_TIMEOUT_TASK;
extern const char* WAIT_PIN_AUTH_TIMEOUT_TASK;
extern const char* SESSION_HEARTBEAT_TIMEOUT_TASK;

extern const char* WAIT_SESSION_CLOSE_TIMEOUT_TASK;

extern const int32_t CLONE_AUTHENTICATE_TIMEOUT;
extern const int32_t CLONE_NEGOTIATE_TIMEOUT;
extern const int32_t CLONE_CONFIRM_TIMEOUT;
extern const int32_t CLONE_ADD_TIMEOUT;
extern const int32_t CLONE_WAIT_NEGOTIATE_TIMEOUT;
extern const int32_t CLONE_WAIT_REQUEST_TIMEOUT;
extern const int32_t CLONE_PIN_AUTH_TIMEOUT;
extern const int32_t CLONE_SESSION_HEARTBEAT_TIMEOUT;

extern const int32_t AUTHENTICATE_TIMEOUT;
extern const int32_t PIN_AUTH_TIMEOUT;
extern const int32_t NEGOTIATE_TIMEOUT;
extern const int32_t WAIT_REQUEST_TIMEOUT;
extern const int32_t EVENT_TIMEOUT;

using CleanNotifyCallback = std::function<void(uint64_t, int32_t)>;
using StopTimerAndDelDpCallback = std::function<void(const std::string&, int32_t, uint64_t)>;
enum DmRole {
    DM_ROLE_UNKNOWN = 0,
    DM_ROLE_USER = 1,
    DM_ROLE_SA,
    DM_ROLE_FA,
};

class AuthManagerBase3rd : public ISoftbusSessionCallback3rd,
                        public IDmDeviceAuthCallback3rd {
public:
    virtual int32_t AuthDevice3rd(const PeerTargetId3rd &targetId, const std::map<std::string, std::string> &authParam,
        int32_t sessionId, uint64_t logicalSessionId);

    virtual void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result);

    virtual void OnSessionClosed(const int32_t sessionId);

    virtual void OnSessionDisable();

    virtual void OnDataReceived(const int32_t sessionId, const std::string &message);

    virtual void OnAuthDeviceDataReceived(int32_t sessionId, std::string message);

    virtual int32_t EstablishAuthChannel(const std::string &deviceId);

    virtual void StartNegotiate(const int32_t sessionId);

    virtual void RespNegotiate(const int32_t sessionId);

    virtual void SendAuthRequest(const int32_t sessionId);

    virtual int32_t StartAuthProcess(const int32_t action);

    virtual void StartRespAuthProcess();

    virtual int32_t GetPinCode(std::string &code);

    virtual void HandleAuthenticateTimeout(std::string name);

    virtual int32_t SetReasonAndFinish(int32_t reason, int32_t state);

    virtual int32_t ImportAuthCodeAndUid(const std::string &pkgName, const std::string &authCode, uint32_t uid);
    virtual void RegisterCleanNotifyCallback(CleanNotifyCallback cleanNotifyCallback);
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_ADAPTER_3RD_H
