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
}

void DeviceManagerServiceListenerTest::TearDownTestCase()
{
    DmSoftbusCache::dmSoftbusCache = nullptr;
    softbusCacheMock_ = nullptr;
    DmCrypto::dmCrypto = nullptr;
    cryptoMock_ = nullptr;
    DmIpcServerListener::dmIpcServerListener = nullptr;
    ipcServerListenerMock_ = nullptr;
    DmKVAdapterManager::dmKVAdapterManager = nullptr;
    kVAdapterManagerMock_ = nullptr;
    DmAppManager::dmAppManager = nullptr;
    appManagerMock_ = nullptr;
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
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
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

HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceScreenStateChange_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    DmDeviceInfo devInfo;
    ProcessInfo processInfoTmp;
    processInfoTmp.pkgName = "ohos.deviceprofile";
    processInfoTmp.userId = 106;
    int32_t dmCommonNotifyEvent = 3;
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfoTmp);
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    listener_->OnDeviceScreenStateChange(processInfo, devInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);

    processInfo.pkgName = "ohos.distributedhardware.devicemanager";
    ProcessInfo processInfo1;
    processInfo1.pkgName = "pkgName";
    processInfo1.userId = 101;
    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo1);

    std::vector<ProcessInfo> processInfos;
    processInfos.push_back(processInfo1);
    std::set<std::string> systemSA;
    systemSA.insert("pkgName");
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).WillOnce(Return(systemSA));
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    listener_->OnDeviceScreenStateChange(processInfo, devInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnCredentialAuthStatus_001, testing::ext::TestSize.Level0)
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

HWTEST_F(DeviceManagerServiceListenerTest, OnSinkBindResult_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworld";
    PeerTargetId targetId;
    int32_t result = 1;
    int32_t status = 0;
    std::string content = "content";
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    std::vector<ProcessInfo> processInfos;
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    listener_->OnSinkBindResult(processInfo, targetId, result, status, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnProcessRemove_001, testing::ext::TestSize.Level0)
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

HWTEST_F(DeviceManagerServiceListenerTest, FillUdidAndUuidToDeviceInfo_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "ohos.deviceprofile";
    DmDeviceInfo dmDeviceInfo;
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("udid"), Return(ERR_DM_FAILED)));
    int32_t ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("udid"), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("uuid"), Return(ERR_DM_FAILED)));
    ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("udid"), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("uuid"), Return(DM_OK)));
    dmDeviceInfo.extraData = "";
    ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    dmDeviceInfo.extraData = "extraData";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("udid"), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("uuid"), Return(DM_OK)));
    ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    JsonObject extraJson;
    extraJson["extraData"] = "extraDataInfo";
    dmDeviceInfo.extraData = extraJson.Dump();
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("udid"), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock_, GetUuidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("uuid"), Return(DM_OK)));
    ret = listener_->FillUdidAndUuidToDeviceInfo(pkgName, dmDeviceInfo);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceStateChange_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    DmDeviceState state = DmDeviceState::DEVICE_STATE_ONLINE;
    DmDeviceInfo info;
    DmDeviceBasicInfo deviceBasicInfo;
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);

    state = DmDeviceState::DEVICE_INFO_CHANGED;
    listener_->ProcessDeviceStateChange(processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessAppStateChange_001, testing::ext::TestSize.Level0)
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
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).WillOnce(Return(systemSA));
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(allProcessInfos));
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(4)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(4)).WillOnce(Return(DM_OK));
    listener_->ProcessAppStateChange(processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, ConvertUdidHashToAnoyAndSave_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "pkgName004";
    DmDeviceInfo deviceInfo;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = listener_->ConvertUdidHashToAnoyAndSave(pkgName, deviceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    ret = listener_->ConvertUdidHashToAnoyAndSave(pkgName, deviceInfo);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceListenerTest, ConvertUdidHashToAnoyAndSave_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string pkgName = "pkgName002";
    std::string udidHash = "udidHash";
    std::string anoyDeviceId = "anoyDeviceId";
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    int32_t ret = listener_->ConvertUdidHashToAnoyDeviceId(pkgName, udidHash, anoyDeviceId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnDeviceTrustChange_001, testing::ext::TestSize.Level0)
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
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).WillOnce(Return(systemSA));
    listener_->OnDeviceTrustChange(udid, uuid, authForm);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceScreenInfo_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    ProcessInfo processInfo;
    DmDeviceInfo deviceInfo;
    processInfo.pkgName = "pkgName";
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    listener_->SetDeviceScreenInfo(pReq, processInfo, deviceInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, RemoveOnlinePkgName_001, testing::ext::TestSize.Level0)
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

HWTEST_F(DeviceManagerServiceListenerTest, OnAppUnintall_001, testing::ext::TestSize.Level0)
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

HWTEST_F(DeviceManagerServiceListenerTest, OnCredentialAuthStatus_002, testing::ext::TestSize.Level0)
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
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).WillOnce(Return(systemSA));
    listener_->OnCredentialAuthStatus(processInfo, deviceList, deviceTypeId, errcode);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, GetWhiteListSAProcessInfo_001, testing::ext::TestSize.Level0)
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
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, GetNotifyProcessInfoByUserId_001, testing::ext::TestSize.Level0)
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
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).WillOnce(Return(systemSA));
    ret = listener_->GetNotifyProcessInfoByUserId(userId, dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), false);

    processInfos.clear();
    processInfo.userId = 10;
    processInfo.pkgName = "packName";
    processInfos.push_back(processInfo);
    systemSA.insert("packName");
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).WillOnce(Return(systemSA));
    ret = listener_->GetNotifyProcessInfoByUserId(userId, dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);

    processInfos.clear();
    ProcessInfo processInfo1;
    processInfo1.pkgName = "pkgNamesjd";
    processInfo1.userId = 102;
    processInfos.push_back(processInfo1);
    userId = 102;
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).WillOnce(Return(systemSA));
    ret = listener_->GetNotifyProcessInfoByUserId(userId, dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), true);

    DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmNotifyEvent, processInfo1);
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).WillOnce(Return(systemSA));
    ret = listener_->GetNotifyProcessInfoByUserId(userId, dmCommonNotifyEvent);
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, DealBindProcessInfo_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgName";
    std::set<std::string> systemSA;
    systemSA.insert("pkgName");
    EXPECT_CALL(*ipcServerListenerMock_, GetSystemSA()).WillOnce(Return(systemSA));
    ProcessInfo pro = listener_->DealBindProcessInfo(processInfo);
    EXPECT_EQ(pro.pkgName, "pkgName");
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceOnline_001, testing::ext::TestSize.Level0)
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
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(4)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    listener_->ProcessDeviceOnline(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);

    listener_->ProcessDeviceOnline(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceOffline_001, testing::ext::TestSize.Level0)
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
    listener_->ProcessDeviceOffline(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessDeviceInfoChange_001, testing::ext::TestSize.Level0)
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
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    listener_->ProcessDeviceInfoChange(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, ProcessAppOffline_001, testing::ext::TestSize.Level0)
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
    EXPECT_CALL(*softbusCacheMock_, CheckIsOnline(_)).WillOnce(Return(false));
    listener_->ProcessAppOffline(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);

    processInfo.pkgName = "pkgNamesa";
    processInfo.userId = 102;
    notifyPkgName = processInfo.pkgName + "#" + std::to_string(processInfo.userId) + "#" + std::string(info.deviceId);
    listener_->alreadyOnlinePkgName_[notifyPkgName] = info;
    EXPECT_CALL(*softbusCacheMock_, CheckIsOnline(_)).WillOnce(Return(true));
    listener_->ProcessAppOffline(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);

    EXPECT_CALL(*softbusCacheMock_, CheckIsOnline(_)).WillOnce(Return(true));
    listener_->ProcessAppOffline(procInfoVec, processInfo, state, info, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), true);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnDevStateCallbackAdd_001, testing::ext::TestSize.Level0)
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
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    listener_->OnDevStateCallbackAdd(processInfo, deviceList);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);

    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    listener_->OnDevStateCallbackAdd(processInfo, deviceList);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnGetDeviceProfileInfoListResult_001, testing::ext::TestSize.Level0)
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

HWTEST_F(DeviceManagerServiceListenerTest, OnGetDeviceIconInfoResult_001, testing::ext::TestSize.Level0)
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

HWTEST_F(DeviceManagerServiceListenerTest, RemoveNotExistProcess_001, testing::ext::TestSize.Level0)
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
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnBindResult_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    PeerTargetId targetId;
    int32_t result = 0;
    int32_t status = 1;
    std::string content = "content";
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld";
    processInfo.userId = 100;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    listener_->OnBindResult(processInfo, targetId, result, status, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnAuthResult_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::string deviceId = "dkdkd";
    std::string token = "kdkddk";
    int32_t status = 1;
    int32_t reason = 2006;
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.hellojk";
    processInfo.userId = 100;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    listener_->OnAuthResult(processInfo, deviceId, token, status, reason);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnUnbindResult_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    PeerTargetId targetId;
    int32_t result = 0;
    std::string content = "content";
    ProcessInfo processInfo;
    processInfo.pkgName = "com.ohos.helloworld01";
    processInfo.userId = 100;
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(DM_OK));
    listener_->OnUnbindResult(processInfo, targetId, result, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, OnSinkBindResult_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    ProcessInfo processInfo;
    processInfo.userId = 100;
    processInfo.pkgName = "com.ohos.helloworldkl";
    PeerTargetId targetId;
    int32_t result = 1;
    int32_t status = 1;
    std::string content = "content";
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyDeviceId(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    std::vector<ProcessInfo> processInfos;
    processInfos.push_back(processInfo);
    EXPECT_CALL(*ipcServerListenerMock_, GetAllProcessInfo()).WillOnce(Return(processInfos));
    listener_->OnSinkBindResult(processInfo, targetId, result, status, content);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}

HWTEST_F(DeviceManagerServiceListenerTest, SetDeviceInfo_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::make_shared<IpcNotifyDeviceStateReq>();
    ProcessInfo processInfo;
    processInfo.pkgName = "pkgNameqaz";
    DmDeviceState state = DmDeviceState::DEVICE_INFO_CHANGED;
    DmDeviceInfo deviceInfo;
    DmDeviceBasicInfo deviceBasicInfo;

    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).Times(::testing::AtLeast(3)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*appManagerMock_, GetAppIdByPkgName(_, _)).Times(::testing::AtLeast(2)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*cryptoMock_, ConvertUdidHashToAnoyAndSave(_, _, _)).WillOnce(Return(DM_OK));
    listener_->SetDeviceInfo(pReq, processInfo, state, deviceInfo, deviceBasicInfo);
    EXPECT_EQ(listener_->alreadyOnlinePkgName_.empty(), false);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
