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

#include "dm_auth_pincode_message_processor_3rd.h"

#include <cstring>
#include <securec.h>
#include <iostream>
#include <sstream>
#include <zlib.h>

#include "parameter.h"

#include "dm_anonymous_3rd.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_pincode_state_machine_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_crypto_3rd.h"
#include "dm_log_3rd.h"

namespace OHOS {
namespace DistributedHardware {

DmAuthPincodeMessageProcessor3rd::DmAuthPincodeMessageProcessor3rd()
{
    LOGI("DmAuthPincodeMessageProcessor3rd constructor");
    ConstructCreateMessageFuncMap();
    ConstructParseMessageFuncMap();
}

DmAuthPincodeMessageProcessor3rd::~DmAuthPincodeMessageProcessor3rd()
{
    LOGI("desconstructor");
}

bool DmAuthPincodeMessageProcessor3rd::IsMessageValid(const JsonItemObject &jsonObject)
{
    if (jsonObject.IsDiscarded()) {
        LOGE("failed, decodeRequestAuth jsonStr error");
        return false;
    }
    if (!jsonObject[TAG_MSG_TYPE].IsNumberInteger()) {
        LOGE("failed, message type error.");
        return false;
    }
    return true;
}

void DmAuthPincodeMessageProcessor3rd::ConstructCreateMessageFuncMap()
{
    createMessageFuncMap_ = {
        {DmPincodeMessageType::AUTH_PINCODE_REQ_NEGOTIATE, &DmAuthPincodeMessageProcessor3rd::CreateMessage1010},
        {DmPincodeMessageType::AUTH_PINCODE_RESP_NEGOTIATE, &DmAuthPincodeMessageProcessor3rd::CreateMessage1020},
        {DmPincodeMessageType::AUTH_PINCODE_REQ_PIN_AUTH_START, &DmAuthPincodeMessageProcessor3rd::CreateMessage1030},
        {DmPincodeMessageType::AUTH_PINCODE_RESP_PIN_AUTH_START, &DmAuthPincodeMessageProcessor3rd::CreateMessage1040},
        {DmPincodeMessageType::AUTH_PINCODE_REQ_PIN_AUTH_MSG_NEGOTIATE,
            &DmAuthPincodeMessageProcessor3rd::CreateMessage1031},
        {DmPincodeMessageType::AUTH_PINCODE_RESP_PIN_AUTH_MSG_NEGOTIATE,
            &DmAuthPincodeMessageProcessor3rd::CreateMessage1041},
        {DmPincodeMessageType::AUTH_PINCODE_REQ_FINISH, &DmAuthPincodeMessageProcessor3rd::CreateMessage1050},
        {DmPincodeMessageType::AUTH_PINCODE_RESP_FINISH, &DmAuthPincodeMessageProcessor3rd::CreateMessage1060},
    };
}

void DmAuthPincodeMessageProcessor3rd::ConstructParseMessageFuncMap()
{
    parseMessageFuncMap_ = {
        {DmPincodeMessageType::AUTH_PINCODE_REQ_NEGOTIATE, &DmAuthPincodeMessageProcessor3rd::ParseMessage1010},
        {DmPincodeMessageType::AUTH_PINCODE_RESP_NEGOTIATE, &DmAuthPincodeMessageProcessor3rd::ParseMessage1020},
        {DmPincodeMessageType::AUTH_PINCODE_REQ_PIN_AUTH_START, &DmAuthPincodeMessageProcessor3rd::ParseMessage1030},
        {DmPincodeMessageType::AUTH_PINCODE_RESP_PIN_AUTH_START, &DmAuthPincodeMessageProcessor3rd::ParseMessage1040},
        {DmPincodeMessageType::AUTH_PINCODE_REQ_PIN_AUTH_MSG_NEGOTIATE,
            &DmAuthPincodeMessageProcessor3rd::ParseMessage1031},
        {DmPincodeMessageType::AUTH_PINCODE_RESP_PIN_AUTH_MSG_NEGOTIATE,
            &DmAuthPincodeMessageProcessor3rd::ParseMessage1041},
        {DmPincodeMessageType::AUTH_PINCODE_REQ_FINISH, &DmAuthPincodeMessageProcessor3rd::ParseMessage1050},
        {DmPincodeMessageType::AUTH_PINCODE_RESP_FINISH, &DmAuthPincodeMessageProcessor3rd::ParseMessage1060},
    };
}

int32_t DmAuthPincodeMessageProcessor3rd::ParseMessage(std::shared_ptr<DmAuthPincodeContext> context,
    const std::string &message)
{
    JsonObject jsonObject(message);
    if (context == nullptr || !IsMessageValid(jsonObject)) {
        LOGE("param invaild");
        return ERR_DM_FAILED;
    }
    DmPincodeMessageType msgType = static_cast<DmPincodeMessageType>(jsonObject[TAG_MSG_TYPE].Get<int32_t>());
    context->msgType = msgType;
    LOGI("ParseMessage message type %{public}d", context->msgType);
    auto itr = parseMessageFuncMap_.find(msgType);
    if (itr == parseMessageFuncMap_.end()) {
        LOGI("ParseMessage message type %{public}d error", context->msgType);
        return ERR_DM_FAILED;
    }
    return (this->*(itr->second))(jsonObject, context);
}

std::string DmAuthPincodeMessageProcessor3rd::CreateMessage(DmPincodeMessageType msgType,
    std::shared_ptr<DmAuthPincodeContext> context)
{
    LOGI("DmAuthPincodeMessageProcessor3rd::CreateMessage start. msgType is %{public}d", msgType);
    if (context == nullptr) {
        LOGE("context is nullptr");
        return "";
    }
    JsonObject jsonObj;
    jsonObj[TAG_MSG_TYPE] = msgType;
    jsonObj[DM_TAG_LOGICAL_SESSION_ID] = context->logicalSessionId;
    auto itr = createMessageFuncMap_.find(msgType);
    if (itr == createMessageFuncMap_.end()) {
        LOGE("DmAuthPincodeMessageProcessor3rd::CreateMessage msgType %{public}d error.", msgType);
        return "";
    }
    int32_t ret = (this->*(itr->second))(context, jsonObj);
    LOGI("start. message is %{public}s", GetAnonyJsonString(jsonObj.Dump()).c_str());
    return (ret == DM_OK) ? jsonObj.Dump() : "";
}

int32_t DmAuthPincodeMessageProcessor3rd::ParseMessage1010(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthPincodeContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
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
    context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSinkPinNegotiateStartState>());
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::ParseMessage1020(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthPincodeContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
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
    context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSrcPinAuthStartState>());
    return DM_OK;
}


int32_t DmAuthPincodeMessageProcessor3rd::ParseMessage1030(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthPincodeContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSinkPinAuthStartState>());
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::ParseMessage1040(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthPincodeContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSrcPinAuthMsgNegotiateState>());
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::ParseMessage1031(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthPincodeContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSinkPinAuthMsgNegotiateState>());
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::ParseMessage1041(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthPincodeContext> context)
{
    if (jsonObject.IsDiscarded()) {
        return ERR_DM_FAILED;
    }
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authPinStateMac3rd, ERR_DM_POINT_NULL);
    if (IsString(jsonObject, TAG_DATA)) {
        context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    }
    context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSrcPinAuthDoneState>());
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::ParseMessage1050(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthPincodeContext> context)
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
        context->authPinStateMac3rd->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSinkFinishState>());
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::ParseMessage1060(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthPincodeContext> context)
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
        context->authPinStateMac3rd->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSrcFinishState>()); // 6
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::CreateMessage1010(std::shared_ptr<DmAuthPincodeContext> context,
    JsonObject &jsonObject)
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
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::CreateMessage1020(std::shared_ptr<DmAuthPincodeContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_DEVICE_VERSION] = context->accessee.dmVersion;
    jsonObject[TAG_DEVICE_ID_HASH] = context->accessee.deviceIdHash;
    jsonObject[TAG_USER_ID] = context->accessee.userId;
    jsonObject[TAG_ACCOUNT_ID_HASH] = context->accessee.accountIdHash;
    jsonObject[TAG_TOKEN_ID_HASH] = context->accessee.tokenIdHash;
    jsonObject[TAG_NETWORKID_ID] = context->accessee.networkId;

    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::CreateMessage1030(std::shared_ptr<DmAuthPincodeContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::CreateMessage1040(std::shared_ptr<DmAuthPincodeContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::CreateMessage1031(std::shared_ptr<DmAuthPincodeContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::CreateMessage1041(std::shared_ptr<DmAuthPincodeContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::CreateMessage1050(std::shared_ptr<DmAuthPincodeContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_REPLY] = context->reply;
    jsonObject[TAG_REASON] = context->reason;
    return DM_OK;
}

int32_t DmAuthPincodeMessageProcessor3rd::CreateMessage1060(std::shared_ptr<DmAuthPincodeContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_REPLY] = context->reply;
    jsonObject[TAG_REASON] = context->reason;
    return DM_OK;
}

void DmAuthPincodeMessageProcessor3rd::CreateAndSendMsg(DmPincodeMessageType msgType,
    std::shared_ptr<DmAuthPincodeContext> context)
{
    auto message = CreateMessage(msgType, context);
    int32_t ret = context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
    if (ret != DM_OK) {
        if (context->direction == DM_AUTH_PINCODE_SOURCE) {
            context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSrcFinishState>());
        } else {
            context->authPinStateMac3rd->TransitionTo(std::make_shared<AuthPincodeSinkFinishState>());
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS