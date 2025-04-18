/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_AUTH_MESSAGE_PROCESSOR_H
#define OHOS_DM_AUTH_MESSAGE_PROCESSOR_H

#include <memory>
#include <mutex>
#include <vector>

#include "crypto_adapter.h"
#include "crypto_mgr.h"
#include "dm_auth_manager.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
extern const char* TAG_NET_ID;
extern const char* TAG_TARGET;
extern const char* TAG_APP_OPERATION;
extern const char* TAG_APP_NAME;
extern const char* TAG_APP_DESCRIPTION;
extern const char* TAG_GROUPIDS;
extern const char* TAG_CUSTOM_DESCRIPTION;
extern const char* TAG_DEVICE_TYPE;
extern const char* TAG_REQUESTER;
extern const char* TAG_LOCAL_DEVICE_TYPE;
extern const char* TAG_INDEX;
extern const char* TAG_SLICE_NUM;
extern const char* TAG_IS_AUTH_CODE_READY;
extern const char* TAG_IS_SHOW_DIALOG;
extern const char* TAG_TOKEN;
extern const char* TAG_CRYPTO_NAME;
extern const char* TAG_CRYPTO_VERSION;
extern const char* QR_CODE_KEY;
extern const char* TAG_AUTH_TOKEN;
extern const char* NFC_CODE_KEY;
extern const char* OLD_VERSION_ACCOUNT;

extern const char* TAG_PUBLICKEY;
extern const char* TAG_SESSIONKEY;
extern const char* TAG_BIND_TYPE_SIZE;
extern const char* TAG_HOST_PKGNAME;
extern const char* TAG_HAVECREDENTIAL;
extern const char* TAG_CONFIRM_OPERATION;
extern const char* TAG_IMPORT_AUTH_CODE;
extern const char* TAG_CRYPTIC_MSG;
extern const char* TAG_SESSIONKEY_ID;

class DmAuthManager;
struct DmAuthRequestContext;
struct DmAuthResponseContext;
class ICryptoAdapter;
class AuthMessageProcessor {
public:
    explicit AuthMessageProcessor(std::shared_ptr<DmAuthManager> authMgr);
    ~AuthMessageProcessor();
    std::vector<std::string> CreateAuthRequestMessage();
    std::string CreateSimpleMessage(int32_t msgType);
    int32_t ParseMessage(const std::string &message);
    void SetRequestContext(std::shared_ptr<DmAuthRequestContext> authRequestContext);
    void SetResponseContext(std::shared_ptr<DmAuthResponseContext> authResponseContext);
    std::shared_ptr<DmAuthResponseContext> GetResponseContext();
    std::shared_ptr<DmAuthRequestContext> GetRequestContext();
    std::string CreateDeviceAuthMessage(int32_t msgType, const uint8_t *data, uint32_t dataLen);
    void CreateResponseAuthMessageExt(JsonObject &json);
    void ParseAuthResponseMessageExt(JsonObject &json);
    void SetEncryptFlag(bool flag);
    int32_t SaveSessionKey(const uint8_t *sessionKey, const uint32_t keyLen);
    int32_t ProcessSessionKey(const uint8_t *sessionKey, const uint32_t keyLen);

private:
    std::string CreateRequestAuthMessage(JsonObject &json);
    void CreateNegotiateMessage(JsonObject &json);
    void CreateRespNegotiateMessage(JsonObject &json);
    void CreateSyncGroupMessage(JsonObject &json);
    void CreateResponseAuthMessage(JsonObject &json);
    void ParseAuthResponseMessage(JsonObject &json);
    int32_t ParseAuthRequestMessage(JsonObject &json);
    void ParseNegotiateMessage(const JsonObject &json);
    void ParseRespNegotiateMessage(const JsonObject &json);
    void CreateResponseFinishMessage(JsonObject &json);
    void ParseResponseFinishMessage(JsonObject &json);
    void GetAuthReqMessage(JsonObject &json);
    void ParsePkgNegotiateMessage(const JsonObject &json);
    void CreatePublicKeyMessageExt(JsonObject &json);
    void ParsePublicKeyMessageExt(JsonObject &json);
    void GetJsonObj(JsonObject &jsonObj);
    void CreateReqReCheckMessage(JsonObject &jsonObj);
    void ParseReqReCheckMessage(JsonObject &json);
    bool IsPincodeImported();

private:
    std::weak_ptr<DmAuthManager> authMgr_;
    std::shared_ptr<ICryptoAdapter> cryptoAdapter_;
    std::shared_ptr<DmAuthRequestContext> authRequestContext_;
    std::shared_ptr<DmAuthResponseContext> authResponseContext_;
    std::mutex encryptFlagMutex_;
    bool encryptFlag_ = false;
    std::shared_ptr<CryptoMgr> cryptoMgr_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_MESSAGE_PROCESSOR_H
