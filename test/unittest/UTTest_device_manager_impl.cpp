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
 * @tc.desc: 1. call DeviceManagerImpl::InitDeviceManager with packName = null, dmInitCallback = nullprt
 *           2. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager_001, testing::ext::TestSize.Level0)
{
    // 1. call DeviceManagerImpl::InitDeviceManager with packName = null, dmInitCallback = nullprt
    std::string packName = "";
    std::shared_ptr<DmInitCallback> dmInitCallback = nullptr;
    int32_t ret= DeviceManager::GetInstance().InitDeviceManager(packName, dmInitCallback);
    // 2. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
    ASSERT_EQ(ret, ERR_DM_INPUT_PARAMETER_EMPTY);
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
    int32_t ret= DeviceManager::GetInstance().InitDeviceManager(packName, callback);
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
    int32_t ret= DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 4. check ret is ERR_DM_INIT_FAILED
    ASSERT_EQ(ret, ERR_DM_INIT_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}

/**
 * @tc.name: InitDeviceManager_004
 * @tc.desc: 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
 *           2. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager_004, testing::ext::TestSize.Level0)
{
    // 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
    std::string packName = "com.ohos.test";
    std::shared_ptr<DmInitCallbackTest> callback = nullptr;
    int32_t ret= DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 2. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
    ASSERT_EQ(ret, ERR_DM_INPUT_PARAMETER_EMPTY);
}

/**
 * @tc.name: InitDeviceManager_005
 * @tc.desc: 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
 *           2. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, InitDeviceManager_005, testing::ext::TestSize.Level0)
{
    // 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
    std::string packName = "";
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret= DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 2. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
    ASSERT_EQ(ret, ERR_DM_INPUT_PARAMETER_EMPTY);
}

/**
 * @tc.name: UnInitDeviceManager_001
 * @tc.desc: 1. call DeviceManagerImpl::InitDeviceManager with packName not null, dmInitCallback = nullprt
 *           2. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, UnInitDeviceManager_001, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "";
    // 2. call DeviceManagerImpl::InitDeviceManager with parameter
    int32_t ret= DeviceManager::GetInstance().UnInitDeviceManager(packName);
    // 3. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
    ASSERT_EQ(ret, ERR_DM_INPUT_PARAMETER_EMPTY);
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
    int32_t ret= DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
    int32_t ret= DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
    int32_t ret= DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
    int32_t ret= DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
 *           3. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
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
    int32_t ret= DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
    ASSERT_EQ(ret, ERR_DM_INPUT_PARAMETER_EMPTY);
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
    int32_t ret= DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
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
    int32_t ret= DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
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
    int32_t ret= DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
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
 *           3. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
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
    int32_t ret= DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    // 3. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
    ASSERT_EQ(ret, ERR_DM_INPUT_PARAMETER_EMPTY);
}


/**
 * @tc.name: GetLocalDeviceInfo_001
 * @tc.desc: 1. set packName null
 *              set extra null
 *              set deviceList null
 *           2. call DeviceManagerImpl::GetTrustedDeviceList with parameter
 *           3. check ret is ERR_DM_INPUT_PARAMETER_EMPTY
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
    int32_t ret= DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
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
    int32_t ret= DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
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
    int32_t ret= DeviceManager::GetInstance().GetLocalDeviceInfo(packName, info);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS