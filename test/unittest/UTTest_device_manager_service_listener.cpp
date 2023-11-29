/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
    std::string pkgName = "com.ohos.helloworld";
    DmDeviceState state = DEVICE_STATE_ONLINE;
    DmDeviceInfo info = {
        .deviceId = "asdad",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    listener_->OnDeviceStateChange(pkgName, state, info);
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq =
        std::static_pointer_cast<IpcNotifyDeviceStateReq>(listener_->ipcServerListener_.req_);
    int32_t dmState = pReq->GetDeviceState();
    EXPECT_EQ(0, dmState);
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
    std::string pkgName = "com.ohos.helloworld";
    std::string deviceId = "dkdkd";
    std::string token = "kdkddk";
    int32_t status = 3;
    int32_t reason = 2006;
    listener_->OnAuthResult(pkgName, deviceId, token, status, reason);
    std::shared_ptr<IpcNotifyAuthResultReq> pReq =
        std::static_pointer_cast<IpcNotifyAuthResultReq>(listener_->ipcServerListener_.req_);
    std::string ret = pReq->GetPkgName();
    EXPECT_EQ(ret, pkgName);
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
    DmDeviceInfo info = {
        .deviceId = "dkdkd",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    uint16_t subscribeId = 1;
    listener_->OnDeviceFound(pkgName, subscribeId, info);
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq =
        std::static_pointer_cast<IpcNotifyDeviceFoundReq>(listener_->ipcServerListener_.req_);
    uint16_t ret = pReq->GetSubscribeId();
    EXPECT_EQ(ret, subscribeId);
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
    uint16_t subscribeId = 1;
    int32_t failedReason = 1;
    listener_->OnDiscoveryFailed(pkgName, subscribeId, failedReason);
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq =
        std::static_pointer_cast<IpcNotifyDiscoverResultReq>(listener_->ipcServerListener_.req_);
    int32_t ret = pReq->GetResult();
    EXPECT_EQ(ret, failedReason);
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
    listener_->OnDiscoverySuccess(pkgName, subscribeId);
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq =
        std::static_pointer_cast<IpcNotifyDiscoverResultReq>(listener_->ipcServerListener_.req_);
    uint16_t ret = pReq->GetSubscribeId();
    EXPECT_EQ(ret, subscribeId);
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
    std::shared_ptr<IpcNotifyPublishResultReq> pReq =
        std::static_pointer_cast<IpcNotifyPublishResultReq>(listener_->ipcServerListener_.req_);
    int32_t ret = pReq->GetPublishId();
    EXPECT_EQ(ret, publishId);
}

/**
 * @tc.name: OnPublishResult_001
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
    std::shared_ptr<IpcNotifyPublishResultReq> pReq =
        std::static_pointer_cast<IpcNotifyPublishResultReq>(listener_->ipcServerListener_.req_);
    int32_t ret = pReq->GetPublishId();
    EXPECT_EQ(ret, publishId);
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
    std::string pkgName = "com.ohos.helloworld";
    std::string paramJson = "ahaha";
    listener_->OnUiCall(pkgName, paramJson);
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq =
        std::static_pointer_cast<IpcNotifyDMFAResultReq>(listener_->ipcServerListener_.req_);
    std::string ret = pReq->GetJsonParam();
    EXPECT_EQ(ret, paramJson);
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
 * @tc.name: OnDeviceFound_002
 * @tc.desc: OnDeviceFound, construct a dummy listener, pass in pkgName, subscribeId, DmDeviceInfo
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceFound_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    uint16_t subscribeId = 1;
    DmDeviceBasicInfo info;
    listener_->OnDeviceFound(pkgName, subscribeId, info);
    EXPECT_EQ(listener_->dmListenerMap_.empty(), true);
}

/**
 * @tc.name: RegisterDmListener_001
 * @tc.desc: RegisterDmListener, construct a dummy listener, pass in pkgName, appId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, RegisterDmListener_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    std::string appId = "appId";
    listener_->RegisterDmListener(pkgName, appId);
    EXPECT_EQ(listener_->dmListenerMap_[pkgName], "appId");
}

/**
 * @tc.name: UnRegisterDmListener_001
 * @tc.desc: UnRegisterDmListener, construct a dummy listener, pass in pkgName
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, UnRegisterDmListener_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    listener_->dmListenerMap_[pkgName] = "appId";
    listener_->UnRegisterDmListener(pkgName);
    EXPECT_EQ(listener_->dmListenerMap_.find(pkgName), listener_->dmListenerMap_.end());
}

/**
 * @tc.name: GetAppId_001
 * @tc.desc: GetAppId, construct a dummy listener, pass in pkgName
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, GetAppId_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.helloworld";
    std::string str = listener_->GetAppId(pkgName);
    EXPECT_EQ(str.empty(), true);
}

/**
 * @tc.name: SetUdidHashMap_001
 * @tc.desc: SetUdidHashMap, construct a dummy listener, pass in udidHash
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, SetUdidHashMap_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string udidHash;
    std::string deviceId;
    std::string pkgName = "com.ohos.helloworld";
    listener_->SetUdidHashMap(udidHash, deviceId, pkgName);
    EXPECT_EQ(listener_->udidHashMap_.empty(), false);
}

/**
 * @tc.name: DeleteDeviceIdFromMap_001
 * @tc.desc: DeleteDeviceIdFromMap, construct a dummy listener, pass in deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, DeleteDeviceIdFromMap_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string deviceId;
    std::string pkgName = "com.ohos.helloworld";
    listener_->DeleteDeviceIdFromMap(deviceId, pkgName);
    EXPECT_EQ(listener_->udidHashMap_.empty(), false);
}

/**
 * @tc.name: GetDeviceId_001
 * @tc.desc: GetDeviceId, construct a dummy listener, pass in udidHash
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, GetDeviceId_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string udidHash;
    std::string pkgName = "com.ohos.helloworld";
    std::string str = listener_->GetDeviceId(udidHash, pkgName);
    EXPECT_EQ(str.empty(), true);
}

/**
 * @tc.name: GetUdidHash_001
 * @tc.desc: GetUdidHash, construct a dummy listener, pass in deviceId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, GetUdidHash_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string deviceId;
    std::string pkgName = "com.ohos.helloworld";
    std::string str = listener_->GetUdidHash(deviceId, pkgName);
    EXPECT_EQ(str.empty(), true);
}

/**
 * @tc.name: CalcDeviceId_001
 * @tc.desc: CalcDeviceId, construct a dummy listener, pass in pkgName
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTest, CalcDeviceId_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName;
    std::string udidHash;
    std::string str = listener_->CalcDeviceId(pkgName, udidHash);
    EXPECT_EQ(str.empty(), true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
