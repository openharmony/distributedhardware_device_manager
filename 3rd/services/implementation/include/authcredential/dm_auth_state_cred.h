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

#ifndef OHOS_DM_AUTH_STATE_CRED_H
#define OHOS_DM_AUTH_STATE_CRED_H

#include <memory>
#include <vector>

#include "json_object.h"
#include "dm_auth_context_cred.h"
#include "dm_auth_info_3rd.h"

namespace OHOS {
namespace DistributedHardware {
// State Types
enum class DmAuthStateType {
    CRED_AUTH_IDLE_STATE = 0,                      // When the device is initialized
    // source end state
    CRED_AUTH_SRC_START_STATE = 1,                 // User triggers auth
    CRED_AUTH_SRC_NEGOTIATE_STATE = 2,             // Receive softbus callback OnSessionOpened, send 3010 message
    CRED_SRC_CREDENTIAL_AUTH_START_STATE = 3,      // Start authentication and send 3030 message.
    CRED_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE = 4,  // Receive 3040 authentication PIN result message, send 3031 message
    CRED_SRC_CREDENTIAL_AUTH_DONE_STATE = 5,       // Receive 3041 authentication PIN result message, call processData
    CRED_AUTH_SRC_FINISH_STATE = 6,                // Receive 3060 message
    
    // sink end state
    CRED_AUTH_SINK_START_STATE = 51,                  // Bus trigger OnSessionOpened
    CRED_AUTH_SINK_NEGOTIATE_STATE = 52,              // Received 3010 trusted relationship negotiation message
    CRED_SINK_CREDENTIAL_AUTH_START_STATE = 53,       // Receive 3030 authentication PIN message, send 3040 message
    CRED_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE = 54,   // Received 3031 authentication PIN message, send 3041 message
    CRED_SINK_CREDENTIAL_AUTH_DONE_STATE = 55,
    CRED_AUTH_SINK_FINISH_STATE = 56,              // Received 3050 synchronization message, send 3060 message
};

enum DmAuthorizedScope : uint8_t {
    SCOPE_DEVICE = 1,
    SCOPE_USER,
    SCOPE_APP,
};

enum DmAuthCredentialAccountRelation : uint8_t {
    DM_AUTH_CREDENTIAL_INVALID = 0,             // Invalid
    DM_AUTH_CREDENTIAL_ACCOUNT_RELATED = 1,     // Account related
    DM_AUTH_CREDENTIAL_ACCOUNT_UNRELATED = 2,   // Account independent
    DM_AUTH_CREDENTIAL_ACCOUNT_ACROSS = 3,      // Share
};

class DmAuthStateCred {
public:
    virtual ~DmAuthStateCred() = default;
    virtual DmAuthStateType GetStateType() = 0;
    virtual int32_t Action(std::shared_ptr<DmAuthCredContext> context) = 0;
    void SourceFinish(std::shared_ptr<DmAuthCredContext> context);
    void SinkFinish(std::shared_ptr<DmAuthCredContext> context);
    static int32_t GetTaskTimeout(std::shared_ptr<DmAuthCredContext> context, const char* taskName,
        int32_t taskTimeOut);
    static void HandleAuthenticateTimeout(std::shared_ptr<DmAuthCredContext> context, const std::string &name);
    static uint64_t GetSysTimeMs();
    int32_t QueryCredential(std::shared_ptr<DmAuthCredContext> context);
    int32_t QueryP2pCredential(std::shared_ptr<DmAuthCredContext> context);
    void BuildTrustDeviceInfos(std::shared_ptr<DmAuthCredContext> context,
        std::vector<TrustDeviceInfo3rd> &deviceInfos);
    bool CheckOpenId(std::shared_ptr<DmAuthCredContext> context, const JsonItemObject &item);
};

class AuthCredSrcStartState : public DmAuthStateCred {
public:
    virtual ~AuthCredSrcStartState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};

class AuthSrcNegotiateStartState : public DmAuthStateCred {
public:
    virtual ~AuthSrcNegotiateStartState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};

class AuthSinkNegotiateStartState : public DmAuthStateCred {
public:
    virtual ~AuthSinkNegotiateStartState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
private:
    int32_t RespQueryAcceseeIds(std::shared_ptr<DmAuthCredContext> context);
};

class AuthSrcCredentialAuthStartState : public DmAuthStateCred {
public:
    virtual ~AuthSrcCredentialAuthStartState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};

class AuthSinkCredentialAuthStartState : public DmAuthStateCred {
public:
    virtual ~AuthSinkCredentialAuthStartState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};

class AuthSrcCredentialAuthMsgNegotiateState : public DmAuthStateCred {
public:
    virtual ~AuthSrcCredentialAuthMsgNegotiateState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};

class AuthSinkCredentialAuthMsgNegotiateState : public DmAuthStateCred {
public:
    virtual ~AuthSinkCredentialAuthMsgNegotiateState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};

class AuthSrcCredentialAuthDoneState : public DmAuthStateCred {
public:
    virtual ~AuthSrcCredentialAuthDoneState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};

class AuthSinkCredentialAuthDoneState : public DmAuthStateCred {
public:
    virtual ~AuthSinkCredentialAuthDoneState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};

class AuthCredSrcFinishState : public DmAuthStateCred {
public:
    virtual ~AuthCredSrcFinishState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};

class AuthCredSinkFinishState : public DmAuthStateCred {
public:
    virtual ~AuthCredSinkFinishState() = default;
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthCredContext> context) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_STATE_CRED_H
