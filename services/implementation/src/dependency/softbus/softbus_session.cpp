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
#include "dm_auth_manager.h"
#include "dm_constants.h"
#include "dm_dfx_constants.h"
#include "dm_hitrace.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"
#include "softbus_connector.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
static QosTV g_qosInfo[] = {
    { .qos = QOS_TYPE_MIN_BW, .value = 64 * 1024 },
    { .qos = QOS_TYPE_MAX_LATENCY, .value = 10000 },
    { .qos = QOS_TYPE_MIN_LATENCY, .value = 2500 },
};
static uint32_t g_qosTVParamIndex = static_cast<uint32_t>(sizeof(g_qosInfo) / sizeof(g_qosInfo[0]));
}

std::shared_ptr<ISoftbusSessionCallback> SoftbusSession::sessionCallback_ = nullptr;
constexpr const char* DM_HITRACE_AUTH_TO_OPPEN_SESSION = "DM_HITRACE_AUTH_TO_OPPEN_SESSION";
constexpr int32_t INTERCEPT_STRING_LENGTH = 20;

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
    for (uint32_t idx = 0; idx < qosCount; idx++) {
        LOGI("QosTV: type: %{public}d, value: %{public}d", (int32_t)qos[idx].qos, qos[idx].value);
    }
}

SoftbusSession::SoftbusSession()
{
    LOGD("SoftbusSession constructor.");
    iSocketListener_.OnShutdown = OnShutdown;
    iSocketListener_.OnBytes = OnBytes;
    iSocketListener_.OnQos = OnQos;
    iSocketListener_.OnFile = nullptr;
    iSocketListener_.OnMessage = nullptr;
    iSocketListener_.OnBind = nullptr;
    iSocketListener_.OnStream = nullptr;
    iSocketListener_.OnError = nullptr;
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

int32_t SoftbusSession::CloseAuthSession(int32_t sessionId)
{
    LOGI("CloseAuthSession.");
    ::CloseSession(sessionId);
    return DM_OK;
}

int32_t SoftbusSession::CloseUnbindSession(int32_t socket)
{
    LOGI("CloseUnbindSession.");
    Shutdown(socket);
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
        return ERR_DM_FAILED;
    }
    LOGE("[SOFTBUS]GetPeerDeviceId failed for session: %{public}d, ret: %{public}d.", sessionId, ret);
    peerDevId = "";
    return DM_OK;
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
    if (SendBytes(sessionId, message.c_str(), strlen(message.c_str())) != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed.");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t SoftbusSession::SendHeartbeatData(int32_t sessionId, std::string &message)
{
    if (SendBytes(sessionId, message.c_str(), strlen(message.c_str())) != DM_OK) {
        LOGE("[SOFTBUS]SendHeartbeatData failed.");
        return ERR_DM_FAILED;
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
    LOGD("OnSessionClosed, sessionId: %{public}d.", sessionId);
}

void SoftbusSession::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (sessionId < 0 || data == nullptr || dataLen <= 0) {
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

void SoftbusSession::OnUnbindSessionOpened(int32_t socket, PeerSocketInfo info)
{
    sessionCallback_->OnUnbindSessionOpened(socket, info);
    LOGI("SoftbusSession::OnUnbindSessionOpened success, socket: %{public}d.", socket);
}

int32_t SoftbusSession::OpenUnbindSession(const std::string &netWorkId)
{
    std::string localSessionName = DM_UNBIND_SESSION_NAME + netWorkId.substr(0, INTERCEPT_STRING_LENGTH);
    SocketInfo info = {
        .name = const_cast<char*>(localSessionName.c_str()),
        .peerName = const_cast<char*>(DM_UNBIND_SESSION_NAME),
        .peerNetworkId = const_cast<char*>(netWorkId.c_str()),
        .pkgName = const_cast<char*>(DM_PKG_NAME),
        .dataType = DATA_TYPE_BYTES
    };

    int32_t socket = Socket(info);
    if (socket <= 0) {
        LOGE("[SOFTBUS]create socket failed, socket: %{public}d", socket);
        return ERR_DM_FAILED;
    }

    int32_t ret = Bind(socket, g_qosInfo, g_qosTVParamIndex, &iSocketListener_);
    if (ret < DM_OK) {
        LOGE("[SOFTBUS]OpenUnbindSession failed, netWorkId: %{public}s, socket: %{public}d",
            GetAnonyString(netWorkId).c_str(), socket);
        sessionCallback_->BindSocketFail();
        Shutdown(socket);
        return ERR_DM_FAILED;
    }
    LOGI("OpenUnbindSession success. socket: %{public}d.", socket);
    sessionCallback_->BindSocketSuccess(socket);
    return socket;
}
} // namespace DistributedHardware
} // namespace OHOS
