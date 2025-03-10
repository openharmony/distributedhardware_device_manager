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

#include "auth_message_processor.h"

#include "dm_auth_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
const char* TAG_REPLY = "REPLY";
const char* TAG_NET_ID = "NETID";
const char* TAG_TARGET = "TARGET";
const char* TAG_APP_OPERATION = "APPOPERATION";
const char* TAG_APP_NAME = "APPNAME";
const char* TAG_APP_DESCRIPTION = "APPDESC";
const char* TAG_GROUPIDS = "GROUPIDLIST";
const char* TAG_CUSTOM_DESCRIPTION = "CUSTOMDESC";
const char* TAG_DEVICE_TYPE = "DEVICETYPE";
const char* TAG_REQUESTER = "REQUESTER";
const char* TAG_LOCAL_DEVICE_ID = "LOCALDEVICEID";
const char* TAG_LOCAL_DEVICE_TYPE = "LOCALDEVICETYPE";
const char* TAG_INDEX = "INDEX";
const char* TAG_SLICE_NUM = "SLICE";
const char* TAG_IS_AUTH_CODE_READY = "IS_AUTH_CODE_READY";
const char* TAG_IS_SHOW_DIALOG = "IS_SHOW_DIALOG";
const char* TAG_TOKEN = "TOKEN";
const char* TAG_CRYPTO_NAME = "CRYPTONAME";
const char* TAG_CRYPTO_VERSION = "CRYPTOVERSION";
const char* TAG_IDENTICAL_ACCOUNT = "IDENTICALACCOUNT";
const char* TAG_ACCOUNT_GROUPID = "ACCOUNTGROUPID";
const char* APP_THUMBNAIL = "appThumbnail";
const char* QR_CODE_KEY = "qrCode";
const char* TAG_AUTH_TOKEN = "authToken";
const char* NFC_CODE_KEY = "nfcCode";
const char* OLD_VERSION_ACCOUNT = "oldVersionAccount";
const char* TAG_AUTH_FINISH = "isFinish";

const char* TAG_HAVE_CREDENTIAL = "haveCredential";
const char* TAG_PUBLICKEY = "publicKey";
const char* TAG_SESSIONKEY = "sessionKey";
const char* TAG_BIND_LEVEL = "bindLevel";
const char* TAG_LOCAL_USERID = "localUserId";
const char* TAG_BIND_TYPE_SIZE = "bindTypeSize";
const char* TAG_ISONLINE = "isOnline";
const char* TAG_AUTHED = "authed";
const char* TAG_LOCAL_ACCOUNTID = "localAccountId";
const char* TAG_DMVERSION = "dmVersion";
const char* TAG_HOST_PKGNAME = "hostPkgname";
const char* TAG_TOKENID = "tokenId";
const char* TAG_HAVECREDENTIAL = "haveCredential";
const char* TAG_CONFIRM_OPERATION = "confirmOperation";
const char* TAG_DATA = "data";
const char* TAG_DATA_LEN = "dataLen";
const char* TAG_IMPORT_AUTH_CODE = "IMPORT_AUTH_CODE";
const char* TAG_HOST_PKGLABEL = "hostPkgLabel";
const char* TAG_EDITION = "edition";
const char* TAG_BUNDLE_NAME = "bundleName";
const char* TAG_CRYPTIC_MSG = "encryptMsg";
const char* TAG_PEER_BUNDLE_NAME = "PEER_BUNDLE_NAME";
const char* TAG_REMOTE_DEVICE_NAME = "REMOTE_DEVICE_NAME";
const char* TAG_SESSIONKEY_ID = "sessionKeyId";

const int32_t MSG_MAX_SIZE = 45 * 1024;
const int32_t GROUP_VISIBILITY_IS_PRIVATE = 0;
const int32_t MAX_BINDTYPE_SIZE = 1000;
constexpr const char* TAG_HOST = "HOST";
constexpr const char* TAG_VISIBILITY = "VISIBILITY";
constexpr const char* TAG_APP_THUMBNAIL = "APPTHUM";
constexpr const char* TAG_THUMBNAIL_SIZE = "THUMSIZE";

AuthMessageProcessor::AuthMessageProcessor(std::shared_ptr<DmAuthManager> authMgr) : authMgr_(authMgr)
{
    LOGI("AuthMessageProcessor constructor");
    cryptoMgr_ = std::make_shared<CryptoMgr>();
}

AuthMessageProcessor::~AuthMessageProcessor()
{
    authMgr_.reset();
    if (cryptoMgr_ != nullptr) {
        cryptoMgr_->ClearSessionKey();
        cryptoMgr_ = nullptr;
    }
}

void AuthMessageProcessor::GetJsonObj(nlohmann::json &jsonObj)
{
    if (authResponseContext_ == nullptr || authResponseContext_->bindType.size() > MAX_BINDTYPE_SIZE) {
        LOGE("GetJsonObj invalid bindType size.");
        return;
    }
    jsonObj[TAG_VER] = DM_ITF_VER;
    jsonObj[TAG_MSG_TYPE] = MSG_TYPE_REQ_AUTH;
    jsonObj[TAG_INDEX] = 0;
    jsonObj[TAG_REQUESTER] = authRequestContext_->localDeviceName;
    jsonObj[TAG_DEVICE_ID] = authRequestContext_->deviceId;
    jsonObj[TAG_LOCAL_DEVICE_ID] = authRequestContext_->localDeviceId;
    jsonObj[TAG_LOCAL_DEVICE_TYPE] = authRequestContext_->localDeviceTypeId;
    jsonObj[TAG_DEVICE_TYPE] = authRequestContext_->deviceTypeId;
    jsonObj[TAG_AUTH_TYPE] = authRequestContext_->authType;
    jsonObj[TAG_TOKEN] = authRequestContext_->token;
    jsonObj[TAG_VISIBILITY] = authRequestContext_->groupVisibility;
    if (authRequestContext_->groupVisibility == GROUP_VISIBILITY_IS_PRIVATE) {
        jsonObj[TAG_TARGET] = authRequestContext_->targetPkgName;
        jsonObj[TAG_HOST] = authRequestContext_->hostPkgName;
    }
    if (authRequestContext_->authType == AUTH_TYPE_IMPORT_AUTH_CODE) {
        jsonObj[TAG_IS_SHOW_DIALOG] = false;
    } else {
        jsonObj[TAG_IS_SHOW_DIALOG] = true;
    }
    jsonObj[TAG_APP_OPERATION] = authRequestContext_->appOperation;
    jsonObj[TAG_CUSTOM_DESCRIPTION] = authRequestContext_->customDesc;
    jsonObj[TAG_APP_NAME] = authRequestContext_->appName;
    jsonObj[TAG_APP_DESCRIPTION] = authRequestContext_->appDesc;
    jsonObj[TAG_BIND_TYPE_SIZE] = authResponseContext_->bindType.size();
    for (uint32_t item = 0; item < authResponseContext_->bindType.size(); item++) {
        std::string itemStr = std::to_string(item);
        jsonObj[itemStr] = authResponseContext_->bindType[item];
    }
}

bool AuthMessageProcessor::IsPincodeImported()
{
    auto sptrAuthMgr = authMgr_.lock();
    if (sptrAuthMgr == nullptr) {
        return false;
    }
    return sptrAuthMgr->IsImportedAuthCodeValid();
}

std::vector<std::string> AuthMessageProcessor::CreateAuthRequestMessage()
{
    LOGI("AuthMessageProcessor::CreateAuthRequestMessage start.");
    std::vector<std::string> jsonStrVec;
    if (authRequestContext_ == nullptr) {
        LOGE("AuthMessageProcessor::CreateAuthRequestMessage authRequestContext_ is nullptr.");
        return jsonStrVec;
    }
    int32_t thumbnailSize = (int32_t)(authRequestContext_->appThumbnail.size());
    int32_t thumbnailSlice = ((thumbnailSize / MSG_MAX_SIZE) + (thumbnailSize % MSG_MAX_SIZE) == 0 ? 0 : 1);
    nlohmann::json jsonObj;
    jsonObj[TAG_SLICE_NUM] = thumbnailSlice + 1;
    jsonObj[TAG_THUMBNAIL_SIZE] = thumbnailSize;
    GetJsonObj(jsonObj);
    jsonStrVec.push_back(SafetyDump(jsonObj));
    for (int32_t idx = 0; idx < thumbnailSlice; idx++) {
        nlohmann::json jsonThumbnailObj;
        jsonThumbnailObj[TAG_VER] = DM_ITF_VER;
        jsonThumbnailObj[TAG_MSG_TYPE] = MSG_TYPE_REQ_AUTH;
        jsonThumbnailObj[TAG_SLICE_NUM] = thumbnailSlice + 1;
        jsonThumbnailObj[TAG_INDEX] = idx + 1;
        jsonThumbnailObj[TAG_DEVICE_ID] = authRequestContext_->deviceId;
        jsonThumbnailObj[TAG_THUMBNAIL_SIZE] = thumbnailSize;
        int32_t leftLen = thumbnailSize - idx * MSG_MAX_SIZE;
        int32_t sliceLen = (leftLen > MSG_MAX_SIZE) ? MSG_MAX_SIZE : leftLen;
        LOGI("TAG_APP_THUMBNAIL encode, idx %{public}d, sliceLen %{public}d, thumbnailSize %{public}d", idx,
            (uint32_t)sliceLen, thumbnailSize);
        jsonObj[TAG_APP_THUMBNAIL] = authRequestContext_->appThumbnail.substr(idx * MSG_MAX_SIZE, sliceLen);
        jsonStrVec.push_back(SafetyDump(jsonThumbnailObj));
    }
    return jsonStrVec;
}

std::string AuthMessageProcessor::CreateSimpleMessage(int32_t msgType)
{
    LOGI("AuthMessageProcessor::CreateSimpleMessage start. msgType is %{public}d", msgType);
    nlohmann::json jsonObj;
    jsonObj[TAG_VER] = DM_ITF_VER;
    jsonObj[TAG_MSG_TYPE] = msgType;
    switch (msgType) {
        case MSG_TYPE_NEGOTIATE:
            CreateNegotiateMessage(jsonObj);
            break;
        case MSG_TYPE_RESP_NEGOTIATE:
            CreateRespNegotiateMessage(jsonObj);
            break;
        case MSG_TYPE_SYNC_GROUP:
            CreateSyncGroupMessage(jsonObj);
            break;
        case MSG_TYPE_RESP_AUTH:
            CreateResponseAuthMessage(jsonObj);
            break;
        case MSG_TYPE_RESP_AUTH_EXT:
            CreateResponseAuthMessageExt(jsonObj);
            break;
        case MSG_TYPE_REQ_AUTH_TERMINATE:
            CreateResponseFinishMessage(jsonObj);
            break;
        case MSG_TYPE_REQ_PUBLICKEY:
        case MSG_TYPE_RESP_PUBLICKEY:
            CreatePublicKeyMessageExt(jsonObj);
            break;
        case MSG_TYPE_REQ_RECHECK_MSG:
        case MSG_TYPE_RESP_RECHECK_MSG:
            CreateReqReCheckMessage(jsonObj);
            break;
        default:
            break;
    }
    return SafetyDump(jsonObj);
}

void AuthMessageProcessor::CreatePublicKeyMessageExt(nlohmann::json &json)
{
    bool encryptFlag = false;
    {
        std::lock_guard<std::mutex> mutexLock(encryptFlagMutex_);
        encryptFlag = encryptFlag_;
    }
    if (!encryptFlag) {
        LOGI("not encrypt publickey.");
        json[TAG_PUBLICKEY] = authResponseContext_->publicKey;
        return;
    } else {
        nlohmann::json jsonTemp;
        auto sptr = authMgr_.lock();
        if (sptr != nullptr && !sptr->IsSrc()) {
            authResponseContext_->localSessionKeyId = sptr->GetSessionKeyIdSync(authResponseContext_->requestId);
            jsonTemp[TAG_SESSIONKEY_ID] = authResponseContext_->localSessionKeyId;
        }
        jsonTemp[TAG_PUBLICKEY] = authResponseContext_->publicKey;
        std::string strTemp = SafetyDump(jsonTemp);
        std::string encryptStr = "";
        CHECK_NULL_VOID(cryptoMgr_);
        if (cryptoMgr_->EncryptMessage(strTemp, encryptStr) != DM_OK) {
            LOGE("EncryptMessage failed.");
            return;
        }
        json[TAG_CRYPTIC_MSG] = encryptStr;
        return;
    }
}

void AuthMessageProcessor::CreateResponseAuthMessageExt(nlohmann::json &json)
{
    json[TAG_REPLY] = authResponseContext_->reply;
    json[TAG_TOKEN] = authResponseContext_->token;
    json[TAG_CONFIRM_OPERATION] = authResponseContext_->confirmOperation;
    json[TAG_REQUEST_ID] = authResponseContext_->requestId;
}

void AuthMessageProcessor::CreateNegotiateMessage(nlohmann::json &json)
{
    if (cryptoAdapter_ == nullptr) {
        json[TAG_CRYPTO_SUPPORT] = false;
    } else {
        json[TAG_CRYPTO_SUPPORT] = true;
        json[TAG_CRYPTO_NAME] = cryptoAdapter_->GetName();
        json[TAG_CRYPTO_VERSION] = cryptoAdapter_->GetVersion();
        json[TAG_DEVICE_ID] = authResponseContext_->deviceId;
    }
    json[TAG_AUTH_TYPE] = authResponseContext_->authType;
    json[TAG_REPLY] = authResponseContext_->reply;
    json[TAG_LOCAL_DEVICE_ID] = authResponseContext_->localDeviceId;
    json[TAG_ACCOUNT_GROUPID] = authResponseContext_->accountGroupIdHash;

    json[TAG_BIND_LEVEL] = authResponseContext_->bindLevel;
    json[TAG_LOCAL_ACCOUNTID] = authResponseContext_->localAccountId;
    json[TAG_LOCAL_USERID] = authResponseContext_->localUserId;
    json[TAG_ISONLINE] = authResponseContext_->isOnline;
    json[TAG_AUTHED] = authResponseContext_->authed;
    json[TAG_DMVERSION] = authResponseContext_->dmVersion;
    json[TAG_HOST] = authResponseContext_->hostPkgName;
    json[TAG_BUNDLE_NAME] = authResponseContext_->bundleName;
    json[TAG_PEER_BUNDLE_NAME] = authResponseContext_->peerBundleName;
    json[TAG_TOKENID] = authResponseContext_->tokenId;
    json[TAG_IDENTICAL_ACCOUNT] = authResponseContext_->isIdenticalAccount;
    json[TAG_HAVE_CREDENTIAL] = authResponseContext_->haveCredential;
    json[TAG_HOST_PKGLABEL] = authResponseContext_->hostPkgLabel;
    json[TAG_EDITION] = authResponseContext_->edition;
    json[TAG_REMOTE_DEVICE_NAME] = authResponseContext_->remoteDeviceName;
}

void AuthMessageProcessor::CreateRespNegotiateMessage(nlohmann::json &json)
{
    if (cryptoAdapter_ == nullptr) {
        json[TAG_CRYPTO_SUPPORT] = false;
    } else {
        json[TAG_CRYPTO_SUPPORT] = true;
        json[TAG_CRYPTO_NAME] = cryptoAdapter_->GetName();
        json[TAG_CRYPTO_VERSION] = cryptoAdapter_->GetVersion();
        json[TAG_DEVICE_ID] = authResponseContext_->deviceId;
    }
    json[TAG_AUTH_TYPE] = authResponseContext_->authType;
    json[TAG_REPLY] = authResponseContext_->reply;
    json[TAG_LOCAL_DEVICE_ID] = authResponseContext_->localDeviceId;
    json[TAG_IS_AUTH_CODE_READY] = authResponseContext_->isAuthCodeReady;
    json[TAG_NET_ID] = authResponseContext_->networkId;

    json[TAG_LOCAL_ACCOUNTID] = authResponseContext_->localAccountId;
    json[TAG_LOCAL_USERID] = authResponseContext_->localUserId;
    json[TAG_TOKENID] = authResponseContext_->tokenId;
    json[TAG_ISONLINE] = authResponseContext_->isOnline;
    json[TAG_AUTHED] = authResponseContext_->authed;
    json[TAG_DMVERSION] = authResponseContext_->dmVersion;
    json[TAG_BIND_LEVEL] = authResponseContext_->bindLevel;
    json[TAG_IDENTICAL_ACCOUNT] = authResponseContext_->isIdenticalAccount;
    json[TAG_HAVE_CREDENTIAL] = authResponseContext_->haveCredential;
    json[TAG_BIND_TYPE_SIZE] = authResponseContext_->bindType.size();
    json[TAG_TARGET_DEVICE_NAME] = authResponseContext_->targetDeviceName;
    json[TAG_IMPORT_AUTH_CODE] = authResponseContext_->importAuthCode;
    for (uint32_t item = 0; item < authResponseContext_->bindType.size(); item++) {
        auto itemStr = std::to_string(item);
        json[itemStr] = authResponseContext_->bindType[item];
    }
}

void AuthMessageProcessor::CreateSyncGroupMessage(nlohmann::json &json)
{
    json[TAG_DEVICE_ID] = authRequestContext_->deviceId;
    json[TAG_GROUPIDS] = authRequestContext_->syncGroupList;
}

void AuthMessageProcessor::CreateResponseAuthMessage(nlohmann::json &json)
{
    json[TAG_REPLY] = authResponseContext_->reply;
    json[TAG_DEVICE_ID] = authResponseContext_->deviceId;
    json[TAG_TOKEN] = authResponseContext_->token;
    if (authResponseContext_->reply == 0) {
        std::string groupId = authResponseContext_->groupId;
        LOGI("AuthMessageProcessor::CreateResponseAuthMessage groupId %{public}s", GetAnonyString(groupId).c_str());
        nlohmann::json jsonObject = nlohmann::json::parse(groupId, nullptr, false);
        if (jsonObject.is_discarded()) {
            LOGE("DecodeRequestAuth jsonStr error");
            return;
        }
        if (!IsString(jsonObject, TAG_GROUP_ID)) {
            LOGE("err json string.");
            return;
        }
        groupId = jsonObject[TAG_GROUP_ID].get<std::string>();
        json[TAG_NET_ID] = authResponseContext_->networkId;
        json[TAG_REQUEST_ID] = authResponseContext_->requestId;
        json[TAG_GROUP_ID] = groupId;
        json[TAG_GROUP_NAME] = authResponseContext_->groupName;
        json[TAG_AUTH_TOKEN] = authResponseContext_->authToken;
        LOGI("AuthMessageProcessor::CreateResponseAuthMessage %{public}s, %{public}s", GetAnonyString(groupId).c_str(),
            GetAnonyString(authResponseContext_->groupName).c_str());
    }
}

void AuthMessageProcessor::CreateResponseFinishMessage(nlohmann::json &json)
{
    json[TAG_REPLY] = authResponseContext_->reply;
    json[TAG_AUTH_FINISH] = authResponseContext_->isFinish;
}

int32_t AuthMessageProcessor::ParseMessage(const std::string &message)
{
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("err json string, first time");
        return ERR_DM_FAILED;
    }
    int32_t msgType = jsonObject[TAG_MSG_TYPE].get<int32_t>();
    authResponseContext_->msgType = msgType;
    LOGI("AuthMessageProcessor::ParseMessage message type %{public}d", authResponseContext_->msgType);
    switch (msgType) {
        case MSG_TYPE_NEGOTIATE:
            ParseNegotiateMessage(jsonObject);
            break;
        case MSG_TYPE_RESP_NEGOTIATE:
            ParseRespNegotiateMessage(jsonObject);
            break;
        case MSG_TYPE_REQ_AUTH:
            return ParseAuthRequestMessage(jsonObject);
            break;
        case MSG_TYPE_RESP_AUTH:
            ParseAuthResponseMessage(jsonObject);
            break;
        case MSG_TYPE_RESP_AUTH_EXT:
            ParseAuthResponseMessageExt(jsonObject);
            break;
        case MSG_TYPE_REQ_AUTH_TERMINATE:
            ParseResponseFinishMessage(jsonObject);
            break;
        case MSG_TYPE_REQ_PUBLICKEY:
        case MSG_TYPE_RESP_PUBLICKEY:
            ParsePublicKeyMessageExt(jsonObject);
            break;
        case MSG_TYPE_REQ_RECHECK_MSG:
        case MSG_TYPE_RESP_RECHECK_MSG:
            ParseReqReCheckMessage(jsonObject);
            break;
        default:
            break;
    }
    return DM_OK;
}

void AuthMessageProcessor::ParsePublicKeyMessageExt(nlohmann::json &json)
{
    bool encryptFlag = false;
    {
        std::lock_guard<std::mutex> mutexLock(encryptFlagMutex_);
        encryptFlag = encryptFlag_;
    }
    if (!encryptFlag && IsString(json, TAG_PUBLICKEY)) {
        authResponseContext_->publicKey = json[TAG_PUBLICKEY].get<std::string>();
        return;
    }
    if (encryptFlag && IsString(json, TAG_CRYPTIC_MSG)) {
        std::string encryptStr = json[TAG_CRYPTIC_MSG].get<std::string>();
        std::string decryptStr = "";
        authResponseContext_->publicKey = "";
        CHECK_NULL_VOID(cryptoMgr_);
        if (cryptoMgr_->DecryptMessage(encryptStr, decryptStr) != DM_OK) {
            LOGE("DecryptMessage failed.");
            return;
        }
        nlohmann::json jsonObject = nlohmann::json::parse(decryptStr, nullptr, false);
        if (jsonObject.is_discarded()) {
            LOGE("DecodeRequestAuth jsonStr error");
            return;
        }
        if (IsString(jsonObject, TAG_PUBLICKEY)) {
            authResponseContext_->publicKey = jsonObject[TAG_PUBLICKEY].get<std::string>();
        }
        if (IsInt32(jsonObject, TAG_SESSIONKEY_ID)) {
            authResponseContext_->remoteSessionKeyId = jsonObject[TAG_SESSIONKEY_ID].get<int32_t>();
            LOGI("get remoteSessionKeyId");
        }
        return;
    }
}

void AuthMessageProcessor::ParseAuthResponseMessageExt(nlohmann::json &json)
{
    LOGI("start ParseAuthResponseMessageExt");
    if (IsInt32(json, TAG_REPLY)) {
        authResponseContext_->reply = json[TAG_REPLY].get<int32_t>();
    }
    if (IsString(json, TAG_TOKEN)) {
        authResponseContext_->token = json[TAG_TOKEN].get<std::string>();
    }
    if (IsInt32(json, TAG_CONFIRM_OPERATION)) {
        authResponseContext_->confirmOperation = json[TAG_CONFIRM_OPERATION].get<int32_t>();
    }
    if (IsInt64(json, TAG_REQUEST_ID)) {
        authResponseContext_->requestId = json[TAG_REQUEST_ID].get<int64_t>();
    }
}

void AuthMessageProcessor::ParseResponseFinishMessage(nlohmann::json &json)
{
    if (IsInt32(json, TAG_REPLY)) {
        authResponseContext_->reply = json[TAG_REPLY].get<int32_t>();
    }
    if (IsBool(json, TAG_AUTH_FINISH)) {
        authResponseContext_->isFinish = json[TAG_AUTH_FINISH].get<bool>();
    }
}

void AuthMessageProcessor::GetAuthReqMessage(nlohmann::json &json)
{
    authResponseContext_->localDeviceId = "";
    authResponseContext_->deviceId = "";
    if (IsInt32(json, TAG_AUTH_TYPE)) {
        authResponseContext_->authType = json[TAG_AUTH_TYPE].get<int32_t>();
    }
    if (IsString(json, TAG_TOKEN)) {
        authResponseContext_->token = json[TAG_TOKEN].get<std::string>();
    }
    if (IsString(json, TAG_DEVICE_ID)) {
        authResponseContext_->deviceId = json[TAG_DEVICE_ID].get<std::string>();
    }
    if (IsString(json, TAG_TARGET)) {
        authResponseContext_->targetPkgName = json[TAG_TARGET].get<std::string>();
    }
    if (IsString(json, TAG_LOCAL_DEVICE_ID)) {
        authResponseContext_->localDeviceId = json[TAG_LOCAL_DEVICE_ID].get<std::string>();
    }
    if (IsString(json, TAG_APP_OPERATION)) {
        authResponseContext_->appOperation = json[TAG_APP_OPERATION].get<std::string>();
    }
    if (IsString(json, TAG_CUSTOM_DESCRIPTION)) {
        authResponseContext_->customDesc = json[TAG_CUSTOM_DESCRIPTION].get<std::string>();
    }
    if (IsString(json, TAG_REQUESTER)) {
        authResponseContext_->deviceName = json[TAG_REQUESTER].get<std::string>();
    }
    if (IsInt32(json, TAG_LOCAL_DEVICE_TYPE)) {
        authResponseContext_->deviceTypeId = json[TAG_LOCAL_DEVICE_TYPE].get<int32_t>();
    } else {
        authResponseContext_->deviceTypeId = DmDeviceType::DEVICE_TYPE_UNKNOWN;
    }
}

int32_t AuthMessageProcessor::ParseAuthRequestMessage(nlohmann::json &json)
{
    LOGI("start ParseAuthRequestMessage");
    int32_t sliceNum = 0;
    int32_t idx = 0;
    if (!IsInt32(json, TAG_INDEX) || !IsInt32(json, TAG_SLICE_NUM)) {
        LOGE("AuthMessageProcessor::ParseAuthRequestMessage err json string, first.");
        return ERR_DM_FAILED;
    }
    idx = json[TAG_INDEX].get<int32_t>();
    sliceNum = json[TAG_SLICE_NUM].get<int32_t>();
    if (idx == 0) {
        GetAuthReqMessage(json);
        authResponseContext_->appThumbnail = "";
    }
    if (idx < sliceNum && IsString(json, TAG_APP_THUMBNAIL)) {
        std::string appSliceThumbnail = json[TAG_APP_THUMBNAIL].get<std::string>();
        authResponseContext_->appThumbnail = authResponseContext_->appThumbnail + appSliceThumbnail;
        return ERR_DM_AUTH_MESSAGE_INCOMPLETE;
    }
    if (IsBool(json, TAG_IS_SHOW_DIALOG)) {
        authResponseContext_->isShowDialog = json[TAG_IS_SHOW_DIALOG].get<bool>();
    } else {
        authResponseContext_->isShowDialog = true;
    }
    if (IsInt32(json, TAG_BIND_TYPE_SIZE)) {
        int32_t bindTypeSize = json[TAG_BIND_TYPE_SIZE].get<int32_t>();
        if (bindTypeSize > MAX_BINDTYPE_SIZE) {
            LOGE("ParseAuthRequestMessage bindTypeSize is over size.");
            return ERR_DM_FAILED;
        }
        authResponseContext_->bindType.clear();
        for (int32_t item = 0; item < bindTypeSize; item++) {
            std::string itemStr = std::to_string(item);
            if (IsInt32(json, itemStr)) {
                authResponseContext_->bindType.push_back(json[itemStr].get<int32_t>());
            }
        }
    }
    return DM_OK;
}

void AuthMessageProcessor::ParseAuthResponseMessage(nlohmann::json &json)
{
    LOGI("start ParseAuthResponseMessage");
    if (!IsInt32(json, TAG_REPLY)) {
        LOGE("AuthMessageProcessor::ParseAuthResponseMessage err json string, first time.");
        return;
    }
    authResponseContext_->reply = json[TAG_REPLY].get<int32_t>();
    if (IsString(json, TAG_DEVICE_ID)) {
        authResponseContext_->deviceId = json[TAG_DEVICE_ID].get<std::string>();
    }
    if (IsString(json, TAG_TOKEN)) {
        authResponseContext_->token = json[TAG_TOKEN].get<std::string>();
    }
    if (authResponseContext_->reply == 0) {
        if (!IsInt64(json, TAG_REQUEST_ID) || !IsString(json, TAG_GROUP_ID) ||
            !IsString(json, TAG_GROUP_NAME) || !IsString(json, TAG_AUTH_TOKEN)) {
            LOGE("AuthMessageProcessor::ParseAuthResponseMessage err json string, second time.");
            return;
        }
        authResponseContext_->requestId = json[TAG_REQUEST_ID].get<int64_t>();
        authResponseContext_->groupId = json[TAG_GROUP_ID].get<std::string>();
        authResponseContext_->groupName = json[TAG_GROUP_NAME].get<std::string>();
        authResponseContext_->authToken = json[TAG_AUTH_TOKEN].get<std::string>();
        if (IsString(json, TAG_NET_ID)) {
            authResponseContext_->networkId = json[TAG_NET_ID].get<std::string>();
        }
        LOGI("AuthMessageProcessor::ParseAuthResponseMessage groupId = %{public}s, groupName = %{public}s",
            GetAnonyString(authResponseContext_->groupId).c_str(),
            GetAnonyString(authResponseContext_->groupName).c_str());
    }
}

void AuthMessageProcessor::ParsePkgNegotiateMessage(const nlohmann::json &json)
{
    if (IsString(json, TAG_LOCAL_ACCOUNTID)) {
        authResponseContext_->localAccountId = json[TAG_LOCAL_ACCOUNTID].get<std::string>();
    }
    if (IsInt32(json, TAG_LOCAL_USERID)) {
        authResponseContext_->localUserId = json[TAG_LOCAL_USERID].get<int32_t>();
    }
    if (IsInt32(json, TAG_BIND_LEVEL)) {
        authResponseContext_->bindLevel = json[TAG_BIND_LEVEL].get<int32_t>();
    }
    if (IsBool(json, TAG_ISONLINE)) {
        authResponseContext_->isOnline = json[TAG_ISONLINE].get<bool>();
    }
    if (IsBool(json, TAG_IDENTICAL_ACCOUNT)) {
        authResponseContext_->isIdenticalAccount = json[TAG_IDENTICAL_ACCOUNT].get<bool>();
    }
    if (IsBool(json, TAG_AUTHED)) {
        authResponseContext_->authed = json[TAG_AUTHED].get<bool>();
    }
    if (IsInt64(json, TAG_TOKENID)) {
        authResponseContext_->remoteTokenId = json[TAG_TOKENID].get<int64_t>();
    }
    if (IsString(json, TAG_DMVERSION)) {
        authResponseContext_->dmVersion = json[TAG_DMVERSION].get<std::string>();
    } else {
        authResponseContext_->dmVersion = "3.2";
    }
    if (IsBool(json, TAG_HAVECREDENTIAL)) {
        authResponseContext_->haveCredential = json[TAG_HAVECREDENTIAL].get<bool>();
    }
    if (IsInt32(json, TAG_BIND_TYPE_SIZE)) {
        int32_t bindTypeSize = json[TAG_BIND_TYPE_SIZE].get<int32_t>();
        if (bindTypeSize > MAX_BINDTYPE_SIZE) {
            LOGE("ParsePkgNegotiateMessage bindTypeSize is over size.");
            return;
        }
        authResponseContext_->bindType.clear();
        for (int32_t item = 0; item < bindTypeSize; item++) {
            std::string itemStr = std::to_string(item);
            if (IsInt32(json, itemStr)) {
                authResponseContext_->bindType.push_back(json[itemStr].get<int32_t>());
            }
        }
    }
    if (IsString(json, TAG_HOST_PKGLABEL)) {
        authResponseContext_->hostPkgLabel = json[TAG_HOST_PKGLABEL].get<std::string>();
    }
}

void AuthMessageProcessor::ParseNegotiateMessage(const nlohmann::json &json)
{
    if (IsBool(json, TAG_CRYPTO_SUPPORT)) {
        authResponseContext_->cryptoSupport = json[TAG_CRYPTO_SUPPORT].get<bool>();
    }
    if (IsString(json, TAG_CRYPTO_NAME)) {
        authResponseContext_->cryptoName = json[TAG_CRYPTO_NAME].get<std::string>();
    }
    if (IsString(json, TAG_CRYPTO_VERSION)) {
        authResponseContext_->cryptoVer = json[TAG_CRYPTO_VERSION].get<std::string>();
    }
    if (IsString(json, TAG_DEVICE_ID)) {
        authResponseContext_->deviceId = json[TAG_DEVICE_ID].get<std::string>();
    }
    if (IsString(json, TAG_LOCAL_DEVICE_ID)) {
        authResponseContext_->localDeviceId = json[TAG_LOCAL_DEVICE_ID].get<std::string>();
    }
    if (IsInt32(json, TAG_AUTH_TYPE)) {
        authResponseContext_->authType = json[TAG_AUTH_TYPE].get<int32_t>();
    }
    if (IsInt32(json, TAG_REPLY)) {
        authResponseContext_->reply = json[TAG_REPLY].get<int32_t>();
    }
    if (IsString(json, TAG_ACCOUNT_GROUPID)) {
        authResponseContext_->accountGroupIdHash = json[TAG_ACCOUNT_GROUPID].get<std::string>();
    } else {
        authResponseContext_->accountGroupIdHash = OLD_VERSION_ACCOUNT;
    }
    if (IsString(json, TAG_HOST)) {
        authResponseContext_->hostPkgName = json[TAG_HOST].get<std::string>();
    }
    if (IsString(json, TAG_EDITION)) {
        authResponseContext_->edition = json[TAG_EDITION].get<std::string>();
    }
    if (IsString(json, TAG_BUNDLE_NAME)) {
        authResponseContext_->bundleName = json[TAG_BUNDLE_NAME].get<std::string>();
    }
    if (IsString(json, TAG_PEER_BUNDLE_NAME)) {
        authResponseContext_->peerBundleName = json[TAG_PEER_BUNDLE_NAME].get<std::string>();
    } else {
        authResponseContext_->peerBundleName = authResponseContext_->hostPkgName;
    }
    if (IsString(json, TAG_REMOTE_DEVICE_NAME)) {
        authResponseContext_->remoteDeviceName = json[TAG_REMOTE_DEVICE_NAME].get<std::string>();
    }
    ParsePkgNegotiateMessage(json);
}

void AuthMessageProcessor::ParseRespNegotiateMessage(const nlohmann::json &json)
{
    if (IsBool(json, TAG_IDENTICAL_ACCOUNT)) {
        authResponseContext_->isIdenticalAccount = json[TAG_IDENTICAL_ACCOUNT].get<bool>();
    }
    if (IsInt32(json, TAG_REPLY)) {
        authResponseContext_->reply = json[TAG_REPLY].get<int32_t>();
    }
    if (IsString(json, TAG_LOCAL_DEVICE_ID)) {
        authResponseContext_->localDeviceId = json[TAG_LOCAL_DEVICE_ID].get<std::string>();
    }
    if (IsBool(json, TAG_IS_AUTH_CODE_READY)) {
        authResponseContext_->isAuthCodeReady = json[TAG_IS_AUTH_CODE_READY].get<bool>();
    }
    if (IsString(json, TAG_ACCOUNT_GROUPID)) {
        authResponseContext_->accountGroupIdHash = json[TAG_ACCOUNT_GROUPID].get<std::string>();
    } else {
        authResponseContext_->accountGroupIdHash = OLD_VERSION_ACCOUNT;
    }
    if (IsString(json, TAG_NET_ID)) {
        authResponseContext_->networkId = json[TAG_NET_ID].get<std::string>();
    }
    if (IsString(json, TAG_TARGET_DEVICE_NAME)) {
        authResponseContext_->targetDeviceName = json[TAG_TARGET_DEVICE_NAME].get<std::string>();
    }
    if (IsString(json, TAG_IMPORT_AUTH_CODE)) {
        authResponseContext_->importAuthCode = json[TAG_IMPORT_AUTH_CODE].get<std::string>();
    }

    ParsePkgNegotiateMessage(json);
}

void AuthMessageProcessor::SetRequestContext(std::shared_ptr<DmAuthRequestContext> authRequestContext)
{
    authRequestContext_ = authRequestContext;
}

void AuthMessageProcessor::SetResponseContext(std::shared_ptr<DmAuthResponseContext> authResponseContext)
{
    authResponseContext_ = authResponseContext;
}

std::shared_ptr<DmAuthResponseContext> AuthMessageProcessor::GetResponseContext()
{
    return authResponseContext_;
}

std::shared_ptr<DmAuthRequestContext> AuthMessageProcessor::GetRequestContext()
{
    return authRequestContext_;
}

std::string AuthMessageProcessor::CreateDeviceAuthMessage(int32_t msgType, const uint8_t *data, uint32_t dataLen)
{
    LOGI("CreateDeviceAuthMessage start, msgType %{public}d.", msgType);
    nlohmann::json jsonObj;
    jsonObj[TAG_MSG_TYPE] = msgType;
    std::string authDataStr = std::string(reinterpret_cast<const char *>(data), dataLen);
    jsonObj[TAG_DATA] = authDataStr;
    jsonObj[TAG_DATA_LEN] = dataLen;
    return SafetyDump(jsonObj);
}

void AuthMessageProcessor::CreateReqReCheckMessage(nlohmann::json &jsonObj)
{
    nlohmann::json jsonTemp;
    jsonTemp[TAG_EDITION] = authResponseContext_->edition;
    jsonTemp[TAG_LOCAL_DEVICE_ID] = authResponseContext_->localDeviceId;
    jsonTemp[TAG_LOCAL_USERID] = authResponseContext_->localUserId;
    jsonTemp[TAG_BUNDLE_NAME] = authResponseContext_->bundleName;
    jsonTemp[TAG_BIND_LEVEL] = authResponseContext_->bindLevel;
    jsonTemp[TAG_LOCAL_ACCOUNTID] = authResponseContext_->localAccountId;
    jsonTemp[TAG_TOKENID] = authResponseContext_->tokenId;
    std::string strTemp = SafetyDump(jsonTemp);
    std::string encryptStr = "";
    CHECK_NULL_VOID(cryptoMgr_);
    if (cryptoMgr_->EncryptMessage(strTemp, encryptStr) != DM_OK) {
        LOGE("EncryptMessage failed.");
        return;
    }
    jsonObj[TAG_CRYPTIC_MSG] = encryptStr;
}

void AuthMessageProcessor::ParseReqReCheckMessage(nlohmann::json &json)
{
    std::string encryptStr = "";
    if (IsString(json, TAG_CRYPTIC_MSG)) {
        encryptStr = json[TAG_CRYPTIC_MSG].get<std::string>();
    }
    std::string decryptStr = "";
    authResponseContext_->edition = "";
    authResponseContext_->localDeviceId = "";
    authResponseContext_->localUserId = 0;
    authResponseContext_->bundleName = "";
    authResponseContext_->localBindLevel = -1;
    authResponseContext_->localAccountId = "";
    authResponseContext_->tokenId = 0;
    CHECK_NULL_VOID(cryptoMgr_);
    if (cryptoMgr_->DecryptMessage(encryptStr, decryptStr) != DM_OK) {
        LOGE("DecryptMessage failed.");
        return;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(decryptStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return;
    }
    if (IsString(jsonObject, TAG_EDITION)) {
        authResponseContext_->edition = jsonObject[TAG_EDITION].get<std::string>();
    }
    if (IsString(jsonObject, TAG_LOCAL_DEVICE_ID)) {
        authResponseContext_->localDeviceId = jsonObject[TAG_LOCAL_DEVICE_ID].get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_LOCAL_USERID)) {
        authResponseContext_->localUserId = jsonObject[TAG_LOCAL_USERID].get<int32_t>();
    }
    if (IsString(jsonObject, TAG_BUNDLE_NAME)) {
        authResponseContext_->bundleName = jsonObject[TAG_BUNDLE_NAME].get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_BIND_LEVEL)) {
        authResponseContext_->localBindLevel = jsonObject[TAG_BIND_LEVEL].get<int32_t>();
    }
    if (IsString(jsonObject, TAG_LOCAL_ACCOUNTID)) {
        authResponseContext_->localAccountId = jsonObject[TAG_LOCAL_ACCOUNTID].get<std::string>();
    }
    if (IsInt64(jsonObject, TAG_TOKENID)) {
        authResponseContext_->tokenId = jsonObject[TAG_TOKENID].get<int64_t>();
    }
}

int32_t AuthMessageProcessor::SaveSessionKey(const uint8_t *sessionKey, const uint32_t keyLen)
{
    CHECK_NULL_RETURN(cryptoMgr_, ERR_DM_POINT_NULL);
    return cryptoMgr_->SaveSessionKey(sessionKey, keyLen);
}

void AuthMessageProcessor::SetEncryptFlag(bool flag)
{
    std::lock_guard<std::mutex> mutexLock(encryptFlagMutex_);
    encryptFlag_ = flag;
}

int32_t AuthMessageProcessor::ProcessSessionKey(const uint8_t *sessionKey, const uint32_t keyLen)
{
    CHECK_NULL_RETURN(cryptoMgr_, ERR_DM_POINT_NULL);
    return cryptoMgr_->ProcessSessionKey(sessionKey, keyLen);
}
} // namespace DistributedHardware
} // namespace OHOS