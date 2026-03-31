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

#ifndef OHOS_DM_AUTH_STATE_3RD_H
#define OHOS_DM_AUTH_STATE_3RD_H
#define TYPE_TV_ID 0x9C

#include <memory>

#include "dm_auth_context_3rd.h"
#include "dm_auth_info_3rd.h"

namespace OHOS {
namespace DistributedHardware {
// State Types
enum class DmAuthStateType {
    ACL_AUTH_IDLE_STATE = 0,                        // When the device is initialized
    // source end state
    ACL_AUTH_SRC_START_STATE = 1,                   // User triggers BindTarget
    ACL_AUTH_SRC_NEGOTIATE_STATE = 2,               // Receive softbus callback OnSessionOpened, send 2010 message
    ACL_AUTH_SRC_PIN_AUTH_START_STATE = 3,          // Start authentication and send 2030 message.
    ACL_AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE = 4,  // Receive 2040 authentication PIN result message, send 2031 message
    ACL_AUTH_SRC_PIN_AUTH_DONE_STATE = 5,          // Receive 2041 authentication PIN result message, call processData
    ACL_AUTH_SRC_DATA_SYNC_STATE = 6,               // Received 2060 message, sent 2070 message
    ACL_AUTH_SRC_FINISH_STATE = 7,                 // Received 2080 message
    
    // sink end state
    ACL_AUTH_SINK_START_STATE = 51,                  // Bus trigger OnSessionOpened
    ACL_AUTH_SINK_NEGOTIATE_STATE = 52,              // Received 2010 trusted relationship negotiation message
    ACL_AUTH_SINK_PIN_AUTH_START_STATE = 53,         // Receive 2030 authentication PIN message, send 2040 message
    ACL_AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE = 54, // Received 2031 authentication PIN message, send 2041 message
    ACL_AUTH_SINK_PIN_AUTH_DONE_STATE = 55,          // Trigger the Onfinish callback event
    ACL_AUTH_SINK_DATA_SYNC_STATE = 56,              // Received 2050 synchronization message, send 2060 message
    ACL_AUTH_SINK_FINISH_STATE = 57,                 // Received 2070 end message, send 2080 message
};

enum DmAuthorizedScope : uint8_t {
    SCOPE_DEVICE = 1,
    SCOPE_USER,
    SCOPE_APP,
};

class DmAuthState3rd {
public:
    virtual ~DmAuthState3rd() {};
    virtual DmAuthStateType GetStateType() = 0;
    virtual int32_t Action(std::shared_ptr<DmAuthContext> context) = 0;
    void SourceFinish(std::shared_ptr<DmAuthContext> context);
    void SinkFinish(std::shared_ptr<DmAuthContext> context);
    static int32_t GetTaskTimeout(std::shared_ptr<DmAuthContext> context, const char* taskName, int32_t taskTimeOut);
    static void HandleAuthenticateTimeout(std::shared_ptr<DmAuthContext> context, const std::string &name);
    void SaveAcl(std::shared_ptr<DmAuthContext> context);
    void SaveProxyAcl(std::shared_ptr<DmAuthContext> context);
    void SaveAclToDb(std::shared_ptr<DmAuthContext> context, const AccessControl3rd &accessControl3rd);
    static uint64_t GetSysTimeMs();
    std::string BuildResultContent(std::shared_ptr<DmAuthContext> context);
};

class AuthSrcStartState : public DmAuthState3rd {
public:
    virtual ~AuthSrcStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcPinNegotiateStartState : public DmAuthState3rd {
public:
    virtual ~AuthSrcPinNegotiateStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkPinNegotiateStartState : public DmAuthState3rd {
public:
    virtual ~AuthSinkPinNegotiateStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
private:
    int32_t RespQueryAcceseeIds(std::shared_ptr<DmAuthContext> context);
    int32_t RespQueryProxyAcceseeIds(std::shared_ptr<DmAuthContext> context);
};

class AuthSrcPinAuthStartState : public DmAuthState3rd {
public:
    virtual ~AuthSrcPinAuthStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkPinAuthStartState : public DmAuthState3rd {
public:
    virtual ~AuthSinkPinAuthStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcPinAuthMsgNegotiateState : public DmAuthState3rd {
public:
    virtual ~AuthSrcPinAuthMsgNegotiateState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkPinAuthMsgNegotiateState : public DmAuthState3rd {
public:
    virtual ~AuthSinkPinAuthMsgNegotiateState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcPinAuthDoneState : public DmAuthState3rd {
public:
    virtual ~AuthSrcPinAuthDoneState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
private:
    int32_t DerivativeSessionKey(std::shared_ptr<DmAuthContext> context);
    int32_t DerivativeProxySessionKey(std::shared_ptr<DmAuthContext> context);
};

class AuthSinkPinAuthDoneState : public DmAuthState3rd {
public:
    virtual ~AuthSinkPinAuthDoneState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
private:
    int32_t DerivativeSessionKey(std::shared_ptr<DmAuthContext> context);
    int32_t DerivativeProxySessionKey(std::shared_ptr<DmAuthContext> context);
};

class AuthSrcDataSyncState : public DmAuthState3rd {
public:
    virtual ~AuthSrcDataSyncState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkDataSyncState : public DmAuthState3rd {
public:
    virtual ~AuthSinkDataSyncState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcFinishState : public DmAuthState3rd {
public:
    virtual ~AuthSrcFinishState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkFinishState : public DmAuthState3rd {
public:
    virtual ~AuthSinkFinishState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_STATE_3RD_H
