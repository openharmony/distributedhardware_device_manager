/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_PIN_HOLDER_TEST_H
#define OHOS_DM_PIN_HOLDER_TEST_H

#include <gtest/gtest.h>
#include <refbase.h>

#include <memory>
#include <cstdint>
#include "mock/mock_ipc_client_proxy.h"
#include "device_manager.h"
#include "single_instance.h"
#include "dm_pin_holder.h"

namespace OHOS {
namespace DistributedHardware {
class DmPinHolderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class DeviceDiscoveryCallbackTest : public DiscoveryCallback {
public:
    DeviceDiscoveryCallbackTest() : DiscoveryCallback() {}
    ~DeviceDiscoveryCallbackTest() {}
    void OnDiscoverySuccess(uint16_t subscribeId) override {}
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override {}
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) override;
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo) override{}
};

class DmInitCallbackTest : public DmInitCallback {
public:
    DmInitCallbackTest() : DmInitCallback() {}
    ~DmInitCallbackTest() override {}
    void OnRemoteDied() override {}
};

class DmPinHolderCallbackTest : public PinHolderCallback {
public:
    DmPinHolderCallbackTest() : PinHolderCallback() {}
    ~DmPinHolderCallbackTest() override {}
    void OnPinHolderCreate(const std::string &deviceId, DmPinType pinType, const std::string &payload) override;
    void OnPinHolderDestroy(DmPinType pinType) override;
    void OnCreateResult(int32_t result) override;
    void OnDestroyResult(int32_t result) override;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DM_PIN_HOLDER_TEST_H
