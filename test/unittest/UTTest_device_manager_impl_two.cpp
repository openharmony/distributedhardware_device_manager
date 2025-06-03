/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <memory>
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
#include "dm_anonymous.h"

using namespace testing;
using namespace testing::ext;
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
    // set subscribeInfo is 1
    uint16_t subscribeId = 1;
    std::string pkgNameTemp = ComposeStr(packName, 1);
    DeviceManagerImpl::GetInstance().pkgName2SubIdMap_[pkgNameTemp] = 10;
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    // set subscribeInfo is 1
    uint16_t subscribeId = 1;
    std::string pkgNameTemp = ComposeStr(packName, 1);
    DeviceManagerImpl::GetInstance().pkgName2SubIdMap_[pkgNameTemp] = 10;
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
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
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
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
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(packName, subscribeId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_NE(ret, DM_OK);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::PublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().PublishDeviceDiscovery(packName, publishInfo, callback);
    // 4. check ret is DEVICEMANAGER_IPC_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::UnPublishDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::StopDeviceDiscovery with parameter
    int32_t ret = DeviceManager::GetInstance().UnPublishDeviceDiscovery(packName, publishId);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    std::string extra = "{\"status\":1}";
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    std::string extra = "{\"status\":1}";
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
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
    std::string extra = "{\"status\":1}";
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    std::string extra = "{\"status\":1}";
    // set callback null
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>();
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().AuthenticateDevice(packName, authType, dmDeviceInfo, extra, callback);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::AuthenticateDevice with parameter
    int32_t ret = DeviceManager::GetInstance().UnAuthenticateDevice(packName, deviceInfo);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    const std::string param = R"({"test":"extra"})";
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
    const std::string param = R"({"test":"extra"})";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    const std::string param = R"({"test":"extra"})";
    // 2. MOCK IpcClientProxy SendRequest return DM_OK
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is DM_OK
    ASSERT_NE(ret, ERR_DM_TIME_OUT);
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
    const std::string param = R"({"test":"extra"})";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_INIT_FAILED
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    const std::string param = R"({"test":"extra"})";
    // 2. MOCK IpcClientProxy SendRequest return ERR_DM_POINT_NULL
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret= DeviceManager::GetInstance().SetUserOperation(packName, action, param);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::GetUdidByNetworkId with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is DM_OK
    ASSERT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
    // DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(packName, netWorkId, udid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_FAILED));
    // 3. call DeviceManagerImpl::GetUuidByNetworkId with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is DM_OK
    ASSERT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 3. call DeviceManagerImpl::SetUserOperation with parameter
    int32_t ret = DeviceManager::GetInstance().GetUuidByNetworkId(packName, netWorkId, uuid);
    // 4. check ret is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));
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

HWTEST_F(DeviceManagerImplTest, RegisterDevStatusCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string extra = "extra";
    std::shared_ptr<DeviceStatusCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterDevStatusCallback(pkgName, extra, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterDevStatusCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStatusCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    ret = DeviceManager::GetInstance().UnRegisterDevStatusCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    uint64_t tokenId = 1;
    std::string filterOptions = "filterOptions";
    std::shared_ptr<DiscoveryCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(pkgName, tokenId, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_201, testing::ext::TestSize.Level0)
{
    uint64_t tokenId = 1;
    std::string pkgName = "";
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(tokenId, pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    ret = DeviceManager::GetInstance().StopDeviceDiscovery(tokenId, pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StopAuthenticateDevice_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t ret = DeviceManager::GetInstance().StopAuthenticateDevice(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    ret = DeviceManager::GetInstance().StopAuthenticateDevice(pkgName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, VerifyAuthentication_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string authPara = "authPara";
    std::shared_ptr<VerifyAuthCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().VerifyAuthentication(pkgName, authPara, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetFaParam_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    DmAuthParam dmFaParam;
    int32_t ret = DeviceManager::GetInstance().GetFaParam(pkgName, dmFaParam);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string extra = "";
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string extra = "";
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(pkgName, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterUiStateCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterUiStateCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RequestCredential_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string reqJsonStr = "reqJsonStr";
    std::string returnJsonStr = "";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().RequestCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, ImportCredential_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string credentialInfo = "credentialInfo";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().ImportCredential(pkgName, credentialInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, DeleteCredential_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string deleteInfo = "deleteInfo";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(pkgName, deleteInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, RegisterCredentialCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::shared_ptr<CredentialCallback> callback = std::make_shared<CredentialCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    callback = nullptr;
    ret = DeviceManager::GetInstance().RegisterCredentialCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    callback = std::make_shared<CredentialCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    ret = DeviceManager::GetInstance().RegisterCredentialCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, NotifyEvent_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t eventId = 1;
    std::string event = "";
    int32_t ret = DeviceManager::GetInstance().NotifyEvent(pkgName, eventId, event);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    eventId = DM_NOTIFY_EVENT_START;
    ret = DeviceManager::GetInstance().NotifyEvent(pkgName, eventId, event);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    eventId = DM_NOTIFY_EVENT_ONDEVICEREADY;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    ret = DeviceManager::GetInstance().NotifyEvent(pkgName, eventId, event);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, OnDmServiceDied_201, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerImpl::GetInstance().OnDmServiceDied();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, CheckAPIAccessPermission_201, testing::ext::TestSize.Level0)
{
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, CheckNewAPIAccessPermission_201, testing::ext::TestSize.Level0)
{
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnBindDevice_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName_201";
    std::string deviceId = "deviceId201";
    std::string extra = "extra";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, ImportAuthCode_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName_201";
    std::string authCode = "asdsedwe";
    int32_t ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    authCode = "123456";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StartAdvertising_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::map<std::string, std::string> advertiseParam;
    std::shared_ptr<PublishCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().StartAdvertising(pkgName, advertiseParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, CheckAccessToTarget_201, testing::ext::TestSize.Level0)
{
    uint64_t tokenId = 1;
    std::string targetId = "";
    int32_t ret = DeviceManager::GetInstance().CheckAccessToTarget(tokenId, targetId);
    ASSERT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerImplTest, AddDiscoveryCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::shared_ptr<DiscoveryCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().AddDiscoveryCallback(pkgName, discoverParam, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, SetDnPolicy_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::map<std::string, std::string> policy;
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(pkgName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegDevTrustChangeCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::shared_ptr<DevTrustChangeCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegDevTrustChangeCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    callback = std::make_shared<DevTrustChangeCallbackTest>();
    ret = DeviceManager::GetInstance().RegDevTrustChangeCallback(pkgName, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterDeviceScreenStatusCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::shared_ptr<DeviceScreenStatusCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceScreenStatusCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    callback = std::make_shared<DeviceScreenStatusCallbackTest>();
    ret = DeviceManager::GetInstance().RegisterDeviceScreenStatusCallback(pkgName, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterDeviceScreenStatusCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceScreenStatusCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    ret = DeviceManager::GetInstance().UnRegisterDeviceScreenStatusCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetNetworkIdByUdid_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string udid = "";
    std::string networkId = "";
    int32_t ret = DeviceManager::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterCredentialAuthStatusCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::shared_ptr<CredentialAuthStatusCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialAuthStatusCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialAuthStatusCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialAuthStatusCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    DmCommonNotifyEvent dmCommonNotifyEvent = DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE;
    pkgName = "";
    DeviceManagerImpl::GetInstance().SyncCallbackToService(dmCommonNotifyEvent, pkgName);

    pkgName = "pkgName";
    dmCommonNotifyEvent = DmCommonNotifyEvent::MIN;
    DeviceManagerImpl::GetInstance().SyncCallbackToService(dmCommonNotifyEvent, pkgName);

    dmCommonNotifyEvent = DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    DeviceManagerImpl::GetInstance().SyncCallbackToService(dmCommonNotifyEvent, pkgName);
}

HWTEST_F(DeviceManagerImplTest, RegisterSinkBindCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::shared_ptr<BindTargetCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterSinkBindCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    callback = std::make_shared<BindTargetCallbackTest>();
    ret = DeviceManager::GetInstance().RegisterSinkBindCallback(pkgName, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterSinkBindCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t ret = DeviceManager::GetInstance().UnRegisterSinkBindCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    pkgName = "pkgName";
    ret = DeviceManager::GetInstance().UnRegisterSinkBindCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceProfileInfoList_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    DmDeviceProfileInfoFilterOptions filterOptions;
    std::shared_ptr<GetDeviceProfileInfoListCallback> callback =
        std::make_shared<GetDeviceProfileInfoListCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterGetDeviceProfileInfoListCallback(_, _))
        .WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterGetDeviceProfileInfoListCallback(_, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    ret = DeviceManager::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceProfileInfoList_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    DmDeviceProfileInfoFilterOptions filterOptions;
    std::shared_ptr<GetDeviceProfileInfoListCallback> callback =
        std::make_shared<GetDeviceProfileInfoListCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterGetDeviceProfileInfoListCallback(_, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceIconInfo_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    DmDeviceIconInfoFilterOptions filterOptions;
    std::shared_ptr<GetDeviceIconInfoCallback> callback = std::make_shared<GetDeviceIconInfoCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterGetDeviceIconInfoCallback(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetDeviceIconInfo(pkgName, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterGetDeviceIconInfoCallback(_, _, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    ret = DeviceManager::GetInstance().GetDeviceIconInfo(pkgName, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceIconInfo_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    DmDeviceIconInfoFilterOptions filterOptions;
    std::shared_ptr<GetDeviceIconInfoCallback> callback = std::make_shared<GetDeviceIconInfoCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterGetDeviceIconInfoCallback(_, _, _))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().GetDeviceIconInfo(pkgName, filterOptions, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StopAuthenticateDevice_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().StopAuthenticateDevice(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnBindDevice_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName_201";
    std::string deviceId = "deviceId201";
    std::string extra = "extra";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnBindDevice_203, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName_201";
    std::string deviceId = "deviceId201";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().UnBindDevice(pkgName, deviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterDiscoveryCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName_2036";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().RegisterDiscoveryCallback(pkgName, discoverParam, filterOptions,
        callback);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, RegisterDiscoveryCallback_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName_2053";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().RegisterDiscoveryCallback(pkgName, discoverParam, filterOptions,
        callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterDiscoveryCallback_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string pkgNameTemp = ComposeStr(pkgName, 0);
    DeviceManagerImpl::GetInstance().pkgName2SubIdMap_[pkgNameTemp] = 10;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnRegisterDiscoveryCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterDiscoveryCallback_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string pkgNameTemp = ComposeStr(pkgName, 0);
    DeviceManagerImpl::GetInstance().pkgName2SubIdMap_[pkgNameTemp] = 10;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().UnRegisterDiscoveryCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StopAdvertising_201, testing::ext::TestSize.Level0)
{
    std::string packName = "pkgName";
    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_PUBLISH_ID] = "10";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INPUT_PARA_INVALID));
    int32_t ret = DeviceManager::GetInstance().StopAdvertising(packName, advertiseParam);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StopAdvertising_202, testing::ext::TestSize.Level0)
{
    std::string packName = "pkgName";
    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_PUBLISH_ID] = "10";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().StopAdvertising(packName, advertiseParam);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, CheckIsSameAccount_201, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INPUT_PARA_INVALID));
    bool ret = DeviceManager::GetInstance().CheckIsSameAccount(caller, callee);
    ASSERT_FALSE(ret);
}

HWTEST_F(DeviceManagerImplTest, CheckIsSameAccount_202, testing::ext::TestSize.Level0)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    bool ret = DeviceManager::GetInstance().CheckIsSameAccount(caller, callee);
    ASSERT_FALSE(ret);
}

HWTEST_F(DeviceManagerImplTest, SetDnPolicy_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName_202";
    std::map<std::string, std::string> policy;
    policy["policy01"] = "101";
    policy["policy02"] = "102";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INPUT_PARA_INVALID));
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(pkgName, policy);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, SetDnPolicy_203, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName_202";
    std::map<std::string, std::string> policy;
    policy["policy01"] = "101";
    policy["policy02"] = "102";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(pkgName, policy);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceScreenStatus_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::string networkId = "";
    int32_t screenStatus = 1;
    int32_t ret = DeviceManager::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceScreenStatus_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string networkId = "networkId";
    int32_t screenStatus = 1;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceScreenStatus_203, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string networkId = "networkId";
    int32_t screenStatus = 1;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetNetworkIdByUdid_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string udid = "udid";
    std::string networkId = "networkId";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(testing::AtLeast(1)).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    ASSERT_EQ(ret, DM_OK);

    std::map<DmCommonNotifyEvent, std::set<std::string>> callbackMap;
    DeviceManagerImpl::GetInstance().SyncCallbacksToService(callbackMap);
    std::set<std::string> strSet;
    callbackMap[DmCommonNotifyEvent::REG_DEVICE_STATE] = strSet;
    strSet.insert("pkgName02");
    callbackMap[DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE] = strSet;
    DeviceManagerImpl::GetInstance().SyncCallbacksToService(callbackMap);
}

HWTEST_F(DeviceManagerImplTest, PutDeviceProfileInfoList_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> deviceProfileInfoList;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, PutDeviceProfileInfoList_202, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> deviceProfileInfoList;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetLocalDisplayDeviceName_201, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t maxNameLength = 1;
    std::string displayName = "displayName";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetLocalDisplayDeviceName(pkgName, maxNameLength, displayName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    ret = DeviceManager::GetInstance().GetLocalDisplayDeviceName(pkgName, maxNameLength, displayName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetLocalDeviceName_201, testing::ext::TestSize.Level0)
{
    std::string deviceName = "deviceName";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceName(deviceName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    ret = DeviceManager::GetInstance().GetLocalDeviceName(deviceName);
    ASSERT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
