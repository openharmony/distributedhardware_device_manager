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

#include "device_manager_service_listener.h"
#include "dm_constants.h"
#include "dm_device_state_manager.h"
#include "dm_log.h"
#include "hichain_connector.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
const uint32_t AUTH_ONCE_STATE_TIMER_MAX = 510;
constexpr const char* FILED_AUTHORIZED_APP_LIST = "authorizedAppList";
void DmDeviceStateManagerTestTwo::SetUp()
{
}

void DmDeviceStateManagerTestTwo::TearDown()
{
    Mock::VerifyAndClearExpectations(softbusConnectorMock_.get());
    Mock::VerifyAndClearExpectations(cryptoMock_.get());
    Mock::VerifyAndClearExpectations(hiChainConnectorMock_.get());
    Mock::VerifyAndClearExpectations(deviceProfileConnectorMock_.get());
    Mock::VerifyAndClearExpectations(multipleUserConnectorMock_.get());
    Mock::VerifyAndClearExpectations(hiChainAuthConnectorMock_.get());
}

void DmDeviceStateManagerTestTwo::SetUpTestCase()
{
    DmSoftbusConnector::dmSoftbusConnector = softbusConnectorMock_;
    DmCrypto::dmCrypto = cryptoMock_;
    DmHiChainConnector::dmHiChainConnector = hiChainConnectorMock_;
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock_;
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
    DmHiChainAuthConnector::dmHiChainAuthConnector = hiChainAuthConnectorMock_;
}

void DmDeviceStateManagerTestTwo::TearDownTestCase()
{
    DmSoftbusConnector::dmSoftbusConnector = nullptr;
    DmCrypto::dmCrypto = nullptr;
    DmHiChainConnector::dmHiChainConnector = nullptr;
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    DmHiChainAuthConnector::dmHiChainAuthConnector = nullptr;
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
    dmDeviceStateManager->HandleDeviceStatusChange(DmDeviceState::DEVICE_STATE_OFFLINE, info, processInfoVec,
        "123", true);
    dmDeviceStateManager->softbusConnector_ = softbusConnector;
    EXPECT_CALL(*softbusConnectorMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(ERR_DM_FAILED));
    dmDeviceStateManager->HandleDeviceStatusChange(DmDeviceState::DEVICE_STATE_OFFLINE, info, processInfoVec,
        "123", true);
    EXPECT_NE(dmDeviceStateManager->softbusConnector_, nullptr);
}

HWTEST_F(DmDeviceStateManagerTestTwo, ProcessDeviceStateChange_001, testing::ext::TestSize.Level0)
{
    ProcessInfo info;
    DmDeviceInfo devInfo;
    std::vector<ProcessInfo> processInfoVec;
    processInfoVec.push_back(info);
    dmDeviceStateManager->ProcessDeviceStateChange(DmDeviceState::DEVICE_STATE_OFFLINE, devInfo, processInfoVec, true);
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
    EXPECT_CALL(*softbusConnectorMock_, GetDeviceInfoByDeviceId("123", _)).WillOnce(Return(info));
    EXPECT_CALL(*softbusConnectorMock_, GetProcessInfo()).WillOnce(Return(processInfoVec));
    dmDeviceStateManager->OnDeviceOnline("123", 0);
    EXPECT_CALL(*softbusConnectorMock_, GetDeviceInfoByDeviceId("123", _)).WillOnce(Return(info));
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
    std::string peerUdid = "peerUdid";
    EXPECT_CALL(*softbusConnectorMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(peerUdid), Return(DM_OK)));
    int32_t localUserId = 100;
    int32_t peerUserId = 100;
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID()).WillOnce(Return(localUserId));
    std::unordered_set<int32_t> peerUserIds;
    EXPECT_CALL(*deviceProfileConnectorMock_, GetActiveAuthOncePeerUserId(_, _)).WillOnce(Return(peerUserIds));
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    dmDeviceStateManager->RegisterOffLineTimer(info);
    std::string key = peerUdid + "_" + std::to_string(peerUserId) + "_" + std::to_string(localUserId);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_.count(key), 0);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, RegisterOffLineTimer_003, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    std::string peerUdid = "peerUdid";
    EXPECT_CALL(*softbusConnectorMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(peerUdid), Return(DM_OK)));
    int32_t localUserId = 100;
    int32_t peerUserId = 100;
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID()).WillOnce(Return(localUserId));
    std::unordered_set<int32_t> peerUserIds {peerUserId};
    EXPECT_CALL(*deviceProfileConnectorMock_, GetActiveAuthOncePeerUserId(_, _)).WillOnce(Return(peerUserIds));
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    auto timer = dmDeviceStateManager->timer_;
    dmDeviceStateManager->timer_ = nullptr;
    for (uint32_t i = 0; i < AUTH_ONCE_STATE_TIMER_MAX; i++) {
        StateTimerInfo stateTimerInfo;
        dmDeviceStateManager->stateTimerInfoMap_[std::to_string(i)] = stateTimerInfo;
    }
    dmDeviceStateManager->RegisterOffLineTimer(info);
    std::string key = peerUdid + "_" + std::to_string(peerUserId) + "_" + std::to_string(localUserId);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_.count(key), 0);
    EXPECT_NE(dmDeviceStateManager->timer_, nullptr);
    dmDeviceStateManager->timer_->DeleteAll();
    dmDeviceStateManager->timer_ = timer;
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, RegisterOffLineTimer_004, testing::ext::TestSize.Level0)
{
    DmDeviceInfo info = {
        .deviceId = "123",
        .deviceName = "asda",
        .deviceTypeId = 1,
    };
    std::string peerUdid = "peerUdid";
    EXPECT_CALL(*softbusConnectorMock_, GetUdidByNetworkId(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(peerUdid), Return(DM_OK)));
    int32_t localUserId = 100;
    int32_t peerUserId = 100;
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID()).WillOnce(Return(localUserId));
    std::unordered_set<int32_t> peerUserIds {peerUserId};
    EXPECT_CALL(*deviceProfileConnectorMock_, GetActiveAuthOncePeerUserId(_, _)).WillOnce(Return(peerUserIds));
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    auto timer = dmDeviceStateManager->timer_;
    dmDeviceStateManager->timer_ = nullptr;
    StateTimerInfo stateTimerInfo = {.isStart = true};
    std::string key = peerUdid + "_" + std::to_string(peerUserId) + "_" + std::to_string(localUserId);
    dmDeviceStateManager->stateTimerInfoMap_[key] = stateTimerInfo;
    dmDeviceStateManager->RegisterOffLineTimer(info);
    EXPECT_FALSE(dmDeviceStateManager->stateTimerInfoMap_[key].isStart);
    EXPECT_NE(dmDeviceStateManager->timer_, nullptr);
    dmDeviceStateManager->timer_->DeleteAll();
    dmDeviceStateManager->timer_ = timer;
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, StartOffLineTimer_001, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "";
    std::unordered_set<AuthOnceAclInfo, AuthOnceAclInfoHash> authOnceAclInfos;
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAuthOnceAclInfos(_)).WillOnce(Return(authOnceAclInfos));
    dmDeviceStateManager->StartOffLineTimer(peerUdid);
}

HWTEST_F(DmDeviceStateManagerTestTwo, StartOffLineTimer_002, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "peerUdid";
    AuthOnceAclInfo aclInfo1 = { .peerUdid = peerUdid, .peerUserId = 100, .localUserId = 100, };
    AuthOnceAclInfo aclInfo2 = { .peerUdid = peerUdid, .peerUserId = 100, .localUserId = 101, };
    AuthOnceAclInfo aclInfo3 = { .peerUdid = peerUdid, .peerUserId = 101, .localUserId = 100, };
    std::unordered_set<AuthOnceAclInfo, AuthOnceAclInfoHash> authOnceAclInfos {aclInfo1, aclInfo2, aclInfo3};
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAuthOnceAclInfos(_)).WillOnce(Return(authOnceAclInfos));
    auto timer = dmDeviceStateManager->timer_;
    dmDeviceStateManager->timer_ = nullptr;
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    StateTimerInfo stateTimerInfo1 = {
        .timerName = "timerName",
        .peerUdid = peerUdid,
        .peerUserId = 100,
        .localUserId = 100,
        .isStart = false,
    };
    auto key = stateTimerInfo1.peerUdid + "_" + std::to_string(stateTimerInfo1.peerUserId) + "_" +
        std::to_string(stateTimerInfo1.localUserId);
    dmDeviceStateManager->stateTimerInfoMap_[key] = stateTimerInfo1;
    StateTimerInfo stateTimerInfo2 = {
        .timerName = "timerName",
        .peerUdid = peerUdid,
        .peerUserId = 100,
        .localUserId = 101,
        .isStart = true,
    };
    key = stateTimerInfo2.peerUdid + "_" + std::to_string(stateTimerInfo2.peerUserId) + "_" +
        std::to_string(stateTimerInfo2.localUserId);
    dmDeviceStateManager->stateTimerInfoMap_[key] = stateTimerInfo2;
    dmDeviceStateManager->StartOffLineTimer(peerUdid);
    key = stateTimerInfo1.peerUdid + "_" + std::to_string(stateTimerInfo1.peerUserId) + "_" +
        std::to_string(stateTimerInfo1.localUserId);
    EXPECT_TRUE(dmDeviceStateManager->stateTimerInfoMap_[key].isStart);
    EXPECT_NE(dmDeviceStateManager->timer_, nullptr);
    dmDeviceStateManager->timer_->DeleteAll();
    dmDeviceStateManager->timer_ = timer;
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteOffLineTimer_001, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "peerUdid";
    auto timer = dmDeviceStateManager->timer_;
    dmDeviceStateManager->timer_ = nullptr;
    auto size = dmDeviceStateManager->stateTimerInfoMap_.size();
    dmDeviceStateManager->DeleteOffLineTimer(peerUdid);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_.size(), size);
    dmDeviceStateManager->timer_ = timer;
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteOffLineTimer_002, testing::ext::TestSize.Level0)
{
    std::string peerUdid;
    if (dmDeviceStateManager->timer_ == nullptr) {
        dmDeviceStateManager->timer_ = std::make_shared<DmTimer>();
    }
    auto size = dmDeviceStateManager->stateTimerInfoMap_.size();
    dmDeviceStateManager->DeleteOffLineTimer(peerUdid);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_.size(), size);
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteOffLineTimer_003, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "peerUdid";
    if (dmDeviceStateManager->timer_ == nullptr) {
        dmDeviceStateManager->timer_ = std::make_shared<DmTimer>();
    }
    std::unordered_set<int32_t> peerUserIds;
    EXPECT_CALL(*deviceProfileConnectorMock_, GetActiveAuthOncePeerUserId(_, _)).WillOnce(Return(peerUserIds));
    auto size = dmDeviceStateManager->stateTimerInfoMap_.size();
    dmDeviceStateManager->DeleteOffLineTimer(peerUdid);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_.size(), size);
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteOffLineTimer_004, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "peerUdid";
    if (dmDeviceStateManager->timer_ == nullptr) {
        dmDeviceStateManager->timer_ = std::make_shared<DmTimer>();
    }
    int32_t localUserId = 100;
    EXPECT_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID()).WillOnce(Return(100));
    std::unordered_set<int32_t> peerUserIds{100};
    EXPECT_CALL(*deviceProfileConnectorMock_, GetActiveAuthOncePeerUserId(_, _)).WillOnce(Return(peerUserIds));
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    StateTimerInfo stateTimerInfo1 = {
        .timerName = "timerName",
        .peerUdid = "peerUdid",
        .peerUserId = 100,
        .localUserId = 100,
        .isStart = true,
    };
    auto key = stateTimerInfo1.peerUdid + "_" + std::to_string(stateTimerInfo1.peerUserId) + "_" +
        std::to_string(stateTimerInfo1.localUserId);
    dmDeviceStateManager->stateTimerInfoMap_[key] = stateTimerInfo1;
    StateTimerInfo stateTimerInfo2 = {
        .timerName = "timerName",
        .peerUdid = "peerUdid",
        .peerUserId = 101,
        .localUserId = 101,
        .isStart = true,
    };
    key = stateTimerInfo2.peerUdid + "_" + std::to_string(stateTimerInfo2.peerUserId) + "_" +
        std::to_string(stateTimerInfo2.localUserId);
    dmDeviceStateManager->stateTimerInfoMap_[key] = stateTimerInfo2;
    dmDeviceStateManager->DeleteOffLineTimer(peerUdid);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_.size(), 1);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
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

HWTEST_F(DmDeviceStateManagerTestTwo, StartDelTimerByDP_001, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "peerUdid";
    int32_t peerUserId = 100;
    int32_t localUserId = 100;
    StateTimerInfo stateTimerInfo = {
        .timerName = "timerName",
        .peerUdid = peerUdid,
        .peerUserId = peerUserId,
        .localUserId = localUserId,
        .isStart = true,
    };
    auto key = peerUdid + "_" + std::to_string(peerUserId) + "_" + std::to_string(localUserId);
    dmDeviceStateManager->stateTimerInfoMap_[key] = stateTimerInfo;
    dmDeviceStateManager->StartDelTimerByDP(peerUdid, peerUserId, localUserId);
    EXPECT_TRUE(dmDeviceStateManager->stateTimerInfoMap_[key].isStart);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, StartDelTimerByDP_002, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "peerUdid";
    int32_t peerUserId = 100;
    int32_t localUserId = 100;
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    for (uint32_t i = 0; i < AUTH_ONCE_STATE_TIMER_MAX; i++) {
        StateTimerInfo stateTimerInfo;
        dmDeviceStateManager->stateTimerInfoMap_[std::to_string(i)] = stateTimerInfo;
    }
    dmDeviceStateManager->StartDelTimerByDP(peerUdid, peerUserId, localUserId);
    auto key = peerUdid + "_" + std::to_string(peerUserId) + "_" + std::to_string(localUserId);
    EXPECT_EQ(dmDeviceStateManager->stateTimerInfoMap_.count(key), 0);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
}

HWTEST_F(DmDeviceStateManagerTestTwo, StartDelTimerByDP_003, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "peerUdid";
    int32_t peerUserId = 100;
    int32_t localUserId = 100;
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    StateTimerInfo stateTimerInfo = {
        .timerName = "timerName",
        .peerUdid = peerUdid,
        .peerUserId = peerUserId,
        .localUserId = localUserId,
        .isStart = false,
    };
    auto timer = dmDeviceStateManager->timer_;
    dmDeviceStateManager->timer_ = nullptr;
    dmDeviceStateManager->StartDelTimerByDP(peerUdid, peerUserId, localUserId);
    auto key = peerUdid + "_" + std::to_string(peerUserId) + "_" + std::to_string(localUserId);
    EXPECT_TRUE(dmDeviceStateManager->stateTimerInfoMap_[key].isStart);
    dmDeviceStateManager->stateTimerInfoMap_.clear();
    EXPECT_NE(dmDeviceStateManager->timer_, nullptr);
    dmDeviceStateManager->timer_->DeleteAll();
    dmDeviceStateManager->timer_ = timer;
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteGroupByDP_001, testing::ext::TestSize.Level0)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfileByUserId(_)).WillOnce(Return(profiles));
    int32_t ret = dmDeviceStateManager->DeleteGroupByDP("peerUdid", 100, 100);
    EXPECT_EQ(ret, DM_OK);
}

DistributedDeviceProfile::AccessControlProfile GenerateAccessControlProfile(
    const std::string &peerUdid, int32_t authType,
    const std::string &accesserBundleName, const std::string &accesserUdId, int32_t accesserUserId,
    const std::string &accesseeBundleName, const std::string &accesseeUdId, int32_t accesseeUserId)
{
    DistributedDeviceProfile::AccessControlProfile accessControlProfile;
    accessControlProfile.SetTrustDeviceId(peerUdid);
    accessControlProfile.SetAuthenticationType(authType);
    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserBundleName(accesserBundleName);
    accesser.SetAccesserDeviceId(accesserUdId);
    accesser.SetAccesserUserId(accesserUserId);
    accessControlProfile.SetAccesser(accesser);
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeBundleName(accesseeBundleName);
    accessee.SetAccesseeDeviceId(accesseeUdId);
    accessee.SetAccesseeUserId(accesseeUserId);
    accessControlProfile.SetAccessee(accessee);
    return accessControlProfile;
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteGroupByDP_002, testing::ext::TestSize.Level0)
{
    std::string peerUdid = "peerUdid";
    int32_t peerUserId = 100;
    int32_t localUserId = 100;
    auto aclProfile1 = GenerateAccessControlProfile(peerUdid, ALLOW_AUTH_ONCE, "BundleName", peerUdid, peerUserId,
        "BundleName", "localUdid", localUserId);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles {aclProfile1};
    EXPECT_CALL(*deviceProfileConnectorMock_, GetAccessControlProfileByUserId(_)).WillOnce(Return(profiles));
    auto hichainConnector = dmDeviceStateManager->hiChainConnector_;
    dmDeviceStateManager->hiChainConnector_ = nullptr;
    int32_t ret = dmDeviceStateManager->DeleteGroupByDP(peerUdid, peerUserId, localUserId);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
    dmDeviceStateManager->hiChainConnector_ = hichainConnector;
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteCredential_001, testing::ext::TestSize.Level0)
{
    DmOfflineParam offlineParam;
    std::string peerUdid = "peerUdid";
    int32_t localUserId = 100;
    EXPECT_CALL(*hiChainAuthConnectorMock_, DeleteCredential(_, _, _)).WillOnce(Return(0));
    dmDeviceStateManager->DeleteCredential(offlineParam, peerUdid, localUserId);
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteCredential_101, testing::ext::TestSize.Level0)
{
    DmAclIdParam acl = { .credId = "", .userId = 0, };
    EXPECT_CALL(*hiChainAuthConnectorMock_, QueryCredInfoByCredId(_, _, _))
        .WillOnce(Return(-1));
    dmDeviceStateManager->DeleteCredential(acl);
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteCredential_102, testing::ext::TestSize.Level0)
{
    DmAclIdParam acl = { .credId = "credId", .userId = 0, };
    JsonObject credJson;
    EXPECT_CALL(*hiChainAuthConnectorMock_, QueryCredInfoByCredId(_, _, _))
        .WillOnce(Invoke([&credJson](int a, const std::string &b, JsonObject &result) {
            result.Duplicate(credJson);
            return DM_OK;
        }));
    dmDeviceStateManager->DeleteCredential(acl);
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteCredential_103, testing::ext::TestSize.Level0)
{
    DmAclIdParam acl = { .credId = "credId", .userId = 0, };
    JsonObject credInfoJson;
    JsonObject credJson;
    credJson.Insert(acl.credId, credInfoJson);
    EXPECT_CALL(*hiChainAuthConnectorMock_, QueryCredInfoByCredId(_, _, _))
        .WillOnce(Invoke([&credJson](int a, const std::string &b, JsonObject &result) {
            result.Duplicate(credJson);
            return DM_OK;
        }));
    EXPECT_CALL(*hiChainAuthConnectorMock_, DeleteCredential(_, _)).WillOnce(Return(-1));
    dmDeviceStateManager->DeleteCredential(acl);
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteCredential_104, testing::ext::TestSize.Level0)
{
    std::unordered_set<int64_t> tokenIds{1, 2};
    DmAclIdParam acl = { .userId = 100, .credId = "credId", .tokenIds = tokenIds };
    std::string credInfoJsonStr = R"({"authorizedAppList":["1", "2"]})";
    JsonObject credInfoJson(credInfoJsonStr);
    JsonObject credJson;
    credJson.Insert(acl.credId, credInfoJson);
    EXPECT_CALL(*hiChainAuthConnectorMock_, QueryCredInfoByCredId(_, _, _))
        .WillOnce(Invoke([&credJson](int a, const std::string &b, JsonObject &result) {
            result.Duplicate(credJson);
            return DM_OK;
        }));
    EXPECT_CALL(*hiChainAuthConnectorMock_, DeleteCredential(_, _)).WillOnce(Return(-1));
    dmDeviceStateManager->DeleteCredential(acl);
}

HWTEST_F(DmDeviceStateManagerTestTwo, DeleteCredential_105, testing::ext::TestSize.Level0)
{
    std::unordered_set<int64_t> tokenIds{1, 2};
    DmAclIdParam acl = { .userId = 100, .credId = "credId", .tokenIds = tokenIds };
    std::string credInfoJsonStr = R"({"authorizedAppList": ["1", "2", "3"]})";
    JsonObject credInfoJson(credInfoJsonStr);
    JsonObject credJson;
    credJson.Insert(acl.credId, credInfoJson);
    EXPECT_CALL(*hiChainAuthConnectorMock_, QueryCredInfoByCredId(_, _, _))
        .WillOnce(Invoke([&credJson](int a, const std::string &b, JsonObject &result) {
            result.Duplicate(credJson);
            return DM_OK;
        }));
    EXPECT_CALL(*hiChainAuthConnectorMock_, UpdateCredential(_, _, _)).WillOnce(Return(-1));
    dmDeviceStateManager->DeleteCredential(acl);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
