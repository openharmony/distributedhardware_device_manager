/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "device_manager_utils.h"

#include <string>
#include <map>
#include <mutex>

#include "device_manager_impl.h"
#include "dm_common_util.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS::DistributedHardware {

namespace {
    constexpr std::size_t MAX_MALLOC_SIZE = 0x10000;
}

void DmFfiInitCallback::OnRemoteDied()
{
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnRemoteDied, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    }
    LOGI("OnRemoteDied, deviceManagerFfi bundleName %{public}s", bundleName_.c_str());
}

void DmFfiDeviceStatusCallback::OnDeviceOnline(const DmDeviceBasicInfo &deviceBasicInfo)
{
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnDeviceOnline, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnDeviceStatusChange(DmFfiDevStatusChange::UNKNOWN, deviceBasicInfo);
    }
}

void DmFfiDeviceStatusCallback::OnDeviceReady(const DmDeviceBasicInfo &deviceBasicInfo)
{
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnDeviceReady, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnDeviceStatusChange(DmFfiDevStatusChange::AVAILABLE, deviceBasicInfo);
    }
}

void DmFfiDeviceStatusCallback::OnDeviceOffline(const DmDeviceBasicInfo &deviceBasicInfo)
{
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnDeviceOffline, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnDeviceStatusChange(DmFfiDevStatusChange::UNAVAILABLE, deviceBasicInfo);
    }
}

void DmFfiDeviceStatusCallback::OnDeviceChanged(const DmDeviceBasicInfo &deviceBasicInfo)
{
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnDeviceChanged, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnDeviceNameChange(deviceBasicInfo.deviceName);
    }
}

void DmFfiDiscoveryCallback::OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGI("OnDeviceFound for %{public}s, subscribeId %{public}d", bundleName_.c_str(), (int32_t)subscribeId);
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnDeviceFound, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnDeviceFound(subscribeId, deviceBasicInfo);
    }
}

void DmFfiDiscoveryCallback::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    LOGI("OnDiscoveryFailed for %{public}s, subscribeId %{public}d", bundleName_.c_str(), (int32_t)subscribeId);
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnDiscoveryFailed, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnDiscoveryFailed(subscribeId, failedReason);
    }
}

void DmFfiDiscoveryCallback::OnDiscoverySuccess(uint16_t subscribeId)
{
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnDiscoverySuccess, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
        return;
    }
    LOGI("DiscoverySuccess for %{public}s, subscribeId %{public}d", bundleName_.c_str(), (int32_t)subscribeId);
}

void DmFfiDiscoveryCallback::IncreaseRefCount()
{
    refCount_++;
}

void DmFfiDiscoveryCallback::DecreaseRefCount()
{
    refCount_--;
}

int32_t DmFfiDiscoveryCallback::GetRefCount()
{
    return refCount_;
}

void DmFfiPublishCallback::OnPublishResult(int32_t publishId, int32_t publishResult)
{
    LOGI("OnPublishResult for %{public}s, publishId %{public}d, publishResult %{public}d", bundleName_.c_str(),
        publishId, publishResult);
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnPublishResult, deviceManagerFfi failed for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnPublishResult(publishId, publishResult);
    }
}

void DmFfiPublishCallback::IncreaseRefCount()
{
    refCount_++;
}

void DmFfiPublishCallback::DecreaseRefCount()
{
    refCount_--;
}

int32_t DmFfiPublishCallback::GetRefCount()
{
    return refCount_;
}

void DmFfiAuthenticateCallback::OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
    int32_t reason)
{
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnAuthResult, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnAuthResult(deviceId, token, status, reason);
    }
}

void DmFfiDeviceManagerUiCallback::OnCall(const std::string &paramJson)
{
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnCall, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnDmUiCall(paramJson);
    }
}

void DmFfiBindTargetCallback::OnBindResult(const PeerTargetId &targetId, int32_t result, int32_t status,
    std::string content)
{
    (void)targetId;
    DeviceManagerFfiImpl *deviceManagerFfi = DeviceManagerFfiImpl::GetDeviceManagerFfi(bundleName_);
    if (deviceManagerFfi == nullptr) {
        LOGE("OnBindResult, deviceManagerFfi not find for bundleName %{public}s", bundleName_.c_str());
    } else {
        deviceManagerFfi->OnAuthResult(content, "", status, result);
    }
}

const std::string &GetDeviceTypeById(DmDeviceType type)
{
    const static std::pair<const DmDeviceType, const std::string &> mapArray[] = {
        {DEVICE_TYPE_UNKNOWN, std::string(DEVICE_TYPE_UNKNOWN_STRING)},
        {DEVICE_TYPE_PHONE, std::string(DEVICE_TYPE_PHONE_STRING)},
        {DEVICE_TYPE_PAD, std::string(DEVICE_TYPE_PAD_STRING)},
        {DEVICE_TYPE_TV, std::string(DEVICE_TYPE_TV_STRING)},
        {DEVICE_TYPE_CAR, std::string(DEVICE_TYPE_CAR_STRING)},
        {DEVICE_TYPE_WATCH, std::string(DEVICE_TYPE_WATCH_STRING)},
        {DEVICE_TYPE_WIFI_CAMERA, std::string(DEVICE_TYPE_WIFICAMERA_STRING)},
        {DEVICE_TYPE_PC, std::string(DEVICE_TYPE_PC_STRING)},
        {DEVICE_TYPE_SMART_DISPLAY, std::string(DEVICE_TYPE_SMART_DISPLAY_STRING)},
        {DEVICE_TYPE_2IN1, std::string(DEVICE_TYPE_2IN1_STRING)},
        {DEVICE_TYPE_GLASSES, std::string(DEVICE_TYPE_GLASSES_STRING)},
    };
    for (const auto& item : mapArray) {
        if (item.first == type) {
            return item.second;
        }
    }
    return mapArray[0].second;
}

char *MallocCStr(const char *in)
{
    std::size_t len = strlen(in);
    if (len >= MAX_MALLOC_SIZE) {
        return nullptr;
    }
    char *result = static_cast<char *>(malloc(len + 1));
    if (result == nullptr) {
        LOGE("Malloc failed.");
        return nullptr;
    }
    std::char_traits<char>::copy(result, in, len + 1);
    return result;
}
} // OHOS::DistributedHardware
