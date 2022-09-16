/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
                         std::shared_ptr<HiChainConnector> hiChainConnector);
    ~DmDeviceStateManager();

    /**
     * @tc.name: DmDeviceStateManager::SaveOnlineDeviceInfo
     * @tc.desc: Save Online DeviceInfo of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void SaveOnlineDeviceInfo(const std::string &pkgName, const DmDeviceInfo &info);

    /**
     * @tc.name: DmDeviceStateManager::DeleteOfflineDeviceInfo
     * @tc.desc: Delete Offline DeviceInfo of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void DeleteOfflineDeviceInfo(const std::string &pkgName, const DmDeviceInfo &info);

    /**
     * @tc.name: DmDeviceStateManager::PostDeviceOnline
     * @tc.desc: Post Device Online of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void PostDeviceOnline(const std::string &pkgName, const DmDeviceInfo &info);

    /**
     * @tc.name: DmDeviceStateManager::PostDeviceOffline
     * @tc.desc: Post Device Offline of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void PostDeviceOffline(const std::string &pkgName, const DmDeviceInfo &info);

    /**
     * @tc.name: DmDeviceStateManager::OnDeviceOnline
     * @tc.desc: OnDevice Online of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void OnDeviceOnline(const std::string &pkgName, const DmDeviceInfo &info);

    /**
     * @tc.name: DmDeviceStateManager::OnDeviceOffline
     * @tc.desc: OnDevice Offline of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void OnDeviceOffline(const std::string &pkgName, const DmDeviceInfo &info);

    /**
     * @tc.name: DmDeviceStateManager::OnDeviceChanged
     * @tc.desc: OnDevice Changed of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void OnDeviceChanged(const std::string &pkgName, const DmDeviceInfo &info);

    /**
     * @tc.name: DmDeviceStateManager::OnDeviceReady
     * @tc.desc: OnDevice Ready of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void OnDeviceReady(const std::string &pkgName, const DmDeviceInfo &info);

    /**
     * @tc.name: DmDeviceStateManager::OnDbReady
     * @tc.desc: OnDb Ready of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void OnDbReady(const std::string &pkgName, const std::string &deviceId);

    /**
     * @tc.name: DmDeviceStateManager::RegisterSoftbusStateCallback
     * @tc.desc: Register Softbus State Callback of the Dm Device State Manager
     * @tc.type: FUNC
     */
    int32_t RegisterSoftbusStateCallback();

    /**
     * @tc.name: DmDeviceStateManager::RegisterOffLineTimer
     * @tc.desc: Register OffLine Timerof the Dm Device State Manager
     * @tc.type: FUNC
     */
    void RegisterOffLineTimer(const DmDeviceInfo &deviceInfo);

    /**
     * @tc.name: DmDeviceStateManager::StartOffLineTimer
     * @tc.desc: Start OffLine Timer of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void StartOffLineTimer(const DmDeviceInfo &deviceInfo);

    /**
     * @tc.name: DmDeviceStateManager::DeleteTimeOutGroup
     * @tc.desc: Delete TimeOut Group of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void DeleteTimeOutGroup(std::string name);

    /**
     * @tc.name: DmDeviceStateManager::RegisterDevStateCallback
     * @tc.desc: Register DevState Callback of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void RegisterDevStateCallback(const std::string &pkgName, const std::string &extra);

    /**
     * @tc.name: DmDeviceStateManager::UnRegisterDevStateCallback
     * @tc.desc: UnRegister DevState Callback of the Dm Device State Manager
     * @tc.type: FUNC
     */
    void UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra);

    /**
     * @tc.name: DmDeviceStateManager::ProcNotifyEvent
     * @tc.desc: Proc Notify Event of the Dm Device State Manager
     * @tc.type: FUNC
     */
    int32_t ProcNotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &deviceId);

private:
    void StartEventThread();
    void StopEventThread();
    void ThreadLoop();
    int32_t AddTask(const std::shared_ptr<NotifyEvent> &task);
    void RunTask(const std::shared_ptr<NotifyEvent> &task);
private:
#if !defined(__LITEOS_M__)
    std::mutex timerMapMutex_;
    std::mutex remoteDeviceInfosMutex_;
#endif
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::map<std::string, DmDeviceInfo> remoteDeviceInfos_;
    std::map<std::string, std::string> decisionInfos_;
    std::map<std::string, StateTimerInfo> stateTimerInfoMap_;
    std::shared_ptr<DmTimer> timer_;
    std::shared_ptr<HiChainConnector> hiChainConnector_;
    std::string decisionSoName_;
    NotifyTask eventTask_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICE_STATE_MANAGER_H
