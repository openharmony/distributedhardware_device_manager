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
#include "ipc_bind_device_req.h"
#include "ipc_auth_info_req.h"
#include "ipc_client_manager.h"
#include "ipc_cmd_register.h"
#include "ipc_common_param_req.h"
#include "ipc_create_pin_holder_req.h"
#include "ipc_register_listener_req.h"
#include "ipc_generate_encrypted_uuid_req.h"
#include "ipc_get_device_screen_status_req.h"
#include "ipc_get_device_screen_status_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_localserviceinfo_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_model_codec.h"
#include "ipc_publish_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_unbind_device_req.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_register_serviceinfo_req.h"
#include "ipc_set_credential_req.h"
#include "ipc_set_credential_rsp.h"
#include "ipc_notify_event_req.h"
#include "device_manager_notify.h"
#include "ipc_req.h"
#include "dm_anonymous.h"
#include "dm_device_info.h"
#include "dm_constants.h"
#include "nlohmann/json.hpp"

#define NUM_200 200

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

int32_t TestIpcRequestNull(int32_t cmdCode)
{
    MessageParcel reply;
    std::shared_ptr<IpcReq> pBaseReq = nullptr;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    auto ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(pBaseReq, reply);
    }
    return ret;
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_001, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_DEVICE_MANAGER_LISTENER;
    ASSERT_EQ(ERR_DM_FAILED, TestReadResponseRspNull(cmdCode));
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
    int32_t cmdCode = REGISTER_UI_STATE_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_010, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_UI_STATE_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_011, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_DISCOVERY_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_012, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REGISTER_DISCOVERY_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_013, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_DISCOVERY_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_014, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREGISTER_DISCOVERY_CALLBACK;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_015, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = START_DISCOVERING;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_016, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_DISCOVERING;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_017, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = START_ADVERTISING;
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
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_020, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_ADVERTISING;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_021, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = STOP_ADVERTISING;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_022, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = BIND_TARGET;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_023, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNBIND_TARGET;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_024, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CREATE_PIN_HOLDER;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_025, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DESTROY_PIN_HOLDER;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_026, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DP_ACL_ADD;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_027, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = DP_ACL_ADD;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_028, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IS_SAME_ACCOUNT;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_029, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SET_DN_POLICY;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_030, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SET_DN_POLICY;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_031, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SHIFT_LNN_GEAR;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_032, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CHECK_SAME_ACCOUNT;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_033, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CHECK_ACCESS_CONTROL;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_034, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = CHECK_API_PERMISSION;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_081, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REG_LOCALSERVICE_INFO;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_082, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREG_LOCALSERVICE_INFO;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_083, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UPDATE_LOCALSERVICE_INFO;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_084, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_SERVICEINFO_BYBUNDLENAME_PINEXCHANGETYPE;
    ASSERT_EQ(TestReadResponseRspNull(cmdCode), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_087, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REG_LOCALSERVICE_INFO;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_088, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UNREG_LOCALSERVICE_INFO;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_089, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UPDATE_LOCALSERVICE_INFO;
    ASSERT_EQ(TestReadResponseRspNotNull(cmdCode), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_090, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = GET_SERVICEINFO_BYBUNDLENAME_PINEXCHANGETYPE;
    MessageParcel reply;
    std::shared_ptr<IpcGetLocalServiceInfoRsp> rsp = std::make_shared<IpcGetLocalServiceInfoRsp>();
    int32_t retCode = 0;
    reply.WriteInt32(retCode);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    ReadResponseFunc ptr = GetResponseFunc(cmdCode);
    if (ptr) {
        ret = ptr(reply, rsp);
    }
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(rsp->GetErrCode(), ERR_DM_IPC_READ_FAILED);

    MessageParcel reply2;
    reply2.WriteInt32(DM_OK);
    DMLocalServiceInfo info;
    info.bundleName = "debug";
    ASSERT_TRUE(IpcModelCodec::EncodeLocalServiceInfo(info, reply2));
    if (ptr) {
        ret = ptr(reply2, rsp);
    }
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(rsp->GetErrCode(), DM_OK);
    ASSERT_TRUE(rsp->GetLocalServiceInfo().bundleName == "debug");
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
    req->SetStr(udid);
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

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_015, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = SET_DN_POLICY;
    MessageParcel data;
    std::shared_ptr<IpcCommonParamReq> req = std::make_shared<IpcCommonParamReq>();
    std::string pkgName = "ohos.dm.test";
    std::string policy = "DM_POLICY_STRATEGY_FOR_BLE:100";
    req->SetPkgName(pkgName);
    req->SetFirstParam(policy);
    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_031, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REG_LOCALSERVICE_INFO;
    DMLocalServiceInfo serviceInfo;
    MessageParcel data;
    std::shared_ptr<IpcRegServiceInfoReq> req = std::make_shared<IpcRegServiceInfoReq>();
    req->SetLocalServiceInfo(serviceInfo);

    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_032, testing::ext::TestSize.Level0)
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
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_033, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = UPDATE_LOCALSERVICE_INFO;
    DMLocalServiceInfo serviceInfo;
    MessageParcel data;
    std::shared_ptr<IpcRegServiceInfoReq> req = std::make_shared<IpcRegServiceInfoReq>();
    req->SetLocalServiceInfo(serviceInfo);

    int ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    SetIpcRequestFunc ptr = GetIpcRequestFunc(cmdCode);
    if (ptr) {
        ret = ptr(req, data);
    }
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_034, testing::ext::TestSize.Level0)
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

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_009, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = REMOTE_DEVICE_TRUST_CHANGE;
    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    std::string pkgName = "ohos.dm.test";
    std::string deviceId = "xxx";
    int32_t authForm = 1;
    data.WriteString(pkgName);
    data.WriteString(deviceId);
    data.WriteInt32(authForm);
    OnIpcCmdFunc ptr = GetIpcCmdFunc(cmdCode);
    if (ptr) {
        ret = ptr(data, reply);
    }
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_010, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcCmdFunc(SERVER_DEVICE_STATE_NOTIFY);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel reply;
    MessageParcel data1;
    data1.WriteString("com.ohos.test");
    data1.WriteInt32(DEVICE_INFO_READY);
    size_t deviceSize = sizeof(DmDeviceInfo);
    data1.WriteRawData(nullptr, deviceSize);
    EXPECT_EQ(ptr(data1, reply), DM_OK);

    MessageParcel data2;
    data2.WriteString("com.ohos.test");
    data2.WriteInt32(DEVICE_STATE_UNKNOWN);
    EXPECT_EQ(ptr(data2, reply), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_011, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcCmdFunc(SERVER_DEVICE_SCREEN_STATE_NOTIFY);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel reply;
    MessageParcel data;
    EXPECT_EQ(ptr(data, reply), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_012, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcCmdFunc(SERVICE_CREDENTIAL_AUTH_STATUS_NOTIFY);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel reply;
    MessageParcel data;
    EXPECT_EQ(ptr(data, reply), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, OnIpcCmdFunc_013, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcCmdFunc(SINK_BIND_TARGET_RESULT);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel reply;
    MessageParcel data;
    EXPECT_EQ(ptr(data, reply), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_016, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcRequestFunc(SERVER_GET_DMFA_INFO);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel data;
    auto req = std::make_shared<IpcSetCredentialReq>();
    std::string pkgName = "ohos.dm.test";
    std::string credential = "git:https://gitee.com";
    req->SetPkgName(pkgName);
    req->SetCredentialParam(credential);
    auto ret = ptr(req, data);
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_017, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcRequestFunc(GENERATE_ENCRYPTED_UUID);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel data;
    auto req = std::make_shared<IpcGenerateEncryptedUuidReq>();
    std::string pkgName = "ohos.dm.test";
    std::string uuid = "123456789";
    std::string appId = "1234";
    req->SetPkgName(pkgName);
    req->SetUuid(uuid);
    req->SetAppId(appId);
    auto ret = ptr(req, data);
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_018, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcRequestFunc(BIND_DEVICE);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel data;
    auto req = std::make_shared<IpcBindDeviceReq>();
    std::string pkgName = "ohos.dm.test";
    int32_t bindType = 4;
    std::string deviceId = "oh.myPhone.No1";
    req->SetPkgName(pkgName);
    req->SetBindType(bindType);
    req->SetDeviceId(deviceId);
    auto ret = ptr(req, data);
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_019, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcRequestFunc(STOP_AUTHENTICATE_DEVICE);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel data;
    auto req = std::make_shared<IpcCommonParamReq>();
    std::string pkgName = "ohos.dm.test";
    req->SetPkgName(pkgName);
    auto ret = ptr(req, data);
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_020, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcRequestFunc(GET_DEVICE_SCREEN_STATUS);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel data;
    auto req = std::make_shared<IpcGetDeviceScreenStatusReq>();
    std::string pkgName = "ohos.dm.test";
    std::string netWorkId = "3a80******fd94";
    req->SetPkgName(pkgName);
    req->SetNetWorkId(netWorkId);
    auto ret = ptr(req, data);
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_021, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcRequestFunc(GET_NETWORKID_BY_UDID);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel data;
    auto req = std::make_shared<IpcGetInfoByNetWorkReq>();
    std::string pkgName = "ohos.dm.test";
    std::string udid = "udid";
    req->SetPkgName(pkgName);
    req->SetUdid(udid);
    auto ret = ptr(req, data);
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_022, testing::ext::TestSize.Level0)
{
    auto ptr = GetIpcRequestFunc(REGISTER_DEV_STATE_CALLBACK);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel data;
    auto req = std::make_shared<IpcReq>();
    std::string pkgName = "ohos.dm.test";
    req->SetPkgName(pkgName);
    auto ret = ptr(req, data);
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_035, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(REQUEST_CREDENTIAL);
    ASSERT_TRUE(ptr != nullptr);

    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    std::string credentialReasult = "The credential is created.";
    data.WriteInt32(DM_OK);
    data.WriteString(credentialReasult);
    auto ipcRspInsance = std::make_shared<IpcSetCredentialRsp>();
    ret = ptr(data, ipcRspInsance);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_036, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(SERVER_GET_DMFA_INFO);
    ASSERT_TRUE(ptr != nullptr);

    int32_t ret = ERR_DM_UNSUPPORTED_IPC_COMMAND;
    MessageParcel data;
    std::string credentialReasult = "The credential is created.";
    data.WriteInt32(DM_OK);
    data.WriteString(credentialReasult);
    auto ipcRspInsance = std::make_shared<IpcSetCredentialRsp>();
    ret = ptr(data, ipcRspInsance);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_037, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(IMPORT_CREDENTIAL);
    ASSERT_TRUE(ptr != nullptr);

    JsonObject jsonObject;
    jsonObject[DM_CREDENTIAL_TYPE] = DM_TYPE_OH;
    std::string message = jsonObject.Dump();

    MessageParcel data;
    data.WriteString(message);
    data.WriteInt32(DM_OK);
    auto ipcRspInsance = std::make_shared<IpcSetCredentialRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_038, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(IMPORT_CREDENTIAL);
    ASSERT_TRUE(ptr != nullptr);

    std::string credential = "git:https://gitee.com";
    JsonObject jsonObject;
    jsonObject[DM_CREDENTIAL_TYPE] = DM_TYPE_MINE;
    jsonObject[DM_CREDENTIAL_RETURNJSONSTR] = credential;
    std::string message = jsonObject.Dump();

    MessageParcel data;
    data.WriteString(message);
    data.WriteInt32(DM_OK);
    auto ipcRspInsance = std::make_shared<IpcSetCredentialRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_039, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(DELETE_CREDENTIAL);
    ASSERT_TRUE(ptr != nullptr);

    JsonObject jsonObject;
    jsonObject[DM_CREDENTIAL_TYPE] = DM_TYPE_OH;
    std::string message = jsonObject.Dump();

    MessageParcel data;
    data.WriteString(message);
    data.WriteInt32(DM_OK);
    auto ipcRspInsance = std::make_shared<IpcSetCredentialRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_040, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(DELETE_CREDENTIAL);
    ASSERT_TRUE(ptr != nullptr);

    std::string credential = "git:https://gitee.com";
    JsonObject jsonObject;
    jsonObject[DM_CREDENTIAL_TYPE] = DM_TYPE_MINE;
    jsonObject[DM_CREDENTIAL_RETURNJSONSTR] = credential;
    std::string message = jsonObject.Dump();

    MessageParcel data;
    data.WriteString(message);
    data.WriteInt32(DM_OK);
    auto ipcRspInsance = std::make_shared<IpcSetCredentialRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_041, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(GENERATE_ENCRYPTED_UUID);
    ASSERT_TRUE(ptr != nullptr);

    std::string uuid = "123456789";
    MessageParcel data;
    data.WriteInt32(DM_OK);
    data.WriteString(uuid);
    auto ipcRspInsance = std::make_shared<IpcGetInfoByNetWorkRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_042, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(BIND_DEVICE);
    ASSERT_TRUE(ptr != nullptr);

    MessageParcel data;
    data.WriteInt32(DM_OK);
    auto ipcRspInsance = std::make_shared<IpcRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_043, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(GET_NETWORKTYPE_BY_NETWORK);
    ASSERT_TRUE(ptr != nullptr);

    int32_t wifi = 1;
    MessageParcel data;
    data.WriteInt32(DM_OK);
    data.WriteInt32(wifi);
    auto ipcRspInsance = std::make_shared<IpcGetInfoByNetWorkRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_044, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(STOP_AUTHENTICATE_DEVICE);
    ASSERT_TRUE(ptr != nullptr);

    int32_t wifi = 1;
    MessageParcel data;
    data.WriteInt32(DM_OK);
    auto ipcRspInsance = std::make_shared<IpcRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_045, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(GET_SECURITY_LEVEL);
    ASSERT_TRUE(ptr != nullptr);

    int32_t securityLevel = 1;
    MessageParcel data;
    data.WriteInt32(DM_OK);
    data.WriteInt32(securityLevel);
    auto ipcRspInsance = std::make_shared<IpcGetInfoByNetWorkRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_046, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(GET_DEVICE_SCREEN_STATUS);
    ASSERT_TRUE(ptr != nullptr);

    int32_t screenStatus = 1;
    MessageParcel data;
    data.WriteInt32(DM_OK);
    data.WriteInt32(screenStatus);
    auto ipcRspInsance = std::make_shared<IpcGetDeviceScreenStatusRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_047, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(GET_NETWORKID_BY_UDID);
    ASSERT_TRUE(ptr != nullptr);

    std::string networkId = "3a80******fd94";
    MessageParcel data;
    data.WriteInt32(DM_OK);
    data.WriteString(networkId);
    auto ipcRspInsance = std::make_shared<IpcGetInfoByNetWorkRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_048, testing::ext::TestSize.Level0)
{
    auto ptr = GetResponseFunc(REGISTER_DEV_STATE_CALLBACK);
    ASSERT_TRUE(ptr != nullptr);

    std::string networkId = "3a80******fd94";
    MessageParcel data;
    data.WriteInt32(DM_OK);
    data.WriteString(networkId);
    auto ipcRspInsance = std::make_shared<IpcRsp>();
    ASSERT_EQ(ptr(data, ipcRspInsance), DM_OK);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_091, testing::ext::TestSize.Level1)
{
    int32_t cmdCode = IMPORT_AUTH_INFO;
    bool result = false;
    if ((TestReadResponseRspNotNull(cmdCode) == ERR_DM_IPC_READ_FAILED) ||
        (TestReadResponseRspNotNull(cmdCode) == DM_OK))
    {
        result = true;
    }
    EXPECT_TRUE(result);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_023, testing::ext::TestSize.Level1)
{
    auto ptr = GetIpcRequestFunc(IMPORT_AUTH_INFO);
    ASSERT_TRUE(ptr != nullptr);
    MessageParcel data;
    DmAuthInfo dmAuthInfo;
    dmAuthInfo.pinConsumerPkgName = "com.test.pkgname";
    dmAuthInfo.bizSrcPkgName = "com.test.SrcPkgname";
    dmAuthInfo.bizSinkPkgName = "com.test.SinkPkgname";
    dmAuthInfo.authType = DMLocalServiceInfoAuthType::TRUST_ONETIME;
    dmAuthInfo.authBoxType = DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM;
    dmAuthInfo.pinExchangeType = DMLocalServiceInfoPinExchangeType::PINBOX;
    dmAuthInfo.description = "test import auth info";
    auto req = std::make_shared<IpcAuthInfoReq>();
    req->SetDmAuthInfo(dmAuthInfo);
    auto ret = ptr(req, data);
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_049, testing::ext::TestSize.Level1)
{
    auto ptr = GetResponseFunc(IMPORT_AUTH_INFO);
    ASSERT_TRUE(ptr != nullptr);
    MessageParcel data;
    data.WriteInt32(DM_OK);
    auto ipcRspInsance = std::make_shared<IpcRsp>();
    bool result = false;
    if ((ptr(data, ipcRspInsance) == ERR_DM_IPC_READ_FAILED) ||
        (ptr(data, ipcRspInsance) == DM_OK))
    {
        result = true;
    }
    EXPECT_TRUE(result);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_092, testing::ext::TestSize.Level1)
{
    int32_t cmdCode = EXPORT_AUTH_INFO;
    bool result = false;
    if ((TestReadResponseRspNotNull(cmdCode) == ERR_DM_IPC_READ_FAILED) ||
        (TestReadResponseRspNotNull(cmdCode) == DM_OK))
    {
        result = true;
    }
    EXPECT_TRUE(result);
}

HWTEST_F(IpcCmdParserClientTest, SetIpcRequestFunc_024, testing::ext::TestSize.Level1)
{
    auto ptr = GetIpcRequestFunc(EXPORT_AUTH_INFO);
    ASSERT_TRUE(ptr != nullptr);
    MessageParcel data;
    DmAuthInfo dmAuthInfo;
    dmAuthInfo.pinConsumerPkgName = "com.test.pkgname";
    dmAuthInfo.bizSrcPkgName = "com.test.SrcPkgname";
    dmAuthInfo.bizSinkPkgName = "com.test.SinkPkgname";
    dmAuthInfo.authType = DMLocalServiceInfoAuthType::TRUST_ONETIME;
    dmAuthInfo.authBoxType = DMLocalServiceInfoAuthBoxType::SKIP_CONFIRM;
    dmAuthInfo.pinExchangeType = DMLocalServiceInfoPinExchangeType::PINBOX;
    dmAuthInfo.description = "test import auth info";
    uint32_t pinLength = 6;
    auto req = std::make_shared<IpcAuthInfoReq>();
    req->SetDmAuthInfo(dmAuthInfo);
    req->SetPinLength(pinLength);
    auto ret = ptr(req, data);
    ASSERT_EQ(DM_OK, ret);
}

HWTEST_F(IpcCmdParserClientTest, ReadResponseFunc_050, testing::ext::TestSize.Level1)
{
    auto ptr = GetResponseFunc(EXPORT_AUTH_INFO);
    ASSERT_TRUE(ptr != nullptr);
    MessageParcel data;
    data.WriteInt32(DM_OK);
    auto ipcRspInsance = std::make_shared<IpcRsp>();
    bool result = false;
    if ((ptr(data, ipcRspInsance) == ERR_DM_IPC_READ_FAILED) ||
        (ptr(data, ipcRspInsance) == DM_OK))
    {
        result = true;
    }
    EXPECT_TRUE(result);
}

HWTEST_F(IpcCmdParserClientTest, TEST_IPC_REQUEST_NULL_001, testing::ext::TestSize.Level2)
{
    EXPECT_EQ(TestIpcRequestNull(REGISTER_DEVICE_MANAGER_LISTENER), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(UNREGISTER_DEVICE_MANAGER_LISTENER), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_TRUST_DEVICE_LIST), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_DEVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_LOCAL_DEVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_UDID_BY_NETWORK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_UUID_BY_NETWORK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(PUBLISH_DEVICE_DISCOVER), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(UNPUBLISH_DEVICE_DISCOVER), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(AUTHENTICATE_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(UNAUTHENTICATE_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(SERVER_USER_AUTH_OPERATION), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(REQUEST_CREDENTIAL), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(SERVER_GET_DMFA_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(IMPORT_CREDENTIAL), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(DELETE_CREDENTIAL), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(REGISTER_CREDENTIAL_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(UNREGISTER_CREDENTIAL_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(NOTIFY_EVENT), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_ENCRYPTED_UUID_BY_NETWOEKID), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GENERATE_ENCRYPTED_UUID), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(BIND_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(UNBIND_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_NETWORKTYPE_BY_NETWORK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(REGISTER_UI_STATE_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(UNREGISTER_UI_STATE_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(IMPORT_AUTH_CODE), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(REGISTER_DISCOVERY_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(START_DISCOVERING), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(STOP_DISCOVERING), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(BIND_TARGET), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(UNBIND_TARGET), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, TEST_IPC_REQUEST_NULL_002, testing::ext::TestSize.Level2)
{
    EXPECT_EQ(TestIpcRequestNull(REGISTER_PIN_HOLDER_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(CREATE_PIN_HOLDER), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(DESTROY_PIN_HOLDER), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(SET_DN_POLICY), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(STOP_AUTHENTICATE_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(DP_ACL_ADD), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_SECURITY_LEVEL), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(IS_SAME_ACCOUNT), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(CHECK_API_PERMISSION), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(CHECK_ACCESS_CONTROL), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(CHECK_SAME_ACCOUNT), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(SHIFT_LNN_GEAR), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_DEVICE_SCREEN_STATUS), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_ANONY_LOCAL_UDID), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_NETWORKID_BY_UDID), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(REGISTER_DEV_STATE_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(SYNC_CALLBACK), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, TEST_IPC_REQUEST_NULL_003, testing::ext::TestSize.Level2)
{
    EXPECT_EQ(TestIpcRequestNull(REG_LOCALSERVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(UNREG_LOCALSERVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(UPDATE_LOCALSERVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(GET_SERVICEINFO_BYBUNDLENAME_PINEXCHANGETYPE), ERR_DM_FAILED);
    EXPECT_EQ(TestIpcRequestNull(IMPORT_AUTH_INFO), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, TEST_READ_RESPONSE_NULL_001, testing::ext::TestSize.Level2)
{
    EXPECT_EQ(TestReadResponseRspNull(UNREGISTER_DEVICE_MANAGER_LISTENER), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_DEVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_LOCAL_DEVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_UDID_BY_NETWORK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_UUID_BY_NETWORK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(PUBLISH_DEVICE_DISCOVER), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(UNPUBLISH_DEVICE_DISCOVER), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(AUTHENTICATE_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(UNAUTHENTICATE_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(SERVER_USER_AUTH_OPERATION), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(REQUEST_CREDENTIAL), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(SERVER_GET_DMFA_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(IMPORT_CREDENTIAL), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(DELETE_CREDENTIAL), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(REGISTER_CREDENTIAL_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(UNREGISTER_CREDENTIAL_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(NOTIFY_EVENT), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_ENCRYPTED_UUID_BY_NETWOEKID), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GENERATE_ENCRYPTED_UUID), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(BIND_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(UNBIND_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_NETWORKTYPE_BY_NETWORK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(REGISTER_UI_STATE_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(UNREGISTER_UI_STATE_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(IMPORT_AUTH_CODE), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(EXPORT_AUTH_CODE), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(REGISTER_DISCOVERY_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(START_DISCOVERING), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(STOP_DISCOVERING), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(REGISTER_PIN_HOLDER_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(CREATE_PIN_HOLDER), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(DESTROY_PIN_HOLDER), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(STOP_AUTHENTICATE_DEVICE), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_SECURITY_LEVEL), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(CHECK_ACCESS_CONTROL), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_DEVICE_SCREEN_STATUS), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_ANONY_LOCAL_UDID), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_NETWORKID_BY_UDID), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(REGISTER_DEV_STATE_CALLBACK), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(SYNC_CALLBACK), ERR_DM_FAILED);
}

HWTEST_F(IpcCmdParserClientTest, TEST_READ_RESPONSE_NULL_002, testing::ext::TestSize.Level2)
{
    EXPECT_EQ(TestReadResponseRspNull(REG_LOCALSERVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(UNREG_LOCALSERVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(UPDATE_LOCALSERVICE_INFO), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(GET_SERVICEINFO_BYBUNDLENAME_PINEXCHANGETYPE), ERR_DM_FAILED);
    EXPECT_EQ(TestReadResponseRspNull(IMPORT_AUTH_INFO), ERR_DM_FAILED);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS