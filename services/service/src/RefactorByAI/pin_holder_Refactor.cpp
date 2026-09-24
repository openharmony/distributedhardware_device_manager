/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "pin_holder_Refactor.h"

#include "dm_anonymous.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "multiple_user_connector.h"

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
constexpr int32_t MSG_TYPE_PIN_HOLDER_CHANGE = 700;
constexpr int32_t MSG_TYPE_PIN_HOLDER_CHANGE_RESP = 701;
constexpr int32_t MSG_TYPE_PIN_CLOSE_SESSION = 800;

constexpr const char* PINHOLDER_CREATE_TIMEOUT_TASK = "deviceManagerTimer:pinholdercreate";
constexpr int32_t PIN_HOLDER_SESSION_CREATE_TIMEOUT = 60;

constexpr const char* TAG_PIN_TYPE = "PIN_TYPE";
constexpr const char* TAG_PAYLOAD = "PAYLOAD";
constexpr const char* TAG_REPLY = "REPLY";
constexpr const char* TAG_REMOTE_DEVICE_ID = "REMOTE_DEVICE_ID";

constexpr int32_t DM_OK = 0;
constexpr int32_t ERR_DM_FAILED = 96929744;
constexpr int32_t ERR_DM_TIME_OUT = 96929745;
constexpr int32_t ERR_DM_CREATE_PIN_HOLDER_BUSY = 96929821;
constexpr const char* TAG_MSG_TYPE = "MSG_TYPE";
constexpr const char* TAG_DM_VERSION = "DM_VERSION";
constexpr const char* DM_CONNECTION_DISCONNECTED = "DM_CONNECTION_DISCONNECTED";
constexpr int32_t DEVICE_UUID_LENGTH = 65;
constexpr int32_t ERR_DM_INPUT_PARA_INVALID = 96929749;
constexpr int32_t ERR_DM_BIND_PEER_UNSUPPORTED = 96929802;

PinHolder::PinHolder(std::shared_ptr<IDeviceManagerServiceListener> listener): listener_(listener)
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

PinHolder::~PinHolder()
{
    SecureClearSensitiveData();
    if (timer_ != nullptr) {
        timer_->DeleteAll();
        timer_ = nullptr;
    }
}

void PinHolder::SecureClearSensitiveData()
{
    if (!payload_.empty()) {
        (void)memset_s(&payload_[0], payload_.size(), 0, payload_.size());
        payload_.clear();
    }
    if (!remoteDeviceId_.empty()) {
        (void)memset_s(&remoteDeviceId_[0], remoteDeviceId_.size(), 0, remoteDeviceId_.size());
        remoteDeviceId_.clear();
    }
}

void PinHolder::ResetSessionState()
{
    sessionId_ = SESSION_ID_INVALID;
    sinkState_ = SINK_INIT;
    sourceState_ = SOURCE_INIT;
    destroyState_ = STATE_UNKNOW;
    isRemoteSupported_ = false;
    isDestroy_.store(false);
    SecureClearSensitiveData();
}

bool PinHolder::ValidatePkgName(const std::string &pkgName)
{
    if (pkgName.empty()) {
        return false;
    }
    if (processInfo_.pkgName.empty() || pkgName != processInfo_.pkgName) {
        return false;
    }
    return true;
}

int32_t PinHolder::RegisterPinHolderCallback(const std::string &pkgName)
{
    if (session_ == nullptr) {
        LOGE("session is nullptr.");
        return ERR_DM_FAILED;
    }
    if (pkgName.empty()) {
        LOGE("pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t userId = -1;
    MultipleUserConnector::GetCallerUserId(userId);
    MultipleUserConnector::GetCallingTokenId(processInfo_.tokenId);
    processInfo_.userId = userId;
    processInfo_.pkgName = pkgName;
    session_->RegisterSessionCallback(shared_from_this());
    return DM_OK;
}

int32_t PinHolder::UnRegisterPinHolderCallback(const std::string &pkgName)
{
    if (session_ == nullptr) {
        LOGE("session is nullptr.");
        return ERR_DM_FAILED;
    }
    if (!ValidatePkgName(pkgName)) {
        LOGE("pkgName validation failed.");
        return ERR_DM_FAILED;
    }
    session_->UnRegisterSessionCallback();
    ResetSessionState();
    LOGI("success.");
    return DM_OK;
}

int32_t PinHolder::CreatePinHolder(const std::string &pkgName,
    const PeerTargetId &targetId, DmPinType pinType, const std::string &payload)
{
    LOGI("CreatePinHolder.");
    if (!ValidatePkgName(pkgName)) {
        LOGE("pkgName validation failed.");
        return ERR_DM_FAILED;
    }
    int32_t ret = CheckTargetIdVaild(targetId);
    if (ret != DM_OK) {
        LOGE("targetId is invalid.");
        return ret;
    }
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("listener or session is nullptr.");
        return ERR_DM_FAILED;
    }
    if (sourceState_ != SOURCE_INIT) {
        LOGE("state is %{public}d.", sourceState_);
        return ERR_DM_FAILED;
    }
    if (sessionId_ != SESSION_ID_INVALID) {
        LOGI("session already create, sessionId: %{public}d.", sessionId_);
        CreateGeneratePinHolderMsg();
        return DM_OK;
    }

    sessionId_ = session_->OpenSessionServer(targetId);
    int32_t stageRes =
        sessionId_ > 0 ? static_cast<int32_t>(StageRes::STAGE_SUCC) : static_cast<int32_t>(StageRes::STAGE_FAIL);
    DmRadarHelper::GetInstance().ReportCreatePinHolder(
        processInfo_.pkgName, sessionId_, targetId.deviceId, sessionId_, stageRes);
    if (sessionId_ < 0) {
        LOGE("[SOFTBUS]open session error, sessionId: %{public}d.", sessionId_);
        listener_->OnCreateResult(processInfo_, sessionId_);
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::CREATE_RESULT,
            sessionId_, "");
        sessionId_ = SESSION_ID_INVALID;
        return sessionId_;
    }
    pinType_ = pinType;
    payload_ = payload;
    return DM_OK;
}

int32_t PinHolder::DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId, DmPinType pinType,
    const std::string &payload)
{
    LOGI("DestroyPinHolder.");
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("listener or session is nullptr.");
        return ERR_DM_FAILED;
    }
    if (!ValidatePkgName(pkgName)) {
        LOGE("pkgName validation failed.");
        return ERR_DM_FAILED;
    }
    int32_t ret = CheckTargetIdVaild(targetId);
    if (ret != DM_OK) {
        LOGE("targetId is invalid.");
        return ret;
    }
    if (sessionId_ == SESSION_ID_INVALID) {
        LOGI("session already destroy.");
        listener_->OnDestroyResult(processInfo_, ret);
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::DESTROY_RESULT, ret, "");
        return ret;
    }
    if (sourceState_ != SOURCE_CREATE) {
        LOGE("state is %{public}d.", sourceState_);
        return ERR_DM_FAILED;
    }
    if (timer_ != nullptr) {
        timer_->DeleteTimer(PINHOLDER_CREATE_TIMEOUT_TASK);
    }

    JsonObject jsonObj;
    jsonObj[TAG_MSG_TYPE] = MSG_TYPE_DESTROY_PIN_HOLDER;
    jsonObj[TAG_PIN_TYPE] = pinType;
    jsonObj[TAG_PAYLOAD] = payload;
    pinType_ = pinType;
    std::string message = jsonObj.Dump();
    LOGI("message type is: %{public}d, pin type is: %{public}d.", MSG_TYPE_DESTROY_PIN_HOLDER,
        pinType);
    ret = session_->SendData(sessionId_, message);
    int32_t stageRes =
        ret == DM_OK ? static_cast<int32_t>(StageRes::STAGE_SUCC) : static_cast<int32_t>(StageRes::STAGE_FAIL);
    DmRadarHelper::GetInstance().ReportDestroyPinHolder(processInfo_.pkgName, targetId.deviceId, ret, stageRes);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed, ret: %{public}d.", ret);
        listener_->OnDestroyResult(processInfo_, ERR_DM_FAILED);
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::DESTROY_RESULT, ERR_DM_FAILED, "");
        return ret;
    }
    return ret;
}

int32_t PinHolder::CreateGeneratePinHolderMsg()
{
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("listener or session is nullptr.");
        return ERR_DM_FAILED;
    }
    if (timer_ != nullptr) {
        timer_->DeleteAll();
        timer_->StartTimer(std::string(PINHOLDER_CREATE_TIMEOUT_TASK), PIN_HOLDER_SESSION_CREATE_TIMEOUT,
            [this] (std::string name) {
                PinHolder::CloseSession(name);
            });
    }
    JsonObject jsonObj;
    jsonObj[TAG_PIN_TYPE] = pinType_;
    jsonObj[TAG_PAYLOAD] = payload_;
    jsonObj[TAG_MSG_TYPE] = MSG_TYPE_CREATE_PIN_HOLDER;
    jsonObj[TAG_DM_VERSION] = "";
    std::string message = jsonObj.Dump();
    LOGI("message type is: %{public}d, pin type is: %{public}d.",
        MSG_TYPE_CREATE_PIN_HOLDER, pinType_);
    int32_t ret = session_->SendData(sessionId_, message);
    int32_t bizStage = static_cast<int32_t>(PinHolderStage::SEND_CREATE_PIN_HOLDER_MSG);
    DmRadarHelper::GetInstance().ReportSendOrReceiveHolderMsg(bizStage,
        std::string("CreateGeneratePinHolderMsg"), "");
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed, ret: %{public}d.", ret);
        listener_->OnCreateResult(processInfo_, ret);
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::CREATE_RESULT, ret, "");
        return ret;
    }
    return ret;
}

int32_t PinHolder::ParseMsgType(const std::string &message)
{
    JsonObject jsonObject(message);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequest jsonStr error");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("err json string.");
        return ERR_DM_FAILED;
    }
    int32_t msgType = jsonObject[TAG_MSG_TYPE].Get<int32_t>();
    return msgType;
}

void PinHolder::ProcessCloseSessionMsg(const std::string &message)
{
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("listener or session is nullptr.");
        return;
    }
    LOGI("CloseSessionMsg, message type is: %{public}d.", MSG_TYPE_PIN_CLOSE_SESSION);
    session_->CloseSessionServer(sessionId_);
    ResetSessionState();
    listener_->OnCreateResult(processInfo_, ERR_DM_CREATE_PIN_HOLDER_BUSY);
}

void PinHolder::ProcessCreateMsg(const std::string &message)
{
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("listener or session is nullptr.");
        return;
    }
    JsonObject jsonObject(message);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequest jsonStr error");
        return;
    }
    if (!IsInt32(jsonObject, TAG_PIN_TYPE) || !IsString(jsonObject, TAG_PAYLOAD)) {
        LOGE("err json string.");
        return;
    }
    DmPinType pinType = static_cast<DmPinType>(jsonObject[TAG_PIN_TYPE].Get<int32_t>());
    std::string payload = jsonObject[TAG_PAYLOAD].Get<std::string>();
    isRemoteSupported_ = jsonObject.Contains(TAG_DM_VERSION);
    int32_t bizStage = static_cast<int32_t>(PinHolderStage::RECEIVE_CREATE_PIN_HOLDER_MSG);
    DmRadarHelper::GetInstance().ReportSendOrReceiveHolderMsg(bizStage, std::string("ProcessCreateMsg"), "");
    JsonObject jsonObj;
    jsonObj[TAG_MSG_TYPE] = MSG_TYPE_CREATE_PIN_HOLDER_RESP;
    if (sinkState_ != SINK_INIT || sourceState_ != SOURCE_INIT) {
        LOGE("invalid state: sinkState=%{public}d, sourceState=%{public}d.", sinkState_, sourceState_);
        jsonObj[TAG_REPLY] = REPLY_FAILED;
    } else {
        jsonObj[TAG_REPLY] = REPLY_SUCCESS;
        sinkState_ = SINK_CREATE;
        sourceState_ = SOURCE_CREATE;
        listener_->OnPinHolderCreate(processInfo_, remoteDeviceId_, pinType, payload);
        JsonObject jsonContent;
        jsonContent[TAG_PIN_TYPE] = pinType;
        jsonContent[TAG_PAYLOAD] = payload;
        jsonContent[TAG_REMOTE_DEVICE_ID] = remoteDeviceId_;
        std::string content = jsonContent.Dump();
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::CREATE, DM_OK, content);
    }
    jsonObj[TAG_DM_VERSION] = "";

    std::string msg = jsonObj.Dump();
    LOGI("message type is: %{public}d.", MSG_TYPE_CREATE_PIN_HOLDER_RESP);
    int32_t ret = session_->SendData(sessionId_, msg);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed, ret: %{public}d.", ret);
        return;
    }
}

void PinHolder::ProcessCreateRespMsg(const std::string &message)
{
    JsonObject jsonObject(message);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequest jsonStr error.");
        return;
    }
    if (!IsInt32(jsonObject, TAG_REPLY)) {
        LOGE("err json string.");
        return;
    }
    isRemoteSupported_ = jsonObject.Contains(TAG_DM_VERSION);
    int32_t reply = jsonObject[TAG_REPLY].Get<int32_t>();
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("listener or session is nullptr.");
        return;
    }
    if (reply == REPLY_SUCCESS) {
        listener_->OnCreateResult(processInfo_, DM_OK);
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::CREATE_RESULT, DM_OK, "");
        sourceState_ = SOURCE_CREATE;
        sinkState_ = SINK_CREATE;
    } else {
        LOGE("remote state is wrong.");
        listener_->OnCreateResult(processInfo_, ERR_DM_FAILED);
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::CREATE_RESULT, ERR_DM_FAILED, "");
        session_->CloseSessionServer(sessionId_);
        destroyState_ = STATE_REMOTE_WRONG;
        ResetSessionState();
    }
}

void PinHolder::ProcessDestroyMsg(const std::string &message)
{
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("listener or session is nullptr.");
        return;
    }
    JsonObject jsonObject(message);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequest jsonStr error.");
        return;
    }
    if (!IsInt32(jsonObject, TAG_PIN_TYPE) || !IsString(jsonObject, TAG_PAYLOAD)) {
        LOGE("err json string.");
        return;
    }
    DmPinType pinType = static_cast<DmPinType>(jsonObject[TAG_PIN_TYPE].Get<int32_t>());
    std::string payload = jsonObject[TAG_PAYLOAD].Get<std::string>();
    int32_t bizStage = static_cast<int32_t>(PinHolderStage::RECEIVE_DESTROY_PIN_HOLDER_MSG);
    DmRadarHelper::GetInstance().ReportSendOrReceiveHolderMsg(bizStage, std::string("ProcessDestroyMsg"), "");
    JsonObject jsonObj;
    jsonObj[TAG_MSG_TYPE] = MSG_TYPE_DESTROY_PIN_HOLDER_RESP;
    if (sinkState_ != SINK_CREATE || sourceState_ != SOURCE_CREATE) {
        LOGE("invalid state for destroy: sinkState=%{public}d, sourceState=%{public}d.", sinkState_, sourceState_);
        jsonObj[TAG_REPLY] = REPLY_FAILED;
    } else {
        jsonObj[TAG_REPLY] = REPLY_SUCCESS;
        if (!isDestroy_.load()) {
            listener_->OnPinHolderDestroy(processInfo_, pinType, payload);
            JsonObject jsonContent;
            jsonContent[TAG_PIN_TYPE] = pinType;
            jsonContent[TAG_PAYLOAD] = payload;
            std::string content = jsonContent.Dump();
            listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::DESTROY, DM_OK, content);
            isDestroy_.store(true);
        }
        (void)memset_s(&payload[0], payload.size(), 0, payload.size());
        ResetSessionState();
    }

    std::string msg = jsonObj.Dump();
    LOGI("message type is: %{public}d.", MSG_TYPE_DESTROY_PIN_HOLDER_RESP);
    int32_t ret = session_->SendData(sessionId_, msg);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed, ret: %{public}d.", ret);
        return;
    }
}

void PinHolder::CloseSession(const std::string &name)
{
    LOGI("start timer name %{public}s.", name.c_str());
    if (session_ == nullptr) {
        LOGE("session is nullptr.");
        return;
    }
    JsonObject jsonObj;
    jsonObj[DM_CONNECTION_DISCONNECTED] = true;
    std::string payload = jsonObj.Dump();
    if (listener_ != nullptr && !isDestroy_.load()) {
        listener_->OnPinHolderDestroy(processInfo_, pinType_, payload);
        JsonObject jsonContent;
        jsonContent[TAG_PIN_TYPE] = pinType_;
        jsonContent[TAG_PAYLOAD] = payload;
        std::string content = jsonContent.Dump();
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::DESTROY, ERR_DM_TIME_OUT, content);
        isDestroy_.store(true);
    }
    session_->CloseSessionServer(sessionId_);
    if (timer_ != nullptr) {
        timer_->DeleteAll();
    }
    destroyState_ = STATE_TIME_OUT;
    ResetSessionState();
}

void PinHolder::ProcessDestroyResMsg(const std::string &message)
{
    JsonObject jsonObject(message);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequest jsonStr error.");
        return;
    }
    if (!IsInt32(jsonObject, TAG_REPLY)) {
        LOGE("err json string.");
        return;
    }
    int32_t reply = jsonObject[TAG_REPLY].Get<int32_t>();
    if (listener_ == nullptr || session_ == nullptr) {
        LOGE("listener or session is nullptr.");
        return;
    }
    if (reply == REPLY_SUCCESS) {
        listener_->OnDestroyResult(processInfo_, DM_OK);
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::DESTROY_RESULT, DM_OK, "");
        if (timer_ != nullptr) {
            timer_->DeleteAll();
        }
    } else {
        LOGE("remote state is wrong.");
        listener_->OnDestroyResult(processInfo_, ERR_DM_FAILED);
        listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::DESTROY_RESULT, ERR_DM_FAILED, "");
    }
    session_->CloseSessionServer(sessionId_);
    ResetSessionState();
}

void PinHolder::OnDataReceived(int32_t sessionId, std::string message)
{
    if (sessionId < 0) {
        LOGE("invalid sessionId: %{public}d.", sessionId);
        return;
    }
    int32_t msgType = ParseMsgType(message);
    LOGI("msgType: %{public}d.", msgType);
    int32_t sessionSide = GetSessionSide(sessionId);
    if (sessionSide == SESSION_SIDE_SERVER && sessionId != sessionId_) {
        LOGE("another session opened, close this sessionId: %{public}d.", sessionId);
        JsonObject jsonObj;
        jsonObj[TAG_MSG_TYPE] = MSG_TYPE_PIN_CLOSE_SESSION;
        std::string msg = jsonObj.Dump();
        int32_t ret = session_->SendData(sessionId, msg);
        if (ret != DM_OK) {
            LOGE("[SOFTBUS] SendBytes failed. ret: %{public}d.", ret);
            listener_->OnPinHolderEvent(processInfo_, DmPinHolderEvent::CREATE_RESULT, ERR_DM_FAILED, "");
        }
        (void)memset_s(&message[0], message.size(), 0, message.size());
        return;
    }
    switch (msgType) {
        case MSG_TYPE_PIN_CLOSE_SESSION:
            ProcessCloseSessionMsg(message);
            break;
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
        case MSG_TYPE_PIN_HOLDER_CHANGE:
            ProcessChangeMsg(message);
            break;
        case MSG_TYPE_PIN_HOLDER_CHANGE_RESP:
            ProcessChangeRespMsg(message);
            break;
        default:
            break;
    }
    (void)memset_s(&message[0], message.size(), 0, message.size());
}

void PinHolder::GetPeerDeviceId(int32_t sessionId, std::string &udidHash)
{
    char peerDeviceId[DEVICE_UUID_LENGTH] = {0};
    int32_t ret = ::GetPeerDeviceId(sessionId, &peerDeviceId[0], DEVICE_UUID_LENGTH);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetPeerDeviceId failed for session: %{public}d.", sessionId);
        udidHash = "";
        (void)memset_s(peerDeviceId, sizeof(peerDeviceId), 0, sizeof(peerDeviceId));
        return;
    }
    std::string deviceId = peerDeviceId;
    (void)memset_s(peerDeviceId, sizeof(peerDeviceId), 0, sizeof(peerDeviceId));
    char udidHashTmp[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(deviceId, reinterpret_cast<uint8_t *>(udidHashTmp)) != DM_OK) {
        LOGE("get udidhash failed.");
        udidHash = "";
        (void)memset_s(&deviceId[0], deviceId.size(), 0, deviceId.size());
        (void)memset_s(udidHashTmp, sizeof(udidHashTmp), 0, sizeof(udidHashTmp));
        return;
    }
    udidHash = udidHashTmp;
    LOGI("udid hash: %{public}s success.", GetAnonyString(udidHash).c_str());
    (void)memset_s(&deviceId[0], deviceId.size(), 0, deviceId.size());
    (void)memset_s(udidHashTmp, sizeof(udidHashTmp), 0, sizeof(udidHashTmp));
}
} // namespace DistributedHardware
} // namespace OHOS
