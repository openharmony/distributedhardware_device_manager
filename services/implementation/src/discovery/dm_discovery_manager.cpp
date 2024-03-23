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

#include "dm_discovery_manager.h"
#include "dm_discovery_filter.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
constexpr const char* DISCOVERY_TIMEOUT_TASK = "deviceManagerTimer:discovery";
const int32_t DISCOVERY_TIMEOUT = 120;

DmDiscoveryManager::DmDiscoveryManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                                       std::shared_ptr<IDeviceManagerServiceListener> listener,
                                       std::shared_ptr<HiChainConnector> hiChainConnector)
    : softbusConnector_(softbusConnector), listener_(listener), hiChainConnector_(hiChainConnector)
{
    LOGI("DmDiscoveryManager constructor");
}

DmDiscoveryManager::~DmDiscoveryManager()
{
    LOGI("DmDiscoveryManager destructor");
}

void DmDiscoveryManager::CfgDiscoveryTimer()
{
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    timer_->StartTimer(std::string(DISCOVERY_TIMEOUT_TASK), DISCOVERY_TIMEOUT,
        [this] (std::string name) {
            DmDiscoveryManager::HandleDiscoveryTimeout(name);
        });
}

int32_t DmDiscoveryManager::CheckDiscoveryQueue(const std::string &pkgName)
{
    uint16_t subscribeId = 0;
    std::string frontPkgName = "";
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        if (discoveryQueue_.empty()) {
            return DM_OK;
        }

        frontPkgName = discoveryQueue_.front();
        if (pkgName == frontPkgName) {
            LOGE("DmDiscoveryManager::StartDeviceDiscovery repeated, pkgName:%{public}s", pkgName.c_str());
            return ERR_DM_DISCOVERY_REPEATED;
        }

        LOGI("DmDiscoveryManager::StartDeviceDiscovery stop preview discovery first, the preview pkgName is %{public}s",
            discoveryQueue_.front().c_str());
        subscribeId = discoveryContextMap_[frontPkgName].subscribeId;
    }
    StopDeviceDiscovery(frontPkgName, subscribeId);
    return DM_OK;
}

int32_t DmDiscoveryManager::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
    const std::string &extra)
{
    DmDeviceFilterOption dmFilter;
    if (dmFilter.TransformToFilter(extra) != DM_OK) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (CheckDiscoveryQueue(pkgName) != DM_OK) {
        return ERR_DM_DISCOVERY_REPEATED;
    }
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        discoveryQueue_.push(pkgName);
        DmDiscoveryContext context = {pkgName, extra, subscribeInfo.subscribeId, dmFilter.filterOp_, dmFilter.filters_};
        discoveryContextMap_.emplace(pkgName, context);
    }
    softbusConnector_->RegisterSoftbusDiscoveryCallback(pkgName,
        std::shared_ptr<ISoftbusDiscoveryCallback>(shared_from_this()));
    CfgDiscoveryTimer();
    return softbusConnector_->StartDiscovery(subscribeInfo);
}

int32_t DmDiscoveryManager::StartDeviceDiscovery(const std::string &pkgName, const uint16_t subscribeId,
    const std::string &filterOptions)
{
    DmDeviceFilterOption dmFilter;
    dmFilter.TransformFilterOption(filterOptions);
    if (CheckDiscoveryQueue(pkgName) != DM_OK) {
        return ERR_DM_DISCOVERY_REPEATED;
    }
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        discoveryQueue_.push(pkgName);
        DmDiscoveryContext context = {pkgName, filterOptions, subscribeId, dmFilter.filterOp_, dmFilter.filters_};
        discoveryContextMap_.emplace(pkgName, context);
    }
    softbusConnector_->RegisterSoftbusDiscoveryCallback(pkgName,
        std::shared_ptr<ISoftbusDiscoveryCallback>(shared_from_this()));
    CfgDiscoveryTimer();
    return softbusConnector_->StartDiscovery(subscribeId);
}

int32_t DmDiscoveryManager::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        if (!discoveryQueue_.empty()) {
            discoveryQueue_.pop();
        }
        if (!discoveryContextMap_.empty()) {
            discoveryContextMap_.erase(pkgName);
            timer_->DeleteTimer(std::string(DISCOVERY_TIMEOUT_TASK));
        }
    }
    softbusConnector_->UnRegisterSoftbusDiscoveryCallback(pkgName);
    return softbusConnector_->StopDiscovery(subscribeId);
}

void DmDiscoveryManager::OnDeviceFound(const std::string &pkgName, DmDeviceInfo &info, bool isOnline)
{
    LOGI("DmDiscoveryManager::OnDeviceFound deviceId = %{public}s", GetAnonyString(info.deviceId).c_str());
    DmDiscoveryContext discoveryContext;
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        auto iter = discoveryContextMap_.find(pkgName);
        if (iter == discoveryContextMap_.end()) {
            LOGE("subscribeId not found by pkgName %{public}s", GetAnonyString(pkgName).c_str());
            return;
        }
        discoveryContext = iter->second;
    }
    DmDiscoveryFilter filter;
    DmDeviceFilterPara filterPara;
    filterPara.isOnline = isOnline;
    filterPara.range = info.range;
    filterPara.deviceType = info.deviceTypeId;
    char localDeviceId[DEVICE_UUID_LENGTH];
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    info.authForm = DmAuthForm::INVALID_TYPE;
    GetAuthForm(localDeviceId, info.deviceId, filterPara.isTrusted, info.authForm);
    filterPara.authForm = info.authForm;
    if (filter.IsValidDevice(discoveryContext.filterOp, discoveryContext.filters, filterPara)) {
        listener_->OnDeviceFound(pkgName, discoveryContext.subscribeId, info);
    }
    return;
}

int32_t DmDiscoveryManager::GetAuthForm(const std::string &localDeviceId, const std::string &deviceId,
    bool &isTrusted, DmAuthForm &authForm)
{
    LOGI("Get localDeviceId: %{public}s auth form.", GetAnonyString(localDeviceId).c_str());
    isTrusted = false;
    if (localDeviceId.empty() || deviceId.empty()) {
        LOGE("Invalid parameter.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (hiChainConnector_ == nullptr || softbusConnector_ == nullptr) {
        LOGE("hiChainConnector_ or softbusConnector_ is nullpter.");
        return ERR_DM_POINT_NULL;
    }

    std::vector<std::string> trustDeviceUdidList = hiChainConnector_->GetTrustedDevices(localDeviceId);
    if (trustDeviceUdidList.empty()) {
        LOGI("Trusted devices is empty.");
        return DM_OK;
    }
    std::string udidHash;
    for (auto udid : trustDeviceUdidList) {
        udidHash = softbusConnector_->GetDeviceUdidHashByUdid(udid);
        if (udidHash == deviceId) {
            isTrusted = true;
            authForm = hiChainConnector_->GetGroupType(udid);
            LOGI("deviceId: %{public}s is trusted!", GetAnonyString(deviceId).c_str());
        }
    }

    return DM_OK;
}

void DmDiscoveryManager::OnDeviceFound(const std::string &pkgName,
    DmDeviceBasicInfo &info, const int32_t range, bool isOnline)
{
    LOGI("DmDiscoveryManager::OnDeviceFound deviceId = %{public}s,isOnline %{public}d",
        GetAnonyString(info.deviceId).c_str(), isOnline);
    DmDiscoveryContext discoveryContext;
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        auto iter = discoveryContextMap_.find(pkgName);
        if (iter == discoveryContextMap_.end()) {
            LOGE("subscribeId not found by pkgName %{public}s", GetAnonyString(pkgName).c_str());
            return;
        }
        discoveryContext = iter->second;
    }
    DmDiscoveryFilter filter;
    DmDeviceFilterPara filterPara;
    filterPara.isOnline = isOnline;
    filterPara.range = range;
    filterPara.deviceType = info.deviceTypeId;
    char localDeviceId[DEVICE_UUID_LENGTH];
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    DmAuthForm authForm = DmAuthForm::INVALID_TYPE;
    GetAuthForm(localDeviceId, info.deviceId, filterPara.isTrusted, authForm);
    filterPara.authForm = authForm;
    if (filter.IsValidDevice(discoveryContext.filterOp, discoveryContext.filters, filterPara)) {
        listener_->OnDeviceFound(pkgName, discoveryContext.subscribeId, info);
    }
    return;
}

void DmDiscoveryManager::OnDiscoveryFailed(const std::string &pkgName, int32_t subscribeId, int32_t failedReason)
{
    LOGI("DmDiscoveryManager::OnDiscoveryFailed subscribeId = %{public}d reason = %{public}d", subscribeId,
        failedReason);
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        if (!discoveryQueue_.empty()) {
            discoveryQueue_.pop();
        }
        if (!discoveryContextMap_.empty()) {
            discoveryContextMap_.erase(pkgName);
            timer_->DeleteTimer(std::string(DISCOVERY_TIMEOUT_TASK));
        }
    }
    softbusConnector_->StopDiscovery(subscribeId);
    listener_->OnDiscoveryFailed(pkgName, (uint32_t)subscribeId, failedReason);
}

void DmDiscoveryManager::OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId)
{
    LOGI("DmDiscoveryManager::OnDiscoverySuccess subscribeId = %{public}d", subscribeId);
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        discoveryContextMap_[pkgName].subscribeId = (uint32_t)subscribeId;
    }
    listener_->OnDiscoverySuccess(pkgName, subscribeId);
}

void DmDiscoveryManager::HandleDiscoveryTimeout(std::string name)
{
    (void)name;
    LOGI("DmDiscoveryManager::HandleDiscoveryTimeout");
    uint16_t subscribeId = 0;
    std::string pkgName = "";
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        if (discoveryQueue_.empty()) {
            LOGE("HandleDiscoveryTimeout: discovery queue is empty.");
            return;
        }

        pkgName = discoveryQueue_.front();
        auto iter = discoveryContextMap_.find(pkgName);
        if (iter == discoveryContextMap_.end()) {
            LOGE("HandleDiscoveryTimeout: subscribeId not found by pkgName %{public}s",
                GetAnonyString(pkgName).c_str());
            return;
        }
        subscribeId = discoveryContextMap_[pkgName].subscribeId;
    }
    StopDeviceDiscovery(pkgName, subscribeId);
}
} // namespace DistributedHardware
} // namespace OHOS
