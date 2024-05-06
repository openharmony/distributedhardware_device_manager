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

#include "dm_device_state_manager.h"

#include <pthread.h>

#include "dm_adapter_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_distributed_hardware_load.h"
#include "dm_log.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "deviceprofile_connector.h"
#endif

namespace OHOS {
namespace DistributedHardware {
const uint32_t DM_EVENT_QUEUE_CAPACITY = 20;
const uint32_t DM_EVENT_WAIT_TIMEOUT = 2;
constexpr const char* THREAD_LOOP = "ThreadLoop";
DmDeviceStateManager::DmDeviceStateManager(std::shared_ptr<SoftbusConnector> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener> listener, std::shared_ptr<HiChainConnector> hiChainConnector,
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector)
    : softbusConnector_(softbusConnector), listener_(listener), hiChainConnector_(hiChainConnector),
    hiChainAuthConnector_(hiChainAuthConnector)
{
    decisionSoName_ = "libdevicemanagerext_decision.z.so";
    StartEventThread();
    LOGI("DmDeviceStateManager constructor");
}

DmDeviceStateManager::~DmDeviceStateManager()
{
    LOGI("DmDeviceStateManager destructor");
    softbusConnector_->UnRegisterSoftbusStateCallback();
    StopEventThread();
}

int32_t DmDeviceStateManager::RegisterSoftbusStateCallback()
{
    if (softbusConnector_ != nullptr) {
        return softbusConnector_->RegisterSoftbusStateCallback(shared_from_this());
    }
    return DM_OK;
}

void DmDeviceStateManager::SaveOnlineDeviceInfo(const DmDeviceInfo &info)
{
    LOGI("SaveOnlineDeviceInfo begin, deviceId = %{public}s", GetAnonyString(std::string(info.deviceId)).c_str());
    std::string udid;
    if (SoftbusConnector::GetUdidByNetworkId(info.networkId, udid) == DM_OK) {
        std::string uuid;
        DmDeviceInfo saveInfo = info;
        SoftbusConnector::GetUuidByNetworkId(info.networkId, uuid);
        {
            std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
            remoteDeviceInfos_[uuid] = saveInfo;
            stateDeviceInfos_[udid] = saveInfo;
        }
        LOGI("SaveOnlineDeviceInfo complete, networkId = %{public}s, udid = %{public}s, uuid = %{public}s",
             GetAnonyString(std::string(info.networkId)).c_str(),
             GetAnonyString(udid).c_str(), GetAnonyString(uuid).c_str());
    }
}

void DmDeviceStateManager::DeleteOfflineDeviceInfo(const DmDeviceInfo &info)
{
    LOGI("DeleteOfflineDeviceInfo begin, deviceId = %{public}s", GetAnonyString(std::string(info.deviceId)).c_str());
    std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
    std::string deviceId = std::string(info.deviceId);
    for (auto iter: remoteDeviceInfos_) {
        if (std::string(iter.second.deviceId) == deviceId) {
            remoteDeviceInfos_.erase(iter.first);
            LOGI("Delete remoteDeviceInfos complete");
            break;
        }
    }
    for (auto iter: stateDeviceInfos_) {
        if (std::string(iter.second.deviceId) == deviceId) {
            stateDeviceInfos_.erase(iter.first);
            LOGI("Delete stateDeviceInfos complete");
            break;
        }
    }
}

void DmDeviceStateManager::OnDeviceOnline(std::string deviceId)
{
    LOGI("DmDeviceStateManager::OnDeviceOnline, deviceId = %{public}s", GetAnonyString(deviceId).c_str());
    DmDeviceInfo devInfo = softbusConnector_->GetDeviceInfoByDeviceId(deviceId);
    devInfo.authForm = GetAuthForm(devInfo.networkId);
    {
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
        stateDeviceInfos_[deviceId] = devInfo;
    }
    std::vector<std::string> pkgName = softbusConnector_->GetPkgName();
    if (pkgName.size() == 0) {
        listener_->OnDeviceStateChange(std::string(DM_PKG_NAME), DEVICE_STATE_ONLINE, devInfo);
    } else {
        for (auto item : pkgName) {
            listener_->OnDeviceStateChange(item, DEVICE_STATE_ONLINE, devInfo);
        }
    }
    softbusConnector_->ClearPkgName();
}

void DmDeviceStateManager::OnDeviceOffline(std::string deviceId)
{
    LOGI("DmDeviceStateManager::OnDeviceOffline, deviceId = %{public}s", GetAnonyString(deviceId).c_str());
    DmDeviceInfo devInfo;
    {
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
        if (stateDeviceInfos_.find(deviceId) == stateDeviceInfos_.end()) {
            LOGE("DmDeviceStateManager::OnDeviceOnline not find deviceId");
            return;
        }
        devInfo = stateDeviceInfos_[deviceId];
    }
    std::vector<std::string> pkgName = softbusConnector_->GetPkgName();
    if (pkgName.size() == 0) {
        listener_->OnDeviceStateChange(std::string(DM_PKG_NAME), DEVICE_STATE_OFFLINE, devInfo);
    } else {
        for (auto item : pkgName) {
            listener_->OnDeviceStateChange(item, DEVICE_STATE_OFFLINE, devInfo);
        }
    }
    softbusConnector_->ClearPkgName();
}

void DmDeviceStateManager::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    LOGI("Handle device status change: devState=%{public}d, deviceId=%{public}s.", devState,
        GetAnonyString(devInfo.deviceId).c_str());
    switch (devState) {
        case DEVICE_STATE_ONLINE:
            RegisterOffLineTimer(devInfo);
            SaveOnlineDeviceInfo(devInfo);
            devInfo.authForm = GetAuthForm(devInfo.networkId);
            DmDistributedHardwareLoad::GetInstance().LoadDistributedHardwareFwk();
            break;
        case DEVICE_STATE_OFFLINE:
            StartOffLineTimer(devInfo);
            DeleteOfflineDeviceInfo(devInfo);
            if (softbusConnector_ != nullptr) {
                std::string udid;
                softbusConnector_->GetUdidByNetworkId(devInfo.networkId, udid);
                softbusConnector_->EraseUdidFromMap(udid);
            }
            break;
        case DEVICE_INFO_CHANGED:
            ChangeDeviceInfo(devInfo);
            break;
        default:
            LOGE("HandleDeviceStatusChange error, unknown device state = %{public}d", devState);
            break;
    }
    if (listener_ == nullptr) {
        LOGE("HandleDeviceStatusChange failed, device manager client listener is null.");
        return;
    }
    if (softbusConnector_ != nullptr) {
        std::vector<std::string> pkgName = softbusConnector_->GetPkgName();
        if (pkgName.size() == 0) {
            listener_->OnDeviceStateChange(std::string(DM_PKG_NAME), devState, devInfo);
        } else {
            for (auto item : pkgName) {
                listener_->OnDeviceStateChange(item, devState, devInfo);
            }
        }
        softbusConnector_->ClearPkgName();
    }
}

void DmDeviceStateManager::OnDbReady(const std::string &pkgName, const std::string &uuid)
{
    if (pkgName.empty() || uuid.empty()) {
        LOGE("On db ready pkgName is empty or uuid is empty");
        return;
    }
    LOGI("OnDbReady function is called with pkgName: %{public}s and uuid = %{public}s", pkgName.c_str(),
         GetAnonyString(uuid).c_str());
    DmDeviceInfo saveInfo;
    {
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
        auto iter = remoteDeviceInfos_.find(uuid);
        if (iter == remoteDeviceInfos_.end()) {
            LOGE("OnDbReady complete not find uuid: %{public}s", GetAnonyString(uuid).c_str());
            return;
        }
        saveInfo = iter->second;
    }
    if (listener_ != nullptr) {
        DmDeviceState state = DEVICE_INFO_READY;
        listener_->OnDeviceStateChange(pkgName, state, saveInfo);
    }
}

void DmDeviceStateManager::RegisterOffLineTimer(const DmDeviceInfo &deviceInfo)
{
    std::string deviceUdid;
    int32_t ret = softbusConnector_->GetUdidByNetworkId(deviceInfo.networkId, deviceUdid);
    if (ret != DM_OK) {
        LOGE("fail to get udid by networkId");
        return;
    }
    LOGI("Register offline timer for deviceUdid: %{public}s", GetAnonyString(deviceUdid).c_str());
    std::lock_guard<std::mutex> mutexLock(timerMapMutex_);
    for (auto &iter : stateTimerInfoMap_) {
        if ((iter.first == deviceUdid) && (timer_ != nullptr)) {
            timer_->DeleteTimer(iter.second.timerName);
            stateTimerInfoMap_.erase(iter.first);
            break;
        }
    }
    if (stateTimerInfoMap_.find(deviceUdid) == stateTimerInfoMap_.end()) {
        std::string timerName = std::string(STATE_TIMER_PREFIX) + GetAnonyString(deviceUdid);
        StateTimerInfo stateTimer = {
            .timerName = timerName,
            .networkId = deviceInfo.networkId,
            .isStart = false,
        };
        stateTimerInfoMap_[deviceUdid] = stateTimer;
    }
}

void DmDeviceStateManager::StartOffLineTimer(const DmDeviceInfo &deviceInfo)
{
    std::lock_guard<std::mutex> mutexLock(timerMapMutex_);
    std::string networkId = deviceInfo.networkId;
    LOGI("Start offline timer for networkId: %{public}s", GetAnonyString(networkId).c_str());
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    for (auto &iter : stateTimerInfoMap_) {
        if ((iter.second.networkId == networkId) && !iter.second.isStart) {
            timer_->StartTimer(iter.second.timerName, OFFLINE_TIMEOUT,
                [this] (std::string name) {
                    DmDeviceStateManager::DeleteTimeOutGroup(name);
                });
            iter.second.isStart = true;
        }
    }
}

void DmDeviceStateManager::DeleteTimeOutGroup(std::string name)
{
    std::lock_guard<std::mutex> mutexLock(timerMapMutex_);
    for (auto iter = stateTimerInfoMap_.begin(); iter != stateTimerInfoMap_.end(); iter++) {
        if (((iter->second).timerName == name) && (hiChainConnector_ != nullptr)) {
            LOGI("remove hichain group with deviceId: %{public}s", GetAnonyString(iter->first).c_str());
            hiChainConnector_->DeleteTimeOutGroup((iter->first).c_str());
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
            uint32_t res = DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(iter->first);
            if (res == 0) {
                hiChainAuthConnector_->DeleteCredential(iter->first,
                                                        MultipleUserConnector::GetCurrentAccountUserID());
            }
#endif
            stateTimerInfoMap_.erase(iter);
            break;
        }
    }
}

void DmDeviceStateManager::StartEventThread()
{
    LOGI("StartEventThread begin");
    eventTask_.threadRunning_ = true;
    eventTask_.queueThread_ = std::thread(&DmDeviceStateManager::ThreadLoop, this);
    LOGI("StartEventThread complete");
}

void DmDeviceStateManager::StopEventThread()
{
    LOGI("StopEventThread begin");
    eventTask_.threadRunning_ = false;
    eventTask_.queueCond_.notify_all();
    eventTask_.queueFullCond_.notify_all();
    if (eventTask_.queueThread_.joinable()) {
        eventTask_.queueThread_.join();
    }
    LOGI("StopEventThread complete");
}

int32_t DmDeviceStateManager::AddTask(const std::shared_ptr<NotifyEvent> &task)
{
    LOGI("AddTask begin, eventId: %{public}d", task->GetEventId());
    {
        std::unique_lock<std::mutex> lock(eventTask_.queueMtx_);
        while (eventTask_.queue_.size() >= DM_EVENT_QUEUE_CAPACITY) {
            eventTask_.queueFullCond_.wait_for(lock, std::chrono::seconds(DM_EVENT_WAIT_TIMEOUT));
        }
        eventTask_.queue_.push(task);
    }
    eventTask_.queueCond_.notify_one();
    LOGI("AddTask complete");
    return DM_OK;
}

void DmDeviceStateManager::ThreadLoop()
{
    LOGI("ThreadLoop begin");
    int32_t ret = pthread_setname_np(pthread_self(), THREAD_LOOP);
    if (ret != DM_OK) {
        LOGE("ThreadLoop setname failed.");
    }
    while (eventTask_.threadRunning_) {
        std::shared_ptr<NotifyEvent> task = nullptr;
        {
            std::unique_lock<std::mutex> lock(eventTask_.queueMtx_);
            while (eventTask_.queue_.empty() && eventTask_.threadRunning_) {
                eventTask_.queueCond_.wait_for(lock, std::chrono::seconds(DM_EVENT_WAIT_TIMEOUT));
            }
            if (!eventTask_.queue_.empty()) {
                task = eventTask_.queue_.front();
                eventTask_.queue_.pop();
                eventTask_.queueFullCond_.notify_one();
            }
        }
        if (task != nullptr) {
            RunTask(task);
        }
    }
    LOGI("ThreadLoop end");
}

void DmDeviceStateManager::RunTask(const std::shared_ptr<NotifyEvent> &task)
{
    LOGI("RunTask begin, eventId: %{public}d", task->GetEventId());
    if (task->GetEventId() == DM_NOTIFY_EVENT_ONDEVICEREADY) {
        OnDbReady(std::string(DM_PKG_NAME), task->GetDeviceId());
    }
    LOGI("RunTask complete");
}

DmAuthForm DmDeviceStateManager::GetAuthForm(const std::string &networkId)
{
    LOGI("GetAuthForm start");
    if (hiChainConnector_ == nullptr) {
        LOGE("hiChainConnector_ is nullptr");
        return DmAuthForm::INVALID_TYPE;
    }

    if (networkId.empty()) {
        LOGE("networkId is empty");
        return DmAuthForm::INVALID_TYPE;
    }

    std::string udid;
    if (SoftbusConnector::GetUdidByNetworkId(networkId.c_str(), udid) == DM_OK) {
        return hiChainConnector_->GetGroupType(udid);
    }

    return DmAuthForm::INVALID_TYPE;
}

int32_t DmDeviceStateManager::ProcNotifyEvent(const int32_t eventId, const std::string &deviceId)
{
    LOGI("ProcNotifyEvent in, eventId: %{public}d", eventId);
    return AddTask(std::make_shared<NotifyEvent>(eventId, deviceId));
}

void DmDeviceStateManager::ChangeDeviceInfo(const DmDeviceInfo &info)
{
    std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
    for (auto iter : remoteDeviceInfos_) {
        if (iter.second.deviceId == info.deviceId) {
            if (memcpy_s(iter.second.deviceName, sizeof(iter.second.deviceName), info.deviceName,
                sizeof(info.deviceName)) != DM_OK) {
                    LOGE("ChangeDeviceInfo copy deviceName failed");
            }
            if (memcpy_s(iter.second.networkId, sizeof(iter.second.networkId), info.networkId,
                sizeof(info.networkId)) != DM_OK) {
                    LOGE("ChangeDeviceInfo copy networkId failed");
            }
            iter.second.deviceTypeId = info.deviceTypeId;
            LOGI("ChangeDeviceInfo complete");
            break;
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS