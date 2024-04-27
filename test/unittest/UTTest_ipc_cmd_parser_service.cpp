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
#include "ipc_client_manager.h"
#include "ipc_cmd_register.h"
#include "ipc_register_listener_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_start_discovery_req.h"
#include "ipc_stop_discovery_req.h"
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
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
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
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
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
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
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
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
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
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
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

HWTEST_F(IpcCmdParserServiceTest, SetIpcRequestFunc_015, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_ON_PIN_HOLDER_EVENT;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_029, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_ON_PIN_HOLDER_EVENT;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserServiceTest, ReadResponseFunc_030, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_ON_PIN_HOLDER_EVENT;
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
} // namespace
} // namespace DistributedHardware
} // namespace OHOS