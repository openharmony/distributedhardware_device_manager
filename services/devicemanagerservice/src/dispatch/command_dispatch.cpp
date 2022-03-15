/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "command_dispatch.h"
#include "dm_log.h"
#include "message_def.h"
#include "dm_constants.h"
#include "server_stub.h"
#include "securec.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "get_trustdevice_req.h"
#include "start_discovery_req.h"
#include "stop_discovery_req.h"
#include "get_useroperation_req.h"
#include "authenticate_device_req.h"
#include "verify_authenticate_req.h"
#include "get_trustdevice_rsp.h"
#include "device_manager_service.h"
#include "get_local_device_info_rsp.h"
#include "get_info_by_network_rsp.h"
#include "get_info_by_network_req.h"
#include "unauthenticate_device_req.h"
#include "get_dmfaparam_rsp.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(CommandDispatch);

int32_t CommandDispatch::MessageSendCmd(int32_t cmdCode, const std::shared_ptr<MessageReq> &req,
                                        const std::shared_ptr<MessageRsp> &rsp)
{
    if (req == nullptr || rsp == nullptr) {
        LOGE("Message req or rsp is null.");
        return DM_INVALID_VALUE;
    }
    uint32_t i = 0;
    for (i = 0; i < (sizeof(g_cmdMap) / sizeof(g_cmdMap[0])); i++) {
        if (g_cmdMap[i].cmdCode == cmdCode) {
            break;
        }
    }
    int32_t ret = g_cmdMap[i].MsgProcess(req, rsp);
    if (ret != DM_OK) {
        LOGE("MessageSendCmd Failed with ret: %d", ret);
        return ret;
    }
    return DM_OK;
}

void CommandDispatch::AddPkgName(const std::string &pkgName)
{
    dmPkgName_.push_back(pkgName);
}

void CommandDispatch::DeletePkgName(const std::string &pkgName)
{
    dmPkgName_.remove(pkgName);
}

const std::list<std::string>& CommandDispatch::GetPkgNameList() const
{
    return dmPkgName_;
}

static int32_t GetTrustedDeviceList(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<GetTrustDeviceReq> pReq = std::static_pointer_cast<GetTrustDeviceReq>(req);
    std::shared_ptr<GetTrustDeviceRsp> prsp = std::static_pointer_cast<GetTrustDeviceRsp>(rsp);
    std::string pkgName = pReq->GetPkgName();
    std::string extra = pReq->GetExtra();

    LOGI("enter GetTrustedDeviceList");
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    prsp->SetDeviceVec(deviceList);
    prsp->SetErrCode(ret);
    return ret;
}

static int32_t GetLocalDeviceInfo(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<GetLocalDeviceInfoRsp> pRsp = std::static_pointer_cast<GetLocalDeviceInfoRsp>(rsp);
    DmDeviceInfo dmDeviceInfo = {0};
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(dmDeviceInfo);
    DmDeviceInfo *Info = &dmDeviceInfo;
    if (Info != nullptr) {
        pRsp->SetLocalDeviceInfo(dmDeviceInfo);
    }
    pRsp->SetErrCode(ret);
    return ret;
}

static int32_t GetUdidByNetworkId(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<GetInfoByNetWorkReq> pReq = std::static_pointer_cast<GetInfoByNetWorkReq>(req);
    std::shared_ptr<GetInfoByNetWorkRsp> pRsp = std::static_pointer_cast<GetInfoByNetWorkRsp>(rsp);
    std::string pkgName = pReq->GetPkgName();
    std::string netWorkId = pReq->GetNetWorkId();
    std::string udid;

    int32_t ret = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);
    pRsp->SetUdid(udid);
    pRsp->SetErrCode(ret);
    return ret;
}

static int32_t GetUuidByNetworkId(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<GetInfoByNetWorkReq> pReq = std::static_pointer_cast<GetInfoByNetWorkReq>(req);
    std::shared_ptr<GetInfoByNetWorkRsp> pRsp = std::static_pointer_cast<GetInfoByNetWorkRsp>(rsp);
    std::string pkgName = pReq->GetPkgName();
    std::string netWorkId = pReq->GetNetWorkId();
    std::string uuid;

    int32_t ret = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);
    pRsp->SetUuid(uuid);
    pRsp->SetErrCode(ret);
    return ret;
}

static int32_t StartDeviceDiscovery(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<StartDiscoveryReq> pReq = std::static_pointer_cast<StartDiscoveryReq>(req);
    std::string pkgName = pReq->GetPkgName();
    std::string extra = pReq->GetExtra();
    const DmSubscribeInfo dmSubscribeInfo = pReq->GetSubscribeInfo();
    LOGI("StartDeviceDiscovery service");

    int32_t ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, dmSubscribeInfo, extra);
    rsp->SetErrCode(ret);
    return ret;
}

static int32_t StopDeviceDiscovery(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    LOGI("StopDeviceDiscovery service");
    std::shared_ptr<StopDiscoveryReq> pReq = std::static_pointer_cast<StopDiscoveryReq>(req);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();

    int32_t ret = DeviceManagerService::GetInstance().StopDeviceDiscovery(pkgName, subscribeId);
    rsp->SetErrCode(ret);
    return ret;
}

static int32_t SetUserOperation(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<GetUserOperationReq> pReq = std::static_pointer_cast<GetUserOperationReq>(req);
    std::string pkgName = pReq->GetPkgName();
    int32_t action = pReq->GetOperation();

    LOGI("enter server user authorization operation.");

    int32_t ret = DeviceManagerService::GetInstance().SetUserOperation(pkgName, action);
    rsp->SetErrCode(ret);
    return ret;
}

static int32_t GetFaParam(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<MessageReq> pReq = std::static_pointer_cast<MessageReq>(req);
    std::shared_ptr<GetDmFaParamRsp> pRsp = std::static_pointer_cast<GetDmFaParamRsp>(rsp);
    std::string pkgName = pReq->GetPkgName();
    DmAuthParam authParam = {
        .authToken = "",
        .packageName = "",
        .appName     = "",
        .appDescription = "",
        .authType    = 0,
        .business    = 0,
        .pincode     = 0,
        .direction   = 0,
        .pinToken    = 0
    };

    LOGI("DeviceManagerStub:: GET_AUTHENTCATION_INFO:pkgName:%s", pkgName.c_str());

    int32_t ret = DeviceManagerService::GetInstance().GetFaParam(pkgName, authParam);
    pRsp->SetDmAuthParam(authParam);
    pRsp->SetErrCode(ret);
    return ret;
}

static int32_t AuthenticateDevice(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<AuthenticateDeviceReq> pReq = std::static_pointer_cast<AuthenticateDeviceReq>(req);
    std::string pkgName = pReq->GetPkgName();
    std::string extra = pReq->GetExtra();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    int32_t authType = pReq->GetAuthType();
    std::string deviceId = deviceInfo.deviceId;

    LOGI("DeviceManagerStub:: AUTHENTCATION_DEVICE:pkgName:%s", pkgName.c_str());

    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    rsp->SetErrCode(ret);
    return ret;
}

static int32_t UnAuthenticateDevice(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<UnAuthenticateDeviceReq> pReq = std::static_pointer_cast<UnAuthenticateDeviceReq>(req);
    std::string pkgName = pReq->GetPkgName();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    std::string deviceId = deviceInfo.deviceId;

    LOGI("enter server user authorization operation.");
    int32_t ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, deviceId);
    rsp->SetErrCode(ret);
    return ret;
}

static int32_t VerifyAuthentication(const std::shared_ptr<MessageReq> &req, const std::shared_ptr<MessageRsp> &rsp)
{
    std::shared_ptr<VerifyAuthenticateReq> pReq = std::static_pointer_cast<VerifyAuthenticateReq>(req);
    std::string pkgName = pReq->GetPkgName();
    std::string authParam = pReq->GetAuthPara();

    LOGI("DeviceManagerStub:: VERIFY_AUTHENTCATION:pkgName:%s", pkgName.c_str());

    int32_t ret = DeviceManagerService::GetInstance().VerifyAuthentication(authParam);
    rsp->SetErrCode(ret);
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
