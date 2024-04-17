/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "softbus_adapter.h"
#include "dm_log.h"
#include <securec.h>
#include <unistd.h>

#include "softbus_bus_center.h"
#include "softbus_common.h"
#include "device_manager_service.h"
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
IMPLEMENT_SINGLE_INSTANCE(SoftbusAdapter);
static void DmOnSoftbusSessionBind(int32_t socket, PeerSocketInfo info)
{
    SoftbusAdapter::GetInstance().OnSoftbusSessionOpened(socket, info);
}

static void DmOnSoftbusSessionClosed(int32_t socket, ShutdownReason reason)
{
    SoftbusAdapter::GetInstance().OnSoftbusSessionClosed(socket, reason);
}

static void DmOnBytesReceived(int32_t socket, const void *data, uint32_t dataLen)
{
    SoftbusAdapter::GetInstance().OnBytesReceived(socket, data, dataLen);
}

static void DmOnStreamReceived(int32_t socket, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *frameInfo)
{
    SoftbusAdapter::GetInstance().OnStreamReceived(socket, data, ext, frameInfo);
}

static void DmOnMessageReceived(int32_t socket, const void *data, uint32_t dataLen)
{
    SoftbusAdapter::GetInstance().OnMessageReceived(socket, data, dataLen);
}

static void DmOnQosEvent(int32_t socket, QoSEvent eventId, const QosTV *qos, uint32_t qosCount)
{
    SoftbusAdapter::GetInstance().OnQosEvent(socket, eventId, qos, qosCount);
}

SoftbusAdapter::SoftbusAdapter()
{
    LOGI("SoftbusAdapter");
    iSocketListener_.OnBind = DmOnSoftbusSessionBind;
    iSocketListener_.OnShutdown = DmOnSoftbusSessionClosed;
    iSocketListener_.OnBytes = DmOnBytesReceived;
    iSocketListener_.OnMessage = DmOnMessageReceived;
    iSocketListener_.OnStream = DmOnStreamReceived;
    iSocketListener_.OnQos = DmOnQosEvent;
    iSocketListener_.OnFile = nullptr;
    iSocketListener_.OnError = nullptr;
}

SoftbusAdapter::~SoftbusAdapter()
{
    LOGI("~SoftbusAdapter");
}

int32_t SoftbusAdapter::CreateSoftbusSessionServer(const std::string &pkgname, const std::string &sessionName)
{
    LOGI("SoftbusAdapter::CreateSoftbusSessionServer.");
    SocketInfo info = {
        .name = const_cast<char*>(sessionName.c_str()),
        .pkgName = const_cast<char*>(pkgname.c_str()),
        .dataType = DATA_TYPE_BYTES
    };
    int32_t socket = Socket(info);
    if (socket <= 0) {
        LOGE("[SOFTBUS]create socket failed, socket: %{public}d", socket);
        return ERR_DM_FAILED;
    }
    int32_t ret = Listen(socket, g_qosInfo, g_qosTVParamIndex, &iSocketListener_);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]Socket Listen failed, ret: %{public}d, socket: %{public}d.", ret, socket);
        Shutdown(socket);
        return ERR_DM_FAILED;
    }
    LOGI("SoftbusAdapter::CreateSoftbusSessionServer success.");
    return DM_OK;
}

int32_t SoftbusAdapter::RemoveSoftbusSessionServer(const std::string &pkgname, const std::string &sessionName)
{
    LOGI("SoftbusAdapter::RemoveSoftbusSessionServer");
    if (RemoveSessionServer(pkgname.c_str(), sessionName.c_str()) != DM_OK) {
        LOGE("RemoveSoftbusSessionServer failed.");
        return ERR_DM_FAILED;
    }
    LOGI("SoftbusAdapter::RemoveSoftbusSessionServer success.");
    return DM_OK;
}

void SoftbusAdapter::OnSoftbusSessionOpened(int32_t socket, PeerSocketInfo info)
{
    LOGI("SoftbusAdapter::OnSoftbusSessionOpened socket: %{public}d", socket);
    DeviceManagerService::GetInstance().OnUnbindSessionOpened(socket, info);
}

void SoftbusAdapter::OnSoftbusSessionClosed(int32_t socket, ShutdownReason reason)
{
    (void)reason;
    LOGI("SoftbusAdapter::OnSoftbusSessionClosed socket: %{public}d", socket);
    DeviceManagerService::GetInstance().OnUnbindSessionCloseed(socket);
}

void SoftbusAdapter::OnBytesReceived(int32_t socket, const void *data, uint32_t dataLen)
{
    LOGI("SoftbusAdapter::OnBytesReceived socket: %{public}d", socket);
    DeviceManagerService::GetInstance().OnUnbindBytesReceived(socket, data, dataLen);
}

void SoftbusAdapter::OnStreamReceived(int32_t socket, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *frameInfo)
{
    (void)data;
    (void)ext;
    (void)frameInfo;
    LOGI("SoftbusAdapter::OnStreamReceived, socket: %{public}d", socket);
}

void SoftbusAdapter::OnMessageReceived(int32_t socket, const void *data, unsigned int dataLen) const
{
    (void)data;
    (void)dataLen;
    LOGI("SoftbusAdapter::OnMessageReceived, socket: %{public}d", socket);
}

void SoftbusAdapter::OnQosEvent(int32_t socket, QoSEvent eventId, const QosTV *qos, uint32_t qosCount) const
{
    (void)eventId;
    (void)qos;
    (void)qosCount;
    LOGI("SoftbusAdapter::OnQosEvent, socket: %{public}d", socket);
}
} // namespace DistributedHardware
} // namespace OHOS