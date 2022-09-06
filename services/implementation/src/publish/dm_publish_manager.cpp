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

#include "dm_publish_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
constexpr const char*  PUBLISH_DISCOVERY_TIMEOUT_TASK = "deviceManagerTimer:publish";
const int32_t PUBLISH_DISCOVERY_TIMEOUT = 120;

DmPublishManager::DmPublishManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                                   std::shared_ptr<IDeviceManagerServiceListener> listener)
    : softbusConnector_(softbusConnector), listener_(listener)
{
    LOGI("DmPublishManager constructor");
}

DmPublishManager::~DmPublishManager()
{
    LOGI("DmPublishManager destructor");
}

void DmPublishManager::CfgPublishTimer()
{
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    timer_->StartTimer(std::string(PUBLISH_DISCOVERY_TIMEOUT_TASK), PUBLISH_DISCOVERY_TIMEOUT,
        [this] (std::string name) {
            DmPublishManager::HandlePublishTimeout(name);
        });
}

int32_t DmPublishManager::CheckPublishQueue(const std::string &pkgName)
{
    if (publishQueue_.empty()) {
        return DM_OK;
    }

    if (pkgName == publishQueue_.front()) {
        LOGE("DmPublishManager::pkgName : %s PublishDeviceDiscovery repeated", pkgName.c_str());
        return ERR_DM_PUBLISH_REPEATED;
    } else {
        LOGI("DmPublishManager::UnPublishDeviceDiscovery the preview pkgName : %s",
            publishQueue_.front().c_str());
        UnPublishDeviceDiscovery(publishQueue_.front(),
                                 publishContextMap_[publishQueue_.front()].publishInfo.publishId);
        return DM_OK;
    }
}

int32_t DmPublishManager::PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo)
{
    int32_t ret = CheckPublishQueue(pkgName);
    if (ret != DM_OK) {
        return ret;
    }
    std::lock_guard<std::mutex> autoLock(locks_);
    publishQueue_.push(pkgName);
    DmPublishContext context = {pkgName, publishInfo};
    publishContextMap_.emplace(pkgName, context);
    softbusConnector_->RegisterSoftbusPublishCallback(pkgName,
                                                      std::shared_ptr<ISoftbusPublishCallback>(shared_from_this()));
    CfgPublishTimer();
    return softbusConnector_->PublishDiscovery(publishInfo);
}

int32_t DmPublishManager::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::lock_guard<std::mutex> autoLock(locks_);
    if (!publishQueue_.empty()) {
        publishQueue_.pop();
    }
    if (!publishContextMap_.empty()) {
        publishContextMap_.erase(pkgName);
        softbusConnector_->UnRegisterSoftbusPublishCallback(pkgName);
        timer_->DeleteTimer(std::string(PUBLISH_DISCOVERY_TIMEOUT_TASK));
    }
    return softbusConnector_->UnPublishDiscovery(publishId);
}

void DmPublishManager::OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DmPublishManager::OnPublishResult, publishId = %d, publishResult = %d", publishId, publishResult);
    if (publishResult != 0) {
        UnPublishDeviceDiscovery(pkgName, publishId);
    }
    listener_->OnPublishResult(pkgName, publishId, publishResult);
}

void DmPublishManager::HandlePublishTimeout(std::string name)
{
    LOGI("DmPublishManager::HandlePublishDiscoveryTimeout");
    if (publishQueue_.empty()) {
        LOGE("HandlePublishDiscoveryTimeout: Publish discovery queue is empty.");
        return;
    }

    std::string pkgName = publishQueue_.front();
    auto iter = publishContextMap_.find(pkgName);
    if (iter == publishContextMap_.end()) {
        LOGE("HandleDiscoveryTimeout:pkgName %s fail to publish", GetAnonyString(pkgName).c_str());
        return;
    }
    UnPublishDeviceDiscovery(pkgName, iter->second.publishInfo.publishId);
}
} // namespace DistributedHardware
} // namespace OHOS
