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

#include "softbus_adapter.h"

#include <securec.h>
#include <unistd.h>

#include "softbus_bus_center.h"
#include "softbus_common.h"
#include "device_manager_service.h"
namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(SoftbusAdapter);
static int32_t ScreenOnSoftbusSessionOpened(int32_t sessionId, int32_t result)
{
    SoftbusAdapter::GetInstance().OnSoftbusSessionOpened(sessionId, result);
    return DM_OK;
}

static void ScreenOnSoftbusSessionClosed(int32_t sessionId)
{
    SoftbusAdapter::GetInstance().OnSoftbusSessionClosed(sessionId);
}

static void ScreenOnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    SoftbusAdapter::GetInstance().OnBytesReceived(sessionId, data, dataLen);
}

static void ScreenOnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *frameInfo)
{
    SoftbusAdapter::GetInstance().OnStreamReceived(sessionId, data, ext, frameInfo);
}

static void ScreenOnMessageReceived(int sessionId, const void *data, unsigned int dataLen)
{
    SoftbusAdapter::GetInstance().OnMessageReceived(sessionId, data, dataLen);
}

static void ScreenOnQosEvent(int sessionId, int eventId, int tvCount, const QosTv *tvList)
{
    SoftbusAdapter::GetInstance().OnQosEvent(sessionId, eventId, tvCount, tvList);
}

SoftbusAdapter::SoftbusAdapter()
{
    LOGI("SoftbusAdapter");
    sessListener_.OnSessionOpened = ScreenOnSoftbusSessionOpened;
    sessListener_.OnSessionClosed = ScreenOnSoftbusSessionClosed;
    sessListener_.OnBytesReceived = ScreenOnBytesReceived;
    sessListener_.OnStreamReceived = ScreenOnStreamReceived;
    sessListener_.OnMessageReceived = ScreenOnMessageReceived;
    sessListener_.OnQosEvent = ScreenOnQosEvent;
}

SoftbusAdapter::~SoftbusAdapter()
{
    LOGI("~SoftbusAdapter");
}

int32_t SoftbusAdapter::CreateSoftbusSessionServer(const std::string &pkgname, const std::string &sessionName)
{
    LOGI("SoftbusAdapter::CreateSoftbusSessionServer.");
    if (CreateSessionServer(pkgname.c_str(), sessionName.c_str(), &sessListener_) != DM_OK) {
        LOGE("CreateSoftbusSessionServer failed.");
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

void SoftbusAdapter::OnSoftbusSessionOpened(int32_t sessionId, int32_t result)
{
    LOGI("SoftbusAdapter::OnSoftbusSessionOpened");
    DeviceManagerService::GetInstance().OnUnbindSessionOpened(sessionId, result);
}

void SoftbusAdapter::OnSoftbusSessionClosed(int32_t sessionId)
{
    LOGI("SoftbusAdapter::OnSoftbusSessionClosed");
    DeviceManagerService::GetInstance().OnUnbindSessionCloseed(sessionId);
}

void SoftbusAdapter::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    LOGI("SoftbusAdapter::OnBytesReceived");
    DeviceManagerService::GetInstance().OnUnbindBytesReceived(sessionId, data, dataLen);
}

void SoftbusAdapter::OnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *frameInfo)
{
    (void)data;
    (void)ext;
    (void)frameInfo;
    LOGI("SoftbusAdapter::OnStreamReceived, sessionId:%d", sessionId);
}

void SoftbusAdapter::OnMessageReceived(int sessionId, const void *data, unsigned int dataLen) const
{
    (void)data;
    (void)dataLen;
    LOGI("SoftbusAdapter::OnMessageReceived, sessionId:%d", sessionId);
}

void SoftbusAdapter::OnQosEvent(int sessionId, int eventId, int tvCount, const QosTv *tvList) const
{
    (void)eventId;
    (void)tvCount;
    (void)tvList;
    LOGI("SoftbusAdapter::OnQosEvent, sessionId:%d", sessionId);
}
} // namespace DistributedHardware
} // namespace OHOS