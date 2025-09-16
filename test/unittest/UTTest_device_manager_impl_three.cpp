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
#include "softbus_error_code.h"
#include "ipc_publish_service_info_rsp.h"
using namespace testing;

namespace OHOS {
namespace DistributedHardware {
namespace {
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
    std::string reqJsonStr = R"(
    {
        "version":"1.0.0.1",
        "userId":"4269DC28B639681698809A67EDAD08E39F207900038F91EFF95DD042FE2874E4"
    }
    )";
    std::string returnJsonStr;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().RequestCredential(packName, reqJsonStr,
                                                                returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    std::string reqJsonStr = R"(
    {
        "version":"1.0.0.1",
        "userId":"4269DC28B639681698809A67EDAD08E39F207900038F91EFF95DD042FE2874E4",
        "CREDENTIAL_TYPE":"MINE",
    }
    )";
    std::string returnJsonStr;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().RequestCredential(packName, reqJsonStr,
                                                                returnJsonStr);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
    std::string reqJsonStr = R"(
    {
        "version":"1.0.0.1",
        "userId":"4269DC28B639681698809A67EDAD08E39F207900038F91EFF95DD042FE2874E4"
    }
    )";
    std::string returnJsonStr;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().RequestCredential(packName, reqJsonStr,
                                                                returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    std::string credentialInfo = R"(
    {
        "processType": 1,
        "authType": 1,
        "userId": "123",
        "credentialData":
        [
            {
                "credentialType": 1,
                "credentialId": "104",
                "authCode": "10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92",
                "serverPk": "",
                "pkInfoSignature": "",
                "pkInfo": "",
                "peerDeviceId": ""
            }
        ]
    }
    )";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().ImportCredential(packName, credentialInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    std::string credentialInfo = R"(
    {
        "processType": 1,
        "authType": 1,
        "userId": "123",
        "credentialData":
        [
            {
                "credentialType": 1,
                "credentialId": "104",
                "authCode": "10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92",
                "serverPk": "",
                "pkInfoSignature": "",
                "pkInfo": "",
                "peerDeviceId": ""
            }
        ]
    }
    )";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().ImportCredential(packName, credentialInfo);
    ASSERT_EQ(ret, DM_OK);
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
    std::string credentialInfo = R"(
    {
        "processType": 1,
        "authType": 1,
        "userId": "123",
        "credentialData":
        [
            {
                "credentialType": 1,
                "credentialId": "104",
                "authCode": "10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92",
                "serverPk": "",
                "pkInfoSignature": "",
                "pkInfo": "",
                "peerDeviceId": ""
            }
        ]
    }
    )";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().ImportCredential(packName, credentialInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(packName, deleteInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(packName, deleteInfo);
    ASSERT_EQ(ret, DM_OK);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(packName, deleteInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    std::shared_ptr<DmInitCallback> initCallback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, initCallback);
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialCallback(packName, callback);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(packName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_INIT_FAILED));
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(packName);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
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
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(packName);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
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
    int32_t ret = DeviceManagerImpl::GetInstance().OnDmServiceDied();
    ASSERT_EQ(ret, DM_OK);
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
    EXPECT_CALL(*ipcClientProxyMock_, OnDmServiceDied()).Times(1).WillOnce(testing::Return(ERR_DM_POINT_NULL));
    // 2. call DeviceManagerImpl::OnDmServiceDied
    int32_t ret = DeviceManagerImpl::GetInstance().OnDmServiceDied();
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, ERR_DM_FAILED);
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
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().NotifyEvent(packName, eventId, event);
    ASSERT_EQ(ret, DM_OK);
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

/**
 * @tc.name: CheckAPIAccessPermission_001
 * @tc.desc: 1. InitDeviceManager
 *           2. call DeviceManagerImpl::CheckAPIAccessPermission
 *           3. check ret is DM_OK
 *           4. UnInitDeviceManager
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, CheckAPIAccessPermission_001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::shared_ptr<CredentialCallbackTest> callback = std::make_shared<CredentialCallbackTest>();
    std::shared_ptr<DmInitCallback> initCallback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, initCallback);
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: RegisterDevStatusCallback_001
 * @tc.desc: 1. InitDeviceManager
 *           2. call DeviceManagerImpl::RegisterDevStatusCallback
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStatusCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string extra;
    std::shared_ptr<DeviceStatusCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterDevStatusCallback(pkgName, extra, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStatusCallback_002
 * @tc.desc: 1. InitDeviceManager
 *           2. call DeviceManagerImpl::RegisterDevStatusCallback
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStatusCallback_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string extra;
    std::shared_ptr<DeviceStatusCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterDevStatusCallback(packName, extra, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStatusCallback_003
 * @tc.desc: 1. InitDeviceManager
 *           2. call DeviceManagerImpl::RegisterDevStatusCallback
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, RegisterDevStatusCallback_003, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string extra;
    std::shared_ptr<DeviceStatusCallback> callback = std::make_shared<DeviceStatusCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegisterDevStatusCallback(packName, extra, callback);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterDevStatusCallback_001
 * @tc.desc: 1. InitDeviceManager
 *           2. call DeviceManagerImpl::UnRegisterDevStatusCallback
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStatusCallback_001, testing::ext::TestSize.Level0)
{
    std::string packName;
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStatusCallback(packName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStatusCallback_002
 * @tc.desc: 1. InitDeviceManager
 *           2. call DeviceManagerImpl::UnRegisterDevStatusCallback
 *           3. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerImplTest, UnRegisterDevStatusCallback_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStatusCallback(packName);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: StartDeviceDiscovery_104
 * @tc.desc: 1. set packName null
 *              set subscribeId 0
 *              set filterOptions null
 *              set callback not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_104, testing::ext::TestSize.Level0)
{
    std::string packName;
    uint16_t subscribeId = 0;
    std::string filterOptions;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    DmDeviceBasicInfo deviceBasicInfo;
    callback->OnDeviceFound(subscribeId, deviceBasicInfo);
    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, initcallback);
    ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeId, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: StartDeviceDiscovery_105
 * @tc.desc: 1. set packName not null
 *              set subscribeId 0
 *              set filterOptions null
 *              set callback not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           4. check ret is ERR_DM_INPUT_PARA_INVALID
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_105, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    uint16_t subscribeId = 0;
    std::string filterOptions;
    std::shared_ptr<DiscoveryCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeId, filterOptions, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartDeviceDiscovery_106
 * @tc.desc: 1. set packName not null
 *              set subscribeId 0
 *              set filterOptions not null
 *              set callback not null
 *           2. InitDeviceManager return DM_OK
 *           3. call DeviceManagerImpl::StartDeviceDiscovery with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery_106, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    uint16_t subscribeId = -1;
    std::string filterOptions = "filterOptions";
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DeviceDiscoveryCallbackTest>();
    std::shared_ptr<DmInitCallback> initcallback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, initcallback);
    ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeId, filterOptions, callback);
    ASSERT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

/**
 * @tc.name: RequestCredential_101
 * @tc.desc: 1. set packName not null
 *              set reqJsonStr not null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::RequestCredential with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, RequestCredential_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string returnJsonStr;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().RequestCredential(packName, returnJsonStr);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: CheckCredential_101
 * @tc.desc: 1. set packName not null
 *              set credentialInfo not null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::ImportCredential with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, CheckCredential_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string credentialInfo = "{\n}";
    std::string returnJsonStr;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().CheckCredential(packName, credentialInfo, returnJsonStr);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ImportCredential_101
 * @tc.desc: 1. set packName not null
 *              set credentialInfo not null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::ImportCredential with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, ImportCredential_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string credentialInfo = R"(
    {
        "processType": 1,
        "authType": 1,
        "userId": "123",
        "credentialData":
        [
            {
                "credentialType": 1,
                "credentialId": "104",
                "authCode": "10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92",
                "serverPk": "",
                "pkInfoSignature": "",
                "pkInfo": "",
                "peerDeviceId": ""
            }
        ]
    }
    )";
    std::string returnJsonStr;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().ImportCredential(packName, credentialInfo, returnJsonStr);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DeleteCredential_101
 * @tc.desc: 1. set packName not null
 *              set deleteInfo not null
 *           2. MOCK IpcClientProxy SendRequest return DM_OK
 *           3. call DeviceManagerImpl::DeleteCredential with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerImplTest, DeleteCredential_101, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    std::string credentialInfo = R"({"isDeleteAll":true})";
    std::string returnJsonStr;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(packName, credentialInfo, returnJsonStr);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
    ASSERT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerImplTest, VerifyAuthentication001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string authPara;
    std::shared_ptr<VerifyAuthCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().VerifyAuthentication(pkgName, authPara, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetFaParam001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DmAuthParam dmFaParam;
    int32_t ret = DeviceManager::GetInstance().GetFaParam(pkgName, dmFaParam);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string extra;
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(pkgName, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string extra;
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStateCallback(pkgName, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RequestCredential001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string reqJsonStr;
    std::string returnJsonStr;
    int32_t ret = DeviceManager::GetInstance().RequestCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RequestCredential002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string reqJsonStr;
    std::string returnJsonStr;
    int32_t ret = DeviceManager::GetInstance().RequestCredential(pkgName, reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RequestCredential003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string reqJsonStr = "reqJsonStr";
    std::string returnJsonStr;
    int32_t ret = DeviceManager::GetInstance().RequestCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, ImportCredential001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string credentialInfo;
    int32_t ret = DeviceManager::GetInstance().ImportCredential(pkgName, credentialInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, ImportCredential002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string credentialInfo;
    int32_t ret = DeviceManager::GetInstance().ImportCredential(pkgName, credentialInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, ImportCredential003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string credentialInfo = "credentialInfo";
    int32_t ret = DeviceManager::GetInstance().ImportCredential(pkgName, credentialInfo);
    EXPECT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, DeleteCredential001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deleteInfo;
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(pkgName, deleteInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, DeleteCredential002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string deleteInfo;
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(pkgName, deleteInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, DeleteCredential003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string deleteInfo = "deleteInfo";
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(pkgName, deleteInfo);
    EXPECT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterCredentialCallback001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::shared_ptr<CredentialCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterCredentialCallback002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::shared_ptr<CredentialCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialCallback001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialCallback002, testing::ext::TestSize.Level0)
{
    DeviceManagerImpl::GetInstance().ipcClientProxy_->ipcClientManager_ = nullptr;
    std::string pkgName = "pkgName";
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialCallback(pkgName);
    EXPECT_NE(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerImpl::GetInstance().ipcClientProxy_->ipcClientManager_ = std::make_shared<IpcClientManager>();
}

void SetSetDnPolicyPermission()
{
    const int32_t permsNum = 1;
    const int32_t indexZero = 0;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.ACCESS_SERVICE_DM";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "collaboration_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

HWTEST_F(DeviceManagerImplTest, SetDnPolicy001, testing::ext::TestSize.Level0)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(packName, policy);
    bool bRet = false;
    if (ret == DM_OK || ret == ERR_DM_UNSUPPORTED_METHOD) {
        bRet = true;
    }
    ASSERT_EQ(bRet, true);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, SetDnPolicy002, testing::ext::TestSize.Level0)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, SetDnPolicy003, testing::ext::TestSize.Level0)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, SetDnPolicy004, testing::ext::TestSize.Level0)
{
    SetSetDnPolicyPermission();
    std::string packName;
    std::map<std::string, std::string> policy;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, SetDnPolicy005, testing::ext::TestSize.Level0)
{
    SetSetDnPolicyPermission();
    std::string packName;
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, SetDnPolicy006, testing::ext::TestSize.Level0)
{
    SetSetDnPolicyPermission();
    std::string packName = "com.ohos.test";
    std::map<std::string, std::string> policy;
    policy[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = "100";
    policy[PARAM_KEY_POLICY_TIME_OUT] = "10";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(packName, policy);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_301, testing::ext::TestSize.Level0)
{
    std::string packName;
    uint64_t tokenId = 123;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(tokenId, packName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery_302, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.test";
    uint64_t tokenId = 123;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManagerImpl::GetInstance().InitDeviceManager(packName, callback);
    DeviceManagerImpl::GetInstance().StopDeviceDiscovery(tokenId, packName);
    int32_t ret = DeviceManagerImpl::GetInstance().StopDeviceDiscovery(tokenId, packName);
    ASSERT_NE(ret, DM_OK);
    DeviceManagerImpl::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, RegisterUiStateCallback_301, testing::ext::TestSize.Level0)
{
    std::string packName;
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterUiStateCallback(packName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterUiStateCallback_301, testing::ext::TestSize.Level0)
{
    std::string packName;
    DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterUiStateCallback(packName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StopAdvertising_301, testing::ext::TestSize.Level0)
{
    std::string packName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManager::GetInstance().StopAdvertising(packName, advertiseParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, CheckAccessToTarget_301, testing::ext::TestSize.Level0)
{
    uint64_t tokenId = 123;
    std::string targetId;
    int32_t ret = DeviceManager::GetInstance().CheckAccessToTarget(tokenId, targetId);
    ASSERT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DeviceManagerImplTest, RegDevTrustChangeCallback_301, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::shared_ptr<DevTrustChangeCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegDevTrustChangeCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegDevTrustChangeCallback_302, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::shared_ptr<DevTrustChangeCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegDevTrustChangeCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegDevTrustChangeCallback_303, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::shared_ptr<DevTrustChangeCallback> callback = std::make_shared<DevTrustChangeCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegDevTrustChangeCallback(pkgName, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegDevTrustChangeCallback_304, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::shared_ptr<DevTrustChangeCallback> callback = std::make_shared<DevTrustChangeCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegDevTrustChangeCallback(pkgName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetErrCode_301, testing::ext::TestSize.Level0)
{
    int32_t errCode = ERR_DM_TIME_OUT;
    int32_t ret = DeviceManager::GetInstance().GetErrCode(errCode);
    ASSERT_EQ(ret, 96929745);
}

HWTEST_F(DeviceManagerImplTest, RegisterDeviceScreenStatusCallback_001, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::shared_ptr<DeviceScreenStatusCallbackTest> callback = std::make_shared<DeviceScreenStatusCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceScreenStatusCallback(packName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterDeviceScreenStatusCallback_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.screenStatustest01";
    std::shared_ptr<DeviceScreenStatusCallbackTest> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceScreenStatusCallback(packName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterDeviceScreenStatusCallback_003, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.screenStatustest02";
    std::shared_ptr<DeviceScreenStatusCallbackTest> callback = std::make_shared<DeviceScreenStatusCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegisterDeviceScreenStatusCallback(packName, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterDeviceScreenStatusCallback_001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.screenStatustest03";
    int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceScreenStatusCallback(packName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterDeviceScreenStatusCallback_002, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    int32_t ret = DeviceManager::GetInstance().UnRegisterDeviceScreenStatusCallback(packName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceScreenStatus_001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.screenStatustest04";
    std::string networkId = "networkIdTest";
    int32_t screenStatus = -1;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().GetDeviceScreenStatus(packName, networkId, screenStatus);
    ASSERT_NE(ret, ERR_DM_TIME_OUT);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceScreenStatus_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.screenStatustest05";
    std::string networkId = "";
    int32_t screenStatus = -1;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().GetDeviceScreenStatus(packName, networkId, screenStatus);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceScreenStatus_003, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::string networkId = "networkIdTest";
    int32_t screenStatus = -1;
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().GetDeviceScreenStatus(packName, networkId, screenStatus);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, RegisterCredentialAuthStatusCallback_001, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::shared_ptr<CandidateRestrictStatusCallbackTest> callback =
        std::make_shared<CandidateRestrictStatusCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialAuthStatusCallback(packName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterCredentialAuthStatusCallback_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.candidateRestrict01";
    std::shared_ptr<CandidateRestrictStatusCallbackTest> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialAuthStatusCallback(packName, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterCredentialAuthStatusCallback_003, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.scandidateRestrict02";
    std::shared_ptr<CandidateRestrictStatusCallbackTest> callback =
        std::make_shared<CandidateRestrictStatusCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegisterCredentialAuthStatusCallback(packName, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialAuthStatusCallback_001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.candidateRestrict03";
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialAuthStatusCallback(packName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterCredentialAuthStatusCallback_002, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    int32_t ret = DeviceManager::GetInstance().UnRegisterCredentialAuthStatusCallback(packName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterAuthenticationType_001, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::map<std::string, std::string> authParam;
    int32_t ret = DeviceManager::GetInstance().RegisterAuthenticationType(packName, authParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterAuthenticationType_002, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.RegisterAuthenticationType";
    std::map<std::string, std::string> authParam;
    int32_t ret = DeviceManager::GetInstance().RegisterAuthenticationType(packName, authParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterAuthenticationType_003, testing::ext::TestSize.Level0)
{
    std::string packName = "";
    std::map<std::string, std::string> authParam;
    authParam[DM_AUTHENTICATION_TYPE] = "0";
    int32_t ret = DeviceManager::GetInstance().RegisterAuthenticationType(packName, authParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterAuthenticationType_004, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.RegisterAuthenticationType";
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    std::map<std::string, std::string> authParam;
    authParam[DM_AUTHENTICATION_TYPE] = "0";
    int32_t ret = DeviceManager::GetInstance().RegisterAuthenticationType(packName, authParam);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterPinHolderCallback_001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.UnRegisterPinHolderCallback";
    std::shared_ptr<DmInitCallback> callback = std::make_shared<DmInitCallbackTest>();
    DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    int32_t ret = DeviceManager::GetInstance().UnRegisterPinHolderCallback(packName);
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}

HWTEST_F(DeviceManagerImplTest, ImportAuthCode_301, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string authCode;
    authCode.append(1025, 'a');
    int32_t ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, ImportAuthCode_302, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.ImportAuthCode";
    std::string authCode = "950800";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManager::GetInstance().ImportAuthCode(pkgName, authCode);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, ExportAuthCode_301, testing::ext::TestSize.Level0)
{
    std::string authCode;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().ExportAuthCode(authCode);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}
HWTEST_F(DeviceManagerImplTest, StartServiceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.discoveryServiceId = 12345;
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().StartServiceDiscovery(pkgName, discParam, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartServiceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.discoveryServiceId = 12345;
    std::shared_ptr<ServiceDiscoveryCallback> callback = nullptr;
    int32_t ret = DeviceManager::GetInstance().StartServiceDiscovery(pkgName, discParam, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartServiceDiscovery_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "";
    discParam.discoveryServiceId = 12345;
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().StartServiceDiscovery(pkgName, discParam, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartServiceDiscovery_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.discoveryServiceId = 0;
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().StartServiceDiscovery(pkgName, discParam, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartServiceDiscovery_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.discoveryServiceId = 12345;
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().StartServiceDiscovery(pkgName, discParam, callback);
    ret = (ret == ERR_DM_IPC_SEND_REQUEST_FAILED) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopServiceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t discoveryServiceId = 12345;
    int32_t ret = DeviceManager::GetInstance().StopServiceDiscovery(pkgName, discoveryServiceId);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopServiceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t discoveryServiceId = 0;
    int32_t ret = DeviceManager::GetInstance().StopServiceDiscovery(pkgName, discoveryServiceId);`
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopServiceDiscovery_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t discoveryServiceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().StopServiceDiscovery(pkgName, discoveryServiceId);
    ret = (ret == ERR_DM_IPC_SEND_REQUEST_FAILED) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, BindServiceTarget_InvalidInput_101, testing::ext::TestSize.Level0)
{
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    std::shared_ptr<BindTargetCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().BindServiceTarget("", targetId, bindParam, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_NOT_INIT);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, BindServiceTarget_InvalidInput_102, testing::ext::TestSize.Level0)
{
    PeerTargetId invalidTargetId;
    std::map<std::string, std::string> bindParam;
    std::shared_ptr<BindTargetCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().BindServiceTarget("valid_pkg", invalidTargetId, bindParam, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_NOT_INIT);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, BindServiceTarget_IpcRequestFailed_101, testing::ext::TestSize.Level0)
{
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    std::shared_ptr<BindTargetCallback> callback = nullptr;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().BindServiceTarget("valid_pkg", targetId, bindParam, callback);
    ret = (ret == ERR_DM_IPC_SEND_REQUEST_FAILED) || (ret == ERR_DM_NOT_INIT);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, BindServiceTarget_IpcResponseError_101, testing::ext::TestSize.Level0)
{
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    std::shared_ptr<BindTargetCallback> callback = nullptr;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManagerImpl::GetInstance().BindServiceTarget("valid_pkg", targetId, bindParam, callback);
    ASSERT_EQ(ret, rsp->GetErrCode());
}

HWTEST_F(DeviceManagerImplTest, BindServiceTarget_Success_101, testing::ext::TestSize.Level0)
{
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    std::shared_ptr<BindTargetCallback> callback = nullptr;
    auto rsp = std::make_shared<IpcRsp>();
    rsp->SetErrCode(DM_OK);
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManagerImpl::GetInstance().BindServiceTarget("valid_pkg", targetId, bindParam, callback);
    ret = (ret == DM_OK) || (ret == ERR_DM_NOT_INIT);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_InvalidInput_101, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget("", 12345);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_NOT_INIT);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_InvalidInput_102, testing::ext::TestSize.Level0)
{
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget("valid_pkg", 0);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_NOT_INIT);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_IpcFailed_101, testing::ext::TestSize.Level0)
{
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget("valid_pkg", 12345);
    ret = (ret == ERR_DM_IPC_SEND_REQUEST_FAILED) || (ret == ERR_DM_NOT_INIT);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_IpcError_101, testing::ext::TestSize.Level0)
{
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget("valid_pkg", 12345);
    ASSERT_EQ(ret, rsp->GetErrCode());
}

HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_Success_101, testing::ext::TestSize.Level0)
{
    auto rsp = std::make_shared<IpcRsp>();
    rsp->SetErrCode(DM_OK);
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget("valid_pkg", 12345);
    ret = (ret == DM_OK) || (ret == ERR_DM_NOT_INIT);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    PublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = nullptr;
    int64_t serviceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, publishServiceParam,
        callback, serviceId);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    PublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    int64_t serviceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, publishServiceParam,
        callback, serviceId);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    PublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = nullptr;
    int64_t serviceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, publishServiceParam,
        callback, serviceId);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    PublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    int64_t serviceId = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, publishServiceParam,
        callback, serviceId);
    ret = (ret == ERR_DM_IPC_SEND_REQUEST_FAILED) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    PublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    int64_t serviceId = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                std::shared_ptr<IpcPublishServiceInfoRsp> publishRsp =
                    std::static_pointer_cast<IpcPublishServiceInfoRsp>(rsp);
                publishRsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, publishServiceParam,
        callback, serviceId);
    ret = (ret == ERR_DM_FAILED) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    PublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    int64_t serviceId = 0;
    int64_t expectedServiceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                std::shared_ptr<IpcPublishServiceInfoRsp> publishRsp =
                    std::static_pointer_cast<IpcPublishServiceInfoRsp>(rsp);
                publishRsp->SetErrCode(DM_OK);
                publishRsp->SetServiceId(12345);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, publishServiceParam,
        callback, serviceId);
    if (ret == DM_OK){
        ASSERT_EQ(serviceId, expectedServiceId);
    }
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_007, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    PublishServiceParam publishServiceParam;
    publishServiceParam.serviceInfo.serviceType = "test_service";
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    int64_t serviceId = 0;
    int64_t expectedServiceId = 67890;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                std::shared_ptr<IpcPublishServiceInfoRsp> publishRsp =
                    std::static_pointer_cast<IpcPublishServiceInfoRsp>(rsp);
                publishRsp->SetErrCode(DM_OK);
                publishRsp->SetServiceId(67890);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, publishServiceParam,
        callback, serviceId);
    if (ret == DM_OK){
        ASSERT_EQ(serviceId, expectedServiceId);
    }
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_008, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    PublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    int64_t serviceId = 9999;
    int64_t expectedServiceId = 54321;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                std::shared_ptr<IpcPublishServiceInfoRsp> publishRsp =
                    std::static_pointer_cast<IpcPublishServiceInfoRsp>(rsp);
                publishRsp->SetErrCode(DM_OK);
                publishRsp->SetServiceId(54321);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, publishServiceParam,
        callback, serviceId);
    if (ret == DM_OK){
        ASSERT_EQ(serviceId, expectedServiceId);
    }
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_009, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    PublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    int64_t serviceId = 0;
    int64_t expectedServiceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                std::shared_ptr<IpcPublishServiceInfoRsp> publishRsp =
                    std::static_pointer_cast<IpcPublishServiceInfoRsp>(rsp);
                publishRsp->SetErrCode(DM_OK);
                publishRsp->SetServiceId(12345);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, publishServiceParam,
        callback, serviceId);
    if (ret == DM_OK){
        ASSERT_EQ(serviceId, expectedServiceId);
    }
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int64_t serviceId = 0;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int64_t serviceId = 12345;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 12345;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterServiceStateCallback(testing::_, testing::_))
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 12345;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterServiceStateCallback(testing::_, testing::_))
        .WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ret = (ret == ERR_DM_FAILED) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 12345;
    std::shared_ptr<ServiceInfoStateCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int64_t serviceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int64_t serviceId = 12345;
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 12345;
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServiceStateCallback(testing::_))
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 12345;
    int32_t expectedError = ERR_DM_FAILED;
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServiceStateCallback(testing::_))
        .WillOnce(testing::Return(ERR_DM_FAILED));

    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ret = (ret == ERR_DM_FAILED) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_001, testing::ext::TestSize.Level0)
{
    int64_t serviceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(serviceId);
    ret = (ret == ERR_DM_INPUT_PARA_INVALID) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_002, testing::ext::TestSize.Level0)
{
    int64_t serviceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(serviceId);
    ret = (ret == ERR_DM_IPC_SEND_REQUEST_FAILED) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_003, testing::ext::TestSize.Level0)
{
    int64_t serviceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(serviceId);
    ret = (ret == ERR_DM_FAILED) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_004, testing::ext::TestSize.Level0)
{
    int64_t serviceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(serviceId);
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_005, testing::ext::TestSize.Level0)
{
    int64_t serviceId = 67890;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(serviceId);
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_006, testing::ext::TestSize.Level0)
{
    int64_t serviceId = -12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(serviceId);
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_007, testing::ext::TestSize.Level0)
{
    int64_t serviceId = INT64_MAX;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(serviceId);
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_008, testing::ext::TestSize.Level0)
{
    int64_t serviceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(serviceId);
    ret = (ret == DM_OK) || (ret == ERR_DM_UNSUPPORTED_METHOD);
    ASSERT_EQ(ret, true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
