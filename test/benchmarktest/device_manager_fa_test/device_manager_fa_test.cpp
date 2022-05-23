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

#include "device_manager.h"
#include "dm_app_image_info.h"
#include "dm_subscribe_info.h"
#include "device_manager_callback.h"
#include "dm_constants.h"

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
    const string extra = "";
    const string bundleName = "";
    const string extraString = "";
    const string packageName = "";
    const int32_t authType = 0;
};

class GetTrustedDeviceListTest : public DeviceManagerFaTest {
 public:
    void SetUp(const ::benchmark::State &state) override {}
    void TearDown(const ::benchmark::State &state) override {}
};

class DeviceDiscoveryTest : public DeviceManagerFaTest {
public:
    void SetUp(const ::benchmark::State &state) override
    {
        DeviceManager::GetInstance().StopDeviceDiscovery(bundleName, subscribeId);
        usleep(usleepTime);
    }
    void TearDown(const ::benchmark::State &state) override
    {
        DeviceManager::GetInstance().StopDeviceDiscovery(bundleName, subscribeId);
        usleep(usleepTime);
    }
protected:
    const int16_t subscribeId = 0;
};

class AuthenticateDeviceTest : public DeviceManagerFaTest {
public:
    void TearDown(const ::benchmark::State &state) override
    {
        DmDeviceInfo deviceInfo;
        DeviceManager::GetInstance().UnAuthenticateDevice(bundleName, deviceInfo);
        usleep(usleepTime);
    }
};

class UnAuthenticateDeviceTest : public DeviceManagerFaTest {
public:
    void SetUp(const ::benchmark::State &state) override
    {
        DmDeviceInfo deviceInfo;
        std::shared_ptr<AuthenticateCallback> callback = nullptr;
        DeviceManager::GetInstance().AuthenticateDevice(pkgName,
        authType, deviceInfo, extraString, callback);
        usleep(usleepTime);
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
        DeviceManager::GetInstance().UnRegisterDevStateCallback(bundleName);
        usleep(usleepTime);
    }
};

// GetTrustedDeviceList
BENCHMARK_F(GetTrustedDeviceListTest, GetTrustedDeviceListTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {   
        std::vector<DmDeviceInfo> devList {};
        int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(bundleName, extra, devList);
        if (ret != DM_OK) {
            state.SkipWithError("GetTrustedDeviceListTestCase failed.");
        }
    }
}

// StartDeviceDiscovery
BENCHMARK_F(DeviceDiscoveryTest, StartDeviceDiscoveryTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        DmSubscribeInfo subInfo;
        std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
        int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(bundleName,
        subInfo, extra, callback);
        if (ret != DM_OK) {
            state.SkipWithError("StartDeviceDiscoveryTestCase faild.");
        }
    }
}

// StopDeviceDiscovery
BENCHMARK_F(DeviceDiscoveryTest, StoptDeviceDiscoveryTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        state.PauseTiming();
        DmSubscribeInfo subInfo;
        std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
        int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(bundleName,
        subInfo, extra, callback);	
        if (ret != DM_OK) {
            state.SkipWithError("StopDeviceDiscoveryTestCase faild.");
        }
        state.ResumeTiming();
        ret =DeviceManager::GetInstance().StopDeviceDiscovery(bundleName, subscribeId);
        if (ret != DM_OK) {
            state.SkipWithError("StopDeviceDiscoveryTestCase faild.");
        }
    }
}

// AuthenticateDevice
BENCHMARK_F(AuthenticateDeviceTest, AuthenticateDeviceTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        DmDeviceInfo deviceInfo;
        std::shared_ptr<AuthenticateCallback> callback = nullptr;
        int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(pkgName,
        authType, deviceInfo, extraString, callback);
		
        if (ret != DM_OK) {
            state.SkipWithError("AuthenticateDeviceTestCase faild.");
        }
    }   
}

// UnAuthenticateDevice
BENCHMARK_F(UnAuthenticateDeviceTest, UnAuthenticateDeviceTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        state.PauseTiming();
        DmDeviceInfo deviceInfo;
        std::shared_ptr<AuthenticateCallback> callback = nullptr;
        int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(pkgName,
        authType, deviceInfo, extraString, callback);
        if (ret != DM_OK) {
            state.SkipWithError("UnAuthenticateDeviceTestCase faild.");
        }
        state.ResumeTiming();
        ret = DeviceManager::GetInstance().UnAuthenticateDevice(bundleName, deviceInfo);	
        if (ret != DM_OK) {
            state.SkipWithError("UnAuthenticateDeviceTestCase faild.");
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
            state.SkipWithError("AuthenticateDeviceTestCase faild.");
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
            state.SkipWithError("AuthenticateDeviceTestCase faild.");
        }
        state.ResumeTiming();
        ret = DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
        if (ret != DM_OK) {
            state.SkipWithError("UnRegisterDeviceManagerFaCallbackTestCase faild.");
        }
    }
}

// RegisterDevStateCallback
BENCHMARK_F(RegisterDevStateTest, RegisterDevStateCallbackTestCase)(
    benchmark::State &state)
{
    while (state.KeepRunning()) {
        int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(bundleName,extra);
        if (ret != DM_OK) {
            state.SkipWithError("RegisterDevStateCallbackTestCase faild.");
        }
    }  
}
}

// Run the benchmark
BENCHMARK_MAIN();

