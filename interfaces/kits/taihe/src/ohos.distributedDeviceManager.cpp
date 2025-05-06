#include "ohos.distributedDeviceManager.proj.hpp"
#include "ohos.distributedDeviceManager.impl.hpp"
#include "taihe/runtime.hpp"
#include "dm_log.h"
#include "stdexcept"
#include "ohos.distributedDeviceManager.h"
#include <string>
#include "device_manager.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include <iostream>

#define DH_LOG_TAG "DeviceManager"

using namespace ANI::distributedDeviceManager;
using namespace taihe;
namespace ANI::distributedDeviceManager {

std::mutex g_initCallbackMapMutex_;
std::map<std::string, std::shared_ptr<DmAniInitCallback>> g_initCallbackMap_;


DeviceManagerImpl::DeviceManagerImpl(const std::string& bundleName) : bundleName_(bundleName) {
    LOGI("ohos.distributedDeviceManager.cpp DeviceManagerImpl constructed with bundleName: %s", bundleName.c_str());
}
DeviceManagerImpl::DeviceManagerImpl(std::shared_ptr<DeviceManagerImpl> impl) {
    LOGI("ohos.distributedDeviceManager.cpp DeviceManagerImpl copy constructed");
    bundleName_ = impl->bundleName_;
}

string DeviceManagerImpl::getLocalDeviceId()
{
    LOGI("ohos.distributedDeviceManager.cpp GetLocalDeviceId in");
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "GetLocalDeviceId for failed1");
        return "";
    }
    std::string deviceId;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceId(bundleName_, deviceId);
    if (ret != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "GetLocalDeviceId for failed2");
        return "error deviceId";
    }

    return string(deviceId);

}

void DeviceManagerImpl::unbindTarget(string_view deviceId)
{
    LOGI("ohos.distributedDeviceManager.cpp UnBindDevice");
    int32_t ret =OHOS::DistributedHardware:: DeviceManager::GetInstance().UnBindDevice(
        bundleName_, std::string(deviceId));
    if (ret != 0) {
        LOGE("UnBindDevice for bundleName %{public}s failed, ret %{public}d",
            bundleName_.c_str(), ret);
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "unbindTarget for failed");
        return;
    }

}

double DeviceManagerImpl::getDeviceType(string_view networkId)
{
    LOGI("ohos.distributedDeviceManager.cpp GetDeviceType in");
    int32_t deviceType;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceType(bundleName_,
                                                                 std::string(networkId), deviceType);
    if (ret != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "GetDeviceType for failed");
        return -1.0;
    }
    return static_cast<double>(deviceType);
}

string DeviceManagerImpl::getDeviceName(string_view networkId)
{
    LOGI("ohos.distributedDeviceManager.cpp GetDeviceName in");
    std::string deviceName;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceName(
        bundleName_, std::string(networkId), deviceName);
    if (ret != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "GetDeviceName for failed");
        return "";
    }
    return string(deviceName);
}

string DeviceManagerImpl::getLocalDeviceNetworkId()
{
    LOGI("ohos.distributedDeviceManager.cpp GetLocalDeviceNetworkId in");
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "getLocalDeviceNetworkId for failed1");
        return "";
    }
    std::string networkId;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceNetWorkId(bundleName_, networkId);
    if (ret != 0) {
        LOGE("GetLocalDeviceNetworkId for failed, ret %{public}d", ret);
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "GetLocalDeviceNetworkId for failed2");
        return "error networkId";
    }
    return string(networkId);

}

void DeviceManagerImpl::ondeviceNameChange(callback_view<string(string_view)> f)
{
    TH_THROW(std::runtime_error, "ondeviceNameChange not implemented");
}

void DeviceManagerImpl::ondiscoverFailure(callback_view<void(int32_t)> f)
{
    TH_THROW(std::runtime_error, "ondiscoverFailure not implemented");
}

void DeviceManagerImpl::onreplyResult(callback_view<string(string_view)> f)
{
    TH_THROW(std::runtime_error, "onreplyResult not implemented");
}

void DeviceManagerImpl::DeviceManagerImpl::onserviceDie(callback_view<string(string_view)> f)
{
    TH_THROW(std::runtime_error, "onserviceDie not implemented");
}

void DeviceManagerImpl::ondiscoverSuccess(callback_view<DeviceBasicInfo(DeviceBasicInfo const&)> f)
{
    TH_THROW(std::runtime_error, "ondiscoverSuccess not implemented");
}

void DeviceManagerImpl::ondeviceStateChange(callback_view<DeviceStatusStructer(DeviceStatusStructer const&)> f)
{
    TH_THROW(std::runtime_error, "ondeviceStateChange not implemented");
}

void DeviceManagerImpl::offdeviceNameChange(callback_view<string(string_view)> f)
{
    TH_THROW(std::runtime_error, "offdeviceNameChange not implemented");
}

void DeviceManagerImpl::offdiscoverFailure(callback_view<void(int32_t)> f)
{
    TH_THROW(std::runtime_error, "offdiscoverFailure not implemented");
}

void DeviceManagerImpl::offreplyResult(callback_view<string(string_view)> f)
{
    TH_THROW(std::runtime_error, "offreplyResult not implemented");
}

void DeviceManagerImpl::offserviceDie(callback_view<string(string_view)> f)
{
    TH_THROW(std::runtime_error, "offserviceDie not implemented");
}

void DeviceManagerImpl::offdiscoverSuccess(callback_view<DeviceBasicInfo(DeviceBasicInfo const&)> f)
{
    TH_THROW(std::runtime_error, "offdiscoverSuccess not implemented");
}

void DeviceManagerImpl::offdeviceStateChange(callback_view<DeviceStatusStructer(DeviceStatusStructer const&)> f)
{
    TH_THROW(std::runtime_error, "offdeviceStateChange not implemented");
}

DeviceManager createDeviceManager(string_view bundleName)
{
    auto impl = std::make_shared<DeviceManagerImpl>(std::string(bundleName));
    std::shared_ptr<DmAniInitCallback> initCallback = std::make_shared<DmAniInitCallback>(bundleName);
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(
        std::string(bundleName), initCallback);
    if (ret != 0) {
        LOGE("CreateDeviceManager for bundleName %{public}s failed, ret %{public}d.", bundleName.c_str(), ret);
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "CreateDeviceManager for failed");
        return make_holder<DeviceManagerImpl, DeviceManager>();
    } 
    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex_);
        g_initCallbackMap_[std::string(bundleName)] = initCallback;
    }
    return make_holder<DeviceManagerImpl, DeviceManager>(impl);
}

}
void DmAniInitCallback::OnRemoteDied()
{
    LOGI("ohos.distributedDeviceManager.cpp DmAniInitCallback::OnRemoteDied called.");

    auto& deviceManager = static_cast<OHOS::DistributedHardware::DeviceManager&>(
        OHOS::DistributedHardware::DeviceManager::GetInstance());
    deviceManager.UnInitDeviceManager(bundleName_);
}

TH_EXPORT_CPP_API_createDeviceManager(createDeviceManager);
