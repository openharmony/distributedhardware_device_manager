/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_dm_device_state_manager_two.h"

#include <iostream>

#include "dm_log.h"
#include "dm_constants.h"
#include "dm_adapter_manager.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_auth_result_req.h"
#include "dm_device_state_manager.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_discover_result_req.h"
#include "hichain_connector.h"
#include "device_manager_service_listener.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void DmDeviceStateManagerTestTwo::SetUp()
{
}

void DmDeviceStateManagerTestTwo::TearDown()
{
}

void DmDeviceStateManagerTestTwo::SetUpTestCase()
{
    DmSoftbusConnector::dmSoftbusConnector = softbusConnectorMock_;
    DmCrypto::dmCrypto = cryptoMock_;
}

void DmDeviceStateManagerTestTwo::TearDownTestCase()
{
    DmSoftbusConnector::dmSoftbusConnector = nullptr;
    softbusConnectorMock_ = nullptr;
    DmCrypto::dmCrypto = nullptr;
}
namespace {
std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
std::shared_ptr<DmDeviceStateManager> dmDeviceStateManager =
    std::make_shared<DmDeviceStateManager>(softbusConnector, listener_, hiChainConnector_, hiChainAuthConnector);

HWTEST_F(DmDeviceStateManagerTestTwo, RegisterSoftbusStateCallback_001, testing::ext::TestSize.Level0)
{
    auto softbusConnector = dmDeviceStateManager->softbusConnector_;
    dmDeviceStateManager->softbusConnector_ = nullptr;
    int32_t ret = dmDeviceStateManager->RegisterSoftbusStateCallback();
    EXPECT_EQ(ret, DM_OK);
    dmDeviceStateManager->softbusConnector_ = softbusConnector;
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteOfflineDeviceInfo_002, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    dmDeviceStateManager->remoteDeviceInfos_["123"] = info;
    dmDeviceStateManager->stateDeviceInfos_["123"] = info;
    dmDeviceStateManager->DeleteOfflineDeviceInfo(info);
    for (auto iter: dmDeviceStateManager->remoteDeviceInfos_) {
        if (iter.second.deviceId == info.deviceId) {
            EXPECT_EQ(dmDeviceStateManager->remoteDeviceInfos_.count(iter.first), 0);
            break;
        }
    }
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteOfflineDeviceInfo_003, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    dmDeviceStateManager->remoteDeviceInfos_["123"] = info;
    dmDeviceStateManager->stateDeviceInfos_["123"] = info;
    strcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, "456");
    dmDeviceStateManager->DeleteOfflineDeviceInfo(info);
    for (auto iter: dmDeviceStateManager->remoteDeviceInfos_) {
        if (iter.second.deviceId == info.deviceId) {
            EXPECT_EQ(dmDeviceStateManager->remoteDeviceInfos_.count(iter.first), 0);
            break;
        }
    }
}

HWTEST_F(DmDeviceStateManagerTestTwo, OnDeviceOffline_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123";
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    dmDeviceStateManager->stateDeviceInfos_["123"] = info;
    dmDeviceStateManager->OnDeviceOffline(deviceId);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
    dmDeviceStateManager->stateDeviceInfos_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, HandleDeviceStatusChange_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    std::vector<ProcessInfo> processInfoVec;
    auto softbusConnector = dmDeviceStateManager->softbusConnector_;
    dmDeviceStateManager->softbusConnector_ = nullptr;
    dmDeviceStateManager->HandleDeviceStatusChange(DmDeviceState::DEVICE_STATE_OFFLINE, info, processInfoVec);
    dmDeviceStateManager->softbusConnector_ = softbusConnector;
    EXPECT_CALL(*softbusConnectorMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(ERR_DM_FAILED));
    dmDeviceStateManager->HandleDeviceStatusChange(DmDeviceState::DEVICE_STATE_OFFLINE, info, processInfoVec);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, ProcessDeviceStateChange_001, testing::ext::TestSize.Level0)
{
    ProcessInfo info;
    DmDeviceInfo devInfo;
    std::vector<ProcessInfo> processInfoVec;
    processInfoVec.push_back(info);
    dmDeviceStateManager->ProcessDeviceStateChange(DmDeviceState::DEVICE_STATE_OFFLINE, devInfo, processInfoVec);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, OnDeviceOnline_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    std::vector<ProcessInfo> processInfoVec;
    EXPECT_CALL(*softbusConnectorMock_, GetDeviceInfoByDeviceId("123")).WillOnce(Return(info));
    EXPECT_CALL(*softbusConnectorMock_, GetProcessInfo()).WillOnce(Return(processInfoVec));
    dmDeviceStateManager->OnDeviceOnline("123", 0);
    EXPECT_CALL(*softbusConnectorMock_, GetDeviceInfoByDeviceId("123")).WillOnce(Return(info));
    EXPECT_CALL(*softbusConnectorMock_, GetProcessInfo()).WillOnce(Return(processInfoVec));
    dmDeviceStateManager->stateDeviceInfos_["123"] = info;
    dmDeviceStateManager->OnDeviceOnline("123", 0);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
    dmDeviceStateManager->stateDeviceInfos_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, OnDbReady_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    dmDeviceStateManager->OnDbReady(pkgName, deviceId);

    pkgName = "123";
    dmDeviceStateManager->OnDbReady(pkgName, deviceId);

    deviceId = "123";
    dmDeviceStateManager->OnDbReady(pkgName, deviceId);

    DmDeviceInfo deviceInfo = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    dmDeviceStateManager->remoteDeviceInfos_["123"] = deviceInfo;
    dmDeviceStateManager->OnDbReady(pkgName, deviceId);
    EXPECT_NE(dmDeviceStateManager->listener_, nullptr);
    auto listener = dmDeviceStateManager->listener_;
    dmDeviceStateManager->listener_ = nullptr;
    dmDeviceStateManager->OnDbReady(pkgName, deviceId);
    dmDeviceStateManager->listener_ = listener;
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, RegisterOffLineTimer_002, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    StateTimerInfo stateTimerInfo = {
        .isStart = true,
    };
    dmDeviceStateManager->stateTimerInfoMap_["123"] = stateTimerInfo;
    EXPECT_CALL(*softbusConnectorMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(ERR_DM_FAILED));
    dmDeviceStateManager->RegisterOffLineTimer(info);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_["123"].isStart, true);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, RegisterOffLineTimer_003, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    StateTimerInfo stateTimerInfo = {
        .isStart = true,
    };
    dmDeviceStateManager->stateTimerInfoMap_["123"] = stateTimerInfo;
    EXPECT_CALL(*softbusConnectorMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(DoAll(WithArgs<1>(Invoke([](unsigned char* udidHash) {
            memcpy_s(udidHash, DM_MAX_DEVICE_ID_LEN, "123", 3);
        })), Return(DM_OK)));
    dmDeviceStateManager->timer_ = std::make_shared<DmTimer>();
    dmDeviceStateManager->RegisterOffLineTimer(info);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_["123"].isStart, false);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, RegisterOffLineTimer_004, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    StateTimerInfo stateTimerInfo;
    dmDeviceStateManager->stateTimerInfoMap_["123"] = stateTimerInfo;
    dmDeviceStateManager->udidhash2udidMap_["123"] = "test";
    EXPECT_CALL(*softbusConnectorMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(DoAll(WithArgs<1>(Invoke([](unsigned char* udidHash) {
            memcpy_s(udidHash, DM_MAX_DEVICE_ID_LEN, "123", 3);
        })), Return(DM_OK)));
    dmDeviceStateManager->timer_ = std::make_shared<DmTimer>();
    dmDeviceStateManager->RegisterOffLineTimer(info);
    EXPECT_NE(dmDeviceStateManager->udidhash2udidMap_["123"], "test");
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    dmDeviceStateManager->udidhash2udidMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, StartOffLineTimer_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    dmDeviceStateManager->timer_ = std::make_shared<DmTimer>();
    dmDeviceStateManager->StartOffLineTimer(deviceInfo);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, StartOffLineTimer_002, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
        .networkId = "aabbcc",
    };
    StateTimerInfo stateTimerInfo = {
        .timerName = "123",
        .networkId = "aabbcc",
        .isStart = false,
    };
    dmDeviceStateManager->stateTimerInfoMap_["123"] = stateTimerInfo;
    dmDeviceStateManager->StartOffLineTimer(info);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_["123"].isStart, true);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    dmDeviceStateManager->udidhash2udidMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteOffLineTimer_001, testing::ext::TestSize.Level0)
{
    StateTimerInfo stateTimerInfo;
    dmDeviceStateManager->stateTimerInfoMap_["123"] = stateTimerInfo;
    dmDeviceStateManager->timer_ = nullptr;
    dmDeviceStateManager->DeleteOffLineTimer("123");
    EXPECT_NE(dmDeviceStateManager->stateTimerInfoMap_.size(), 0);
    dmDeviceStateManager->timer_ = std::make_shared<DmTimer>();
    dmDeviceStateManager->DeleteOffLineTimer("");
    EXPECT_NE(dmDeviceStateManager->stateTimerInfoMap_.size(), 0);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteOffLineTimer_002, testing::ext::TestSize.Level0)
{
    dmDeviceStateManager->timer_ = std::make_shared<DmTimer>();
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    StateTimerInfo stateTimerInfo;
    dmDeviceStateManager->stateTimerInfoMap_["123"] = stateTimerInfo;
    dmDeviceStateManager->DeleteOffLineTimer("deviceInfo");
    EXPECT_NE(dmDeviceStateManager->stateTimerInfoMap_.size(), 0);
    dmDeviceStateManager->DeleteOffLineTimer("123");
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_.size(), 0);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteOffLineTimer_003, testing::ext::TestSize.Level0)
{
    dmDeviceStateManager->timer_ = std::make_shared<DmTimer>();
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    StateTimerInfo stateTimerInfo;
    dmDeviceStateManager->stateTimerInfoMap_["123"] = stateTimerInfo;
    dmDeviceStateManager->udidhash2udidMap_["deviceInfo"] = "test";
    dmDeviceStateManager->DeleteOffLineTimer("123");
    EXPECT_NE(dmDeviceStateManager->udidhash2udidMap_.size(), 0);

    dmDeviceStateManager->udidhash2udidMap_["123"] = "test";
    dmDeviceStateManager->DeleteOffLineTimer("123");
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    dmDeviceStateManager->udidhash2udidMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteTimeOutGroup_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    StateTimerInfo stateTimerInfo;
    dmDeviceStateManager->stateTimerInfoMap_["test"] = stateTimerInfo;
    dmDeviceStateManager->DeleteTimeOutGroup("123");
    stateTimerInfo.timerName = "123";
    dmDeviceStateManager->stateTimerInfoMap_["123"] = stateTimerInfo;
    auto hiChainConnector = dmDeviceStateManager->hiChainConnector_;
    dmDeviceStateManager->hiChainConnector_ = nullptr;
    dmDeviceStateManager->DeleteTimeOutGroup("123");
    dmDeviceStateManager->hiChainConnector_ = hiChainConnector;
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteTimeOutGroup_002, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    StateTimerInfo stateTimerInfo;
    stateTimerInfo.timerName = "123";
    dmDeviceStateManager->stateTimerInfoMap_["123"] = stateTimerInfo;
    dmDeviceStateManager->DeleteTimeOutGroup("123");
    dmDeviceStateManager->udidhash2udidMap_["123"] = "test";
    dmDeviceStateManager->DeleteTimeOutGroup("123");
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, RunTask_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<NotifyEvent> task = std::make_shared<NotifyEvent>(1, "");
    dmDeviceStateManager->RunTask(task);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, GetAuthForm_001, testing::ext::TestSize.Level0)
{
    EXPECT_CALL(*softbusConnectorMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    dmDeviceStateManager->GetAuthForm("task");
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, ChangeDeviceInfo_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123";
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    dmDeviceStateManager->stateDeviceInfos_.clear();
    dmDeviceStateManager->stateDeviceInfos_["123"] = info;
    dmDeviceStateManager->ChangeDeviceInfo(info);
    strcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, "456");
    dmDeviceStateManager->ChangeDeviceInfo(info);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, CheckIsOnline_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    dmDeviceStateManager->stateDeviceInfos_["123"] = info;
    bool ret = dmDeviceStateManager->CheckIsOnline("123");
    EXPECT_EQ(ret, true);
}

HWTEST_F(DmDeviceStateManagerTestTwo, GetUdidByNetWorkId_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
        .networkId = "123"
    };
    dmDeviceStateManager->stateDeviceInfos_["123"] = info;
    std::string ret = dmDeviceStateManager->GetUdidByNetWorkId("123");
    EXPECT_EQ(ret, "123");
    ret = dmDeviceStateManager->GetUdidByNetWorkId("test");
    EXPECT_EQ(ret, "");
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
