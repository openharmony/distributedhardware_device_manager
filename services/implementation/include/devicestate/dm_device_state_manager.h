/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "dm_timer.h"
#include "hichain_auth_connector.h"
#include "hichain_connector.h"
#include "idevice_manager_service_listener.h"
#include "multiple_user_connector.h"
#include "softbus_connector.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "deviceprofile_connector.h"
#include "ffrt.h"
#endif

namespace OHOS {
namespace DistributedHardware {
#define OFFLINE_TIMEOUT 300
struct StateTimerInfo {
    std::string timerName;
    std::string peerUdid;
    int32_t peerUserId;
    int32_t localUserId;
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

    void SaveNotifyEventInfos(const int32_t eventId, const std::string &deviceId);
    void GetNotifyEventInfos(std::vector<DmDeviceInfo> &deviceList);
    int32_t ProcNotifyEvent(const int32_t eventId, const std::string &deviceId);
    void SaveOnlineDeviceInfo(const DmDeviceInfo &info);
    void DeleteOfflineDeviceInfo(const DmDeviceInfo &info);
    void HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo,
        std::vector<ProcessInfo> &processInfoVec, const std::string &peerUdid);
    void OnDbReady(const std::string &pkgName, const std::string &uuid);
    void ChangeDeviceInfo(const DmDeviceInfo &info);
    int32_t RegisterSoftbusStateCallback();
    void OnDeviceOnline(std::string deviceId, int32_t authForm);
    void OnDeviceOffline(std::string deviceId);
    std::string GetUdidByNetWorkId(std::string networkId);
    bool CheckIsOnline(const std::string &udid);
    void DeleteOffLineTimer(const std::string &peerUdid);
    void HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo, std::vector<ProcessInfo> &processInfos);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    void StartDelTimerByDP(const std::string &peerUdid, int32_t peerUserId, int32_t localUserId);
#endif
private:
    void StartEventThread();
    void StopEventThread();
    void ThreadLoop();
    int32_t AddTask(const std::shared_ptr<NotifyEvent> &task);
    void RunTask(const std::shared_ptr<NotifyEvent> &task);
    DmAuthForm GetAuthForm(const std::string &networkId);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    void RegisterOffLineTimer(const DmDeviceInfo &deviceInfo);
    void StartOffLineTimer(const std::string &peerUdid);
    void DeleteTimeOutGroup(const std::string &timerName);
    int32_t DeleteGroupByDP(const std::string &peerUdid, int32_t peerUserId, int32_t localUserId);
    void DeleteCredential(const DmOfflineParam &offlineParam, const std::string &peerUdid, int32_t localUserId);
    int32_t DeleteSkCredAndAcl(const std::vector<DmAclIdParam> &acls);
    void DeleteCredential(const DmAclIdParam &acl);
#endif
    void ProcessDeviceStateChange(const DmDeviceState devState, const DmDeviceInfo &devInfo,
        std::vector<ProcessInfo> &processInfoVec);
private:
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::mutex timerMapMutex_;
    ffrt::mutex remoteDeviceInfosMutex_;
#else
    std::mutex remoteDeviceInfosMutex_;
#endif
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::map<std::string, DmDeviceInfo> remoteDeviceInfos_;
    std::map<std::string, DmDeviceInfo> stateDeviceInfos_;
    std::map<std::string, StateTimerInfo> stateTimerInfoMap_;
    std::shared_ptr<DmTimer> timer_;
    std::shared_ptr<HiChainConnector> hiChainConnector_;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector_;
    std::string decisionSoName_;
    NotifyTask eventTask_;
    std::mutex notifyEventInfosMutex_;
    std::map<std::string, DmDeviceInfo> notifyEventInfos_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICE_STATE_MANAGER_H
