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
#include "dm_random.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t AUTO_STOP_ADVERTISE_DEFAULT_TIME = 120;
const std::string AUTO_STOP_ADVERTISE_TASK = "AutoStopAdvertisingTask";
const int32_t DM_MIN_RANDOM = 1;
const int32_t DM_MAX_RANDOM = INT32_MAX;
const int32_t DM_INVALID_FLAG_ID = 0;

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
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("begin for pkgName = %{public}s.", pkgName.c_str());
    DmPublishInfo dmPubInfo;
    ConfigAdvParam(advertiseParam, &dmPubInfo, pkgName);
    std::string capability = DM_CAPABILITY_OSD;
    if (advertiseParam.find(PARAM_KEY_DISC_CAPABILITY) != advertiseParam.end()) {
        capability = advertiseParam.find(PARAM_KEY_DISC_CAPABILITY)->second;
    }
    if (capability == DM_CAPABILITY_APPROACH || capability == DM_CAPABILITY_TOUCH) {
        dmPubInfo.mode = DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE;
    }
    if (capability == DM_CAPABILITY_OOP) {
        dmPubInfo.ranging = false;
    }
    std::string customData = "";
    if (advertiseParam.find(PARAM_KEY_CUSTOM_DATA) != advertiseParam.end()) {
        customData = advertiseParam.find(PARAM_KEY_CUSTOM_DATA)->second;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->PublishSoftbusLNN(dmPubInfo, capability, customData);
    if (ret != DM_OK) {
        LOGE("failed, softbus publish lnn ret: %{public}d", ret);
        return ret;
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
    DmPublishInfo *dmPubInfo, const std::string &pkgName)
{
    if (dmPubInfo == nullptr) {
        LOGE("ConfigAdvParam failed, dmPubInfo is nullptr.");
        return;
    }
    dmPubInfo->publishId = -1;
    dmPubInfo->mode = DmDiscoverMode::DM_DISCOVER_MODE_PASSIVE;
    dmPubInfo->freq = DmExchangeFreq::DM_LOW;
    dmPubInfo->ranging = true;
    dmPubInfo->medium = DmExchangeMedium::DM_AUTO;

    if (advertiseParam.find(PARAM_KEY_META_TYPE) != advertiseParam.end()) {
        LOGI("StartAdvertising input MetaType=%{public}s", (advertiseParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    if (advertiseParam.find(PARAM_KEY_PUBLISH_ID) != advertiseParam.end() &&
        IsNumberString(advertiseParam.find(PARAM_KEY_PUBLISH_ID)->second)) {
            int32_t publishId = std::atoi((advertiseParam.find(PARAM_KEY_PUBLISH_ID)->second).c_str());
            LOGI("PublishId=%{public}d", publishId);
            dmPubInfo->publishId = GenInnerPublishId(pkgName, publishId);
    }
    if (advertiseParam.find(PARAM_KEY_DISC_MODE) != advertiseParam.end()) {
        dmPubInfo->mode =
            static_cast<DmDiscoverMode>(std::atoi((advertiseParam.find(PARAM_KEY_DISC_MODE)->second).c_str()));
    }
    if (advertiseParam.find(PARAM_KEY_DISC_FREQ) != advertiseParam.end()) {
        dmPubInfo->freq =
            static_cast<DmExchangeFreq>(std::atoi((advertiseParam.find(PARAM_KEY_DISC_FREQ)->second).c_str()));
    }
    if (advertiseParam.find(PARAM_KEY_DISC_MEDIUM) != advertiseParam.end()) {
        if (IsNumberString(advertiseParam.find(PARAM_KEY_DISC_MEDIUM)->second)) {
            dmPubInfo->medium =
                static_cast<DmExchangeMedium>(std::atoi((advertiseParam.find(PARAM_KEY_DISC_MEDIUM)->second).c_str()));
        }
    }
}

int32_t AdvertiseManager::StopAdvertising(const std::string &pkgName, int32_t publishId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("begin for pkgName = %{public}s, publishId = %{public}d.", pkgName.c_str(), publishId);
    int32_t innerPublishId = GetAndRemoveInnerPublishId(pkgName, publishId);
    if (innerPublishId == DM_INVALID_FLAG_ID) {
        LOGE("Failed: cannot find pkgName in cache map.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    return softbusListener_->StopPublishSoftbusLNN(innerPublishId);
}

void AdvertiseManager::HandleAutoStopAdvertise(const std::string &timerName, const std::string &pkgName,
    int32_t publishId)
{
    LOGI("auto stop advertise task timeout, timerName=%{public}s", timerName.c_str());
    StopAdvertising(pkgName, publishId);
}

int32_t AdvertiseManager::GenInnerPublishId(const std::string &pkgName, int32_t publishId)
{
    int32_t tempPublishId = DM_INVALID_FLAG_ID;
    {
        std::lock_guard<std::mutex> autoLock(pubMapLock_);
        CHECK_SIZE_RETURN(pkgName2PubIdMap_, DM_INVALID_FLAG_ID);
        CHECK_SIZE_RETURN(publishIdSet_, DM_INVALID_FLAG_ID);
        if (pkgName2PubIdMap_[pkgName].find(publishId) != pkgName2PubIdMap_[pkgName].end()) {
            return pkgName2PubIdMap_[pkgName][publishId];
        }
        if (pkgName2PubIdMap_.find(pkgName) == pkgName2PubIdMap_.end()) {
            pkgName2PubIdMap_[pkgName] = std::map<int32_t, int32_t>();
        }
        bool isExist = false;
        do {
            tempPublishId = GenRandInt(DM_MIN_RANDOM, DM_MAX_RANDOM);
            if (publishIdSet_.find(tempPublishId) != publishIdSet_.end() ||
                tempPublishId == DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID) {
                LOGE("The tempPublishId: %{public}d is exist.", tempPublishId);
                isExist = true;
            } else {
                isExist = false;
            }
        } while (isExist);
        publishIdSet_.emplace(tempPublishId);
        pkgName2PubIdMap_[pkgName][publishId] = tempPublishId;
    }
    return tempPublishId;
}

int32_t AdvertiseManager::GetAndRemoveInnerPublishId(const std::string &pkgName, int32_t publishId)
{
    int32_t tempPublishId = DM_INVALID_FLAG_ID;
    {
        std::lock_guard<std::mutex> autoLock(pubMapLock_);
        if (pkgName2PubIdMap_.find(pkgName) != pkgName2PubIdMap_.end() &&
            pkgName2PubIdMap_[pkgName].find(publishId) != pkgName2PubIdMap_[pkgName].end()) {
                tempPublishId = pkgName2PubIdMap_[pkgName][publishId];
                pkgName2PubIdMap_[pkgName].erase(publishId);
                publishIdSet_.erase(tempPublishId);
        }
        if (pkgName2PubIdMap_[pkgName].empty()) {
            pkgName2PubIdMap_.erase(pkgName);
        }
    }
    return tempPublishId;
}

void AdvertiseManager::ClearPublishIdCache(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("Begin for pkgName = %{public}s.", pkgName.c_str());
    CHECK_NULL_VOID(softbusListener_);
    std::lock_guard<std::mutex> autoLock(pubMapLock_);
    for (auto iter : pkgName2PubIdMap_[pkgName]) {
        softbusListener_->StopPublishSoftbusLNN(iter.second);
        publishIdSet_.erase(iter.second);
    }
    pkgName2PubIdMap_.erase(pkgName);
}
} // namespace DistributedHardware
} // namespace OHOS
