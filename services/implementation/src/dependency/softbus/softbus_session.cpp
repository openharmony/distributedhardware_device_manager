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

#include "softbus_session.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_dfx_constants.h"
#include "dm_hitrace.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"
#include "softbus_connector.h"
#include "softbus_error_code.h"
#ifndef DEVICE_MANAGER_COMMON_FLAG
#include "session_ex.h"
#endif

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<ISoftbusSessionCallback> SoftbusSession::sessionCallback_ = nullptr;
constexpr const char* DM_HITRACE_AUTH_TO_OPPEN_SESSION = "DM_HITRACE_AUTH_TO_OPPEN_SESSION";
constexpr int32_t MAX_DATA_LEN = 65535;

static void OnShutdown(int32_t socket, ShutdownReason reason)
{
    LOGI("[SOFTBUS]OnShutdown socket : %{public}d, reason: %{public}d", socket, (int32_t)reason);
    SoftbusSession::OnSessionClosed(socket);
}

static void OnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    LOGI("[SOFTBUS]OnBytes socket : %{public}d", socket);
    SoftbusSession::OnBytesReceived(socket, data, dataLen);
}

static void OnQos(int32_t socket, QoSEvent eventId, const QosTV *qos, uint32_t qosCount)
{
    LOGI("[SOFTBUS]OnQos, socket: %{public}d, QoSEvent: %{public}d, qosCount: %{public}u", socket, (int32_t)eventId,
        qosCount);
    if (qosCount > QOS_TYPE_BUTT) {
        LOGE("OnQos invalid qosCount");
        return;
    }
    for (uint32_t idx = 0; idx < qosCount; idx++) {
        LOGI("QosTV: type: %{public}d, value: %{public}d", (int32_t)qos[idx].qos, qos[idx].value);
    }
}

SoftbusSession::SoftbusSession()
{
    LOGD("SoftbusSession constructor.");
    if (memset_s(&iSocketListener_, sizeof(ISocketListener), 0, sizeof(ISocketListener)) != DM_OK) {
        LOGE("SoftbusSession::SoftbusSession memset_s failed.");
        return;
    }

    iSocketListener_.OnShutdown = OnShutdown;
    iSocketListener_.OnBytes = OnBytes;
    iSocketListener_.OnQos = OnQos;
    iSocketListener_.OnFile = nullptr;
    iSocketListener_.OnMessage = nullptr;
    iSocketListener_.OnBind = nullptr;
    iSocketListener_.OnStream = nullptr;
    iSocketListener_.OnError = nullptr;
    iSocketListener_.OnNegotiate = nullptr;
}

SoftbusSession::~SoftbusSession()
{
    LOGD("SoftbusSession destructor.");
}

int32_t SoftbusSession::RegisterSessionCallback(std::shared_ptr<ISoftbusSessionCallback> callback)
{
    sessionCallback_ = callback;
    return DM_OK;
}

int32_t SoftbusSession::UnRegisterSessionCallback()
{
    sessionCallback_ = nullptr;
    return DM_OK;
}

int32_t SoftbusSession::OpenAuthSession(const std::string &deviceId)
{
    DmTraceStart(std::string(DM_HITRACE_AUTH_TO_OPPEN_SESSION));
    int32_t sessionId = -1;
    std::string connectAddr;
    ConnectionAddr *addrInfo = SoftbusConnector::GetConnectAddr(deviceId, connectAddr);
    if (addrInfo == nullptr) {
        LOGE("[SOFTBUS]addrInfo is nullptr. sessionId: %{public}d.", sessionId);
        return sessionId;
    }
    sessionId = ::OpenAuthSession(DM_SESSION_NAME, addrInfo, 1, nullptr);
    if (sessionId < 0) {
        LOGE("[SOFTBUS]open session error, sessionId: %{public}d.", sessionId);
        return sessionId;
    }
    DmTraceEnd();
    LOGI("OpenAuthSession success. sessionId: %{public}d.", sessionId);
    return sessionId;
}

int32_t SoftbusSession::OpenAuthSessionWithPara(const std::string &deviceId, int32_t actionId, bool isEnable160m)
{
#ifdef DEVICE_MANAGER_COMMON_FLAG
    LOGE("[SOFTBUS] OpenAuthSessionWithPara no implement");
    return SOFTBUS_NOT_IMPLEMENT;
#else
    DmTraceStart(std::string(DM_HITRACE_AUTH_TO_OPPEN_SESSION));
    LinkPara para;
    para.type = PARA_ACTION;
    para.action.actionId = actionId;
    para.enable160M = isEnable160m;
    para.accountInfo = false;
    int32_t sessionId = ::OpenAuthSessionWithPara(DM_SESSION_NAME, &para);
    if (sessionId < 0) {
        LOGE("[SOFTBUS]open session error, sessionId: %{public}d.", sessionId);
        return sessionId;
    }
    DmTraceEnd();
    LOGI("OpenAuthSessionWithPara success. sessionId: %{public}d.", sessionId);
    return sessionId;
#endif
}

int32_t SoftbusSession::CloseAuthSession(int32_t sessionId)
{
    LOGI("CloseAuthSession.");
    ::CloseSession(sessionId);
    return DM_OK;
}

int32_t SoftbusSession::GetPeerDeviceId(int32_t sessionId, std::string &peerDevId)
{
    char peerDeviceId[DEVICE_UUID_LENGTH] = {0};
    int32_t ret = ::GetPeerDeviceId(sessionId, &peerDeviceId[0], DEVICE_UUID_LENGTH);
    if (ret == DM_OK) {
        peerDevId = peerDeviceId;
        LOGI("[SOFTBUS]GetPeerDeviceId success for session: %{public}d, peerDeviceId: %{public}s.", sessionId,
            GetAnonyString(peerDevId).c_str());
        return DM_OK;
    }
    LOGE("[SOFTBUS]GetPeerDeviceId failed for session: %{public}d, ret: %{public}d.", sessionId, ret);
    peerDevId = "";
    return ret;
}

int32_t SoftbusSession::SendData(int32_t sessionId, std::string &message)
{
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("extrasJson error, message: %{public}s.", GetAnonyString(message).c_str());
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("SoftbusSession::SendData err json string.");
        return ERR_DM_FAILED;
    }
    int32_t msgType = jsonObject[TAG_MSG_TYPE].get<int32_t>();
    LOGI("start, msgType: %{public}d.", msgType);
    if (sessionCallback_->GetIsCryptoSupport()) {
        LOGI("SendData Start encryption.");
    }
    int32_t ret = SendBytes(sessionId, message.c_str(), strlen(message.c_str()));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed.");
        return ret;
    }
    return DM_OK;
}

int32_t SoftbusSession::SendHeartbeatData(int32_t sessionId, std::string &message)
{
    int32_t ret = SendBytes(sessionId, message.c_str(), strlen(message.c_str()));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendHeartbeatData failed.");
        return ret;
    }
    return DM_OK;
}

int SoftbusSession::OnSessionOpened(int sessionId, int result)
{
    int32_t sessionSide = GetSessionSide(sessionId);
    sessionCallback_->OnSessionOpened(sessionId, sessionSide, result);
    LOGD("OnSessionOpened, success, sessionId: %{public}d.", sessionId);
    return DM_OK;
}

void SoftbusSession::OnSessionClosed(int sessionId)
{
    LOGI("OnSessionClosed, sessionId: %{public}d.", sessionId);
    CHECK_NULL_VOID(sessionCallback_);
    sessionCallback_->OnSessionClosed(sessionId);
}

void SoftbusSession::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (sessionId < 0 || data == nullptr || dataLen <= 0 || dataLen > MAX_DATA_LEN) {
        LOGI("[SOFTBUS]fail to receive data from softbus with sessionId: %{public}d, dataLen: %{public}d.", sessionId,
            dataLen);
        return;
    }
    LOGI("start, sessionId: %{public}d, dataLen: %{public}d.", sessionId, dataLen);
    if (sessionCallback_->GetIsCryptoSupport()) {
        LOGI("Start decryption.");
    }
    std::string message = std::string(reinterpret_cast<const char *>(data), dataLen);
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return;
    }
    if (!IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("err json string, first time");
        return;
    }
    if (jsonObject[TAG_MSG_TYPE].get<int32_t>() == AUTH_DEVICE_REQ_NEGOTIATE ||
        jsonObject[TAG_MSG_TYPE].get<int32_t>() == AUTH_DEVICE_RESP_NEGOTIATE) {
        sessionCallback_->OnAuthDeviceDataReceived(sessionId, message);
    } else {
        sessionCallback_->OnDataReceived(sessionId, message);
    }
    LOGI("completed.");
}
} // namespace DistributedHardware
} // namespace OHOS
