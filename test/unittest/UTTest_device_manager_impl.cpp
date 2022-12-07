/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "device_manager_notify.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_get_dmfaparam_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_req.h"
#include "ipc_rsp.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_skeleton.h"
#include "ipc_start_discovery_req.h"
#include "ipc_stop_discovery_req.h"
#include "ipc_publish_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_verify_authenticate_req.h"
#include "securec.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerImplTest::SetUp()
{
}

void DeviceManagerImplTest::TearDown()
{
}

void DeviceManagerImplTest::SetUpTestCase()
{
}

void DeviceManagerImplTest::TearDownTestCase()
{
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
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_DISCOVERY_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    std::string extra= "";
    // set subscribeInfo null
    DmSubscribeInfo subscribeInfo;
    // set callback not null
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    // 2. InitDeviceManager return DM_OK
    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, initcallback);
    // 3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
    ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    // 4. check ret is ERR_DM_DISCOVERY_FAILED
    ASSERT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: StopDeviceDiscovery_101
 * @tc.desc: 1. set packName not null
 *              set subscribeId is 0
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_DISCOVERY_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_101, testing::ext::TestSize.Level0)
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
    ASSERT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: PublishDeviceDiscovery_101
 * @tc.desc: 1. set packName not null
 *              set publishInfo null
 *              set callback not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_PUBLISH_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set publishInfo null
    DmPublishInfo publishInfo;
    // set callback not null
    std::shared_ptr<PublishCallback> callback = std::make_shared<DevicePublishCallbackTest>();
    // 2. InitDeviceManager return DM_OK
    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, initcallback);
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is ERR_DM_PUBLISH_FAILED
    ASSERT_EQ(ret, ERR_DM_PUBLISH_FAILED);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: UnPublishDeviceDiscovery_101
 * @tc.desc: 1. set packName not null
 *              set publishId is 0
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_PUBLISH_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, UnPublishDeviceDiscovery_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set subscribeInfo is 0
    int32_t publishId = 0;
    // 2. InitDeviceManager return DM_OK
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_PUBLISH_FAILED
    ASSERT_EQ(ret, ERR_DM_PUBLISH_FAILED);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
 *           4. check ret is ERR_DM_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnAuthenticateDevice_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    DmDeviceInfo deviceInfo;
    deviceInfo.deviceId[0] = '1';
    deviceInfo.deviceId[1] = '2';
    deviceInfo.deviceId[2] = '\0';
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. InitDeviceManager return DM_OK
    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, initcallback);
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is ERR_DM_FAILED
    ASSERT_EQ(ret, ERR_DM_FAILED);
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
 *           2. MOCK IpcClientProxy Init return DM_OK
 *           3. call DeviceManagerImpl::InitDeviceManager with parameter
 *           4. check ret is DM_OK
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
    // 2. MOCK IpcClientProxy Init return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, Init(testing::_)).Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::InitDeviceManager with parameter
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, Init(testing::_)).Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::InitDeviceManager with parameter
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 4. check ret is ERR_DM_INIT_FAILED
    ASSERT_EQ(ret, ERR_DM_INIT_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, UnInit(testing::_)).Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is ERR_DM_FAILED
    ASSERT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, UnInit(testing::_)).Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, UnInit(testing::_)).Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is ERR_DM_FAILED
    ASSERT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, UnInit(testing::_)).Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is ERR_DM_FAILED
    ASSERT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    // set extra null
    DmDeviceInfo info;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_IPC_SEND_REQUEST_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::string extra= "";
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
    std::string extra= "";
    // set callback not null
    std::shared_ptr<DeviceStateCallback> callback =std::make_shared<DeviceStateCallbackTest>();
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra, callback);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
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
    std::string extra= "test";
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
    std::string extra= "test";
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
    std::string extra= "test1";
    // set callback nullptr
    std::shared_ptr<DeviceStateCallback> callback = nullptr;
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extra, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::string extra= "test";
    // set subscribeInfo null
    DmSubscribeInfo subscribeInfo;
    // set callback not null
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    std::string extra= "test";
    // set subscribeInfo null
    DmSubscribeInfo subscribeInfo;
    // set callback not null
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, callback);
    // 4. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: StopDeviceDiscovery_001
 * @tc.desc: 1. set packName null
 *              set subscribeId is 0
 *           2. call DeviceManagerImpl::StopDeviceDiscovery with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set subscribeInfo is 0
    uint16_t subscribeId = 0;
    // 2. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StopDeviceDiscovery_002
 * @tc.desc: 1. set packName not null
 *                     set subscribeId is 0
 *                  2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
 *                  2. call DeviceManagerImpl::StopDeviceDiscovery with parameter
 *                  3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set subscribeInfo is 0
    uint16_t subscribeId = 0;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: StopDeviceDiscovery_003
 * @tc.desc: 1. set packName not null
 *              set subscribeId is 0
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set subscribeInfo is 0
    uint16_t subscribeId = 0;
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: StopDeviceDiscovery_004
 * @tc.desc: 1. set packName not null
 *              set subscribeId is 0
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_004, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set subscribeInfo is 0
    uint16_t subscribeId = 0;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: StopDeviceDiscovery_005
 * @tc.desc: 1. set packName not null
 *              set subscribeId is 0
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
 *           3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_005, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set subscribeInfo is 0
    uint16_t subscribeId = 0;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: PublishDeviceDiscovery_001
 * @tc.desc: 1. set packName null
 *              set subscribeInfo null
 *              set callback null
 *           2. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set publishInfo null
    DmPublishInfo publishInfo;
    // set callback null
    std::shared_ptr<PublishCallback> callback = nullptr;
    // 2. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: PublishDeviceDiscovery_002
 * @tc.desc: 1. set packName null
 *              set subscribeInfo null
 *              set callback null
 *           2. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.helloworld";
    // set publishInfo null
    DmPublishInfo publishInfo;
    // set callback null
    std::shared_ptr<PublishCallback> callback = nullptr;
    // 2. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: PublishDeviceDiscovery_003
 * @tc.desc: 1. set packName null
 *              set publishInfo null
 *              set callback null
 *           2. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
 *           3. check ret is DEVICEMANAGER_INVALID_VALUE
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set publishInfo is 0
    DmPublishInfo publishInfo;
    std::shared_ptr<PublishCallback> callback = std::make_shared<DevicePublishCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: PublishDeviceDiscovery_004
 * @tc.desc: 1. set packName not null
 *              set publishInfo null
 *              set callback not null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery_004, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set publishInfo null
    DmPublishInfo publishInfo;
    // set callback not null
    std::shared_ptr<PublishCallback> callback = std::make_shared<DevicePublishCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name:PublishDeviceDiscovery_005
 * @tc.desc: 1. set packName not null
 *              set subscribeInfo null
 *              set callback not null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
 *           3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, PublishDeviceDiscovery_005, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set publishInfo null
    DmPublishInfo publishInfo;
    // set callback not null
    std::shared_ptr<PublishCallback> callback = std::make_shared<DevicePublishCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnPublishDeviceDiscovery_001
 * @tc.desc: 1. set packName null
 *              set publishId is 0
 *           2. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, UnPublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set publishId is 0
    int32_t publishId = 0;
    // 2. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnPublishDeviceDiscovery_002
 * @tc.desc: 1. set packName not null
 *              set publishId is 0
 *           2. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, UnPublishDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set subscribeInfo is 0
    int32_t publishId = 0;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnPublishDeviceDiscovery_003
 * @tc.desc: 1. set packName not null
 *              set publishId is 0
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, UnPublishDeviceDiscovery_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set subscribeInfo is 0
    int32_t publishId = 0;
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnPublishDeviceDiscovery_004
 * @tc.desc: 1. set packName not null
 *              set publishId is 0
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, UnPublishDeviceDiscovery_004, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set publishId is 0
    int32_t publishId = 0;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnPublishDeviceDiscovery_005
 * @tc.desc: 1. set packName not null
 *              set publishId is 0
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
 *           3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(DeviceManagerImplTest, UnPublishDeviceDiscovery_005, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set publishId is 0
    int32_t publishId = 0;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: AuthenticateDevice_001
 * @tc.desc: 1. set packName = null
 *              set dmDeviceInfo = null
 *              set dmAppImageInfo = null
 *              set extra = null
 *              set callback = nullptr
 *           2. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           3. check ret is DEVICEMANAGER_INVALID_VALUE
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, AuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    // 1. set packName = null
    std::string packName = "";
    int32_t authType = 1;
    // set dmDeviceInfo = null
    DmDeviceInfo dmDeviceInfo;
    // set extra = null
    std::string extra= "";
    // set callback = nullptr
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 3. check ret is DEVICEMANAGER_INVALID_VALUE
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: AuthenticateDevice_002
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, AuthenticateDevice_002, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    int32_t authType = 1;
    // set dmDeviceInfo null
    DmDeviceInfo dmDeviceInfo;
    // set extra null
    std::string extra = "";
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: AuthenticateDevice_003
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, AuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set dmDeviceInfo null
    int32_t authType = 1;
    // set dmAppImageInfo null
    DmDeviceInfo dmDeviceInfo;
    // set extra null
    std::string extra = "";
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: AuthenticateDevice_004
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, AuthenticateDevice_004, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set dmDeviceInfo null
    int32_t authType = 1;
    // set dmAppImageInfo null
    DmDeviceInfo dmDeviceInfo;
    // set extra null
    std::string extra = "";
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: AuthenticateDevice_005
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, AuthenticateDevice_005, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set dmDeviceInfo null
    int32_t authType = 1;
    // set dmAppImageInfo null
    DmDeviceInfo dmDeviceInfo;
    // set extra null
    std::string extra = "";
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnAuthenticateDevice_001
 * @tc.desc: 1. set packName = null
 *              set dmDeviceInfo = null
 *              set dmAppImageInfo = null
 *              set extra = null
 *              set callback = nullptr
 *           2. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           3. check ret is DEVICEMANAGER_INVALID_VALUE
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    // 1. set packName = null
    std::string packName = "";
    DmDeviceInfo deviceInfo;
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 3. check ret is DEVICEMANAGER_INVALID_VALUE
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnAuthenticateDevice_002
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnAuthenticateDevice_002, testing::ext::TestSize.Level0)
{
    // 1. set packName = null
    std::string packName = "com.ohos.helloworld";
    DmDeviceInfo deviceInfo;
    deviceInfo.deviceId[0] = '\0';
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 3. check ret is DEVICEMANAGER_INVALID_VALUE
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnAuthenticateDevice_003
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnAuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    DmDeviceInfo deviceInfo;
    deviceInfo.deviceId[0] = '1';
    deviceInfo.deviceId[1] = '2';
    deviceInfo.deviceId[2] = '\0';
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnAuthenticateDevice_004
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnAuthenticateDevice_004, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set dmDeviceInfo null
    DmDeviceInfo deviceInfo;
    deviceInfo.deviceId[0] = '1';
    deviceInfo.deviceId[1] = '2';
    deviceInfo.deviceId[2] = '\0';
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnAuthenticateDevice_005
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnAuthenticateDevice_005, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set dmDeviceInfo null
    DmDeviceInfo deviceInfo;
    deviceInfo.deviceId[0] = '1';
    deviceInfo.deviceId[1] = '2';
    deviceInfo.deviceId[2] = '\0';
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetFaParam_001
 * @tc.desc: 1. set packName = null
 *              set dmDeviceInfo = null
 *              set dmAppImageInfo = null
 *              set extra = null
 *              set callback = nullptr
 *           2. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetFaParam_001, testing::ext::TestSize.Level0)
{
    // 1. set packName = null
    std::string packName = "";
    DmAuthParam dmFaParam;
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().GetFaParam(packName, dmFaParam);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetFaParam_002
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetFaParam_002, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set callback null
    DmAuthParam dmFaParam;
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().GetFaParam(packName, dmFaParam);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetFaParam_003
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetFaParam_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set callback null
    DmAuthParam dmFaParam;
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().GetFaParam(packName, dmFaParam);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetFaParam_004
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetFaParam_004, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set dmDeviceInfo null
    DmAuthParam dmFaParam;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().GetFaParam(packName, dmFaParam);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetFaParam_005
 * @tc.desc: 1. set packName not null
 *              set dmDeviceInfo null
 *              set dmAppImageInfo null
 *              set extra null
 *              set callback null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
 *           3. call DeviceManagerImpl::AuthenticateDevice with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetFaParam_005, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.helloworld";
    // set dmDeviceInfo null
    DmAuthParam dmFaParam;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().GetFaParam(packName, dmFaParam);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: SetUserOperation_001
 * @tc.desc: 1. set packName null
 *              set action null
 *           2. call DeviceManagerImpl::SetUserOperation with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, SetUserOperation_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set authParam null
    int32_t action = 0;
    const std::string param = "extra";
    // 2. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetUserOperation_002
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, SetUserOperation_002, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set authParam null
    int32_t action = 0;
    const std::string param = "extra";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: SetUserOperation_003
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, SetUserOperation_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set authParam null
    int32_t action = 0;
    const std::string param = "extra";
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: SetUserOperation_004
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, SetUserOperation_004, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set authParam null
    int32_t action = 0;
    const std::string param = "extra";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: SetUserOperation_005
 * @tc.desc: 1. set packName not null
 *               set action null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, SetUserOperation_005, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set authParam null
    int32_t action = 0;
    const std::string param = "extra";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetUdidByNetworkId_001
 * @tc.desc: 1. set packName null
 *              set action null
 *           2. call DeviceManagerImpl::SetUserOperation with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUdidByNetworkId_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set authParam null
    std::string netWorkId = "111";
    std::string udid = "222";
    // 2. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetUdidByNetworkId_002
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUdidByNetworkId_002, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set authParam null
    std::string netWorkId = "111";
    std::string udid = "222";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetUdidByNetworkId_003
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUdidByNetworkId_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set authParam null
    std::string netWorkId = "111";
    std::string udid = "222";
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetUdidByNetworkId_004
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUdidByNetworkId_004, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set authParam null
    std::string netWorkId = "111";
    std::string udid = "222";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetUdidByNetworkId_005
 * @tc.desc: 1. set packName not null
 *                     set action null
 *                  2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
 *                  3. call DeviceManagerImpl::SetUserOperation with parameter
 *                  4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUdidByNetworkId_005, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set authParam null
    std::string netWorkId = "111";
    std::string udid = "222";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetUuidByNetworkId_001
 * @tc.desc: 1. set packName null
 *              set action null
 *           2. call DeviceManagerImpl::SetUserOperation with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUuidByNetworkId_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set authParam null
    std::string netWorkId = "111";
    std::string uuid = "222";
    // 2. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetUuidByNetworkId_002
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUuidByNetworkId_002, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set authParam null
    std::string netWorkId = "111";
    std::string uuid = "222";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetUuidByNetworkId_003
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUuidByNetworkId_003, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.test";
    // set authParam null
    std::string netWorkId = "111";
    std::string uuid = "222";
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetUuidByNetworkId_004
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUuidByNetworkId_004, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set authParam null
    std::string netWorkId = "111";
    std::string uuid = "222";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: GetUuidByNetworkId_005
 * @tc.desc: 1. set packName not null
 *                     set action null
 *                  2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
 *                  3. call DeviceManagerImpl::SetUserOperation with parameter
 *                  4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, GetUuidByNetworkId_005, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set authParam null
    std::string netWorkId = "111";
    std::string uuid = "222";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: RegisterDeviceManagerFaCallback_001
 * @tc.desc: 1. set packName not null
 *              set action null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
 *           3. call DeviceManagerImpl::SetUserOperation with parameter
 *           4. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDeviceManagerFaCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set callback null
    std::shared_ptr<DeviceManagerUiCallback> callback = nullptr;
    // 2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packName, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDeviceManagerFaCallback_002
 * @tc.desc: 1. set packName not null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDeviceManagerFaCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set callback null
    std::shared_ptr<DeviceManagerFaCallbackTest> callback = std::make_shared<DeviceManagerFaCallbackTest>();
    // 2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packName, callback);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterDeviceManagerFaCallback_003
 * @tc.desc: 1. set packName not null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDeviceManagerFaCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set callback null
    std::shared_ptr<DeviceManagerFaCallbackTest> callback = nullptr;
    // 2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packName, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDeviceManagerFaCallback_004
 * @tc.desc: 1. set packName not null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDeviceManagerFaCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set callback null
    std::shared_ptr<DeviceManagerFaCallbackTest> callback = std::make_shared<DeviceManagerFaCallbackTest>();
    // 2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packName, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDeviceManagerFaCallback_005
 * @tc.desc: 1. set packName not null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDeviceManagerFaCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName ("");
    // set callback null
    std::shared_ptr<DeviceManagerFaCallbackTest> callback = nullptr;
    // 2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packName, callback);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDeviceManagerFaCallback_001
 * @tc.desc: 1. set packName null
 *           2. call DeviceManagerImpl::UnRegisterDeviceManagerFaCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDeviceManagerFaCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // 2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(packName);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDeviceManagerFaCallback_002
 * @tc.desc: 1. set packName not null
 *           2. call DeviceManagerImpl::UnRegisterDeviceManagerFaCallback with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDeviceManagerFaCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // 2. call DeviceManagerImpl::RegisterDeviceManagerFaCallback with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(packName);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterDeviceManagerFaCallback_003
 * @tc.desc: 1. set packName not null
 *              set callback not null
 *           2. call DeviceManager RegisterDeviceManagerFaCallback with parameter
 *           3. check ret is DM_OK
 *           4. set checkMap null
 *           5. Get checkMap from DeviceManagerNotify
 *           6. call DeviceManager UnRegisterDeviceManagerFaCallback with parameter
 *           7. Get checkMap from pkgName
 *           8. check checkMap null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDeviceManagerFaCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
    std::shared_ptr<DeviceManagerFaCallbackTest> callback = std::make_shared<DeviceManagerFaCallbackTest>();
    // 2. call DeviceManager RegisterDeviceManagerFaCallback with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(pkgName, callback);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    // 4. set checkMap null
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    // 5. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManager UnRegisterDeviceManagerFaCallback with parameter
    DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    // 7. Get checkMap from pkgName
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 8 check checkMap null
    ASSERT_EQ(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeviceManagerFaCallback_004
 * @tc.desc: 1. set packName not null
 *              set callback not null
 *           2. call DeviceManager RegisterDeviceManagerFaCallback with parameter
 *           3. check ret is DM_OK
 *           4. set checkMap null
 *           5. Get checkMap from DeviceManagerNotify
 *           6. call DeviceManager UnRegisterDeviceManagerFaCallback with parameter
 *           7. Get checkMap from pkgName
 *           8. check checkMap null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDeviceManagerFaCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
    std::shared_ptr<DeviceManagerFaCallbackTest> callback = std::make_shared<DeviceManagerFaCallbackTest>();
    // 2. call DeviceManager RegisterDeviceManagerFaCallback with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(pkgName, callback);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    // 4. set checkMap null
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    // 5. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set unRegisterPkgNamr different from pkgName
    std::string unRegisterPkgName = "com.ohos.test1";
    // 7. call DeviceManager UnRegisterDeviceManagerFaCallback with unRegisterPkgName
    DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(unRegisterPkgName);
    // 7. Get checkMap from pkgName
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeviceManagerFaCallback_005
 * @tc.desc: 1. set packName not null
 *           2. Set checkMap null
 *           3. Get checkMap from DeviceManagerNotify
 *           4. check checkMap not null
 *           5. Set unRegisterPkgName is different from register pkgName
 *           6. call DeviceManager UnRegisterDevStateCallback with parameter
 *           7. Get checkMap from DeviceManagerNotify
 *           8. check checkMap not null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDeviceManagerFaCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // 2. Set checkMap null
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    // 3. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 4. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 5. Set unRegisterPkgName is different from register pkgName
    std::string unRegisterPkgName = "com.ohos.test1";
    // 6. call DeviceManager UnRegisterDeviceManagerFaCallback with parameter
    DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(unRegisterPkgName);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 8. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDevStateCallback_006
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_006, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string pkgName = "";
    // set extra null
    std::string extra= "test";
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extra);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID;
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStateCallback_007
 * @tc.desc: 1. set packName not null
 *              set extra null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_007, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set extra null
    std::string extra= "";
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStateCallback_008
 * @tc.desc: 1. set packName null
 *              set extra not null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_008, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra= "test";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: RegisterDevStateCallback_009
 * @tc.desc: 1. set packName not null
 *              set extra not null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_009, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra= "test";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: RegisterDevStateCallback_010
 * @tc.desc: 1. set packName not null
 *              set extra not null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_010, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra= "test";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnRegisterDevStateCallback_006
 * @tc.desc: 1. set packName null
 *           2. call DeviceManagerImpl::UnRegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_006, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set extra null
    std::string extra= "";
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName, extra);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_007
 * @tc.desc: 1. set packName not null
 *           2. call DeviceManagerImpl::UnRegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_007, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "";
    // set extra null
    std::string extra= "test";
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName, extra);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_008
 * @tc.desc: 1. set packName null
 *              set extra not null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_008, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra= "test";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    //  2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName, extra);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnRegisterDevStateCallback_009
 * @tc.desc: 1. set packName null
 *              set extra not null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_009, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra= "test";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName, extra);
    // 3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnRegisterDevStateCallback_010
 * @tc.desc: 1. set packName not null
 *              set extra not null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterDevStateCallback with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_010, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    // set extra null
    std::string extra= "test";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 2. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(packName, extra);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: RequestCredential_001
 * @tc.desc: 1. set packName null
 *              set reqJsonStr null
 *           2. call DeviceManagerImpl::RequestCredential with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RequestCredential_001, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string reqJsonStr;
    std::string returnJsonStr;
    int32_t ret = DeviceManager::GetInstance().RequestCredential(packName, reqJsonStr,
                                                                returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RequestCredential_002
 * @tc.desc: 1. set packName not null
 *              set reqJsonStr not null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_IPC_SEND_REQUEST_FAILED
 *           3. call DeviceManagerImpl::RequestCredential with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RequestCredential_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string reqJsonStr = R"({"version":"1.0.0.1","userId":"123"})";
    std::string returnJsonStr;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().RequestCredential(packName, reqJsonStr,
                                                                returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: RequestCredential_003
 * @tc.desc: 1. set packName not null
 *              set reqJsonStr not null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::RequestCredential with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RequestCredential_003, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string reqJsonStr = R"({"version":"1.0.0.1","userId":"123"})";
    std::string returnJsonStr;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().RequestCredential(packName, reqJsonStr,
                                                                returnJsonStr);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: RequestCredential_004
 * @tc.desc: 1. set packName not null
 *              set reqJsonStr not null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::RequestCredential with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RequestCredential_004, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string reqJsonStr = R"({"version":"1.0.0.1","userId":"123"})";
    std::string returnJsonStr;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().RequestCredential(packName, reqJsonStr,
                                                                returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: ImportCredential_001
 * @tc.desc: 1. set packName null
 *              set reqJsonStr null
 *           2. call DeviceManagerImpl::ImportCredential with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, ImportCredential_001, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string credentialInfo;
    int32_t ret = DeviceManager::GetInstance().ImportCredential(packName, credentialInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ImportCredential_002
 * @tc.desc: 1. set packName not null
 *              set credentialInfo not null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_IPC_SEND_REQUEST_FAILED
 *           3. call DeviceManagerImpl::ImportCredential with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, ImportCredential_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string credentialInfo = R"({"processType":1,"authType":1,"userId":"123",
    "credentialData": [{ "credentialType": 1, "credentialId": "104",
    "authCode":"10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92",
    "serverPk":"","pkInfoSignature":"","pkInfo":"","peerDeviceId":"" }]})";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().ImportCredential(packName, credentialInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: ImportCredential_003
 * @tc.desc: 1. set packName not null
 *              set credentialInfo not null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::ImportCredential with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, ImportCredential_003, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string credentialInfo = R"({"processType":1,"authType":1,"userId":"123",
    "credentialData": [{ "credentialType": 1, "credentialId": "104",
    "authCode":"10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92",
    "serverPk":"","pkInfoSignature":"","pkInfo":"","peerDeviceId":"" }]})";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().ImportCredential(packName, credentialInfo);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: ImportCredential_004
 * @tc.desc: 1. set packName not null
 *              set credentialInfo not null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::ImportCredential with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, ImportCredential_004, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string credentialInfo = R"({"processType":1,"authType":1,"userId":"123",
    "credentialData": [{ "credentialType": 1, "credentialId": "104",
    "authCode":"10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92",
    "serverPk":"","pkInfoSignature":"","pkInfo":"","peerDeviceId":"" }]})";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().ImportCredential(packName, credentialInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: DeleteCredential_001
 * @tc.desc: 1. set packName null
 *              set deleteInfo null
 *           2. call DeviceManagerImpl::DeleteCredential with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, DeleteCredential_001, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::string deleteInfo;
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(packName, deleteInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DeleteCredential_002
 * @tc.desc: 1. set packName not null
 *              set deleteInfo not null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_IPC_SEND_REQUEST_FAILED
 *           3. call DeviceManagerImpl::DeleteCredential with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, DeleteCredential_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string deleteInfo = R"({"processType":1,"authType":1,"userId":"123"})";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(packName, deleteInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: DeleteCredential_003
 * @tc.desc: 1. set packName not null
 *              set deleteInfo not null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::DeleteCredential with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, DeleteCredential_003, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string deleteInfo = R"({"processType":1,"authType":1,"userId":"123"})";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(packName, deleteInfo);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: DeleteCredential_004
 * @tc.desc: 1. set packName not null
 *              set credentialInfo not null
 *           2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
 *           3. call DeviceManagerImpl::DeleteCredential with parameter
 *           4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, DeleteCredential_004, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string deleteInfo = R"({"processType":1,"authType":1,"userId":"123"})";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(packName, deleteInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: RegisterCredentialCallback_001
 * @tc.desc: 1. set packName null
 *              set callback null
 *           3. call DeviceManagerImpl::RegisterCredentialCallback with parameter
 *           4. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterCredentialCallback_001, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::shared_ptr<CredentialCallbackTest> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialCallback(packName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterCredentialCallback_002
 * @tc.desc: 1. set packName not null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterCredentialCallback with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterCredentialCallback_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::shared_ptr<CredentialCallbackTest> callback = std::make_shared<CredentialCallbackTest>();
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialCallback(packName, callback);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: RegisterCredentialCallback_003
 * @tc.desc: 1. set packName not null
 *              set callback null
 *           2. call DeviceManagerImpl::RegisterCredentialCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterCredentialCallback_003, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::shared_ptr<CredentialCallbackTest> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialCallback(packName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterCredentialCallback_004
 * @tc.desc: 1. set packName null
 *              set callback not null
 *           2. call DeviceManagerImpl::RegisterCredentialCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RegisterCredentialCallback_004, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::shared_ptr<CredentialCallbackTest> callback = std::make_shared<CredentialCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialCallback(packName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterCredentialCallback_001
 * @tc.desc: 1. set packName null
 *           2. call DeviceManagerImpl::UnRegisterCredentialCallback with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialCallback_001, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(packName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterCredentialCallback_002
 * @tc.desc: 1. set packName not null
 *           2. call DeviceManagerImpl::UnRegisterCredentialCallback with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialCallback_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(packName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnRegisterCredentialCallback_003
 * @tc.desc: 1. set packName not null
 *              set callback null
 *           2. call DeviceManagerImpl::UnRegisterCredentialCallback with parameter
 *           3. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set packName null
    std::string packName = "com.ohos.test";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(packName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: UnRegisterCredentialCallback_004
 * @tc.desc: 1. set packName not null
 *           2. call DeviceManagerImpl::UnRegisterCredentialCallback with parameter
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialCallback_004, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(packName);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: OnDmServiceDied_001
 * @tc.desc: 1. mock IpcClientProxy
 *           2. call DeviceManagerImpl::OnDmServiceDied
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, OnDmServiceDied_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    int32_t ret = DeviceManagerImpl::GetInstance().OnDmServiceDied();
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: OnDmServiceDied_001
 * @tc.desc: 1. mock IpcClientProxy
 *           2. call DeviceManagerImpl::OnDmServiceDied
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, OnDmServiceDied_002, testing::ext::TestSize.Level0)
{
    // 1. mock IpcClientProxy
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, OnDmServiceDied()).Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 2. call DeviceManagerImpl::OnDmServiceDied
    int32_t ret = DeviceManagerImpl::GetInstance().OnDmServiceDied();
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: NotifyEvent_001
 * @tc.desc: 1. mock IpcClientProxy
 *           2. call DeviceManagerImpl::NotifyEvent
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, NotifyEvent_001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    int32_t eventId = DM_NOTIFY_EVENT_ONDEVICEREADY;
    std::string event = R"({"extra": {"deviceId": "123"})";
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().NotifyEvent(packName, eventId, event);
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: NotifyEvent_002
 * @tc.desc: 1. mock IpcClientProxy
 *           2. call DeviceManagerImpl::NotifyEvent
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, NotifyEvent_002, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    int32_t eventId = DM_NOTIFY_EVENT_ONDEVICEREADY;
    std::string event = R"({"extra": {"deviceId": "123"})";
    int32_t ret = DeviceManager::GetInstance().NotifyEvent(packName, eventId, event);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: NotifyEvent_003
 * @tc.desc: 1. mock IpcClientProxy
 *           2. call DeviceManagerImpl::NotifyEvent
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, NotifyEvent_003, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    int32_t eventId = DM_NOTIFY_EVENT_START;
    std::string event = R"({"extra": {"deviceId": "123"})";
    int32_t ret = DeviceManager::GetInstance().NotifyEvent(packName, eventId, event);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: NotifyEvent_004
 * @tc.desc: 1. mock IpcClientProxy
 *           2. call DeviceManagerImpl::NotifyEvent
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, NotifyEvent_004, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    int32_t eventId = DM_NOTIFY_EVENT_BUTT;
    std::string event = R"({"extra": {"deviceId": "123"})";
    int32_t ret = DeviceManager::GetInstance().NotifyEvent(packName, eventId, event);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS