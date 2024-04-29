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
#include <vector>

#include "crypto_adapter.h"
#include "dm_auth_manager.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
constexpr const char* TAG_REPLY = "REPLY";
constexpr const char* TAG_NET_ID = "NETID";
constexpr const char* TAG_TARGET = "TARGET";
constexpr const char* TAG_APP_OPERATION = "APPOPERATION";
constexpr const char* TAG_APP_NAME = "APPNAME";
constexpr const char* TAG_APP_DESCRIPTION = "APPDESC";
constexpr const char* TAG_GROUPIDS = "GROUPIDLIST";
constexpr const char* TAG_CUSTOM_DESCRIPTION = "CUSTOMDESC";
constexpr const char* TAG_DEVICE_TYPE = "DEVICETYPE";
constexpr const char* TAG_REQUESTER = "REQUESTER";
constexpr const char* TAG_LOCAL_DEVICE_ID = "LOCALDEVICEID";
constexpr const char* TAG_LOCAL_DEVICE_TYPE = "LOCALDEVICETYPE";
constexpr const char* TAG_INDEX = "INDEX";
constexpr const char* TAG_SLICE_NUM = "SLICE";
constexpr const char* TAG_IS_AUTH_CODE_READY = "IS_AUTH_CODE_READY";
constexpr const char* TAG_IS_SHOW_DIALOG = "IS_SHOW_DIALOG";
constexpr const char* TAG_TOKEN = "TOKEN";
constexpr const char* TAG_CRYPTO_NAME = "CRYPTONAME";
constexpr const char* TAG_CRYPTO_VERSION = "CRYPTOVERSION";
constexpr const char* TAG_IDENTICAL_ACCOUNT = "IDENTICALACCOUNT";
constexpr const char* TAG_ACCOUNT_GROUPID = "ACCOUNTGROUPID";
constexpr const char* APP_THUMBNAIL = "appThumbnail";
constexpr const char* QR_CODE_KEY = "qrCode";
constexpr const char* TAG_AUTH_TOKEN = "authToken";
constexpr const char* NFC_CODE_KEY = "nfcCode";
constexpr const char* OLD_VERSION_ACCOUNT = "oldVersionAccount";

constexpr const char* TAG_HAVE_CREDENTIAL = "haveCredential";
constexpr const char* TAG_PUBLICKEY = "publicKey";
constexpr const char* TAG_SESSIONKEY = "sessionKey";
constexpr const char* TAG_BIND_LEVEL = "bindLevel";
constexpr const char* TAG_LOCAL_USERID = "localUserId";
constexpr const char* TAG_BIND_TYPE_SIZE = "bindTypeSize";
constexpr const char* TAG_ISONLINE = "isOnline";
constexpr const char* TAG_AUTHED = "authed";
constexpr const char* TAG_LOCAL_ACCOUNTID = "localAccountId";
constexpr const char* TAG_DMVERSION = "dmVersion";
constexpr const char* TAG_HOST_PKGNAME = "hostPkgname";
constexpr const char* TAG_TOKENID = "tokenId";
constexpr const char* TAG_HAVECREDENTIAL = "haveCredential";
constexpr const char* TAG_CONFIRM_OPERATION = "confirmOperation";
constexpr const char* TAG_DATA = "data";
constexpr const char* TAG_DATA_LEN = "dataLen";
constexpr const char* TAG_IMPORT_AUTH_CODE = "IMPORT_AUTH_CODE";

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
    void CreateResponseAuthMessageExt(nlohmann::json &json);
    void ParseAuthResponseMessageExt(nlohmann::json &json);
    
private:
    std::string CreateRequestAuthMessage(nlohmann::json &json);
    void CreateNegotiateMessage(nlohmann::json &json);
    void CreateRespNegotiateMessage(nlohmann::json &json);
    void CreateSyncGroupMessage(nlohmann::json &json);
    void CreateResponseAuthMessage(nlohmann::json &json);
    void ParseAuthResponseMessage(nlohmann::json &json);
    int32_t ParseAuthRequestMessage(nlohmann::json &json);
    void ParseNegotiateMessage(const nlohmann::json &json);
    void ParseRespNegotiateMessage(const nlohmann::json &json);
    void CreateResponseFinishMessage(nlohmann::json &json);
    void ParseResponseFinishMessage(nlohmann::json &json);
    void GetAuthReqMessage(nlohmann::json &json);
    void ParsePkgNegotiateMessage(const nlohmann::json &json);
    void CreatePublicKeyMessageExt(nlohmann::json &json);
    void ParsePublicKeyMessageExt(nlohmann::json &json);
    void CreateSyncDeleteMessageExt(nlohmann::json &json);
    void ParseSyncDeleteMessageExt(nlohmann::json &json);
    void GetJsonObj(nlohmann::json &jsonObj);

private:
    std::weak_ptr<DmAuthManager> authMgr_;
    std::shared_ptr<ICryptoAdapter> cryptoAdapter_;
    std::shared_ptr<DmAuthRequestContext> authRequestContext_;
    std::shared_ptr<DmAuthResponseContext> authResponseContext_;
    std::vector<nlohmann::json> authSplitJsonList_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_MESSAGE_PROCESSOR_H
