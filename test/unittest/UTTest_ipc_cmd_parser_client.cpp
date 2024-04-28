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

#include "UTTest_ipc_cmd_parser_client.h"

#include <unistd.h>

#include "device_manager_ipc_interface_code.h"
#include "ipc_acl_profile_req.h"
#include "ipc_client_manager.h"
#include "ipc_cmd_register.h"
#include "ipc_common_param_req.h"
#include "ipc_create_pin_holder_req.h"
#include "ipc_register_listener_req.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_start_discovery_req.h"
#include "ipc_stop_discovery_req.h"
#include "ipc_publish_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_unbind_device_req.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_set_credential_req.h"
#include "ipc_notify_event_req.h"
#include "device_manager_notify.h"
#include "ipc_req.h"
#include "dm_device_info.h"
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
void IpcCmdParserClientTest::SetUp()
{
}

void IpcCmdParserClientTest::TearDown()
{
}

void IpcCmdParserClientTest::SetUpTestCase()
{
}

void IpcCmdParserClientTest::TearDownTestCase()
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

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_001, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_DEVICE_MANAGER_LISTENER;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_002, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_AVAILABLE_DEVICE_LIST;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_003, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_DEVICE_DISCOVER;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_004, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNPUBLISH_DEVICE_DISCOVER;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_005, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = AUTHENTICATE_DEVICE;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GENERATE_ENCRYPTED_UUID;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_DEVICE;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_DEVICE;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_009, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_NETWORKTYPE_BY_NETWORK;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_010, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_UI_STATE_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_011, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_UI_STATE_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_012, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_DISCOVERY_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_013, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_DISCOVERY_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_014, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_DISCOVERY_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_015, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_DISCOVERY_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_016, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = START_DISCOVERING;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_017, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_DISCOVERING;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_018, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = START_ADVERTISING;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_019, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = START_ADVERTISING;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_020, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = START_ADVERTISING;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_021, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_ADVERTISING;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_022, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_ADVERTISING;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_023, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_TARGET;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_024, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_TARGET;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_025, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CREATE_PIN_HOLDER;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_026, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DESTROY_PIN_HOLDER;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_027, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DP_ACL_ADD;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_028, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DP_ACL_ADD;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_029, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_SECURITY_LEVEL;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_030, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IS_SAME_ACCOUNT;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_031, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CHECK_API_PERMISSION;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_001, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CHECK_API_PERMISSION;
    ASSERT_EQ(ERR_DM_FAILED, TestIpcRequestFuncReqNull(cmdCode));
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_002, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_NETWORKTYPE_BY_NETWORK;
    MessageParcel data;
    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::string pkgName = "ohos.dm.test";
    std::string netWorkId = "xxx";
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_003, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_DEVICE;
    MessageParcel data;
    std::shared_ptr<IpcUnBindDeviceReq> req = std::make_shared<IpcUnBindDeviceReq>();
    std::string pkgName = "ohos.dm.test";
    std::string deviceId = "xxx";
    req->SetPkgName(pkgName);
    req->SetDeviceId(deviceId);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_004, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_NETWORKTYPE_BY_NETWORK;
    MessageParcel data;
    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::string pkgName = "ohos.dm.test";
    std::string netWorkId = "xxx";
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_005, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_UI_STATE_CALLBACK;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::string pkgName = "ohos.dm.test";
    req->SetPkgName(pkgName);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_UI_STATE_CALLBACK;
    MessageParcel data;
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::string pkgName = "ohos.dm.test";
    req->SetPkgName(pkgName);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_DISCOVERY_CALLBACK;
    MessageParcel data;
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::string pkgName = "ohos.dm.test";
    std::string discParaStr = "XXX";
    std::string filterOpStr = "XXX";
    req->SetPkgName(pkgName);
    req->SetFirstParam(discParaStr);
    req->SetSecondParam(filterOpStr);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_DISCOVERY_CALLBACK;
    MessageParcel data;
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::string pkgName = "ohos.dm.test";
    std::string discParaStr = "XXX";
    req->SetPkgName(pkgName);
    req->SetFirstParam(discParaStr);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_009, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = START_ADVERTISING;
    MessageParcel data;
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::string pkgName = "ohos.dm.test";
    std::string discParaStr = "XXX";
    req->SetPkgName(pkgName);
    req->SetFirstParam(discParaStr);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_010, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_ADVERTISING;
    MessageParcel data;
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::string pkgName = "ohos.dm.test";
    std::string discParaStr = "XXX";
    req->SetPkgName(pkgName);
    req->SetFirstParam(discParaStr);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_011, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CREATE_PIN_HOLDER;
    MessageParcel data;
    std::shared_ptr<IpcCreatePinHolderReq> req = std::make_shared<IpcCreatePinHolderReq>();
    std::string pkgName = "ohos.dm.test";
    PeerTargetId targetId;
    DmPinType pinType = NUMBER_PIN_CODE;
    std::string payload = "XXX";
    req->SetPkgName(pkgName);
    req->SetPeerTargetId(targetId);
    req->SetPinType(pinType);
    req->SetPayload(payload);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_012, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DESTROY_PIN_HOLDER;
    MessageParcel data;
    std::shared_ptr<IpcCreatePinHolderReq> req = std::make_shared<IpcCreatePinHolderReq>();
    std::string pkgName = "ohos.dm.test";
    PeerTargetId targetId;
    DmPinType pinType = NUMBER_PIN_CODE;
    std::string payload = "XXX";
    req->SetPkgName(pkgName);
    req->SetPeerTargetId(targetId);
    req->SetPinType(pinType);
    req->SetPayload(payload);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_013, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DP_ACL_ADD;
    MessageParcel data;
    std::shared_ptr<IpcAclProfileReq> req = std::make_shared<IpcAclProfileReq>();
    std::string udid = "XXX";
    req->SetUdid(udid);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_014, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_SECURITY_LEVEL;
    MessageParcel data;
    std::shared_ptr<IpcGetInfoByNetWorkReq> req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::string networkId = "XXX";
    std::string pkgName = "ohos.dm.test";
    req->SetPkgName(pkgName);
    req->SetNetWorkId(networkId);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_001, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DEVICE_DISCOVERY;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    int16_t subscribeId = 100;
    std::string deviceId = "xxx";
    data.WriteString(pkgName);
    data.WriteInt16(subscribeId);
    DmDeviceBasicInfo basicInfo;
    data.WriteRawData(&basicInfo, sizeof(DmDeviceBasicInfo));
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_002, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_TARGET_RESULT;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    PeerTargetId targetId;
    int32_t result = 1;
    int32_t status = 1;
    std::string content = "XX";
    data.WriteString(pkgName);
    EncodePeerTargetId(targetId, data);
    data.WriteInt32(result);
    data.WriteInt32(status);
    data.WriteString(content);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_003, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_TARGET_RESULT;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    PeerTargetId targetId;
    int32_t result = 1;
    std::string content = "XX";
    data.WriteString(pkgName);
    EncodePeerTargetId(targetId, data);
    data.WriteInt32(result);
    data.WriteString(content);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_004, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREATE_PIN_HOLDER;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string deviceId = "xxx";
    int32_t pinType = 1;
    std::string payload = "xx";
    data.WriteString(pkgName);
    data.WriteString(deviceId);
    data.WriteInt32(pinType);
    data.WriteString(payload);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_005, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DESTROY_PIN_HOLDER;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    int32_t pinType = 1;
    std::string payload = "xx";
    data.WriteString(pkgName);
    data.WriteInt32(pinType);
    data.WriteString(payload);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_006, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_CREATE_PIN_HOLDER_RESULT;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    int32_t result = 1;
    data.WriteString(pkgName);
    data.WriteInt32(result);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_007, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_DESTROY_PIN_HOLDER_RESULT;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    int32_t result = 1;
    data.WriteString(pkgName);
    data.WriteInt32(result);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_008, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SERVER_ON_PIN_HOLDER_EVENT;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    int32_t result = 1;
    std::string content = "xxx";
    int32_t pinHolderEvent = 1;
    data.WriteString(pkgName);
    data.WriteInt32(result);
    data.WriteInt32(pinHolderEvent);
    data.WriteString(content);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS