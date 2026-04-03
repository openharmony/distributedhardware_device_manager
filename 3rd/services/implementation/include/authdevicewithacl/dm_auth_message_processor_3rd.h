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

#ifndef OHOS_DM_AUTH_MESSAGE_PROCESSOR_3RD
#define OHOS_DM_AUTH_MESSAGE_PROCESSOR_3RD

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "access_control_profile.h"
#include "crypto_mgr_3rd.h"
#include "dm_auth_info_3rd.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
struct DmAuthContext;
struct DmAccess;
struct DmProxyAuthContext;
struct DmProxyAccess;

constexpr const char* DM_TAG_LOGICAL_SESSION_ID = "logicalSessionId";
// Message type
enum DmMessageType {
    ACL_REQ_NEGOTIATE = 2010,
    ACL_RESP_NEGOTIATE = 2020,
    ACL_REQ_PIN_AUTH_START = 2030,
    ACL_RESP_PIN_AUTH_START = 2040,
    ACL_REQ_PIN_AUTH_MSG_NEGOTIATE = 2031,
    ACL_RESP_PIN_AUTH_MSG_NEGOTIATE = 2041,
    ACL_REQ_DATA_SYNC = 2050,
    ACL_RESP_DATA_SYNC = 2060,
    ACL_REQ_FINISH = 2070,
    ACL_RESP_FINISH = 2080
};


class DmAuthMessageProcessor3rd {
public:
    using CreateMessageFuncPtr =
        int32_t (DmAuthMessageProcessor3rd::*)(std::shared_ptr<DmAuthContext>, JsonObject &jsonObject);
    using ParseMessageFuncPtr =
        int32_t (DmAuthMessageProcessor3rd::*)(const JsonObject &, std::shared_ptr<DmAuthContext>);
    DmAuthMessageProcessor3rd();
    ~DmAuthMessageProcessor3rd();
    // Parse the message, and save the parsed information to the context
    int32_t ParseMessage(std::shared_ptr<DmAuthContext> context, const std::string &message);
    // Create a message, construct the corresponding message based on msgType
    std::string CreateMessage(DmMessageType msgType, std::shared_ptr<DmAuthContext> context);

    // Create and send a message
    void CreateAndSendMsg(DmMessageType msgType, std::shared_ptr<DmAuthContext> context);

    // Save the session key
    int32_t SaveSessionKey(const uint8_t *sessionKey, const uint32_t keyLen);

    // Save the permanent session key to the data profile
    int32_t SaveSessionKeyToDP(int32_t userId, int32_t &skId, std::vector<unsigned char> &sessionKey);
    int32_t SaveDerivativeSessionKeyToDP(int32_t userId, const std::string &suffix, int32_t &skId,
        std::vector<unsigned char> &sessionKey);
    int32_t DeleteSessionKeyToDP(int32_t userId, int32_t skId);
private:
    // construct function implementation
    void ConstructCreateMessageFuncMap();
    void ConstructParseMessageFuncMap();
    std::unordered_map<DmMessageType, CreateMessageFuncPtr> createMessageFuncMap_;
    std::unordered_map<DmMessageType, ParseMessageFuncPtr> parseMessageFuncMap_;
    int32_t ParseMessage2010(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2010Proxy(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2020(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2020Proxy(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2030(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2040(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2031(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2041(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2050(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2060(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2070(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    int32_t ParseMessage2080(const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context);
    
    int32_t CreateMessage2010(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessage2020(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessage2030(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessage2040(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessage2031(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessage2041(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessage2050(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessage2060(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessage2070(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    int32_t CreateMessage2080(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject);
    
    int32_t DecryptSyncMessage(std::shared_ptr<DmAuthContext> &context, DmAccess &access, std::string &enSyncMsg);
    int32_t ParseSyncMessage(std::shared_ptr<DmAuthContext> &context, DmAccess &access, JsonObject &jsonObject);
    bool CheckAccessValidityAndAssign(std::shared_ptr<DmAuthContext> &context, DmAccess &access, DmAccess &accessTmp);
    int32_t ParseProxyAccessToSync(std::shared_ptr<DmAuthContext> &context, JsonObject &jsonObject);
    std::string DecompressSyncMsg(std::string &compressed, uint32_t oriLen);
    std::string Base64Encode(std::string &inputStr);
    std::string Base64Decode(std::string &inputStr);
    int32_t SetSyncMsgJson(std::shared_ptr<DmAuthContext> &context, const DmAccess &accessSide,
        JsonObject &syncMsgJson);
    int32_t CreateProxyAccessMessage(std::shared_ptr<DmAuthContext> &context, JsonObject &syncMsgJson);
    int32_t EncryptSyncMessage(std::shared_ptr<DmAuthContext> &context, DmAccess &accessSide, std::string &encSyncMsg);
    std::string CompressSyncMsg(std::string &inputStr);
private:
    std::shared_ptr<CryptoMgr3rd> cryptoMgr_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_MESSAGE_PROCESSOR_3RD
