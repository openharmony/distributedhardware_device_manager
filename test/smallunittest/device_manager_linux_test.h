/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IMPL_TEST_H
#define OHOS_DM_IMPL_TEST_H

#include <gtest/gtest.h>

#include "device_manager.h"
#include "device_manager_callback.h"
#include "device_manager_impl.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class DmInitCallbackTest : public DmInitCallback {
public:
    DmInitCallbackTest() : DmInitCallback() {}
    ~DmInitCallbackTest() {}
    void OnRemoteDied() {}
};

class DmDeviceStateCallbackTest : public DeviceStateCallback {
public:
    DmDeviceStateCallbackTest() : DeviceStateCallback() {}
    ~DmDeviceStateCallbackTest() {}
    void OnDeviceOnline(const DmDeviceInfo &deviceInfo) {}
    void OnDeviceOffline(const DmDeviceInfo &deviceInfo) {}
    void OnDeviceChanged(const DmDeviceInfo &deviceInfo) {}
    void OnDeviceReady(const DmDeviceInfo &deviceInfo) {}
};

class DiscoveryCallbackTest : public DiscoveryCallback {
public:
    DiscoveryCallbackTest() : DiscoveryCallback() {}
    ~DiscoveryCallbackTest() {}
    void OnDiscoverySuccess(uint16_t subscribeId) {};
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) {};
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) {};
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo) {};
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DM_IMPL_TEST_H
