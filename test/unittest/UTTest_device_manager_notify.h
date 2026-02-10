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

#ifndef OHOS_DEVICE_MANAGER_NOTIFY_TEST_H
#define OHOS_DEVICE_MANAGER_NOTIFY_TEST_H

#include <gtest/gtest.h>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <list>

#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "device_manager_callback.h"
#include "dm_single_instance.h"
namespace OHOS {
namespace DistributedHardware {
class DeviceManagerNotifyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class DmInitCallbackTest : public DmInitCallback {
public:
    explicit DmInitCallbackTest(int &count);
    virtual ~DmInitCallbackTest() {}
    void OnRemoteDied() override;
private:
    int *count_ = nullptr;
};

class DeviceStateCallbackTest : public DeviceStateCallback {
public:
    explicit DeviceStateCallbackTest(int &count);
    virtual ~DeviceStateCallbackTest() {}
    void OnDeviceOnline(const DmDeviceInfo &deviceInfo) override;
    void OnDeviceReady(const DmDeviceInfo &deviceInfo) override;
    void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override;
    void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override;
private:
    int *count_ = nullptr;
};

class DiscoveryCallbackTest : public DiscoveryCallback {
public:
    explicit DiscoveryCallbackTest(int &count);
    virtual ~DiscoveryCallbackTest() {}
    void OnDiscoverySuccess(uint16_t subscribeId) override;
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override;
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) override;
private:
    int *count_ = nullptr;
};

class PublishCallbackTest : public PublishCallback {
public:
    explicit PublishCallbackTest(int &count);
    virtual  ~PublishCallbackTest() {}
    void OnPublishResult(int32_t publishId, int32_t failedReason) override;
private:
    int *count_ = nullptr;
};

class AuthenticateCallbackTest : public AuthenticateCallback {
public:
    explicit AuthenticateCallbackTest(int &count);
    virtual ~AuthenticateCallbackTest() {}
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
                              int32_t reason) override;
private:
    int *count_ = nullptr;
};

class DeviceManagerFaCallbackTest : public DeviceManagerUiCallback {
public:
    explicit DeviceManagerFaCallbackTest(int &count);
    virtual ~DeviceManagerFaCallbackTest() {}
    void OnCall(const std::string &paramJson) override;
private:
    int *count_ = nullptr;
};

class CredentialCallbackTest : public CredentialCallback {
public:
    void OnCredentialResult(int32_t &action, const std::string &credentialResult) override {}
};

class DeviceStatusCallbackTest : public DeviceStatusCallback {
public:
    void OnDeviceOnline(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceOffline(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceChanged(const DmDeviceBasicInfo &deviceBasicInfo) override {}
    void OnDeviceReady(const DmDeviceBasicInfo &deviceBasicInfo) override {}
};

class PinHolderCallbackTest : public PinHolderCallback {
public:
    void OnPinHolderCreate(const std::string &deviceId, DmPinType pinType, const std::string &payload) {}
    void OnPinHolderDestroy(DmPinType pinType, const std::string &payload) {}
    void OnCreateResult(int32_t result) {}
    void OnDestroyResult(int32_t result) {}
    void OnPinHolderEvent(DmPinHolderEvent event, int32_t result, const std::string &content) {}
};

class BindTargetCallbackTest : public BindTargetCallback {
public:
    void OnBindResult(const PeerTargetId &targetId, int32_t result, int32_t status, std::string content) override {}
};

class AuthCodeInvalidCallbackTest : public AuthCodeInvalidCallback {
public:
    virtual ~AuthCodeInvalidCallbackTest()
    {
    }
    void OnAuthCodeInvalid() override {};
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DEVICE_MANAGER_NOTIFY_TEST_H
