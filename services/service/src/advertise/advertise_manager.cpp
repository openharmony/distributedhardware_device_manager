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

#include "advertise_manager.h"

#include "dm_constants.h"
#include "dm_log.h"
#include "dm_publish_info.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t AUTO_STOP_ADVERTISE_DEFAULT_TIME = 120;
const std::string AUTO_STOP_ADVERTISE_TASK = "AutoStopAdvertisingTask";

AdvertiseManager::AdvertiseManager(std::shared_ptr<SoftbusListener> softbusListener) : softbusListener_(softbusListener)
{
    LOGI("AdvertiseManager constructor.");
}

AdvertiseManager::~AdvertiseManager()
{
    LOGI("AdvertiseManager destructor.");
}

int32_t AdvertiseManager::StartAdvertising(const std::string &pkgName,
    const std::map<std::string, std::string> &advertiseParam)
{
    LOGI("AdvertiseManager::StartAdvertising begin for pkgName = %{public}s.", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmPublishInfo dmPubInfo;
    ConfigAdvParam(advertiseParam, &dmPubInfo);
    std::string capability = DM_CAPABILITY_OSD;
    if (advertiseParam.find(PARAM_KEY_DISC_CAPABILITY) != advertiseParam.end()) {
        capability = advertiseParam.find(PARAM_KEY_DISC_CAPABILITY)->second;
    }
    std::string customData = "";
    if (advertiseParam.find(PARAM_KEY_CUSTOM_DATA) != advertiseParam.end()) {
        customData = advertiseParam.find(PARAM_KEY_CUSTOM_DATA)->second;
    }

    int32_t ret = softbusListener_->PublishSoftbusLNN(dmPubInfo, capability, customData);
    if (ret != DM_OK) {
        LOGE("StartAdvertising failed, softbus publish lnn ret: %{public}d", ret);
        return ERR_DM_START_ADVERTISING_FAILED;
    }

    if (advertiseParam.find(PARAM_KEY_AUTO_STOP_ADVERTISE) != advertiseParam.end()) {
        int32_t stopTime = std::atoi((advertiseParam.find(PARAM_KEY_AUTO_STOP_ADVERTISE)->second).c_str());
        if ((stopTime <= 0) || (stopTime > AUTO_STOP_ADVERTISE_DEFAULT_TIME)) {
            LOGE("StartAdvertising error, invalid input auto stop advertise time: %{public}d", stopTime);
            return DM_OK;
        }
        if (timer_ == nullptr) {
            timer_ = std::make_shared<DmTimer>();
        }
        int32_t publishId = dmPubInfo.publishId;
        timer_->StartTimer(std::string(AUTO_STOP_ADVERTISE_TASK), stopTime,
            [this, pkgName, publishId] (std::string name) {
                AdvertiseManager::HandleAutoStopAdvertise(name, pkgName, publishId);
            });
    }
    return DM_OK;
}

void AdvertiseManager::ConfigAdvParam(const std::map<std::string, std::string> &advertiseParam,
    DmPublishInfo *dmPubInfo)
{
    if (dmPubInfo == nullptr) {
        LOGE("ConfigAdvParam failed, dmPubInfo is nullptr.");
        return;
    }
    dmPubInfo->publishId = -1;
    dmPubInfo->mode = DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE;
    dmPubInfo->freq = DmExchangeFreq::DM_LOW;
    dmPubInfo->ranging = true;

    if (advertiseParam.find(PARAM_KEY_META_TYPE) != advertiseParam.end()) {
        LOGI("StartAdvertising input MetaType=%{public}s", (advertiseParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    if (advertiseParam.find(PARAM_KEY_PUBLISH_ID) != advertiseParam.end()) {
        dmPubInfo->publishId = std::atoi((advertiseParam.find(PARAM_KEY_PUBLISH_ID)->second).c_str());
    }
    if (advertiseParam.find(PARAM_KEY_DISC_MODE) != advertiseParam.end()) {
        dmPubInfo->mode =
            static_cast<DmDiscoverMode>(std::atoi((advertiseParam.find(PARAM_KEY_DISC_MODE)->second).c_str()));
    }
    if (advertiseParam.find(PARAM_KEY_DISC_FREQ) != advertiseParam.end()) {
        dmPubInfo->freq =
            static_cast<DmExchangeFreq>(std::atoi((advertiseParam.find(PARAM_KEY_DISC_FREQ)->second).c_str()));
    }
}

int32_t AdvertiseManager::StopAdvertising(const std::string &pkgName, int32_t publishId)
{
    LOGI("AdvertiseManager::StopDiscovering begin for pkgName = %{public}s.", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return softbusListener_->StopPublishSoftbusLNN(publishId);
}

void AdvertiseManager::HandleAutoStopAdvertise(const std::string &timerName, const std::string &pkgName,
    int32_t publishId)
{
    LOGI("HandleAutoStopAdvertise, auto stop advertise task timeout, timerName=%{public}s", timerName.c_str());
    StopAdvertising(pkgName, publishId);
}
} // namespace DistributedHardware
} // namespace OHOS
