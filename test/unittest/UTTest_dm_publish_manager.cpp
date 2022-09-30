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

#include "UTTest_dm_publish_manager.h"

#include "dm_log.h"
#include "dm_constants.h"
#include "dm_anonymous.h"
#include "ipc_server_listener.h"
#include "device_manager_service_listener.h"
#include "softbus_bus_center.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
void DmPublishManagerTest::SetUp()
{
}

void DmPublishManagerTest::TearDown()
{
}

void DmPublishManagerTest::SetUpTestCase()
{
}

void DmPublishManagerTest::TearDownTestCase()
{
}

namespace {
std::shared_ptr<SoftbusConnector> softbusConnector_ = std::make_shared<SoftbusConnector>();
std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<DmPublishManager> publishMgr_ = std::make_shared<DmPublishManager>(softbusConnector_, listener_);

/**
 * @tc.name: DmPublishManager_001
 * @tc.desc: Test whether the DmPublishManager function can generate a new pointer
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DmPublishManagerTest, DmPublishManager_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmPublishManager> Test = std::make_shared<DmPublishManager>(softbusConnector_, listener_);
    ASSERT_NE(Test, nullptr);
}

/**
 * @tc.name: DmPublishManager_002
 * @tc.desc: Test whether the DmPublishManager function can delete a new pointer
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DmPublishManagerTest, DmPublishManager_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmPublishManager> Test = std::make_shared<DmPublishManager>(softbusConnector_, listener_);
    Test.reset();
    EXPECT_EQ(Test, nullptr);
}

/**
 * @tc.name: PublishDeviceDiscovery_001
 * @tc.desc: keeping pkgame unchanged, call PublishDeviceDiscovery twice
 *           so that its PublishQueue is not empty and return ERR_DM_PUBLISH_REPEATED
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DmPublishManagerTest, PublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmPublishInfo publishInfo;
    publishInfo.publishId = 9;
    publishInfo.mode = DM_DISCOVER_MODE_PASSIVE;
    publishInfo.freq = DM_HIGH;
    publishInfo.ranging = 1;
    publishMgr_->PublishDeviceDiscovery(pkgName, publishInfo);
    int32_t ret = publishMgr_->PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_EQ(ret, ERR_DM_PUBLISH_REPEATED);
    publishMgr_->UnPublishDeviceDiscovery(pkgName, publishInfo.publishId);
}

/**
 * @tc.name: PublishDeviceDiscovery_002
 * @tc.desc: pkgame changed, call PublishDeviceDiscovery twice
 *           so that its discoveryQueue is not empty and return ERR_DM_PUBLISH_REPEATED
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DmPublishManagerTest, PublishDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmPublishInfo publishInfo;
    publishInfo.publishId = 9;
    publishInfo.mode = DM_DISCOVER_MODE_PASSIVE;
    publishInfo.freq = DM_FREQ_BUTT;
    publishInfo.ranging = 1;
    publishMgr_->PublishDeviceDiscovery(pkgName, publishInfo);
    pkgName = "com.ohos.helloworld.new";
    int32_t ret = publishMgr_->PublishDeviceDiscovery(pkgName, publishInfo);
    ASSERT_EQ(ret, ERR_DM_PUBLISH_FAILED);
    publishMgr_->UnPublishDeviceDiscovery(pkgName, publishInfo.publishId);
}

/**
 * @tc.name: OnPublishResult_001
 * @tc.desc: The OnPublishFailed function takes the wrong case and emptying pkgName
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DmPublishManagerTest, OnPublishResult_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    int32_t publishId = 1;
    int32_t failedReason = 3;
    publishMgr_->OnPublishResult(pkgName, publishId, failedReason);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq =
        std::static_pointer_cast<IpcNotifyPublishResultReq>(listener_->ipcServerListener_.req_);
    std::string ret = pReq->GetPkgName();
    EXPECT_EQ(ret, pkgName);
}

/**
 * @tc.name: OnPublishResult_002
 * @tc.desc: The OnPublishSuccess function takes the wrong case and return pkgName
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DmPublishManagerTest, OnPublishResult_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t publishId = 1;
    publishMgr_->OnPublishResult(pkgName, publishId, 0);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq =
        std::static_pointer_cast<IpcNotifyPublishResultReq>(listener_->ipcServerListener_.req_);
    std::string ret = pReq->GetPkgName();
    EXPECT_NE(ret, pkgName);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
