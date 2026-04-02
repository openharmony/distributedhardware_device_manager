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
#include "dm_auth_message_processor_3rd.h"

#include <iostream>
#include <sstream>
#include <zlib.h>
#include <mbedtls/base64.h>

#include "access_control_profile.h"
#include "distributed_device_profile_client.h"
#include "service_info_unique_key.h"
#include "parameter.h"

#include "deviceprofile_connector_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_context_3rd.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_state_machine_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"

namespace OHOS {
namespace DistributedHardware {
const char* TAG_COMPRESS_ORI_LEN = "compressOriLen";
const char* TAG_COMPRESS = "compressMsg";
const char* TAG_TRANSMIT_SK_ID = "accessAppSKId";
const char* TAG_TRANSMIT_SK_TIMESTAMP = "accessAppSKTimeStamp";
constexpr uint32_t MAX_MESSAGE_LENGTH = 64 * 1024 * 1024;
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;

DmAuthMessageProcessor3rd::DmAuthMessageProcessor3rd()
{
    LOGI("DmAuthMessageProcessor3rd constructor");
    cryptoMgr_ = std::make_shared<CryptoMgr3rd>();
    ConstructCreateMessageFuncMap();
    ConstructParseMessageFuncMap();
}

DmAuthMessageProcessor3rd::~DmAuthMessageProcessor3rd()
{
    if (cryptoMgr_ != nullptr) {
        cryptoMgr_->ClearSessionKey();
        cryptoMgr_ = nullptr;
    }
}

bool IsMessageValid(const JsonItemObject &jsonObject)
{
    if (jsonObject.IsDiscarded()) {
        LOGE("DmAuthMessageProcessor3rd::ParseMessage failed, decodeRequestAuth jsonStr error");
        return false;
    }
    if (!jsonObject[TAG_MSG_TYPE].IsNumberInteger()) {
        LOGE("DmAuthMessageProcessor3rd::ParseMessage failed, message type error.");
        return false;
    }
    return true;
}

void DmAuthMessageProcessor3rd::ConstructCreateMessageFuncMap()
{
    createMessageFuncMap_ = {
        {DmMessageType::ACL_REQ_NEGOTIATE, &DmAuthMessageProcessor3rd::CreateMessage2010},
        {DmMessageType::ACL_RESP_NEGOTIATE, &DmAuthMessageProcessor3rd::CreateMessage2020},
        {DmMessageType::ACL_REQ_PIN_AUTH_START, &DmAuthMessageProcessor3rd::CreateMessage2030},
        {DmMessageType::ACL_RESP_PIN_AUTH_START, &DmAuthMessageProcessor3rd::CreateMessage2040},
        {DmMessageType::ACL_REQ_PIN_AUTH_MSG_NEGOTIATE, &DmAuthMessageProcessor3rd::CreateMessage2031},
        {DmMessageType::ACL_RESP_PIN_AUTH_MSG_NEGOTIATE, &DmAuthMessageProcessor3rd::CreateMessage2041},
        {DmMessageType::ACL_REQ_DATA_SYNC, &DmAuthMessageProcessor3rd::CreateMessage2050},
        {DmMessageType::ACL_RESP_DATA_SYNC, &DmAuthMessageProcessor3rd::CreateMessage2060},
        {DmMessageType::ACL_REQ_FINISH, &DmAuthMessageProcessor3rd::CreateMessage2070},
        {DmMessageType::ACL_RESP_FINISH, &DmAuthMessageProcessor3rd::CreateMessage2080},
    };
}

void DmAuthMessageProcessor3rd::ConstructParseMessageFuncMap()
{
    parseMessageFuncMap_ = {
        {DmMessageType::ACL_REQ_NEGOTIATE, &DmAuthMessageProcessor3rd::ParseMessage2010},
        {DmMessageType::ACL_RESP_NEGOTIATE, &DmAuthMessageProcessor3rd::ParseMessage2020},
        {DmMessageType::ACL_REQ_PIN_AUTH_START, &DmAuthMessageProcessor3rd::ParseMessage2030},
        {DmMessageType::ACL_RESP_PIN_AUTH_START, &DmAuthMessageProcessor3rd::ParseMessage2040},
        {DmMessageType::ACL_REQ_PIN_AUTH_MSG_NEGOTIATE, &DmAuthMessageProcessor3rd::ParseMessage2031},
        {DmMessageType::ACL_RESP_PIN_AUTH_MSG_NEGOTIATE, &DmAuthMessageProcessor3rd::ParseMessage2041},
        {DmMessageType::ACL_REQ_DATA_SYNC, &DmAuthMessageProcessor3rd::ParseMessage2050},
        {DmMessageType::ACL_RESP_DATA_SYNC, &DmAuthMessageProcessor3rd::ParseMessage2060},
        {DmMessageType::ACL_REQ_FINISH, &DmAuthMessageProcessor3rd::ParseMessage2070},
        {DmMessageType::ACL_RESP_FINISH, &DmAuthMessageProcessor3rd::ParseMessage2080},
    };
}

int32_t DmAuthMessageProcessor3rd::ParseMessage(std::shared_ptr<DmAuthContext> context, const std::string &message)
{
    JsonObject jsonObject(message);
    if (context == nullptr || !IsMessageValid(jsonObject)) {
        return ERR_DM_FAILED;
    }
    DmMessageType msgType = static_cast<DmMessageType>(jsonObject[TAG_MSG_TYPE].Get<int32_t>());
    context->msgType = msgType;
    LOGI("DmAuthMessageProcessor3rd::ParseMessage message type %{public}d", context->msgType);
    auto itr = parseMessageFuncMap_.find(msgType);
    if (itr == parseMessageFuncMap_.end()) {
        LOGI("DmAuthMessageProcessor3rd::ParseMessage message type error %{public}d", context->msgType);
        return ERR_DM_FAILED;
    }
    return (this->*(itr->second))(jsonObject, context);
}

std::string DmAuthMessageProcessor3rd::CreateMessage(DmMessageType msgType, std::shared_ptr<DmAuthContext> context)
{
    LOGI("DmAuthMessageProcessor3rd::CreateMessage start. msgType is %{public}d", msgType);
    JsonObject jsonObj;
    jsonObj[TAG_MSG_TYPE] = msgType;
    jsonObj[DM_TAG_LOGICAL_SESSION_ID] = context->logicalSessionId;
    auto itr = createMessageFuncMap_.find(msgType);
    if (itr == createMessageFuncMap_.end()) {
        LOGE("DmAuthMessageProcessor3rd::CreateMessage msgType %{public}d error.", msgType);
        return "";
    }
    int32_t ret = (this->*(itr->second))(context, jsonObj);
    LOGI("start. message is %{public}s", GetAnonyJsonString(jsonObj.Dump()).c_str());
    return (ret == DM_OK) ? jsonObj.Dump() : "";
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2010(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (jsonObject[DM_TAG_LOGICAL_SESSION_ID].IsNumberInteger()) {
        context->logicalSessionId = jsonObject[DM_TAG_LOGICAL_SESSION_ID].Get<uint64_t>();
        context->requestId = static_cast<int64_t>(context->logicalSessionId);
    }
    if (IsString(jsonObject, TAG_DEVICE_ID_HASH)) {
        context->accesser.deviceIdHash = jsonObject[TAG_DEVICE_ID_HASH].Get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_USER_ID)) {
        context->accesser.userId = jsonObject[TAG_USER_ID].Get<int32_t>();
    }
    if (IsString(jsonObject, TAG_ACCOUNT_ID_HASH)) {
        context->accesser.accountIdHash = jsonObject[TAG_ACCOUNT_ID_HASH].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_TOKEN_ID_HASH)) {
        context->accesser.tokenIdHash = jsonObject[TAG_TOKEN_ID_HASH].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_BUSINESS_NAME)) {
        context->accesser.businessName = jsonObject[TAG_BUSINESS_NAME].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_PROCESS_NAME)) {
        context->accesser.processName = jsonObject[TAG_PROCESS_NAME].Get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_UID)) {
        context->accesser.uid = jsonObject[TAG_UID].Get<int32_t>();
    }
    if (IsString(jsonObject, TAG_VERSION)) {
        context->accesser.dmVersion = jsonObject[TAG_VERSION].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_PEER_BUSINESS_NAME)) {
        context->accessee.businessName = jsonObject[TAG_PEER_BUSINESS_NAME].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_PEER_PROCESS_NAME)) {
        context->accessee.processName = jsonObject[TAG_PEER_PROCESS_NAME].Get<std::string>();
    }
    ParseMessage2010Proxy(jsonObject, context);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinNegotiateStartState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2010Proxy(
    const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    context->IsProxyBind = false;
    if (IsBool(jsonObject, PARAM_KEY_IS_PROXY_BIND)) {
        context->IsProxyBind = jsonObject[PARAM_KEY_IS_PROXY_BIND].Get<bool>();
    }
    if (!context->IsProxyBind) {
        return DM_OK;
    }
    if (IsBool(jsonObject, PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT)) {
        context->IsCallingProxyAsSubject = jsonObject[PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT].Get<bool>();
    }
    if (!IsString(jsonObject, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string subjectProxyOnesStr = jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_PROXY_CONTEXT_ID) || !IsString(item, TAG_PROCESS_NAME) ||
            !IsString(item, TAG_PEER_PROCESS_NAME) || !IsString(item, TAG_TOKEN_ID_HASH)) {
            continue;
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = item[TAG_PROXY_CONTEXT_ID].Get<std::string>();
        proxyAuthContext.proxyAccesser.processName = item[TAG_PROCESS_NAME].Get<std::string>();
        proxyAuthContext.proxyAccessee.processName = item[TAG_PEER_PROCESS_NAME].Get<std::string>();
        proxyAuthContext.proxyAccesser.businessName = item[TAG_BUSINESS_NAME].Get<std::string>();
        proxyAuthContext.proxyAccessee.businessName = item[TAG_PEER_BUSINESS_NAME].Get<std::string>();
        proxyAuthContext.proxyAccesser.tokenIdHash = item[TAG_TOKEN_ID_HASH].Get<std::string>();
        context->subjectProxyOnes.push_back(proxyAuthContext);
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2020(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (jsonObject[TAG_DEVICE_VERSION].IsString()) {
        context->accessee.dmVersion = jsonObject[TAG_DEVICE_VERSION].Get<std::string>();
    }
    if (jsonObject[TAG_DEVICE_ID_HASH].IsString()) {
        context->accessee.deviceIdHash = jsonObject[TAG_DEVICE_ID_HASH].Get<std::string>();
    }
    if (jsonObject[TAG_USER_ID].IsNumberInteger()) {
        context->accessee.userId = jsonObject[TAG_USER_ID].Get<int32_t>();
    }
    if (jsonObject[TAG_ACCOUNT_ID_HASH].IsString()) {
        context->accessee.accountIdHash = jsonObject[TAG_ACCOUNT_ID_HASH].Get<std::string>();
    }
    if (jsonObject[TAG_TOKEN_ID_HASH].IsString()) {
        context->accessee.tokenIdHash = jsonObject[TAG_TOKEN_ID_HASH].Get<std::string>();
    }
    if (jsonObject[TAG_NETWORKID_ID].IsString()) {
        context->accessee.networkId = jsonObject[TAG_NETWORKID_ID].Get<std::string>();
    }
    ParseMessage2020Proxy(jsonObject, context);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinAuthStartState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2020Proxy(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    auto &targetList = context->subjectProxyOnes;
    auto targetKey = PARAM_KEY_SUBJECT_PROXYED_SUBJECTS;

    if (IsBool(jsonObject, PARAM_KEY_IS_PROXY_BIND)) {
        context->IsProxyBind = jsonObject[PARAM_KEY_IS_PROXY_BIND].Get<bool>();
    }
    if (!context->IsProxyBind) {
        return DM_OK;
    }
    if (!IsString(jsonObject, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string subjectProxyOnesStr = jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_PROXY_CONTEXT_ID) || !IsString(item, TAG_TOKEN_ID_HASH)) {
            continue;
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = item[TAG_PROXY_CONTEXT_ID].Get<std::string>();
        auto it = std::find(context->subjectProxyOnes.begin(), context->subjectProxyOnes.end(), proxyAuthContext);
        if (it != context->subjectProxyOnes.end()) {
            it->proxyAccessee.tokenIdHash = item[TAG_TOKEN_ID_HASH].Get<std::string>();
        }
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2030(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinAuthStartState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2040(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinAuthMsgNegotiateState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2031(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinAuthMsgNegotiateState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2041(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinAuthDoneState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2050(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded() || !jsonObject[TAG_SYNC].IsString()) {
        LOGE("ParseMessageSyncReq json error");
        return ERR_DM_FAILED;
    }
    std::string enSyncMsg = jsonObject[TAG_SYNC].Get<std::string>();
    // Decrypt data and parse data into context
    int32_t ret = DecryptSyncMessage(context, context->accesser, enSyncMsg);
    if (ret != DM_OK) {
        LOGE("DecryptSyncMessage enSyncMsg error");
        return ret;
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkDataSyncState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2060(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded() || !jsonObject[TAG_SYNC].IsString()) {
        LOGE("ParseMessageSyncResp json error");
        return ERR_DM_FAILED;
    }
    std::string enSyncMsg = jsonObject[TAG_SYNC].Get<std::string>();
    // Decrypt data and parse data into context
    int32_t ret = DecryptSyncMessage(context, context->accessee, enSyncMsg);
    if (ret != DM_OK) {
        LOGE("DecryptSyncMessage enSyncMsg error");
        return ret;
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcDataSyncState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::DecryptSyncMessage(std::shared_ptr<DmAuthContext> &context,
    DmAccess &access, std::string &enSyncMsg)
{
    std::string syncMsgCompress = "";
    int32_t ret = cryptoMgr_->DecryptMessage(enSyncMsg, syncMsgCompress);
    if (ret != DM_OK) {
        LOGE("syncMsg error");
        return ret;
    }
    JsonObject plainJson(syncMsgCompress);
    if (plainJson.IsDiscarded()) {
        LOGE("plainJson error");
        return ERR_DM_FAILED;
    }
    if (!plainJson[TAG_COMPRESS_ORI_LEN].IsNumberInteger()) {
        LOGE("TAG_COMPRESS_ORI_LEN json error");
        return ERR_DM_FAILED;
    }
    int32_t dataLen = plainJson[TAG_COMPRESS_ORI_LEN].Get<int32_t>();
    if (dataLen > MAX_MESSAGE_LENGTH) {
        LOGE("data is too long.");
        return ERR_DM_FAILED;
    }
    if (!plainJson[TAG_COMPRESS].IsString()) {
        LOGE("TAG_COMPRESS json error");
        return ERR_DM_FAILED;
    }
    std::string compressMsg = plainJson[TAG_COMPRESS].Get<std::string>();
    std::string compressBase64 = Base64Decode(compressMsg);
    std::string syncMsg = DecompressSyncMsg(compressBase64, dataLen);
    JsonObject jsonObject(syncMsg);
    if (jsonObject.IsDiscarded()) {
        LOGE("jsonStr error");
        return ERR_DM_FAILED;
    }
    ret = ParseSyncMessage(context, access, jsonObject);
    if (ret != DM_OK) {
        LOGE("ParseSyncMessage jsonStr error");
        return ret;
    }
    return DM_OK;
}

int32_t ParseInfoToDmAccess(const JsonObject &jsonObject, DmAccess &access)
{
    if (!IsString(jsonObject, TAG_TRANSMIT_SK_ID)) {
        LOGE("TAG_TRANSMIT_SK_ID error");
        return ERR_DM_FAILED;
    }
    access.transmitSessionKeyId = std::atoi(jsonObject[TAG_TRANSMIT_SK_ID].Get<std::string>().c_str());

    if (!IsInt64(jsonObject, TAG_TRANSMIT_SK_TIMESTAMP)) {
        LOGE("TAG_TRANSMIT_SK_TIMESTAMP error");
        return ERR_DM_FAILED;
    }
    access.transmitSkTimeStamp = jsonObject[TAG_TRANSMIT_SK_TIMESTAMP].Get<int64_t>();

    if (!IsString(jsonObject, TAG_DMVERSION)) {
        LOGE("ParseSyncMessage TAG_DMVERSION error");
        return ERR_DM_FAILED;
    }
    access.dmVersion = jsonObject[TAG_DMVERSION].Get<std::string>();

    if (IsString(jsonObject, TAG_DEVICE_ID)) {
        access.deviceId = jsonObject[TAG_DEVICE_ID].Get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_USER_ID)) {
        access.userId = jsonObject[TAG_USER_ID].Get<int32_t>();
    }
    if (IsString(jsonObject, TAG_ACCOUNT_ID)) {
        access.accountId = jsonObject[TAG_ACCOUNT_ID].Get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_TOKEN_ID)) {
        access.tokenId = jsonObject[TAG_TOKEN_ID].Get<int32_t>();
    }
    if (IsString(jsonObject, TAG_BUSINESS_NAME)) {
        access.businessName = jsonObject[TAG_BUSINESS_NAME].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_PROCESS_NAME)) {
        access.processName = jsonObject[TAG_PROCESS_NAME].Get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_UID)) {
        access.uid = jsonObject[TAG_UID].Get<int32_t>();
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseSyncMessage(std::shared_ptr<DmAuthContext> &context,
    DmAccess &access, JsonObject &jsonObject)
{
    DmAccess accessTmp;
    if (ParseInfoToDmAccess(jsonObject, accessTmp) != DM_OK) {
        LOGE("Parse DataSync prarm err");
        return ERR_DM_FAILED;
    }
    // check access validity
    if (ParseProxyAccessToSync(context, jsonObject) != DM_OK) {
        LOGE("ParseProxyAccessToSync error, stop auth.");
        return ERR_DM_FAILED;
    }
    if (!CheckAccessValidityAndAssign(context, access, accessTmp)) {
        LOGE("ParseSyncMessage CheckAccessValidityAndAssign error, data between two stages different, stop auth.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

bool DmAuthMessageProcessor3rd::CheckAccessValidityAndAssign(std::shared_ptr<DmAuthContext> &context, DmAccess &access,
    DmAccess &accessTmp)
{
    const DmAccess &selfAccess = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;

    bool isSame = accessTmp.dmVersion == access.dmVersion &&
        Crypto3rd::GetUdidHash(accessTmp.deviceId) == access.deviceIdHash &&
        accessTmp.userId == access.userId &&
        accessTmp.processName == access.processName &&
        accessTmp.businessName == access.businessName;
        if (context->direction == DM_AUTH_SINK) {
            isSame = (isSame && (Crypto::GetAccountIdHash16(accessTmp.accountId) == access.accountIdHash) &&
            (Crypto::GetTokenIdHash(std::to_string(accessTmp.tokenId)) == access.tokenIdHash));
        }
    if (isSame) {
        access.transmitSessionKeyId = accessTmp.transmitSessionKeyId;
        access.transmitSkTimeStamp = accessTmp.transmitSkTimeStamp;
        access.tokenId = accessTmp.tokenId;
        access.deviceId = accessTmp.deviceId;
        access.userId = accessTmp.userId;
        access.accountId = accessTmp.accountId;
    }
    return isSame;
}

int32_t DmAuthMessageProcessor3rd::ParseProxyAccessToSync(std::shared_ptr<DmAuthContext> &context,
    JsonObject &jsonObject)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return DM_OK;
    }
    if (!IsString(jsonObject, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        LOGE("no subjectProxyOnes");
        return ERR_DM_FAILED;
    }
    std::string subjectProxyOnesStr = jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_PROXY_CONTEXT_ID)) {
            LOGE("no proxyContextId");
            return ERR_DM_FAILED;
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = item[TAG_PROXY_CONTEXT_ID].Get<std::string>();
        auto it = std::find(context->subjectProxyOnes.begin(), context->subjectProxyOnes.end(), proxyAuthContext);
        if (it != context->subjectProxyOnes.end()) {
            if (!IsString(item, TAG_TRANSMIT_SK_ID) || !IsInt64(item, TAG_TRANSMIT_SK_TIMESTAMP)) {
                LOGE("proxyContext format error");
                return ERR_DM_FAILED;
            }
            DmProxyAccess &access = (context->direction == DM_AUTH_SOURCE) ? it->proxyAccessee : it->proxyAccesser;
            DmProxyAccess &selfAccess = (context->direction == DM_AUTH_SOURCE) ? it->proxyAccesser : it->proxyAccessee;
            access.transmitSessionKeyId = std::atoi(item[TAG_TRANSMIT_SK_ID].Get<std::string>().c_str());
            access.transmitSkTimeStamp = item[TAG_TRANSMIT_SK_TIMESTAMP].Get<int64_t>();
        } else {
            LOGE("proxyContextId not exist");
            return ERR_DM_FAILED;
        }
    }
    return DM_OK;
}

std::string DmAuthMessageProcessor3rd::DecompressSyncMsg(std::string& compressed, uint32_t oriLen)
{
    if (oriLen <= 0) {
        LOGE("DmAuthMessageProcessor3rd::DecompressSyncMsg decompress oriLen param error");
        return "";
    }
    std::string decompressed;
    decompressed.resize(oriLen);
    unsigned long destLen = oriLen; // Actual usage length
    int32_t ret = uncompress(reinterpret_cast<Bytef *>(&decompressed[0]), &destLen,
                             reinterpret_cast<const Bytef *>(compressed.data()),  // Skip header when decompressing
                             compressed.size());
    if (ret != Z_OK || destLen != oriLen) {
        LOGE("DmAuthMessageProcessor3rd::DecompressSyncMsg decompress failed");
        return "";
    }
    return decompressed;
}

std::string DmAuthMessageProcessor3rd::Base64Encode(std::string &inputStr)
{
    // Convert input string to binary
    const unsigned char* src = reinterpret_cast<const unsigned char*>(inputStr.data());
    size_t srcLen = inputStr.size();
    if (srcLen >= MAX_MESSAGE_LENGTH) {
        LOGE("inputStr too long");
        return "";
    }
    // Calculate the maximum length after base64 encoding
    size_t maxEncodeLen = ((srcLen + 2) / 3) * 4 + 1;
    std::vector<unsigned char> buffer(maxEncodeLen);

    // Actual encoding length
    size_t encodedLen = 0;
    int32_t ret = mbedtls_base64_encode(buffer.data(), buffer.size(), &encodedLen, src, srcLen);
    if (ret != 0) {
        LOGE("DmAuthMessageProcessor3rd::Base64Encode mbedtls_base64_encode failed");
        return "";
    }
    return std::string(reinterpret_cast<const char*>(buffer.data()), encodedLen); // No terminator needed
}

std::string DmAuthMessageProcessor3rd::Base64Decode(std::string &inputStr)
{
    // Convert input string to binary
    const unsigned char* src = reinterpret_cast<const unsigned char*>(inputStr.data());
    size_t srcLen = inputStr.size();
    if (srcLen >= MAX_MESSAGE_LENGTH) {
        LOGE("inputStr too long");
        return "";
    }
    // Calculate the maximum length after base64 encoding
    size_t maxEncodeLen = (srcLen / 4) *  3 + 1;
    std::vector<unsigned char> buffer(maxEncodeLen);

    // Actual encoding length
    size_t decodedLen = 0;
    int32_t ret = mbedtls_base64_decode(buffer.data(), buffer.size(), &decodedLen, src, srcLen);
    if (ret != 0) {
        LOGE("DmAuthMessageProcessor3rd::Base64Decode mbedtls_base64_decode failed");
        return "";
    }
    return std::string(reinterpret_cast<const char*>(buffer.data()), decodedLen);
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2070(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    if (jsonObject[TAG_REPLY].IsNumberInteger()) {
        context->reply = jsonObject[TAG_REPLY].Get<int32_t>();
    }
    if (jsonObject[TAG_REASON].IsNumberInteger()) {
        context->reason = jsonObject[TAG_REASON].Get<int32_t>();
    }
    if (context->reason != DM_OK) {
        context->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::ParseMessage2080(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    if (IsInt32(jsonObject, TAG_REPLY)) {
        context->reply = jsonObject[TAG_REPLY].Get<int32_t>();
    }
    if (IsInt32(jsonObject, TAG_REASON)) {
        context->reason = jsonObject[TAG_REASON].Get<int32_t>();
    }
    if (context->reason != DM_OK) {
        context->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2010(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    jsonObject[TAG_DEVICE_ID_HASH] = context->accesser.deviceIdHash;
    jsonObject[TAG_USER_ID] = context->accesser.userId;
    jsonObject[TAG_ACCOUNT_ID_HASH] = context->accesser.accountIdHash;
    jsonObject[TAG_TOKEN_ID_HASH] = context->accesser.tokenIdHash;
    jsonObject[TAG_BUSINESS_NAME] = context->accesser.businessName;
    jsonObject[TAG_PROCESS_NAME] = context->accesser.processName;
    jsonObject[TAG_UID] = context->accesser.uid;
    jsonObject[TAG_VERSION] = context->accesser.dmVersion;
    jsonObject[TAG_PEER_BUSINESS_NAME] = context->accessee.businessName;
    jsonObject[TAG_PEER_PROCESS_NAME] = context->accessee.processName;
    
    jsonObject[PARAM_KEY_IS_PROXY_BIND] = context->IsProxyBind;
    jsonObject[PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT] = context->IsCallingProxyAsSubject;
    if (context != nullptr && context->IsProxyBind && !context->subjectProxyOnes.empty()) {
        JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
        for (const auto &app : context->subjectProxyOnes) {
            JsonObject object;
            object[TAG_PROXY_CONTEXT_ID] = app.proxyContextId;
            object[TAG_BUSINESS_NAME] = app.proxyAccesser.businessName;
            object[TAG_PEER_BUSINESS_NAME] = app.proxyAccessee.businessName;
            object[TAG_PROCESS_NAME] = app.proxyAccesser.processName;
            object[TAG_PEER_PROCESS_NAME] = app.proxyAccessee.processName;
            object[TAG_TOKEN_ID_HASH] = app.proxyAccesser.tokenIdHash;
            allProxyObj.PushBack(object);
        }
        jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = allProxyObj.Dump();
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2020(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    jsonObject[TAG_DEVICE_VERSION] = context->accessee.dmVersion;
    jsonObject[TAG_DEVICE_ID_HASH] = context->accessee.deviceIdHash;
    jsonObject[TAG_USER_ID] = context->accessee.userId;
    jsonObject[TAG_ACCOUNT_ID_HASH] = context->accessee.accountIdHash;
    jsonObject[TAG_TOKEN_ID_HASH] = context->accessee.tokenIdHash;
    jsonObject[TAG_NETWORKID_ID] = context->accessee.networkId;
    jsonObject[PARAM_KEY_IS_PROXY_BIND] = context->IsProxyBind;
    jsonObject[PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT] = context->IsCallingProxyAsSubject;
    if (context != nullptr && context->IsProxyBind && !context->subjectProxyOnes.empty()) {
        JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
        for (const auto &app : context->subjectProxyOnes) {
            JsonObject object;
            object[TAG_PROXY_CONTEXT_ID] = app.proxyContextId;
            object[TAG_TOKEN_ID_HASH] = app.proxyAccessee.tokenIdHash;
            allProxyObj.PushBack(object);
        }
        jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = allProxyObj.Dump();
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2030(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2040(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2031(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2041(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2050(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    std::string encSyncMsg;
    int32_t ret = EncryptSyncMessage(context, context->accesser, encSyncMsg);
    if (ret != DM_OK) {
        LOGE("DmAuthMessageProcessor3rd::CreateSyncMessage encrypt failed");
        return ret;
    }
    jsonObject[TAG_SYNC] = encSyncMsg;
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::EncryptSyncMessage(std::shared_ptr<DmAuthContext> &context, DmAccess &accessSide,
    std::string &encSyncMsg)
{
    JsonObject syncMsgJson;
    SetSyncMsgJson(context, accessSide, syncMsgJson);
    CreateProxyAccessMessage(context, syncMsgJson);
    std::string syncMsg = syncMsgJson.Dump();
    std::string compressMsg = CompressSyncMsg(syncMsg);
    if (compressMsg.empty()) {
        LOGE("DmAuthMessageProcessor3rd::EncryptSyncMessage compress failed");
        return ERR_DM_FAILED;
    }
    JsonObject plainJson;
    plainJson[TAG_COMPRESS_ORI_LEN] = syncMsg.size();
    plainJson[TAG_COMPRESS] = Base64Encode(compressMsg);
    return cryptoMgr_->EncryptMessage(plainJson.Dump(), encSyncMsg);
}

int32_t DmAuthMessageProcessor3rd::SetSyncMsgJson(std::shared_ptr<DmAuthContext> &context,
    const DmAccess &accessSide, JsonObject &syncMsgJson)
{
    DmAccess &remoteAccess = (context->direction == DM_AUTH_SOURCE) ? context->accessee : context->accesser;
    syncMsgJson[TAG_TRANSMIT_SK_ID] = std::to_string(accessSide.transmitSessionKeyId);
    syncMsgJson[TAG_TRANSMIT_SK_TIMESTAMP] = accessSide.transmitSkTimeStamp;
    syncMsgJson[TAG_DMVERSION] = accessSide.dmVersion;
    syncMsgJson[TAG_DEVICE_ID] = accessSide.deviceId;
    syncMsgJson[TAG_USER_ID] = accessSide.userId;
    syncMsgJson[TAG_ACCOUNT_ID] = accessSide.accountId;
    syncMsgJson[TAG_TOKEN_ID] = accessSide.tokenId;
    syncMsgJson[TAG_BUSINESS_NAME] = accessSide.businessName;
    syncMsgJson[TAG_PROCESS_NAME] = accessSide.processName;
    syncMsgJson[TAG_PEER_BUSINESS_NAME] = remoteAccess.businessName;
    syncMsgJson[TAG_PEER_PROCESS_NAME] = remoteAccess.processName;
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateProxyAccessMessage(std::shared_ptr<DmAuthContext> &context,
    JsonObject &syncMsgJson)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return DM_OK;
    }
    JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    for (const auto &app : context->subjectProxyOnes) {
        DmProxyAccess access;
        JsonObject object;
        if (context->direction == DM_AUTH_SOURCE) {
            access = app.proxyAccesser;
        } else {
            access = app.proxyAccessee;
        }
        object[TAG_PROXY_CONTEXT_ID] = app.proxyContextId;
        object[TAG_TOKEN_ID] = access.tokenId;
        object[TAG_BIND_LEVEL] = access.bindLevel;
        object[TAG_BUSINESS_NAME] = access.businessName;
        object[TAG_TRANSMIT_SK_ID] = std::to_string(access.transmitSessionKeyId);
        object[TAG_TRANSMIT_SK_TIMESTAMP] = access.transmitSkTimeStamp;
        allProxyObj.PushBack(object);
    }
    syncMsgJson[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = allProxyObj.Dump();
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2060(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    std::string encSyncMsg;
    int32_t ret = EncryptSyncMessage(context, context->accessee, encSyncMsg);
    if (ret != DM_OK) {
        LOGE("DmAuthMessageProcessor3rd::CreateMessageSyncResp encrypt failed");
        return ret;
    }
    jsonObject[TAG_SYNC] = encSyncMsg;
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2070(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    jsonObject[TAG_REPLY] = context->reply;
    jsonObject[TAG_REASON] = context->reason;
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::CreateMessage2080(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    jsonObject[TAG_REPLY] = context->reply;
    jsonObject[TAG_REASON] = context->reason;
    return DM_OK;
}

int32_t DmAuthMessageProcessor3rd::SaveSessionKey(const uint8_t *sessionKey, const uint32_t keyLen)
{
    if (cryptoMgr_ == nullptr) {
        LOGE("DmAuthMessageProcessor3rd::SaveSessionKey failed, cryptoMgr_ is nullptr.");
        return ERR_DM_FAILED;
    }
    return cryptoMgr_->ProcessSessionKey(sessionKey, keyLen);
}

int32_t DmAuthMessageProcessor3rd::DeleteSessionKeyToDP(int32_t userId, int32_t skId)
{
    return DeviceProfileConnector3rd::GetInstance().DeleteSessionKey(userId, skId);
}

void DmAuthMessageProcessor3rd::CreateAndSendMsg(DmMessageType msgType, std::shared_ptr<DmAuthContext> context)
{
    auto message = CreateMessage(msgType, context);
    int32_t ret = context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
    if (ret != DM_OK) {
        if (context->direction == DM_AUTH_SOURCE) {
            context->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
        } else {
            context->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
        }
    }
}

std::string DmAuthMessageProcessor3rd::CompressSyncMsg(std::string &inputStr)
{
    uint32_t srcLen = inputStr.size();
    uint32_t boundSize = compressBound(srcLen);  // Maximum compression length
    if (boundSize == 0) {
        LOGE("DmAuthMessageProcessor3rd::CompressSyncMsg zlib compressBound failed");
        return "";
    }
    std::string compressed(boundSize, '\0');

    // Compress to reserved space
    unsigned long destSize = boundSize;  // Actual usable length
    int32_t ret = compress(reinterpret_cast<Bytef *>(&compressed[0]), &destSize,
                           reinterpret_cast<const Bytef *>(inputStr.data()), srcLen);
    if (ret != Z_OK) {
        LOGE("DmAuthMessageProcessor3rd::CompressSyncMsg zlib compress failed");
        return "";
    }
    compressed.resize(destSize); // Actual usage length
    return compressed;
}

int32_t DmAuthMessageProcessor3rd::SaveSessionKeyToDP(int32_t userId, int32_t &skId,
    std::vector<unsigned char> &sessionKey)
{
    if (cryptoMgr_ == nullptr) {
        LOGE("DmAuthMessageProcessor3rd::SaveSessionKeyToDP failed, cryptoMgr_ is nullptr.");
        return ERR_DM_FAILED;
    }
    sessionKey = cryptoMgr_->GetSessionKey();
    return DeviceProfileConnector3rd::GetInstance().PutSessionKey(userId, cryptoMgr_->GetSessionKey(), skId);
}

int32_t DmAuthMessageProcessor3rd::SaveDerivativeSessionKeyToDP(int32_t userId, const std::string &suffix,
    int32_t &skId, std::vector<unsigned char> &sessionKey)
{
    if (cryptoMgr_ == nullptr) {
        LOGE("DmAuthMessageProcessor3rd::SaveSessionKeyToDP failed, cryptoMgr_ is nullptr.");
        return ERR_DM_FAILED;
    }
    std::vector<unsigned char> sessionKeyTemp = cryptoMgr_->GetSessionKey();
    size_t keyLen = sessionKeyTemp.size();
    std::string keyStr(sessionKeyTemp.begin(), sessionKeyTemp.end());
    std::string newKeyStr = Crypto3rd::Sha256(keyStr + suffix);
    DMSessionKey newSessionKey;
    size_t newKeyLen = std::min(keyLen, newKeyStr.size());
    if (newKeyLen == 0 || newKeyLen > MAX_SESSION_KEY_LENGTH) {
        LOGE("newKeyLen invaild, cannot allocate memory.");
        return ERR_DM_FAILED;
    }
    newSessionKey.key = (uint8_t*)calloc(newKeyLen, sizeof(uint8_t));
    if (memcpy_s(newSessionKey.key, newKeyLen, newKeyStr.data(), newKeyLen) != DM_OK) {
        LOGE("copy key data failed.");
        if (newSessionKey.key != nullptr) {
            (void)memset_s(newSessionKey.key, newKeyLen, 0, newKeyLen);
            free(newSessionKey.key);
            newSessionKey.key = nullptr;
            newSessionKey.keyLen = 0;
        }
        return ERR_DM_FAILED;
    }
    newSessionKey.keyLen = newKeyLen;
    sessionKey = std::vector<unsigned char>(newSessionKey.key, newSessionKey.key + newSessionKey.keyLen);
    int ret = DeviceProfileConnector3rd::GetInstance().PutSessionKey(userId,
        std::vector<unsigned char>(newSessionKey.key, newSessionKey.key + newSessionKey.keyLen), skId);
    if (ret != DM_OK) {
        LOGE("DP save user session key failed %{public}d", ret);
    }
    if (newSessionKey.key != nullptr) {
        (void)memset_s(newSessionKey.key, newSessionKey.keyLen, 0, newSessionKey.keyLen);
        free(newSessionKey.key);
        newSessionKey.key = nullptr;
        newSessionKey.keyLen = 0;
    }
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS