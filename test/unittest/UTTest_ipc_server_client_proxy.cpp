/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "UTTest_ipc_server_client_proxy.h"

#include <unistd.h>

#include "dm_device_info.h"
#include "ipc_remote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ipc_client_manager.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_rsp.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_verify_auth_result_req.h"
#include "ipc_notify_dmfa_result_req.h"
#include "ipc_notify_credential_req.h"
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
void IpcServerClientProxyTest::SetUp()
{
}

void IpcServerClientProxyTest::TearDown()
{
}

void IpcServerClientProxyTest::SetUpTestCase()
{
}

void IpcServerClientProxyTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: SendCmd_001
 * @tc.desc: 1. set cmdCode not null
 *           2. set remoteObject nullptr
 *           3. call IpcServerClientProxy SendCmd
 *           4. check ret is DEVICEMANAGER_NULLPTR
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcServerClientProxyTest, SendCmd_001, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = 1;
    // 2. set remoteObject nullptr
    sptr<IRemoteObject> remoteObject = nullptr;
    // 3. call IpcServerClientProxy SendCmd
    auto instance = new IpcServerClientProxy(remoteObject);
    int ret = instance->SendCmd(cmdCode, nullptr, nullptr);
    // 4. check ret is DEVICEMANAGER_NULLPTR
    ASSERT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: SendCmd_002
 * @tc.desc: 1. set cmdCode not null
 *              set pkgName not null
 *              set action not null
 *           2. set remoteObject not nullptr
 *              set req not null
 *              set rsp not null
 *           3. call IpcServerClientProxy SendCmd with parameter
 *           4. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcServerClientProxyTest, SendCmd_002, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = SERVER_DEVICE_STATE_NOTIFY;
    // set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set action not null
    int deviceState = 1;
    DmDeviceInfo deviceInfo;
    // 2. set remoteObject not nullptr
    sptr<IpcClientStub> remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, remoteObject);
    std::shared_ptr<IpcNotifyDeviceStateReq> req = std::make_shared<IpcNotifyDeviceStateReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    // set req not null
    req->SetPkgName(pkgName);
    // set rsp not null
    req->SetDeviceState(deviceState);
    req->SetDeviceInfo(deviceInfo);
    // 3. call IpcServerClientProxy SendCmd with parameter
    int ret = 0;
    std::shared_ptr<IpcServerListener> ipcServerListener = std::make_shared<IpcServerListener>();
    ret = ipcServerListener->SendRequest(cmdCode, req, rsp);
    // 4. check ret is not ERR_DM_FAILED
    ASSERT_NE(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: SendCmd_003
 * @tc.desc: 1. set cmdCode not null
 *              set pkgName not null
 *              set action not null
 *           2. set remoteObject not nullptr
 *              set req not null
 *              set rsp not null
 *           3. call IpcServerClientProxy SendCmd with parameter
 *           4. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcServerClientProxyTest, SendCmd_003, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = SERVER_DEVICE_FOUND;
    // set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set action not null
    uint16_t subscribeId = 1;
    DmDeviceInfo dmDeviceInfo;
    // 2. set remoteObject not nullptr
    sptr<IpcClientStub> remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, remoteObject);
    std::shared_ptr<IpcNotifyDeviceFoundReq> req = std::make_shared<IpcNotifyDeviceFoundReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    // set req not null
    req->SetPkgName(pkgName);
    // set rsp not null
    req->SetSubscribeId(subscribeId);
    req->SetDeviceInfo(dmDeviceInfo);
    // 3. call IpcServerClientProxy SendCmd with parameter
    int ret = 0;
    std::shared_ptr<IpcServerListener> ipcServerListener = std::make_shared<IpcServerListener>();
    ret = ipcServerListener->SendRequest(cmdCode, req, rsp);
    // 4. check ret is not ERR_DM_FAILED
    ASSERT_NE(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: SendCmd_004
 * @tc.desc: 1. set cmdCode not null
 *              set pkgName not null
 *              set action not null
 *           2. set remoteObject not nullptr
 *              set req not null
 *              set rsp not null
 *           3. call IpcServerClientProxy SendCmd with parameter
 *           4. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcServerClientProxyTest, SendCmd_004, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = SERVER_DISCOVER_FINISH;
    // set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set action not null
    uint16_t subscribeId = 1;
    int32_t result = 1;
    // 2. set remoteObject not nullptr
    sptr<IpcClientStub> remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, remoteObject);
    std::shared_ptr<IpcNotifyDiscoverResultReq> req = std::make_shared<IpcNotifyDiscoverResultReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    // set req not null
    req->SetPkgName(pkgName);
    // set rsp not null
    req->SetSubscribeId(subscribeId);
    req->SetResult(result);
    // 3. call IpcServerClientProxy SendCmd with parameter
    int ret = 0;
    std::shared_ptr<IpcServerListener> ipcServerListener = std::make_shared<IpcServerListener>();
    ret = ipcServerListener->SendRequest(cmdCode, req, rsp);
    // 4. check ret is not ERR_DM_FAILED
    ASSERT_NE(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: SendCmd_005
 * @tc.desc: 1. set cmdCode not null
 *              set pkgName not null
 *              set action not null
 *           2. set remoteObject not nullptr
 *              set req not null
 *              set rsp not null
 *           3. call IpcServerClientProxy SendCmd with parameter
 *           4. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(IpcServerClientProxyTest, SendCmd_005, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = SERVER_PUBLISH_FINISH;
    // set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set action not null
    int32_t publishId = 1;
    int32_t result = 1;
    // 2. set remoteObject not nullptr
    sptr<IpcClientStub> remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, remoteObject);
    std::shared_ptr<IpcNotifyPublishResultReq> req = std::make_shared<IpcNotifyPublishResultReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    // set req not null
    req->SetPkgName(pkgName);
    // set rsp not null
    req->SetPublishId(publishId);
    req->SetResult(result);
    // 3. call IpcServerClientProxy SendCmd with parameter
    int ret = 0;
    std::shared_ptr<IpcServerListener> ipcServerListener = std::make_shared<IpcServerListener>();
    ret = ipcServerListener->SendRequest(cmdCode, req, rsp);
    // 4. check ret is not ERR_DM_FAILED
    ASSERT_NE(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: SendCmd_006
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(IpcServerClientProxyTest, SendCmd_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_AUTH_RESULT;
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "123";
    std::string token = "123456";
    int32_t status = 1;
    int32_t reason = 0;
    sptr<IpcClientStub> remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, remoteObject);
    std::shared_ptr<IpcNotifyAuthResultReq> req = std::make_shared<IpcNotifyAuthResultReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceId(deviceId);
    req->SetToken(token);
    req->SetStatus(status);
    req->SetReason(reason);
    int ret = 0;
    std::shared_ptr<IpcServerListener> ipcServerListener = std::make_shared<IpcServerListener>();
    ret = ipcServerListener->SendRequest(cmdCode, req, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SendCmd_007
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(IpcServerClientProxyTest, SendCmd_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_VERIFY_AUTH_RESULT;
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "123";
    int32_t result = 0;
    int32_t flag = 1;
    sptr<IpcClientStub> remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, remoteObject);
    std::shared_ptr<IpcNotifyVerifyAuthResultReq> req = std::make_shared<IpcNotifyVerifyAuthResultReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetDeviceId(deviceId);
    req->SetFlag(flag);
    req->SetResult(result);
    int ret = 0;
    std::shared_ptr<IpcServerListener> ipcServerListener = std::make_shared<IpcServerListener>();
    ret = ipcServerListener->SendRequest(cmdCode, req, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SendCmd_008
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(IpcServerClientProxyTest, SendCmd_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FA_NOTIFY;
    std::string pkgName = "com.ohos.test";
    std::string paramJson = "123";
    sptr<IpcClientStub> remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, remoteObject);
    std::shared_ptr<IpcNotifyDMFAResultReq> req = std::make_shared<IpcNotifyDMFAResultReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetJsonParam(paramJson);
    int ret = 0;
    std::shared_ptr<IpcServerListener> ipcServerListener = std::make_shared<IpcServerListener>();
    ret = ipcServerListener->SendRequest(cmdCode, req, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SendCmd_09
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(IpcServerClientProxyTest, SendCmd_09, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREDENTIAL_RESULT;
    std::string pkgName = "com.ohos.test";
    int32_t action = 1;
    std::string credentialResult = "123";
    sptr<IpcClientStub> remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    IpcServerStub::GetInstance().RegisterDeviceManagerListener(pkgName, remoteObject);
    std::shared_ptr<IpcNotifyCredentialReq> req = std::make_shared<IpcNotifyCredentialReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetCredentialResult(credentialResult);
    req->SetCredentialAction(action);
    int ret = 0;
    std::shared_ptr<IpcServerListener> ipcServerListener = std::make_shared<IpcServerListener>();
    ret = ipcServerListener->SendRequest(cmdCode, req, rsp);
    ASSERT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
