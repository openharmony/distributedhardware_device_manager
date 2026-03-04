/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "UTTest_device_manager_service_listener.h"

#include <cstring>
#include <memory>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_publish_result_req.h"
#include "device_manager_service_notify.h"
#include "app_manager.h"
#include "ipc_service_publish_result_req.h"
#include "ipc_server_listener_mock.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceListenerTest::SetUp()
{
}

void DeviceManagerServiceListenerTest::TearDown()
{
}

void DeviceManagerServiceListenerTest::SetUpTestCase()
{
    DmSoftbusCache::dmSoftbusCache = softbusCacheMock_;
    DmCrypto::dmCrypto = cryptoMock_;
    DmIpcServerListener::dmIpcServerListener = ipcServerListenerMock_;
    DmKVAdapterManager::dmKVAdapterManager = kVAdapterManagerMock_;
    DmAppManager::dmAppManager = appManagerMock_;
    DmDeviceManagerService::dmDeviceManagerService = deviceManagerServiceMock_;
}

void DeviceManagerServiceListenerTest::TearDownTestCase()
{
    DmSoftbusCache::dmSoftbusCache = nullptr;
    DmCrypto::dmCrypto = nullptr;
    DmIpcServerListener::dmIpcServerListener = nullptr;
    DmKVAdapterManager::dmKVAdapterManager = nullptr;
    DmAppManager::dmAppManager = nullptr;
}

namespace {
/**
 * @tc.name: OnDeviceStateChange_001
 * @tc.desc: OnDeviceStateChange, construct a dummy listener, pass in pkgName, use the constructed listener to get
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceStateChange_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    DmDeviceState state = DEVICE_STATE_ONLINE;
    DmDeviceInfo info = {
        .deviceId = "asdad",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    listener_->OnDeviceStateChange(processInfo, state, info, true);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnDeviceStateChange_002
 * @tc.desc: OnDeviceStateChange, construct a dummy listener, pass in pkgName, use the constructed listener to get
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceStateChange_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "ohos.distributedhardware.devicemanager";
    processInfo.userId = 100;
    DmDeviceState state = DEVICE_STATE_OFFLINE;
    DmDeviceInfo info = {
        .deviceId = "asdad",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    listener_->OnDeviceStateChange(processInfo, state, info, true);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnAuthResult_001
 * @tc.desc:OnAuthResult, construct a dummy listener, pass in pkgName, use the constructed listener to get pkgName
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnAuthResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string deviceId = "dkdkd";
    std::string token = "kdkddk";
    int32_t status = 3;
    int32_t reason = 2006;
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    listener_->OnAuthResult(processInfo, deviceId, token, status, reason);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnAuthResult_002
 * @tc.desc:OnAuthResult, construct a dummy listener, pass in pkgName, use the constructed listener to get pkgName
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnAuthResult_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string deviceId = "dkdkd";
    std::string token = "kdkddk";
    int32_t status = 8;
    int32_t reason = 2006;
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    listener_->OnAuthResult(processInfo, deviceId, token, status, reason);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnAuthResult_003
 * @tc.desc:OnAuthResult, construct a dummy listener, pass in pkgName, use the constructed listener to get pkgName
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnAuthResult_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string deviceId = "dkdkd";
    std::string token = "kdkddk";
    int32_t status = -1;
    int32_t reason = 2006;
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    listener_->OnAuthResult(processInfo, deviceId, token, status, reason);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnDeviceFound_001
 * @tc.desc: OnDeviceFound,construct a dummy listener, pass in pkgName, subscribeId, info
 * deviceId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceFound_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    ProcessInfo processInfo;
    processInfo.pkgName = pkgName;
    DmDeviceInfo info = {
        .deviceId = "dkdkd",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    uint16_t subscribeId = 1;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    listener_->OnDeviceFound(processInfo, subscribeId, info);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnDiscoveryFailed_001
 * @tc.desc: OnDeviceFound,construct a dummy listener, pass in pkgName, subscribeId, failedReason
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDiscoveryFailed_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    std::string deviceId = "dkdkd";
    ProcessInfo processInfo;
    processInfo.pkgName = pkgName;
    uint16_t subscribeId = 1;
    int32_t failedReason = 1;
    listener_->OnDiscoveryFailed(processInfo, subscribeId, failedReason);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnDiscoverySuccess_001
 * @tc.desc: OnDeviceFound,construct a dummy listener, pass in pkgName, subscribeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDiscoverySuccess_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    uint16_t subscribeId = 1;
    ProcessInfo processInfo;
    processInfo.pkgName = pkgName;
    listener_->OnDiscoverySuccess(processInfo, subscribeId);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnPublishResult_001
 * @tc.desc: OnPublishResult, construct a dummy listener, pass in pkgName, publishId, failedReason
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnPublishResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    std::string deviceId = "dkdkd";
    int32_t publishId = 1;
    int32_t failedReason = 1;
    listener_->OnPublishResult(pkgName, publishId, failedReason);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnPublishResult_002
 * @tc.desc: OnDeviceResult,construct a dummy listener, pass in pkgName, publishId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnPublishResult_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    int32_t publishId = 1;
    int32_t failedReason = 0;
    listener_->OnPublishResult(pkgName, publishId, failedReason);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnUiCall_001
 * @tc.desc: OnUiCall, construct a dummy listener, pass in pkgName, paramJson
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnUiCall_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string paramJson = "ahaha";
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    listener_->OnUiCall(processInfo, paramJson);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnCredentialResult_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnCredentialResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    ProcessInfo processInfo;
    processInfo.pkgName = pkgName;
    int32_t action = 1;
    std::string resultInfo = "resultInfo";
    listener_->OnCredentialResult(processInfo, action, resultInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnBindResult_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnBindResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    PeerTargetId targetId;
    int32_t result = 0;
    int32_t status = 1;
    std::string content = "content";
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    listener_->OnBindResult(processInfo, targetId, result, status, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnBindResult_002
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnBindResult_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    PeerTargetId targetId;
    int32_t result = 0;
    int32_t status = 8;
    std::string content = "content";
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    listener_->OnBindResult(processInfo, targetId, result, status, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnBindResult_003
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnBindResult_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    PeerTargetId targetId;
    int32_t result = 0;
    int32_t status = -1;
    std::string content = "content";
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    listener_->OnBindResult(processInfo, targetId, result, status, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnUnbindResult_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnUnbindResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    PeerTargetId targetId;
    int32_t result = 0;
    std::string content = "content";
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    listener_->OnUnbindResult(processInfo, targetId, result, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: ConvertDeviceInfoToDeviceBasicInfo_001
 * @tc.desc: ConvertDeviceInfoToDeviceBasicInfo, construct a dummy listener, pass in pkgName, DmDeviceInfo
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ConvertDeviceInfoToDeviceBasicInfo_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    DmDeviceInfo info = {
        .deviceId = "12",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    DmDeviceBasicInfo deviceBasicInfo;
    listener_->ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);
    EXPECT_EQ(deviceBasicInfo.deviceTypeId, 1);
}

/**
 * @tc.name: ConvertDeviceInfoToDeviceBasicInfo_002
 * @tc.desc: Test conversion with all fields populated
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ConvertDeviceInfoToDeviceBasicInfo_002, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    std::string testDeviceId = "test_device_id_12345";
    std::string testDeviceName = "TestDevice";
    std::string testNetworkId = "test_network_id_67890";

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    memcpy_s(info.deviceId, sizeof(info.deviceId), testDeviceId.c_str(), testDeviceId.length());
    memcpy_s(info.deviceName, sizeof(info.deviceName), testDeviceName.c_str(), testDeviceName.length());
    memcpy_s(info.networkId, sizeof(info.networkId), testNetworkId.c_str(), testNetworkId.length());
    info.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PHONE);

    DmDeviceBasicInfo deviceBasicInfo;

    // Act
    listener_->ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);

    // Assert
    EXPECT_EQ(std::string(deviceBasicInfo.deviceId), testDeviceId);
    EXPECT_EQ(std::string(deviceBasicInfo.deviceName), testDeviceName);
    EXPECT_EQ(std::string(deviceBasicInfo.networkId), testNetworkId);
    EXPECT_EQ(deviceBasicInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PHONE));
}

/**
 * @tc.name: ConvertDeviceInfoToDeviceBasicInfo_003
 * @tc.desc: Test conversion with extraData containing valid custom data
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ConvertDeviceInfoToDeviceBasicInfo_003, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    std::string customData = "{\"key\":\"value\"}";
    std::string extraData = "{\"CUSTOM_DATA\":\"{\\\"key\\\":\\\"value\\\"}\"}";

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string testDeviceId = "test_device_id";
    std::string testDeviceName = "TestDevice";
    std::string testNetworkId = "test_network_id";
    memcpy_s(info.deviceId, sizeof(info.deviceId), testDeviceId.c_str(), testDeviceId.length());
    memcpy_s(info.deviceName, sizeof(info.deviceName), testDeviceName.c_str(), testDeviceName.length());
    memcpy_s(info.networkId, sizeof(info.networkId), testNetworkId.c_str(), testNetworkId.length());
    info.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PAD);
    info.extraData = extraData;

    DmDeviceBasicInfo deviceBasicInfo;

    // Act
    listener_->ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);

    // Assert
    EXPECT_EQ(std::string(deviceBasicInfo.deviceId), "test_device_id");
    EXPECT_EQ(std::string(deviceBasicInfo.deviceName), "TestDevice");
    EXPECT_EQ(std::string(deviceBasicInfo.networkId), "test_network_id");
    EXPECT_EQ(deviceBasicInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PAD));
    EXPECT_FALSE(deviceBasicInfo.extraData.empty());
}

/**
 * @tc.name: ConvertDeviceInfoToDeviceBasicInfo_004
 * @tc.desc: Test conversion with invalid extraData JSON
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ConvertDeviceInfoToDeviceBasicInfo_004, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    std::string invalidJsonExtraData = "invalid_json_{";

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string testDeviceId = "test_device_id";
    std::string testDeviceName = "TestDevice";
    std::string testNetworkId = "test_network_id";
    memcpy_s(info.deviceId, sizeof(info.deviceId), testDeviceId.c_str(), testDeviceId.length());
    memcpy_s(info.deviceName, sizeof(info.deviceName), testDeviceName.c_str(), testDeviceName.length());
    memcpy_s(info.networkId, sizeof(info.networkId), testNetworkId.c_str(), testNetworkId.length());
    info.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_WATCH);
    info.extraData = invalidJsonExtraData;

    DmDeviceBasicInfo deviceBasicInfo;

    // Act
    listener_->ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);

    // Assert - basic fields should still be copied even if extraData is invalid
    EXPECT_EQ(std::string(deviceBasicInfo.deviceId), "test_device_id");
    EXPECT_EQ(std::string(deviceBasicInfo.deviceName), "TestDevice");
    EXPECT_EQ(std::string(deviceBasicInfo.networkId), "test_network_id");
    EXPECT_EQ(deviceBasicInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_WATCH));
}

/**
 * @tc.name: ConvertDeviceInfoToDeviceBasicInfo_005
 * @tc.desc: Test conversion with extraData missing CUSTOM_DATA key
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ConvertDeviceInfoToDeviceBasicInfo_005, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    std::string extraDataWithoutCustomData = "{\"OTHER_KEY\":\"value\"}";

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string testDeviceId = "test_device_id";
    std::string testDeviceName = "TestDevice";
    std::string testNetworkId = "test_network_id";
    memcpy_s(info.deviceId, sizeof(info.deviceId), testDeviceId.c_str(), testDeviceId.length());
    memcpy_s(info.deviceName, sizeof(info.deviceName), testDeviceName.c_str(), testDeviceName.length());
    memcpy_s(info.networkId, sizeof(info.networkId), testNetworkId.c_str(), testNetworkId.length());
    info.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_TV);
    info.extraData = extraDataWithoutCustomData;

    DmDeviceBasicInfo deviceBasicInfo;

    // Act
    listener_->ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);

    // Assert
    EXPECT_EQ(std::string(deviceBasicInfo.deviceId), "test_device_id");
    EXPECT_EQ(std::string(deviceBasicInfo.deviceName), "TestDevice");
    EXPECT_EQ(std::string(deviceBasicInfo.networkId), "test_network_id");
    EXPECT_EQ(deviceBasicInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_TV));
}

/**
 * @tc.name: ConvertDeviceInfoToDeviceBasicInfo_006
 * @tc.desc: Test conversion with empty device info fields
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ConvertDeviceInfoToDeviceBasicInfo_006, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    info.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_UNKNOWN);

    DmDeviceBasicInfo deviceBasicInfo;

    // Act
    listener_->ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);

    // Assert
    EXPECT_EQ(deviceBasicInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_UNKNOWN));
    EXPECT_EQ(strlen(deviceBasicInfo.deviceId), 0U);
    EXPECT_EQ(strlen(deviceBasicInfo.deviceName), 0U);
    EXPECT_EQ(strlen(deviceBasicInfo.networkId), 0U);
}

/**
 * @tc.name: ConvertDeviceInfoToDeviceBasicInfo_007
 * @tc.desc: Test conversion with various device types
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ConvertDeviceInfoToDeviceBasicInfo_007, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";

    std::vector<DmDeviceType> deviceTypes = {
        DmDeviceType::DEVICE_TYPE_SMART_DISPLAY,
        DmDeviceType::DEVICE_TYPE_2IN1,
        DmDeviceType::DEVICE_TYPE_GLASSES,
        DmDeviceType::DEVICE_TYPE_CAR
    };

    for (auto deviceType : deviceTypes) {
        DmDeviceInfo info;
        memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        std::string testDeviceId = "test_device_id";
        std::string testDeviceName = "TestDevice";
        std::string testNetworkId = "test_network_id";
        memcpy_s(info.deviceId, sizeof(info.deviceId), testDeviceId.c_str(), testDeviceId.length());
        memcpy_s(info.deviceName, sizeof(info.deviceName), testDeviceName.c_str(), testDeviceName.length());
        memcpy_s(info.networkId, sizeof(info.networkId), testNetworkId.c_str(), testNetworkId.length());
        info.deviceTypeId = static_cast<uint16_t>(deviceType);

        DmDeviceBasicInfo deviceBasicInfo;

        // Act
        listener_->ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);

        // Assert
        EXPECT_EQ(deviceBasicInfo.deviceTypeId, static_cast<uint16_t>(deviceType));
    }
}

/**
 * @tc.name: ConvertDeviceInfoToDeviceBasicInfo_008
 * @tc.desc: Test conversion with extraData containing empty CUSTOM_DATA
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ConvertDeviceInfoToDeviceBasicInfo_008, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    std::string extraDataWithEmptyCustomData = "{\"CUSTOM_DATA\":\"\"}";

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string testDeviceId = "test_device_id";
    std::string testDeviceName = "TestDevice";
    std::string testNetworkId = "test_network_id";
    memcpy_s(info.deviceId, sizeof(info.deviceId), testDeviceId.c_str(), testDeviceId.length());
    memcpy_s(info.deviceName, sizeof(info.deviceName), testDeviceName.c_str(), testDeviceName.length());
    memcpy_s(info.networkId, sizeof(info.networkId), testNetworkId.c_str(), testNetworkId.length());
    info.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PC);
    info.extraData = extraDataWithEmptyCustomData;

    DmDeviceBasicInfo deviceBasicInfo;

    // Act
    listener_->ConvertDeviceInfoToDeviceBasicInfo(pkgName, info, deviceBasicInfo);

    // Assert
    EXPECT_EQ(std::string(deviceBasicInfo.deviceId), "test_device_id");
    EXPECT_EQ(std::string(deviceBasicInfo.deviceName), "TestDevice");
    EXPECT_EQ(std::string(deviceBasicInfo.networkId), "test_network_id");
    EXPECT_EQ(deviceBasicInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PC));
}

/**
 * @tc.name: SetDeviceInfo_001
 * @tc.desc: Test SetDeviceInfo with successful AppId retrieval
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceInfo_001, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();

    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;

    DmDeviceState state = DEVICE_STATE_ONLINE;

    DmDeviceInfo deviceInfo;
    memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string testDeviceId = "test_device_id_123";
    memcpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), testDeviceId.c_str(), testDeviceId.length());
    deviceInfo.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PHONE);

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));
    std::string testNetworkId = "test_network_id";
    memcpy_s(deviceBasicInfo.networkId, sizeof(deviceBasicInfo.networkId), testNetworkId.c_str(),
        testNetworkId.length());

    std::string appId = "test_app_id";

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(processInfo.pkgName, _, processInfo.userId))
        .WillOnce(DoAll(testing::SetArgReferee<1>(appId), testing::Return(DM_OK)));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(testing::Return(DM_OK));

    // Act
    listener_->SetDeviceInfo(pReq, processInfo, state, deviceInfo, deviceBasicInfo);

    // Assert
    EXPECT_EQ(pReq->GetPkgName(), processInfo.pkgName);
    EXPECT_EQ(pReq->GetDeviceState(), state);
    const DmDeviceInfo &resultInfo = pReq->GetDeviceInfo();
    EXPECT_EQ(resultInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PHONE));
    const DmDeviceBasicInfo &resultBasicInfo = pReq->GetDeviceBasicInfo();
    EXPECT_EQ(std::string(resultBasicInfo.deviceId), testDeviceId);
}

/**
 * @tc.name: SetDeviceInfo_002
 * @tc.desc: Test SetDeviceInfo when AppId retrieval fails
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceInfo_002, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();

    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;

    DmDeviceState state = DEVICE_STATE_ONLINE;

    DmDeviceInfo deviceInfo;
    memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string testDeviceId = "test_device_id_456";
    memcpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), testDeviceId.c_str(), testDeviceId.length());
    deviceInfo.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PAD);

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));
    std::string testNetworkId = "test_network_id_789";
    memcpy_s(deviceBasicInfo.networkId, sizeof(deviceBasicInfo.networkId), testNetworkId.c_str()
        testNetworkId.length());

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(processInfo.pkgName, _, processInfo.userId))
        .WillOnce(testing::Return(ERR_DM_FAILED));

    // Act
    listener_->SetDeviceInfo(pReq, processInfo, state, deviceInfo, deviceBasicInfo);

    // Assert
    EXPECT_EQ(pReq->GetPkgName(), processInfo.pkgName);
    EXPECT_EQ(pReq->GetDeviceState(), state);
    const DmDeviceInfo &resultInfo = pReq->GetDeviceInfo();
    EXPECT_EQ(resultInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PAD));
    const DmDeviceBasicInfo &resultBasicInfo = pReq->GetDeviceBasicInfo();
    EXPECT_EQ(std::string(resultBasicInfo.networkId), testNetworkId);
}

/**
 * @tc.name: SetDeviceInfo_003
 * @tc.desc: Test SetDeviceInfo with various device states
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceInfo_003, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    std::vector<DmDeviceState> deviceStates = {
        DEVICE_STATE_ONLINE,
        DEVICE_INFO_READY,
        DEVICE_STATE_OFFLINE,
        DEVICE_INFO_CHANGED
    };

    for (auto state : deviceStates) {
        std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();

        ProcessInfo processInfo;
        processInfo.pkgName = "com.ohos.helloworld";
        processInfo.userId = 100;

        DmDeviceInfo deviceInfo;
        memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        deviceInfo.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_WATCH);

        DmDeviceBasicInfo deviceBasicInfo;
        memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

        EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(testing::Return(ERR_DM_FAILED));

        // Act
        listener_->SetDeviceInfo(pReq, processInfo, state, deviceInfo, deviceBasicInfo);

        // Assert
        EXPECT_EQ(pReq->GetDeviceState(), state);
    }
}

/**
 * @tc.name: SetDeviceInfo_004
 * @tc.desc: Test SetDeviceInfo with different package names
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceInfo_004, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    std::vector<std::string> pkgNames = {
        "com.ohos.helloworld",
        "com.example.testapp",
        "ohos.distributedhardware.devicemanager"
    };

    for (const auto &pkgName : pkgNames) {
        std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();

        ProcessInfo processInfo;
        processInfo.pkgName = pkgName;
        processInfo.userId = 100;

        DmDeviceState state = DEVICE_STATE_ONLINE;

        DmDeviceInfo deviceInfo;
        memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        deviceInfo.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_TV);

        DmDeviceBasicInfo deviceBasicInfo;
        memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

        EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(pkgName, _, _)).WillOnce(testing::Return(ERR_DM_FAILED));

        // Act
        listener_->SetDeviceInfo(pReq, processInfo, state, deviceInfo, deviceBasicInfo);

        // Assert
        EXPECT_EQ(pReq->GetPkgName(), pkgName);
    }
}

/**
 * @tc.name: SetDeviceInfo_005
 * @tc.desc: Test SetDeviceInfo with various device types
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceInfo_005, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();

    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;

    DmDeviceState state = DEVICE_STATE_ONLINE;

    std::vector<DmDeviceType> deviceTypes = {
        DmDeviceType::DEVICE_TYPE_SMART_DISPLAY,
        DmDeviceType::DEVICE_TYPE_2IN1,
        DmDeviceType::DEVICE_TYPE_GLASSES,
        DmDeviceType::DEVICE_TYPE_CAR
    };

    for (auto deviceType : deviceTypes) {
        DmDeviceInfo deviceInfo;
        memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        deviceInfo.deviceTypeId = static_cast<uint16_t>(deviceType);

        DmDeviceBasicInfo deviceBasicInfo;
        memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

        EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(testing::Return(ERR_DM_FAILED));

        // Act
        listener_->SetDeviceInfo(pReq, processInfo, state, deviceInfo, deviceBasicInfo);

        // Assert
        const DmDeviceInfo &resultInfo = pReq->GetDeviceInfo();
        EXPECT_EQ(resultInfo.deviceTypeId, static_cast<uint16_t>(deviceType));
    }
}

/**
 * @tc.name: SetDeviceInfo_006
 * @tc.desc: Test SetDeviceInfo with empty device info
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceInfo_006, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();

    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;

    DmDeviceState state = DEVICE_STATE_ONLINE;

    DmDeviceInfo deviceInfo;
    memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    deviceInfo.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_UNKNOWN);

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(testing::Return(ERR_DM_FAILED));

    // Act
    listener_->SetDeviceInfo(pReq, processInfo, state, deviceInfo, deviceBasicInfo);

    // Assert
    EXPECT_EQ(pReq->GetPkgName(), processInfo.pkgName);
    EXPECT_EQ(pReq->GetDeviceState(), state);
    const DmDeviceInfo &resultInfo = pReq->GetDeviceInfo();
    EXPECT_EQ(resultInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_UNKNOWN));
}

/**
 * @tc.name: SetDeviceInfo_007
 * @tc.desc: Test SetDeviceInfo with ConvertUdidHashToAnoyAndSave success
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceInfo_007, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();

    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;

    DmDeviceState state = DEVICE_STATE_ONLINE;

    DmDeviceInfo deviceInfo;
    memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string testDeviceId = "test_device_id_789";
    memcpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId), testDeviceId.c_str(), testDeviceId.length());
    deviceInfo.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_AUDIO);

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));
    std::string testNetworkId = "test_network_id";
    memcpy_s(deviceBasicInfo.networkId, sizeof(deviceBasicInfo.networkId), testNetworkId.c_str()
        testNetworkId.length());

    std::string appId = "test_app_id_success";

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(processInfo.pkgName, _, processInfo.userId))
        .WillOnce(DoAll(testing::SetArgReferee<1>(appId), testing::Return(DM_OK)));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(testing::Return(DM_OK));

    // Act
    listener_->SetDeviceInfo(pReq, processInfo, state, deviceInfo, deviceBasicInfo);

    // Assert
    const DmDeviceBasicInfo &resultBasicInfo = pReq->GetDeviceBasicInfo();
    EXPECT_EQ(std::string(resultBasicInfo.deviceId), testDeviceId);
    const DmDeviceInfo &resultInfo = pReq->GetDeviceInfo();
    EXPECT_EQ(resultInfo.deviceTypeId, static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_AUDIO));
}

/**
 * @tc.name: SetDeviceInfo_008
 * @tc.desc: Test SetDeviceInfo with different user IDs
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceInfo_008, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    std::vector<int32_t> userIds = {0, 100, 200, 9999};

    for (auto userId : userIds) {
        std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();

        ProcessInfo processInfo;
        processInfo.pkgName = "com.ohos.helloworld";
        processInfo.userId = userId;

        DmDeviceState state = DEVICE_STATE_ONLINE;

        DmDeviceInfo deviceInfo;
        memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        deviceInfo.deviceTypeId = static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_WIFI_CAMERA);

        DmDeviceBasicInfo deviceBasicInfo;
        memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

        EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, userId)).WillOnce(testing::Return(ERR_DM_FAILED));

        // Act
        listener_->SetDeviceInfo(pReq, processInfo, state, deviceInfo, deviceBasicInfo);

        // Assert
        EXPECT_EQ(pReq->GetDeviceState(), state);
    }
}

/**
 * @tc.name: FillUdidAndUuidToDeviceInfo_001
 * @tc.desc: Test FillUdidAndUuidToDeviceInfo with non-high-priority package name
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_001, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";  // Not in high priority set

    DmDeviceInfo dmDeviceInfo;
    memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string testExtraData = "{\"key\":\"value\"}";
    dmDeviceInfo.extraData = testExtraData;

    // Act
    int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);

    // Assert
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(dmDeviceInfo.extraData, testExtraData);  // Should remain unchanged
}

/**
 * @tc.name: FillUdidAndUuidToDeviceInfo_002
 * @tc.desc: Test FillUdidAndUuidToDeviceInfo with high-priority package when GetUdidFromCache fails
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_002, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "ohos.deviceprofile";  // High priority package

    DmDeviceInfo dmDeviceInfo;
    memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string networkId = "test_network_id";
    memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), networkId.c_str(), networkId.length());
    dmDeviceInfo.extraData = "{\"key\":\"value\"}";

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(testing::Return(ERR_DM_FAILED));

    // Act
    int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);

    // Assert
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: FillUdidAndUuidToDeviceInfo_003
 * @tc.desc: Test FillUdidAndUuidToDeviceInfo with high-priority package when GetUuidFromCache fails
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_003, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "ohos.distributeddata.service";  // High priority package

    DmDeviceInfo dmDeviceInfo;
    memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string networkId = "test_network_id";
    memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), networkId.c_str(), networkId.length());
    dmDeviceInfo.extraData = "{\"key\":\"value\"}";

    std::string udid = "test_udid";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(testing::SetArgReferee<1>(udid), testing::Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _)).WillOnce(testing::Return(ERR_DM_FAILED));

    // Act
    int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);

    // Assert
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: FillUdidAndUuidToDeviceInfo_004
 * @tc.desc: Test FillUdidAndUuidToDeviceInfo with empty extraData
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_004, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "ohos.deviceprofile";  // High priority package

    DmDeviceInfo dmDeviceInfo;
    memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string networkId = "test_network_id";
    memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), networkId.c_str(), networkId.length());
    dmDeviceInfo.extraData = "";  // Empty extraData

    std::string udid = "test_udid";
    std::string uuid = "test_uuid";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(testing::SetArgReferee<1>(udid), testing::Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _))
        .WillOnce(DoAll(testing::SetArgReferee<1>(uuid), testing::Return(DM_OK)));

    // Act
    int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);

    // Assert
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: FillUdidAndUuidToDeviceInfo_005
 * @tc.desc: Test FillUdidAndUuidToDeviceInfo with invalid JSON in extraData
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_005, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "ohos.deviceprofile";  // High priority package

    DmDeviceInfo dmDeviceInfo;
    memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string networkId = "test_network_id";
    memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), networkId.c_str(), networkId.length());
    dmDeviceInfo.extraData = "invalid_json_{";  // Invalid JSON

    std::string udid = "test_udid";
    std::string uuid = "test_uuid";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(testing::SetArgReferee<1>(udid), testing::Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _))
        .WillOnce(DoAll(testing::SetArgReferee<1>(uuid), testing::Return(DM_OK)));

    // Act
    int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);

    // Assert
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: FillUdidAndUuidToDeviceInfo_006
 * @tc.desc: Test FillUdidAndUuidToDeviceInfo with successful UDID and UUID addition
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_006, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "ohos.deviceprofile";  // High priority package

    DmDeviceInfo dmDeviceInfo;
    memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string networkId = "test_network_id";
    memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), networkId.c_str(), networkId.length());
    dmDeviceInfo.extraData = "{\"existingKey\":\"existingValue\"}";

    std::string udid = "test_udid_12345";
    std::string uuid = "test_uuid_67890";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(testing::SetArgReferee<1>(udid), testing::Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _))
        .WillOnce(DoAll(testing::SetArgReferee<1>(uuid), testing::Return(DM_OK)));

    // Act
    int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);

    // Assert
    EXPECT_EQ(ret, DM_OK);
    // Verify extraData contains UDID and UUID
    EXPECT_NE(dmDeviceInfo.extraData.find("\"udid\""), std::string::npos);
    EXPECT_NE(dmDeviceInfo.extraData.find(udid), std::string::npos);
    EXPECT_NE(dmDeviceInfo.extraData.find("\"uuid\""), std::string::npos);
    EXPECT_NE(dmDeviceInfo.extraData.find(uuid), std::string::npos);
    EXPECT_NE(dmDeviceInfo.extraData.find("existingKey"), std::string::npos);
}

/**
 * @tc.name: FillUdidAndUuidToDeviceInfo_007
 * @tc.desc: Test FillUdidAndUuidToDeviceInfo with different high-priority packages
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_007, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    std::vector<std::string> highPriorityPkgs = {
        "ohos.deviceprofile",
        "ohos.distributeddata.service"
    };

    for (const auto &pkgName : highPriorityPkgs) {
        DmDeviceInfo dmDeviceInfo;
        memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        std::string networkId = "test_network_id_" + pkgName;
        memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), networkId.c_str(), networkId.length());
        dmDeviceInfo.extraData = "{\"key\":\"value\"}";

        std::string udid = "udid_" + pkgName;
        std::string uuid = "uuid_" + pkgName;
        EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
            .WillOnce(DoAll(testing::SetArgReferee<1>(udid), testing::Return(DM_OK)));
        EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _))
            .WillOnce(DoAll(testing::SetArgReferee<1>(uuid), testing::Return(DM_OK)));

        // Act
        int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);

        // Assert
        EXPECT_EQ(ret, DM_OK);
        EXPECT_NE(dmDeviceInfo.extraData.find(udid), std::string::npos);
        EXPECT_NE(dmDeviceInfo.extraData.find(uuid), std::string::npos);
    }
}

/**
 * @tc.name: FillUdidAndUuidToDeviceInfo_008
 * @tc.desc: Test FillUdidAndUuidToDeviceInfo with empty extraData initially, should fail
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_008, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "ohos.distributeddata.service";  // High priority package

    DmDeviceInfo dmDeviceInfo;
    memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string networkId = "test_network_id";
    memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), networkId.c_str(), networkId.length());
    dmDeviceInfo.extraData = "";  // Empty extraData

    // Act
    int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);

    // Assert - should return error because extraData is empty
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: FillUdidAndUuidToDeviceInfo_009
 * @tc.desc: Test FillUdidAndUuidToDeviceInfo with valid extraData containing special characters
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_009, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "ohos.deviceprofile";  // High priority package

    DmDeviceInfo dmDeviceInfo;
    memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string networkId = "test_network_id";
    memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), networkId.c_str(), networkId.length());
    dmDeviceInfo.extraData = "{\"key\":\"value with spaces \\\"quotes\\\"\"}";

    std::string udid = "test_udid_with_special_chars";
    std::string uuid = "test_uuid_with_special_chars";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(testing::SetArgReferee<1>(udid), testing::Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _))
        .WillOnce(DoAll(testing::SetArgReferee<1>(uuid), testing::Return(DM_OK)));

    // Act
    int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);

    // Assert
    EXPECT_EQ(ret, DM_OK);
    EXPECT_NE(dmDeviceInfo.extraData.find(udid), std::string::npos);
    EXPECT_NE(dmDeviceInfo.extraData.find(uuid), std::string::npos);
}

/**
 * @tc.name: ProcessDeviceStateChange_001
 * @tc.desc: Test ProcessDeviceStateChange with DEVICE_STATE_ONLINE
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_001, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";

    DmDeviceState state = DmDeviceState::DEVICE_STATE_ONLINE;

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = true;

    // Create mock process info list
    std::vector<ProcessInfo> processInfoVec;
    ProcessInfo pro1;
    pro1.pkgName = "ohos.deviceprofile";  // High priority
    pro1.userId = 100;
    ProcessInfo pro2;
    pro2.pkgName = "com.ohos.helloworld";  // Low priority
    pro2.userId = 100;
    processInfoVec.push_back(pro1);
    processInfoVec.push_back(pro2);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert - Verify device was marked as online
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

/**
 * @tc.name: ProcessDeviceStateChange_002
 * @tc.desc: Test ProcessDeviceStateChange with DEVICE_STATE_OFFLINE
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_002, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";

    DmDeviceState state = DmDeviceState::DEVICE_STATE_OFFLINE;

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id_offline";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = true;

    // Pre-populate alreadyOnlinePkgName_
    std::string notifyPkgName = processInfo.pkgName + "#" + std::to_string(processInfo.userId) +
        "#" + std::string(info.deviceId);
    listener_->alreadyOnlinePkgName_[notifyPkgName] = info;

    // Create mock process info list
    std::vector<ProcessInfo> processInfoVec;
    ProcessInfo pro;
    pro.pkgName = "com.ohos.helloworld";
    pro.userId = 100;
    processInfoVec.push_back(pro);

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert - Verify device was marked as offline (removed from alreadyOnlinePkgName_)
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: ProcessDeviceStateChange_003
 * @tc.desc: Test ProcessDeviceStateChange with DEVICE_INFO_READY
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_003, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";

    DmDeviceState state = DmDeviceState::DEVICE_INFO_READY;

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id_ready";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = true;

    // Create mock process info list
    std::vector<ProcessInfo> processInfoVec;
    ProcessInfo pro;
    pro.pkgName = "ohos.distributeddata.service";  // High priority
    pro.userId = 100;
    processInfoVec.push_back(pro);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

/**
 * @tc.name: ProcessDeviceStateChange_004
 * @tc.desc: Test ProcessDeviceStateChange with DEVICE_INFO_CHANGED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_004, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";

    DmDeviceState state = DmDeviceState::DEVICE_INFO_CHANGED;

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id_changed";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = true;

    // Create mock process info list
    std::vector<ProcessInfo> processInfoVec;
    ProcessInfo pro;
    pro.pkgName = "com.ohos.helloworld";
    pro.userId = 100;
    processInfoVec.push_back(pro);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

/**
 * @tc.name: ProcessDeviceStateChange_005
 * @tc.desc: Test ProcessDeviceStateChange with unknown state (default case)
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_005, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";

    DmDeviceState state = static_cast<DmDeviceState>(999);  // Unknown state

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id_unknown";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = true;

    // Create mock process info list
    std::vector<ProcessInfo> processInfoVec;
    ProcessInfo pro;
    pro.pkgName = "com.ohos.helloworld";
    pro.userId = 100;
    processInfoVec.push_back(pro);

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert - For unknown state, nothing should happen
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: ProcessDeviceStateChange_006
 * @tc.desc: Test ProcessDeviceStateChange with empty process info list
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_006, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";

    DmDeviceState state = DmDeviceState::DEVICE_STATE_ONLINE;

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id_empty";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = true;

    // Empty process info list will be returned by GetNotifyProcessInfoByUserId
    // (no mock setup, default behavior returns empty)

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert - No process info means no state change processing
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: ProcessDeviceStateChange_007
 * @tc.desc: Test ProcessDeviceStateChange with mixed high and low priority processes
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_007, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";

    DmDeviceState state = DmDeviceState::DEVICE_STATE_ONLINE;

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id_mixed";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = true;

    // Create mock process info list with both high and low priority
    std::vector<ProcessInfo> processInfoVec;
    ProcessInfo pro1;
    pro1.pkgName = "com.ohos.helloworld";  // Low priority
    pro1.userId = 100;
    ProcessInfo pro2;
    pro2.pkgName = "ohos.deviceprofile";  // High priority
    pro2.userId = 100;
    ProcessInfo pro3;
    pro3.pkgName = "ohos.distributeddata.service";  // High priority
    pro3.userId = 100;
    processInfoVec.push_back(pro1);
    processInfoVec.push_back(pro2);
    processInfoVec.push_back(pro3);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert - Verify device was marked as online
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

/**
 * @tc.name: ProcessDeviceStateChange_008
 * @tc.desc: Test ProcessDeviceStateChange with only high priority processes
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_008, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "ohos.deviceprofile";

    DmDeviceState state = DmDeviceState::DEVICE_STATE_ONLINE;

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id_hp_only";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = true;

    // Create mock process info list with only high priority packages
    std::vector<ProcessInfo> processInfoVec;
    ProcessInfo pro1;
    pro1.pkgName = "ohos.deviceprofile";
    pro1.userId = 100;
    ProcessInfo pro2;
    pro2.pkgName = "ohos.distributeddata.service";
    pro2.userId = 100;
    processInfoVec.push_back(pro1);
    processInfoVec.push_back(pro2);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

/**
 * @tc.name: ProcessDeviceStateChange_009
 * @tc.desc: Test ProcessDeviceStateChange with only low priority processes
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_009, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";

    DmDeviceState state = DmDeviceState::DEVICE_STATE_ONLINE;

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id_lp_only";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = true;

    // Create mock process info list with only low priority packages
    std::vector<ProcessInfo> processInfoVec;
    ProcessInfo pro1;
    pro1.pkgName = "com.ohos.helloworld";
    pro1.userId = 100;
    ProcessInfo pro2;
    pro2.pkgName = "com.example.testapp";
    pro2.userId = 100;
    processInfoVec.push_back(pro1);
    processInfoVec.push_back(pro2);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

/**
 * @tc.name: ProcessDeviceStateChange_010
 * @tc.desc: Test ProcessDeviceStateChange OFFLINE with isOnline = false
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_010, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";

    DmDeviceState state = DmDeviceState::DEVICE_STATE_OFFLINE;

    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    std::string deviceId = "test_device_id_offline_false";
    memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

    bool isOnline = false;

    // Pre-populate alreadyOnlinePkgName_
    std::string notifyPkgName = processInfo.pkgName + "#" + std::to_string(processInfo.userId) +
        "#" + std::string(info.deviceId);
    listener_->alreadyOnlinePkgName_[notifyPkgName] = info;

    // Create mock process info list
    std::vector<ProcessInfo> processInfoVec;
    ProcessInfo pro;
    pro.pkgName = "com.ohos.helloworld";
    pro.userId = 100;
    processInfoVec.push_back(pro);

    // Act
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

    // Assert
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: ProcessDeviceStateChange_011
 * @tc.desc: Test ProcessDeviceStateChange with different user IDs
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_011, testing::ext::TestSize.Level1)
{
    // Arrange
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    std::vector<int32_t> userIds = {0, 100, 200, 9999};

    for (auto userId : userIds) {
        ProcessInfo processInfo;
        processInfo.userId = userId;
        processInfo.pkgName = "com.ohos.helloworld";

        DmDeviceState state = DmDeviceState::DEVICE_STATE_ONLINE;

        DmDeviceInfo info;
        memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        std::string deviceId = "test_device_id_user_" + std::to_string(userId);
        memcpy_s(info.deviceId, sizeof(info.deviceId), deviceId.c_str(), deviceId.length());

        DmDeviceBasicInfo deviceBasicInfo;
        memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));

        bool isOnline = true;

        // Create mock process info list
        std::vector<ProcessInfo> processInfoVec;
        ProcessInfo pro;
        pro.pkgName = "com.ohos.helloworld";
        pro.userId = userId;
        processInfoVec.push_back(pro);

        EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, userId))
            .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));
        EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
            .Times(::testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));

        // Act
        listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, isOnline);

        // Assert - Clear for next iteration
        listener_->alreadyOnlinePkgName_.clear();
    }

    // Final assertion - all iterations completed
    EXPECT_TRUE(true);
}

/**
 * @tc.name: OnPinHolderCreate_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnPinHolderCreate_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string deviceId = "153123";
    DmPinType pinType = static_cast<DmPinType>(1);
    std::string payload = "payload";
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    listener_->OnPinHolderCreate(processInfo, deviceId, pinType, payload);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnPinHolderDestroy_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnPinHolderDestroy_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    DmPinType pinType = static_cast<DmPinType>(1);
    std::string payload = "payload";
    listener_->OnPinHolderDestroy(processInfo, pinType, payload);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnCreateResult_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnCreateResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    int32_t result = 0;
    listener_->OnCreateResult(processInfo, result);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnDestroyResult_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDestroyResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    int32_t result = 0;
    listener_->OnDestroyResult(processInfo, result);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnPinHolderEvent_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnPinHolderEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    DmPinHolderEvent event = DmPinHolderEvent::CREATE_RESULT;
    int32_t result = 0;
    std::string content = "content";
    listener_->OnPinHolderEvent(processInfo, event, result, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceScreenStateChange_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    DmDeviceInfo devInfo;
    listener_->OnDeviceScreenStateChange(processInfo, devInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);

    processInfo.pkgName = "ohos.distributedhardware.devicemanager";
    listener_->OnDeviceScreenStateChange(processInfo, devInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnCredentialAuthStatus_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    std::string deviceList = "deviceList";
    uint16_t deviceTypeId = 1;
    int32_t errcode = 0;
    listener_->OnCredentialAuthStatus(processInfo, deviceList, deviceTypeId, errcode);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnSinkBindResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    PeerTargetId targetId;
    int32_t result = 1;
    int32_t status = 0;
    std::string content = "content";
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    std::vector<ProcessInfo> processInfos;
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).Times(::testing::AtLeast(1))
        .WillOnce(Return(processInfos));
    listener_->OnSinkBindResult(processInfo, targetId, result, status, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnProcessRemove_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    DmDeviceInfo dmDeviceInfo;
    listener_->alreadyOnlinePkgName_["com.ohos.helloworld#100"] = dmDeviceInfo;
    DmDeviceInfo dmDeviceInfo1;
    listener_->alreadyOnlinePkgName_["com.ohos.network"] = dmDeviceInfo;
    listener_->OnProcessRemove(processInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    DmDeviceState state = DmDeviceState::DEVICE_STATE_ONLINE;
    DmDeviceInfo info;
    DmDeviceBasicInfo deviceBasicInfo;
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, true);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);

    state = DmDeviceState::DEVICE_INFO_CHANGED;
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo, true);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessAppStateChange_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "procName";
    DmDeviceState state = DmDeviceState::DEVICE_INFO_CHANGED;
    DmDeviceInfo info;
    DmDeviceBasicInfo deviceBasicInfo;
    std::set<std::string> systemSA;
    systemSA.insert("pkgName");
    std::vector<ProcessInfo> allProcessInfos;
    ProcessInfo processInfo1;
    processInfo1.pkgName = "_pickerProxy_" + processInfo.pkgName;
    allProcessInfos.push_back(processInfo1);
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).Times(::testing::AtLeast(1)).WillOnce(Return(systemSA));
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).Times(::testing::AtLeast(1))
        .WillOnce(Return(allProcessInfos));
    listener_->ProcessAppStateChange(processInfo, state, info, deviceBasicInfo, true);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, ConvertUdidHashToAnoyAndSave_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "pkgName004";
    DmDeviceInfo deviceInfo;
    int32_t userId = 10001;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = listener_->ConvertUdidHashToAnoyAndSave(pkgName, deviceInfo, userId);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    ret = listener_->ConvertUdidHashToAnoyAndSave(pkgName, deviceInfo, userId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceListenerTest, ConvertUdidHashToAnoyAndSave_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "pkgName002";
    std::string udidHash = "udidHash";
    std::string anoyDeviceId = "anoyDeviceId";
    int32_t userId = 10001;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    int32_t ret = listener_->ConvertUdidHashToAnoyDeviceId(pkgName, udidHash, anoyDeviceId, userId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceTrustChange_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string udid = "udid";
    std::string uuid = "uuid";
    DmAuthForm authForm = DmAuthForm::ACROSS_ACCOUNT;
    int32_t dmCommonNotifyEvent = 7;
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgNameyh";
    processInfo.userId = 103;
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo);

    std::vector<ProcessInfo> processInfos;
    processInfos.push_back(processInfo);
    std::set<std::string> systemSA;
    systemSA.insert("pkgName");
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    listener_->OnDeviceTrustChange(udid, uuid, authForm);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceScreenInfo_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    ProcessInfo processInfo;
    DmDeviceInfo deviceInfo;
    processInfo.pkgName = "pkgName";
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    listener_->SetDeviceScreenInfo(pReq, processInfo, deviceInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, RemoveOnlinePkgName_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    DmDeviceInfo info;
    memcpy_s(info.deviceId, sizeof(info.deviceId), "pkgName", sizeof("pkgName"));
    listener_->alreadyOnlinePkgName_["onlinePkgName1"] = info;
    DmDeviceInfo info1;
    listener_->alreadyOnlinePkgName_["onlinePkgName2"] = info1;
    listener_->RemoveOnlinePkgName(info);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnAppUnintall_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    DmDeviceInfo info;
    listener_->alreadyOnlinePkgName_["onlinePkgName1"] = info;
    DmDeviceInfo info1;
    listener_->alreadyOnlinePkgName_["onlinePkgName2"] = info1;
    std::string pkgName = "onlinePkgName1";
    listener_->OnAppUnintall(pkgName);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnCredentialAuthStatus_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    std::string deviceList;
    uint16_t deviceTypeId = 1;
    int32_t errcode = 2;

    int32_t dmCommonNotifyEvent = 7;
    ProcessInfo processInfo1;
    processInfo1.pkgName = "pkgNameed";
    processInfo1.userId = 102;
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo1);
    std::vector<ProcessInfo> processInfos;
    processInfos.push_back(processInfo1);
    std::set<std::string> systemSA;
    systemSA.insert("pkgName");
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    listener_->OnCredentialAuthStatus(processInfo, deviceList, deviceTypeId, errcode);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, GetWhiteListSAProcessInfo_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    DmCommonNotifyEvent dmCommonNotifyEvent = DmCommonNotifyEvent::MIN;
    auto ret = listener_->GetWhiteListSAProcessInfo(dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);

    dmCommonNotifyEvent = DmCommonNotifyEvent::REG_CREDENTIAL_AUTH_STATUS_NOTIFY;
    int32_t dmNotifyEvent = 7;
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgNamefg";
    processInfo.userId = 108;
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmNotifyEvent, processInfo);
    ret = listener_->GetWhiteListSAProcessInfo(dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);

    ProcessInfo processInfo1;
    processInfo1.pkgName = "ohos.deviceprofile";
    processInfo1.userId = 0;
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmNotifyEvent, processInfo1);
    ret = listener_->GetWhiteListSAProcessInfo(dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, GetNotifyProcessInfoByUserId_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    DmCommonNotifyEvent dmCommonNotifyEvent = DmCommonNotifyEvent::MIN;
    int32_t userId = 0;
    auto ret = listener_->GetNotifyProcessInfoByUserId(userId, dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);

    dmCommonNotifyEvent = DmCommonNotifyEvent::REG_CREDENTIAL_AUTH_STATUS_NOTIFY;
    int32_t dmNotifyEvent = 7;
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgName";
    processInfo.userId = 0;
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmNotifyEvent, processInfo);

    std::vector<ProcessInfo> processInfos;
    processInfos.push_back(processInfo);
    std::set<std::string> systemSA;
    systemSA.insert("pkgName");
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    ret = listener_->GetNotifyProcessInfoByUserId(userId, dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);

    processInfos.clear();
    processInfo.userId = 10;
    processInfo.pkgName = "packName";
    processInfos.push_back(processInfo);
    systemSA.insert("packName");
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    ret = listener_->GetNotifyProcessInfoByUserId(userId, dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);

    processInfos.clear();
    ProcessInfo processInfo1;
    processInfo1.pkgName = "pkgNamesjd";
    processInfo1.userId = 102;
    processInfos.push_back(processInfo1);
    userId = 102;
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    ret = listener_->GetNotifyProcessInfoByUserId(userId, dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);

    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmNotifyEvent, processInfo1);
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).Times(::testing::AtLeast(1))
        .WillOnce(Return(processInfos));
    ret = listener_->GetNotifyProcessInfoByUserId(userId, dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, DealBindProcessInfo_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgName";
    std::set<std::string> systemSA;
    systemSA.insert("pkgName");
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).Times(::testing::AtLeast(1)).WillOnce(Return(systemSA));
    ProcessInfo pro = listener_->DealBindProcessInfo(processInfo);
    EXPECT_EQ(pro.pkgName, "pkgName");
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceOnline_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::vector<ProcessInfo> procInfoVec;
    ProcessInfo pro;
    pro.pkgName = "pkgName";
    pro.userId = 101;
    procInfoVec.push_back(pro);
    ProcessInfo processInfo;
    DmDeviceState state = DmDeviceState::DEVICE_INFO_CHANGED;
    DmDeviceInfo info;
    DmDeviceBasicInfo deviceBasicInfo;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).Times(::testing::AtLeast(4)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    listener_->ProcessDeviceOnline(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);

    listener_->ProcessDeviceOnline(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceOffline_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::vector<ProcessInfo> procInfoVec;
    ProcessInfo pro;
    pro.pkgName = "pkgName";
    pro.userId = 101;
    procInfoVec.push_back(pro);
    ProcessInfo processInfo;
    DmDeviceState state = DmDeviceState::DEVICE_INFO_CHANGED;
    DmDeviceInfo info;
    DmDeviceBasicInfo deviceBasicInfo;
    std::string notifyPkgName = pro.pkgName + "#" + std::to_string(pro.userId) + "#" + std::string(info.deviceId);
    listener_->alreadyOnlinePkgName_[notifyPkgName] = info;
    listener_->ProcessDeviceOffline(procInfoVec, processInfo, state, info, deviceBasicInfo, true);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceInfoChange_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::vector<ProcessInfo> procInfoVec;
    ProcessInfo pro;
    pro.pkgName = "pkgName";
    pro.userId = 101;
    procInfoVec.push_back(pro);
    ProcessInfo processInfo;
    DmDeviceState state = DmDeviceState::DEVICE_INFO_CHANGED;
    DmDeviceInfo info;
    DmDeviceBasicInfo deviceBasicInfo;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    listener_->ProcessDeviceInfoChange(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnDevStateCallbackAdd_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgNamewer";
    processInfo.userId = 110;
    std::vector<DmDeviceInfo> deviceList;
    DmDeviceInfo info;
    info.authForm = DmAuthForm::ACROSS_ACCOUNT;
    info.deviceTypeId = 1;
    deviceList.push_back(info);
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    listener_->OnDevStateCallbackAdd(processInfo, deviceList);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnGetDeviceProfileInfoListResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgName";
    processInfo.userId = 101;
    std::vector<DmDeviceProfileInfo> deviceProfileInfos;
    int32_t code = 1;
    listener_->OnGetDeviceProfileInfoListResult(processInfo, deviceProfileInfos, code);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnGetDeviceIconInfoResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgName";
    processInfo.userId = 101;
    DmDeviceIconInfo dmDeviceIconInfo;
    int32_t code = 1;
    listener_->OnGetDeviceIconInfoResult(processInfo, dmDeviceIconInfo, code);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, RemoveNotExistProcess_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgName";
    processInfo.userId = 101;
    ProcessInfo processInfo1;
    processInfo1.pkgName = "pkgNamebmn";
    processInfo1.userId = 102;
    ProcessInfo pro;
    pro.pkgName = "pkgNamejk";
    pro.userId = 103;
    int32_t dmNotifyEvent = 1;
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmNotifyEvent, processInfo);
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmNotifyEvent, processInfo1);
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmNotifyEvent, pro);
    std::string notifyPkgName = processInfo.pkgName + "#" + std::to_string(processInfo.userId);
    DmDeviceInfo info;
    listener_->alreadyOnlinePkgName_[notifyPkgName] = info;
    listener_->RemoveNotExistProcess();
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnBindResult_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    PeerTargetId targetId;
    int32_t result = 0;
    int32_t status = 1;
    std::string content = "content";
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    listener_->OnBindResult(processInfo, targetId, result, status, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnAuthResult_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string deviceId = "dkdkd";
    std::string token = "kdkddk";
    int32_t status = 1;
    int32_t reason = 2006;
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.hellojk";
    processInfo.userId = 100;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    listener_->OnAuthResult(processInfo, deviceId, token, status, reason);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnUnbindResult_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    PeerTargetId targetId;
    int32_t result = 0;
    std::string content = "content";
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld01";
    processInfo.userId = 100;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    listener_->OnUnbindResult(processInfo, targetId, result, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnSinkBindResult_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworldkl";
    PeerTargetId targetId;
    int32_t result = 1;
    int32_t status = 1;
    std::string content = "content";
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    std::vector<ProcessInfo> processInfos;
    processInfos.push_back(processInfo);
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo())
        .Times(::testing::AtLeast(1)).WillOnce(Return(processInfos));
    listener_->OnSinkBindResult(processInfo, targetId, result, status, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, GetLocalDisplayDeviceName_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    EXPECT_NE(listener_, nullptr);
    std::string displayDeviceName = "";
    displayDeviceName = listener_->GetLocalDisplayDeviceName();
    EXPECT_FALSE(displayDeviceName.empty());
}

HWTEST_F(DeviceManagerServiceListenerTest, OpenAuthSessionWithPara_001, testing::ext::TestSize.Level1)
{
    const std::string deviceId = "OpenAuthSessionWithPara";
    int32_t actionId = 0;
    bool isEnable160m = false;
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    int32_t ret = listener_->OpenAuthSessionWithPara(deviceId, actionId, isEnable160m);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnAuthCodeInvalid_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    listener_->OnAuthCodeInvalid(pkgName, "");
    EXPECT_NE(listener_, nullptr);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnAuthCodeInvalid_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    listener_->OnAuthCodeInvalid(pkgName, "");
    EXPECT_TRUE(pkgName.empty());
}

HWTEST_F(DeviceManagerServiceListenerTest, OnAuthCodeInvalid_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    ProcessInfo targetProcessInfo;
    targetProcessInfo.pkgName = pkgName;
    std::vector<ProcessInfo> processInfos;
    processInfos.push_back(targetProcessInfo);
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    listener_->OnAuthCodeInvalid(pkgName, "");
    EXPECT_NE(listener_, nullptr);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
