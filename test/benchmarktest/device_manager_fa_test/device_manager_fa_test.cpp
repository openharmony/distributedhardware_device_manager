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
 
#include <benchmark/benchmark.h>
#include <unistd.h>
#include <vector>
#include <securec.h>
#include <cstdlib>

#include "device_manager.h"
#include "dm_app_image_info.h"
#include "dm_subscribe_info.h"
#include "device_manager_callback.h"
#include "dm_constants.h"
#include "system_ability_definition.h"
#include "softbus_common.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::DistributedHardware;

namespace {
class DeviceDiscoveryCallbackTest : public DiscoveryCallback {
public:
    DeviceDiscoveryCallbackTest() : DiscoveryCallback() {}
    virtual ~DeviceDiscoveryCallbackTest() {}
    virtual void OnDiscoverySuccess(uint16_t subscribeId) override {}
    virtual void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override {}
    virtual void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) override {}
};

class DevicePublishCallbackTest : public PublishCallback {
public:
    DevicePublishCallbackTest() : PublishCallback() {}
    virtual ~DevicePublishCallbackTest() {}
    virtual void OnPublishResult(int32_t publishId, int32_t failedReason) override {}
};

class BenchmarkDmInit : public DmInitCallback {
public:
    BenchmarkDmInit() : DmInitCallback() {}
    virtual ~BenchmarkDmInit() override {}
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

class DeviceManagerFaCallbackTest : public DeviceManagerUiCallback {
public:
    DeviceManagerFaCallbackTest() : DeviceManagerUiCallback() {}
    virtual ~DeviceManagerFaCallbackTest() override {}
    virtual void OnCall(const std::string &paramJson) override {}
};

class DeviceManagerFaTest : public benchmark::Fixture {
public:
    DeviceManagerFaTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~DeviceManagerFaTest() override = default;

    void SetUp(const ::benchmark::State &state) override
    {
        std::shared_ptr<BenchmarkDmInit> callback = std::make_shared<BenchmarkDmInit>();
        DeviceManager::GetInstance().InitDeviceManager(pkgName, callback);
    }

    void TearDown(const ::benchmark::State &state) override
    {
    }
protected:
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
    // sleep 1000ms
    const int32_t usleepTime = 1000 * 1000;
    const string pkgName = "com.ohos.devicemanager";
    const string extra = "extra_";
    const string bundleName = "bundleName_";
    const string extraString = "extraString_";
    const string packageName = "com.ohos.devicemanager";
    const int32_t authType = 1;
};

class GetTrustedDeviceListTest : public DeviceManagerFaTest {
public:
    void SetUp(const ::benchmark::State &state) override
    {
    }
    void TearDown(const ::benchmark::State &state) override
    {
    }
};

class GetLocalDeviceInfoTest : public DeviceManagerFaTest {
public:
    void SetUp(const ::benchmark::State &state) override
    {
    }
    void TearDown(const ::benchmark::State &state) override
    {
    }
};

class DeviceDiscoveryTest : public DeviceManagerFaTest {
public:
    void SetUp(const ::benchmark::State &state) override
    {
    }
    void TearDown(const ::benchmark::State &state) override
    {
    }
};

class RegisterDeviceManagerFaTest : public DeviceManagerFaTest {
public:
    void TearDown(const ::benchmark::State &state) override
    {
        DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
        usleep(usleepTime);
    }
};

class UnRegisterDeviceManagerFaTest : public DeviceManagerFaTest {
public:
    void SetUp(const ::benchmark::State &state) override
    {
        std::shared_ptr<DeviceManagerFaCallbackTest> callback = std::make_shared<DeviceManagerFaCallbackTest>();
        DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(pkgName, callback);
        usleep(usleepTime);
    }
};

class RegisterDevStateTest : public DeviceManagerFaTest {
public:
    void TearDown(const ::benchmark::State &state) override
    {
        DeviceManager::GetInstance().UnRegisterDevStateCallback(pkgName);
        usleep(usleepTime);
    }
};

class UnRegisterDevStateTest : public DeviceManagerFaTest {
public:
    void SetUp(const ::benchmark::State &state) override
    {
    }
    void TearDown(const ::benchmark::State &state) override
    {
    }
};

// GetTrustedDeviceList
BENCHMARK_F(GetTrustedDeviceListTest, GetTrustedDeviceListTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        state.PauseTiming();
        std::shared_ptr<BenchmarkDmInit> callback = std::make_shared<BenchmarkDmInit>();
        DeviceManager::GetInstance().InitDeviceManager(pkgName, callback);
        state.ResumeTiming();
        std::vector<DmDeviceInfo> devList {};
        DmDeviceInfo deviceInfo;
        string deviceInfoId = "12345678";
        string deviceInfoName = "com.OHOS";
        deviceInfo.deviceTypeId = 0;
        string deviceNetworkId = "com.OHOS.app";
        strncpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, deviceInfoId.c_str(), deviceInfoId.length());
        strncpy_s(deviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN, deviceInfoName.c_str(), deviceInfoName.length());
        strncpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, deviceNetworkId.c_str(), deviceNetworkId.length());
        int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(pkgName, extra, devList);
        if (ret != DM_OK) {
            state.SkipWithError("GetTrustedDeviceListTestCase failed.");
        }
    }
}

// GetLocalDeviceInfo
BENCHMARK_F(GetLocalDeviceInfoTest, GetLocalDeviceInfoTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        state.PauseTiming();
        std::shared_ptr<BenchmarkDmInit> callback = std::make_shared<BenchmarkDmInit>();
        DeviceManager::GetInstance().InitDeviceManager(pkgName, callback);
        state.ResumeTiming();
        std::vector<DmDeviceInfo> devList {};
        DmDeviceInfo deviceInfo;
        string deviceInfoId = "12345678";
        string deviceInfoName = "com.OHOS";
        deviceInfo.deviceTypeId = 0;
        string deviceNetworkId = "com.OHOS.app";
        strncpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, deviceInfoId.c_str(), deviceInfoId.length());
        strncpy_s(deviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN, deviceInfoName.c_str(), deviceInfoName.length());
        strncpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, deviceNetworkId.c_str(), deviceNetworkId.length());
        int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(pkgName, deviceInfo);
        if (ret != DM_OK) {
            state.SkipWithError("GetLocalDeviceInfoTestCase failed.");
        }
    }
}

// StartDeviceDiscovery
BENCHMARK_F(DeviceDiscoveryTest, StartDeviceDiscoveryTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        string packageName = "com.devicemanager";
        state.PauseTiming();
        std::shared_ptr<BenchmarkDmInit> callback_ = std::make_shared<BenchmarkDmInit>();
        DeviceManager::GetInstance().InitDeviceManager(packageName, callback_);
        state.ResumeTiming();
        DmSubscribeInfo subInfo;
        subInfo.subscribeId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
        subInfo.mode = DM_DISCOVER_MODE_ACTIVE;
        subInfo.medium = DM_AUTO;
        subInfo.freq = DM_HIGH;
        subInfo.isSameAccount = false;
        subInfo.isWakeRemote = false;
        strcpy_s(subInfo.capability, DM_MAX_DEVICE_CAPABILITY_LEN, DM_CAPABILITY_OSD);
        std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
        std::string str;
        int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packageName, subInfo, str, callback);
        if (ret != DM_OK) {
            state.SkipWithError("StartDeviceDiscoveryTestCase failed.");
        }
        state.PauseTiming();
        uint16_t subscribeId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
        DeviceManager::GetInstance().StopDeviceDiscovery(packageName, subscribeId);
    }
}

// StopDeviceDiscovery
BENCHMARK_F(DeviceDiscoveryTest, StoptDeviceDiscoveryTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        string packageName = "com.devicemanager";
        state.PauseTiming();
        std::shared_ptr<BenchmarkDmInit> callback_ = std::make_shared<BenchmarkDmInit>();
        DeviceManager::GetInstance().InitDeviceManager(packageName, callback_);
        DmSubscribeInfo subInfo;
        subInfo.subscribeId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
        subInfo.mode = DM_DISCOVER_MODE_ACTIVE;
        subInfo.medium = DM_AUTO;
        subInfo.freq = DM_HIGH;
        subInfo.isSameAccount = false;
        subInfo.isWakeRemote = false;
        strcpy_s(subInfo.capability, DM_MAX_DEVICE_CAPABILITY_LEN, DM_CAPABILITY_OSD);
        std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
        std::string str;
        int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(pkgName,
        subInfo, str, callback);
        if (ret != DM_OK) {
            state.SkipWithError("StopDeviceDiscoveryTestCase failed.");
        }
        state.ResumeTiming();
        uint16_t subscribeId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
        ret =DeviceManager::GetInstance().StopDeviceDiscovery(packageName, subscribeId);
        if (ret != DM_OK) {
            state.SkipWithError("StopDeviceDiscoveryTestCase failed.");
        }
    }
}

// RegisterDeviceManagerFaCallback
BENCHMARK_F(RegisterDeviceManagerFaTest, RegisterDeviceManagerFaCallbackTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::shared_ptr<DeviceManagerFaCallbackTest> callback = std::make_shared<DeviceManagerFaCallbackTest>();
        int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packageName, callback);
        if (ret != DM_OK) {
            state.SkipWithError("AuthenticateDeviceTestCase failed.");
        }
    }
}

// UnRegisterDeviceManagerFaCallback
BENCHMARK_F(UnRegisterDeviceManagerFaTest, UnRegisterDeviceManagerFaCallbackTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        state.PauseTiming();
        std::shared_ptr<DeviceManagerFaCallbackTest> callback = std::make_shared<DeviceManagerFaCallbackTest>();
        int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packageName, callback);
        if (ret != DM_OK) {
            state.SkipWithError("AuthenticateDeviceTestCase failed.");
        }
        state.ResumeTiming();
        ret = DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(packageName);
        if (ret != DM_OK) {
            state.SkipWithError("UnRegisterDeviceManagerFaCallbackTestCase failed.");
        }
    }
}

// RegisterDevStateCallback
BENCHMARK_F(RegisterDevStateTest, RegisterDevStateCallbackTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extra);
        if (ret != DM_OK) {
            state.SkipWithError("RegisterDevStateCallbackTestCase failed.");
        }
    }
}

// UnRegisterDevStateCallback
BENCHMARK_F(UnRegisterDevStateTest, UnRegisterDevStateCallbackTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(pkgName);
        if (ret != DM_OK) {
            state.SkipWithError("UnRegisterDevStateCallbackTestCase failed.");
        }
    }
}
}

// Run the benchmark
BENCHMARK_MAIN();
