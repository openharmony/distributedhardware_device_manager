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

#ifndef OHOS_DM_AUTH_PINCODE_STATE_3RD_H
#define OHOS_DM_AUTH_PINCODE_STATE_3RD_H

#include <memory>

#include "dm_auth_info_3rd.h"
#include "dm_auth_pincode_context_3rd.h"

namespace OHOS {
namespace DistributedHardware {
// State Types
enum class DmAuthPincodeStateType {
    // When the device is initialized
    ACL_AUTH_IDLE_STATE = 0,
    // User triggers BindTarget
    ACL_AUTH_PINCODE_SRC_START_STATE = 1,
    // Receive softbus callback OnSessionOpened, send 1010 message
    ACL_AUTH_PINCODE_SRC_NEGOTIATE_STATE = 2,
    // Start authentication and send 1030 message.
    ACL_AUTH_PINCODE_SRC_PIN_AUTH_START_STATE = 3,
    // Receive 1040 authentication PIN result message, send 1031 message
    ACL_AUTH_PINCODE_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE = 4,
    // Receive 1041 authentication PIN result message, call processData
    ACL_AUTH_PINCODE_SRC_PIN_AUTH_DONE_STATE = 5,
    // Received 1060 message
    ACL_AUTH_PINCODE_SRC_FINISH_STATE = 6,

    // Bus trigger OnSessionOpened
    ACL_AUTH_PINCODE_SINK_START_STATE = 51,
    // Received 1010 trusted relationship negotiation message
    ACL_AUTH_PINCODE_SINK_NEGOTIATE_STATE = 52,
    // Receive 1030 authentication PIN message, send 1040 message
    ACL_AUTH_PINCODE_SINK_PIN_AUTH_START_STATE = 53,
    // Received 1031 authentication PIN message, send 1041 message
    ACL_AUTH_PINCODE_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE = 54,
    // Received 1050 synchronization message, send 1060 message
    ACL_AUTH_PINCODE_SINK_FINISH_STATE = 55,
};

enum DmAuthorizedScope : uint8_t {
    SCOPE_DEVICE = 1,
    SCOPE_USER,
    SCOPE_APP,
};

class DmAuthPincodeState3rd {
public:
    virtual ~DmAuthPincodeState3rd() = default;
    virtual DmAuthPincodeStateType GetStateType() = 0;
    virtual int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) = 0;
    void SourceFinish(std::shared_ptr<DmAuthPincodeContext> context);
    void SinkFinish(std::shared_ptr<DmAuthPincodeContext> context);
    static int32_t GetTaskTimeout(std::shared_ptr<DmAuthPincodeContext> context, const char* taskName,
        int32_t taskTimeOut);
    static void HandleAuthenticateTimeout(std::shared_ptr<DmAuthPincodeContext> context, const std::string &name);
    void BuildResultContent(std::shared_ptr<DmAuthPincodeContext> context,
        std::vector<TrustDeviceInfo3rd> &deviceInfos);
};

class AuthPincodeSrcStartState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSrcStartState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSrcPinNegotiateStartState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSrcPinNegotiateStartState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSinkPinNegotiateStartState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSinkPinNegotiateStartState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
private:
    int32_t RespQueryAcceseeIds(std::shared_ptr<DmAuthPincodeContext> context);
};

class AuthPincodeSrcPinAuthStartState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSrcPinAuthStartState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSinkPinAuthStartState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSinkPinAuthStartState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSrcPinAuthMsgNegotiateState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSrcPinAuthMsgNegotiateState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSinkPinAuthMsgNegotiateState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSinkPinAuthMsgNegotiateState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSrcPinAuthDoneState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSrcPinAuthDoneState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSinkPinAuthDoneState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSinkPinAuthDoneState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSrcDataSyncState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSrcDataSyncState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSinkDataSyncState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSinkDataSyncState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSrcFinishState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSrcFinishState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};

class AuthPincodeSinkFinishState : public DmAuthPincodeState3rd {
public:
    virtual ~AuthPincodeSinkFinishState() {};
    DmAuthPincodeStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthPincodeContext> context) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_PINCODE_STATE_3RD_H
