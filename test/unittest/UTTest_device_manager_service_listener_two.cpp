/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "UTTest_device_manager_service_listener_two.h"

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
#include "ipc_notify_bind_result_req.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceListenerTwoTest::SetUp()
{
}

void DeviceManagerServiceListenerTwoTest::TearDown()
{
}

void DeviceManagerServiceListenerTwoTest::SetUpTestCase()
{
    DmIpcServerListener::dmIpcServerListener = ipcServerListenerMock_;
    DmDeviceManagerService::dmDeviceManagerService = deviceManagerServiceMock_;
}

void DeviceManagerServiceListenerTwoTest::TearDownTestCase()
{
    deviceManagerServiceMock_ = nullptr;
    DmIpcServerListener::dmIpcServerListener = nullptr;
    ipcServerListenerMock_ = nullptr;
}

namespace {
/**
 * @tc.name: OnDeviceStateChange_003
 * @tc.desc: Test OnDeviceStateChange with DEVICE_INFO_READY state
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, OnDeviceStateChange_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    DmDeviceState state = DmDeviceState::DEVICE_INFO_READY;
    DmDeviceInfo info = {
        .deviceId = "testDeviceId123",
        .deviceName = "testDevice",
        .deviceTypeId = 1,
    };
    ASSERT_NE(listener_, nullptr);
    listener_->OnDeviceStateChange(processInfo, state, info, true);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnDeviceStateChange_004
 * @tc.desc: Test OnDeviceStateChange with DEVICE_INFO_CHANGED state
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, OnDeviceStateChange_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "ohos.distributedhardware.devicemanager";
    processInfo.userId = 100;
    DmDeviceState state = DmDeviceState::DEVICE_INFO_CHANGED;
    DmDeviceInfo info = {
        .deviceId = "testDeviceId456",
        .deviceName = "testDevice2",
        .deviceTypeId = 1,
    };
    ASSERT_NE(listener_, nullptr);
    listener_->OnDeviceStateChange(processInfo, state, info, true);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: GetWhiteListSAProcessInfo_002
 * @tc.desc: Test GetWhiteListSAProcessInfo with invalid event
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, GetWhiteListSAProcessInfo_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ASSERT_NE(listener_, nullptr);
    std::vector<ProcessInfo> result = listener_->GetWhiteListSAProcessInfo(
        static_cast<DmCommonNotifyEvent>(999));
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: GetNotifyProcessInfoByUserId_002
 * @tc.desc: Test GetNotifyProcessInfoByUserId with invalid event
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, GetNotifyProcessInfoByUserId_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ASSERT_NE(listener_, nullptr);
    std::vector<ProcessInfo> result = listener_->GetNotifyProcessInfoByUserId(
        100, static_cast<DmCommonNotifyEvent>(999));
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: GetNotifyProcessInfoByUserId_003
 * @tc.desc: Test GetNotifyProcessInfoByUserId with empty callback
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, GetNotifyProcessInfoByUserId_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();

    ASSERT_NE(listener_, nullptr);
    std::vector<ProcessInfo> result = listener_->GetNotifyProcessInfoByUserId(
        100, DmCommonNotifyEvent::REG_DEVICE_STATE);
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: SetNeedNotifyProcessInfos_001
 * @tc.desc: Test SetNeedNotifyProcessInfos when process not initialized
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, SetNeedNotifyProcessInfos_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.test";
    processInfo.userId = 100;

    std::vector<ProcessInfo> processInfos;
    std::vector<ProcessInfo> procInfoVec;

    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo())
        .WillOnce(Return(processInfos));

    ASSERT_NE(listener_, nullptr);
    listener_->SetNeedNotifyProcessInfos(processInfo, procInfoVec);
    EXPECT_EQ(procInfoVec.size(), 0);
}

/**
 * @tc.name: SetNeedNotifyProcessInfos_002
 * @tc.desc: Test SetNeedNotifyProcessInfos when callback not exist
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, SetNeedNotifyProcessInfos_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.test";
    processInfo.userId = 100;

    std::vector<ProcessInfo> processInfos;
    ProcessInfo proc;
    proc.pkgName = "com.ohos.test";
    proc.userId = 100;
    processInfos.push_back(proc);

    std::vector<ProcessInfo> procInfoVec;

    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo())
        .WillOnce(Return(processInfos));

    ASSERT_NE(listener_, nullptr);
    listener_->SetNeedNotifyProcessInfos(processInfo, procInfoVec);
    EXPECT_EQ(procInfoVec.size(), 0);
}

/**
 * @tc.name: SetNeedNotifyProcessInfos_003
 * @tc.desc: Test SetNeedNotifyProcessInfos when already in vector
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, SetNeedNotifyProcessInfos_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.test";
    processInfo.userId = 100;

    std::vector<ProcessInfo> processInfos;
    ProcessInfo proc;
    proc.pkgName = "com.ohos.test";
    proc.userId = 100;
    processInfos.push_back(proc);

    std::vector<ProcessInfo> procInfoVec;
    procInfoVec.push_back(proc);

    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo())
        .WillOnce(Return(processInfos));

    ASSERT_NE(listener_, nullptr);
    listener_->SetNeedNotifyProcessInfos(processInfo, procInfoVec);
    EXPECT_EQ(procInfoVec.size(), 1);
}

/**
 * @tc.name: OnGetDeviceProfileInfoListResult_002
 * @tc.desc: Test OnGetDeviceProfileInfoListResult with user defined device name
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, OnGetDeviceProfileInfoListResult_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.test";
    processInfo.userId = 100;

    DmDeviceProfileInfo deviceProfileInfo;
    deviceProfileInfo.isLocalDevice = true;
    deviceProfileInfo.deviceName = "OriginalName";
    std::vector<DmDeviceProfileInfo> deviceProfileInfos;
    deviceProfileInfos.push_back(deviceProfileInfo);
    int32_t code = DM_OK;

    ASSERT_NE(listener_, nullptr);
    listener_->OnGetDeviceProfileInfoListResult(processInfo, deviceProfileInfos, code);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnLeaveLNNResult_002
 * @tc.desc: Test OnLeaveLNNResult when process info not found
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, OnLeaveLNNResult_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "com.ohos.test";
    std::string networkId = "testNetworkId";
    int32_t retCode = DM_OK;

    std::vector<ProcessInfo> processInfos;

    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo())
        .WillOnce(Return(processInfos));

    ASSERT_NE(listener_, nullptr);
    listener_->OnLeaveLNNResult(pkgName, networkId, retCode);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnSetLocalDeviceNameResult_002
 * @tc.desc: Test OnSetLocalDeviceNameResult with success code
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, OnSetLocalDeviceNameResult_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.test";
    processInfo.userId = 100;
    std::string deviceName = "NewDeviceName";
    int32_t code = DM_OK;

    ASSERT_NE(listener_, nullptr);
    listener_->OnSetLocalDeviceNameResult(processInfo, deviceName, code);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnSetRemoteDeviceNameResult_002
 * @tc.desc: Test OnSetRemoteDeviceNameResult with different parameters
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, OnSetRemoteDeviceNameResult_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.test";
    processInfo.userId = 100;
    std::string deviceId = "testDeviceId";
    std::string deviceName = "RemoteDeviceName";
    int32_t code = DM_OK;

    ASSERT_NE(listener_, nullptr);
    listener_->OnSetRemoteDeviceNameResult(processInfo, deviceId, deviceName, code);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: OnDeviceStateChange_006
 * @tc.desc: Test OnDeviceStateChange with DM package and serviceIds
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, OnDeviceStateChange_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "ohos.distributedware.devicemanager";
    processInfo.userId = 100;
    DmDeviceState state = DmDeviceState::DEVICE_STATE_ONLINE;
    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    strcpy_s(info.deviceId, sizeof(info.deviceId), "testDeviceId");
    std::vector<int64_t> serviceIds;
    serviceIds.push_back(1001);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _))
        .WillOnce(Return(DM_OK));

    ASSERT_NE(listener_, nullptr);
    listener_->OnDeviceStateChange(processInfo, state, info, serviceIds);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

/**
 * @tc.name: SetExistPkgName_001
 * @tc.desc: Test SetExistPkgName
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceListenerTwoTest, SetExistPkgName_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::set<std::string> pkgNameSet;
    pkgNameSet.insert("com.ohos.test1");
    pkgNameSet.insert("com.ohos.test2");

    ASSERT_NE(listener_, nullptr);
    listener_->SetExistPkgName(pkgNameSet);

    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.test";
    processInfo.userId = 100;
    DmDeviceState state = DmDeviceState::DEVICE_STATE_OFFLINE;
    DmDeviceInfo info;
    memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    strcpy_s(info.deviceId, sizeof(info.deviceId), "testDeviceId");
    DmDeviceBasicInfo deviceBasicInfo;
    memset_s(&deviceBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo));
    bool isOnline = false;

    std::vector<ProcessInfo> procInfoVec;
    ProcessInfo procInfo;
    procInfo.pkgName = "com.ohos.test1";
    procInfo.userId = 100;
    procInfoVec.push_back(procInfo);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _, _))
        .WillRepeatedly(Return(DM_OK));

    listener_->ProcessDeviceOffline(procInfoVec, processInfo, state, info, deviceBasicInfo, isOnline);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
