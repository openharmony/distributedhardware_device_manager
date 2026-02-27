/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "UTTest_ipc_cmd_parser_service.h"

#include <unistd.h>

#include "device_manager_ipc_interface_code.h"
#include "device_manager_notify.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "ipc_auth_info_req.h"
#include "ipc_auth_info_rsp.h"
#include "ipc_client_manager.h"
#include "ipc_cmd_register.h"
#include "ipc_common_param_req.h"
#include "ipc_create_pin_holder_req.h"
#include "ipc_credential_auth_status_req.h"
#include "ipc_destroy_pin_holder_req.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_bind_result_req.h"
#include "ipc_notify_credential_req.h"
#include "ipc_notify_device_discovery_req.h"
#include "ipc_notify_dmfa_result_req.h"
#include "ipc_notify_event_req.h"
#include "ipc_notify_pin_holder_event_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_publish_req.h"
#include "ipc_register_serviceinfo_req.h"
#include "ipc_register_listener_req.h"
#include "ipc_req.h"
#include "ipc_set_credential_req.h"
#include "ipc_set_credential_rsp.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_unpublish_req.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
void IpcCmdParserServiceTest::SetUp()
{
}

void IpcCmdParserServiceTest::TearDown()
{
}

void IpcCmdParserServiceTest::SetUpTestCase()
{
}

void IpcCmdParserServiceTest::TearDownTestCase()
{
}

namespace {
SetIpcRequestFunc GetIpcRequestFunc(int32_t cmdCode)
{
    SetIpcRequestFunc ptr = nullptr;
    auto setRequestMapIter = IpcCmdRegister::GetInstance().setIpcRequestFuncMap_.find(cmdCode);
    if (setRequestMapIter != IpcCmdRegister::GetInstance().setIpcRequestFuncMap_.end()) {
        ptr = setRequestMapIter->second;
    }
    return ptr;
}

ReadResponseFunc GetResponseFunc(int32_t cmdCode)
{
    auto readResponseMapIter = IpcCmdRegister::GetInstance().readResponseFuncMap_.find(cmdCode);
    if (readResponseMapIter == IpcCmdRegister::GetInstance().readResponseFuncMap_.end()) {
        return nullptr;
    }
    return readResponseMapIter->second;
}

OnIpcCmdFunc GetIpcCmdFunc(int32_t cmdCode)
{
    auto onIpcCmdMapIter = IpcCmdRegister::GetInstance().onIpcCmdFuncMap_.find(cmdCode);
    if (onIpcCmdMapIter == IpcCmdRegister::GetInstance().onIpcCmdFuncMap_.end()) {
        return nullptr;
    }
    return onIpcCmdMapIter->second;
}

int32_t TestIpcRequestFuncReqNull(int32_t cmdCode)
{
    MessageParcel data;
    std::shared_ptr<IpcReq> req = nullptr;
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    return ret;
}

int32_t TestReadResponseRspNull(int32_t cmdCode)
{
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = nullptr;
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    ReadResponseFunc ptr = GetResponseFunc(cmdCode);
    if (ptr) {
        ret = ptr(reply, rsp);
    }
    return ret;
}

int32_t TestReadResponseRspNotNull(int32_t cmdCode)
{
    MessageParcel reply;
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    int32_t retCode = 0;
    reply.WriteInt32(retCode);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    ReadResponseFunc ptr = GetResponseFunc(cmdCode);
    if (ptr) {
        ret = ptr(reply, rsp);
    }
    return ret;
}

bool EncodePeerTargetId(const PeerTargetId &targetId, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(targetId.deviceId));
    bRet = (bRet && parcel.WriteString(targetId.brMac));
    bRet = (bRet && parcel.WriteString(targetId.bleMac));
    bRet = (bRet && parcel.WriteString(targetId.wifiIp));
    bRet = (bRet && parcel.WriteUint16(targetId.wifiPort));
    return bRet;
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_001, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_STATE_NOTIFY;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_001, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_STATE_NOTIFY;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_002, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_STATE_NOTIFY;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_002, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FOUND;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_003, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FOUND;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_004, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FOUND;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_003, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_DISCOVERY;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_005, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_DISCOVERY;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_DISCOVERY;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_004, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DISCOVER_FINISH;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DISCOVER_FINISH;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DISCOVER_FINISH;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_005, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_PUBLISH_FINISH;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_009, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_PUBLISH_FINISH;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_010, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_PUBLISH_FINISH;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_AUTH_RESULT;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_011, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_AUTH_RESULT;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_012, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_AUTH_RESULT;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FA_NOTIFY;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_013, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FA_NOTIFY;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_014, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FA_NOTIFY;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREDENTIAL_RESULT;
    MessageParcel data;
    std::shared_ptr<IpcNotifyCredentialReq> req = nullptr;
    int ret = ERR_DM_FAILED;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, ERR_DM_FAILED);

    req = std::make_shared<IpcNotifyCredentialReq>();
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_015, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREDENTIAL_RESULT;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_016, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREDENTIAL_RESULT;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_009, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_TARGET_RESULT;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_017, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_TARGET_RESULT;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_018, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_TARGET_RESULT;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_010, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_TARGET_RESULT;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_019, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_TARGET_RESULT;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_020, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_TARGET_RESULT;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_011, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREATE_PIN_HOLDER;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = nullptr;
    int ret = ERR_DM_FAILED;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, ERR_DM_FAILED);

    req = std::make_shared<IpcCreatePinHolderReq>();
    std::string pkgName = "com.ohos.test";
    req->SetPkgName(pkgName);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_021, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREATE_PIN_HOLDER;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_022, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREATE_PIN_HOLDER;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_012, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DESTROY_PIN_HOLDER;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = nullptr;
    int ret = ERR_DM_FAILED;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, ERR_DM_FAILED);

    req = std::make_shared<IpcDestroyPinHolderReq>();
    std::string pkgName = "com.ohos.test";
    req->SetPkgName(pkgName);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_023, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DESTROY_PIN_HOLDER;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_024, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DESTROY_PIN_HOLDER;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_013, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREATE_PIN_HOLDER_RESULT;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = nullptr;
    int ret = ERR_DM_FAILED;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, ERR_DM_FAILED);

    req = std::make_shared<IpcNotifyPublishResultReq>();
    std::string pkgName = "com.ohos.test";
    req->SetPkgName(pkgName);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_025, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREATE_PIN_HOLDER_RESULT;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_026, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREATE_PIN_HOLDER_RESULT;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_014, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DESTROY_PIN_HOLDER_RESULT;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = nullptr;
    int ret = ERR_DM_FAILED;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, ERR_DM_FAILED);

    req = std::make_shared<IpcNotifyPublishResultReq>();
    std::string pkgName = "com.ohos.test";
    req->SetPkgName(pkgName);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_027, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DESTROY_PIN_HOLDER_RESULT;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_028, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DESTROY_PIN_HOLDER_RESULT;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_001, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_DEVICE;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string deviceId = "xxx";
    data.WriteString(pkgName);
    data.WriteString("");
    data.WriteString(deviceId);
    data.WriteInt32(1);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_002, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_DEVICE;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string deviceId = "xxx";
    data.WriteString(pkgName);
    data.WriteString(deviceId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_003, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_NETWORKTYPE_BY_NETWORK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string networkId = "xxx";
    data.WriteString(pkgName);
    data.WriteString(networkId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_004, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_UI_STATE_CALLBACK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_005, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_UI_STATE_CALLBACK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IMPORT_AUTH_CODE;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string authCode = "123456";
    data.WriteString(pkgName);
    data.WriteString(authCode);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = EXPORT_AUTH_CODE;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_DISCOVERY_CALLBACK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string param = "xxx";
    data.WriteString(pkgName);
    data.WriteString(param);
    data.WriteString(param);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_009, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_DISCOVERY_CALLBACK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string param = "xxx";
    data.WriteString(pkgName);
    data.WriteString(param);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_010, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = START_DISCOVERING;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string param = "xxx";
    data.WriteString(pkgName);
    data.WriteString(param);
    data.WriteString(param);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_011, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_DISCOVERING;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string param = "xxx";
    data.WriteString(pkgName);
    data.WriteString(param);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_012, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = START_ADVERTISING;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string param = "xxx";
    data.WriteString(pkgName);
    data.WriteString(param);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_013, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_ADVERTISING;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string param = "xxx";
    data.WriteString(pkgName);
    data.WriteString(param);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_014, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_TARGET;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string param = "xxx";
    data.WriteString(pkgName);
    PeerTargetId targetId;
    EncodePeerTargetId(targetId, data);
    data.WriteString(param);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_015, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_TARGET;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string param = "xxx";
    data.WriteString(pkgName);
    PeerTargetId targetId;
    EncodePeerTargetId(targetId, data);
    data.WriteString(param);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_016, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_PIN_HOLDER_CALLBACK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_017, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CREATE_PIN_HOLDER;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    PeerTargetId targetId;
    EncodePeerTargetId(targetId, data);
    std::string param = "xxx";
    data.WriteString(param);
    data.WriteInt32(1);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_018, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DESTROY_PIN_HOLDER;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    PeerTargetId targetId;
    EncodePeerTargetId(targetId, data);
    std::string param = "xxx";
    data.WriteString(param);
    data.WriteString(param);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_019, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DP_ACL_ADD;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_020, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_SECURITY_LEVEL;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string networkId = "xxx";
    data.WriteString(pkgName);
    data.WriteString(networkId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_021, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IS_SAME_ACCOUNT;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string udid = "xxx";
    data.WriteString(udid);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_022, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CHECK_API_PERMISSION;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_023, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_TRUST_DEVICE_LIST;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string extra = "";
    bool isRefresh = true;
    data.WriteString(pkgName);
    data.WriteString(extra);
    data.WriteBool(isRefresh);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_025, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_DEVICE_MANAGER_LISTENER;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "pkgName";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, ERR_DM_POINT_NULL);

    data.WriteString(pkgName);
    sptr<IRemoteObject> remoteObject = nullptr;
    data.WriteRemoteObject(remoteObject);

    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, ERR_DM_POINT_NULL);

    data.WriteString(pkgName);
    remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    data.WriteRemoteObject(remoteObject);

    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_026, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_DEVICE_MANAGER_LISTENER;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_030, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = PUBLISH_DEVICE_DISCOVER;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    DmPublishInfo dmPublishInfo;
    dmPublishInfo.publishId = 1000;
    data.WriteString(pkgName);
    data.WriteRawData(&dmPublishInfo, sizeof(DmPublishInfo));
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_031, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNPUBLISH_DEVICE_DISCOVER;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    int32_t publishId = 1000;
    data.WriteString(pkgName);
    data.WriteInt32(publishId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_032, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = AUTHENTICATE_DEVICE;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string extra = "";
    int32_t authType = 1;
    DmDeviceInfo deviceInfo;
    std::string deviceId = "12345678";
    data.WriteString(pkgName);
    data.WriteString(extra);
    data.WriteString(deviceId);
    data.WriteInt32(authType);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_033, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNAUTHENTICATE_DEVICE;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string networkId = "12345678";
    data.WriteString(pkgName);
    data.WriteString(networkId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_034, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_DEVICE_INFO;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string networkId = "12345678";
    data.WriteString(pkgName);
    data.WriteString(networkId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_035, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_LOCAL_DEVICE_INFO;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_040, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_UDID_BY_NETWORK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string netWorkId = "12345678";
    data.WriteString(pkgName);
    data.WriteString(netWorkId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_041, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_UUID_BY_NETWORK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string netWorkId = "12345678";
    data.WriteString(pkgName);
    data.WriteString(netWorkId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_042, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_USER_AUTH_OPERATION;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    int32_t action = 1;
    std::string params = "";
    data.WriteString(pkgName);
    data.WriteInt32(action);
    data.WriteString(params);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_043, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REQUEST_CREDENTIAL;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string requestJsonStr = "";
    data.WriteString(pkgName);
    data.WriteString(requestJsonStr);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_044, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IMPORT_CREDENTIAL;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string credentialInfo = "";
    data.WriteString(pkgName);
    data.WriteString(credentialInfo);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_045, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DELETE_CREDENTIAL;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string deleteInfo = "";
    data.WriteString(pkgName);
    data.WriteString(deleteInfo);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_046, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_GET_DMFA_INFO;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string reqJsonStr = "";
    data.WriteString(pkgName);
    data.WriteString(reqJsonStr);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_047, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_GET_DMFA_INFO;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string reqJsonStr = "";
    data.WriteString(pkgName);
    data.WriteString(reqJsonStr);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_048, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_CREDENTIAL_CALLBACK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_049, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_CREDENTIAL_CALLBACK;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    data.WriteString(pkgName);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_050, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = NOTIFY_EVENT;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    int32_t eventId = 1;
    std::string event = "";
    data.WriteString(pkgName);
    data.WriteInt32(eventId);
    data.WriteString(event);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_051, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_ENCRYPTED_UUID_BY_NETWOEKID;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string netWorkId = "123456789";
    data.WriteString(pkgName);
    data.WriteString(netWorkId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_052, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GENERATE_ENCRYPTED_UUID;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string uuid = "123456789";
    std::string appId = "1234";
    data.WriteString(pkgName);
    data.WriteString(uuid);
    data.WriteString(appId);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_053, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IMPORT_CREDENTIAL;
    int32_t ret = DM_OK;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    JsonObject jsonObject;
    jsonObject[DM_CREDENTIAL_TYPE] = DM_TYPE_OH;
    jsonObject[DM_CREDENTIAL_REQJSONSTR] = "";
    std::string credentialInfo = jsonObject.Dump();
    data.WriteString(pkgName);
    data.WriteString(credentialInfo);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);

    cmdCode = DELETE_CREDENTIAL;
    data.WriteString(pkgName);
    data.WriteString(credentialInfo);
    OnIpcCmdFunc ptr1 = GetIpcCmdFunc(cmdCode);
    if (ptr1) {
        ret = ptr1(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);

    cmdCode = REQUEST_CREDENTIAL;
    data.WriteString(pkgName);
    data.WriteString(credentialInfo);
    OnIpcCmdFunc ptr2 = GetIpcCmdFunc(cmdCode);
    if (ptr2) {
        ret = ptr2(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_054, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IMPORT_CREDENTIAL;
    int32_t ret = DM_OK;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    JsonObject jsonObject;
    jsonObject[DM_CREDENTIAL_TYPE] = DM_TYPE_MINE;
    jsonObject[DM_CREDENTIAL_REQJSONSTR] = "";
    std::string credentialInfo = jsonObject.Dump();
    data.WriteString(pkgName);
    data.WriteString(credentialInfo);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);

    cmdCode = DELETE_CREDENTIAL;
    data.WriteString(pkgName);
    data.WriteString(credentialInfo);
    OnIpcCmdFunc ptr1 = GetIpcCmdFunc(cmdCode);
    if (ptr1) {
        ret = ptr1(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);

    cmdCode = REQUEST_CREDENTIAL;
    data.WriteString(pkgName);
    data.WriteString(credentialInfo);
    OnIpcCmdFunc ptr2 = GetIpcCmdFunc(cmdCode);
    if (ptr2) {
        ret = ptr2(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_060, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REG_LOCALSERVICE_INFO;
    DMLocalServiceInfo serviceInfo;
    serviceInfo.bundleName = "testbundle";
    serviceInfo.extraInfo = "testextra";
    MessageParcel data;
    std::shared_ptr<IpcRegAuthInfoReq> req = std::make_shared<IpcRegAuthInfoReq>();
    req->SetLocalServiceInfo(serviceInfo);

    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);

    auto cmdptr = GetIpcCmdFunc(cmdCode);
    ASSERT_TRUE(cmdptr != nullptr);
    MessageParcel reply;
    EXPECT_EQ(cmdptr(data, reply), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_061, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREG_LOCALSERVICE_INFO;
    DMLocalServiceInfo serviceInfo;
    MessageParcel data;
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    req->SetFirstParam(std::string("testbundle"));
    req->SetInt32Param(100);

    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);

    auto cmdptr = GetIpcCmdFunc(cmdCode);
    ASSERT_TRUE(cmdptr != nullptr);
    MessageParcel reply;
    EXPECT_EQ(cmdptr(data, reply), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_062, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UPDATE_LOCALSERVICE_INFO;
    DMLocalServiceInfo serviceInfo;
    MessageParcel data;
    std::shared_ptr<IpcRegAuthInfoReq> req = std::make_shared<IpcRegAuthInfoReq>();
    req->SetLocalServiceInfo(serviceInfo);

    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);

    auto cmdptr = GetIpcCmdFunc(cmdCode);
    ASSERT_TRUE(cmdptr != nullptr);
    MessageParcel reply;
    EXPECT_EQ(cmdptr(data, reply), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_063, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_SERVICEINFO_BYBUNDLENAME_PINEXCHANGETYPE;
    DMLocalServiceInfo serviceInfo;
    MessageParcel data;
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    req->SetFirstParam(std::string("testbundle"));
    req->SetInt32Param(100);

    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);

    auto cmdptr = GetIpcCmdFunc(cmdCode);
    ASSERT_TRUE(cmdptr != nullptr);
    MessageParcel reply;
    EXPECT_EQ(cmdptr(data, reply), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_016, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_DISCOVERY;
    MessageParcel data;
    std::shared_ptr<IpcNotifyDeviceDiscoveryReq> pReq = std::make_shared<IpcNotifyDeviceDiscoveryReq>();
    std::string pkgName = "com.ohos.test";
    uint16_t subscribeId = 100;
    DmDeviceBasicInfo deviceBasicInfo;
    pReq->SetPkgName(pkgName);
    pReq->SetSubscribeId(subscribeId);
    pReq->SetDeviceBasicInfo(deviceBasicInfo);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(pReq, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_017, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_AUTH_RESULT;
    MessageParcel data;
    std::shared_ptr<IpcNotifyAuthResultReq> pReq = std::make_shared<IpcNotifyAuthResultReq>();
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "112233445";
    std::string token = "134354656";
    int32_t status = 1;
    int32_t reason = 1;
    pReq->SetPkgName(pkgName);
    pReq->SetDeviceId(deviceId);
    pReq->SetToken(token);
    pReq->SetStatus(status);
    pReq->SetReason(reason);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(pReq, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_018, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_FA_NOTIFY;
    MessageParcel data;
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::make_shared<IpcNotifyDMFAResultReq>();
    std::string pkgName = "com.ohos.test";
    std::string paramJson = "{}";
    pReq->SetPkgName(pkgName);
    pReq->SetJsonParam(paramJson);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(pReq, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_019, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_TARGET_RESULT;
    MessageParcel data;
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::make_shared<IpcNotifyBindResultReq>();
    std::string pkgName = "com.ohos.test";
    PeerTargetId targetId;
    int32_t result = 1;
    int32_t status = 1;
    std::string content = "";
    pReq->SetPkgName(pkgName);
    pReq->SetPeerTargetId(targetId);
    pReq->SetResult(result);
    pReq->SetStatus(status);
    pReq->SetContent(content);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(pReq, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_020, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_TARGET_RESULT;
    MessageParcel data;
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::make_shared<IpcNotifyBindResultReq>();
    std::string pkgName = "com.ohos.test";
    PeerTargetId targetId;
    int32_t result = 1;
    std::string content = "";
    pReq->SetPkgName(pkgName);
    pReq->SetPeerTargetId(targetId);
    pReq->SetResult(result);
    pReq->SetContent(content);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(pReq, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_021, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_ON_PIN_HOLDER_EVENT;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = nullptr;
    int ret = ERR_DM_FAILED;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, ERR_DM_FAILED);

    req = std::make_shared<IpcNotifyPinHolderEventReq>();
    std::string pkgName = "com.ohos.test";
    req->SetPkgName(pkgName);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_029, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_ON_PIN_HOLDER_EVENT;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_055, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CHECK_ACCESS_CONTROL;
    int32_t ret = DM_OK;
    MessageParcel data;
    MessageParcel reply;
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);

    cmdCode = CHECK_SAME_ACCOUNT;
    OnIpcCmdFunc ptr1 = GetIpcCmdFunc(cmdCode);
    if (ptr1) {
        ret = ptr1(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);

    cmdCode = SHIFT_LNN_GEAR;
    OnIpcCmdFunc ptr2 = GetIpcCmdFunc(cmdCode);
    if (ptr2) {
        ret = ptr2(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_022, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVICE_CREDENTIAL_AUTH_STATUS_NOTIFY;
    MessageParcel data;
    std::shared_ptr<IpcNotifyCredentialAuthStatusReq> req = nullptr;
    int ret = ERR_DM_FAILED;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, ERR_DM_FAILED);

    req = std::make_shared<IpcNotifyCredentialAuthStatusReq>();
    std::string pkgName = "com.ohos.test";
    std::string deviceList = "test";
    uint16_t deviceTypeId = 0x00;
    int32_t errcode = -1;
    req->SetPkgName(pkgName);
    req->SetDeviceList(deviceList);
    req->SetDeviceTypeId(deviceTypeId);
    req->SetErrCode(errcode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_030, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVICE_CREDENTIAL_AUTH_STATUS_NOTIFY;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_023, testing::ext::TestSize.Level1)
{
    int32_t cmdCode = IMPORT_AUTH_INFO;
    MessageParcel data;
    std::shared_ptr<IpcAuthInfoReq> req = nullptr;
    int ret = ERR_DM_FAILED;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, ERR_DM_FAILED);

    req = std::make_shared<IpcAuthInfoReq>();
    DmAuthInfo dmAuthInfo;
    uint32_t pinLength = 4;
    req->SetDmAuthInfo(dmAuthInfo);
    req->SetPinLength(pinLength);

    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_024, testing::ext::TestSize.Level1)
{
    int32_t cmdCode = EXPORT_AUTH_INFO;
    MessageParcel data;
    std::shared_ptr<IpcAuthInfoReq> req = nullptr;
    int ret = ERR_DM_FAILED;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, ERR_DM_FAILED);

    req = std::make_shared<IpcAuthInfoReq>();
    DmAuthInfo dmAuthInfo;
    uint32_t pinLength = 4;
    req->SetDmAuthInfo(dmAuthInfo);
    req->SetPinLength(pinLength);

    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_064, testing::ext::TestSize.Level1)
{
    int32_t cmdCode = EXPORT_AUTH_INFO;
    DmAuthInfo dmAuthInfo;
    MessageParcel data;
    std::shared_ptr<IpcAuthInfoReq> req = std::make_shared<IpcAuthInfoReq>();
    uint32_t pinLength = 4;
    req->SetDmAuthInfo(dmAuthInfo);
    req->SetPinLength(pinLength);

    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);

    auto cmdptr = GetIpcCmdFunc(cmdCode);
    ASSERT_TRUE(cmdptr != nullptr);
    MessageParcel reply;
    EXPECT_EQ(cmdptr(data, reply), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, OnIpcCmdFunc_065, testing::ext::TestSize.Level1)
{
    int32_t cmdCode = IMPORT_AUTH_INFO;
    DmAuthInfo dmAuthInfo;
    MessageParcel data;
    std::shared_ptr<IpcAuthInfoReq> req = std::make_shared<IpcAuthInfoReq>();
    uint32_t pinLength = 4;
    req->SetDmAuthInfo(dmAuthInfo);
    req->SetPinLength(pinLength);

    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);

    auto cmdptr = GetIpcCmdFunc(cmdCode);
    ASSERT_TRUE(cmdptr != nullptr);
    MessageParcel reply;
    EXPECT_EQ(cmdptr(data, reply), DM_OK);
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_031, testing::ext::TestSize.Level1)
{
    int32_t cmdCode = EXPORT_AUTH_INFO;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_032, testing::ext::TestSize.Level1)
{
    int32_t cmdCode = IMPORT_AUTH_INFO;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS