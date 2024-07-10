/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_IMPL_TEST_H
#define OHOS_DEVICE_MANAGER_IMPL_TEST_H

#include <gtest/gtest.h>
#include <refbase.h>

#include <memory>
#include <cstdint>
#include "mock/mock_ipc_client_proxy.h"
#include "device_manager.h"
#include "single_instance.h"
#include "device_manager_impl.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class DeviceDiscoveryCallbackTest : public DiscoveryCallback {
public:
    DeviceDiscoveryCallbackTest() : DiscoveryCallback() {}
    ~DeviceDiscoveryCallbackTest() {}
    void OnDiscoverySuccess(uint16_t subscribeId) override {}
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override {}
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) override {}
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo) override{}
};

class DevicePublishCallbackTest : public PublishCallback {
public:
    DevicePublishCallbackTest() : PublishCallback() {}
    virtual ~DevicePublishCallbackTest() {}
    void OnPublishResult(int32_t publishId, int32_t failedReason) override {}
};

class DmInitCallbackTest : public DmInitCallback {
public:
    DmInitCallbackTest() : DmInitCallback() {}
    virtual ~DmInitCallbackTest() {}
    void OnRemoteDied() override {}
};

class DeviceStateCallbackTest : public DeviceStateCallback {
public:
    DeviceStateCallbackTest() : DeviceStateCallback() {}
    virtual ~DeviceStateCallbackTest() {}
    void OnDeviceOnline(const DmDeviceInfo &deviceInfo) override {}
    void OnDeviceReady(const DmDeviceInfo &deviceInfo) override {}
    void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override {}
    void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override {}
};

class DeviceManagerFaCallbackTest : public DeviceManagerUiCallback {
public:
    DeviceManagerFaCallbackTest() : DeviceManagerUiCallback() {}
    virtual ~DeviceManagerFaCallbackTest() {}
    void OnCall(const std::string &paramJson) override {}
};

class CredentialCallbackTest : public CredentialCallback {
public:
    virtual ~CredentialCallbackTest() {}
    void OnCredentialResult(int32_t &action, const std::string &credentialResult) override {}
};

class DeviceStatusCallbackTest : public DeviceStatusCallback {
public:
    ~DeviceStatusCallbackTest() {}
    void OnDeviceOnline(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceOffline(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceChanged(const DmDeviceBasicInfo &deviceBasicInfo)  override {}
    void OnDeviceReady(const DmDeviceBasicInfo &deviceBasicInfo) override {}
};

class AuthenticateCallbackTest : public AuthenticateCallback {
public:
    ~AuthenticateCallbackTest()
    {
    }
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
                      int32_t reason) override {}
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DEVICE_MANAGER_IMPL_TEST_H
