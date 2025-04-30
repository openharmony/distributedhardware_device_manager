/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "UTTest_device_manager_impl.h"
#include "dm_device_info.h"

#include <unistd.h>
#include "accesstoken_kit.h"
#include "device_manager_notify.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_req.h"
#include "ipc_rsp.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_skeleton.h"
#include "ipc_publish_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DM_STRING_LENGTH_MAX = 1024;
void DeviceManagerImplTest::SetUp()
{
    const int32_t permsNum = 2;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.ACCESS_SERVICE_DM";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dsoftbus_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void DeviceManagerImplTest::TearDown()
{
    testing::Mock::VerifyAndClearExpectations(deviceManagerNotifyMock_.get());
    testing::Mock::VerifyAndClearExpectations(ipcClientProxyMock_.get());
}

void DeviceManagerImplTest::SetUpTestCase()
{
    DmDeviceManagerNotify::dmDeviceManagerNotify = deviceManagerNotifyMock_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxyMock_;
}

void DeviceManagerImplTest::TearDownTestCase()
{
    DmDeviceManagerNotify::dmDeviceManagerNotify = nullptr;
    deviceManagerNotifyMock_ = nullptr;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
    ipcClientProxyMock_ = nullptr;
}

namespace {
/**
 * @tc.name: InitDeviceManager_001
 * @tc.desc: 1. set packName not null
 *              set dmInitCallback not null
 *           2. call DeviceManagerImpl::InitDeviceManager with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set dmInitCallback not null
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: UnInitDeviceManager_101
 * @tc.desc: 1. set packName not null
 *           2. MOCK IpcClientProxy UnInit return DM_OK
 *           3. call DeviceManagerImpl::UnInitDeviceManager with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnInitDeviceManager_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test2";
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetTrustedDeviceList_101
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set deviceList null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetTrustedDeviceList_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra = "";
    // set deviceList null
    std::vector<DmDeviceInfo> deviceList;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: GetAvailableDeviceList_101
 * @tc.desc: 1. set packName null
 *              set deviceList null
 *           2. call DeviceManagerImpl::GetAvailableDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetAvailableDeviceList_101, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::vector<DmDeviceBasicInfo> deviceList;
    int32_t ret = DeviceManager::GetInstance().GetAvailableDeviceList(packName, deviceList);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetAvailableDeviceList_102
 * @tc.desc: 1. set packName not null
 *              set deviceList null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::GetAvailableDeviceList with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetAvailableDeviceList_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::vector<DmDeviceBasicInfo> deviceList;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().GetAvailableDeviceList(packName, deviceList);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: GetLocalDeviceNetWorkId_101
 * @tc.desc: 1. set packName null
 *              set networkId null
 *           2. call DeviceManagerImpl::GetLocalDeviceNetWorkId with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceNetWorkId_101, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string networkId;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceNetWorkId(packName, networkId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetLocalDeviceNetWorkId_102
 * @tc.desc: 1. set packName not null
 *              set networkId not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::GetLocalDeviceNetWorkId with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceNetWorkId_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string networkId = "networkId";
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().GetLocalDeviceNetWorkId(packName, networkId);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: GetLocalDeviceId_101
 * @tc.desc: 1. set packName null
 *              set deviceId null
 *           2. call DeviceManagerImpl::GetLocalDeviceId with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceId_101, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string deviceId;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceId(packName, deviceId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetLocalDeviceId_102
 * @tc.desc: 1. set packName not null
 *              set deviceId not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::GetLocalDeviceId with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceId_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string deviceId = "deviceId";
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().GetLocalDeviceId(packName, deviceId);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: GetLocalDeviceName_101
 * @tc.desc: 1. set packName null
 *              set deviceName null
 *           2. call DeviceManagerImpl::GetLocalDeviceName with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceName_101, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string deviceName;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceName(packName, deviceName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetLocalDeviceName_102
 * @tc.desc: 1. set packName not null
 *              set deviceName not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::GetLocalDeviceName with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceName_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string deviceName = "deviceName";
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().GetLocalDeviceName(packName, deviceName);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}


/**
 * @tc.name: GetLocalDeviceType_101
 * @tc.desc: 1. set packName null
 *              set deviceType 0
 *           2. call DeviceManagerImpl::GetLocalDeviceType with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceType_101, testing::ext::TestSize.Level0)
{
    std::string packName;
    int32_t deviceType = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceType(packName, deviceType);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetLocalDeviceType_102
 * @tc.desc: 1. set packName not null
 *              set deviceType 0
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::GetLocalDeviceType with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceType_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    int32_t deviceType = 0;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().GetLocalDeviceType(packName, deviceType);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: GetDeviceName_101
 * @tc.desc: 1. set packName null
 *              set networkId not null
 *              set deviceName null
 *           3. call DeviceManagerImpl::GetDeviceName with parameter
 *           4. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetDeviceName_101, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string networkId = "networkId";
    std::string deviceName;
    int32_t ret = DeviceManager::GetInstance().GetDeviceName(packName, networkId, deviceName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetDeviceName_102
 * @tc.desc: 1. set packName not null
 *              set networkId null
 *              set deviceName null
 *           2. call DeviceManagerImpl::GetDeviceName with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetDeviceName_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string networkId;
    std::string deviceName;
    int32_t ret = DeviceManager::GetInstance().GetDeviceName(packName, networkId, deviceName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetDeviceName_103
 * @tc.desc: 1. set packName not null
 *              set networkId not null
 *              set deviceName null
 *           2. call DeviceManagerImpl::GetDeviceName with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetDeviceName_103, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string networkId = "networkId";
    std::string deviceName;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetDeviceName(packName, networkId, deviceName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetDeviceName_104
 * @tc.desc: 1. set packName not null
 *              set networkId not null
 *              set deviceName null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::GetDeviceName with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetDeviceName_104, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string networkId = "networkId";
    std::string deviceName;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().GetDeviceName(packName, networkId, deviceName);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: GetDeviceType_101
 * @tc.desc: 1. set packName null
 *              set deviceList not null
 *              set deviceType 0
 *           2. call DeviceManagerImpl::GetDeviceType with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetDeviceType_101, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string networkId = "networkId";
    int32_t deviceType = 0;
    int32_t ret = DeviceManager::GetInstance().GetDeviceType(packName, networkId, deviceType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetDeviceType_102
 * @tc.desc: 1. set packName not null
 *              set networkId null
 *              set deviceType 0
 *           2. call DeviceManagerImpl::GetDeviceType with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetDeviceType_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string networkId;
    int32_t deviceType = 0;
    int32_t ret = DeviceManager::GetInstance().GetDeviceType(packName, networkId, deviceType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetDeviceType_103
 * @tc.desc: 1. set packName not null
 *              set networkId not null
 *              set deviceType 0
 *           2. call DeviceManagerImpl::GetDeviceType with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetDeviceType_103, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string networkId = "networkId";
    int32_t deviceType = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetDeviceType(packName, networkId, deviceType);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetDeviceType_104
 * @tc.desc: 1. set packName not null
 *              set networkId not null
 *              set deviceType 0
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::GetDeviceType with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetDeviceType_104, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string networkId = "networkId";
    int32_t deviceType = 0;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().GetDeviceType(packName, networkId, deviceType);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: UnBindDevice_101
 * @tc.desc: 1. set packName null
 *              set deviceId not null
 *           2. call DeviceManagerImpl::UnBindDevice with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, UnBindDevice_101, testing::ext::TestSize.Level0)
{
    std::string packName ;
    std::string deviceId = "deviceId";
    int32_t ret = DeviceManager::GetInstance().UnBindDevice(packName, deviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnBindDevice_102
 * @tc.desc: 1. set packName not null
 *              set deviceId null
 *           2. call DeviceManagerImpl::UnBindDevice with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, UnBindDevice_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string deviceId;
    int32_t ret = DeviceManager::GetInstance().UnBindDevice(packName, deviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnBindDevice_103
 * @tc.desc: 1. set packName not null
 *              set deviceId not null
 *           2. call DeviceManagerImpl::UnBindDevice with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, UnBindDevice_103, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string deviceId = "deviceId";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnBindDevice(packName, deviceId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: UnBindDevice_104
 * @tc.desc: 1. set packName not null
 *              set deviceId not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::UnBindDevice with parameter
 *           4. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, UnBindDevice_104, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string deviceId;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().UnBindDevice(packName, deviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, UnBindDevice_105, testing::ext::TestSize.Level0)
{
    std::string packName ;
    std::string deviceId = "deviceId";
    std::string extra = "extra";
    int32_t ret = DeviceManager::GetInstance().UnBindDevice(packName, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnBindDevice_106, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string deviceId;
    std::string extra = "extra";
    int32_t ret = DeviceManager::GetInstance().UnBindDevice(packName, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: BindDevice_101
 * @tc.desc: 1. set packName null
 *              set bindType 0
 *              set deviceId not null
 *              set bindParam null
 *              set callback null
 *           2. call DeviceManagerImpl::BindDevice with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, BindDevice_101, testing::ext::TestSize.Level0)
{
    std::string packName ;
    int32_t bindType = 0;
    std::string deviceId = "deviceId";
    std::string bindParam;
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().BindDevice(packName, bindType, deviceId, bindParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: BindDevice_102
 * @tc.desc: 1. set packName not null
 *              set bindType 0
 *              set deviceId null
 *              set bindParam null
 *              set callback null
 *           2. call DeviceManagerImpl::BindDevice with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, BindDevice_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    int32_t bindType = 0;
    std::string deviceId;
    std::string bindParam;
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().BindDevice(packName, bindType, deviceId, bindParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: BindDevice_103
 * @tc.desc: 1. set packName not null
 *              set bindType 0
 *              set deviceId not null
 *              set bindParam null
 *              set callback null
 *           2. call DeviceManagerImpl::BindDevice with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, BindDevice_103, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    int32_t bindType = 0;
    std::string deviceId = "deviceId";
    std::string bindParam = "{\"status\":1}";
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().BindDevice(packName, bindType, deviceId, bindParam, callback);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: BindDevice_104
 * @tc.desc: 1. set packName not null
 *              set bindType 0
 *              set deviceId not null
 *              set bindParam null
 *              set callback null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::BindDevice with parameter
 *           4. check ret is not DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, BindDevice_104, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    int32_t bindType = 0;
    std::string deviceId = "deviceId";
    std::string bindParam;
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    std::shared_ptr<DmInitCallback> initCallback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, initCallback);
    int32_t ret = DeviceManager::GetInstance().BindDevice(packName, bindType, deviceId, bindParam, callback);
    ASSERT_NE(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: RegisterDevStateCallback_101
 * @tc.desc: 1. set packName not null
 *              set extra not null
 *              set callback not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string extra = "extra";
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>();
    std::shared_ptr<DmInitCallback> initCallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, initCallback);
    ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra, callback);
    ASSERT_NE(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: RegisterDevStateCallback_102
 * @tc.desc: 1. set packName null
 *              set extra not null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_102, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string extra = "extra";
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStateCallback_103
 * @tc.desc: 1. set packName null
 *              set extra not null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_103, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string extra = "extra";
    std::shared_ptr<DeviceStateCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_101
 * @tc.desc: 1. set packName not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::UnRegisterDevStateCallback with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallbackk_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::shared_ptr<DmInitCallback> initCallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, initCallback);
    ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: UnRegisterDevStateCallback_102
 * @tc.desc: 1. set packName null
 *           2. call DeviceManagerImpl::UnRegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_102, testing::ext::TestSize.Level0)
{
    std::string packName;
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetLocalDeviceInfo_101
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set deviceList null
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceInfo_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set extra null
    DmDeviceInfo info;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_IPC_SEND_REQUEST_FAILED
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: StartDeviceDiscovery_101
 * @tc.desc: 1. set packName not null
 *              set subscribeInfo null
 *              set callback not null
 *           2. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string extra = "";
    DmSubscribeInfo subscribeInfo;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeInfo.subscribeId);
}

/**
 * @tc.name: StartDeviceDiscovery_102
 * @tc.desc: 1. set packName not null
 *              set subscribeInfo null
 *              set callback not null
 *           2. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_102, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string extra = "{\"findDeviceMode\":1}";
    DmSubscribeInfo subscribeInfo;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeInfo.subscribeId);
}

/**
 * @tc.name: StartDeviceDiscovery_103
 * @tc.desc: 1. set packName not null
 *              set subscribeId 0
 *              set filterOptions null
 *              set callback not null
 *           2. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_103, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    uint16_t subscribeId = 0;
    std::string filterOptions;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeId, filterOptions, callback);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
}

/**
 * @tc.name: StartDeviceDiscovery_104
 * @tc.desc: 1. set packName not null
 *              set subscribeId 0
 *              set filterOptions null
 *              set callback not null
 *           2. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           3. check ret is ERR_DM_DISCOVERY_REPEATED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_104, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    uint16_t subscribeId = 0;
    std::string filterOptions;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    DmDeviceInfo deviceInfo;
    callback->OnDeviceFound(subscribeId, deviceInfo);
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeId, filterOptions, callback);
    ASSERT_NE(ret, DM_OK);
}

/**
 * @tc.name: StopDeviceDiscovery_101
 * @tc.desc: 1. set packName not null
 *              set subscribeInfo null
 *              set callback not null
 *           2. call DeviceManagerImpl::StopDeviceDiscovery with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string extra = "";
    DmSubscribeInfo subscribeInfo;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeInfo.subscribeId);
    ASSERT_NE(ret, DM_OK);
}

/**
 * @tc.name: StopDeviceDiscovery_102
 * @tc.desc: 1. set packName not null
 *              set subscribeId is 0
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_102, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set subscribeInfo is 0
    uint16_t subscribeId = 0;
    // 2. InitDeviceManager return DM_OK
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is DM_OK
    ASSERT_NE(ret, ERR_DM_TIME_OUT);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: PublishDeviceDiscovery_101
 * @tc.desc: 1. set packName not null
 *              set publishInfo null
 *              set callback not null
 *           2. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    DmPublishInfo publishInfo;
    std::shared_ptr<PublishCallback> callback = std::make_shared<DevicePublishCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnPublishDeviceDiscovery_101
 * @tc.desc: 1. set packName not null
 *              set publishId is 0
 *           2. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, UnPublishDeviceDiscovery_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    int32_t publishId = 0;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: AuthenticateDevice_101
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, AuthenticateDevice_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set dmDeviceInfo null
    int32_t authType = 1;
    // set dmAppImageInfo null
    DmDeviceInfo dmDeviceInfo;
    strcpy_s(dmDeviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, "123XXXX");
    strcpy_s(dmDeviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN, "234");
    dmDeviceInfo.deviceTypeId = 0;
    // set extra null
    std::string extra = "test";
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2.InitDeviceManager return DM_OK
    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, initcallback);
    ASSERT_EQ(ret, DM_OK);
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: UnAuthenticateDevice_101
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnAuthenticateDevice_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    DmDeviceInfo deviceInfo;
    deviceInfo.networkId[0] = '1';
    deviceInfo.networkId[1] = '2';
    deviceInfo.networkId[2] = '\0';
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. InitDeviceManager return DM_OK
    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, initcallback);
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is ERR_DM_FAILED
    ASSERT_NE(ret, ERR_DM_TIME_OUT);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: SetUserOperation_101
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, SetUserOperation_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set authParam null
    int32_t action = 0;
    const std::string param = "extra";
    // 2. InitDeviceManager return DM_OK
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: InitDeviceManager_001
 * @tc.desc: 1. call DeviceManagerImpl::InitDeviceManager with packName = null, dmInitCallback = nullprt
 *           2. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager_001, testing::ext::TestSize.Level0)
{
    // 1. call DeviceManagerImpl::InitDeviceManager with packName = null, dmInitCallback = nullprt
    std::string packName = "";
    std::shared_ptr<DmInitCallback> dmInitCallback = nullptr;
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, dmInitCallback);
    // 2. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: InitDeviceManager_002
 * @tc.desc: 1. set packName not null
 *              set dmInitCallback not null
 *           2. call DeviceManagerImpl::InitDeviceManager with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager_002, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set dmInitCallback not null
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    // 2. call DeviceManagerImpl::InitDeviceManager with parameter
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: InitDeviceManager_003
 * @tc.desc: 1. set packName not null
 *              set dmInitCallback not null
 *           2. MOCK IpcClientProxy Init return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::InitDeviceManager with parameter
 *           4. check ret is ERR_DM_INIT_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    //    set dmInitCallback not null
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    // 2. MOCK IpcClientProxy Init return ERR_DM_FAILED
    // 3. call DeviceManagerImpl::InitDeviceManager with parameter
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: InitDeviceManager_004
 * @tc.desc: 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
 *           2. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager_004, testing::ext::TestSize.Level0)
{
    // 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
    std::string packName = "com.ohos.test";
    std::shared_ptr<DmInitCallbackTest> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 2. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: InitDeviceManager_005
 * @tc.desc: 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
 *           2. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager_005, testing::ext::TestSize.Level0)
{
    // 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
    std::string packName = "";
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 2. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnInitDeviceManager_001
 * @tc.desc: 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
 *           2. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnInitDeviceManager_001, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "";
    // 2. call DeviceManagerImpl::InitDeviceManager with parameter
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnInitDeviceManager_002
 * @tc.desc: 1. set packName not null
 *           2. MOCK IpcClientProxy UnInit return ERR_DM_FAILED
 *           3. call DeviceManagerImpl::UnInitDeviceManager with parameter
 *           4. check ret is ERR_DM_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnInitDeviceManager_002, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // 2. call DeviceManagerImpl::InitDeviceManager with parameter
    EXPECT_CALL(*ipcClientProxyMock_, UnInit(testing::_)).Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is ERR_DM_FAILED
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: UnInitDeviceManager_003
 * @tc.desc: 1. set packName not null
 *           2. MOCK IpcClientProxy UnInit return DM_OK
 *           3. call DeviceManagerImpl::UnInitDeviceManager with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnInitDeviceManager_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // 2. call DeviceManagerImpl::InitDeviceManager with parameter
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnInitDeviceManager_004
 * @tc.desc: 1. set packName not null
 *           2. MOCK IpcClientProxy UnInit return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::UnInitDeviceManager with parameter
 *           4. check ret is ERR_DM_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnInitDeviceManager_004, testing::ext::TestSize.Level0)
{
    // 1. set packNamen not null
    std::string packName = "com.ohos.test";
    // 2. call DeviceManagerImpl::InitDeviceManager with parameter
    EXPECT_CALL(*ipcClientProxyMock_, UnInit(testing::_)).Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is ERR_DM_FAILED
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: UnInitDeviceManager_005
 * @tc.desc: 1. set packName not null
 *           2. MOCK IpcClientProxy UnInit return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::UnInitDeviceManager with parameter
 *           4. check ret is ERR_DM_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnInitDeviceManager_005, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // 2. call DeviceManagerImpl::InitDeviceManager with parameter
    EXPECT_CALL(*ipcClientProxyMock_, UnInit(testing::_)).Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is ERR_DM_FAILED
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetTrustedDeviceList_001
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set deviceList null
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetTrustedDeviceList_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set extra null
    std::string extra = "";
    // set deviceList null
    std::vector<DmDeviceInfo> deviceList;
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetTrustedDeviceList_002
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set deviceList null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_IPC_SEND_REQUEST_FAILED
 *           3. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetTrustedDeviceList_002, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra = "";
    // set deviceList null
    std::vector<DmDeviceInfo> deviceList;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_IPC_SEND_REQUEST_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetTrustedDeviceList_003
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set deviceList null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetTrustedDeviceList_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra = "";
    // set deviceList null
    std::vector<DmDeviceInfo> deviceList;
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: GetTrustedDeviceList_004
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set deviceList null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetTrustedDeviceList_004, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra = "test";
    // set deviceList null
    std::vector<DmDeviceInfo> deviceList;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetTrustedDeviceList_005
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set deviceList null
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetTrustedDeviceList_005, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set extra null
    std::string extra = "test";
    // set deviceList null
    std::vector<DmDeviceInfo> deviceList;
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetTrustedDeviceList_006
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set deviceList null
 *              set isRefresh true
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetTrustedDeviceList_006, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::string extra = "";
    bool  isRefresh = true;
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, isRefresh, deviceList);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetTrustedDeviceList_007
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set deviceList null
 *              set isRefresh true
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetTrustedDeviceList_007, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string extra = "";
    bool  isRefresh = true;
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, isRefresh, deviceList);
    ASSERT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetTrustedDeviceList_008
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set deviceList null
 *              set isRefresh true
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetTrustedDeviceList_008, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::map<std::string, std::string> filterOptions;
    bool isRefresh = false;
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, filterOptions, isRefresh, deviceList);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    packName = "packNameTest";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, filterOptions, isRefresh, deviceList);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: ImportCredential_001
 * @tc.desc: 1. set pkgName not null
 *              set reqJsonStr null
 *              set returnJsonStr null
 *           2. call DeviceManagerImpl::ImportCredential with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, ImportCredential_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string reqJsonStr = "";
    std::string returnJsonStr = "";
    int32_t ret = DeviceManager::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "";
    reqJsonStr = "reqJsonStrTest";
    ret = DeviceManager::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_NE(ret, ERR_DM_TIME_OUT);
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(3).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED))
                .WillOnce(testing::Return(DM_OK)).WillOnce(testing::Return(DM_OK));
    ret = DeviceManager::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    ret = DeviceManager::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RequestCredential_001
 * @tc.desc: 1. set pkgName not null
 *              set reqJsonStr null
 *              set returnJsonStr null
 *           2. call DeviceManagerImpl::RequestCredential with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, RequestCredential_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string reqJsonStr = "";
    std::string returnJsonStr = "";
    int32_t ret = DeviceManager::GetInstance().RequestCredential(pkgName, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().RequestCredential(pkgName, returnJsonStr);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "";
    reqJsonStr = "reqJsonStrTest";
    ret = DeviceManager::GetInstance().RequestCredential(pkgName, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().RequestCredential(pkgName, returnJsonStr);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GenerateEncryptedUuid_001
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set deviceList null
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GenerateEncryptedUuid_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string uuid = "";
    std::string appId = "";
    std::string encryptedUuid = "";
    int32_t ret = DeviceManager::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "";
    uuid = "uuidTest";
    ret = DeviceManager::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetNetworkTypeByNetworkId_001
 * @tc.desc: 1. set pkgName null
 *              set networkId null
 *              set networkType null
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetNetworkTypeByNetworkId_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string networkId = "";
    int32_t netWorkType = 0;
    int32_t ret = DeviceManager::GetInstance().GetNetworkTypeByNetworkId(pkgName, networkId, netWorkType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().GetNetworkTypeByNetworkId(pkgName, networkId, netWorkType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "";
    networkId = "networkIdTest";
    ret = DeviceManager::GetInstance().GetNetworkTypeByNetworkId(pkgName, networkId, netWorkType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().GetNetworkTypeByNetworkId(pkgName, networkId, netWorkType);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().GetNetworkTypeByNetworkId(pkgName, networkId, netWorkType);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ImportAuthCode_001
 * @tc.desc: 1. set packName null
 *              set authCode null
 *           2. call DeviceManagerImpl::ImportAuthCode with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, ImportAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string authCode = "";
    int32_t ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "";
    authCode = "authCodeTest";
    ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    authCode = "authoCo";
    ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDiscoveryCallback_001
 * @tc.desc: 1. set packName null
 *              set discoverParam null
 *              set filterOptions null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDiscoveryCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, RegisterDiscoveryCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    std::shared_ptr<DiscoveryCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterDiscoveryCallback(pkgName,
        discoverParam, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().UnRegisterDiscoveryCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().StopDiscovering(pkgName, discoverParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().RegisterDiscoveryCallback(pkgName, discoverParam, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    ret = DeviceManager::GetInstance().UnRegisterDiscoveryCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().StopDiscovering(pkgName, discoverParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartAdvertising_001
 * @tc.desc: 1. set packName null
 *              set discoverParam null
 *              set filterOptions null
 *              set callback null
 *           2. call DeviceManagerImpl::StartAdvertising with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, StartAdvertising_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::map<std::string, std::string> advertiseParam;
    std::shared_ptr<PublishCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().StartAdvertising(pkgName, advertiseParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().StopDiscovering(pkgName, advertiseParam);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().StartAdvertising(pkgName, advertiseParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().StopDiscovering(pkgName, advertiseParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    callback = std::make_shared<DevicePublishCallbackTest>();
    ret = DeviceManager::GetInstance().StartAdvertising(pkgName, advertiseParam, callback);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: BindTarget_001
 * @tc.desc: 1. set packName null
 *              set targetId null
 *              set unbindParam null
 *              set callback null
 *           2. call DeviceManagerImpl::BindTarget with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, BindTarget_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    std::shared_ptr<BindTargetCallback> callback = nullptr;
    std::shared_ptr<UnbindTargetCallback> uncallback = nullptr;
    int32_t ret = DeviceManager::GetInstance().BindTarget(pkgName, targetId, unbindParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().UnbindTarget(pkgName, targetId, unbindParam, uncallback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    targetId.deviceId = "deviceIdTest";
    targetId.brMac = "brMacTest";
    targetId.bleMac = "bleMacTest";
    targetId.wifiIp = "wifiIpTest";
    ret = DeviceManager::GetInstance().BindTarget(pkgName, targetId, unbindParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = DeviceManager::GetInstance().UnbindTarget(pkgName, targetId, unbindParam, uncallback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().BindTarget(pkgName, targetId, unbindParam, callback);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().UnbindTarget(pkgName, targetId, unbindParam, uncallback);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetLocalDeviceInfo_001
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set deviceList null
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceInfo_001, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    DmDeviceInfo info;
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: GetLocalDeviceInfo_002
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set deviceList null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_IPC_SEND_REQUEST_FAILED
 *           3. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceInfo_002, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set extra null
    DmDeviceInfo info;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_IPC_SEND_REQUEST_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetLocalDeviceInfo_003
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set deviceList null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceInfo_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set extra null
    DmDeviceInfo info;
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: GetLocalDeviceInfo_004
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set deviceList null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceInfo_004, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set extra null
    DmDeviceInfo info;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: GetLocalDeviceInfo_005
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set deviceList null
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetLocalDeviceInfo_005, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "";
    // set extra null
    DmDeviceInfo info;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: RegisterDevStateCallback_001
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is DEVICEMANAGER_INVALID_VALUE
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set extra null
    std::string extra = "";
    // set callback nullptr
    std::shared_ptr<DeviceStateCallback> callback = nullptr;
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStateCallback_002
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra = "";
    // set callback not null
    std::shared_ptr<DeviceStateCallback> dsCallback =std::make_shared<DeviceStateCallbackTest>();
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra, dsCallback);
    // 3. check ret is DM_OK
    ASSERT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: RegisterDevStateCallback_003
 * @tc.desc: 1. set packName null
 *              set extra not null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string pkgName = "";
    // set extra null
    std::string extra = "test";
    // set callback nullptr
    std::shared_ptr<DeviceStateCallback> callback = nullptr;
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extra, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStateCallback_004
 * @tc.desc: 1. set packName not null
 *              set extra not null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string pkgName = "com.ohos.test";
    // set extra null
    std::string extra = "test";
    // set callback nullptr
    std::shared_ptr<DeviceStateCallback> callback = nullptr;
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extra, callback);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStateCallback_005
 * @tc.desc: 1. set packName not null
 *              set extra not null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string pkgName = "com.ohos.test";
    // set extra null
    std::string extra = "test1";
    // set callback nullptr
    std::shared_ptr<DeviceStateCallback> callback = nullptr;
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extra, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStateCallback_006
 * @tc.desc: 1. set packName not null
 *              set extra not null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::map<std::string, std::string> extraParam;
    std::shared_ptr<DeviceStateCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extraParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extraParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    callback = std::make_shared<DeviceStateCallbackTest>();
    ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extraParam, callback);
    ASSERT_EQ(ret, DM_OK);
    pkgName = "";
    ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extraParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_001
 * @tc.desc: 1. set packName null
 *           2. call DeviceManagerImpl::UnRegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_002
 * @tc.desc: 1. set packName null
 *           2. call DeviceManagerImpl::UnRegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_003
 * @tc.desc: 1. set packName null
 *           2. call DeviceManagerImpl::UnRegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_004
 * @tc.desc: 1. set packName null
 *           2. call DeviceManagerImpl::UnRegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_005
 * @tc.desc: 1. set packName null
 *           2. call DeviceManagerImpl::UnRegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartDeviceDiscovery_001
 * @tc.desc: 1. set packName null
 *              set subscribeInfo null
 *              set callback null
 *           2. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set subscribeInfo null
    std::string extra = "test";
    DmSubscribeInfo subscribeInfo;
    // set callback null
    std::shared_ptr<DiscoveryCallback> callback = nullptr;
    // 2. call DeviceManagerImpl::StartDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartDeviceDiscovery_002
 * @tc.desc: 1. set packName null
 *              set subscribeInfo null
 *              set callback null
 *           2. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.helloworld";
    std::string extra = "test";
    // set subscribeInfo null
    DmSubscribeInfo subscribeInfo;
    // set callback null
    std::shared_ptr<DiscoveryCallback> callback = nullptr;
    // 2. call DeviceManagerImpl::StartDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartDeviceDiscovery_003
 * @tc.desc: 1. set packName null
 *              set subscribeInfo null
 *              set callback null
 *           2. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           3. check ret is DEVICEMANAGER_INVALID_VALUE
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set subscribeInfo is 0
    DmSubscribeInfo subscribeInfo;
    std::string extra = "test";
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: StartDeviceDiscovery_004
 * @tc.desc: 1. set packName not null
 *              set subscribeInfo null
 *              set callback not null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_004, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    std::string extra = "test";
    // set subscribeInfo null
    DmSubscribeInfo subscribeInfo;
    // set callback not null
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: StartDeviceDiscovery_005
 * @tc.desc: 1. set packName not null
 *              set subscribeInfo null
 *              set callback not null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
 *           3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_005, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    std::string extra = "test";
    // set subscribeInfo null
    DmSubscribeInfo subscribeInfo;
    // set callback not null
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    // 4. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: IsSameAccount_001
 * @tc.desc: 1. set udid or bundleName null
 *           2. call DeviceManagerImpl::IsSameAccount with parameter
 *           3. check ret is false
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, IsSameAccount_001, testing::ext::TestSize.Level0)
{
    std::string udid = "";
    bool ret = DeviceManager::GetInstance().IsSameAccount(udid);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: IsSameAccount_002
 * @tc.desc: 1. set udid and bundleName not null
 *           2. call DeviceManagerImpl::IsSameAccount with parameter
 *           3. check ret is true
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, IsSameAccount_002, testing::ext::TestSize.Level0)
{
    std::string udid = "udidTest";
    std::string pkgName = "com.ohos.sameaccounttest";
    std::shared_ptr<DmInitCallback> initCallback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(pkgName, initCallback);
    bool ret = DeviceManager::GetInstance().IsSameAccount(udid);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: AddPublishCallback_001
 * @tc.desc: 1. set udid and bundleName not null
 *           2. call DeviceManagerImpl::AddPublishCallback with parameter
 *           3. check ret is true
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, AddPublishcCllback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t ret = DeviceManagerImpl::GetInstance().AddPublishCallback(pkgName);
    ASSERT_NE(ret, 0);
    ret = DeviceManagerImpl::GetInstance().RemovePublishCallback(pkgName);
    ASSERT_NE(ret, 0);
}

/**
 * @tc.name: RegisterPinHolderCallback_001
 * @tc.desc: 1. set udid and bundleName not null
 *           2. call DeviceManagerImpl::RegisterPinHolderCallback with parameter
 *           3. check ret is true
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, RegisterPinHolderCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::shared_ptr<PinHolderCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterPinHolderCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "pkgNameTest";
    ret = DeviceManager::GetInstance().RegisterPinHolderCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: CreatePinHolder_001
 * @tc.desc: 1. set udid and bundleName not null
 *           2. call DeviceManagerImpl::CreatePinHolder with parameter
 *           3. check ret is true
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, CreatePinHolder_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload = "payload";
    int32_t ret = DeviceManager::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "com.ohos.test";
    targetId.deviceId = "deviceIdTest";
    targetId.brMac = "brMacTest";
    targetId.bleMac = "bleMacTest";
    targetId.wifiIp = "wifiIpTest";
    pinType = static_cast<DmPinType>(6);
    ret = DeviceManager::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pinType = DmPinType::QR_CODE;
    payload.resize(DM_STRING_LENGTH_MAX * 2);
    ret = DeviceManager::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    payload.resize(10);
    ret = DeviceManager::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DestroyPinHolder_001
 * @tc.desc: 1. set udid and bundleName not null
 *           2. call DeviceManagerImpl::DestroyPinHolder with parameter
 *           3. check ret is true
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, DestoryPinHolder_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload = "payload";
    int32_t ret = DeviceManager::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "com.ohos.test";
    targetId.deviceId = "deviceIdTest";
    targetId.brMac = "brMacTest";
    targetId.bleMac = "bleMacTest";
    targetId.wifiIp = "wifiIpTest";
    pinType = static_cast<DmPinType>(6);
    ret = DeviceManager::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pinType = DmPinType::QR_CODE;
    payload.resize(DM_STRING_LENGTH_MAX * 2);
    ret = DeviceManager::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    payload.resize(10);
    ret = DeviceManager::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DpAclAdd_001
 * @tc.desc: 1. set DpAclAdd and Udid not null
 *           2. call DeviceManagerImpl::DpAclAdd with parameter
 *           3. check ret is true
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, DpAclAdd_001, testing::ext::TestSize.Level0)
{
    int64_t accessControlId = 0;
    std::string udid = "udidTest";
    int32_t bindType = PEER_TO_PEER;
    int32_t ret = DeviceManager::GetInstance().DpAclAdd(accessControlId, udid, bindType);
    ASSERT_EQ(ret, DM_OK);
    bindType = IDENTICAL_ACCOUNT;
    ret = DeviceManager::GetInstance().DpAclAdd(accessControlId, udid, bindType);
    ASSERT_EQ(ret, DM_OK);
    ret = DeviceManager::GetInstance().DpAclAdd(accessControlId, udid, bindType);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetDeviceSecurityLevel_001
 * @tc.desc: 1. set DpAclAdd and Udid not null
 *           2. call DeviceManagerImpl::GetDeviceSecurityLevel with parameter
 *           3. check ret is true
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, GetDeviceSecurityLevel_001, testing::ext::TestSize.Level0)
{
    int32_t securityLevel = 3;
    std::string pkgName = "";
    std::string netWorkId = "";
    int32_t ret = DeviceManager::GetInstance().GetDeviceSecurityLevel(pkgName, netWorkId, securityLevel);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    pkgName = "com.ohos.test";
    ret = DeviceManager::GetInstance().GetDeviceSecurityLevel(pkgName, netWorkId, securityLevel);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    netWorkId = "netWorkIdTest";
    ret = DeviceManager::GetInstance().GetDeviceSecurityLevel(pkgName, netWorkId, securityLevel);
    ASSERT_EQ(ret, DM_OK);
    pkgName = "";
    ret = DeviceManager::GetInstance().GetDeviceSecurityLevel(pkgName, netWorkId, securityLevel);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetAllTrustedDeviceList_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string extra = "extraInfo";
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = DeviceManager::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "p*******lo";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    ret = DeviceManager::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    ret = DeviceManager::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterLocalServiceInfo_001, testing::ext::TestSize.Level0)
{
    DMLocalServiceInfo info;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().RegisterLocalServiceInfo(info);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    ret = DeviceManager::GetInstance().RegisterLocalServiceInfo(info);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterLocalServiceInfo_001, testing::ext::TestSize.Level0)
{
    std::string bundleName = "b*********kl";
    int32_t pinExchangeType = 1;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnRegisterLocalServiceInfo(bundleName, pinExchangeType);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    ret = DeviceManager::GetInstance().UnRegisterLocalServiceInfo(bundleName, pinExchangeType);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UpdateLocalServiceInfo_001, testing::ext::TestSize.Level0)
{
    DMLocalServiceInfo info;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().UpdateLocalServiceInfo(info);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    ret = DeviceManager::GetInstance().UpdateLocalServiceInfo(info);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetLocalServiceInfoByBundleNameAndPinExchangeType_001, testing::ext::TestSize.Level0)
{
    std::string bundleName = "b*********kl";
    int32_t pinExchangeType = 1;
    DMLocalServiceInfo info;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(bundleName,
        pinExchangeType, info);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    ret = DeviceManager::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(bundleName,
        pinExchangeType, info);
    ASSERT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
