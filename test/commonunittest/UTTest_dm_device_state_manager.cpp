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

#include "UTTest_dm_device_state_manager.h"

#include <iostream>

#include "dm_log.h"
#include "dm_constants.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_auth_result_req.h"
#include "dm_device_state_manager.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_discover_result_req.h"
#include "hichain_connector.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
void DmDeviceStateManagerTest::SetUp()
{
}

void DmDeviceStateManagerTest::TearDown()
{
}

void DmDeviceStateManagerTest::SetUpTestCase()
{
}

void DmDeviceStateManagerTest::TearDownTestCase()
{
}
namespace {
std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
std::shared_ptr<DmDeviceStateManager> dmDeviceStateManager =
    std::make_shared<DmDeviceStateManager>(softbusConnector, listener_, hiChainConnector_, hiChainAuthConnector);

/**
 * @tc.name: DmDeviceStateManager_001
 * @tc.desc: set DmDeviceStateManager to tne new pointer, and it's not nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDeviceStateManagerTest, DmDeviceStateManager_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmDeviceStateManager> p = std::make_shared<DmDeviceStateManager>(softbusConnector, listener_,
        hiChainConnector_, hiChainAuthConnector);
    ASSERT_NE(p, nullptr);
}

/**
 * @tc.name: DmDeviceStateManager_002
 * @tc.desc: set DmDeviceStateManager to tne new pointer,it's not nullptr and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDeviceStateManagerTest, DmDeviceStateManager_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmDeviceStateManager> p = std::make_shared<DmDeviceStateManager>(softbusConnector, listener_,
        hiChainConnector_, hiChainAuthConnector);
    p.reset();
    EXPECT_EQ(p, nullptr);
}

/**
 * @tc.name: OnDbReady_001
 * @tc.desc: set info.deviceId to some para, and return it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDeviceStateManagerTest, OnDbReady_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    DmDeviceInfo info;
    strcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, "123");
    dmDeviceStateManager->OnDbReady(pkgName, deviceId);
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq =
        std::static_pointer_cast<IpcNotifyDeviceStateReq>(listener_->ipcServerListener_.req_);
    DmDeviceInfo dminfo;
    if (pReq == nullptr) {
        strcpy_s(dminfo.deviceId, DM_MAX_DEVICE_ID_LEN, "123");
    } else {
        dminfo = pReq->GetDeviceInfo();
    }
    int result = strcmp(info.deviceId, dminfo.deviceId);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: RegisterOffLineTimer_001
 * @tc.desc: call RegisterOffLineTimer
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDeviceStateManagerTest, RegisterOffLineTimer_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "12345678901234567890987654321234",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    dmDeviceStateManager->RegisterOffLineTimer(info);
    std::string deviceId;
    SoftbusConnector::GetUdidByNetworkId(info.deviceId, deviceId);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_.count(deviceId), 0);
}

/**
 * @tc.name: SaveOnlineDeviceInfo_001
 * @tc.desc: call SaveOnlineDeviceInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmDeviceStateManagerTest, SaveOnlineDeviceInfo_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    dmDeviceStateManager->SaveOnlineDeviceInfo(info);
    std::string uuid;
    SoftbusConnector::GetUuidByNetworkId(info.networkId, uuid);
    EXPECT_EQ(dmDeviceStateManager->remoteDeviceInfos_.count(uuid), 0);
}

/**
 * @tc.name: DeleteOfflineDeviceInfo_001
 * @tc.desc: call DeleteOfflineDeviceInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmDeviceStateManagerTest, DeleteOfflineDeviceInfo_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    dmDeviceStateManager->DeleteOfflineDeviceInfo(info);
    for (auto iter: dmDeviceStateManager->remoteDeviceInfos_) {
        if (iter.second.deviceId == info.deviceId) {
            EXPECT_EQ(dmDeviceStateManager->remoteDeviceInfos_.count(iter.first), 0);
            break;
        }
    }
}

HWTEST_F(DmDeviceStateManagerTest, OnDeviceOnline_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    dmDeviceStateManager->OnDeviceOnline(deviceId, DmAuthForm::ACROSS_ACCOUNT);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTest, OnDeviceOffline_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    dmDeviceStateManager->OnDeviceOffline(deviceId, false);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTest, StartOffLineTimer_001, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "";
    dmDeviceStateManager->timer_ = nullptr;
    dmDeviceStateManager->StartOffLineTimer(peerUdid);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTest, DeleteTimeOutGroup_001, testing::ext::TestSize.Level0)
{
    auto hiChainConnectorTemp = dmDeviceStateManager->hiChainConnector_;
    dmDeviceStateManager->hiChainConnector_ = nullptr;
    std::string timerName = "timerName";
    dmDeviceStateManager->DeleteTimeOutGroup(timerName);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
    dmDeviceStateManager->hiChainConnector_ = hiChainConnectorTemp;
}

HWTEST_F(DmDeviceStateManagerTest, DeleteTimeOutGroup_002, testing::ext::TestSize.Level0)
{
    auto softbusConnectorTemp = dmDeviceStateManager->softbusConnector_;
    dmDeviceStateManager->softbusConnector_ = nullptr;
    std::string timerName = "timerName";
    dmDeviceStateManager->DeleteTimeOutGroup(timerName);
    EXPECT_NE(dmDeviceStateManager->hiChainConnector_, nullptr);
    dmDeviceStateManager->softbusConnector_ = softbusConnectorTemp;
}

HWTEST_F(DmDeviceStateManagerTest, DeleteTimeOutGroup_003, testing::ext::TestSize.Level0)
{
    std::string timerName = "timerName";
    dmDeviceStateManager->DeleteTimeOutGroup(timerName);
    EXPECT_NE(dmDeviceStateManager->hiChainConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTest, AddTask_001, testing::ext::TestSize.Level0)
{
    int32_t eventId = 0;
    std::string deviceId = "deviceId";
    std::shared_ptr<NotifyEvent> task = std::make_shared<NotifyEvent>(eventId, deviceId);
    int32_t ret = dmDeviceStateManager->AddTask(task);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmDeviceStateManagerTest, RunTask_001, testing::ext::TestSize.Level0)
{
    int32_t eventId = 0;
    std::string deviceId = "deviceId";
    std::shared_ptr<NotifyEvent> task = std::make_shared<NotifyEvent>(eventId, deviceId);
    dmDeviceStateManager->RunTask(task);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTest, GetAuthForm_001, testing::ext::TestSize.Level0)
{
    std::string networkId;
    dmDeviceStateManager->hiChainConnector_ = nullptr;
    int32_t ret = dmDeviceStateManager->GetAuthForm(networkId);
    EXPECT_EQ(ret, DmAuthForm::INVALID_TYPE);
}

HWTEST_F(DmDeviceStateManagerTest, GetAuthForm_002, testing::ext::TestSize.Level0)
{
    std::string networkId;
    dmDeviceStateManager->hiChainConnector_ = std::make_shared<HiChainConnector>();
    int32_t ret = dmDeviceStateManager->GetAuthForm(networkId);
    EXPECT_EQ(ret, DmAuthForm::INVALID_TYPE);
}

HWTEST_F(DmDeviceStateManagerTest, GetAuthForm_003, testing::ext::TestSize.Level0)
{
    std::string networkId = "networkId";
    int32_t ret = dmDeviceStateManager->GetAuthForm(networkId);
    EXPECT_EQ(ret, DmAuthForm::INVALID_TYPE);
}

HWTEST_F(DmDeviceStateManagerTest, ProcNotifyEvent_001, testing::ext::TestSize.Level0)
{
    int32_t eventId = 0;
    std::string deviceId;
    int32_t ret = dmDeviceStateManager->ProcNotifyEvent(eventId, deviceId);
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
        .networkId = "14569",
    };
    std::string udid = "udidTest";
    DmDeviceInfo remoteInfo = {
        .deviceId = "123",
    };
    dmDeviceStateManager->remoteDeviceInfos_[udid] = remoteInfo;
    dmDeviceStateManager->ChangeDeviceInfo(info);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
