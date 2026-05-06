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

#include <cstring>
#include <securec.h>
#include <iostream>
#include <sstream>
#include <zlib.h>
#include <mbedtls/base64.h>

#include "access_control_profile.h"
#include "distributed_device_profile_client.h"
#include "service_info_unique_key.h"
#include "parameter.h"
#include "dm_auth_message_processor_cred.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_context_cred.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_state_machine_cred.h"
#include "dm_constants_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"

namespace OHOS {
namespace DistributedHardware {
constexpr const char* DM_TAG_LOGICAL_SESSION_ID = "logicalSessionId";
DmAuthMessageProcessorCred::DmAuthMessageProcessorCred()
{
    LOGI("DmAuthMessageProcessorCred constructor");
    cryptoMgr_ = std::make_shared<CryptoMgr3rd>();
    ConstructCreateMessageFuncMap();
    ConstructParseMessageFuncMap();
}

DmAuthMessageProcessorCred::~DmAuthMessageProcessorCred()
{
    if (cryptoMgr_ != nullptr) {
        cryptoMgr_->ClearSessionKey();
        cryptoMgr_ = nullptr;
    }
}

bool IsCredMessageValid(const JsonItemObject &jsonObject)
{
    if (jsonObject.IsDiscarded()) {
        LOGE("DmAuthMessageProcessorCred::ParseMessage failed, decodeRequestAuth jsonStr error");
        return false;
    }
    if (!IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("DmAuthMessageProcessorCred::ParseMessage failed, message type error.");
        return false;
    }
    return true;
}

void DmAuthMessageProcessorCred::ConstructCreateMessageFuncMap()
{
    createMessageFuncMap_ = {
        {DmCredMessageType::CRED_REQ_NEGOTIATE, &DmAuthMessageProcessorCred::CreateMessage3010},
        {DmCredMessageType::CRED_RESP_NEGOTIATE, &DmAuthMessageProcessorCred::CreateMessage3020},
        {DmCredMessageType::CRED_REQ_CREDENTIAL_AUTH_START, &DmAuthMessageProcessorCred::CreateMessage3030},
        {DmCredMessageType::CRED_RESP_CREDENTIAL_AUTH_START, &DmAuthMessageProcessorCred::CreateMessage3040},
        {DmCredMessageType::CRED_REQ_FINISH, &DmAuthMessageProcessorCred::CreateMessage3050},
        {DmCredMessageType::CRED_RESP_FINISH, &DmAuthMessageProcessorCred::CreateMessage3060},
    };
}

void DmAuthMessageProcessorCred::ConstructParseMessageFuncMap()
{
    parseMessageFuncMap_ = {
        {DmCredMessageType::CRED_REQ_NEGOTIATE, &DmAuthMessageProcessorCred::ParseMessage3010},
        {DmCredMessageType::CRED_RESP_NEGOTIATE, &DmAuthMessageProcessorCred::ParseMessage3020},
        {DmCredMessageType::CRED_REQ_CREDENTIAL_AUTH_START, &DmAuthMessageProcessorCred::ParseMessage3030},
        {DmCredMessageType::CRED_RESP_CREDENTIAL_AUTH_START, &DmAuthMessageProcessorCred::ParseMessage3040},
        {DmCredMessageType::CRED_REQ_FINISH, &DmAuthMessageProcessorCred::ParseMessage3050},
        {DmCredMessageType::CRED_RESP_FINISH, &DmAuthMessageProcessorCred::ParseMessage3060},
    };
}

int32_t DmAuthMessageProcessorCred::ParseMessage(std::shared_ptr<DmAuthCredContext> context,
    const std::string &message)
{
    JsonObject jsonObject(message);
    if (context == nullptr || !IsCredMessageValid(jsonObject)) {
        return ERR_DM_FAILED;
    }
    DmCredMessageType msgType = static_cast<DmCredMessageType>(jsonObject[TAG_MSG_TYPE].Get<int32_t>());
    LOGI("DmAuthMessageProcessorCred::ParseMessage message type %{public}d", msgType);
    auto itr = parseMessageFuncMap_.find(msgType);
    if (itr == parseMessageFuncMap_.end()) {
        LOGI("DmAuthMessageProcessorCred::ParseMessage message type error %{public}d", msgType);
        return ERR_DM_FAILED;
    }
    return (this->*(itr->second))(jsonObject, context);
}

std::string DmAuthMessageProcessorCred::CreateMessage(DmCredMessageType msgType,
    std::shared_ptr<DmAuthCredContext> context)
{
    LOGI("DmAuthMessageProcessorCred::CreateMessage start. msgType is %{public}d", msgType);
    CHECK_NULL_RETURN(context, "");
    JsonObject jsonObj;
    jsonObj[TAG_MSG_TYPE] = msgType;
    jsonObj[DM_TAG_LOGICAL_SESSION_ID] = context->logicalSessionId;
    auto itr = createMessageFuncMap_.find(msgType);
    if (itr == createMessageFuncMap_.end()) {
        LOGE("DmAuthMessageProcessorCred::CreateMessage msgType %{public}d error.", msgType);
        return "";
    }
    int32_t ret = (this->*(itr->second))(context, jsonObj);
    std::string anonyJsonStr = GetAnonyJsonString(jsonObj.Dump());
    LOGI("start. message is %{public}s", anonyJsonStr.c_str());
    return (ret == DM_OK) ? jsonObj.Dump() : "";
}

int32_t DmAuthMessageProcessorCred::ParseMessage3010(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthCredContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_VERSION)) {
        context->accesser.dmVersion = jsonObject[TAG_VERSION].Get<std::string>();
    }
    if (IsUint64(jsonObject, DM_TAG_LOGICAL_SESSION_ID)) {
        context->logicalSessionId = jsonObject[DM_TAG_LOGICAL_SESSION_ID].Get<uint64_t>();
        context->requestId = static_cast<int64_t>(context->logicalSessionId);
    }
    if (IsInt32(jsonObject, TAG_USER_ID)) {
        context->accesser.userId = jsonObject[TAG_USER_ID].Get<int32_t>();
    }
    if (IsString(jsonObject, TAG_DEVICE_ID_HASH)) {
        context->accesser.deviceIdHash = jsonObject[TAG_DEVICE_ID_HASH].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_BUSINESS_NAME)) {
        context->accesser.businessName = jsonObject[TAG_BUSINESS_NAME].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_PROCESS_NAME)) {
        context->accesser.processName = jsonObject[TAG_PROCESS_NAME].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_PEER_BUSINESS_NAME)) {
        context->accessee.businessName = jsonObject[TAG_PEER_BUSINESS_NAME].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_PEER_PROCESS_NAME)) {
        context->accessee.processName = jsonObject[TAG_PEER_PROCESS_NAME].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_OPENID_HASH)) {
        context->accesser.openIdHash = jsonObject[TAG_OPENID_HASH].Get<std::string>();
    }
    if (IsString(jsonObject, TAG_OWNER_ID)) {
        context->accesser.ownerId = jsonObject[TAG_OWNER_ID].Get<std::string>();
    }
    if (IsUint8(jsonObject, TAG_CRED_TYPE)) {
        context->accesser.credType = jsonObject[TAG_CRED_TYPE].Get<uint8_t>();
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkNegotiateStartState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::ParseMessage3020(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthCredContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (jsonObject[TAG_DEVICE_VERSION].IsString()) {
        context->accessee.dmVersion = jsonObject[TAG_DEVICE_VERSION].Get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_USER_ID)) {
        context->accessee.userId = jsonObject[TAG_USER_ID].Get<int32_t>();
    }
    if (IsString(jsonObject, TAG_DEVICE_ID_HASH)) {
        context->accessee.deviceIdHash = jsonObject[TAG_DEVICE_ID_HASH].Get<std::string>();
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcCredentialAuthStartState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::ParseMessage3030(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthCredContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authStateMachine->NotifyEventFinish(ON_NEXT);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkCredentialAuthMsgNegotiateState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::ParseMessage3040(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthCredContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authStateMachine, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authStateMachine->NotifyEventFinish(ON_NEXT);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcCredentialAuthMsgNegotiateState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::ParseMessage3050(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthCredContext> context)
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
    context->authStateMachine->NotifyEventFinish(ON_NEXT);
    context->authStateMachine->TransitionTo(std::make_shared<AuthCredSinkFinishState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::ParseMessage3060(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthCredContext> context)
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
    context->authStateMachine->NotifyEventFinish(ON_NEXT);
    context->authStateMachine->TransitionTo(std::make_shared<AuthCredSrcFinishState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::CreateMessage3010(std::shared_ptr<DmAuthCredContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_VERSION] = context->accesser.dmVersion;
    jsonObject[TAG_USER_ID] = context->accesser.userId;
    jsonObject[TAG_DEVICE_ID_HASH] = context->accesser.deviceIdHash;
    jsonObject[TAG_BUSINESS_NAME] = context->accesser.businessName;
    jsonObject[TAG_PROCESS_NAME] = context->accesser.processName;
    jsonObject[TAG_PEER_BUSINESS_NAME] = context->accessee.businessName;
    jsonObject[TAG_PEER_PROCESS_NAME] = context->accessee.processName;
    jsonObject[TAG_OPENID_HASH] = context->accesser.openIdHash;
    jsonObject[TAG_CRED_TYPE] = context->accesser.credType;
    jsonObject[TAG_OWNER_ID] = context->accesser.ownerId;
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::CreateMessage3020(std::shared_ptr<DmAuthCredContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_DEVICE_VERSION] = context->accessee.dmVersion;
    jsonObject[TAG_USER_ID] = context->accessee.userId;
    jsonObject[TAG_DEVICE_ID_HASH] = context->accessee.deviceIdHash;
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::CreateMessage3030(std::shared_ptr<DmAuthCredContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::CreateMessage3040(std::shared_ptr<DmAuthCredContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::CreateMessage3050(std::shared_ptr<DmAuthCredContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_REPLY] = context->reply;
    jsonObject[TAG_REASON] = context->reason;
    return DM_OK;
}

int32_t DmAuthMessageProcessorCred::CreateMessage3060(std::shared_ptr<DmAuthCredContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_REPLY] = context->reply;
    jsonObject[TAG_REASON] = context->reason;
    return DM_OK;
}

void DmAuthMessageProcessorCred::CreateAndSendMsg(DmCredMessageType msgType,
    std::shared_ptr<DmAuthCredContext> context)
{
    auto message = CreateMessage(msgType, context);
    int32_t ret = context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
    if (ret != DM_OK) {
        LOGI("SendData failed ret: %{public}d", ret);
        if (context->direction == DM_AUTH_CRED_SOURCE) {
            context->authStateMachine->TransitionTo(std::make_shared<AuthCredSrcFinishState>());
        } else {
            context->authStateMachine->TransitionTo(std::make_shared<AuthCredSinkFinishState>());
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS