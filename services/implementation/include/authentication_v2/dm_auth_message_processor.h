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

#ifndef OHOS_DM_AUTH_MESSAGE_PROCESSOR_V2_H
#define OHOS_DM_AUTH_MESSAGE_PROCESSOR_V2_H

#include <memory>

#include "access_control_profile.h"
#include "crypto_mgr.h"
#include "deviceprofile_connector.h"
#include "dm_ability_manager.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
struct DmAuthContext;
struct DmAccess;
struct DmProxyAuthContext;
struct DmProxyAccess;

extern const char* TAG_LNN_PUBLIC_KEY;
extern const char* TAG_TRANSMIT_PUBLIC_KEY;
extern const char* TAG_LNN_CREDENTIAL_ID;
extern const char* TAG_TRANSMIT_CREDENTIAL_ID;
extern const char* TAG_CONFIRM_OPERATION_V2;
extern const char* TAG_AUTH_TYPE_LIST;
extern const char* TAG_CURRENT_AUTH_TYPE_IDX;

// IS interface input parameter json format string key
extern const char* TAG_METHOD;
extern const char* TAG_PEER_USER_SPACE_ID;
extern const char* TAG_SUBJECT;
extern const char* TAG_CRED_TYPE;
extern const char* TAG_KEY_FORMAT;
extern const char* TAG_ALGORITHM_TYPE;
extern const char* TAG_PROOF_TYPE;
extern const char* TAG_KEY_VALUE;
extern const char* TAG_AUTHORIZED_SCOPE;
extern const char* TAG_AUTHORIZED_APP_LIST;
extern const char* TAG_CREDENTIAL_OWNER;
extern const char* TAG_SYNC;
extern const char* TAG_ACCESS;
extern const char* TAG_PROXY;
extern const char* TAG_ACL;
extern const char* TAG_ACCESSER;
extern const char* TAG_ACCESSEE;
extern const char* TAG_SERVICEINFO;
extern const char* TAG_USER_CONFIRM_OPT;
// The local SK information is synchronized to the remote end to construct acl-accesser/accessee.
extern const char* TAG_TRANSMIT_SK_ID;
extern const char* TAG_LNN_SK_ID;
extern const char* TAG_TRANSMIT_SK_TIMESTAMP;
extern const char* TAG_LNN_SK_TIMESTAMP;
extern const char* TAG_USER_ID;
extern const char* TAG_TOKEN_ID;
extern const char* TAG_NETWORKID_ID;
extern const char* TAG_ISSUER;

extern const char* TAG_DEVICE_VERSION;
extern const char* TAG_DEVICE_NAME;
extern const char* TAG_DEVICE_ID_HASH;
extern const char* TAG_ACCOUNT_ID_HASH;
extern const char* TAG_TOKEN_ID_HASH;
extern const char* TAG_SESSION_NAME;
extern const char* TAG_ACL_CHECKSUM;
extern const char* TAG_COMPRESS_ORI_LEN;
extern const char* TAG_COMPRESS;
extern const char* TAG_REPLY;
extern const char* TAG_STATE;
extern const char* TAG_REASON;
extern const char* TAG_PEER_USER_ID;
extern const char* TAG_PEER_DISPLAY_ID;
extern const char* TAG_EXTRA_INFO;

extern const char* TAG_IS_ONLINE;
extern const char* TAG_IS_AUTHED;
extern const char* TAG_CREDENTIAL_INFO;
extern const char* TAG_CERT_INFO;
extern const char* TAG_LANGUAGE;
extern const char* TAG_ULTRASONIC_SIDE;

// Logical Session Tag
constexpr const char* DM_TAG_LOGICAL_SESSION_ID = "logicalSessionId";

// Message type
enum DmMessageType {
    // Terminate/Exception Message
    MSG_TYPE_UNKNOWN = 0,
    MSG_TYPE_AUTH_TERMINATE = 1,
    // Normal Message
    MSG_TYPE_REQ_ACL_NEGOTIATE = 80,
    MSG_TYPE_RESP_ACL_NEGOTIATE = 90,
    MSG_TYPE_REQ_USER_CONFIRM = 100,
    MSG_TYPE_FORWARD_ULTRASONIC_START = 101,
    MSG_TYPE_REVERSE_ULTRASONIC_START = 102,
    MSG_TYPE_RESP_USER_CONFIRM = 110,
    MSG_TYPE_FORWARD_ULTRASONIC_NEGOTIATE = 111,
    MSG_TYPE_REVERSE_ULTRASONIC_DONE = 112,
    MSG_TYPE_REQ_PIN_AUTH_START = 120,
    MSG_TYPE_RESP_PIN_AUTH_START = 130,
    MSG_TYPE_REQ_PIN_AUTH_MSG_NEGOTIATE = 121,
    MSG_TYPE_RESP_PIN_AUTH_MSG_NEGOTIATE = 131,
    MSG_TYPE_REQ_CREDENTIAL_EXCHANGE = 140,
    MSG_TYPE_RESP_CREDENTIAL_EXCHANGE = 150,
    MSG_TYPE_REQ_SK_DERIVE = 141,
    MSG_TYPE_RESP_SK_DERIVE = 151,
    MSG_TYPE_REQ_CREDENTIAL_AUTH_START = 160,
    MSG_TYPE_RESP_CREDENTIAL_AUTH_START = 170,
    MSG_TYPE_REQ_CREDENTIAL_AUTH_NEGOTIATE = 161,
    MSG_TYPE_RESP_CREDENTIAL_AUTH_NEGOTIATE = 171,
    MSG_TYPE_REQ_DATA_SYNC = 180,
    MSG_TYPE_RESP_DATA_SYNC = 190,
    MSG_TYPE_AUTH_REQ_FINISH = 200,
    MSG_TYPE_AUTH_RESP_FINISH = 201,
};

struct DmAccessControlTable {
    int32_t accessControlId;
    int64_t accesserId;
    int64_t accesseeId;
    std::string deviceId;
    std::string sessionKey;
    int32_t bindType;
    uint32_t authType;
    uint32_t deviceType;
    std::string deviceIdHash;
    int32_t status;
    int32_t validPeriod;
    int32_t lastAuthTime;
    uint32_t bindLevel;
};

// Structure used for synchronizing ACL access
// Attention: Modifying this structure requires updating the From/ToJson functions in dm_auth_message_processor.cpp.
struct DmAccessToSync {
    // the deviceName translate in 80/90 messages, cleaned the privacy info
    std::string deviceName;
    // the device display name, which need save in DB
    std::string deviceNameFull;
    // For A->B communication, whether it's the A end or B end, the Accesser object stores
    // the A end's deviceId, and the Accessee object stores the B end's deviceId
    std::string deviceId;
    int32_t userId;
    std::string accountId;
    uint64_t tokenId;
    std::string bundleName;       // Stores the bundleName, HAP -> bundleName in BMS, SA -> process name
    std::string pkgName;          // Store pkgName set by client
    int32_t bindLevel;            // Passed through for business purposes, no custom definition required
    int32_t sessionKeyId;         // User credential ID
    int64_t skTimeStamp;          // Used for aging, time is 2 days, user-level credential timestamp
    // the user confirm operation result for the confirm dialog, means allow or allow always.
    int32_t userConfirmOperation = UiAction::USER_OPERATION_TYPE_CANCEL_AUTH;
};

// json and struct conversion functions
void ToJson(JsonItemObject &itemObject, const DmAccessControlTable &table);
void FromJson(const JsonItemObject &itemObject, DmAccessControlTable &table);
void ToJson(JsonItemObject &itemObject, const DmAccessToSync &table);
void FromJson(const JsonItemObject &itemObject, DmAccessToSync &table);

using JsonTypeCheckFuncPtr = bool (JsonItemObject::*)() const;

template <typename T>
void SetValueFromJson(const JsonItemObject &itemObject, const std::string &key,
    JsonTypeCheckFuncPtr funcPtr, T &value)
{
    if (itemObject.Contains(key) && (itemObject[key].*funcPtr)()) {
        value = itemObject[key].Get<T>();
    }
}

class DmAuthMessageProcessor {
public:
    using CreateMessageFuncPtr =
        int32_t (DmAuthMessageProcessor::*)(std::shared_ptr<DmAuthContext>, JsonObject &jsonObject);
    using ParaseMessageFuncPtr =
        int32_t (DmAuthMessageProcessor::*)(const JsonObject &, std::shared_ptr<DmAuthContext>);

    DmAuthMessageProcessor();
    ~DmAuthMessageProcessor();
    // Parse the message, and save the parsed information to the context
    int32_t ParseMessage(std::shared_ptr<DmAuthContext> context, const std::string &message);
    // Create a message, construct the corresponding message based on msgType
    std::string CreateMessage(DmMessageType msgType, std::shared_ptr<DmAuthContext> context);

    // Create and send a message
    void CreateAndSendMsg(DmMessageType msgType, std::shared_ptr<DmAuthContext> context);

    // Save the session key
    int32_t SaveSessionKey(const uint8_t *sessionKey, const uint32_t keyLen);

    // Save the permanent session key to the data profile
    int32_t SaveSessionKeyToDP(int32_t userId, int32_t &skId);
    int32_t SaveDerivativeSessionKeyToDP(int32_t userId, const std::string &suffix, int32_t &skId);
    int32_t GetSessionKey(int32_t userId, int32_t &skId);
    // Save the current access control list
    int32_t PutAccessControlList(std::shared_ptr<DmAuthContext> context,
        DmAccess &access, std::string trustDeviceId);
    int32_t SetProxyAccess(std::shared_ptr<DmAuthContext> context, DmProxyAuthContext &proxyAuthContext,
        DistributedDeviceProfile::Accesser &accesser, DistributedDeviceProfile::Accessee &accessee);
    int32_t PutProxyAccessControlList(std::shared_ptr<DmAuthContext> context,
        DistributedDeviceProfile::AccessControlProfile &profile, DistributedDeviceProfile::Accesser &accesser,
        DistributedDeviceProfile::Accessee &accessee);
    bool IsExistTheToken(JsonObject &proxyObj, int64_t tokenId);
    void SetAclProxyRelate(std::shared_ptr<DmAuthContext> context);
    void SetAclProxyRelate(std::shared_ptr<DmAuthContext> context,
        DistributedDeviceProfile::AccessControlProfile &profile);
    // Extract the access control list (ACL) for message parsing and bus usage.
    // If no ACL is available, return an empty string. The returned string is in
    // JSON format: {dmversion:x,accesser:[{accesserDeviceId:y,...},...], accessee:{...}}
    int32_t DeleteSessionKeyToDP(int32_t userId, int32_t skId);

    void DmAuthUltrasonicMessageProcessor();

private:
    // Internal implementations for various message types

    // Used to encrypt the synchronization message
    int32_t EncryptSyncMessage(std::shared_ptr<DmAuthContext> &context, DmAccess &accessSide, std::string &encSyncMsg);
    int32_t CreateProxyAccessMessage(std::shared_ptr<DmAuthContext> &context, JsonObject &syncMsgJson);
    // Parse the authentication start message
    int32_t ParseAuthStartMessage(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);

    // Parse the 80 message
    int32_t ParseNegotiateMessage(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseProxyNegotiateMessage(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 90 message
    int32_t ParseMessageRespAclNegotiate(const JsonObject &json, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessageProxyRespAclNegotiate(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 100 message
    int32_t ParseMessageReqUserConfirm(const JsonObject &json, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessageProxyReqUserConfirm(const JsonObject &json, std::shared_ptr<DmAuthContext> context);
    // Parse the 110 message
    int32_t ParseMessageRespUserConfirm(const JsonObject &json, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessageProxyRespUserConfirm(const JsonObject &json, std::shared_ptr<DmAuthContext> context);
    // Parse the 120 message
    int32_t ParseMessageReqPinAuthStart(const JsonObject &json, std::shared_ptr<DmAuthContext> context);
    // Parse the 130 message
    int32_t ParseMessageRespPinAuthStart(const JsonObject &json, std::shared_ptr<DmAuthContext> context);
    // Parse the 121 message
    int32_t ParseMessageReqPinAuthNegotiate(const JsonObject &json, std::shared_ptr<DmAuthContext> context);
    // Parse the 131 message
    int32_t ParseMessageRespPinAuthNegotiate(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 140 message
    int32_t ParseMessageReqCredExchange(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 150 message
    int32_t ParseMessageRspCredExchange(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseProxyCredExchangeToSync(std::shared_ptr<DmAuthContext> &context, JsonObject &jsonObject);
    // Parse the 141 message
    int32_t ParseMessageReqSKDerive(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 151 message
    int32_t ParseMessageRspSKDerive(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 161, 170, and 171 messages
    int32_t ParseMessageNegotiateTransmit(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 180 message
    int32_t ParseMessageSyncReq(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 190 message
    int32_t ParseMessageSyncResp(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 200 message
    int32_t ParseMessageSinkFinish(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 201 message
    int32_t ParseMessageSrcFinish(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 102 message
    int32_t ParseMessageReverseUltrasonicStart(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 112 message
    int32_t ParseMessageReverseUltrasonicDone(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 101 message
    int32_t ParseMessageForwardUltrasonicStart(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    // Parse the 111 message
    int32_t ParseMessageForwardUltrasonicNegotiate(const JsonObject &jsonObject,
        std::shared_ptr<DmAuthContext> context);

    // Create the 80 message
    int32_t CreateNegotiateMessage(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateProxyNegotiateMessage(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    // Create the 90 message
    int32_t CreateRespNegotiateMessage(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateProxyRespNegotiateMessage(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    // Create the 100 message
    int32_t CreateMessageReqUserConfirm(std::shared_ptr<DmAuthContext> context, JsonObject &json);
    int32_t CreateMessageProxyReqUserConfirm(std::shared_ptr<DmAuthContext> context, JsonObject &json);
    // Create the 110 message
    int32_t CreateMessageRespUserConfirm(std::shared_ptr<DmAuthContext> context, JsonObject &json);
    // Create the 120 message
    int32_t CreateMessageReqPinAuthStart(std::shared_ptr<DmAuthContext> context, JsonObject &json);
    // Create the 130 message
    int32_t CreateMessageRespPinAuthStart(std::shared_ptr<DmAuthContext> context, JsonObject &json);
    // Create the 121 message
    int32_t CreateMessageReqPinAuthNegotiate(std::shared_ptr<DmAuthContext> context, JsonObject &json);
    // Create the 131 message
    int32_t CreateMessageRespPinAuthNegotiate(std::shared_ptr<DmAuthContext> context, JsonObject &json);
    // Create the 140 message
    int32_t CreateMessageReqCredExchange(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    // Create the 150 message
    int32_t CreateMessageRspCredExchange(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateProxyCredExchangeMessage(std::shared_ptr<DmAuthContext> &context, JsonObject &jsonData);
    // Create 141 message.
    int32_t CreateMessageReqSKDerive(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    // Create 151 message.
    int32_t CreateMessageRspSKDerive(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    // Create the 160 message
    int32_t CreateMessageReqCredAuthStart(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    // Construct the 161, 170, and 171 credential authentication messages
    int32_t CreateCredentialNegotiateMessage(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateNegotiateOldMessage(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    // Construct the 180 and 190 sync messages
    int32_t CreateSyncMessage(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    // Create the 190 message
    int32_t CreateMessageSyncResp(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    // Create the 200 message
    int32_t CreateMessageFinish(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);

    // Compress the sync message
    std::string CompressSyncMsg(std::string &inputStr);
    // Decompress the sync message
    std::string DecompressSyncMsg(std::string& compressed, uint32_t oriLen);
    // Serialize the ACL
    int32_t ACLToStr(DistributedDeviceProfile::AccessControlProfile acl, std::string aclStr);
    // Decrypt the 180 and 190 messages
    int32_t DecryptSyncMessage(std::shared_ptr<DmAuthContext> &context,
        DmAccess &access, std::string &enSyncMsg);
    // Parse the sync message
    int32_t ParseSyncMessage(std::shared_ptr<DmAuthContext> &context,
        DmAccess &access, JsonObject &jsonObject);
    int32_t ParseProxyAccessToSync(std::shared_ptr<DmAuthContext> &context, JsonObject &jsonObject);
    int32_t CreateMessageForwardUltrasonicStart(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessageReverseUltrasonicStart(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessageForwardUltrasonicNegotiate(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessageReverseUltrasonicDone(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);

    std::string Base64Encode(std::string &inputStr);
    std::string Base64Decode(std::string &inputStr);
    void SetAccessControlList(std::shared_ptr<DmAuthContext> context,
        DistributedDeviceProfile::AccessControlProfile &profile);
    void SetTransmitAccessControlList(std::shared_ptr<DmAuthContext> context,
        DistributedDeviceProfile::Accesser &accesser, DistributedDeviceProfile::Accessee &accessee);
    void SetLnnAccessControlList(std::shared_ptr<DmAuthContext> context,
        DistributedDeviceProfile::Accesser &accesser, DistributedDeviceProfile::Accessee &accessee);
    int32_t CheckLogicalSessionId(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    bool CheckAccessValidityAndAssign(std::shared_ptr<DmAuthContext> &context, DmAccess &access,
        DmAccess &accessTmp);
    void ParseUltrasonicSide(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    void ParseCert(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    void ParseAccesserInfo(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    std::shared_ptr<CryptoMgr> cryptoMgr_ = nullptr;
    std::unordered_map<DmMessageType, CreateMessageFuncPtr> createMessageFuncMap_;
    std::unordered_map<DmMessageType, ParaseMessageFuncPtr> paraseMessageFuncMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_MESSAGE_PROCESSOR_V2_H