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
std::shared_ptr<ISoftbusSessionCallback> SoftbusSession::sessionCallback_ = nullptr;
constexpr const char* DM_HITRACE_AUTH_TO_OPPEN_SESSION = "DM_HITRACE_AUTH_TO_OPPEN_SESSION";

SoftbusSession::SoftbusSession()
{
    LOGD("SoftbusSession constructor.");
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
        LOGE("[SOFTBUS]addrInfo is nullptr. sessionId: %d.", sessionId);
        return sessionId;
    }
    sessionId = ::OpenAuthSession(DM_SESSION_NAME, addrInfo, 1, nullptr);
    if (sessionId < 0) {
        LOGE("[SOFTBUS]open session error, sessionId: %d.", sessionId);
        return sessionId;
    }
    DmTraceEnd();
    LOGI("OpenAuthSession success. sessionId: %d.", sessionId);
    return sessionId;
}

int32_t SoftbusSession::CloseAuthSession(int32_t sessionId)
{
    LOGD("CloseAuthSession.");
    ::CloseSession(sessionId);
    return DM_OK;
}

int32_t SoftbusSession::GetPeerDeviceId(int32_t sessionId, std::string &peerDevId)
{
    char peerDeviceId[DEVICE_UUID_LENGTH] = {0};
    int32_t ret = ::GetPeerDeviceId(sessionId, &peerDeviceId[0], DEVICE_UUID_LENGTH);
    if (ret == DM_OK) {
        peerDevId = peerDeviceId;
        LOGI("[SOFTBUS]GetPeerDeviceId success for session: %d, peerDeviceId: %s.", sessionId,
            GetAnonyString(peerDevId).c_str());
        return ERR_DM_FAILED;
    }
    LOGE("[SOFTBUS]GetPeerDeviceId failed for session: %d, ret: %d.", sessionId, ret);
    peerDevId = "";
    return DM_OK;
}

int32_t SoftbusSession::SendData(int32_t sessionId, std::string &message)
{
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("extrasJson error, message: %s.", GetAnonyString(message).c_str());
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, TAG_MSG_TYPE)) {
        LOGE("SoftbusSession::SendData err json string.");
        return ERR_DM_FAILED;
    }
    int32_t msgType = jsonObject[TAG_MSG_TYPE].get<int32_t>();
    LOGI("start, msgType: %d.", msgType);
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
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
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
    LOGD("OnSessionOpened, success, sessionId: %d.", sessionId);
    return DM_OK;
}

void SoftbusSession::OnSessionClosed(int sessionId)
{
    LOGD("OnSessionClosed, sessionId: %d.", sessionId);
}

void SoftbusSession::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (sessionId < 0 || data == nullptr || dataLen <= 0) {
        LOGI("[SOFTBUS]fail to receive data from softbus with sessionId: %d, dataLen: %d.", sessionId, dataLen);
        return;
    }
    LOGI("start, sessionId: %d, dataLen: %d.", sessionId, dataLen);
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

void SoftbusSession::OnUnbindSessionOpened(int sessionId, int result)
{
    int32_t sessionSide = GetSessionSide(sessionId);
    sessionCallback_->OnUnbindSessionOpened(sessionId, sessionSide, result);
    LOGD("OnUnbindSessionOpened, success, sessionId: %d.", sessionId);
}

int32_t SoftbusSession::OpenUnbindSession(const std::string &netWorkId)
{
    int dataType = TYPE_BYTES;
    int streamType = INVALID;
    SessionAttribute attr = { 0 };
    attr.dataType = dataType;
    attr.linkTypeNum = LINK_TYPE_MAX;
    LinkType linkTypeList[LINK_TYPE_MAX] = {
        LINK_TYPE_WIFI_P2P,
        LINK_TYPE_WIFI_WLAN_5G,
        LINK_TYPE_WIFI_WLAN_2G,
        LINK_TYPE_BR,
    };
    if (memcpy_s(attr.linkType, sizeof(attr.linkType), linkTypeList, sizeof(linkTypeList)) != DM_OK) {
        LOGE("OpenUnbindSession: Data copy failed.");
        return ERR_DM_FAILED;
    }
    attr.attr.streamAttr.streamType = streamType;
    int32_t sessionId = OpenSession(DM_UNBIND_SESSION_NAME, DM_UNBIND_SESSION_NAME, netWorkId.c_str(), "0", &attr);
    if (sessionId < 0) {
        LOGE("[SOFTBUS]open OpenUnbindSession error, sessionId: %d.", sessionId);
        return sessionId;
    }
    LOGI("OpenUnbindSession success. sessionId: %d.", sessionId);
    return sessionId;
}
} // namespace DistributedHardware
} // namespace OHOS
