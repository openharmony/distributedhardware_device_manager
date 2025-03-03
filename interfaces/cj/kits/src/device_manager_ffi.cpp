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
#include "device_manager_ffi.h"

#include <string>

#include "device_manager.h"
#include "device_manager_impl.h"
#include "dm_log.h"

extern "C" {
int64_t FfiOHOSDistributedDeviceManagerCreateDeviceManager(const char *name, int32_t *errCode)
{
    auto deviceManager = OHOS::FFI::FFIData::Create<OHOS::DistributedHardware::DeviceManagerFfiImpl>(
        std::string(name), errCode);
    if (deviceManager == nullptr) {
        return 0;
    }
    return deviceManager->GetID();
}

void FfiOHOSDistributedDeviceManagerReleaseDeviceManager(int64_t id, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return;
    }
    *errCode = instance->ReleaseDeviceManager();
}

void FfiOHOSDistributedDeviceManagerGetAvailableDeviceList(
    int64_t id, FfiDeviceBasicInfoArray *deviceInfoList, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return;
    }
    *errCode = instance->GetAvailableDeviceList(*deviceInfoList);
}

void FfiOHOSDistributedDeviceManagerFreeDeviceInfoList(FfiDeviceBasicInfoArray deviceInfoList)
{
    OHOS::DistributedHardware::DeviceManagerFfiImpl::DeviceListFree(deviceInfoList);
}

const char *FfiOHOSDistributedDeviceManagerGetLocalDeviceNetworkId(int64_t id, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    const char *networkIdPtr = nullptr;
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return networkIdPtr;
    }
    *errCode = instance->GetLocalDeviceNetworkId(networkIdPtr);
    return networkIdPtr;
}

const char *FfiOHOSDistributedDeviceManagerGetLocalDeviceName(int64_t id, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    const char *deviceName = nullptr;
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return deviceName;
    }
    *errCode = instance->GetLocalDeviceName(deviceName);
    return deviceName;
}

int32_t FfiOHOSDistributedDeviceManagerGetLocalDeviceType(int64_t id, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    int32_t deviceType = 0;
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return deviceType;
    }
    *errCode = instance->GetLocalDeviceType(deviceType);
    return deviceType;
}

const char *FfiOHOSDistributedDeviceManagerGetLocalDeviceId(int64_t id, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    const char *deviceId = nullptr;
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return deviceId;
    }
    *errCode = instance->GetLocalDeviceId(deviceId);
    return deviceId;
}

const char *FfiOHOSDistributedDeviceManagerGetDeviceName(int64_t id, const char *networkId, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    const char *deviceName = nullptr;
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return deviceName;
    }
    *errCode = instance->GetDeviceName(networkId, deviceName);
    return deviceName;
}

int32_t FfiOHOSDistributedDeviceManagerGetDeviceType(int64_t id, const char *networkId, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    int32_t deviceType = 0;
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return deviceType;
    }
    *errCode = instance->GetDeviceType(networkId, deviceType);
    return deviceType;
}

void FfiOHOSDistributedDeviceManagerStartDiscovering(int64_t id, const char *extra, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return;
    }
    *errCode = instance->StartDiscovering(extra);
}

void FfiOHOSDistributedDeviceManagerStopDiscovering(int64_t id, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return;
    }
    *errCode = instance->StopDiscovering();
}

void FfiOHOSDistributedDeviceManagerBindTarget(
    int64_t id, const char *deviceId, const char *bindParam, bool isMetaType, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return;
    }
    *errCode = instance->BindTarget(deviceId, bindParam, isMetaType);
}

void FfiOHOSDistributedDeviceManagerUnbindTarget(int64_t id, const char *deviceId, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return;
    }
    *errCode = instance->UnbindTarget(deviceId);
}

void FfiOHOSDistributedDeviceManagerOn(int64_t id, const char *type, void *callback, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return;
    }
    *errCode = instance->EventOn(type, callback);
}

void FfiOHOSDistributedDeviceManagerOff(int64_t id, const char *type, int32_t *errCode)
{
    auto instance = OHOS::FFI::FFIData::GetData<OHOS::DistributedHardware::DeviceManagerFfiImpl>(id);
    if (instance == nullptr) {
        LOGE("instance is nullptr");
        int32_t errSerive = 11600102;
        *errCode = errSerive;
        return;
    }
    *errCode = instance->EventOff(type);
}
}
