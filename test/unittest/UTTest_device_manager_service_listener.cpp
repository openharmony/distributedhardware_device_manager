/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
}

void DeviceManagerServiceListenerTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: OnDeviceStateChange_001
 * @tc.desc: OnDeviceStateChange, construct a dummy listener, pass in pkgName, use the constructed listener to get
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceStateChange_001, testing::ext::TestSize.Level0)
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
    listener_->OnDeviceStateChange(processInfo, state, info);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

/**
 * @tc.name: OnDeviceStateChange_002
 * @tc.desc: OnDeviceStateChange, construct a dummy listener, pass in pkgName, use the constructed listener to get
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceStateChange_002, testing::ext::TestSize.Level0)
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
    listener_->OnDeviceStateChange(processInfo, state, info);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnAuthResult_001
 * @tc.desc:OnAuthResult, construct a dummy listener, pass in pkgName, use the constructed listener to get pkgName
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnAuthResult_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnAuthResult_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnAuthResult_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceFound_001, testing::ext::TestSize.Level0)
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
    listener_->OnDeviceFound(processInfo, subscribeId, info);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnDiscoveryFailed_001
 * @tc.desc: OnDeviceFound,construct a dummy listener, pass in pkgName, subscribeId, failedReason
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDiscoveryFailed_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnDiscoverySuccess_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnPublishResult_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnPublishResult_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnUiCall_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnCredentialResult_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnBindResult_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnBindResult_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnBindResult_003, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnUnbindResult_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, ConvertDeviceInfoToDeviceBasicInfo_001, testing::ext::TestSize.Level0)
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
 * @tc.name: OnPinHolderCreate_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnPinHolderCreate_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnPinHolderDestroy_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnCreateResult_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnDestroyResult_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DeviceManagerServiceListenerTest, OnPinHolderEvent_001, testing::ext::TestSize.Level0)
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
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
