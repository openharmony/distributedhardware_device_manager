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
const char* TAG_NET_ID = "NETID";
const char* TAG_TARGET = "TARGET";
const char* TAG_APP_OPERATION = "APPOPERATION";
const char* TAG_APP_NAME = "APPNAME";
const char* TAG_APP_DESCRIPTION = "APPDESC";
const char* TAG_GROUPIDS = "GROUPIDLIST";
const char* TAG_CUSTOM_DESCRIPTION = "CUSTOMDESC";
const char* TAG_DEVICE_TYPE = "DEVICETYPE";
const char* TAG_REQUESTER = "REQUESTER";
const char* TAG_LOCAL_DEVICE_TYPE = "LOCALDEVICETYPE";
const char* TAG_INDEX = "INDEX";
const char* TAG_SLICE_NUM = "SLICE";
const char* TAG_IS_AUTH_CODE_READY = "IS_AUTH_CODE_READY";
const char* TAG_IS_SHOW_DIALOG = "IS_SHOW_DIALOG";
const char* TAG_TOKEN = "TOKEN";
const char* TAG_CRYPTO_NAME = "CRYPTONAME";
const char* TAG_CRYPTO_VERSION = "CRYPTOVERSION";
const char* QR_CODE_KEY = "qrCode";
const char* TAG_AUTH_TOKEN = "authToken";
const char* NFC_CODE_KEY = "nfcCode";
const char* OLD_VERSION_ACCOUNT = "oldVersionAccount";

const char* TAG_PUBLICKEY = "publicKey";
const char* TAG_SESSIONKEY = "sessionKey";
const char* TAG_BIND_TYPE_SIZE = "bindTypeSize";
const char* TAG_HOST_PKGNAME = "hostPkgname";
const char* TAG_HAVECREDENTIAL = "haveCredential";
const char* TAG_CONFIRM_OPERATION = "confirmOperation";
const char* TAG_IMPORT_AUTH_CODE = "IMPORT_AUTH_CODE";
const char* TAG_CRYPTIC_MSG = "encryptMsg";
const char* TAG_SESSIONKEY_ID = "sessionKeyId";

const int32_t MSG_MAX_SIZE = 45 * 1024;
const int32_t GROUP_VISIBILITY_IS_PRIVATE = 0;
const int32_t MAX_BINDTYPE_SIZE = 1000;
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

void AuthMessageProcessor::GetJsonObj(JsonObject &jsonObj)
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
    LOGI("start.");
    std::vector<std::string> jsonStrVec;
    if (authRequestContext_ == nullptr) {
        LOGE("AuthMessageProcessor::CreateAuthRequestMessage authRequestContext_ is nullptr.");
        return jsonStrVec;
    }
    int32_t thumbnailSize = (int32_t)(authRequestContext_->appThumbnail.size());
    int32_t thumbnailSlice = ((thumbnailSize / MSG_MAX_SIZE) + (thumbnailSize % MSG_MAX_SIZE) == 0 ? 0 : 1);
    JsonObject jsonObj;
    jsonObj[TAG_SLICE_NUM] = thumbnailSlice + 1;
    jsonObj[TAG_THUMBNAIL_SIZE] = thumbnailSize;
    GetJsonObj(jsonObj);
    jsonStrVec.push_back(jsonObj.Dump());
    for (int32_t idx = 0; idx < thumbnailSlice; idx++) {
        JsonObject jsonThumbnailObj;
        jsonThumbnailObj[TAG_VER] = DM_ITF_VER;
        jsonThumbnailObj[TAG_MSG_TYPE] = MSG_TYPE_REQ_AUTH;
        jsonThumbnailObj[TAG_SLICE_NUM] = thumbnailSlice + 1;
        jsonThumbnailObj[TAG_INDEX] = idx + 1;
        jsonThumbnailObj[TAG_DEVICE_ID] = authRequestContext_->deviceId;
        jsonThumbnailObj[TAG_THUMBNAIL_SIZE] = thumbnailSize;
        int32_t leftLen = thumbnailSize - idx * MSG_MAX_SIZE;
        int32_t sliceLen = (leftLen > MSG_MAX_SIZE) ? MSG_MAX_SIZE : leftLen;
        jsonObj[TAG_APP_THUMBNAIL] = authRequestContext_->appThumbnail.substr(idx * MSG_MAX_SIZE, sliceLen);
        jsonStrVec.push_back(jsonThumbnailObj.Dump());
    }
    return jsonStrVec;
}

std::string AuthMessageProcessor::CreateSimpleMessage(int32_t msgType)
{
    LOGI("start. msgType is %{public}d", msgType);
    JsonObject jsonObj;
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
    return jsonObj.Dump();
}

void AuthMessageProcessor::CreatePublicKeyMessageExt(JsonObject &json)
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
        JsonObject jsonTemp;
        auto sptr = authMgr_.lock();
        if (sptr != nullptr && !sptr->IsSrc()) {
            authResponseContext_->localSessionKeyId = sptr->GetSessionKeyIdSync(authResponseContext_->requestId);
            jsonTemp[TAG_SESSIONKEY_ID] = authResponseContext_->localSessionKeyId;
        }
        jsonTemp[TAG_PUBLICKEY] = authResponseContext_->publicKey;
        std::string strTemp = jsonTemp.Dump();
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

void AuthMessageProcessor::CreateResponseAuthMessageExt(JsonObject &json)
{
    json[TAG_REPLY] = authResponseContext_->reply;
    json[TAG_TOKEN] = authResponseContext_->token;
    json[TAG_CONFIRM_OPERATION] = authResponseContext_->confirmOperation;
    json[TAG_REQUEST_ID] = authResponseContext_->requestId;
}

void AuthMessageProcessor::CreateNegotiateMessage(JsonObject &json)
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

void AuthMessageProcessor::CreateRespNegotiateMessage(JsonObject &json)
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

void AuthMessageProcessor::CreateSyncGroupMessage(JsonObject &json)
{
    json[TAG_DEVICE_ID] = authRequestContext_->deviceId;
    json[TAG_GROUPIDS] = authRequestContext_->syncGroupList;
}

void AuthMessageProcessor::CreateResponseAuthMessage(JsonObject &json)
{
    json[TAG_REPLY] = authResponseContext_->reply;
    json[TAG_DEVICE_ID] = authResponseContext_->deviceId;
    json[TAG_TOKEN] = authResponseContext_->token;
    if (authResponseContext_->reply == 0) {
        std::string groupId = authResponseContext_->groupId;
        LOGI("groupId %{public}s", GetAnonyString(groupId).c_str());
        JsonObject jsonObject(groupId);
        if (jsonObject.IsDiscarded()) {
            LOGE("DecodeRequestAuth jsonStr error");
            return;
        }
        if (!IsString(jsonObject, TAG_GROUP_ID)) {
            LOGE("err json string.");
            return;
        }
        groupId = jsonObject[TAG_GROUP_ID].Get<std::string>();
        json[TAG_NET_ID] = authResponseContext_->networkId;
        json[TAG_REQUEST_ID] = authResponseContext_->requestId;
        json[TAG_GROUP_ID] = groupId;
        json[TAG_GROUP_NAME] = authResponseContext_->groupName;
        json[TAG_AUTH_TOKEN] = authResponseContext_->authToken;
        LOGI("AuthMessageProcessor::CreateResponseAuthMessage %{public}s, %{public}s", GetAnonyString(groupId).c_str(),
            GetAnonyString(authResponseContext_->groupName).c_str());
    }
}

void AuthMessageProcessor::CreateResponseFinishMessage(JsonObject &json)
{
    json[TAG_REPLY] = authResponseContext_->reply;
    json[TAG_AUTH_FINISH] = authResponseContext_->isFinish;
}

int32_t AuthMessageProcessor::ParseMessage(const std::string &message)
{
    JsonObject jsonObject(message);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("err json string, first time");
        return ERR_DM_FAILED;
    }
    int32_t msgType = jsonObject[TAG_MSG_TYPE].Get<int32_t>();
    authResponseContext_->msgType = msgType;
    LOGI("message type %{public}d", authResponseContext_->msgType);
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

void AuthMessageProcessor::ParsePublicKeyMessageExt(JsonObject &json)
{
    bool encryptFlag = false;
    {
        std::lock_guard<std::mutex> mutexLock(encryptFlagMutex_);
        encryptFlag = encryptFlag_;
    }
    if (!encryptFlag && IsString(json, TAG_PUBLICKEY)) {
        authResponseContext_->publicKey = json[TAG_PUBLICKEY].Get<std::string>();
        return;
    }
    if (encryptFlag && IsString(json, TAG_CRYPTIC_MSG)) {
        std::string encryptStr = json[TAG_CRYPTIC_MSG].Get<std::string>();
        std::string decryptStr = "";
        authResponseContext_->publicKey = "";
        CHECK_NULL_VOID(cryptoMgr_);
        if (cryptoMgr_->DecryptMessage(encryptStr, decryptStr) != DM_OK) {
            LOGE("DecryptMessage failed.");
            return;
        }
        JsonObject jsonObject(decryptStr);
        if (jsonObject.IsDiscarded()) {
            LOGE("DecodeRequestAuth jsonStr error");
            return;
        }
        if (IsString(jsonObject, TAG_PUBLICKEY)) {
            authResponseContext_->publicKey = jsonObject[TAG_PUBLICKEY].Get<std::string>();
        }
        if (IsInt32(jsonObject, TAG_SESSIONKEY_ID)) {
            authResponseContext_->remoteSessionKeyId = jsonObject[TAG_SESSIONKEY_ID].Get<int32_t>();
        }
        return;
    }
}

void AuthMessageProcessor::ParseAuthResponseMessageExt(JsonObject &json)
{
    LOGI("start");
    if (IsInt32(json, TAG_REPLY)) {
        authResponseContext_->reply = json[TAG_REPLY].Get<int32_t>();
    }
    if (IsString(json, TAG_TOKEN)) {
        authResponseContext_->token = json[TAG_TOKEN].Get<std::string>();
    }
    if (IsInt32(json, TAG_CONFIRM_OPERATION)) {
        authResponseContext_->confirmOperation = json[TAG_CONFIRM_OPERATION].Get<int32_t>();
    }
    if (IsInt64(json, TAG_REQUEST_ID)) {
        authResponseContext_->requestId = json[TAG_REQUEST_ID].Get<int64_t>();
    }
}

void AuthMessageProcessor::ParseResponseFinishMessage(JsonObject &json)
{
    if (IsInt32(json, TAG_REPLY)) {
        authResponseContext_->reply = json[TAG_REPLY].Get<int32_t>();
    }
    if (IsBool(json, TAG_AUTH_FINISH)) {
        authResponseContext_->isFinish = json[TAG_AUTH_FINISH].Get<bool>();
    }
}

void AuthMessageProcessor::GetAuthReqMessage(JsonObject &json)
{
    authResponseContext_->localDeviceId = "";
    authResponseContext_->deviceId = "";
    if (IsInt32(json, TAG_AUTH_TYPE)) {
        authResponseContext_->authType = json[TAG_AUTH_TYPE].Get<int32_t>();
    }
    if (IsString(json, TAG_TOKEN)) {
        authResponseContext_->token = json[TAG_TOKEN].Get<std::string>();
    }
    if (IsString(json, TAG_DEVICE_ID)) {
        authResponseContext_->deviceId = json[TAG_DEVICE_ID].Get<std::string>();
    }
    if (IsString(json, TAG_TARGET)) {
        authResponseContext_->targetPkgName = json[TAG_TARGET].Get<std::string>();
    }
    if (IsString(json, TAG_LOCAL_DEVICE_ID)) {
        authResponseContext_->localDeviceId = json[TAG_LOCAL_DEVICE_ID].Get<std::string>();
    }
    if (IsString(json, TAG_APP_OPERATION)) {
        authResponseContext_->appOperation = json[TAG_APP_OPERATION].Get<std::string>();
    }
    if (IsString(json, TAG_CUSTOM_DESCRIPTION)) {
        authResponseContext_->customDesc = json[TAG_CUSTOM_DESCRIPTION].Get<std::string>();
    }
    if (IsString(json, TAG_REQUESTER)) {
        authResponseContext_->deviceName = json[TAG_REQUESTER].Get<std::string>();
    }
    if (IsInt32(json, TAG_LOCAL_DEVICE_TYPE)) {
        authResponseContext_->deviceTypeId = json[TAG_LOCAL_DEVICE_TYPE].Get<int32_t>();
    } else {
        authResponseContext_->deviceTypeId = DmDeviceType::DEVICE_TYPE_UNKNOWN;
    }
}

int32_t AuthMessageProcessor::ParseAuthRequestMessage(JsonObject &json)
{
    LOGI("start");
    int32_t sliceNum = 0;
    int32_t idx = 0;
    if (!IsInt32(json, TAG_INDEX) || !IsInt32(json, TAG_SLICE_NUM)) {
        LOGE("AuthMessageProcessor::ParseAuthRequestMessage err json string, first.");
        return ERR_DM_FAILED;
    }
    idx = json[TAG_INDEX].Get<int32_t>();
    sliceNum = json[TAG_SLICE_NUM].Get<int32_t>();
    if (idx == 0) {
        GetAuthReqMessage(json);
        authResponseContext_->appThumbnail = "";
    }
    if (idx < sliceNum && IsString(json, TAG_APP_THUMBNAIL)) {
        std::string appSliceThumbnail = json[TAG_APP_THUMBNAIL].Get<std::string>();
        authResponseContext_->appThumbnail = authResponseContext_->appThumbnail + appSliceThumbnail;
        return ERR_DM_AUTH_MESSAGE_INCOMPLETE;
    }
    if (IsBool(json, TAG_IS_SHOW_DIALOG)) {
        authResponseContext_->isShowDialog = json[TAG_IS_SHOW_DIALOG].Get<bool>();
    } else {
        authResponseContext_->isShowDialog = true;
    }
    if (IsInt32(json, TAG_BIND_TYPE_SIZE)) {
        int32_t bindTypeSize = json[TAG_BIND_TYPE_SIZE].Get<int32_t>();
        if (bindTypeSize > MAX_BINDTYPE_SIZE) {
            LOGE("ParseAuthRequestMessage bindTypeSize is over size.");
            return ERR_DM_FAILED;
        }
        authResponseContext_->bindType.clear();
        for (int32_t item = 0; item < bindTypeSize; item++) {
            std::string itemStr = std::to_string(item);
            if (IsInt32(json, itemStr)) {
                authResponseContext_->bindType.push_back(json[itemStr].Get<int32_t>());
            }
        }
    }
    return DM_OK;
}

void AuthMessageProcessor::ParseAuthResponseMessage(JsonObject &json)
{
    LOGI("start");
    if (!IsInt32(json, TAG_REPLY)) {
        LOGE("AuthMessageProcessor::ParseAuthResponseMessage err json string, first time.");
        return;
    }
    authResponseContext_->reply = json[TAG_REPLY].Get<int32_t>();
    if (IsString(json, TAG_DEVICE_ID)) {
        authResponseContext_->deviceId = json[TAG_DEVICE_ID].Get<std::string>();
    }
    if (IsString(json, TAG_TOKEN)) {
        authResponseContext_->token = json[TAG_TOKEN].Get<std::string>();
    }
    if (authResponseContext_->reply == 0) {
        if (!IsInt64(json, TAG_REQUEST_ID) || !IsString(json, TAG_GROUP_ID) ||
            !IsString(json, TAG_GROUP_NAME) || !IsString(json, TAG_AUTH_TOKEN)) {
            LOGE("AuthMessageProcessor::ParseAuthResponseMessage err json string, second time.");
            return;
        }
        authResponseContext_->requestId = json[TAG_REQUEST_ID].Get<int64_t>();
        authResponseContext_->groupId = json[TAG_GROUP_ID].Get<std::string>();
        authResponseContext_->groupName = json[TAG_GROUP_NAME].Get<std::string>();
        authResponseContext_->authToken = json[TAG_AUTH_TOKEN].Get<std::string>();
        if (IsString(json, TAG_NET_ID)) {
            authResponseContext_->networkId = json[TAG_NET_ID].Get<std::string>();
        }
        LOGI("AuthMessageProcessor::ParseAuthResponseMessage groupId = %{public}s, groupName = %{public}s",
            GetAnonyString(authResponseContext_->groupId).c_str(),
            GetAnonyString(authResponseContext_->groupName).c_str());
    }
}

void AuthMessageProcessor::ParsePkgNegotiateMessage(const JsonObject &json)
{
    if (IsString(json, TAG_LOCAL_ACCOUNTID)) {
        authResponseContext_->localAccountId = json[TAG_LOCAL_ACCOUNTID].Get<std::string>();
    }
    if (IsInt32(json, TAG_LOCAL_USERID)) {
        authResponseContext_->localUserId = json[TAG_LOCAL_USERID].Get<int32_t>();
    }
    if (IsInt32(json, TAG_BIND_LEVEL)) {
        authResponseContext_->bindLevel = json[TAG_BIND_LEVEL].Get<int32_t>();
    }
    if (IsBool(json, TAG_ISONLINE)) {
        authResponseContext_->isOnline = json[TAG_ISONLINE].Get<bool>();
    }
    if (IsBool(json, TAG_IDENTICAL_ACCOUNT)) {
        authResponseContext_->isIdenticalAccount = json[TAG_IDENTICAL_ACCOUNT].Get<bool>();
    }
    if (IsBool(json, TAG_AUTHED)) {
        authResponseContext_->authed = json[TAG_AUTHED].Get<bool>();
    }
    if (IsInt64(json, TAG_TOKENID)) {
        authResponseContext_->remoteTokenId = json[TAG_TOKENID].Get<int64_t>();
    }
    if (IsString(json, TAG_DMVERSION)) {
        authResponseContext_->dmVersion = json[TAG_DMVERSION].Get<std::string>();
    } else {
        authResponseContext_->dmVersion = "3.2";
    }
    if (IsBool(json, TAG_HAVECREDENTIAL)) {
        authResponseContext_->haveCredential = json[TAG_HAVECREDENTIAL].Get<bool>();
    }
    if (IsInt32(json, TAG_BIND_TYPE_SIZE)) {
        int32_t bindTypeSize = json[TAG_BIND_TYPE_SIZE].Get<int32_t>();
        if (bindTypeSize > MAX_BINDTYPE_SIZE) {
            LOGE("ParsePkgNegotiateMessage bindTypeSize is over size.");
            return;
        }
        authResponseContext_->bindType.clear();
        for (int32_t item = 0; item < bindTypeSize; item++) {
            std::string itemStr = std::to_string(item);
            if (IsInt32(json, itemStr)) {
                authResponseContext_->bindType.push_back(json[itemStr].Get<int32_t>());
            }
        }
    }
    if (IsString(json, TAG_HOST_PKGLABEL)) {
        authResponseContext_->hostPkgLabel = json[TAG_HOST_PKGLABEL].Get<std::string>();
    }
}

void AuthMessageProcessor::ParseNegotiateMessage(const JsonObject &json)
{
    if (IsBool(json, TAG_CRYPTO_SUPPORT)) {
        authResponseContext_->cryptoSupport = json[TAG_CRYPTO_SUPPORT].Get<bool>();
    }
    if (IsString(json, TAG_CRYPTO_NAME)) {
        authResponseContext_->cryptoName = json[TAG_CRYPTO_NAME].Get<std::string>();
    }
    if (IsString(json, TAG_CRYPTO_VERSION)) {
        authResponseContext_->cryptoVer = json[TAG_CRYPTO_VERSION].Get<std::string>();
    }
    if (IsString(json, TAG_DEVICE_ID)) {
        authResponseContext_->deviceId = json[TAG_DEVICE_ID].Get<std::string>();
    }
    if (IsString(json, TAG_LOCAL_DEVICE_ID)) {
        authResponseContext_->localDeviceId = json[TAG_LOCAL_DEVICE_ID].Get<std::string>();
    }
    if (IsInt32(json, TAG_AUTH_TYPE)) {
        authResponseContext_->authType = json[TAG_AUTH_TYPE].Get<int32_t>();
    }
    if (IsInt32(json, TAG_REPLY)) {
        authResponseContext_->reply = json[TAG_REPLY].Get<int32_t>();
    }
    if (IsString(json, TAG_ACCOUNT_GROUPID)) {
        authResponseContext_->accountGroupIdHash = json[TAG_ACCOUNT_GROUPID].Get<std::string>();
    } else {
        authResponseContext_->accountGroupIdHash = OLD_VERSION_ACCOUNT;
    }
    if (IsString(json, TAG_HOST)) {
        authResponseContext_->hostPkgName = json[TAG_HOST].Get<std::string>();
    }
    if (IsString(json, TAG_EDITION)) {
        authResponseContext_->edition = json[TAG_EDITION].Get<std::string>();
    }
    if (IsString(json, TAG_BUNDLE_NAME)) {
        authResponseContext_->bundleName = json[TAG_BUNDLE_NAME].Get<std::string>();
    }
    if (IsString(json, TAG_PEER_BUNDLE_NAME)) {
        authResponseContext_->peerBundleName = json[TAG_PEER_BUNDLE_NAME].Get<std::string>();
    } else {
        authResponseContext_->peerBundleName = authResponseContext_->hostPkgName;
    }
    if (IsString(json, TAG_REMOTE_DEVICE_NAME)) {
        authResponseContext_->remoteDeviceName = json[TAG_REMOTE_DEVICE_NAME].Get<std::string>();
    }
    ParsePkgNegotiateMessage(json);
}

void AuthMessageProcessor::ParseRespNegotiateMessage(const JsonObject &json)
{
    if (IsBool(json, TAG_IDENTICAL_ACCOUNT)) {
        authResponseContext_->isIdenticalAccount = json[TAG_IDENTICAL_ACCOUNT].Get<bool>();
    }
    if (IsInt32(json, TAG_REPLY)) {
        authResponseContext_->reply = json[TAG_REPLY].Get<int32_t>();
    }
    if (IsString(json, TAG_LOCAL_DEVICE_ID)) {
        authResponseContext_->localDeviceId = json[TAG_LOCAL_DEVICE_ID].Get<std::string>();
    }
    if (IsBool(json, TAG_IS_AUTH_CODE_READY)) {
        authResponseContext_->isAuthCodeReady = json[TAG_IS_AUTH_CODE_READY].Get<bool>();
    }
    if (IsString(json, TAG_ACCOUNT_GROUPID)) {
        authResponseContext_->accountGroupIdHash = json[TAG_ACCOUNT_GROUPID].Get<std::string>();
    } else {
        authResponseContext_->accountGroupIdHash = OLD_VERSION_ACCOUNT;
    }
    if (IsString(json, TAG_NET_ID)) {
        authResponseContext_->networkId = json[TAG_NET_ID].Get<std::string>();
    }
    if (IsString(json, TAG_TARGET_DEVICE_NAME)) {
        authResponseContext_->targetDeviceName = json[TAG_TARGET_DEVICE_NAME].Get<std::string>();
    }
    if (IsString(json, TAG_IMPORT_AUTH_CODE)) {
        authResponseContext_->importAuthCode = json[TAG_IMPORT_AUTH_CODE].Get<std::string>();
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
    LOGI("start, msgType %{public}d.", msgType);
    JsonObject jsonObj;
    jsonObj[TAG_MSG_TYPE] = msgType;
    std::string authDataStr = std::string(reinterpret_cast<const char *>(data), dataLen);
    jsonObj[TAG_DATA] = authDataStr;
    jsonObj[TAG_DATA_LEN] = dataLen;
    return jsonObj.Dump();
}

void AuthMessageProcessor::CreateReqReCheckMessage(JsonObject &jsonObj)
{
    JsonObject jsonTemp;
    jsonTemp[TAG_EDITION] = authResponseContext_->edition;
    jsonTemp[TAG_LOCAL_DEVICE_ID] = authResponseContext_->localDeviceId;
    jsonTemp[TAG_LOCAL_USERID] = authResponseContext_->localUserId;
    jsonTemp[TAG_BUNDLE_NAME] = authResponseContext_->bundleName;
    jsonTemp[TAG_BIND_LEVEL] = authResponseContext_->bindLevel;
    jsonTemp[TAG_LOCAL_ACCOUNTID] = authResponseContext_->localAccountId;
    jsonTemp[TAG_TOKENID] = authResponseContext_->tokenId;
    std::string strTemp = jsonTemp.Dump();
    std::string encryptStr = "";
    CHECK_NULL_VOID(cryptoMgr_);
    if (cryptoMgr_->EncryptMessage(strTemp, encryptStr) != DM_OK) {
        LOGE("EncryptMessage failed.");
        return;
    }
    jsonObj[TAG_CRYPTIC_MSG] = encryptStr;
}

void AuthMessageProcessor::ParseReqReCheckMessage(JsonObject &json)
{
    std::string encryptStr = "";
    if (IsString(json, TAG_CRYPTIC_MSG)) {
        encryptStr = json[TAG_CRYPTIC_MSG].Get<std::string>();
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
    JsonObject jsonObject(decryptStr);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return;
    }
    if (IsString(jsonObject, TAG_EDITION)) {
        authResponseContext_->edition = jsonObject[TAG_EDITION].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_LOCAL_DEVICE_ID)) {
        authResponseContext_->localDeviceId = jsonObject[TAG_LOCAL_DEVICE_ID].Get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_LOCAL_USERID)) {
        authResponseContext_->localUserId = jsonObject[TAG_LOCAL_USERID].Get<int32_t>();
    }
    if (IsString(jsonObject, TAG_BUNDLE_NAME)) {
        authResponseContext_->bundleName = jsonObject[TAG_BUNDLE_NAME].Get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_BIND_LEVEL)) {
        authResponseContext_->localBindLevel = jsonObject[TAG_BIND_LEVEL].Get<int32_t>();
    }
    if (IsString(jsonObject, TAG_LOCAL_ACCOUNTID)) {
        authResponseContext_->localAccountId = jsonObject[TAG_LOCAL_ACCOUNTID].Get<std::string>();
    }
    if (IsInt64(jsonObject, TAG_TOKENID)) {
        authResponseContext_->tokenId = jsonObject[TAG_TOKENID].Get<int64_t>();
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