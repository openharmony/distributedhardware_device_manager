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

#include "dm_device_state_manager.h"

#include "dm_adapter_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_distributed_hardware_load.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
const uint32_t DM_EVENT_QUEUE_CAPACITY = 20;
const uint32_t DM_EVENT_WAIT_TIMEOUT = 2;
DmDeviceStateManager::DmDeviceStateManager(std::shared_ptr<SoftbusConnector> softbusConnector,
    std::shared_ptr<IDeviceManagerServiceListener> listener, std::shared_ptr<HiChainConnector> hiChainConnector)
    : softbusConnector_(softbusConnector), listener_(listener), hiChainConnector_(hiChainConnector)
{
    decisionSoName_ = "libdevicemanagerext_decision.z.so";
    StartEventThread();
    LOGI("DmDeviceStateManager constructor");
}

DmDeviceStateManager::~DmDeviceStateManager()
{
    LOGI("DmDeviceStateManager destructor");
    if (softbusConnector_ != nullptr) {
        softbusConnector_->UnRegisterSoftbusStateCallback("DM_PKG_NAME");
    }
    StopEventThread();
}

void DmDeviceStateManager::SaveOnlineDeviceInfo(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("SaveOnlineDeviceInfo begin, deviceId = %s", GetAnonyString(std::string(info.deviceId)).c_str());
    std::string udid;
    if (SoftbusConnector::GetUdidByNetworkId(info.networkId, udid) == DM_OK) {
        std::string uuid;
        DmDeviceInfo saveInfo = info;
        SoftbusConnector::GetUuidByNetworkId(info.networkId, uuid);
        {
#if defined(__LITEOS_M__)
            DmMutex mutexLock;
#else
            std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
            remoteDeviceInfos_[uuid] = saveInfo;
        }
        LOGI("SaveOnlineDeviceInfo complete, networkId = %s, udid = %s, uuid = %s", GetAnonyString(
            std::string(info.networkId)).c_str(), GetAnonyString(udid).c_str(), GetAnonyString(uuid).c_str());
    }
}

void DmDeviceStateManager::DeleteOfflineDeviceInfo(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("DeleteOfflineDeviceInfo begin, deviceId = %s", GetAnonyString(std::string(info.deviceId)).c_str());
#if defined(__LITEOS_M__)
    DmMutex mutexLock;
#else
    std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
    for (auto iter: remoteDeviceInfos_) {
        if (iter.second.deviceId == info.deviceId) {
            remoteDeviceInfos_.erase(iter.first);
            LOGI("DeleteOfflineDeviceInfo complete");
            break;
        }
    }
}

void DmDeviceStateManager::PostDeviceOnline(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("DmDeviceStateManager::PostDeviceOnline in");
    if (listener_ != nullptr) {
        DmDeviceState state = DEVICE_STATE_ONLINE;
        listener_->OnDeviceStateChange(pkgName, state, info);
    }
    LOGI("DmDeviceStateManager::PostDeviceOnline out");
}

void DmDeviceStateManager::PostDeviceOffline(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("DmDeviceStateManager::PostDeviceOffline in");
    if (listener_ != nullptr) {
        DmDeviceState state = DEVICE_STATE_OFFLINE;
        listener_->OnDeviceStateChange(pkgName, state, info);
    }
    LOGI("DmDeviceStateManager::PostDeviceOffline out");
}

void DmDeviceStateManager::OnDeviceOnline(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("OnDeviceOnline function is called with pkgName: %s", pkgName.c_str());
    RegisterOffLineTimer(info);
    SaveOnlineDeviceInfo(pkgName, info);

    DmAdapterManager &adapterMgrPtr = DmAdapterManager::GetInstance();
    std::shared_ptr<IDecisionAdapter> decisionAdapter = adapterMgrPtr.GetDecisionAdapter(decisionSoName_);
    if (decisionAdapter == nullptr) {
        LOGE("OnDeviceOnline decision adapter is null");
        PostDeviceOnline(pkgName, info);
    } else if (decisionInfos_.size() == 0) {
        PostDeviceOnline(pkgName, info);
    } else {
        std::vector<DmDeviceInfo> infoList;
        LOGI("OnDeviceOnline decision decisionInfos_ size: %d", decisionInfos_.size());
        for (auto iter : decisionInfos_) {
            std::string listenerPkgName = iter.first;
            std::string extra = iter.second;
            infoList.clear();
            infoList.push_back(info);
            decisionAdapter->FilterDeviceList(infoList, extra);
            if (infoList.size() == 1) {
                PostDeviceOnline(listenerPkgName, info);
            }
        }
    }
    LOGI("DmDeviceStateManager::OnDeviceOnline out");
}

void DmDeviceStateManager::OnDeviceOffline(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("OnDeviceOffline function is called with pkgName: %s", pkgName.c_str());
    StartOffLineTimer(info);
    DeleteOfflineDeviceInfo(pkgName, info);

    DmAdapterManager &adapterMgrPtr = DmAdapterManager::GetInstance();
    std::shared_ptr<IDecisionAdapter> decisionAdapter = adapterMgrPtr.GetDecisionAdapter(decisionSoName_);
    if (decisionAdapter == nullptr) {
        LOGE("OnDeviceOffline decision adapter is null");
        PostDeviceOffline(pkgName, info);
    } else if (decisionInfos_.size() == 0) {
        PostDeviceOffline(pkgName, info);
    } else {
        std::vector<DmDeviceInfo> infoList;
        LOGI("OnDeviceOffline decision decisionInfos_ size: %d", decisionInfos_.size());
        for (auto iter : decisionInfos_) {
            std::string listenerPkgName = iter.first;
            std::string extra = iter.second;
            infoList.clear();
            infoList.push_back(info);
            decisionAdapter->FilterDeviceList(infoList, extra);
            if (infoList.size() == 1) {
                PostDeviceOffline(listenerPkgName, info);
            }
        }
    }
    LOGI("DmDeviceStateManager::OnDeviceOffline out");
}

void DmDeviceStateManager::OnDeviceChanged(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("OnDeviceChanged function is called back with pkgName: %s", pkgName.c_str());
}

void DmDeviceStateManager::OnDeviceReady(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("OnDeviceReady function is called back with pkgName: %s", pkgName.c_str());
}

void DmDeviceStateManager::OnDbReady(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("OnDbReady function is called with pkgName: %s", pkgName.c_str());
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("On db ready pkgName is empty or deviceId is deviceId");
        return;
    }
    DmDeviceInfo saveInfo;
    {
#if defined(__LITEOS_M__)
        DmMutex mutexLock;
#else
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
        auto iter = remoteDeviceInfos_.find(deviceId);
        if (iter == remoteDeviceInfos_.end()) {
            LOGE("OnDbReady complete not find deviceId: %s", GetAnonyString(deviceId).c_str());
            return;
        }
        saveInfo = iter->second;
    }
    DmDistributedHardwareLoad::GetInstance().LoadDistributedHardwareFwk();
    if (listener_ != nullptr) {
        DmDeviceState state = DEVICE_INFO_READY;
        listener_->OnDeviceStateChange(pkgName, state, saveInfo);
    }
}

int32_t DmDeviceStateManager::RegisterSoftbusStateCallback()
{
    if (softbusConnector_ != nullptr) {
        return softbusConnector_->RegisterSoftbusStateCallback(std::string(DM_PKG_NAME), shared_from_this());
    }
    return DM_OK;
}

void DmDeviceStateManager::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("DmDeviceStateManager::RegisterDevStateCallback input param is empty");
        return;
    }
    LOGI("DmDeviceStateManager::RegisterDevStateCallback pkgName = %s, extra = %s",
        GetAnonyString(pkgName).c_str(), GetAnonyString(extra).c_str());
    if (decisionInfos_.count(pkgName) == 0) {
        decisionInfos_.insert(std::map<std::string, std::string>::value_type (pkgName, extra));
    }
}

void DmDeviceStateManager::UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("DmDeviceStateManager::UnRegisterDevStateCallback input param is empty");
        return;
    }
    LOGI("DmDeviceStateManager::UnRegisterDevStateCallback pkgName = %s, extra = %s",
        GetAnonyString(pkgName).c_str(), GetAnonyString(extra).c_str());
    if (decisionInfos_.count(pkgName) > 0) {
        auto iter = decisionInfos_.find(pkgName);
        if (iter != decisionInfos_.end()) {
            decisionInfos_.erase(pkgName);
        }
    }
}

void DmDeviceStateManager::RegisterOffLineTimer(const DmDeviceInfo &deviceInfo)
{
    std::string deviceId;
    int32_t ret = softbusConnector_->GetUdidByNetworkId(deviceInfo.deviceId, deviceId);
    if (ret != DM_OK) {
        LOGE("fail to get udid by networkId");
        return;
    }
    LOGI("Register offline timer for deviceId: %s", GetAnonyString(deviceId).c_str());
#if defined(__LITEOS_M__)
    DmMutex mutexLock;
#else
    std::lock_guard<std::mutex> mutexLock(timerMapMutex_);
#endif
    for (auto &iter : stateTimerInfoMap_) {
        if ((iter.first == deviceId) && (timer_ != nullptr)) {
            timer_->DeleteTimer(iter.second.timerName);
            stateTimerInfoMap_.erase(iter.first);
            break;
        }
    }
    if (stateTimerInfoMap_.find(deviceId) == stateTimerInfoMap_.end()) {
        std::string timerName = std::string(STATE_TIMER_PREFIX) + GetAnonyString(deviceId);
        StateTimerInfo stateTimer = {
            .timerName = timerName,
            .networkId = deviceInfo.deviceId,
            .isStart = false,
        };
        stateTimerInfoMap_[deviceId] = stateTimer;
    }
}

void DmDeviceStateManager::StartOffLineTimer(const DmDeviceInfo &deviceInfo)
{
#if defined(__LITEOS_M__)
    DmMutex mutexLock;
#else
    std::lock_guard<std::mutex> mutexLock(timerMapMutex_);
#endif
    std::string networkId = deviceInfo.deviceId;
    LOGI("Start offline timer for networkId: %s", GetAnonyString(networkId).c_str());
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
#if defined(__LITEOS_M__)
    DmMutex mutexLock;
#else
    std::lock_guard<std::mutex> mutexLock(timerMapMutex_);
#endif
    for (auto iter = stateTimerInfoMap_.begin(); iter != stateTimerInfoMap_.end(); iter++) {
        if (((iter->second).timerName == name) && (hiChainConnector_ != nullptr)) {
            LOGI("remove hichain group with deviceId: %s", GetAnonyString(iter->first).c_str());
            hiChainConnector_->DeleteTimeOutGroup((iter->first).c_str());
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
    LOGI("AddTask begin, eventId: %d", task->GetEventId());
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
    LOGI("RunTask begin, eventId: %d", task->GetEventId());
    if (task->GetEventId() == DM_NOTIFY_EVENT_ONDEVICEREADY) {
        OnDbReady(std::string(DM_PKG_NAME), task->GetDeviceId());
    }
    LOGI("RunTask complete");
}

int32_t DmDeviceStateManager::ProcNotifyEvent(const std::string &pkgName, const int32_t eventId,
    const std::string &deviceId)
{
    LOGI("ProcNotifyEvent in, eventId: %d", eventId);
    return AddTask(std::make_shared<NotifyEvent>(eventId, deviceId));
}
} // namespace DistributedHardware
} // namespace OHOS