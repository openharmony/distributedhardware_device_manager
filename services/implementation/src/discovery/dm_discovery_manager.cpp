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

#include "dm_discovery_manager.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
const std::string DISCOVERY_TIMEOUT_TASK = TIMER_PREFIX + "discovery";
const int32_t DISCOVERY_TIMEOUT = 120;

DmDiscoveryManager::DmDiscoveryManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                                       std::shared_ptr<IDeviceManagerServiceListener> listener)
    : softbusConnector_(softbusConnector), listener_(listener)
{
    LOGI("DmDiscoveryManager constructor");
}

DmDiscoveryManager::~DmDiscoveryManager()
{
    LOGI("DmDiscoveryManager destructor");
}

int32_t DmDiscoveryManager::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                                 const std::string &extra)
{
    if (!discoveryQueue_.empty()) {
        if (pkgName == discoveryQueue_.front()) {
            LOGE("DmDiscoveryManager::StartDeviceDiscovery repeated, pkgName:%s", pkgName.c_str());
            return ERR_DM_DISCOVERY_REPEATED;
        } else {
            LOGI("DmDiscoveryManager::StartDeviceDiscovery stop preview discovery first, the preview pkgName is %s",
                 discoveryQueue_.front().c_str());
            StopDeviceDiscovery(discoveryQueue_.front(), discoveryContextMap_[discoveryQueue_.front()].subscribeId);
        }
    }
    std::lock_guard<std::mutex> autoLock(locks_);
    discoveryQueue_.push(pkgName);
    DmDiscoveryContext context = {pkgName, extra, subscribeInfo.subscribeId};
    discoveryContextMap_.emplace(pkgName, context);
    softbusConnector_->RegisterSoftbusDiscoveryCallback(pkgName,
                                                        std::shared_ptr<ISoftbusDiscoveryCallback>(shared_from_this()));
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    timer_->StartTimer(DISCOVERY_TIMEOUT_TASK, DISCOVERY_TIMEOUT,
        [this] (std::string name) {
            DmDiscoveryManager::HandleDiscoveryTimeout(name);
        });
    return softbusConnector_->StartDiscovery(subscribeInfo);
}

int32_t DmDiscoveryManager::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    std::lock_guard<std::mutex> autoLock(locks_);
    if (!discoveryQueue_.empty()) {
        discoveryQueue_.pop();
    }
    if (!discoveryContextMap_.empty()) {
        discoveryContextMap_.erase(pkgName);
        softbusConnector_->UnRegisterSoftbusDiscoveryCallback(pkgName);
        timer_->DeleteTimer(DISCOVERY_TIMEOUT_TASK);
    }
    return softbusConnector_->StopDiscovery(subscribeId);
}

void DmDiscoveryManager::OnDeviceFound(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("DmDiscoveryManager::OnDeviceFound deviceId=%s", GetAnonyString(info.deviceId).c_str());
    auto iter = discoveryContextMap_.find(pkgName);
    if (iter == discoveryContextMap_.end()) {
        LOGE("subscribeId not found by pkgName %s", GetAnonyString(pkgName).c_str());
        return;
    }
    listener_->OnDeviceFound(pkgName, iter->second.subscribeId, info);
}

void DmDiscoveryManager::OnDiscoveryFailed(const std::string &pkgName, int32_t subscribeId, int32_t failedReason)
{
    LOGI("DmDiscoveryManager::OnDiscoveryFailed subscribeId=%d reason=%d", subscribeId, failedReason);
    StopDeviceDiscovery(pkgName, (uint32_t)subscribeId);
    listener_->OnDiscoveryFailed(pkgName, (uint32_t)subscribeId, failedReason);
}

void DmDiscoveryManager::OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId)
{
    LOGI("DmDiscoveryManager::OnDiscoverySuccess subscribeId=%d", subscribeId);
    discoveryContextMap_[pkgName].subscribeId = (uint32_t)subscribeId;
    listener_->OnDiscoverySuccess(pkgName, subscribeId);
}

void DmDiscoveryManager::HandleDiscoveryTimeout(std::string name)
{
    LOGI("DmDiscoveryManager::HandleDiscoveryTimeout");
    if (discoveryQueue_.empty()) {
        LOGE("HandleDiscoveryTimeout: discovery queue is empty.");
        return;
    }

    std::string pkgName = discoveryQueue_.front();
    auto iter = discoveryContextMap_.find(pkgName);
    if (iter == discoveryContextMap_.end()) {
        LOGE("HandleDiscoveryTimeout: subscribeId not found by pkgName %s", GetAnonyString(pkgName).c_str());
        return;
    }
    StopDeviceDiscovery(pkgName, discoveryContextMap_[pkgName].subscribeId);
}
} // namespace DistributedHardware
} // namespace OHOS
