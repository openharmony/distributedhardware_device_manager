/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dm_pin_holder.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_softbus_adapter_crypto.h"
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "softbus_connector.h"

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t SESSION_SIDE_SERVER = 0;
constexpr int32_t SESSION_ID_INVALID = -1;
constexpr int32_t REPLY_SUCCESS = 0;
constexpr int32_t REPLY_FAILED = -1;

constexpr int32_t MSG_TYPE_CREATE_PIN_HOLDER = 600;
constexpr int32_t MSG_TYPE_CREATE_PIN_HOLDER_RESP = 601;
constexpr int32_t MSG_TYPE_DESTROY_PIN_HOLDER = 650;
constexpr int32_t MSG_TYPE_DESTROY_PIN_HOLDER_RESP = 651;

constexpr const char* PINHOLDER_CREATE_TIMEOUT_TASK = "deviceManagerTimer:pinholdercreate";
constexpr const char* PINHOLDER_DESTROY_TIMEOUT_TASK = "deviceManagerTimer:pinholderdestroy";
constexpr int32_t PIN_HOLDER_SESSION_CREATE_TIMEOUT = 300;
constexpr int32_t PIN_HOLDER_SESSION_TTL_TIMEOUT = 300;

constexpr const char* TAG_PIN_TYPE = "PIN_TYPE";
constexpr const char* TAG_PAYLOAD = "PAYLOAD";
constexpr const char* TAG_REPLY = "REPLY";
DmPinHolder::DmPinHolder(std::shared_ptr<IDeviceManagerServiceListener> listener): listener_(listener)
{
    if (session_ == nullptr) {
        session_ = std::make_shared<PinHolderSession>();
    }
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    sinkState_ = SINK_INIT;
    sourceState_ = SOURCE_INIT;
}

DmPinHolder::~DmPinHolder()
{
    if (session_ != nullptr) {
        session_->UnRegisterSessionCallback();
        session_ = nullptr;
    }
    if (timer_ != nullptr) {
        timer_->DeleteAll();
        timer_ = nullptr;
    }
}

int32_t DmPinHolder::RegisterPinHolderCallback(const std::string &pkgName)
{
    if (session_ == nullptr) {
        LOGE("RegisterPinHolderCallback session is nullptr.");
        return ERR_DM_FAILED;
    }
    registerPkgName_ = pkgName;
    session_->RegisterSessionCallback(shared_from_this());
    return DM_OK;
}

int32_t DmPinHolder::CreatePinHolder(const std::string &pkgName,
    const PeerTargetId &targetId, DmPinType pinType, const std::string &payload)
{
    LOGI("CreatePinHolder.");
    if (registerPkgName_.empty() || registerPkgName_ != pkgName) {
        LOGE("CreatePinHolder pkgName: %s is not register callback.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    int32_t ret = CheckTargetIdVaild(targetId);
    if (ret != DM_OK) {
        LOGE("CreatePinHolder targetId is invalid.");
        return ret;
    }

    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("CreatePinHolder listener or session is nullptr.");
        return ERR_DM_FAILED;
    }

    if (sourceState_ != SOURCE_INIT) {
        LOGE("CreatePinHolder failed, state is %d.", sourceState_);
        return ERR_DM_FAILED;
    }

    if (sessionId_ != SESSION_ID_INVALID) {
        LOGI("CreatePinHolder session already create, sessionId: %d.", sessionId_);
        CreateGeneratePinHolderMsg();
        return DM_OK;
    }

    sessionId_ = session_->OpenSessionServer(targetId);
    if (sessionId_ < 0) {
        LOGE("[SOFTBUS]open session error, sessionId: %d.", sessionId_);
        sessionId_ = SESSION_ID_INVALID;
        listener_->OnCreateResult(registerPkgName_, ERR_DM_AUTH_OPEN_SESSION_FAILED);
        return ERR_DM_AUTH_OPEN_SESSION_FAILED;
    }
    pinType_ = pinType;
    payload_ = payload;
    return DM_OK;
}

int32_t DmPinHolder::DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId, DmPinType pinType)
{
    LOGI("DestroyPinHolder.");
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("DestroyPinHolder listener or session is nullptr.");
        return ERR_DM_FAILED;
    }

    if (registerPkgName_.empty() || pkgName != registerPkgName_) {
        LOGE("DestroyPinHolder pkgName: %s is not register callback.", pkgName.c_str());
        return ERR_DM_FAILED;
    }

    int32_t ret = CheckTargetIdVaild(targetId);
    if (ret != DM_OK) {
        LOGE("DestroyPinHolder targetId is invalid.");
        return ret;
    }

    if (sessionId_ == SESSION_ID_INVALID) {
        LOGI("DestroyPinHolder session already destroy.");
        listener_->OnDestroyResult(registerPkgName_, ERR_DM_FAILED);
        return ERR_DM_FAILED;
    }
    if (sourceState_ != SOURCE_CREATE) {
        LOGE("DestroyPinHolder failed, state is %d.", sourceState_);
        return ERR_DM_FAILED;
    }

    timer_->DeleteTimer(PINHOLDER_CREATE_TIMEOUT_TASK);

    nlohmann::json jsonObj;
    jsonObj[TAG_MSG_TYPE] = MSG_TYPE_DESTROY_PIN_HOLDER;
    jsonObj[TAG_PIN_TYPE] = pinType;
    std::string message = jsonObj.dump();
    LOGI("DestroyPinHolder, message is: %s.", message.c_str());
    ret = session_->SendData(sessionId_, message);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed, ret: %d.", ret);
        listener_->OnDestroyResult(registerPkgName_, ERR_DM_FAILED);
        return ERR_DM_FAILED;
    }
    return ret;
}

int32_t DmPinHolder::CreateGeneratePinHolderMsg()
{
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("CreateGeneratePinHolderMsg listener or session is nullptr.");
        return ERR_DM_FAILED;
    }

    timer_->DeleteTimer(PINHOLDER_CREATE_TIMEOUT_TASK);
    timer_->StartTimer(std::string(PINHOLDER_CREATE_TIMEOUT_TASK), PIN_HOLDER_SESSION_CREATE_TIMEOUT,
        [this] (std::string name) {
            DmPinHolder::CloseSession(name);
        });
    nlohmann::json jsonObj;
    jsonObj[TAG_PIN_TYPE] = pinType_;
    jsonObj[TAG_PAYLOAD] = payload_;
    jsonObj[TAG_MSG_TYPE] = MSG_TYPE_CREATE_PIN_HOLDER;
    std::string message = jsonObj.dump();
    LOGI("CreateGeneratePinHolderMsg, message type is: %d, pin type is: %d.", MSG_TYPE_CREATE_PIN_HOLDER, pinType_);
    int32_t ret = session_->SendData(sessionId_, message);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed, ret: %d.", ret);
        listener_->OnCreateResult(registerPkgName_, ERR_DM_FAILED);
        return ERR_DM_FAILED;
    }
    return ret;
}

int32_t DmPinHolder::ParseMsgType(const std::string &message)
{
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("ParseMsgType DecodeRequest jsonStr error");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("ParseMsgType err json string.");
        return ERR_DM_FAILED;
    }
    int32_t msgType = jsonObject[TAG_MSG_TYPE].get<int32_t>();
    return msgType;
}

void DmPinHolder::ProcessCreateMsg(const std::string &message)
{
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("ProcessCreateMsg listener or session is nullptr.");
        return;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("ProcessCreateMsg DecodeRequest jsonStr error");
        return;
    }
    if (!IsInt32(jsonObject, TAG_PIN_TYPE) || !IsString(jsonObject, TAG_PAYLOAD)) {
        LOGE("ProcessCreateMsg err json string.");
        return;
    }
    DmPinType pinType = static_cast<DmPinType>(jsonObject[TAG_PIN_TYPE].get<int32_t>());
    std::string payload = jsonObject[TAG_PAYLOAD].get<std::string>();

    nlohmann::json jsonObj;
    jsonObj[TAG_MSG_TYPE] = MSG_TYPE_CREATE_PIN_HOLDER_RESP;
    if (sinkState_ != SINK_INIT) {
        jsonObj[TAG_REPLY] = REPLY_FAILED;
    } else {
        jsonObj[TAG_REPLY] = REPLY_SUCCESS;
        sinkState_ = SINK_CREATE;
        sourceState_ = SOURCE_CREATE;
        listener_->OnPinHolderCreate(registerPkgName_, remoteDeviceId_, pinType, payload);
    }

    std::string msg = jsonObj.dump();
    LOGI("ProcessCreateMsg, message type is: %d.", MSG_TYPE_CREATE_PIN_HOLDER_RESP);
    int32_t ret = session_->SendData(sessionId_, msg);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed, ret: %d.", ret);
        return;
    }
}

void DmPinHolder::ProcessCreateRespMsg(const std::string &message)
{
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("ProcessCreateRespMsg DecodeRequest jsonStr error.");
        return;
    }
    if (!IsInt32(jsonObject, TAG_REPLY)) {
        LOGE("ProcessCreateRespMsg err json string.");
        return;
    }
    int32_t reply = jsonObject[TAG_REPLY].get<int32_t>();
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("ProcessCreateRespMsg listener or session is nullptr.");
        return;
    }
    if (reply == REPLY_SUCCESS) {
        listener_->OnCreateResult(registerPkgName_, DM_OK);
        sourceState_ = SOURCE_CREATE;
        sinkState_ = SINK_CREATE;
    } else {
        LOGE("ProcessCreateRespMsg remote state is wrong.");
        listener_->OnCreateResult(registerPkgName_, ERR_DM_FAILED);
        session_->CloseSessionServer(sessionId_);
    }
}

void DmPinHolder::ProcessDestroyMsg(const std::string &message)
{
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("ProcessDestroyMsg listener or session is nullptr.");
        return;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("ProcessDestroyMsg DecodeRequest jsonStr error.");
        return;
    }
    if (!IsInt32(jsonObject, TAG_PIN_TYPE)) {
        LOGE("ProcessDestroyMsg err json string.");
        return;
    }
    DmPinType pinType = static_cast<DmPinType>(jsonObject[TAG_PIN_TYPE].get<int32_t>());

    nlohmann::json jsonObj;
    jsonObj[TAG_MSG_TYPE] = MSG_TYPE_DESTROY_PIN_HOLDER_RESP;
    if (sinkState_ != SINK_CREATE) {
        jsonObj[TAG_REPLY] = REPLY_FAILED;
    } else {
        jsonObj[TAG_REPLY] = REPLY_SUCCESS;
        sinkState_ = SINK_INIT;
        sourceState_ = SOURCE_INIT;
        listener_->OnPinHolderDestroy(registerPkgName_, pinType);
    }

    std::string msg = jsonObj.dump();
    LOGI("ProcessDestroyMsg, message type is: %d.", MSG_TYPE_DESTROY_PIN_HOLDER_RESP);
    int32_t ret = session_->SendData(sessionId_, msg);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed, ret: %d.", ret);
        return;
    }
}

void DmPinHolder::CloseSession(const std::string &name)
{
    LOGI("DmPinHolder::CloseSession start timer name %s.", name.c_str());
    if (session_ == nullptr) {
        LOGE("CloseSession session is nullptr.");
        return;
    }
    session_->CloseSessionServer(sessionId_);
}

void DmPinHolder::ProcessDestroyResMsg(const std::string &message)
{
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("ProcessDestroyResMsg DecodeRequest jsonStr error.");
        return;
    }
    if (!IsInt32(jsonObject, TAG_REPLY)) {
        LOGE("ProcessDestroyResMsg err json string.");
        return;
    }
    int32_t reply = jsonObject[TAG_REPLY].get<int32_t>();
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("ProcessDestroyResMsg listener or session is nullptr.");
        return;
    }
    if (reply == REPLY_SUCCESS) {
        listener_->OnDestroyResult(registerPkgName_, DM_OK);
        sourceState_ = SOURCE_INIT;
        sinkState_ = SINK_INIT;
        timer_->StartTimer(std::string(PINHOLDER_DESTROY_TIMEOUT_TASK), PIN_HOLDER_SESSION_TTL_TIMEOUT,
            [this] (std::string name) {
                DmPinHolder::CloseSession(name);
            });
    } else {
        LOGE("ProcessDestroyResMsg remote state is wrong.");
        listener_->OnDestroyResult(registerPkgName_, ERR_DM_FAILED);
        session_->CloseSessionServer(sessionId_);
    }
}

void DmPinHolder::OnDataReceived(int32_t sessionId, std::string message)
{
    int32_t msgType = ParseMsgType(message);
    LOGI("OnDataReceived, msgType: %d.", msgType);

    switch (msgType) {
        case MSG_TYPE_CREATE_PIN_HOLDER:
            ProcessCreateMsg(message);
            break;
        case MSG_TYPE_CREATE_PIN_HOLDER_RESP:
            ProcessCreateRespMsg(message);
            break;
        case MSG_TYPE_DESTROY_PIN_HOLDER:
            ProcessDestroyMsg(message);
            break;
        case MSG_TYPE_DESTROY_PIN_HOLDER_RESP:
            ProcessDestroyResMsg(message);
            break;
        default:
            break;
    }
}

void DmPinHolder::GetPeerDeviceId(int32_t sessionId, std::string &udidHash)
{
    char peerDeviceId[DEVICE_UUID_LENGTH] = {0};
    int32_t ret = ::GetPeerDeviceId(sessionId, &peerDeviceId[0], DEVICE_UUID_LENGTH);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetPeerDeviceId failed for session: %d.", sessionId);
        udidHash = "";
        return;
    }
    std::string deviceId = peerDeviceId;
    char udidHashTmp[DM_MAX_DEVICE_ID_LEN] = {0};
    if (DmSoftbusAdapterCrypto::GetUdidHash(deviceId, (uint8_t *)udidHashTmp) != DM_OK) {
        LOGE("get udidhash by udid: %s failed.", GetAnonyString(deviceId).c_str());
        udidHash = "";
        return ;
    }
    udidHash = udidHashTmp;
    LOGI("GetPeerDeviceId udid hash: %s success.", GetAnonyString(udidHash).c_str());
}

void DmPinHolder::OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result)
{
    sessionId_ = sessionId;
    if (sessionSide == SESSION_SIDE_SERVER) {
        LOGI("[SOFTBUS]onSesssionOpened success, side is sink. sessionId: %d.", sessionId);
        GetPeerDeviceId(sessionId, remoteDeviceId_);
        return;
    }
    if (result == DM_OK) {
        CreateGeneratePinHolderMsg();
        return;
    }
    LOGE("[SOFTBUS]onSesssionOpened failed. sessionId: %d.", sessionId);
    if (listener_ != nullptr) {
        listener_->OnCreateResult(registerPkgName_, ERR_DM_FAILED);
    }
    return;
}

void DmPinHolder::OnSessionClosed(int32_t sessionId)
{
    LOGI("[SOFTBUS]OnSessionClosed sessionId: %d.", sessionId);
    sessionId_ = SESSION_ID_INVALID;
    sinkState_ = SINK_INIT;
    sourceState_ = SOURCE_INIT;
    remoteDeviceId_ = "";
    return;
}

int32_t DmPinHolder::CheckTargetIdVaild(const PeerTargetId &targetId)
{
    if (targetId.deviceId.empty() && targetId.brMac.empty() && targetId.bleMac.empty() && targetId.wifiIp.empty()) {
        LOGE("CheckTargetIdVaild failed. targetId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS