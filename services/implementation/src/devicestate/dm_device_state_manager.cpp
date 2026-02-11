/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_device_info.h"
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
constexpr const char* FILED_AUTHORIZED_APP_LIST = "authorizedAppList";
const uint32_t AUTH_ONCE_STATE_TIMER_MAX = 500;
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
            std::lock_guard<ffrt::mutex> mutexLock(remoteDeviceInfosMutex_);
#else
            std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
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
    LOGI("DmDeviceStateManager::OnDeviceOffline, deviceId = %{public}s", GetAnonyString(deviceId).c_str());
    {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<ffrt::mutex> mutexLock(remoteDeviceInfosMutex_);
#else
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
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
    std::lock_guard<std::mutex> mutexLock(notifyEventInfosMutex_);
    std::string deviceId = std::string(info.deviceId);
    for (auto iter: notifyEventInfos_) {
        if (std::string(iter.second.deviceId) == deviceId) {
            notifyEventInfos_.erase(iter.first);
            LOGI("Delete notifyEventInfos_ complete");
            break;
        }
    }
}

void DmDeviceStateManager::OnDeviceOnline(std::string deviceId, int32_t authForm)
{
    std::string uuid = "";
    DmDeviceInfo devInfo = softbusConnector_->GetDeviceInfoByDeviceId(deviceId, uuid);
    if (devInfo.deviceId[0] == '\0') {
        LOGE("deviceId is empty.");
        return;
    }
    LOGI("deviceId: %{public}s,  uuid: %{public}s", GetAnonyString(deviceId).c_str(), GetAnonyString(uuid).c_str());
    devInfo.authForm = static_cast<DmAuthForm>(authForm);
    {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<ffrt::mutex> mutexLock(remoteDeviceInfosMutex_);
#else
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
        if (stateDeviceInfos_.find(deviceId) == stateDeviceInfos_.end()) {
            stateDeviceInfos_[deviceId] = devInfo;
        }
        if (remoteDeviceInfos_.find(uuid) == remoteDeviceInfos_.end()) {
            remoteDeviceInfos_[uuid] = devInfo;
        }
    }
    std::vector<ProcessInfo> processInfoVec = softbusConnector_->GetProcessInfo();
    ProcessDeviceStateChange(DEVICE_STATE_ONLINE, devInfo, processInfoVec, true);
    softbusConnector_->ClearProcessInfo();
}

void DmDeviceStateManager::OnDeviceOffline(std::string deviceId, const bool isOnline)
{
    LOGI("OnDeviceOffline, deviceId = %{public}s", GetAnonyString(deviceId).c_str());
    DmDeviceInfo devInfo;
    {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<ffrt::mutex> mutexLock(remoteDeviceInfosMutex_);
#else
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
        if (stateDeviceInfos_.find(deviceId) == stateDeviceInfos_.end()) {
            LOGE("DmDeviceStateManager::OnDeviceOffline not find deviceId");
            return;
        }
        devInfo = stateDeviceInfos_[deviceId];
    }
    std::vector<ProcessInfo> processInfoVec = softbusConnector_->GetProcessInfo();
    ProcessDeviceStateChange(DEVICE_STATE_OFFLINE, devInfo, processInfoVec, isOnline);
    softbusConnector_->ClearProcessInfo();
}

void DmDeviceStateManager::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo,
    std::vector<ProcessInfo> &processInfoVec, const std::string &peerUdid, const bool isOnline)
{
    LOGI("Handle device status change: devState=%{public}d, deviceId=%{public}s.", devState,
        GetAnonyString(devInfo.deviceId).c_str());
    switch (devState) {
        case DEVICE_STATE_ONLINE:
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
            RegisterOffLineTimer(devInfo);
#endif
            SaveOnlineDeviceInfo(devInfo);
            ProcessDeviceStateChange(devState, devInfo, processInfoVec, isOnline);
            break;
        case DEVICE_STATE_OFFLINE:
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
            StartOffLineTimer(peerUdid);
#endif
            DeleteOfflineDeviceInfo(devInfo);
            if (softbusConnector_ != nullptr) {
                std::string udid;
                softbusConnector_->GetUdidByNetworkId(devInfo.networkId, udid);
                softbusConnector_->EraseUdidFromMap(udid);
            }
            ProcessDeviceStateChange(devState, devInfo, processInfoVec, isOnline);
            break;
        case DEVICE_INFO_CHANGED:
            ChangeDeviceInfo(devInfo);
            ProcessDeviceStateChange(devState, devInfo, processInfoVec, isOnline);
            break;
        default:
            LOGE("HandleDeviceStatusChange error, unknown device state = %{public}d", devState);
            break;
    }
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DmDeviceStateManager::ProcessDeviceStateChange(const DmDeviceState devState, const DmDeviceInfo &devInfo,
    std::vector<ProcessInfo> &processInfoVec, const bool isOnline)
{
    LOGI("begin, devState = %{public}d networkId: %{public}s.", devState, GetAnonyString(devInfo.networkId).c_str());
    std::unordered_set<int64_t> remoteTokenIds;
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid(localDeviceId);
    CHECK_NULL_VOID(softbusConnector_);
    std::string udid = softbusConnector_->GetDeviceUdidByUdidHash(devInfo.deviceId);
    DeviceProfileConnector::GetInstance().GetRemoteTokenIds(localUdid, udid, remoteTokenIds);
    std::unordered_set<int64_t> remoteServiceIds;
    for (const auto &item : remoteTokenIds) {
        std::vector<ServiceInfoProfile> serviceInfos;
        if (DeviceProfileConnector::GetInstance().GetServiceInfoProfileByTokenId(item, serviceInfos) != DM_OK) {
            LOGE("GetServiceInfoProfileByTokenId failed.");
            continue;
        }
        for (const auto &serviceInfo : serviceInfos) {
            remoteServiceIds.insert(serviceInfo.serviceId);
        }
    }
    LOGI("ProcessDeviceStateChange, remoteServiceIds size: %{public}zu", remoteServiceIds.size());

    CHECK_NULL_VOID(listener_);
    for (const auto &item : processInfoVec) {
        if (!item.pkgName.empty()) {
            LOGI("ProcessDeviceStateChange, pkgName = %{public}s", item.pkgName.c_str());
            if (!remoteServiceIds.empty() && devState == DEVICE_STATE_ONLINE) {
                std::vector<int64_t> remoteServiceIdVec(remoteServiceIds.begin(), remoteServiceIds.end());
                listener_->OnDeviceStateChange(item, devState, devInfo, remoteServiceIdVec);
            } else {
                listener_->OnDeviceStateChange(item, devState, devInfo, isOnline);
            }
        }
    }
}
#else
void DmDeviceStateManager::ProcessDeviceStateChange(const DmDeviceState devState, const DmDeviceInfo &devInfo,
    std::vector<ProcessInfo> &processInfoVec, const bool isOnline)
{
    LOGI("begin, devState = %{public}d networkId: %{public}s.", devState,
        GetAnonyString(devInfo.networkId).c_str());
    CHECK_NULL_VOID(listener_);
    for (const auto &item : processInfoVec) {
        if (!item.pkgName.empty()) {
            listener_->OnDeviceStateChange(item, devState, devInfo, isOnline);
        }
    }
}
#endif

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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<ffrt::mutex> mutexLock(remoteDeviceInfosMutex_);
#else
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
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
        listener_->OnDeviceStateChange(processInfo, state, saveInfo, true);
    }
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DmDeviceStateManager::RegisterOffLineTimer(const DmDeviceInfo &deviceInfo)
{
    std::string peerUdid;
    CHECK_NULL_VOID(softbusConnector_);
    int32_t ret = softbusConnector_->GetUdidByNetworkId(deviceInfo.networkId, peerUdid);
    if (ret != DM_OK) {
        LOGE("fail to get udid by networkId: %{public}s", GetAnonyString(deviceInfo.networkId).c_str());
        return;
    }
    int32_t localUserId = MultipleUserConnector::GetCurrentAccountUserID();
    LOGI("Register offline timer for udid: %{public}s, localUserId: %{public}d",
        GetAnonyString(peerUdid).c_str(), localUserId);
    std::unordered_set<int32_t> peerUserIds =
        DeviceProfileConnector::GetInstance().GetActiveAuthOncePeerUserId(peerUdid, localUserId);
    if (peerUserIds.empty()) {
        LOGE("fail to get peerUserId by peerUdid:%{public}s and localUserId:%{public}d",
            GetAnonyString(peerUdid).c_str(), localUserId);
        return;
    }
    std::lock_guard<ffrt::mutex> mutexLock(timerMapMutex_);
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    for (const auto &peerUserId : peerUserIds) {
        std::string key = peerUdid + "_" + std::to_string(peerUserId) + "_" +  std::to_string(localUserId);
        auto iter = stateTimerInfoMap_.find(key);
        if (iter != stateTimerInfoMap_.end()) {
            timer_->DeleteTimer(iter->second.timerName);
            stateTimerInfoMap_.erase(iter);
        }
        if (stateTimerInfoMap_.size() > AUTH_ONCE_STATE_TIMER_MAX) {
            LOGW("stateTimerInfoMap_ size more than limit, peerUdid:%{public}s, peerUserId:%{public}d,"
                " localUserId:%{public}d", GetAnonyString(peerUdid).c_str(), peerUserId, localUserId);
            return;
        }
        std::string sha256UdidHash = Crypto::Sha256(peerUdid);
        std::string timerName = std::string(STATE_TIMER_PREFIX) + sha256UdidHash + "_" + std::to_string(peerUserId) +
            "_" +  std::to_string(localUserId);
        StateTimerInfo stateTimer = {
            .timerName = timerName,
            .peerUdid = peerUdid,
            .peerUserId = peerUserId,
            .localUserId = localUserId,
            .isStart = false,
        };
        stateTimerInfoMap_[key] = stateTimer;
    }
}

void DmDeviceStateManager::StartOffLineTimer(const std::string &peerUdid)
{
    std::unordered_set<AuthOnceAclInfo, AuthOnceAclInfoHash> authOnceAclInfos =
        DeviceProfileConnector::GetInstance().GetAuthOnceAclInfos(peerUdid);
    if (authOnceAclInfos.empty()) {
        LOGE("fail to get peerUserId and localUserId by peerUdid:%{public}s", GetAnonyString(peerUdid).c_str());
        return;
    }
    std::lock_guard<ffrt::mutex> mutexLock(timerMapMutex_);
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    for (const auto &item : authOnceAclInfos) {
        LOGI("Start offline timer for peerUdid:%{public}s, peerUserId:%{public}d, localUserId:%{public}d",
            GetAnonyString(peerUdid).c_str(), item.peerUserId, item.localUserId);
        std::string key = peerUdid + "_" + std::to_string(item.peerUserId) + "_" +  std::to_string(item.localUserId);
        auto iter = stateTimerInfoMap_.find(key);
        if (iter == stateTimerInfoMap_.end() || iter->second.isStart) {
            continue;
        }
        timer_->StartTimer(iter->second.timerName, OFFLINE_TIMEOUT, [this] (std::string name) {
            DmDeviceStateManager::DeleteTimeOutGroup(name);
        });
        iter->second.isStart = true;
    }
}

void DmDeviceStateManager::DeleteTimeOutGroup(const std::string &timerName)
{
    LOGI("start timerName:%{public}s", timerName.c_str());
    std::lock_guard<ffrt::mutex> mutexLock(timerMapMutex_);
    if (hiChainConnector_ == nullptr || softbusConnector_ == nullptr) {
        LOGW("DmDeviceStateManager::DeleteTimeOutGroup hiChainConnector_ or softbusConnector_ is nullptr");
        return;
    }
    auto iter = stateTimerInfoMap_.begin();
    for (;iter != stateTimerInfoMap_.end(); iter++) {
        if (iter->second.timerName == timerName) {
            break;
        }
    }
    if (iter == stateTimerInfoMap_.end()) {
        LOGW("DmDeviceStateManager::DeleteTimeOutGroup not found by timerName:%{public}s", timerName.c_str());
        return;
    }
    auto timerInfo = iter->second;
    bool isOnline = softbusConnector_->CheckIsOnline(iter->second.peerUdid);
    bool isActive = DeviceProfileConnector::GetInstance().AuthOnceAclIsActive(timerInfo.peerUdid, timerInfo.peerUserId,
        timerInfo.localUserId);
    if (isOnline && isActive) {
        LOGW("device is online and status is acive peerUdid:%{public}s, peerUserId:%{public}d, localUserId:%{public}d",
            GetAnonyString(timerInfo.peerUdid).c_str(), timerInfo.peerUserId, timerInfo.localUserId);
        iter->second.isStart = false;
        return;
    }
    LOGI("remove hichain group with peerUdid:%{public}s, peerUserId:%{public}d, localUserId:%{public}d",
        GetAnonyString(timerInfo.peerUdid).c_str(), timerInfo.peerUserId, timerInfo.localUserId);
    hiChainConnector_->DeleteTimeOutGroup(timerInfo.peerUdid, timerInfo.localUserId);
    DeleteGroupByDP(timerInfo.peerUdid, timerInfo.peerUserId, timerInfo.localUserId);
    DmOfflineParam offlineParam;
    uint32_t allAclCnt = DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(timerInfo.peerUdid,
        timerInfo.peerUserId, timerInfo.localUserId, offlineParam);
    if (allAclCnt > 0 && allAclCnt == static_cast<uint32_t>(offlineParam.needDelAclInfos.size())) {
        DeleteCredential(offlineParam, timerInfo.peerUdid, timerInfo.localUserId);
    }
    DeleteSkCredAndAcl(offlineParam.needDelAclInfos);
    stateTimerInfoMap_.erase(iter);
}

void DmDeviceStateManager::DeleteCredential(const DmOfflineParam &offlineParam, const std::string &peerUdid,
    int32_t localUserId)
{
    if (offlineParam.skIdVec.empty()) {
        CHECK_NULL_VOID(hiChainAuthConnector_);
        hiChainAuthConnector_->DeleteCredential(peerUdid, localUserId, offlineParam.peerUserId);
    }
}

int32_t DmDeviceStateManager::DeleteSkCredAndAcl(const std::vector<DmAclIdParam> &acls)
{
    LOGI("DmDeviceStateManager::DeleteSkCredAndAcl start.");
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
        DeleteCredential(item);
        DeviceProfileConnector::GetInstance().DeleteAccessControlById(item.accessControlId);
    }
    return ret;
}

void DmDeviceStateManager::DeleteCredential(const DmAclIdParam &acl)
{
    LOGI("DmDeviceStateManager::DeleteCredential by acl start.");
    CHECK_NULL_VOID(hiChainAuthConnector_);
    JsonObject credJson;
    int32_t ret = hiChainAuthConnector_->QueryCredInfoByCredId(acl.userId, acl.credId, credJson);
    if (ret != DM_OK || !credJson.Contains(acl.credId)) {
        LOGE("DeleteCredential err, ret:%{public}d", ret);
        return;
    }
    if (!credJson[acl.credId].Contains(FILED_AUTHORIZED_APP_LIST)) {
        ret = hiChainAuthConnector_->DeleteCredential(acl.userId, acl.credId);
        if (ret != DM_OK) {
            LOGE("DeletecredId err, ret:%{public}d", ret);
        }
        return;
    }
    std::vector<std::string> appList;
    credJson[acl.credId][FILED_AUTHORIZED_APP_LIST].Get(appList);
    for (auto tokenId : acl.tokenIds) {
        auto iter = std::find(appList.begin(), appList.end(), std::to_string(tokenId));
        if (iter != appList.end()) {
            appList.erase(iter);
        }
    }
    if (appList.empty()) {
        ret = hiChainAuthConnector_->DeleteCredential(acl.userId, acl.credId);
        if (ret != DM_OK) {
            LOGE("DeletecredId err, ret:%{public}d", ret);
        }
        return;
    }
    ret = hiChainAuthConnector_->UpdateCredential(acl.credId, acl.userId, appList);
    if (ret != DM_OK) {
        LOGE("UpdateCredential err, ret:%{public}d", ret);
    }
}
#endif

void DmDeviceStateManager::DeleteOffLineTimer(const std::string &peerUdid)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    int32_t localUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (timer_ == nullptr || peerUdid.empty()) {
        return;
    }
    std::unordered_set<int32_t> peerUserIds =
        DeviceProfileConnector::GetInstance().GetActiveAuthOncePeerUserId(peerUdid, localUserId);
    if (peerUserIds.empty()) {
        LOGE("fail to get peerUserId by peerUdid and localUserId");
        return;
    }
    std::lock_guard<ffrt::mutex> mutexLock(timerMapMutex_);
    for (const auto &peerUserId : peerUserIds) {
        std::string key = peerUdid + "_" + std::to_string(peerUserId) + "_" +  std::to_string(localUserId);
        auto iter = stateTimerInfoMap_.find(key);
        if (iter == stateTimerInfoMap_.end()) {
            continue;
        }
        timer_->DeleteTimer(iter->second.timerName);
        stateTimerInfoMap_.erase(iter);
    }
#else
    (void)peerUdid;
#endif
}

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

void DmDeviceStateManager::SaveNotifyEventInfos(const int32_t eventId, const std::string &deviceId)
{
    LOGI("SaveNotifyEventInfos in, eventId: %{public}d", eventId);
    DmDeviceInfo saveInfo;
    {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<ffrt::mutex> mutexLock(remoteDeviceInfosMutex_);
#else
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
        auto iter = remoteDeviceInfos_.find(deviceId);
        if (iter == remoteDeviceInfos_.end()) {
            LOGE("SaveNotifyEventInfos complete not find deviceId: %{public}s", GetAnonyString(deviceId).c_str());
            return;
        }
        saveInfo = iter->second;
    }
    std::lock_guard<std::mutex> mutexLock(notifyEventInfosMutex_);
    notifyEventInfos_[deviceId] = saveInfo;
}

void DmDeviceStateManager::GetNotifyEventInfos(std::vector<DmDeviceInfo> &deviceList)
{
    std::lock_guard<std::mutex> mutexLock(notifyEventInfosMutex_);
    for (const auto &item: notifyEventInfos_) {
        deviceList.push_back(item.second);
    }
}

int32_t DmDeviceStateManager::ProcNotifyEvent(const int32_t eventId, const std::string &deviceId)
{
    LOGI("ProcNotifyEvent in, eventId: %{public}d", eventId);
    return AddTask(std::make_shared<NotifyEvent>(eventId, deviceId));
}

void DmDeviceStateManager::ChangeDeviceInfo(const DmDeviceInfo &info)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::lock_guard<ffrt::mutex> mutexLock(remoteDeviceInfosMutex_);
#else
    std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
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
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<ffrt::mutex> mutexLock(remoteDeviceInfosMutex_);
#else
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
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
int32_t DmDeviceStateManager::DeleteGroupByDP(const std::string &peerUdid, int32_t peerUserId, int32_t localUserId)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles =
        DeviceProfileConnector::GetInstance().GetAccessControlProfileByUserId(localUserId);
    LOGI("DeleteGroupByDP, AccessControlProfile size is %{public}zu", profiles.size());
    std::vector<std::string> delPkgNameVec;
    for (auto &item : profiles) {
        if (item.GetTrustDeviceId() != peerUdid || item.GetAuthenticationType() != ALLOW_AUTH_ONCE) {
            continue;
        }
        auto accesser = item.GetAccesser();
        auto accessee = item.GetAccessee();
        if (accesser.GetAccesserDeviceId() == peerUdid && accesser.GetAccesserUserId() == peerUserId &&
            accessee.GetAccesseeUserId() == localUserId && !accessee.GetAccesseeBundleName().empty()) {
            delPkgNameVec.push_back(accessee.GetAccesseeBundleName());
        }
        if (accessee.GetAccesseeDeviceId() == peerUdid && accessee.GetAccesseeUserId() == peerUserId &&
            accesser.GetAccesserUserId() == localUserId && !accesser.GetAccesserBundleName().empty()) {
            delPkgNameVec.push_back(accesser.GetAccesserBundleName());
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
    hiChainConnector_->GetRelatedGroupsExt(peerUdid, groupListExt);
    for (auto &iter : groupListExt) {
        for (auto &pkgName : delPkgNameVec) {
            if (iter.groupName.find(pkgName) != std::string::npos) {
                int32_t ret = hiChainConnector_->DeleteGroupExt(localUserId, iter.groupId);
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
    LOGI("start, udid: %{public}s", GetAnonyString(std::string(udid)).c_str());
    {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
        std::lock_guard<ffrt::mutex> mutexLock(remoteDeviceInfosMutex_);
#else
        std::lock_guard<std::mutex> mutexLock(remoteDeviceInfosMutex_);
#endif
        if (stateDeviceInfos_.find(udid) != stateDeviceInfos_.end()) {
            return true;
        }
    }
    return false;
}

void DmDeviceStateManager::HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo,
    std::vector<ProcessInfo> &processInfos)
{
    CHECK_NULL_VOID(listener_);
    LOGI("pkgName size: %{public}zu", processInfos.size());
    for (const auto &item : processInfos) {
        listener_->OnDeviceScreenStateChange(item, devInfo);
    }
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DmDeviceStateManager::StartDelTimerByDP(const std::string &peerUdid, int32_t peerUserId, int32_t localUserId)
{
    LOGI("StartDelTimerByDP for peerUdid:%{public}s, peerUserId:%{public}d, localUserId:%{public}d",
        GetAnonyString(peerUdid).c_str(), peerUserId, localUserId);
    std::lock_guard<ffrt::mutex> mutexLock(timerMapMutex_);
    std::string key = peerUdid + "_" + std::to_string(peerUserId) + "_" +  std::to_string(localUserId);
    auto iter = stateTimerInfoMap_.find(key);
    if (iter != stateTimerInfoMap_.end() && iter->second.isStart) {
        return;
    }
    std::string sha256UdidHash = Crypto::Sha256(peerUdid);
    std::string timerName = std::string(STATE_TIMER_PREFIX) + sha256UdidHash + "_" + std::to_string(peerUserId) +
        "_" +  std::to_string(localUserId);
    if (iter == stateTimerInfoMap_.end()) {
        if (stateTimerInfoMap_.size() > AUTH_ONCE_STATE_TIMER_MAX) {
            LOGW("stateTimerInfoMap_ size more than limit");
            return;
        }
        StateTimerInfo stateTimer = {
            .timerName = timerName,
            .peerUdid = peerUdid,
            .peerUserId = peerUserId,
            .localUserId = localUserId,
            .isStart = true,
        };
        stateTimerInfoMap_[key] = stateTimer;
    } else {
        iter->second.isStart = true;
    }
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    timer_->StartTimer(timerName, OFFLINE_TIMEOUT, [this] (std::string name) {
        DmDeviceStateManager::DeleteTimeOutGroup(name);
    });
}
#endif
} // namespace DistributedHardware
} // namespace OHOS