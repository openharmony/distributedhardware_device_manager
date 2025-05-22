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

#include "ohos.distributedDeviceManager.h"
#include <iostream>
#include <string>
#include "device_manager.h"
#include "dm_ani_callback.h"
#include "dm_constants.h"
#include "dm_log.h"

#define DH_LOG_TAG "DeviceManager"

namespace ANI::distributedDeviceManager {
namespace {
constexpr double DEVICE_TYPE_UNKNOWN = -1.0;
constexpr const char *DEVICE_TYPE_EMPTY_STR = "";
constexpr const char *ERROR_DEVICE_ID = "error deviceId";
constexpr const char *ERROR_NETWORK_ID = "error networkId";

std::mutex g_initCallbackMapMutex;
std::mutex g_deviceNameChangeCallbackMapMutex;
std::mutex g_discoveryFailedCallbackMapMutex;
std::mutex g_deviceStateChangeDataCallbackMapMutex;
std::mutex g_discoverySuccessCallbackMapMutex;
std::mutex g_dmUiCallbackMapMutex;

std::map<std::string, std::shared_ptr<DmAniInitCallback>> g_initCallbackMap;
std::map<std::string, std::shared_ptr<DmAniDiscoveryFailedCallback>> g_discoveryFailedCallbackMap;
std::map<std::string, std::shared_ptr<DmAniDeviceNameChangeCallback>> g_deviceNameChangeCallbackMap;
std::map<std::string, std::shared_ptr<DmAniDeviceStateChangeDataCallback>> g_deviceStateChangeDataCallbackMap;
std::map<std::string, std::shared_ptr<DmAniDiscoverySuccessCallback>> g_discoverySuccessCallbackMap;
std::map<std::string, std::shared_ptr<DmAniDeviceManagerUiCallback>> g_dmUiCallbackMap;
} //namespace

DeviceManagerImpl::DeviceManagerImpl(const std::string& bundleName)
    : bundleName_(bundleName)
{
}

DeviceManagerImpl::DeviceManagerImpl(std::shared_ptr<DeviceManagerImpl> impl)
{
    std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
    if (impl == nullptr) {
        LOGE("Create DeviceManagerImpl for shared_ptr failed.");
    } else {
        bundleName_ = impl->bundleName_;
    }
}

ohos::distributedDeviceManager::DeviceManager CreateDeviceManager(taihe::string_view bundleName)
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
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        g_initCallbackMap[std::string(bundleName)] = initCallback;
    }
    return taihe::make_holder<DeviceManagerImpl, ohos::distributedDeviceManager::DeviceManager>(impl);
}

ohos::distributedDeviceManager::DeviceBasicInfo MakeDeviceBasicInfo(taihe::string_view deviceId,
    taihe::string_view deviceName, taihe::string_view deviceType,
    taihe::string_view networkId, taihe::string_view extraData)
{
    return {deviceId, deviceName, deviceType, networkId, extraData};
}

ohos::distributedDeviceManager::DeviceResult MakeDeviceResult(taihe::string_view deviceId)
{
    return {deviceId};
}

ohos::distributedDeviceManager::DeviceResultNumber MakeDeviceResultNumber(int32_t reason)
{
    return {reason};
}

ohos::distributedDeviceManager::DeviceStateChangeAction MakeDeviceStateChangeAction(
    ohos::distributedDeviceManager::DeviceBasicInfo const& device)
{
    return {device};
}

ohos::distributedDeviceManager::DeviceStateChangeData MakeDeviceStateChangeData(
    ohos::distributedDeviceManager::DeviceStateChange deviceStateChange,
    ohos::distributedDeviceManager::DeviceBasicInfo const& deviceBasicInfo)
{
    return {deviceStateChange, deviceBasicInfo};
}

std::string DeviceManagerImpl::GetLocalDeviceId()
{
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

void DeviceManagerImpl::UnbindTarget(taihe::string_view deviceId)
{
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnBindDevice(
        bundleName_, std::string(deviceId));
    if (ret != 0) {
        LOGE("UnBindDevice for bundleName %{public}s failed, ret %{public}d",
            bundleName_.c_str(), ret);
        taihe::set_business_error(DM_ERR_FAILED, "UnbindTarget for failed");
        return;
    }
}

double DeviceManagerImpl::GetDeviceType(taihe::string_view networkId)
{
    int32_t deviceType;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceType(
        bundleName_, std::string(networkId), deviceType);
    if (ret != 0) {
        taihe::set_business_error(DM_ERR_FAILED, "GetDeviceType for failed");
        return DEVICE_TYPE_UNKNOWN;
    }
    return static_cast<double>(deviceType);
}

std::string DeviceManagerImpl::GetDeviceName(taihe::string_view networkId)
{
    std::string deviceName;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceName(
        bundleName_, std::string(networkId), deviceName);
    if (ret != 0) {
        taihe::set_business_error(DM_ERR_FAILED, "GetDeviceName for failed");
        return DEVICE_TYPE_EMPTY_STR;
    }
    return std::string(deviceName);
}

std::string DeviceManagerImpl::GetLocalDeviceNetworkId()
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "GetLocalDeviceNetworkId failed");
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

void DeviceManagerImpl::OnDeviceNameChange(taihe::callback_view<void(
    ohos::distributedDeviceManager::DeviceResult const&)> onDeviceNameChangecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "ondeviceNameChange check permission failed.");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_deviceNameChangeCallbackMapMutex);
        if (g_deviceNameChangeCallbackMap.find(bundleName_) != g_deviceNameChangeCallbackMap.end()) {
            LOGI("bundleName already register.");
            return;
        }
    }

    auto dmonDeviceNameChangecb = std::make_shared<DmAniDeviceNameChangeCallback>(bundleName_, onDeviceNameChangecb);
    std::string extra = "";
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName_, extra,
        dmonDeviceNameChangecb) != 0) {
        LOGE("RegisterDevStatusCallback failed");
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceNameChangeCallbackMapMutex);
        g_deviceNameChangeCallbackMap[bundleName_] = dmonDeviceNameChangecb;
    }
    return;
}

void DeviceManagerImpl::OnDiscoverFailure(taihe::callback_view<void(
    ohos::distributedDeviceManager::DeviceResultNumber const&)> onDiscoverFailurecb)
{
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "ondiscoverFailure check permission failed.");
        return;
    }

    auto dmDiscoverFailurecb = std::make_shared<DmAniDiscoveryFailedCallback>(bundleName_, onDiscoverFailurecb);
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryFailedCallbackMapMutex);
        g_discoveryFailedCallbackMap[bundleName_] = dmDiscoverFailurecb;
    }
    std::shared_ptr<DmAniDiscoveryFailedCallback> discoveryCallback = dmDiscoverFailurecb;
    discoveryCallback->IncreaseRefCount();
    return;
}

void DeviceManagerImpl::OnreplyResult(taihe::callback_view<void(
    ohos::distributedDeviceManager::DeviceResult const&)> onreplyResultcb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "OnreplyResult check permission failed");
        return;
    }

    auto dmcallback = std::make_shared<DmAniDeviceManagerUiCallback>(onreplyResultcb, bundleName_);
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(
        bundleName_, dmcallback);
    if (ret != 0) {
        LOGE("RegisterDeviceManagerFaCallback failed for bundleName %{public}s", bundleName_.c_str());
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        g_dmUiCallbackMap[bundleName_] = dmcallback;
    }
    return;
}

void DeviceManagerImpl::OnDiscoverSuccess(taihe::callback_view<void(
    ohos::distributedDeviceManager::DeviceStateChangeAction const&)> onDiscoverSuccesscb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "ondiscoverSuccess check Permission failed");
        return;
    }

    auto dmCallback = std::make_shared<DmAniDiscoverySuccessCallback>(bundleName_, onDiscoverSuccesscb);
    {
        std::lock_guard<std::mutex> autoLock(g_discoverySuccessCallbackMapMutex);
        g_discoverySuccessCallbackMap[bundleName_] = dmCallback;
    }
    std::shared_ptr<DmAniDiscoverySuccessCallback> discoveryCallback = dmCallback;
    discoveryCallback->IncreaseRefCount();
    return ;
}

void DeviceManagerImpl::OnDeviceStateChange(taihe::callback_view<void(
    ohos::distributedDeviceManager::DeviceStateChangeData const&)> onDeviceStateChangecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "ondeviceStateChange check Permission failed");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateChangeDataCallbackMapMutex);
        if (g_deviceStateChangeDataCallbackMap.find(bundleName_) != g_deviceStateChangeDataCallbackMap.end()) {
            LOGI("bundleName already register.");
            return;
        }
    }

    auto dmCallback = std::make_shared<DmAniDeviceStateChangeDataCallback>(bundleName_, onDeviceStateChangecb);
    std::string extra = "";
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName_,
        extra, dmCallback);
    if (ret != 0) {
        LOGE("RegisterDevStatusCallback failed ret %{public}d", ret);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateChangeDataCallbackMapMutex);
        g_deviceStateChangeDataCallbackMap[bundleName_] = dmCallback;
    }
    return;
}

void DeviceManagerImpl::OnServiceDie(taihe::callback_view<void()> onServiceDiecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "OnreplyResult check permission failed");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        auto dmCallback = g_initCallbackMap[bundleName_];
        dmCallback->SetServiceDieCallback(std::make_shared<taihe::callback<void()>>(onServiceDiecb));
    }
    return;
}

void DeviceManagerImpl::OffDeviceNameChange(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceResult const&)>> offDeviceNameChangecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "offdeviceNameChange check permission failed.");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_deviceNameChangeCallbackMapMutex);
        auto iter = g_deviceNameChangeCallbackMap.find(bundleName_);
        if (iter == g_deviceNameChangeCallbackMap.end()) {
            LOGE("ReleaseDmCallback: cannot find statusCallback for bundleName "
                "%{public}s",
                bundleName_.c_str());
            return;
        }
    }

    if (OHOS::DistributedHardware::DeviceManager::GetInstance().UnRegisterDevStatusCallback(bundleName_) != 0) {
        LOGE("UnRegisterDevStatusCallback failed for bundleName %{public}s", bundleName_.c_str());
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceNameChangeCallbackMapMutex);
        g_deviceNameChangeCallbackMap.erase(bundleName_);
    }
    return;
}

void DeviceManagerImpl::OffDiscoverFailure(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceResultNumber const&)>> offDiscoverFailurecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "offdiscoverFailure check permission failed.");
        return;
    }

    std::shared_ptr<DmAniDiscoveryFailedCallback> discoveryCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryFailedCallbackMapMutex);
        auto iter = g_discoveryFailedCallbackMap.find(bundleName_);
        if (iter == g_discoveryFailedCallbackMap.end()) {
            return;
        }
        discoveryCallback = iter->second;
    }

    discoveryCallback->DecreaseRefCount();
    if (discoveryCallback->GetRefCount() == 0) {
        std::lock_guard<std::mutex> autoLock(g_discoveryFailedCallbackMapMutex);
        g_discoveryFailedCallbackMap.erase(bundleName_);
    }
    return;
}

void DeviceManagerImpl::OffreplyResult(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceResult const&)>> offreplyResultcb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "OffreplyResult check permission failed");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        auto iter = g_dmUiCallbackMap.find(bundleName_);
        if (iter == g_dmUiCallbackMap.end()) {
            LOGE("cannot find dmFaCallback for bundleName %{public}s", bundleName_.c_str());
            return;
        }
    }

    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(
        bundleName_);
    if (ret != 0) {
        LOGE("UnRegisterDeviceManagerFaCallback failed for bundleName %{public}s", bundleName_.c_str());
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        g_dmUiCallbackMap.erase(bundleName_);
    }
}

void DeviceManagerImpl::OffDiscoverSuccess(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceStateChangeAction const&)>> offDiscoverSuccesscb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "offdiscoverSuccess check Permission failed");
        return;
    }

    std::shared_ptr<DmAniDiscoverySuccessCallback> disCoveryCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_discoverySuccessCallbackMapMutex);
        auto iter = g_discoverySuccessCallbackMap.find(bundleName_);
        if (iter == g_discoverySuccessCallbackMap.end()) {
            LOGE("Cannot find discoveryCallback for bundleName %{public}s", bundleName_.c_str());
            return;
        }
        disCoveryCallback = iter->second;
    }

    disCoveryCallback->DecreaseRefCount();
    if (disCoveryCallback->GetRefCount() == 0) {
        std::lock_guard<std::mutex> autoLock(g_discoverySuccessCallbackMapMutex);
        g_discoverySuccessCallbackMap.erase(bundleName_);
    }
    return;
}

void DeviceManagerImpl::OffDeviceStateChange(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceStateChangeData const&)>> offDeviceStateChangecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "offdeviceStateChange check Permission failed");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateChangeDataCallbackMapMutex);
        auto iter = g_deviceStateChangeDataCallbackMap.find(bundleName_);
        if (iter == g_deviceStateChangeDataCallbackMap.end()) {
            LOGE("Cannot find stateChangeDataCallback for bundleName %{public}s", bundleName_.c_str());
            return;
        }
    }

    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnRegisterDevStatusCallback(bundleName_);
    if (ret != 0) {
        LOGE("UnRegisterDevStatusCallback failed for bundleName %{public}s", bundleName_.c_str());
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateChangeDataCallbackMapMutex);
        g_deviceStateChangeDataCallbackMap.erase(bundleName_);
    }
    return;
}

void DeviceManagerImpl::OffServiceDie(taihe::optional_view<taihe::callback<void()>> offServiceDiecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "offdiscoverFailure check permission failed.");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        auto iter = g_initCallbackMap.find(bundleName_);
        if (iter == g_initCallbackMap.end() || !iter->second) {
            LOGE("Cannot find ServiceDieCallback for bundleName %{public}s", bundleName_.c_str());
            return;
        }
        iter->second->ReleaseServiceDieCallback();
    }
}
} // namespace ANI::distributedDeviceManager

TH_EXPORT_CPP_API_CreateDeviceManager(ANI::distributedDeviceManager::CreateDeviceManager);
TH_EXPORT_CPP_API_MakeDeviceBasicInfo(ANI::distributedDeviceManager::MakeDeviceBasicInfo);
TH_EXPORT_CPP_API_MakeDeviceResult(ANI::distributedDeviceManager::MakeDeviceResult);
TH_EXPORT_CPP_API_MakeDeviceResultNumber(ANI::distributedDeviceManager::MakeDeviceResultNumber);
TH_EXPORT_CPP_API_MakeDeviceStateChangeAction(ANI::distributedDeviceManager::MakeDeviceStateChangeAction);
TH_EXPORT_CPP_API_MakeDeviceStateChangeData(ANI::distributedDeviceManager::MakeDeviceStateChangeData);
