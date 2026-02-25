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

#include "UTTest_device_manager_notify.h"

#include <unistd.h>

#include "ipc_def.h"
#include "ipc_remote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "device_manager_notify.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_error_type.h"
#include "ipc_client_manager.h"
#include "ipc_rsp.h"
#include "ipc_set_useroperation_req.h"


namespace OHOS {
namespace DistributedHardware {

HWTEST_F(DeviceManagerNotifyTest, RegisterServicePublishCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    DeviceManagerNotify::GetInstance().servicePublishCallbacks_.clear();

    size_t initialSize = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();
    DeviceManagerNotify::GetInstance().RegisterServicePublishCallback(pkgName, serviceId, callback);
    size_t finalSize = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();
    ASSERT_EQ(initialSize + 1, finalSize);
    auto key = std::make_pair(pkgName, serviceId);
    auto it = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.find(key);
    ASSERT_NE(it, DeviceManagerNotify::GetInstance().servicePublishCallbacks_.end());
    ASSERT_EQ(it->second, callback);
}

/**
 * @tc.name: OnCredentialResult4
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, OnCredentialResult4, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test4";
    int32_t action = 1;
    std::string credentialResult = "failed";
    DeviceManagerNotify::GetInstance().OnCredentialResult(pkgName, action, credentialResult);
    int count = DeviceManagerNotify::GetInstance().credentialCallback_.count(pkgName);
    EXPECT_EQ(count, 0);
}

/**
 * @tc.name: UnRegisterDeviceStatusCallback1
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceStatusCallback1, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test4";
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStatusCallback(pkgName);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().deviceStatusCallback_.empty(), true);
}

/**
 * @tc.name: UnRegisterDeviceStatusCallback2
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceStatusCallback2, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStatusCallback(pkgName);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().deviceStatusCallback_.empty(), true);
}

/**
 * @tc.name: RegisterDeviceStatusCallback1
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceStatusCallback1, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test4";
    std::shared_ptr<DeviceStatusCallback> callback = std::make_shared<DeviceStatusCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterDeviceStatusCallback(pkgName, callback);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().deviceStatusCallback_.empty(), false);
}

/**
 * @tc.name: RegisterDeviceStatusCallback2
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceStatusCallback2, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::shared_ptr<DeviceStatusCallback> callback = std::make_shared<DeviceStatusCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterDeviceStatusCallback(pkgName, callback);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().deviceStatusCallback_.empty(), false);
    DeviceManagerNotify::GetInstance().deviceStatusCallback_.clear();
}

/**
 * @tc.name: OnDeviceOnline1
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOnline1, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test4";
    DmDeviceBasicInfo deviceBasicInfo;
    DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, deviceBasicInfo);
    auto ptr = DeviceManagerNotify::GetInstance().deviceStatusCallback_[pkgName];
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.name: OnDeviceOnline2
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOnline2, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DmDeviceBasicInfo deviceBasicInfo;
    DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, deviceBasicInfo);
    auto ptr = DeviceManagerNotify::GetInstance().deviceStatusCallback_[pkgName];
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.name: OnDeviceOffline1
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOffline1, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test4";
    DmDeviceBasicInfo deviceBasicInfo;
    DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, deviceBasicInfo);
    auto ptr = DeviceManagerNotify::GetInstance().deviceStatusCallback_[pkgName];
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.name: OnDeviceOffline2
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOffline2, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DmDeviceBasicInfo deviceBasicInfo;
    DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, deviceBasicInfo);
    auto ptr = DeviceManagerNotify::GetInstance().deviceStatusCallback_[pkgName];
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.name: OnDeviceReady1
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceReady1, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test4";
    DmDeviceBasicInfo deviceBasicInfo;
    DeviceManagerNotify::GetInstance().OnDeviceReady(pkgName, deviceBasicInfo);
    auto ptr = DeviceManagerNotify::GetInstance().deviceStatusCallback_[pkgName];
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.name: OnDeviceReady2
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceReady2, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DmDeviceBasicInfo deviceBasicInfo;
    DeviceManagerNotify::GetInstance().OnDeviceReady(pkgName, deviceBasicInfo);
    auto ptr = DeviceManagerNotify::GetInstance().deviceStatusCallback_[pkgName];
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.name: OnDeviceFound6
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceFound6, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test4";
    uint16_t subscribeId = 0;
    DmDeviceBasicInfo deviceBasicInfo;
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, deviceBasicInfo);
    auto map = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName];
    EXPECT_EQ(map.empty(), true);
}

/**
 * @tc.name: OnDeviceFound7
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceFound7, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    uint16_t subscribeId = 0;
    DmDeviceBasicInfo deviceBasicInfo;
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, deviceBasicInfo);
    auto map = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName];
    EXPECT_EQ(map.empty(), false);
}

/**
 * @tc.name: RegisterCredentialCallback_301
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterCredentialCallback_301, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::shared_ptr<CredentialCallback> callback = std::make_shared<CredentialCallbackTest>();
    DeviceManagerNotify::GetInstance().credentialCallback_.clear();
    DeviceManagerNotify::GetInstance().RegisterCredentialCallback(pkgName, callback);
    DeviceManagerNotify::GetInstance().UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().credentialCallback_.empty(), true);
}

/**
 * @tc.name: RegisterCredentialCallback_302
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterCredentialCallback_302, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::shared_ptr<CredentialCallback> callback = nullptr;
    DeviceManagerNotify::GetInstance().credentialCallback_.clear();
    DeviceManagerNotify::GetInstance().RegisterCredentialCallback(pkgName, callback);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().credentialCallback_.empty(), true);
}

/**
 * @tc.name: RegisterCredentialCallback_303
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterCredentialCallback_303, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::shared_ptr<CredentialCallback> callback = nullptr;
    DeviceManagerNotify::GetInstance().credentialCallback_.clear();
    DeviceManagerNotify::GetInstance().RegisterCredentialCallback(pkgName, callback);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().credentialCallback_.empty(), true);
}

/**
 * @tc.name: RegisterPinHolderCallback_301
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterPinHolderCallback_301, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::shared_ptr<PinHolderCallback> callback = nullptr;
    DeviceManagerNotify::GetInstance().pinHolderCallback_.clear();
    DeviceManagerNotify::GetInstance().RegisterPinHolderCallback(pkgName, callback);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().pinHolderCallback_.empty(), true);
}

/**
 * @tc.name: RegisterPinHolderCallback_302
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterPinHolderCallback_302, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::shared_ptr<PinHolderCallback> callback = nullptr;
    DeviceManagerNotify::GetInstance().pinHolderCallback_.clear();
    DeviceManagerNotify::GetInstance().RegisterPinHolderCallback(pkgName, callback);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().pinHolderCallback_.empty(), true);
}

/**
 * @tc.name: RegisterPinHolderCallback_303
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterPinHolderCallback_303, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::shared_ptr<PinHolderCallback> callback = std::make_shared<PinHolderCallbackTest>();
    DeviceManagerNotify::GetInstance().pinHolderCallback_.clear();
    DeviceManagerNotify::GetInstance().RegisterPinHolderCallback(pkgName, callback);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().pinHolderCallback_.empty(), true);
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterBindCallback_001, testing::ext::TestSize.Level0)
{
    std::string emptyPkgName = "testTargetId";
    PeerTargetId targetId;
    DeviceManagerNotify::GetInstance().UnRegisterBindCallback(emptyPkgName, targetId);
    ASSERT_TRUE(DeviceManagerNotify::GetInstance().bindCallback_.empty());
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterBindCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    PeerTargetId targetId;
    std::shared_ptr<BindTargetCallback> callback = std::make_shared<BindTargetCallbackTest>();
    DeviceManagerNotify::GetInstance().bindCallback_[pkgName][targetId] = callback;
    std::string nonExistentPkgName = "com.ohos.nonexistent";
    DeviceManagerNotify::GetInstance().UnRegisterBindCallback(nonExistentPkgName, targetId);
    ASSERT_EQ(DeviceManagerNotify::GetInstance().bindCallback_[pkgName][targetId], callback);
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterBindCallback_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    PeerTargetId targetId;
    std::shared_ptr<BindTargetCallback> callback = std::make_shared<BindTargetCallbackTest>();
    DeviceManagerNotify::GetInstance().bindCallback_[pkgName][targetId] = callback;
    DeviceManagerNotify::GetInstance().UnRegisterBindCallback(pkgName, targetId);
    ASSERT_TRUE(DeviceManagerNotify::GetInstance().bindCallback_.empty());
}

HWTEST_F(DeviceManagerNotifyTest, RegisterAuthCodeInvalidCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::shared_ptr<AuthCodeInvalidCallback> cb = nullptr;
    DeviceManagerNotify::GetInstance().RegisterAuthCodeInvalidCallback(pkgName, cb);
    EXPECT_TRUE(pkgName.empty());
}

HWTEST_F(DeviceManagerNotifyTest, RegisterAuthCodeInvalidCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::shared_ptr<AuthCodeInvalidCallback> cb = std::make_shared<AuthCodeInvalidCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterAuthCodeInvalidCallback(pkgName, cb);
    EXPECT_TRUE(pkgName.empty());
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterAuthCodeInvalidCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    DeviceManagerNotify::GetInstance().UnRegisterAuthCodeInvalidCallback(pkgName);
    EXPECT_TRUE(pkgName.empty());
}

HWTEST_F(DeviceManagerNotifyTest, OnAuthCodeInvalid_001, testing::ext::TestSize.Level1)
{
    DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.clear();
    std::string pkgName = "ohos.devicemanager.test";
    auto cb = std::make_shared<AuthCodeInvalidCallbackTest>();
    DeviceManagerNotify::GetInstance().authCodeInvalidCallback_[pkgName] = cb;
    DeviceManagerNotify::GetInstance().OnAuthCodeInvalid(pkgName);
    auto it = DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.find(pkgName);
    ASSERT_NE(it, DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.end());
}

HWTEST_F(DeviceManagerNotifyTest, OnAuthCodeInvalid_002, testing::ext::TestSize.Level1)
{
    DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.clear();
    std::string pkgName;
    DeviceManagerNotify::GetInstance().OnAuthCodeInvalid(pkgName);
    EXPECT_TRUE(pkgName.empty());
}

HWTEST_F(DeviceManagerNotifyTest, OnAuthCodeInvalid_003, testing::ext::TestSize.Level1)
{
    DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.clear();
    std::string pkgName = "ohos.devicemanager.test";
    DeviceManagerNotify::GetInstance().OnAuthCodeInvalid(pkgName);
    auto it = DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.find(pkgName);
    ASSERT_EQ(it, DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.end());
}

HWTEST_F(DeviceManagerNotifyTest, OnAuthCodeInvalid_004, testing::ext::TestSize.Level1)
{
    DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.clear();
    std::string pkgName = "ohos.devicemanager.test";
    DeviceManagerNotify::GetInstance().authCodeInvalidCallback_[pkgName] = nullptr;
    DeviceManagerNotify::GetInstance().OnAuthCodeInvalid(pkgName);
    auto it = DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.find(pkgName);
    ASSERT_NE(it, DeviceManagerNotify::GetInstance().authCodeInvalidCallback_.end());
    ASSERT_EQ(it->second, nullptr);
}

DmInitCallbackTest::DmInitCallbackTest(int &count) : DmInitCallback()
{
    count_ = &count;
}
void DmInitCallbackTest::OnRemoteDied()
{
    *count_ = *count_ + 1;
}

DeviceStateCallbackTest::DeviceStateCallbackTest(int &count) : DeviceStateCallback()
{
    count_ = &count;
}

void DeviceStateCallbackTest::OnDeviceOnline(const DmDeviceInfo &deviceInfo)
{
    *count_ = *count_ + 1;
}

void DeviceStateCallbackTest::OnDeviceReady(const DmDeviceInfo &deviceInfo)
{
    *count_ = *count_ + 1;
}

void DeviceStateCallbackTest::OnDeviceOffline(const DmDeviceInfo &deviceInfo)
{
    *count_ = *count_ + 1;
}

void DeviceStateCallbackTest::OnDeviceChanged(const DmDeviceInfo &deviceInfo)
{
    *count_ = *count_ + 1;
}

DiscoveryCallbackTest::DiscoveryCallbackTest(int &count) : DiscoveryCallback()
{
    count_ = &count;
}

void DiscoveryCallbackTest::OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo)
{
    *count_ = *count_ + 1;
}

void DiscoveryCallbackTest::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    *count_ = *count_ + 1;
}

void DiscoveryCallbackTest::OnDiscoverySuccess(uint16_t subscribeId)
{
    *count_ = *count_ + 1;
}

PublishCallbackTest::PublishCallbackTest(int &count) : PublishCallback()
{
    count_ = &count;
}

void PublishCallbackTest::OnPublishResult(int32_t publishId, int32_t failedReason)
{
    *count_ = *count_ + 1;
}

AuthenticateCallbackTest::AuthenticateCallbackTest(int &count) : AuthenticateCallback()
{
    count_ = &count;
}

void AuthenticateCallbackTest::OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status,
    int32_t reason)
{
    *count_ = *count_ + 1;
}

DeviceManagerFaCallbackTest::DeviceManagerFaCallbackTest(int &count) : DeviceManagerUiCallback()
{
    count_ = &count;
}

void DeviceManagerFaCallbackTest::OnCall(const std::string &paramJson)
{
    *count_ = *count_ + 1;
}
    

HWTEST_F(DeviceManagerNotifyTest, RegisterServicePublishCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1002;
    std::shared_ptr<ServicePublishCallback> callback = nullptr;
    DeviceManagerNotify::GetInstance().servicePublishCallbacks_.clear();

    size_t initialSize = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();
    DeviceManagerNotify::GetInstance().RegisterServicePublishCallback(pkgName, serviceId, callback);
    size_t finalSize = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();

    ASSERT_EQ(initialSize, finalSize);
}

HWTEST_F(DeviceManagerNotifyTest, RegisterServicePublishCallback_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int64_t serviceId = 1003;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    DeviceManagerNotify::GetInstance().servicePublishCallbacks_.clear();

    size_t initialSize = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();
    DeviceManagerNotify::GetInstance().RegisterServicePublishCallback(pkgName, serviceId, callback);
    size_t finalSize = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();

    ASSERT_EQ(initialSize + 1, finalSize);
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterServicePublishCallback_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 2001;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterServicePublishCallback(pkgName, serviceId, callback);

    size_t sizeAfterRegister = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();
    auto key = std::make_pair(pkgName, serviceId);
    auto it = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.find(key);
    ASSERT_NE(it, DeviceManagerNotify::GetInstance().servicePublishCallbacks_.end());

    DeviceManagerNotify::GetInstance().UnRegisterServicePublishCallback(pkgName, serviceId);
    size_t finalSize = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();
    ASSERT_EQ(sizeAfterRegister - 1, finalSize);

    it = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.find(key);
    ASSERT_EQ(it, DeviceManagerNotify::GetInstance().servicePublishCallbacks_.end());
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterServicePublishCallback_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 2002;
    DeviceManagerNotify::GetInstance().servicePublishCallbacks_.clear();

    size_t initialSize = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();
    DeviceManagerNotify::GetInstance().UnRegisterServicePublishCallback(pkgName, serviceId);
    size_t finalSize = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.size();

    ASSERT_EQ(initialSize, finalSize);
}

HWTEST_F(DeviceManagerNotifyTest, OnServicePublishResult_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 3001;
    int32_t publishResult = 0; // DM_OK
    int count = 0;

    class TestServicePublishCallback : public ServicePublishCallback {
    public:
        explicit TestServicePublishCallback(int &count) : count_(count) {}
        void OnServicePublishResult(int64_t serviceId, int32_t reason) override {
            count_++;
        }
    private:
        int &count_;
    };

    std::shared_ptr<TestServicePublishCallback> callback =
        std::make_shared<TestServicePublishCallback>(count);
    DeviceManagerNotify::GetInstance().RegisterServicePublishCallback(pkgName, serviceId, callback);

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().OnServicePublishResult(pkgName, serviceId, publishResult));
}

HWTEST_F(DeviceManagerNotifyTest, OnServicePublishResult_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 3002;
    int32_t publishResult = 0;

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().OnServicePublishResult(pkgName, serviceId, publishResult));
}

HWTEST_F(DeviceManagerNotifyTest, OnServicePublishResult_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 3003;
    int32_t publishResult = -1; // Failed

    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterServicePublishCallback(pkgName, serviceId, callback);

    auto key = std::make_pair(pkgName, serviceId);
    auto it = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.find(key);
    ASSERT_NE(it, DeviceManagerNotify::GetInstance().servicePublishCallbacks_.end());

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().OnServicePublishResult(pkgName, serviceId, publishResult));

    it = DeviceManagerNotify::GetInstance().servicePublishCallbacks_.find(key);
    ASSERT_EQ(it, DeviceManagerNotify::GetInstance().servicePublishCallbacks_.end());
}

HWTEST_F(DeviceManagerNotifyTest, RegisterServiceDiscoveryCallback_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string serviceType = "testService";
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.clear();

    size_t initialSize = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.size();
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(pkgName, serviceType, callback);
    size_t finalSize = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.size();

    ASSERT_EQ(initialSize + 1, finalSize);
    auto key = std::make_pair(pkgName, serviceType);
    auto it = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.find(key);
    ASSERT_NE(it, DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.end());
    ASSERT_EQ(it->second, callback);
}

HWTEST_F(DeviceManagerNotifyTest, RegisterServiceDiscoveryCallback_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string serviceType = "testService";
    std::shared_ptr<ServiceDiscoveryCallback> callback = nullptr;
    DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.clear();

    size_t initialSize = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.size();
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(pkgName, serviceType, callback);
    size_t finalSize = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.size();

    ASSERT_EQ(initialSize, finalSize);
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterServiceDiscoveryCallback_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string serviceType = "testService";
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(pkgName, serviceType, callback);

    size_t sizeAfterRegister = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.size();
    auto key = std::make_pair(pkgName, serviceType);
    auto it = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.find(key);
    ASSERT_NE(it, DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.end());

    DeviceManagerNotify::GetInstance().UnRegisterServiceDiscoveryCallback(pkgName, serviceType);
    size_t finalSize = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.size();
    ASSERT_EQ(sizeAfterRegister - 1, finalSize);

    it = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.find(key);
    ASSERT_EQ(it, DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.end());
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterServiceDiscoveryCallback_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string serviceType = "testService";
    DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.clear();

    size_t initialSize = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.size();
    DeviceManagerNotify::GetInstance().UnRegisterServiceDiscoveryCallback(pkgName, serviceType);
    size_t finalSize = DeviceManagerNotify::GetInstance().discoveryServiceCallbacks_.size();

    ASSERT_EQ(initialSize, finalSize);
}

HWTEST_F(DeviceManagerNotifyTest, OnServiceDiscoveryResult_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string serviceType = "testService";
    int32_t resReason = 0;
    int count = 0;

    class TestServiceDiscoveryCallback : public ServiceDiscoveryCallback {
    public:
        explicit TestServiceDiscoveryCallback(int &count) : count_(count) {}
        void OnServiceFound(const DmServiceInfo &service) override {}
        void OnServiceDiscoveryResult(int32_t resReason) override {
            count_++;
        }
    private:
        int &count_;
    };

    std::shared_ptr<TestServiceDiscoveryCallback> callback =
        std::make_shared<TestServiceDiscoveryCallback>(count);
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(pkgName, serviceType, callback);

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().OnServiceDiscoveryResult(pkgName, serviceType, resReason));
}

HWTEST_F(DeviceManagerNotifyTest, OnServiceDiscoveryResult_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string serviceType = "testService";
    int32_t resReason = 0;

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().OnServiceDiscoveryResult(pkgName, serviceType, resReason));
}

HWTEST_F(DeviceManagerNotifyTest, OnServiceFound_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string serviceType = "testService";
    int count = 0;

    class TestServiceDiscoveryCallback : public ServiceDiscoveryCallback {
    public:
        explicit TestServiceDiscoveryCallback(int &count) : count_(count) {}
        void OnServiceFound(const DmServiceInfo &service) override {
            count_++;
        }
        void OnServiceDiscoveryResult(int32_t resReason) override {}
    private:
        int &count_;
    };

    std::shared_ptr<TestServiceDiscoveryCallback> callback =
        std::make_shared<TestServiceDiscoveryCallback>(count);
    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(pkgName, serviceType, callback);

    DmServiceInfo dmServiceInfo;
    dmServiceInfo.serviceType = serviceType;
    dmServiceInfo.serviceId = 1001;
    dmServiceInfo.userId = 100;

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().OnServiceFound(pkgName, dmServiceInfo));
}

HWTEST_F(DeviceManagerNotifyTest, OnServiceFound_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DmServiceInfo dmServiceInfo;
    dmServiceInfo.serviceType = "testService";
    dmServiceInfo.serviceId = 1002;
    dmServiceInfo.userId = 100;

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().OnServiceFound(pkgName, dmServiceInfo));
}

HWTEST_F(DeviceManagerNotifyTest, RegisterSyncServiceInfoCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "network123";
    int64_t serviceId = 4001;
    int count = 0;

    std::shared_ptr<SyncServiceInfoCallback> callback =
        std::make_shared<SyncServiceInfoCallbackTest>(&count);
    DeviceManagerNotify::GetInstance().syncServiceInfoCallback_.clear();

    DeviceManagerNotify::GetInstance().RegisterSyncServiceInfoCallback(
        pkgName, localUserId, networkId, callback, serviceId);

    ASSERT_GT(DeviceManagerNotify::GetInstance().syncServiceInfoCallback_.size(), 0u);
    ASSERT_GE(DeviceManagerNotify::GetInstance().syncServiceInfoCallback_[pkgName].size(), 1u);
}

HWTEST_F(DeviceManagerNotifyTest, RegisterSyncServiceInfoCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t localUserId = 100;
    std::string networkId = "network123";
    int64_t serviceId = 4002;
    int count = 0;

    std::shared_ptr<SyncServiceInfoCallback> callback =
        std::make_shared<SyncServiceInfoCallbackTest>(&count);
    DeviceManagerNotify::GetInstance().syncServiceInfoCallback_.clear();

    size_t initialSize = DeviceManagerNotify::GetInstance().syncServiceInfoCallback_.size();
    DeviceManagerNotify::GetInstance().RegisterSyncServiceInfoCallback(
        pkgName, localUserId, networkId, callback, serviceId);
    size_t finalSize = DeviceManagerNotify::GetInstance().syncServiceInfoCallback_.size();

    ASSERT_EQ(initialSize, finalSize);
}

HWTEST_F(DeviceManagerNotifyTest, RegisterSyncServiceInfoCallback_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = -1;
    std::string networkId = "network123";
    int64_t serviceId = 4003;
    int count = 0;

    std::shared_ptr<SyncServiceInfoCallback> callback =
        std::make_shared<SyncServiceInfoCallbackTest>(&count);
    DeviceManagerNotify::GetInstance().syncServiceInfoCallback_.clear();

    size_t initialSize = DeviceManagerNotify::GetInstance().syncServiceInfoCallback_.size();
    DeviceManagerNotify::GetInstance().RegisterSyncServiceInfoCallback(
        pkgName, localUserId, networkId, callback, serviceId);
    size_t finalSize = DeviceManagerNotify::GetInstance().syncServiceInfoCallback_.size();

    ASSERT_EQ(initialSize, finalSize);
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterSyncServiceInfoCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "network123";
    int64_t serviceId = 5001;
    int count = 0;

    std::shared_ptr<SyncServiceInfoCallback> callback =
        std::make_shared<SyncServiceInfoCallbackTest>(&count);
    DeviceManagerNotify::GetInstance().RegisterSyncServiceInfoCallback(
        pkgName, localUserId, networkId, callback, serviceId);

    ASSERT_GT(DeviceManagerNotify::GetInstance().syncServiceInfoCallback_[pkgName].size(), 0u);

    DeviceManagerNotify::GetInstance().UnRegisterSyncServiceInfoCallback(
        pkgName, localUserId, networkId, serviceId);

    ASSERT_EQ(DeviceManagerNotify::GetInstance().syncServiceInfoCallback_[pkgName].size(), 0u);
}

HWTEST_F(DeviceManagerNotifyTest, UnRegisterSyncServiceInfoCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "network123";
    int64_t serviceId = 5002;

    DeviceManagerNotify::GetInstance().syncServiceInfoCallback_.clear();

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().UnRegisterSyncServiceInfoCallback(
            pkgName, localUserId, networkId, serviceId));
}

HWTEST_F(DeviceManagerNotifyTest, OnSyncServiceInfoResult_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "network123";
    int64_t serviceId = 6001;
    int32_t result = 0;
    std::string content = "test content";
    int count = 0;

    std::shared_ptr<SyncServiceInfoCallback> callback =
        std::make_shared<SyncServiceInfoCallbackTest>(&count);
    DeviceManagerNotify::GetInstance().RegisterSyncServiceInfoCallback(
        pkgName, localUserId, networkId, callback, serviceId);

    ASSERT_GT(DeviceManagerNotify::GetInstance().syncServiceInfoCallback_[pkgName].size(), 0u);

    ServiceSyncInfo serviceSyncInfo;
    serviceSyncInfo.pkgName = pkgName;
    serviceSyncInfo.localUserId = localUserId;
    serviceSyncInfo.networkId = networkId;
    serviceSyncInfo.serviceId = serviceId;

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().OnSyncServiceInfoResult(serviceSyncInfo, result, content));

    ASSERT_EQ(DeviceManagerNotify::GetInstance().syncServiceInfoCallback_[pkgName].size(), 0u);
}

HWTEST_F(DeviceManagerNotifyTest, OnSyncServiceInfoResult_002, testing::ext::TestSize.Level0)
{
    ServiceSyncInfo serviceSyncInfo;
    serviceSyncInfo.pkgName = "com.ohos.test";
    serviceSyncInfo.localUserId = 100;
    serviceSyncInfo.networkId = "network123";
    serviceSyncInfo.serviceId = 6002;
    int32_t result = 0;
    std::string content = "test content";

    ASSERT_NO_FATAL_FAILURE(
        DeviceManagerNotify::GetInstance().OnSyncServiceInfoResult(serviceSyncInfo, result, content));
}

HWTEST_F(DeviceManagerNotifyTest, GetServiceCallBack_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 7001;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();

    DeviceManagerNotify::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);

    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> serviceCallbackMap;
    DeviceManagerNotify::GetInstance().GetServiceCallBack(serviceCallbackMap);

    ASSERT_GT(serviceCallbackMap.size(), 0u);
    ASSERT_GT(serviceCallbackMap[DmCommonNotifyEvent::REG_SERVICE_STATE].size(), 0u);

    auto it = serviceCallbackMap[DmCommonNotifyEvent::REG_SERVICE_STATE].find(std::make_pair(pkgName, serviceId));
    ASSERT_NE(it, serviceCallbackMap[DmCommonNotifyEvent::REG_SERVICE_STATE].end());
}

HWTEST_F(DeviceManagerNotifyTest, GetServiceCallBack_002, testing::ext::TestSize.Level0)
{
    DeviceManagerNotify::GetInstance().serviceStateCallback_.clear();

    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> serviceCallbackMap;
    DeviceManagerNotify::GetInstance().GetServiceCallBack(serviceCallbackMap);

    ASSERT_EQ(serviceCallbackMap.size(), 0u);
}
} // namespace DistributedHardware
} // namespace OHOS
