/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "dm_log.h"
#include "nlohmann/json.hpp"
#include "softbus_connector.h"

namespace OHOS {
namespace DistributedHardware {
std::map<std::string, std::shared_ptr<ISoftbusSessionCallback>> SoftbusSession::sessionCallbackMap_ = {};

SoftbusSession::SoftbusSession()
{
    ISessionListener sessionListener = {.OnSessionOpened = SoftbusSession::OnSessionOpened,
                                        .OnSessionClosed = SoftbusSession::OnSessionClosed,
                                        .OnBytesReceived = SoftbusSession::OnBytesReceived,
                                        .OnMessageReceived = nullptr,
                                        .OnStreamReceived = nullptr};
    int32_t ret = CreateSessionServer(DM_PKG_NAME.c_str(), DM_SESSION_NAME.c_str(), &sessionListener);
    if (ret != DM_OK) {
        LOGE("CreateSessionServer failed");
    } else {
        LOGI("CreateSessionServer ok");
    }
}

SoftbusSession::~SoftbusSession()
{
    RemoveSessionServer(DM_PKG_NAME.c_str(), DM_SESSION_NAME.c_str());
}

int32_t SoftbusSession::RegisterSessionCallback(const std::string &pkgName,
                                                std::shared_ptr<ISoftbusSessionCallback> callback)
{
    sessionCallbackMap_[pkgName] = callback;
    return DM_OK;
}

int32_t SoftbusSession::UnRegisterSessionCallback(const std::string &pkgName)
{
    sessionCallbackMap_.erase(pkgName);
    return DM_OK;
}

int32_t SoftbusSession::OpenAuthSession(const std::string &deviceId)
{
    LOGE("SoftbusSession::OpenAuthSession");
    int32_t sessionId = -1;
    std::string connectAddr;
    ConnectionAddr *addrInfo = SoftbusConnector::GetConnectAddr(deviceId, connectAddr);
    if (addrInfo == nullptr) {
        LOGE("GetConnectAddr error");
        return sessionId;
    }
    sessionId = ::OpenAuthSession(DM_SESSION_NAME.c_str(), addrInfo, 1, nullptr);
    if (sessionId < 0) {
        LOGE("open session error, ret:%d", sessionId);
        return sessionId;
    }
    LOGI("SoftbusSession::OpenAuthSession success. sessionId is:%d", sessionId);
    return sessionId;
}

int32_t SoftbusSession::CloseAuthSession(int32_t sessionId)
{
    LOGI("SoftbusSession::CloseAuthSession");
    ::CloseSession(sessionId);
    return DM_OK;
}

int32_t SoftbusSession::GetPeerDeviceId(int32_t sessionId, std::string &peerDevId)
{
    char peerDeviceId[DEVICE_UUID_LENGTH] = {0};
    int32_t ret = ::GetPeerDeviceId(sessionId, &peerDeviceId[0], DEVICE_UUID_LENGTH);
    if (ret == 0) {
        peerDevId = peerDeviceId;
        LOGI("GetPeerDeviceId success for session:%d, peerDeviceId:%s", sessionId, GetAnonyString(peerDevId).c_str());
        return DM_FAILED;
    }
    LOGE("GetPeerDeviceId failed for session:%d", sessionId);
    peerDevId = "";
    return DM_OK;
}

int32_t SoftbusSession::SendData(int32_t sessionId, std::string &message)
{
    LOGE("SendData Start");
    nlohmann::json jsonObject = nlohmann::json::parse(message, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("extrasJson error");
        return DM_FAILED;
    }
    int32_t msgType = jsonObject[TAG_TYPE];
    LOGI("AuthMessageProcessor::ParseAuthRequestMessage  msgType = %d", msgType);
    bool isCryptoSupport = false;
    for (auto &iter : sessionCallbackMap_) {
        iter.second->GetIsCryptoSupport(isCryptoSupport);
    }
    if (isCryptoSupport) {
        LOGI("SoftbusSession::SendData Start encryption");
    }
    int32_t ret = SendBytes(sessionId, message.c_str(), strlen(message.c_str()));
    if (ret != DM_OK) {
        LOGE("SendData Start failed");
        return DM_FAILED;
    }
    LOGE("SendData Start success");
    return DM_OK;
}

int32_t SoftbusSession::OnSessionOpened(int32_t sessionId, int32_t result)
{
    int32_t sessionSide = GetSessionSide(sessionId);
    for (auto &iter : sessionCallbackMap_) {
        iter.second->OnSessionOpened(iter.first, sessionId, sessionSide, result);
    }
    LOGI("OnSessionOpened, success:");
    return DM_OK;
}

void SoftbusSession::OnSessionClosed(int32_t sessionId)
{
    LOGI("OnSessionClosed, sessionId:%d", sessionId);
}

void SoftbusSession::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    LOGI("OnBytesReceived, sessionId:%d, dataLen:%d", sessionId, dataLen);
    if (sessionId < 0 || data == nullptr || dataLen <= 0) {
        LOGI("OnBytesReceived param check failed");
        return;
    }
    bool isCryptoSupport = false;
    for (auto &iter : sessionCallbackMap_) {
        iter.second->GetIsCryptoSupport(isCryptoSupport);
    }
    if (isCryptoSupport) {
        LOGI("SoftbusSession::OnBytesReceived Start decryption");
    }
    std::string message = std::string((const char *)data, dataLen);
    for (auto &iter : sessionCallbackMap_) {
        iter.second->OnDataReceived(iter.first, sessionId, message);
        return;
    }
    LOGI("OnBytesReceived completed");
}
} // namespace DistributedHardware
} // namespace OHOS
