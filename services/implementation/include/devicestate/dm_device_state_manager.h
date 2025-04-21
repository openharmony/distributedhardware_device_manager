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

#ifndef OHOS_DM_DEVICE_STATE_MANAGER_H
#define OHOS_DM_DEVICE_STATE_MANAGER_H

#include <condition_variable>
#include <memory>
#include <queue>
#include <string>
#include <thread>

#if defined(__LITEOS_M__)
#include "dm_mutex.h"
#else
#include <mutex>
#endif
#include "idevice_manager_service_listener.h"
#include "dm_adapter_manager.h"
#include "softbus_connector.h"
#include "dm_timer.h"
#include "hichain_connector.h"
#include "hichain_auth_connector.h"
#include "multiple_user_connector.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "deviceprofile_connector.h"
#endif

namespace OHOS {
namespace DistributedHardware {
#define OFFLINE_TIMEOUT 300
struct StateTimerInfo {
    std::string timerName;
    std::string networkId;
    bool isStart;
};

class NotifyEvent {
public:
    NotifyEvent(int32_t eventId, const std::string &deviceId) : eventId_(eventId), deviceId_(deviceId) {};
    ~NotifyEvent() {};

    int32_t GetEventId() const
    {
        return eventId_;
    };
    std::string GetDeviceId() const
    {
        return deviceId_;
    };
private:
    int32_t eventId_;
    std::string deviceId_;
};

typedef struct NotifyTask {
    std::thread queueThread_;
    std::condition_variable queueCond_;
    std::condition_variable queueFullCond_;
    std::mutex queueMtx_;
    std::queue<std::shared_ptr<NotifyEvent>> queue_;
    bool threadRunning_ = false;
} NotifyTask;

class DmDeviceStateManager final : public ISoftbusStateCallback,
                                   public std::enable_shared_from_this<DmDeviceStateManager> {
public:
    DmDeviceStateManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                         std::shared_ptr<IDeviceManagerServiceListener> listener,
                         std::shared_ptr<HiChainConnector> hiChainConnector,
                         std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector);
    ~DmDeviceStateManager();

    int32_t ProcNotifyEvent(const int32_t eventId, const std::string &deviceId);
    void SaveOnlineDeviceInfo(const DmDeviceInfo &info);
    void DeleteOfflineDeviceInfo(const DmDeviceInfo &info);
    void HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo);
    void OnDbReady(const std::string &pkgName, const std::string &uuid);
    void RegisterOffLineTimer(const DmDeviceInfo &deviceInfo);
    void StartOffLineTimer(const DmDeviceInfo &deviceInfo);
    void DeleteTimeOutGroup(std::string name);
    void ChangeDeviceInfo(const DmDeviceInfo &info);
    int32_t RegisterSoftbusStateCallback();
    void OnDeviceOnline(std::string deviceId, int32_t authForm);
    void OnDeviceOffline(std::string deviceId);
    std::string GetUdidByNetWorkId(std::string networkId);
    bool CheckIsOnline(const std::string &udid);
    void DeleteOffLineTimer(std::string udidHash);
    void HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    int32_t DeleteSkCredAndAcl(const std::vector<DmAclIdParam> &acls);
#endif
private:
    void StartEventThread();
    void StopEventThread();
    void ThreadLoop();
    int32_t AddTask(const std::shared_ptr<NotifyEvent> &task);
    void RunTask(const std::shared_ptr<NotifyEvent> &task);
    DmAuthForm GetAuthForm(const std::string &networkId);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    int32_t DeleteGroupByDP(const std::string &deviceId);
    void DeleteCredential(DmOfflineParam offlineParam, const std::string &deviceId);
#endif
    void ProcessDeviceStateChange(const DmDeviceState devState, const DmDeviceInfo &devInfo);
private:
    std::mutex timerMapMutex_;
    std::mutex remoteDeviceInfosMutex_;
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::map<std::string, DmDeviceInfo> remoteDeviceInfos_;
    std::map<std::string, DmDeviceInfo> stateDeviceInfos_;
    std::map<std::string, StateTimerInfo> stateTimerInfoMap_;
    std::map<std::string, std::string> udidhash2udidMap_;
    std::shared_ptr<DmTimer> timer_;
    std::shared_ptr<HiChainConnector> hiChainConnector_;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector_;
    std::string decisionSoName_;
    NotifyTask eventTask_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICE_STATE_MANAGER_H
