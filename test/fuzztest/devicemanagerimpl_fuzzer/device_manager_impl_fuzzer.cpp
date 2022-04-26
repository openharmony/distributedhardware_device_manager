/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <securec.h>
#include <uv.h>
#include <string>
#include <iostream>
#include "device_manager_impl.h"
#include "device_manager.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "device_manager_callback.h"
#include "dm_app_image_info.h"
#include "dm_device_info.h"
#include "dm_native_event.h"
#include "dm_subscribe_info.h"
#include "nlohmann/json.hpp"
#include "native_devicemanager_js.h"
#include "device_manager_impl_fuzzer.h"

const int nCapabiltyBufferSize = 65;

namespace OHOS {
namespace DistributedHardware {

class DeviceDiscoveryCallbackTest : public DiscoveryCallback {
public:
    DeviceDiscoveryCallbackTest() : DiscoveryCallback() {}
    virtual ~DeviceDiscoveryCallbackTest() {}
    virtual void OnDiscoverySuccess(uint16_t subscribeId) override {}
    virtual void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override {}
    virtual void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) override {}
};

class DmInitCallbackTest : public DmInitCallback {
public:
    DmInitCallbackTest() : DmInitCallback() {}
    virtual ~DmInitCallbackTest() override {}
    virtual void OnRemoteDied() override {}
};

class DeviceStateCallbackTest : public DeviceStateCallback {
public:
    DeviceStateCallbackTest() : DeviceStateCallback() {}
    virtual ~DeviceStateCallbackTest() override {}
    virtual void OnDeviceOnline(const DmDeviceInfo &deviceInfo) override {}
    virtual void OnDeviceReady(const DmDeviceInfo &deviceInfo) override {}
    virtual void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override {}
    virtual void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override {}
};

class DeviceManagerFaCallbackTest : public DeviceManagerFaCallback {
public:
    DeviceManagerFaCallbackTest() : DeviceManagerFaCallback() {}
    virtual ~DeviceManagerFaCallbackTest() override {}
    virtual void OnCall(const std::string &paramJson) override {}
};

void InitDeviceManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::string packName(reinterpret_cast<const char*>(data), size);
    std::string bundleName(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();

    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

void DeviceListFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string extra(reinterpret_cast<const char*>(data), size);
    std::vector<DmDeviceInfo> devList;
    std::string bundleName(reinterpret_cast<const char*>(data), size);
    DmDeviceInfo deviceInfo;

    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(bundleName, extra, devList);
    ret = DeviceManager::GetInstance().GetLocalDeviceInfo(bundleName, deviceInfo);
}

void DeviceDiscoveryFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::string bundleName(reinterpret_cast<const char*>(data), size);

    DmSubscribeInfo subInfo;
    subInfo.subscribeId = *(reinterpret_cast<const uint16_t*>(data));
    subInfo.mode = *(reinterpret_cast<const DmDiscoverMode*>(data));
    subInfo.medium = *(reinterpret_cast<const DmExchangeMedium*>(data));
    subInfo.freq = *(reinterpret_cast<const DmExchangeFreq*>(data));
    subInfo.isSameAccount = *(reinterpret_cast<const bool*>(data));
    subInfo.isWakeRemote = *(reinterpret_cast<const bool*>(data));
    strncpy_s(subInfo.capability, DM_MAX_DEVICE_CAPABILITY_LEN, (char*)data, nCapabiltyBufferSize);
    std::string extra(reinterpret_cast<const char*>(data), size);
    int16_t subscribeId = *(reinterpret_cast<const int16_t*>(data));

    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(bundleName,
        subInfo, extra, callback);
    ret = DeviceManager::GetInstance().StopDeviceDiscovery(bundleName, subscribeId);
}

void AuthenticateDeviceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string pkgName(reinterpret_cast<const char*>(data), size);
    int32_t authType = *(reinterpret_cast<const int32_t*>(data));
    DmDeviceInfo deviceInfo;
    std::string extraString(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<AuthenticateCallback> callback = nullptr;

    DeviceManager::GetInstance().AuthenticateDevice(pkgName,
        authType, deviceInfo, extraString, callback);
}

void UnAuthenticateDeviceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string bundleName(reinterpret_cast<const char*>(data), size);
    std::string deviceId(reinterpret_cast<const char*>(data), size);

    DeviceManager::GetInstance().UnAuthenticateDevice(bundleName, deviceId);
}

void RegisterDeviceManagerFaCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string bundleName(reinterpret_cast<const char*>(data), size);
    std::string packageName(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<DeviceManagerFaCallbackTest> callback = std::make_shared<DeviceManagerFaCallbackTest>();

    DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packageName, callback);
}

void UnRegisterDeviceManagerFaCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string packName(reinterpret_cast<const char*>(data), size);
    DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(packName);
}

void GetFaParamFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string packName(reinterpret_cast<const char*>(data), size);
    DmAuthParam authParam;

    DeviceManager::GetInstance().GetFaParam(packName, authParam);
}

void SetUserOperationFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    int32_t action = *(reinterpret_cast<const int32_t*>(data));

    DeviceManager::GetInstance().SetUserOperation(pkgName, action);
}

void GetUdidByNetworkIdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string netWorkId(reinterpret_cast<const char*>(data), size);
    std::string udid(reinterpret_cast<const char*>(data), size);

    DeviceManager::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
}

void GetUuidByNetworkIdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string packName(reinterpret_cast<const char*>(data), size);
    std::string netWorkId(reinterpret_cast<const char*>(data), size);
    std::string uuid(reinterpret_cast<const char*>(data), size);

    DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
}

void RegisterDevStateCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::string bundleName(reinterpret_cast<const char*>(data), size);
    std::string extra(reinterpret_cast<const char*>(data), size);

    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(bundleName,
        extra);
    ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(bundleName);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::InitDeviceManagerFuzzTest(data, size);
    OHOS::DistributedHardware::DeviceListFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterDevStateCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::DeviceDiscoveryFuzzTest(data, size);
    OHOS::DistributedHardware::AuthenticateDeviceFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterDeviceManagerFaCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::UnRegisterDeviceManagerFaCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::GetFaParamFuzzTest(data, size);
    OHOS::DistributedHardware::SetUserOperationFuzzTest(data, size);
    OHOS::DistributedHardware::GetUdidByNetworkIdFuzzTest(data, size);
    OHOS::DistributedHardware::GetUuidByNetworkIdFuzzTest(data, size);

    return 0;
}
