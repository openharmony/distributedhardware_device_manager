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

#ifndef OHOS_DM_AUTH_STATE_V2_H
#define OHOS_DM_AUTH_STATE_V2_H
#define TYPE_TV_ID 0x9C

#include <memory>

#include "access_control_profile.h"
#include "dm_auth_context.h"

namespace OHOS {
namespace DistributedHardware {
const char* const FILED_CRED_OWNER = "credOwner";
const char* const FILED_DEVICE_ID = "deviceId";
const char* const FILED_USER_ID = "userId";
const char* const FILED_DEVICE_ID_HASH = "deviceIdHash";
const char* const FILED_PEER_USER_SPACE_ID = "peerUserSpaceId";
const char* const FILED_CRED_ID = "credId";
const char* const FILED_CRED_TYPE = "credType";
const char* const FILED_AUTHORIZED_SCOPE = "authorizedScope";
const char* const FILED_AUTHORIZED_APP_LIST = "authorizedAppList";
const char* const FILED_SUBJECT = "subject";

// State Types
enum class DmAuthStateType {
    AUTH_IDLE_STATE = 0,    // When the device is initialized
    // source end state
    AUTH_SRC_START_STATE = 1,                   // User triggers BindTarget
    AUTH_SRC_NEGOTIATE_STATE = 2,               // Receive softbus callback OnSessionOpened, send 80 message
    AUTH_SRC_CONFIRM_STATE = 3,                 // Receive 90 authorization result message, send 100 message
    AUTH_SRC_PIN_NEGOTIATE_START_STATE = 4,     // Start negotiating PIN code, receive 110 authorization message
                                                // or rollback or 90 jump
    AUTH_SRC_PIN_INPUT_STATE = 5,               // Enter PIN
    AUTH_SRC_REVERSE_ULTRASONIC_START_STATE = 6,
    AUTH_SRC_REVERSE_ULTRASONIC_DONE_STATE = 7,
    AUTH_SRC_FORWARD_ULTRASONIC_START_STATE = 8,
    AUTH_SRC_FORWARD_ULTRASONIC_DONE_STATE = 9,
    AUTH_SRC_PIN_AUTH_START_STATE = 10,          // Start authentication and send 120 message.
    AUTH_SRC_PIN_AUTH_MSG_NEGOTIATE_STATE = 11,  // Receive 130 authentication PIN result message, send 121 message
    AUTH_SRC_PIN_AUTH_DONE_STATE = 12,           // Receive 131 authentication PIN result message, call processData
    AUTH_SRC_CREDENTIAL_EXCHANGE_STATE = 13,    // Trigger the Onfinish callback event and send a 140 message
    AUTH_SRC_CREDENTIAL_AUTH_START_STATE = 14,   // Received 150 encrypted messages, sent 160 messages
    AUTH_SRC_CREDENTIAL_AUTH_NEGOTIATE_STATE = 15,   // 收到170凭据认证报文，发送161报文
    AUTH_SRC_CREDENTIAL_AUTH_DONE_STATE = 16,   // Received 170 credential authentication message, sent 161 message
    AUTH_SRC_DATA_SYNC_STATE = 17,              // Received 190 message, sent 200 message
    AUTH_SRC_FINISH_STATE = 18,                 // Received 201 message

    // sink end state
    AUTH_SINK_START_STATE = 50,                 // Bus trigger OnSessionOpened
    AUTH_SINK_NEGOTIATE_STATE = 51,             // Received 80 trusted relationship negotiation message, send 90 message
    AUTH_SINK_CONFIRM_STATE = 52,               // Received 100 user authorization messages, sent 110 messages
    AUTH_SINK_PIN_NEGOTIATE_START_STATE = 53,   // Start negotiating PIN code, active migration or error rollback
    AUTH_SINK_PIN_DISPLAY_STATE = 54,           // Generate and display PIN
    AUTH_SINK_REVERSE_ULTRASONIC_START_STATE = 55,
    AUTH_SINK_REVERSE_ULTRASONIC_DONE_STATE = 56,
    AUTH_SINK_FORWARD_ULTRASONIC_START_STATE = 57,
    AUTH_SINK_FORWARD_ULTRASONIC_DONE_STATE = 58,
    AUTH_SINK_PIN_AUTH_START_STATE = 59,        // Receive 120 authentication PIN message, send 130 message
    AUTH_SINK_PIN_AUTH_MSG_NEGOTIATE_STATE = 60, // Received 121 authentication PIN message, send 131 message
    AUTH_SINK_PIN_AUTH_DONE_STATE = 61,         // Trigger the Onfinish callback event
    AUTH_SINK_CREDENTIAL_EXCHANGE_STATE = 62,   // Received 140 encrypted messages, sent 150 messages
    AUTH_SINK_CREDENTIAL_AUTH_START_STATE = 63, // Receive 160 authentication message, send 170 message
    AUTH_SINK_CREDENTIAL_AUTH_NEGOTIATE_STATE = 64, // Received 161 credential negotiation message,
    AUTH_SINK_DATA_SYNC_STATE = 65,             // Received 180 synchronization message, send 190 message
    AUTH_SINK_FINISH_STATE = 66,                // Received 200 end message, send 201 message
};

// Credential Addition Method
enum DmAuthCredentialAddMethod : uint8_t {
    DM_AUTH_CREDENTIAL_ADD_METHOD_GENERATE = 1, // Generate
    DM_AUTH_CREDENTIAL_ADD_METHOD_IMPORT,       // Import
};

// Credential Subject
enum DmAuthCredentialSubject : uint8_t {
    DM_AUTH_CREDENTIAL_SUBJECT_PRIMARY = 1,     // Main Control
    DM_AUTH_CREDENTIAL_SUBJECT_SUPPLEMENT,      // Accessories
};

// Credentials and account association
enum DmAuthCredentialAccountRelation : uint8_t {
    DM_AUTH_CREDENTIAL_INVALID = 0,             // Invalid
    DM_AUTH_CREDENTIAL_ACCOUNT_RELATED = 1,     // Account related
    DM_AUTH_CREDENTIAL_ACCOUNT_UNRELATED = 2,   // Account independent
    DM_AUTH_CREDENTIAL_ACCOUNT_ACROSS = 3,      // Share
};

// Key Type
enum DmAuthKeyFormat : uint8_t {
    DM_AUTH_KEY_FORMAT_SYMM_IMPORT = 1,  // Symmetric key (supported only for import)
    DM_AUTH_KEY_FORMAT_ASYMM_IMPORT,    // Asymmetric key public key (supported only for import)
    DM_AUTH_KEY_FORMAT_ASYMM_GENERATE,  // Asymmetric key (supported only for generation)
    DM_AUTH_KEY_FORMAT_X509,            // X509 certificate
};

// Algorithm type
enum DmAuthAlgorithmType : uint8_t {
    DM_AUTH_ALG_TYPE_AES256 = 1,                // AES256
    DM_AUTH_ALG_TYPE_AES128,                    // AES128
    DM_AUTH_ALG_TYPE_P256,                      // P256
    DM_AUTH_ALG_TYPE_ED25519                    // ED25519
};

// Credential proof type
enum DmAuthCredentialProofType : uint8_t {
    DM_AUTH_CREDENTIAL_PROOF_PSK = 1,           // PSK
    DM_AUTH_CREDENTIAL_PROOF_PKI,               // PKI
};

enum DmCredType : uint8_t {
    ACCOUNT_RELATED = 1,
    ACCOUNT_UNRELATED,
    ACCOUNT_ACROSS
};

enum DmAuthorizedScope : uint8_t {
    SCOPE_DEVICE = 1,
    SCOPE_USER,
    SCOPE_APP,
};

enum DM_SUBJECT : uint8_t {
    SUBJECT_PRIMARY = 1,
    SUBJECT_SECONDARY,
};

class DmAuthState {
public:
    virtual ~DmAuthState() {};
    virtual DmAuthStateType GetStateType() = 0;
    virtual int32_t Action(std::shared_ptr<DmAuthContext> context) = 0;
    void SourceFinish(std::shared_ptr<DmAuthContext> context);
    void SinkFinish(std::shared_ptr<DmAuthContext> context);
    std::string GenerateBindResultContent(std::shared_ptr<DmAuthContext> context);
    static bool IsScreenLocked();
    static int32_t GetTaskTimeout(std::shared_ptr<DmAuthContext> context, const char* taskName, int32_t taskTimeOut);
    static void HandleAuthenticateTimeout(std::shared_ptr<DmAuthContext> context, std::string name);
    static bool IsImportAuthCodeCompatibility(DmAuthType authType);

    void SetAclExtraInfo(std::shared_ptr<DmAuthContext> context);
    void SetAclInfo(std::shared_ptr<DmAuthContext> context);
    int32_t GetAclBindType(std::shared_ptr<DmAuthContext> context, std::string credId);
    static bool CheckProcessNameInWhiteList(const std::string &processName);
    int32_t GetOutputState(const std::string &processName, int32_t state);
    int32_t GetOutputReplay(const std::string &processName, int32_t replay);
    static uint64_t GetSysTimeMs();
    static void DeleteAcl(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::AccessControlProfile &profile);
protected:
    bool NeedReqUserConfirm(std::shared_ptr<DmAuthContext> context);
    bool NeedAgreeCredential(std::shared_ptr<DmAuthContext> context);
    bool NeedAgreeAcl(std::shared_ptr<DmAuthContext> context);
    uint32_t GetCredType(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credInfo);
    uint32_t GetCredentialType(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credInfo);
    bool HaveSameTokenId(std::shared_ptr<DmAuthContext> context, const std::vector<std::string> &tokenList);
    void SetProcessInfo(std::shared_ptr<DmAuthContext> context);
};

class AuthSrcConfirmState : public DmAuthState {
public:
    virtual ~AuthSrcConfirmState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
private:
    void NegotiateCredential(std::shared_ptr<DmAuthContext> context, JsonObject &credTypeNegoRsult);
    void NegotiateAcl(std::shared_ptr<DmAuthContext> context, JsonObject &aclNegoRsult);
    void GetSrcCredentialInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo);
    void GetIdenticalCredentialInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo);
    void GetShareCredentialInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo);
    void GetP2PCredentialInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo);
    void GetSrcAclInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo, JsonObject &aclInfo);
    void GetSrcAclInfoForP2P(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, JsonObject &aclInfo);
    bool IdenticalAccountAclCompare(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee);
    bool ShareAclCompare(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee);
    bool Point2PointAclCompare(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee);
    bool LnnAclCompare(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee);
    bool CheckCredIdInAcl(std::shared_ptr<DmAuthContext> context,
            const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, uint32_t bindType);
    void CheckCredIdInAclForP2P(std::shared_ptr<DmAuthContext> context, std::string &credId,
        const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, uint32_t bindType,
        bool &checkResult);
    void GetSrcCredType(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo, JsonObject &aclInfo,
        JsonObject &credTypeJson);
    void GetSrcCredTypeForP2P(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credObj,
        JsonObject &aclInfo, JsonObject &credTypeJson, int32_t credType, std::vector<std::string> &deleteCredInfo);
};

class AuthSinkStatePinAuthComm {
public:
    static bool IsPinCodeValid(int32_t numpin);
    static bool IsPinCodeValid(const std::string& strpin);
    static bool IsAuthCodeReady(std::shared_ptr<DmAuthContext> context);
    static void GeneratePincode(std::shared_ptr<DmAuthContext> context);
    static int32_t ShowAuthInfoDialog(std::shared_ptr<DmAuthContext> context);
private:
    static void HandleSessionHeartbeat(std::shared_ptr<DmAuthContext> context, std::string name);
};

class AuthSinkConfirmState : public DmAuthState {
public:
    virtual ~AuthSinkConfirmState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
private:
    void NegotiateCredential(std::shared_ptr<DmAuthContext> context, JsonObject &credTypeNegoRsult);
    void NegotiateAcl(std::shared_ptr<DmAuthContext> context, JsonObject &aclNegoRsult);
    int32_t ShowConfigDialog(std::shared_ptr<DmAuthContext> context);
    int32_t EndDream();
    void ReadServiceInfo(std::shared_ptr<DmAuthContext> context);
    void MatchFallBackCandidateList(std::shared_ptr<DmAuthContext> context, DmAuthType authType);
    int32_t ProcessBindAuthorize(std::shared_ptr<DmAuthContext> context);
    int32_t ProcessNoBindAuthorize(std::shared_ptr<DmAuthContext> context);
    std::string GetCredIdByCredType(std::shared_ptr<DmAuthContext> context, int32_t credType);
};

class AuthSrcPinNegotiateStartState : public DmAuthState {
public:
    virtual ~AuthSrcPinNegotiateStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
private:
    int32_t NegotiatePinAuth(std::shared_ptr<DmAuthContext> context, bool firstTime);
    int32_t ProcessCredAuth(std::shared_ptr<DmAuthContext> context);
    int32_t ProcessPinBind(std::shared_ptr<DmAuthContext> context);
    std::string GetCredIdByCredType(std::shared_ptr<DmAuthContext> context, int32_t credType);
};

class AuthSrcPinInputState : public DmAuthState {
public:
    virtual ~AuthSrcPinInputState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
private:
    int32_t ShowStartAuthDialog(std::shared_ptr<DmAuthContext> context);
};

class AuthSinkPinNegotiateStartState : public DmAuthState {
public:
    virtual ~AuthSinkPinNegotiateStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkPinDisplayState : public DmAuthState {
public:
    virtual ~AuthSinkPinDisplayState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcReverseUltrasonicStartState : public DmAuthState {
public:
    virtual ~AuthSrcReverseUltrasonicStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcReverseUltrasonicDoneState : public DmAuthState {
public:
    virtual ~AuthSrcReverseUltrasonicDoneState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcForwardUltrasonicStartState : public DmAuthState {
public:
    virtual ~AuthSrcForwardUltrasonicStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcForwardUltrasonicDoneState : public DmAuthState {
public:
    virtual ~AuthSrcForwardUltrasonicDoneState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkReverseUltrasonicStartState : public DmAuthState {
public:
    virtual ~AuthSinkReverseUltrasonicStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkReverseUltrasonicDoneState : public DmAuthState {
public:
    virtual ~AuthSinkReverseUltrasonicDoneState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkForwardUltrasonicStartState : public DmAuthState {
public:
    virtual ~AuthSinkForwardUltrasonicStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkForwardUltrasonicDoneState : public DmAuthState {
public:
    virtual ~AuthSinkForwardUltrasonicDoneState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcPinAuthStartState : public DmAuthState {
public:
    virtual ~AuthSrcPinAuthStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
private:
    int32_t ShowStartAuthDialog(std::shared_ptr<DmAuthContext> context); // Display PIN input box to user
};

class AuthSinkPinAuthStartState : public DmAuthState {
public:
    virtual ~AuthSinkPinAuthStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcPinAuthMsgNegotiateState : public DmAuthState {
public:
    virtual ~AuthSrcPinAuthMsgNegotiateState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkPinAuthMsgNegotiateState : public DmAuthState {
public:
    virtual ~AuthSinkPinAuthMsgNegotiateState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkPinAuthDoneState : public DmAuthState {
public:
    virtual ~AuthSinkPinAuthDoneState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcPinAuthDoneState : public DmAuthState {
public:
    virtual ~AuthSrcPinAuthDoneState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcStartState : public DmAuthState {
public:
    virtual ~AuthSrcStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcNegotiateStateMachine : public DmAuthState {
public:
    virtual ~AuthSrcNegotiateStateMachine() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
private:
    std::string GetAccountGroupIdHash(std::shared_ptr<DmAuthContext> context);
};

// Middle class encapsulates public interfaces related to business.
class AuthCredentialAgreeState : public DmAuthState {
public:
    virtual ~AuthCredentialAgreeState() {};
protected:
    std::string CreateAuthParamsString(DmAuthScope authorizedScope, DmAuthCredentialAddMethod method,
        const std::shared_ptr<DmAuthContext> &authContext);
    int32_t GenerateCredIdAndPublicKey(DmAuthScope authorizedScope, std::shared_ptr<DmAuthContext> &authContext);
    int32_t AgreeCredential(DmAuthScope authorizedScope, std::shared_ptr<DmAuthContext> &authContext);
};

class AuthSrcCredentialExchangeState : public AuthCredentialAgreeState {
public:
    virtual ~AuthSrcCredentialExchangeState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkCredentialExchangeState : public AuthCredentialAgreeState {
public:
    virtual ~AuthSinkCredentialExchangeState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcCredentialAuthStartState : public AuthCredentialAgreeState {
public:
    virtual ~AuthSrcCredentialAuthStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcCredentialAuthNegotiateState : public DmAuthState {
public:
    virtual ~AuthSrcCredentialAuthNegotiateState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcCredentialAuthDoneState : public DmAuthState {
public:
    virtual ~AuthSrcCredentialAuthDoneState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkCredentialAuthStartState : public DmAuthState {
public:
    virtual ~AuthSinkCredentialAuthStartState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkCredentialAuthNegotiateState : public DmAuthState {
public:
    virtual ~AuthSinkCredentialAuthNegotiateState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkNegotiateStateMachine : public DmAuthState {
public:
    virtual ~AuthSinkNegotiateStateMachine() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;

private:
    int32_t RespQueryAcceseeIds(std::shared_ptr<DmAuthContext> context);
    int32_t ProcRespNegotiate5_1_0(std::shared_ptr<DmAuthContext> context);
    void GetSinkCredentialInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo);
    void GetIdenticalCredentialInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo);
    void GetShareCredentialInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo);
    void GetP2PCredentialInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo);
    void GetSinkAclInfo(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo, JsonObject &aclInfo);
    void GetSinkAclInfoForP2P(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, JsonObject &aclInfo);
    bool IdenticalAccountAclCompare(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee);
    bool ShareAclCompare(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee);
    bool Point2PointAclCompare(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee);
    bool LnnAclCompare(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::Accesser &accesser, const DistributedDeviceProfile::Accessee &accessee);
    bool CheckCredIdInAcl(std::shared_ptr<DmAuthContext> context,
        const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, uint32_t bindType);
    void CheckCredIdInAclForP2P(std::shared_ptr<DmAuthContext> context, std::string &credId,
        const DistributedDeviceProfile::AccessControlProfile &profile, JsonObject &credInfo, uint32_t bindType,
        bool &checkResult);
    void GetSinkCredType(std::shared_ptr<DmAuthContext> context, JsonObject &credInfo, JsonObject &aclInfo,
        JsonObject &credTypeJson);
    void GetSinkCredTypeForP2P(std::shared_ptr<DmAuthContext> context, const JsonItemObject &credObj,
        JsonObject &aclInfo, JsonObject &credTypeJson, int32_t credType, std::vector<std::string> &deleteCredInfo);
};

class AuthSinkDataSyncState : public DmAuthState {
public:
    virtual ~AuthSinkDataSyncState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcDataSyncState : public DmAuthState {
public:
    virtual ~AuthSrcDataSyncState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSinkFinishState : public DmAuthState {
public:
    virtual ~AuthSinkFinishState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};

class AuthSrcFinishState : public DmAuthState {
public:
    virtual ~AuthSrcFinishState() {};
    DmAuthStateType GetStateType() override;
    int32_t Action(std::shared_ptr<DmAuthContext> context) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_STATE_V2_H