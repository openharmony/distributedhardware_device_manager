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
#include "dm_constants.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::DistributedHardware;

namespace {
class BenchmarkDmInit : public DmInitCallback {
public:
    BenchmarkDmInit() : DmInitCallback() {}
    virtual ~BenchmarkDmInit() override {}
    virtual void OnRemoteDied() override {}
};
class DeviceManagerTest : public benchmark::Fixture {
public:
    DeviceManagerTest()
    {
        Iterations(iterations);
        Repetitions(repetitions);
        ReportAggregatesOnly();
    }

    ~DeviceManagerTest() override = default;

    void SetUp(const ::benchmark::State &state) override
    {
        std::shared_ptr<BenchmarkDmInit> callback = std::make_shared<BenchmarkDmInit>();
        DeviceManager::GetInstance().InitDeviceManager(pkgName, callback);
    }

    void TearDown(const ::benchmark::State &state) override
    {
    }
protected:
    const string pkgName = "ohos.distributedhardware.devicemanager";
    const int32_t repetitions = 3;
    const int32_t iterations = 1000;
};

// InitDeviceManager
BENCHMARK_F(DeviceManagerTest, InitDeviceManagerTestCase)(benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::shared_ptr<BenchmarkDmInit> callback = std::make_shared<BenchmarkDmInit>();
        int32_t ret = DeviceManager::GetInstance().InitDeviceManager(pkgName, callback);
        if (ret != DM_OK) {
            state.SkipWithError("InitDeviceManagerTestCase failed.");
        }
    }
}

// GetFaParam
BENCHMARK_F(DeviceManagerTest, GetFaParamTestCase)(benchmark::State &state)
{
    while (state.KeepRunning()) {
        DmAuthParam authParam;
        int32_t ret = DeviceManager::GetInstance().GetFaParam(pkgName, authParam);
        if (ret != DM_OK) {
            state.SkipWithError("GetFaParamTestCase. failed");
        }
    }
}

// SetUserOperation
BENCHMARK_F(DeviceManagerTest, SetUserOperationTestCase)(benchmark::State &state)
{
    while (state.KeepRunning()) {
        int32_t action = 0;
        const std::string params = "extra";
        int32_t ret = DeviceManager::GetInstance().SetUserOperation(pkgName, action, params);
        if (ret != DM_OK) {
            state.SkipWithError("SetUserOperationTestCase failed.");
        }
    }
}

// GetUdidByNetworkId
BENCHMARK_F(DeviceManagerTest, GetUdidByNetworkIdTestCase)(benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::string netWorkId = "netWorkId_";
        std::string udid = "udid_";
        int32_t ret =  DeviceManager::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
        if (ret != DM_OK) {
            state.SkipWithError("SetUserOperationTestCase failed.");
        }
    }
}

// GetUuidByNetworkId
BENCHMARK_F(DeviceManagerTest, GetUuidByNetworkIdTestCase)(benchmark::State &state)
{
    while (state.KeepRunning()) {
        std::string netWorkId = "netWorkId_";
        std::string uuid = "uuid_";
        int32_t ret =  DeviceManager::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
        if (ret != DM_OK) {
            state.SkipWithError("SetUserOperationTestCase failed.");
        }
    }
}
}

// Run the benchmark
BENCHMARK_MAIN();
