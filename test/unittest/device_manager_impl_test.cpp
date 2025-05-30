/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "device_manager_impl_test.h"

#include <unistd.h>

#include "dm_constants.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerImplTest::SetUp()
{
}

void DeviceManagerImplTest::TearDown()
{
}

void DeviceManagerImplTest::SetUpTestCase()
{
}

void DeviceManagerImplTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: InitDeviceManager
 * @tc.desc: 1. set packName not null
 *              set callback nullptr
 *           2. MOCK DeviceManager InitDeviceManager return ERR_DM_INPUT_PARA_INVALID
 *           3. call DeviceManager::InitDeviceManager with parameter
 *           4. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set callback nullptr
    // 3. call DeviceManager::InitDeviceManager with parameter
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, nullptr);
    // 4. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, AuthenticateDevice1, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    int32_t authType = 0;
    DmDeviceInfo dmDeviceInfo;
    std::string extra = "";
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, AuthenticateDevice2, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    int32_t authType = 0;
    DmDeviceInfo dmDeviceInfo;
    std::string extra = "";
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    ASSERT_EQ(ret, ERR_DM_IPC_RESPOND_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, AuthenticateDevice3, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    int32_t authType = 0;
    DmDeviceInfo dmDeviceInfo;
    std::string extra = "";
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery1, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    DmSubscribeInfo subscribeInfo;
    std::string extra = "";
    std::shared_ptr<DiscoveryCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery2, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    DmSubscribeInfo subscribeInfo;
    std::string extra = "";
    test_callback_ = std::make_shared<DeviceDiscoveryCallback>();
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, test_callback_);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery3, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    DmSubscribeInfo subscribeInfo;
    std::string extra = "";
    test_callback_ = std::make_shared<DeviceDiscoveryCallback>();
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, test_callback_);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery1, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    DmPublishInfo publishInfo;
    std::shared_ptr<PublishCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery2, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    DmPublishInfo publishInfo;
    testPublishCallback_ = std::make_shared<DevicePublishCallback>();
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, testPublishCallback_);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery3, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    DmPublishInfo publishInfo;
    testPublishCallback_ = std::make_shared<DevicePublishCallback>();
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, testPublishCallback_);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}
} // namespace

void DeviceDiscoveryCallback::OnDiscoverySuccess(uint16_t subscribeId)
{
    (void)subscribeId;
}

void DeviceDiscoveryCallback::OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason)
{
    (void)subscribeId;
    (void)failedReason;
}

void DeviceDiscoveryCallback::OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo)
{
    (void)subscribeId;
    (void)deviceInfo;
}
void DevicePublishCallback::OnPublishResult(int32_t publishId, int32_t failedReason)
{
    (void)publishId;
    (void)failedReason;
}
} // namespace DistributedHardware
} // namespace OHOS
