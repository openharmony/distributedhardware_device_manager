/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "dm_single_instance.h"
#include "device_manager_impl.h"
#include "softbus_error_code.h"
#include "device_manager_notify_mock.h"
#include "device_manager_notify.h"
#include "dm_publish_info.h"
#include "dm_subscribe_info.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<MockIpcClientProxy> ipcClientProxyMock_ = std::make_shared<MockIpcClientProxy>();
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

class DeviceScreenStatusCallbackTest : public DeviceScreenStatusCallback {
public:
    ~DeviceScreenStatusCallbackTest()
    {
    }
    void OnDeviceScreenStatus(const DmDeviceInfo &deviceInfo) override {}
};

class DevTrustChangeCallbackTest : public DevTrustChangeCallback {
public:
    virtual ~DevTrustChangeCallbackTest()
    {
    }
    void OnDeviceTrustChange(const std::string &udid, const std::string &uuid, DmAuthForm authForm) override {}
};

class CandidateRestrictStatusCallbackTest : public CredentialAuthStatusCallback {
public:
    virtual ~CandidateRestrictStatusCallbackTest()
    {
    }
    void OnCredentialAuthStatus(const std::string &deviceList, uint16_t deviceTypeId, int32_t errcode) override {}
};

class GetDeviceProfileInfoListCallbackTest : public GetDeviceProfileInfoListCallback {
public:
    virtual ~GetDeviceProfileInfoListCallbackTest()
    {
    }
    void OnResult(const std::vector<DmDeviceProfileInfo> &deviceProfileInfos, int32_t code) override {}
};

class GetDeviceIconInfoCallbackTest : public GetDeviceIconInfoCallback {
public:
    virtual ~GetDeviceIconInfoCallbackTest()
    {
    }
    void OnResult(const DmDeviceIconInfo &deviceIconInfo, int32_t code) override {}
};

class BindTargetCallbackTest : public BindTargetCallback {
public:
    virtual ~BindTargetCallbackTest()
    {
    }
    void OnBindResult(const PeerTargetId &targetId, int32_t result, int32_t status, std::string content) override {}
};

class LeaveLNNCallbackTest : public LeaveLNNCallback {
public:
    virtual ~LeaveLNNCallbackTest()
    {
    }
    void OnLeaveLNNCallback(const std::string &networkId, int32_t retCode) override {}
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

#endif // OHOS_DEVICE_MANAGER_IMPL_TEST_H
