/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ohos.distributedDeviceManager.proj.hpp"
#include "ohos.distributedDeviceManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "dm_log.h"
#include "ohos.distributedDeviceManager.h"
#include <string>
#include "device_manager.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include <iostream>

#define DH_LOG_TAG "DeviceManager"

namespace ANI::distributedDeviceManager {

const double DEVICE_TYPE_UNKNOWN = -1.0;
const std::string DEVICE_TYPE_EMPTY_STR = "";
const std::string ERROR_DEVICE_ID = "error deviceId";
const std::string ERROR_NETWORK_ID = "error networkId";

std::mutex g_initCallbackMapMutex_;
std::map<std::string, std::shared_ptr<DmAniInitCallback>> g_initCallbackMap_;

DeviceManagerImpl::DeviceManagerImpl(const std::string& bundleName) : bundleName_(bundleName)
{
    LOGI("ohos.distributedDeviceManager.cpp DeviceManagerImpl constructed with bundleName: %s", bundleName.c_str());
}

DeviceManagerImpl::DeviceManagerImpl(std::shared_ptr<DeviceManagerImpl> impl)
{
    LOGI("ohos.distributedDeviceManager.cpp DeviceManagerImpl copy constructed");
    bundleName_ = impl->bundleName_;
}

std::string DeviceManagerImpl::getLocalDeviceId()
{
    LOGI("ohos.distributedDeviceManager.cpp GetLocalDeviceId in");
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "GetLocalDeviceId for failed1");
        return DEVICE_TYPE_EMPTY_STR;
    }
    std::string deviceId;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceId(bundleName_, deviceId);
    if (ret != 0) {
        taihe::set_business_error(DM_ERR_FAILED, "GetLocalDeviceId for failed2");
        return ERROR_DEVICE_ID;
    }
    return std::string(deviceId);
}

void DeviceManagerImpl::unbindTarget(taihe::string_view deviceId)
{
    LOGI("ohos.distributedDeviceManager.cpp UnBindDevice");
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnBindDevice(
        bundleName_, std::string(deviceId));
    if (ret != 0) {
        LOGE("UnBindDevice for bundleName %{public}s failed, ret %{public}d",
            bundleName_.c_str(), ret);
        taihe::set_business_error(DM_ERR_FAILED, "unbindTarget for failed");
        return;
    }
}

double DeviceManagerImpl::getDeviceType(taihe::string_view networkId)
{
    LOGI("ohos.distributedDeviceManager.cpp GetDeviceType in");
    int32_t deviceType;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceType(
        bundleName_, std::string(networkId), deviceType);
    if (ret != 0) {
        taihe::set_business_error(DM_ERR_FAILED, "GetDeviceType for failed");
        return DEVICE_TYPE_UNKNOWN;
    }
    return static_cast<double>(deviceType);
}

std::string DeviceManagerImpl::getDeviceName(taihe::string_view networkId)
{
    LOGI("ohos.distributedDeviceManager.cpp GetDeviceName in");
    std::string deviceName;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceName(
        bundleName_, std::string(networkId), deviceName);
    if (ret != 0) {
        taihe::set_business_error(DM_ERR_FAILED, "GetDeviceName for failed");
        return DEVICE_TYPE_EMPTY_STR;
    }
    return std::string(deviceName);
}

std::string DeviceManagerImpl::getLocalDeviceNetworkId()
{
    LOGI("ohos.distributedDeviceManager.cpp GetLocalDeviceNetworkId in");
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "getLocalDeviceNetworkId failed");
        return DEVICE_TYPE_EMPTY_STR;
    }
    std::string networkId;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceNetWorkId(
        bundleName_, networkId);
    if (ret != 0) {
        LOGE("GetLocalDeviceNetworkId for failed, ret %{public}d", ret);
        taihe::set_business_error(DM_ERR_FAILED, "GetLocalDeviceNetworkId failed");
        return ERROR_NETWORK_ID;
    }
    return std::string(networkId);
}

void DeviceManagerImpl::ondeviceNameChange(taihe::callback_view<taihe::string(taihe::string_view)> f)
{
    TH_THROW(std::runtime_error, "ondeviceNameChange not implemented");
}

void DeviceManagerImpl::ondiscoverFailure(taihe::callback_view<void(int32_t)> f)
{
    TH_THROW(std::runtime_error, "ondiscoverFailure not implemented");
}

void DeviceManagerImpl::onreplyResult(taihe::callback_view<taihe::string(taihe::string_view)> f)
{
    TH_THROW(std::runtime_error, "onreplyResult not implemented");
}

void DeviceManagerImpl::DeviceManagerImpl::onserviceDie(taihe::callback_view<taihe::string(taihe::string_view)> f)
{
    TH_THROW(std::runtime_error, "onserviceDie not implemented");
}

void DeviceManagerImpl::ondiscoverSuccess(taihe::callback_view<ohos::distributedDeviceManager::DeviceBasicInfo(
    ohos::distributedDeviceManager::DeviceBasicInfo const&)> f)
{
    TH_THROW(std::runtime_error, "ondiscoverSuccess not implemented");
}

void DeviceManagerImpl::ondeviceStateChange(taihe::callback_view<ohos::distributedDeviceManager::DeviceStatusStructer(
    ohos::distributedDeviceManager::DeviceStatusStructer const&)> f)
{
    TH_THROW(std::runtime_error, "ondeviceStateChange not implemented");
}

void DeviceManagerImpl::offdeviceNameChange(taihe::callback_view<taihe::string(taihe::string_view)> f)
{
    TH_THROW(std::runtime_error, "offdeviceNameChange not implemented");
}

void DeviceManagerImpl::offdiscoverFailure(taihe::callback_view<void(int32_t)> f)
{
    TH_THROW(std::runtime_error, "offdiscoverFailure not implemented");
}

void DeviceManagerImpl::offreplyResult(taihe::callback_view<taihe::string(taihe::string_view)> f)
{
    TH_THROW(std::runtime_error, "offreplyResult not implemented");
}

void DeviceManagerImpl::offserviceDie(taihe::callback_view<taihe::string(taihe::string_view)> f)
{
    TH_THROW(std::runtime_error, "offserviceDie not implemented");
}

void DeviceManagerImpl::offdiscoverSuccess(taihe::callback_view<ohos::distributedDeviceManager::DeviceBasicInfo(
    ohos::distributedDeviceManager::DeviceBasicInfo const &)> f)
{
    TH_THROW(std::runtime_error, "offdiscoverSuccess not implemented");
}

void DeviceManagerImpl::offdeviceStateChange(taihe::callback_view<ohos::distributedDeviceManager::DeviceStatusStructer(
    ohos::distributedDeviceManager::DeviceStatusStructer const&)> f)
{
    TH_THROW(std::runtime_error, "offdeviceStateChange not implemented");
}

ohos::distributedDeviceManager::DeviceManager createDeviceManager(taihe::string_view bundleName)
{
    auto impl = std::make_shared<DeviceManagerImpl>(std::string(bundleName));
    std::shared_ptr<DmAniInitCallback> initCallback = std::make_shared<DmAniInitCallback>(bundleName);
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(
        std::string(bundleName), initCallback);
    if (ret != 0) {
        LOGE("CreateDeviceManager for bundleName %{public}s failed, ret %{public}d.", bundleName.c_str(), ret);
        taihe::set_business_error(DM_ERR_FAILED, "CreateDeviceManager for failed");
        return taihe::make_holder<DeviceManagerImpl, ohos::distributedDeviceManager::DeviceManager>();
    }
    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex_);
        g_initCallbackMap_[std::string(bundleName)] = initCallback;
    }
    return taihe::make_holder<DeviceManagerImpl, ohos::distributedDeviceManager::DeviceManager>(impl);
}
} // namespace ANI::distributedDeviceManager

void ANI::distributedDeviceManager::DmAniInitCallback::OnRemoteDied()
{
    LOGI("ohos.distributedDeviceManager.cpp DmAniInitCallback::OnRemoteDied called.");
    auto& deviceManager = static_cast<OHOS::DistributedHardware::DeviceManager&>(
        OHOS::DistributedHardware::DeviceManager::GetInstance());
    deviceManager.UnInitDeviceManager(bundleName_);
}

TH_EXPORT_CPP_API_createDeviceManager(ANI::distributedDeviceManager::createDeviceManager);
