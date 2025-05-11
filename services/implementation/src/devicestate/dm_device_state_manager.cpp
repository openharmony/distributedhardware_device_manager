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
#include "dm_crypto.h"
#include "dm_device_info.h"
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
constexpr const char* STATE_TIMER_PREFIX = "deviceManagerTimer:stateTimer_";
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

void DmDeviceStateManager::OnDeviceOnline(std::string deviceId, int32_t authForm)
{
    LOGI("DmDeviceStateManager::OnDeviceOnline, deviceId = %{public}s", GetAnonyString(deviceId).c_str());
    DmDeviceInfo devInfo = softbusConnector_->GetDeviceInfoByDeviceId(deviceId);
    if (devInfo.deviceId[0] == '\0') {
        LOGE("deviceId is empty.");
        return;
    }
    devInfo.authForm = static_cast<DmAuthForm>(authForm);
    {
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
        if (stateDeviceInfos_.find(deviceId) == stateDeviceInfos_.end()) {
            stateDeviceInfos_[deviceId] = devInfo;
        }
    }
    ProcessDeviceStateChange(DEVICE_STATE_ONLINE, devInfo);
    softbusConnector_->ClearProcessInfo();
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
    ProcessDeviceStateChange(DEVICE_STATE_OFFLINE, devInfo);
    softbusConnector_->ClearProcessInfo();
}

void DmDeviceStateManager::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    LOGI("Handle device status change: devState=%{public}d, deviceId=%{public}s.", devState,
        GetAnonyString(devInfo.deviceId).c_str());
    switch (devState) {
        case DEVICE_STATE_ONLINE:
            RegisterOffLineTimer(devInfo);
            SaveOnlineDeviceInfo(devInfo);
            DmDistributedHardwareLoad::GetInstance().LoadDistributedHardwareFwk();
            ProcessDeviceStateChange(devState, devInfo);
            softbusConnector_->ClearProcessInfo();
            break;
        case DEVICE_STATE_OFFLINE:
            StartOffLineTimer(devInfo);
            DeleteOfflineDeviceInfo(devInfo);
            if (softbusConnector_ != nullptr) {
                std::string udid;
                softbusConnector_->GetUdidByNetworkId(devInfo.networkId, udid);
                softbusConnector_->EraseUdidFromMap(udid);
            }
            ProcessDeviceStateChange(devState, devInfo);
            softbusConnector_->ClearProcessInfo();
            break;
        case DEVICE_INFO_CHANGED:
            ChangeDeviceInfo(devInfo);
            ProcessDeviceStateChange(devState, devInfo);
            softbusConnector_->ClearChangeProcessInfo();
            break;
        default:
            LOGE("HandleDeviceStatusChange error, unknown device state = %{public}d", devState);
            break;
    }
}

void DmDeviceStateManager::ProcessDeviceStateChange(const DmDeviceState devState, const DmDeviceInfo &devInfo)
{
    LOGI("ProcessDeviceStateChange begin, devState = %{public}d networkId: %{public}s.", devState,
        GetAnonyString(devInfo.networkId).c_str());
    CHECK_NULL_VOID(softbusConnector_);
    CHECK_NULL_VOID(listener_);
    std::vector<ProcessInfo> processInfoVec;
    if (devState == DEVICE_INFO_CHANGED) {
        processInfoVec = softbusConnector_->GetChangeProcessInfo();
    } else {
        processInfoVec = softbusConnector_->GetProcessInfo();
    }
    for (const auto &item : processInfoVec) {
        if (!item.pkgName.empty()) {
            LOGI("ProcessDeviceStateChange, pkgName = %{public}s", item.pkgName.c_str());
            listener_->OnDeviceStateChange(item, devState, devInfo);
        }
    }
}

void DmDeviceStateManager::OnDbReady(const std::string &pkgName, const std::string &uuid)
{
    LOGI("OnDbReady function is called with pkgName: %{public}s and uuid = %{public}s",
         pkgName.c_str(), GetAnonyString(uuid).c_str());
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
        ProcessInfo processInfo;
        processInfo.pkgName = pkgName;
        processInfo.userId = MultipleUserConnector::GetFirstForegroundUserId();
        listener_->OnDeviceStateChange(processInfo, state, saveInfo);
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
    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(deviceUdid, reinterpret_cast<uint8_t *>(udidHash)) != DM_OK) {
        LOGE("get udidhash by udid: %{public}s failed.", GetAnonyString(deviceUdid).c_str());
        return;
    }
    LOGI("Register offline timer for udidHash: %{public}s", GetAnonyString(std::string(udidHash)).c_str());
    std::lock_guard<std::mutex> mutexLock(timerMapMutex_);
    for (auto &iter : stateTimerInfoMap_) {
        if ((iter.first == std::string(udidHash)) && (timer_ != nullptr)) {
            timer_->DeleteTimer(iter.second.timerName);
            stateTimerInfoMap_.erase(iter.first);
            auto idIter = udidhash2udidMap_.find(udidHash);
            if (idIter != udidhash2udidMap_.end()) {
                udidhash2udidMap_.erase(idIter->first);
            }
            break;
        }
    }
    if (stateTimerInfoMap_.find(std::string(udidHash)) == stateTimerInfoMap_.end()) {
        std::string sha256UdidHash = Crypto::Sha256(std::string(udidHash));
        std::string timerName = std::string(STATE_TIMER_PREFIX) + sha256UdidHash.substr(0, sha256UdidHash.size() / 2);
        StateTimerInfo stateTimer = {
            .timerName = timerName,
            .networkId = deviceInfo.networkId,
            .isStart = false,
        };
        stateTimerInfoMap_[std::string(udidHash)] = stateTimer;
    }
    if (udidhash2udidMap_.find(std::string(udidHash)) == udidhash2udidMap_.end()) {
        udidhash2udidMap_[std::string(udidHash)] = deviceUdid;
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

void DmDeviceStateManager::DeleteOffLineTimer(std::string udidHash)
{
    std::lock_guard<std::mutex> mutexLock(timerMapMutex_);
    LOGI("DELETE offline timer for networkId: %{public}s", GetAnonyString(udidHash).c_str());
    if (timer_ == nullptr || udidHash.empty()) {
        return;
    }
    auto iter = stateTimerInfoMap_.find(udidHash);
    if (iter != stateTimerInfoMap_.end()) {
        timer_->DeleteTimer(iter->second.timerName);
        iter->second.isStart = false;
        stateTimerInfoMap_.erase(iter->first);
        auto idIter = udidhash2udidMap_.find(udidHash);
        if (idIter != udidhash2udidMap_.end()) {
            udidhash2udidMap_.erase(idIter->first);
        }
    }
    return;
}

void DmDeviceStateManager::DeleteTimeOutGroup(std::string name)
{
    std::lock_guard<std::mutex> mutexLock(timerMapMutex_);
    for (auto iter = stateTimerInfoMap_.begin(); iter != stateTimerInfoMap_.end(); iter++) {
        if (((iter->second).timerName == name) && (hiChainConnector_ != nullptr)) {
            auto idIter = udidhash2udidMap_.find(iter->first);
            if (idIter == udidhash2udidMap_.end()) {
                LOGE("remove hichain group find deviceId: %{public}s failed.", GetAnonyString(iter->first).c_str());
                break;
            }
            LOGI("remove hichain group with deviceId: %{public}s", GetAnonyString(idIter->second).c_str());
            hiChainConnector_->DeleteTimeOutGroup((idIter->second).c_str());
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
            DeleteGroupByDP(idIter->second);
            DmOfflineParam offlineParam;
            uint32_t res = DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(idIter->second, offlineParam);
            if (res == 0) {
                DeleteCredential(offlineParam, idIter->second);
                DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
            }
#endif
            stateTimerInfoMap_.erase(iter);
            break;
        }
    }
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DmDeviceStateManager::DeleteCredential(DmOfflineParam offlineParam, const std::string &deviceId)
{
    if (offlineParam.skIdVec.empty()) {
        CHECK_NULL_VOID(hiChainAuthConnector_);
        hiChainAuthConnector_->DeleteCredential(deviceId, MultipleUserConnector::GetCurrentAccountUserID(),
            offlineParam.peerUserId);
    }
}

int32_t DmDeviceStateManager::DeleteSkCredAndAcl(const std::vector<DmAclIdParam> &acls)
{
    LOGI("DeleteSkCredAndAcl start.");
    int32_t ret = DM_OK;
    if (acls.empty()) {
        return ret;
    }
    CHECK_NULL_RETURN(hiChainAuthConnector_, ERR_DM_POINT_NULL);
    for (auto item : acls) {
        ret = DeviceProfileConnector::GetInstance().DeleteSessionKey(item.userId, item.skId);
        if (ret != DM_OK) {
            LOGE("DeleteSessionKey err, userId:%{public}d, skId:%{public}d, ret:%{public}d", item.userId, item.skId,
                ret);
        }
        ret = hiChainAuthConnector_->DeleteCredential(item.userId, item.credId);
        if (ret != DM_OK) {
            LOGE("DeletecredId err, userId:%{public}d, credId:%{public}s, ret:%{public}d", item.userId,
                item.credId.c_str(), ret);
        }
        DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.accessControlId);
    }
    return ret;
}
#endif

void DmDeviceStateManager::StartEventThread()
{
    LOGI("StartEventThread begin");
    eventTask_.threadRunning_ = true;
    eventTask_.queueThread_ = std::thread([this]() { this->ThreadLoop(); });
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
        if (std::string(iter.second.deviceId) == std::string(info.deviceId)) {
            if (memcpy_s(iter.second.deviceName, sizeof(iter.second.deviceName), info.deviceName,
                         sizeof(info.deviceName)) != DM_OK) {
                    LOGE("ChangeDeviceInfo remoteDeviceInfos copy deviceName failed");
                    return;
            }
            if (memcpy_s(iter.second.networkId, sizeof(iter.second.networkId), info.networkId,
                         sizeof(info.networkId)) != DM_OK) {
                    LOGE("ChangeDeviceInfo remoteDeviceInfos copy networkId failed");
                    return;
            }
            iter.second.deviceTypeId = info.deviceTypeId;
            LOGI("Change remoteDeviceInfos complete");
            break;
        }
    }
    for (auto iter : stateDeviceInfos_) {
        if (std::string(iter.second.deviceId) == std::string(info.deviceId)) {
            if (memcpy_s(iter.second.deviceName, sizeof(iter.second.deviceName), info.deviceName,
                         sizeof(info.deviceName)) != DM_OK) {
                    LOGE("ChangeDeviceInfo stateDeviceInfos copy deviceName failed");
                    return;
            }
            if (memcpy_s(iter.second.networkId, sizeof(iter.second.networkId), info.networkId,
                         sizeof(info.networkId)) != DM_OK) {
                    LOGE("ChangeDeviceInfo stateDeviceInfos copy networkId failed");
                    return;
            }
            iter.second.deviceTypeId = info.deviceTypeId;
            LOGI("Change stateDeviceInfos complete");
            break;
        }
    }
}

std::string DmDeviceStateManager::GetUdidByNetWorkId(std::string networkId)
{
    LOGI("DmDeviceStateManager::GetUdidByNetWorkId networkId %{public}s", GetAnonyString(networkId).c_str());
    {
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
        for (auto &iter : stateDeviceInfos_) {
            if (networkId == iter.second.networkId) {
                return iter.first;
            }
        }
    }
    LOGI("Not find udid by networkid in stateDeviceInfos.");
    return "";
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
int32_t DmDeviceStateManager::DeleteGroupByDP(const std::string &deviceId)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAccessControlProfile();
    LOGI("DeleteGroupByDP, AccessControlProfile size is %{public}zu", profiles.size());
    std::vector<std::string> delPkgNameVec;
    for (auto &item : profiles) {
        std::string trustDeviceId = item.GetTrustDeviceId();
        if (trustDeviceId != deviceId) {
            continue;
        }
        if (item.GetAuthenticationType() == ALLOW_AUTH_ONCE && !item.GetAccesser().GetAccesserBundleName().empty()) {
            delPkgNameVec.push_back(item.GetAccesser().GetAccesserBundleName());
        }
    }
    if (delPkgNameVec.size() == 0) {
        LOGI("delPkgNameVec is empty");
        return DM_OK;
    }
    if (hiChainConnector_ == nullptr) {
        LOGE("hiChainConnector_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    std::vector<GroupInfo> groupListExt;
    hiChainConnector_->GetRelatedGroupsExt(deviceId, groupListExt);
    for (auto &iter : groupListExt) {
        for (auto &pkgName : delPkgNameVec) {
            if (iter.groupName.find(pkgName) != std::string::npos) {
                int32_t ret = hiChainConnector_->DeleteGroupExt(iter.groupId);
                LOGI("DeleteGroupByDP delete groupId %{public}s ,result %{public}d.",
                    GetAnonyString(iter.groupId).c_str(), ret);
            }
        }
    }
    return DM_OK;
}
#endif

bool DmDeviceStateManager::CheckIsOnline(const std::string &udid)
{
    LOGI("DmDeviceStateManager::CheckIsOnline start.");
    {
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
        if (stateDeviceInfos_.find(udid) != stateDeviceInfos_.end()) {
            return true;
        }
    }
    return false;
}

void DmDeviceStateManager::HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo)
{
    CHECK_NULL_VOID(softbusConnector_);
    CHECK_NULL_VOID(listener_);
    std::vector<ProcessInfo> processInfos = softbusConnector_->GetProcessInfo();
    softbusConnector_->ClearProcessInfo();
    LOGI("pkgName size: %{public}zu", processInfos.size());
    for (const auto &item : processInfos) {
        listener_->OnDeviceScreenStateChange(item, devInfo);
    }
}
} // namespace DistributedHardware
} // namespace OHOS