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

#include "UTTest_ipc_cmd_register.h"

#include <unistd.h>

#include "device_manager_ipc_interface_code.h"
#include "ipc_client_manager.h"
#include "ipc_register_listener_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_publish_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_set_credential_req.h"
#include "ipc_set_credential_rsp.h"
#include "ipc_notify_event_req.h"
#include "device_manager_notify.h"
#include "ipc_req.h"
#include "dm_device_info.h"
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
void IpcCmdRegisterTest::SetUp()
{
}

void IpcCmdRegisterTest::TearDown()
{
}

void IpcCmdRegisterTest::SetUpTestCase()
{
}

void IpcCmdRegisterTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: SetRequest_001
 * @tc.desc: 1. set cmdCode not null
 *              set MessageParcel data null
 *              set IpcReq null
 *           2. call IpcCmdRegister OnRemoteRequest with parameter
 *           3. check ret is ERR_DM_UNSUPPORTED_IPC_COMMAND
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_001, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = 999;
    //  set MessageParcel data null
    MessageParcel data;
    // set IpcReq null
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    // 2. call IpcCmdRegister OnRemoteRequest with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_UNSUPPORTED_IPC_COMMAND);
}

/**
 * @tc.name: SetRequest_002
 * @tc.desc: 1. set cmdCode not null
 *              set MessageParcel data null
 *              set IpcRegisterListenerReq null
 *           2. call IpcCmdRegister OnRemoteRequest with parameter
 *           3. check ret is ERR_DM_IPC_WRITE_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_002, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = REGISTER_DEVICE_MANAGER_LISTENER;
    //  set MessageParcel data null
    MessageParcel data;
    // set IpcRegisterListenerReq null
    std::shared_ptr<IpcRegisterListenerReq> req = std::make_shared<IpcRegisterListenerReq>();
    // 2. call IpcCmdRegister OnRemoteRequest with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    // 3. check ret is ERR_DM_IPC_WRITE_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_WRITE_FAILED);
}

/**
 * @tc.name: SetRequest_003
 * @tc.desc: 1. set cmdCode not null
 *              set MessageParcel data null
 *              set IpcRegisterListenerReq with pkgName not null
 *           2. call IpcCmdRegister OnRemoteRequest with parameter
 *           3. check ret is ERR_DM_IPC_WRITE_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_003, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = REGISTER_DEVICE_MANAGER_LISTENER;
    //  set MessageParcel data null
    MessageParcel data;
    // set IpcRegisterListenerReq null
    std::shared_ptr<IpcRegisterListenerReq> req = std::make_shared<IpcRegisterListenerReq>();
    std::string pkgName = "com.ohos.test";
    req->SetPkgName(pkgName);
    // 2. call IpcCmdRegister OnRemoteRequest with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    // 3. check ret is ERR_DM_IPC_WRITE_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_WRITE_FAILED);
}

/**
 * @tc.name: SetRequest_004
 * @tc.desc: 1. set cmdCode not null
 *              set MessageParcel data null
 *              set IpcRegisterListenerReq with listener
 *           2. call IpcCmdRegister OnRemoteRequest with parameter
 *           3. check ret is ERR_DM_IPC_WRITE_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_004, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = REGISTER_DEVICE_MANAGER_LISTENER;
    //  set MessageParcel data null
    MessageParcel data;
    // set IpcRegisterListenerReq null
    std::shared_ptr<IpcRegisterListenerReq> req = std::make_shared<IpcRegisterListenerReq>();
    sptr<IRemoteObject> listener = nullptr;
    std::string pkgName = "";
    req->SetPkgName(pkgName);
    req->SetListener(listener);
    // 2. call IpcCmdRegister OnRemoteRequest with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    // 3. check ret is ERR_DM_IPC_WRITE_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_WRITE_FAILED);
}

/**
 * @tc.name: SetRequest_005
 * @tc.desc: 1. set cmdCode not null
 *              set MessageParcel data null
 *              set IpcRegisterListenerReq with listener
 *           2. call IpcCmdRegister OnRemoteRequest with parameter
 *           3. check ret is ERR_DM_IPC_WRITE_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_005, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = REGISTER_DEVICE_MANAGER_LISTENER;
    //  set MessageParcel data null
    MessageParcel data;
    // set IpcRegisterListenerReq null
    std::shared_ptr<IpcRegisterListenerReq> req = std::make_shared<IpcRegisterListenerReq>();
    sptr<IRemoteObject> listener = nullptr;
    std::string pkgName = "listen not null";
    req->SetPkgName(pkgName);
    req->SetListener(listener);
    // 2. call IpcCmdRegister OnRemoteRequest with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    // 3. check ret is ERR_DM_IPC_WRITE_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_WRITE_FAILED);
}

/**
 * @tc.name: SetRequest_006
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_DEVICE_MANAGER_LISTENER;
    MessageParcel data;
    std::shared_ptr<IpcRegisterListenerReq> req = std::make_shared<IpcRegisterListenerReq>();
    std::string pkgName = "ohos.test";
    req->SetPkgName(pkgName);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_007
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_TRUST_DEVICE_LIST;
    MessageParcel data;
    std::shared_ptr<IpcGetTrustDeviceReq> req = std::make_shared<IpcGetTrustDeviceReq>();
    std::string pkgName = "ohos.test";
    std::string extra;
    req->SetPkgName(pkgName);
    req->SetExtra(extra);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_008
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_LOCAL_DEVICE_INFO;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::string pkgName = "ohos.test";
    req->SetPkgName(pkgName);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_009
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_09, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_UDID_BY_NETWORK;
    MessageParcel data;
    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::string pkgName = "ohos.test";
    std::string netWorkId = "123";
    req->SetPkgName(pkgName);
    req->SetNetworkId(netWorkId);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_010
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_010, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_UUID_BY_NETWORK;
    MessageParcel data;
    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::string pkgName = "ohos.test";
    std::string netWorkId = "1234";
    req->SetPkgName(pkgName);
    req->SetNetworkId(netWorkId);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_013
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_013, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = PUBLISH_DEVICE_DISCOVER;
    MessageParcel data;
    std::shared_ptr<IpcPublishReq> req = std::make_shared<IpcPublishReq>();
    std::string pkgName = "ohos.test";
    DmPublishInfo dmPublishInfo;
    req->SetPkgName(pkgName);
    req->SetPublishInfo(dmPublishInfo);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_014
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_014, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNPUBLISH_DEVICE_DISCOVER;
    MessageParcel data;
    std::shared_ptr<IpcUnPublishReq> req = std::make_shared<IpcUnPublishReq>();
    std::string pkgName = "ohos.test";
    int32_t publishId = 1;
    req->SetPkgName(pkgName);
    req->SetPublishId(publishId);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_015
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_015, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = AUTHENTICATE_DEVICE;
    MessageParcel data;
    std::shared_ptr<IpcAuthenticateDeviceReq> req = std::make_shared<IpcAuthenticateDeviceReq>();
    std::string pkgName = "ohos.test";
    std::string extra;
    int32_t authType = 1;
    DmDeviceInfo deviceInfo;
    req->SetPkgName(pkgName);
    req->SetAuthType(authType);
    req->SetExtra(extra);
    req->SetDeviceInfo(deviceInfo);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_016
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_016, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = NOTIFY_EVENT;
    MessageParcel data;
    std::shared_ptr<IpcNotifyEventReq> req = std::make_shared<IpcNotifyEventReq>();
    std::string pkgName = "ohos.test";
    int32_t eventId = 1;
    std::string event;
    req->SetPkgName(pkgName);
    req->SetEventId(eventId);
    req->SetEvent(event);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_019
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_019, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_USER_AUTH_OPERATION;
    MessageParcel data;
    std::shared_ptr<IpcGetOperationReq> req = std::make_shared<IpcGetOperationReq>();
    std::string pkgName = "ohos.test";
    req->SetPkgName(pkgName);
    int32_t action = 1;
    std::string params;
    req->SetOperation(action);
    req->SetParams(params);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_022
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_022, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REQUEST_CREDENTIAL;
    MessageParcel data;
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::string pkgName = "ohos.test";
    std::string requestJsonStr;
    req->SetPkgName(pkgName);
    req->SetCredentialParam(requestJsonStr);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_023
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_023, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IMPORT_CREDENTIAL;
    MessageParcel data;
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::string pkgName = "ohos.test";
    std::string credentialInfo;
    req->SetPkgName(pkgName);
    req->SetCredentialParam(credentialInfo);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_024
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_024, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DELETE_CREDENTIAL;
    MessageParcel data;
    std::shared_ptr<IpcSetCredentialReq> req = std::make_shared<IpcSetCredentialReq>();
    std::string pkgName = "ohos.test";
    std::string deleteInfo;
    req->SetPkgName(pkgName);
    req->SetCredentialParam(deleteInfo);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_025
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_025, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_CREDENTIAL_CALLBACK;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::string pkgName = "ohos.test";
    req->SetPkgName(pkgName);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetRequest_026
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, SetRequest_026, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_CREDENTIAL_CALLBACK;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::string pkgName = "ohos.test";
    req->SetPkgName(pkgName);
    int ret = IpcCmdRegister::GetInstance().SetRequest(cmdCode, req, data);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_001
 * @tc.desc: 1. set cmdCode 9999
 *              set MessageParcel reply null
 *           2. call IpcCmdRegister ReadResponse with parameter
 *           3. check ret is ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_001, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = 9999;
    //  set MessageParcel data null
    MessageParcel reply;
    // set IpcRegisterListenerReq null
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    // 2. call IpcCmdRegister ReadResponse with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    // 3. check ret is ERR_DM_INPUT_PARA_INVALID
    ASSERT_EQ(ret, ERR_DM_UNSUPPORTED_IPC_COMMAND);
}

/**
 * @tc.name: ReadResponse_002
 * @tc.desc: 1. set cmdCode REGISTER_DEVICE_MANAGER_LISTENER
 *              set MessageParcel reply null
 *              set IpcRsp null
 *           2. call IpcCmdRegister ReadResponse with parameter
 *           3. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_002, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = REGISTER_DEVICE_MANAGER_LISTENER;
    //  set MessageParcel data null
    MessageParcel reply;
    // set IpcRegisterListenerReq null
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    // 2. call IpcCmdRegister OnRemoteRequest with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_003
 * @tc.desc: 1. set cmdCode UNREGISTER_DEVICE_MANAGER_LISTENER
 *              set MessageParcel reply null
 *              set IpcRsp null
 *           2. call IpcCmdRegister ReadResponse with parameter
 *           3. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_003, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = UNREGISTER_DEVICE_MANAGER_LISTENER;
    //  set MessageParcel data null
    MessageParcel reply;
    // set IpcRegisterListenerReq null
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    // 2. call IpcCmdRegister OnRemoteRequest with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_004
 * @tc.desc: 1. set cmdCode GET_TRUST_DEVICE_LIST
 *              set MessageParcel reply null
 *              set IpcRsp null
 *           2. call IpcCmdRegister ReadResponse with parameter
 *           3. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_004, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = GET_TRUST_DEVICE_LIST;
    //  set MessageParcel data null
    MessageParcel reply;
    // set IpcRegisterListenerReq null
    std::shared_ptr<IpcGetTrustDeviceRsp> rsp = std::make_shared<IpcGetTrustDeviceRsp>();
    // 2. call IpcCmdRegister OnRemoteRequest with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_006
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_LOCAL_DEVICE_INFO;
    MessageParcel reply;
    std::shared_ptr<IpcGetLocalDeviceInfoRsp> rsp = std::make_shared<IpcGetLocalDeviceInfoRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_007
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_UDID_BY_NETWORK;
    MessageParcel reply;
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_008
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_UUID_BY_NETWORK;
    MessageParcel reply;
    std::shared_ptr<IpcGetInfoByNetWorkRsp> rsp = std::make_shared<IpcGetInfoByNetWorkRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_011
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_011, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = PUBLISH_DEVICE_DISCOVER;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_012
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_012, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNPUBLISH_DEVICE_DISCOVER;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_013
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_013, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = AUTHENTICATE_DEVICE;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_014
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_014, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNAUTHENTICATE_DEVICE;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_016
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_016, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = NOTIFY_EVENT;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_017
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_017, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_USER_AUTH_OPERATION;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_020
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_020, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_CREDENTIAL_CALLBACK;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_021
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_021, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IMPORT_CREDENTIAL;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_022
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_022, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DELETE_CREDENTIAL;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ReadResponse_023
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, ReadResponse_023, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_CREDENTIAL_CALLBACK;
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int ret = IpcCmdRegister::GetInstance().ReadResponse(cmdCode, reply, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_001
 * @tc.desc: 1. set cmdCode 9999
 *              set MessageParcel reply null
 *           2. call IpcCmdRegister OnIpcCmd with parameter
 *           3. check ret is DEVICEMANAGER_IPC_NOT_REGISTER_FUNC
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_001, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = 9999;
    //  set MessageParcel data null
    MessageParcel reply;
    MessageParcel data;
    // set IpcRegisterListenerReq null
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    // 2. call IpcCmdRegister ReadResponse with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    // 3. check ret is ERR_DM_UNSUPPORTED_IPC_COMMAND
    ASSERT_EQ(ret, ERR_DM_UNSUPPORTED_IPC_COMMAND);
}

/**
 * @tc.name: OnIpcCmd_002
 * @tc.desc: 1. set cmdCode SERVER_DEVICE_STATE_NOTIFY
 *           2. data.WriteString(pkgname)
 *              data.WriteInt32(DEVICE_STATE_ONLINE)
 *              data.WriteRawData(&dmDeviceInfo, deviceSize)
 *           3. call IpcCmdRegister OnIpcCmd with parameter
 *           4. check ret is DM_OK
 *              check result is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_002, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode SERVER_DEVICE_STATE_NOTIFY
    int32_t cmdCode = SERVER_DEVICE_STATE_NOTIFY;
    MessageParcel reply;
    MessageParcel data;
    // 2. data.WriteString(pkgname)
    data.WriteString("com.ohos.test");
    // data.WriteInt32(DEVICE_STATE_ONLINE)
    data.WriteInt32(DEVICE_STATE_ONLINE);
    DmDeviceInfo dmDeviceInfo;
    size_t deviceSize = sizeof(DmDeviceInfo);
    // data.WriteRawData(&dmDeviceInfo, deviceSize)
    data.WriteRawData(&dmDeviceInfo, deviceSize);
    // 3. call IpcCmdRegister OnIpcCmd with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    // check ret is DEVICEMANAGER_IPC_NOT_REGISTER_FUNC
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_003
 * @tc.desc: 1. set cmdCode SERVER_DEVICE_STATE_NOTIFY
 *           2. data.WriteString(pkgname)
 *              data.WriteInt32(DEVICE_STATE_ONLINE)
 *              data.WriteRawData(nullptr, deviceSize)
 *           3. call IpcCmdRegister OnIpcCmd with parameter
 *           4. check ret is DEVICEMANAGER_OK
 *              check result is DEVICE_STATE_ONLINE
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_003, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode SERVER_DEVICE_STATE_NOTIFY
    int32_t cmdCode = SERVER_DEVICE_STATE_NOTIFY;
    MessageParcel reply;
    MessageParcel data;
    // 2. data.WriteString(pkgname)
    data.WriteString("com.ohos.test");
    // data.WriteInt32(DEVICE_STATE_ONLINE)
    data.WriteInt32(DEVICE_STATE_ONLINE);
    // data.WriteRawData(nullptr, deviceSize)
    size_t deviceSize = sizeof(DmDeviceInfo);
    data.WriteRawData(nullptr, deviceSize);
    // 3. call IpcCmdRegister OnIpcCmd with parameter
    int ret = 0;
    int32_t result = 0;
    ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    result = reply.ReadInt32();
    // 4. check result is DEVICE_STATE_ONLINE
    ASSERT_EQ(result, DEVICE_STATE_ONLINE);
    // check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_004
 * @tc.desc: 1. set cmdCode SERVER_DEVICE_STATE_NOTIFY
 *           2. data.WriteString(pkgname)
 *              data.WriteInt32(DEVICE_STATE_OFFLINE)
 *              data.WriteRawData(nullptr, deviceSize)
 *           3. call IpcCmdRegister OnIpcCmd with parameter
 *           4. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_004, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode SERVER_DEVICE_STATE_NOTIFY
    int32_t cmdCode = SERVER_DEVICE_STATE_NOTIFY;
    MessageParcel reply;
    MessageParcel data;
    // 2. data.WriteString(pkgname)
    data.WriteString("com.ohos.test");
    // data.WriteInt32(DEVICE_STATE_OFFLINE)
    data.WriteInt32(DEVICE_STATE_OFFLINE);
    size_t deviceSize = sizeof(DmDeviceInfo);
    // data.WriteRawData(nullptr, deviceSize)
    data.WriteRawData(nullptr, deviceSize);
    // 3. call IpcCmdRegister OnIpcCmd with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    // check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_005
 * @tc.desc: 1. set cmdCode SERVER_DEVICE_STATE_NOTIFY
 *           2. data.WriteString(pkgname)
 *              data.WriteInt32(DEVICE_INFO_CHANGED)
 *              data.WriteRawData(nullptr, deviceSize)
 *           3. call IpcCmdRegister OnIpcCmd with parameter
 *           4. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_005, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode SERVER_DEVICE_STATE_NOTIFY
    int32_t cmdCode = SERVER_DEVICE_STATE_NOTIFY;
    MessageParcel reply;
    MessageParcel data;
    // 2. data.WriteString(pkgname)
    data.WriteString("com.ohos.test");
    // data.WriteInt32(DEVICE_INFO_CHANGED)
    data.WriteInt32(DEVICE_INFO_CHANGED);
    size_t deviceSize = sizeof(DmDeviceInfo);
    // data.WriteRawData(nullptr, deviceSize)
    data.WriteRawData(nullptr, deviceSize);
    // 3. call IpcCmdRegister OnIpcCmd with parameter
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    // 4.check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_006
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FOUND;
    MessageParcel reply;
    MessageParcel data;
    uint16_t subscribeId = 1;
    DmDeviceInfo deviceInfo;
    data.WriteString("com.ohos.test");
    data.WriteInt16(subscribeId);
    size_t deviceSize = sizeof(DmDeviceInfo);
    data.WriteRawData(&deviceInfo, deviceSize);
    int ret = 0;
    ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_007
 * @tc.type: FUNC
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = -1;
    MessageParcel reply;
    MessageParcel data;
    int ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    ASSERT_EQ(ret, ERR_DM_UNSUPPORTED_IPC_COMMAND);
}

/**
 * @tc.name: OnIpcCmd_008
 * @tc.type: FUNC
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IPC_MSG_BUTT;
    MessageParcel reply;
    MessageParcel data;
    int ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    ASSERT_EQ(ret, ERR_DM_UNSUPPORTED_IPC_COMMAND);
}

/**
 * @tc.name: OnIpcCmd_009
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_009, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DISCOVER_FINISH;
    MessageParcel reply;
    MessageParcel data;
    uint16_t subscribeId = 1;
    int32_t failedReason = 0;
    data.WriteString("com.ohos.test");
    data.WriteInt16(subscribeId);
    data.WriteInt32(failedReason);
    int ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_010
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_010, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_PUBLISH_FINISH;
    MessageParcel reply;
    MessageParcel data;
    int32_t publishId = 1;
    int32_t publishResult = 0;
    data.WriteString("com.ohos.test");
    data.WriteInt32(publishId);
    data.WriteInt32(publishResult);
    int ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_011
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_011, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_AUTH_RESULT;
    MessageParcel reply;
    MessageParcel data;
    std::string pkgName = "ohos.test";
    std::string deviceId = "123";
    std::string token = "123456";
    int32_t status = 1;
    int32_t reason = 0;
    data.WriteString(pkgName);
    data.WriteString(deviceId);
    data.WriteString(token);
    data.WriteInt32(status);
    data.WriteInt32(reason);
    int ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_013
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_013, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FA_NOTIFY;
    MessageParcel reply;
    MessageParcel data;
    std::string pkgName = "ohos.test";
    std::string paramJson = "123";
    data.WriteString(pkgName);
    data.WriteString(paramJson);
    int ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnIpcCmd_014
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcCmdRegisterTest, OnIpcCmd_014, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREDENTIAL_RESULT;
    MessageParcel reply;
    MessageParcel data;
    std::string pkgName = "ohos.test";
    std::string credentialResult = "123";
    int32_t action = 0;
    data.WriteString(pkgName);
    data.WriteInt32(action);
    data.WriteString(credentialResult);
    int ret = IpcCmdRegister::GetInstance().OnIpcCmd(cmdCode, data, reply);
    ASSERT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS