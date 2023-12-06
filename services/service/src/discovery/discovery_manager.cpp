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

#include "discovery_manager.h"

#include <securec.h>

#include "discovery_filter.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t DISCOVERY_TIMEOUT = 120;
const uint16_t DM_INVALID_FLAG_ID = 0;
constexpr const char* LNN_DISC_CAPABILITY = "capability";
constexpr const char* DISCOVERY_TIMEOUT_TASK = "deviceManagerTimer:discovery";
constexpr const char* DM_PKG_NAME_APPROACH = "ohos.distributedhardware.devicemanager.approach";

DiscoveryManager::DiscoveryManager(std::shared_ptr<SoftbusListener> softbusListener,
    std::shared_ptr<IDeviceManagerServiceListener> listener) : softbusListener_(softbusListener), listener_(listener)
{
    LOGI("DiscoveryManager constructor.");
}

DiscoveryManager::~DiscoveryManager()
{
    LOGI("DiscoveryManager destructor.");
}

int32_t DiscoveryManager::EnableDiscoveryListener(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    LOGI("DiscoveryManager::EnableDiscoveryListener begin for pkgName = %s.", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmSubscribeInfo dmSubInfo;
    dmSubInfo.subscribeId = DM_INVALID_FLAG_ID;
    dmSubInfo.mode = DmDiscoverMode::DM_DISCOVER_MODE_PASSIVE;
    dmSubInfo.medium = DmExchangeMedium::DM_BLE;
    dmSubInfo.freq = DmExchangeFreq::DM_LOW;
    dmSubInfo.isSameAccount = false;
    dmSubInfo.isWakeRemote = false;
    strcpy_s(dmSubInfo.capability, DM_MAX_DEVICE_CAPABILITY_LEN, DM_CAPABILITY_APPROACH);

    if (discoverParam.find(PARAM_KEY_META_TYPE) != discoverParam.end()) {
        std::string metaType = discoverParam.find(PARAM_KEY_META_TYPE)->second;
        LOGI("EnableDiscoveryListener, input MetaType = %s in discoverParam map.", metaType.c_str());
    }
    if (discoverParam.find(PARAM_KEY_SUBSCRIBE_ID) != discoverParam.end()) {
        dmSubInfo.subscribeId = std::atoi((discoverParam.find(PARAM_KEY_SUBSCRIBE_ID)->second).c_str());
        pkgName2SubIdMap_[pkgName] = dmSubInfo.subscribeId;
    }

    int32_t ret = softbusListener_->RefreshSoftbusLNN(DM_PKG_NAME_APPROACH, dmSubInfo, LNN_DISC_CAPABILITY);
    if (ret != DM_OK) {
        LOGE("EnableDiscoveryListener failed, softbus refresh lnn ret: %d.", ret);
        return ERR_DM_ENABLE_DISCOVERY_LISTENER_FAILED;
    }
    softbusListener_->RegisterSoftbusLnnOpsCbk(pkgName, std::shared_ptr<ISoftbusLnnOpsCallback>(shared_from_this()));
    return DM_OK;
}

int32_t DiscoveryManager::DisableDiscoveryListener(const std::string &pkgName,
    const std::map<std::string, std::string> &extraParam)
{
    LOGI("DiscoveryManager::DisableDiscoveryListener begin for pkgName = %s.", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    if (extraParam.find(PARAM_KEY_META_TYPE) != extraParam.end()) {
        LOGI("DisableDiscoveryListener, input MetaType = %s", (extraParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    uint16_t subscribeId = DM_INVALID_FLAG_ID;
    if (extraParam.find(PARAM_KEY_SUBSCRIBE_ID) != extraParam.end()) {
        subscribeId = std::atoi((extraParam.find(PARAM_KEY_SUBSCRIBE_ID)->second).c_str());
        pkgName2SubIdMap_.erase(pkgName);
    }
    softbusListener_->UnRegisterSoftbusLnnOpsCbk(pkgName);
    return softbusListener_->StopRefreshSoftbusLNN(subscribeId);
}

int32_t DiscoveryManager::StartDiscovering(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    LOGI("DiscoveryManager::StartDiscovering begin for pkgName = %s.", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmSubscribeInfo dmSubInfo;
    dmSubInfo.subscribeId = DM_INVALID_FLAG_ID;
    dmSubInfo.mode = DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE;
    dmSubInfo.medium = DmExchangeMedium::DM_AUTO;
    dmSubInfo.freq = DmExchangeFreq::DM_SUPER_HIGH;
    dmSubInfo.isSameAccount = false;
    dmSubInfo.isWakeRemote = false;
    if (discoverParam.find(PARAM_KEY_SUBSCRIBE_ID) != discoverParam.end()) {
        dmSubInfo.subscribeId = std::atoi((discoverParam.find(PARAM_KEY_SUBSCRIBE_ID)->second).c_str());
    }
    if (discoverParam.find(PARAM_KEY_DISC_MEDIUM) != discoverParam.end()) {
        int32_t medium = std::atoi((discoverParam.find(PARAM_KEY_DISC_MEDIUM)->second).c_str());
        dmSubInfo.medium = static_cast<DmExchangeMedium>(medium);
    }
    if (HandleDiscoveryQueue(pkgName, dmSubInfo.subscribeId, filterOptions) != DM_OK) {
        return ERR_DM_DISCOVERY_REPEATED;
    }

    bool isStandardMetaNode = true;
    if (discoverParam.find(PARAM_KEY_META_TYPE) != discoverParam.end()) {
        MetaNodeType metaType = (MetaNodeType)(std::atoi((discoverParam.find(PARAM_KEY_META_TYPE)->second).c_str()));
        isStandardMetaNode = (metaType == MetaNodeType::PROXY_TRANSMISION);
    }

    int32_t ret = DM_OK;
    if (isStandardMetaNode) {
        LOGI("StartDiscovering for standard meta node process.");
        ret = StartDiscoveringNoMetaType(dmSubInfo, discoverParam);
    } else {
        LOGI("StartDiscovering for meta node process, input metaType = %s",
            (discoverParam.find(PARAM_KEY_META_TYPE)->second).c_str());
        ret = StartDiscovering4MetaType(dmSubInfo, discoverParam);
    }
    if (ret != DM_OK) {
        LOGE("StartDiscovering for meta node process failed, ret = %d", ret);
        return ERR_DM_START_DISCOVERING_FAILED;
    }

    StartDiscoveryTimer();
    return softbusListener_->RegisterSoftbusLnnOpsCbk(pkgName,
        std::shared_ptr<ISoftbusLnnOpsCallback>(shared_from_this()));
}

int32_t DiscoveryManager::StartDiscoveringNoMetaType(DmSubscribeInfo &dmSubInfo,
    const std::map<std::string, std::string> &param)
{
    (void)param;
    strcpy_s(dmSubInfo.capability, DM_MAX_DEVICE_CAPABILITY_LEN, DM_CAPABILITY_OSD);

    int32_t ret = softbusListener_->RefreshSoftbusLNN(DM_PKG_NAME, dmSubInfo, LNN_DISC_CAPABILITY);
    if (ret != DM_OK) {
        LOGE("StartDiscoveringNoMetaType failed, softbus refresh lnn ret: %d.", ret);
    }
    return ret;
}

int32_t DiscoveryManager::StartDiscovering4MetaType(DmSubscribeInfo &dmSubInfo,
    const std::map<std::string, std::string> &param)
{
    MetaNodeType metaType = (MetaNodeType)(std::atoi((param.find(PARAM_KEY_META_TYPE)->second).c_str()));
    switch (metaType) {
        case MetaNodeType::PROXY_SHARE:
            LOGI("StartDiscovering4MetaType for share meta node process.");
            strcpy_s(dmSubInfo.capability, DM_MAX_DEVICE_CAPABILITY_LEN, DM_CAPABILITY_SHARE);
            break;
        case MetaNodeType::PROXY_WEAR:
            LOGI("StartDiscovering4MetaType for wear meta node process.");
            strcpy_s(dmSubInfo.capability, DM_MAX_DEVICE_CAPABILITY_LEN, DM_CAPABILITY_WEAR);
            break;
        case MetaNodeType::PROXY_CASTPLUS:
            LOGI("StartDiscovering4MetaType for cast_plus meta node process.");
            strcpy_s(dmSubInfo.capability, DM_MAX_DEVICE_CAPABILITY_LEN, DM_CAPABILITY_CASTPLUS);
            break;
        default:
            LOGE("StartDiscovering4MetaType failed, unsupport meta type : %d.", metaType);
            return ERR_DM_UNSUPPORTED_METHOD;
    }

    std::string customData = "";
    if (param.find(PARAM_KEY_CUSTOM_DATA) != param.end()) {
        customData = param.find(PARAM_KEY_CUSTOM_DATA)->second;
    }

    int32_t ret = softbusListener_->RefreshSoftbusLNN(DM_PKG_NAME, dmSubInfo, customData);
    if (ret != DM_OK) {
        LOGE("StartDiscovering4MetaType failed, softbus refresh lnn ret: %d.", ret);
    }
    return ret;
}

int32_t DiscoveryManager::StopDiscovering(const std::string &pkgName, uint16_t subscribeId)
{
    LOGI("DiscoveryManager::StopDiscovering begin for pkgName = %s.", pkgName.c_str());
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
    softbusListener_->UnRegisterSoftbusLnnOpsCbk(pkgName);
    return softbusListener_->StopRefreshSoftbusLNN(subscribeId);
}

void DiscoveryManager::OnDeviceFound(const std::string &pkgName, const DmDeviceInfo &info, bool isOnline)
{
    DiscoveryContext discoveryContext;
    {
        std::lock_guard<std::mutex> autoLock(locks_);
        auto iter = discoveryContextMap_.find(pkgName);
        if (iter == discoveryContextMap_.end()) {
            listener_->OnDeviceFound(pkgName, pkgName2SubIdMap_[pkgName], info);
            return;
        }
        discoveryContext = iter->second;
    }
    DiscoveryFilter filter;
    DeviceFilterPara filterPara;
    filterPara.isOnline = isOnline;
    filterPara.range = info.range;
    filterPara.deviceType = info.deviceTypeId;
    if (filter.IsValidDevice(discoveryContext.filterOp, discoveryContext.filters, filterPara)) {
        listener_->OnDeviceFound(pkgName, discoveryContext.subscribeId, info);
    }
}

void DiscoveryManager::OnDiscoveringResult(const std::string &pkgName, int32_t subscribeId, int32_t result)
{
    LOGI("DiscoveryManager::OnDiscoveringResult, subscribeId = %d, result = %d.", subscribeId, result);
    if (pkgName.empty() || (listener_ == nullptr)) {
        LOGE("DiscoveryManager::OnDiscoveringResult failed, IDeviceManagerServiceListener is null.");
        return;
    }
    if (result == 0) {
        std::lock_guard<std::mutex> autoLock(locks_);
        discoveryContextMap_[pkgName].subscribeId = (uint32_t)subscribeId;
        listener_->OnDiscoverySuccess(pkgName, subscribeId);
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
    listener_->OnDiscoveryFailed(pkgName, (uint32_t)subscribeId, result);
    softbusListener_->StopRefreshSoftbusLNN(subscribeId);
}

void DiscoveryManager::OnAdvertisingResult(const std::string &pkgName, int32_t publishId, int32_t result)
{
    (void)pkgName;
    (void)publishId;
    (void)result;
    LOGI("DiscoveryManager::OnAdvertisingResult, ignore.");
}

void DiscoveryManager::StartDiscoveryTimer()
{
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    timer_->StartTimer(std::string(DISCOVERY_TIMEOUT_TASK), DISCOVERY_TIMEOUT,
        [this] (std::string name) {
            DiscoveryManager::HandleDiscoveryTimeout(name);
        });
}

int32_t DiscoveryManager::HandleDiscoveryQueue(const std::string &pkgName, uint16_t subscribeId,
    const std::map<std::string, std::string> &filterOps)
{
    std::string filterData = "";
    if (filterOps.find(PARAM_KEY_FILTER_OPTIONS) != filterOps.end()) {
        filterData = filterOps.find(PARAM_KEY_FILTER_OPTIONS)->second;
    }
    DeviceFilterOption dmFilter;
    if ((dmFilter.TransformToFilter(filterData) != DM_OK) && (dmFilter.TransformFilterOption(filterData) != DM_OK)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::lock_guard<std::mutex> autoLock(locks_);
    if (!discoveryQueue_.empty()) {
        std::string frontPkgName = discoveryQueue_.front();
        if (pkgName == frontPkgName) {
            LOGE("DiscoveryManager::HandleDiscoveryQueue repeated, pkgName : %s.", pkgName.c_str());
            return ERR_DM_DISCOVERY_REPEATED;
        }
        StopDiscovering(frontPkgName, discoveryContextMap_[frontPkgName].subscribeId);
    }

    discoveryQueue_.push(pkgName);
    DiscoveryContext context = {pkgName, filterData, subscribeId, dmFilter.filterOp_, dmFilter.filters_};
    discoveryContextMap_.emplace(pkgName, context);
    return DM_OK;
}

void DiscoveryManager::HandleDiscoveryTimeout(std::string name)
{
    (void)name;
    LOGI("DiscoveryManager::HandleDiscoveryTimeout");
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
            LOGE("HandleDiscoveryTimeout: subscribeId not found by pkgName %s.", GetAnonyString(pkgName).c_str());
            return;
        }
        subscribeId = discoveryContextMap_[pkgName].subscribeId;
    }
    StopDiscovering(pkgName, subscribeId);
}
} // namespace DistributedHardware
} // namespace OHOS
