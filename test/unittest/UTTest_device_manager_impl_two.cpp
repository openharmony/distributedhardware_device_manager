/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "ipc_start_discovery_req.h"
#include "ipc_stop_discovery_req.h"
#include "ipc_publish_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::string extra = "";
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
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    deviceInfo.networkId[0] = '1';
    deviceInfo.networkId[1] = '2';
    deviceInfo.networkId[2] = '\0';
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    deviceInfo.networkId[0] = '1';
    deviceInfo.networkId[1] = '2';
    deviceInfo.networkId[2] = '\0';
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    deviceInfo.networkId[0] = '1';
    deviceInfo.networkId[1] = '2';
    deviceInfo.networkId[2] = '\0';
    std::shared_ptr<AuthenticateCallback> callback = nullptr;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 3. call DeviceManagerImpl::GetUdidByNetworkId with parameter
    ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    // DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 3. call DeviceManagerImpl::GetUuidByNetworkId with parameter
    ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(2).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(packName, callback);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(2).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(packName);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(2).WillOnce(testing::Return(DM_OK));
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
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(2).WillOnce(testing::Return(DM_OK));
    DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    // 7. Get checkMap from pkgName
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 8 check checkMap null
    ASSERT_EQ(checkMap, nullptr);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
    std::shared_ptr<MockIpcClientProxy> mockInstance = std::make_shared<MockIpcClientProxy>();
    std::shared_ptr<IpcClientProxy> ipcClientProxy = DeviceManagerImpl::GetInstance().ipcClientProxy_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = mockInstance;
    EXPECT_CALL(*mockInstance, SendRequest(testing::_, testing::_, testing::_))
                .Times(2).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(pkgName, callback);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxy;
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
 *           6. call DeviceManager UnRegisterDeviceManagerFaCallback with parameter
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
} // namespace
} // namespace DistributedHardware
} // namespace OHOS