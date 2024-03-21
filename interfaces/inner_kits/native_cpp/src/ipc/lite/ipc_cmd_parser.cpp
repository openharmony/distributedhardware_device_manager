/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "device_manager_ipc_interface_code.h"
#include "device_manager_notify.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_subscribe_info.h"
#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "ipc_get_availabledevice_req.h"
#include "ipc_get_availabledevice_rsp.h"
#include "ipc_get_device_info_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_local_device_name_rsp.h"
#include "ipc_get_local_device_networkId_rsp.h"
#include "ipc_get_local_device_type_rsp.h"
#include "ipc_get_local_deviceId_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_register_listener_req.h"
#include "ipc_set_credential_req.h"
#include "ipc_set_credential_rsp.h"
#include "ipc_start_discover_req.h"
#include "ipc_stop_discovery_req.h"
#include "securec.h"

namespace OHOS {
namespace DistributedHardware {
void DecodeDmDeviceInfo(IpcIo &reply, DmDeviceInfo &devInfo)
{
    size_t len = 0;
    std::string deviceIdStr = (const char *)ReadString(&reply, &len);
    strcpy_s(devInfo.deviceId, deviceIdStr.size() + 1, deviceIdStr.c_str());
    std::string deviceNameStr = (const char *)ReadString(&reply, &len);
    strcpy_s(devInfo.deviceName, deviceNameStr.size() + 1, deviceNameStr.c_str());
    ReadUint16(&reply, &devInfo.deviceTypeId);
    std::string networkIdStr = (const char *)ReadString(&reply, &len);
    strcpy_s(devInfo.networkId, networkIdStr.size() + 1, networkIdStr.c_str());
    ReadInt32(&reply, &devInfo.range);
    ReadInt32(&reply, &devInfo.networkType);
    int32_t authForm = 0;
    ReadInt32(&reply, &authForm);
    devInfo.authForm = (DmAuthForm)authForm;
    devInfo.extraData = (const char *)ReadString(&reply, &len);
}

int32_t SetRspErrCode(IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    pBaseRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(REGISTER_DEVICE_MANAGER_LISTENER, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcRegisterListenerReq> pReq = std::static_pointer_cast<IpcRegisterListenerReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    SvcIdentity svcIdentity = pReq->GetSvcIdentity();

    IpcIoInit(&request, buffer, buffLen, 1);
    WriteString(&request, pkgName.c_str());
    bool ret = WriteRemoteObject(&request, &svcIdentity);
    if (!ret) {
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_DEVICE_MANAGER_LISTENER, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return SetRspErrCode(reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(UNREGISTER_DEVICE_MANAGER_LISTENER, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request,
                   uint8_t *buffer, size_t buffLen)
{
    std::string pkgName = pBaseReq->GetPkgName();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNREGISTER_DEVICE_MANAGER_LISTENER, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return SetRspErrCode(reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(GET_LOCAL_DEVICE_NETWORKID, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::string pkgName = pBaseReq->GetPkgName();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DEVICE_NETWORKID, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetLocalDeviceNetworkIdRsp> pRsp =
        std::static_pointer_cast<IpcGetLocalDeviceNetworkIdRsp>(pBaseRsp);
    size_t networkIdLen = 0;
    std::string networkIdStr = (const char *)ReadString(&reply, &networkIdLen);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    pRsp->SetLocalDeviceNetworkId(networkIdStr);
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_LOCAL_DEVICEID, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::string pkgName = pBaseReq->GetPkgName();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DEVICEID, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetLocalDeviceIdRsp> pRsp =
        std::static_pointer_cast<IpcGetLocalDeviceIdRsp>(pBaseRsp);
    size_t deviceIdLen = 0;
    std::string deviceId = (const char *)ReadString(&reply, &deviceIdLen);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    pRsp->SetLocalDeviceId(deviceId);
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_LOCAL_DEVICE_NAME, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::string pkgName = pBaseReq->GetPkgName();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DEVICE_NAME, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetLocalDeviceNameRsp> pRsp =
        std::static_pointer_cast<IpcGetLocalDeviceNameRsp>(pBaseRsp);
    size_t deviceNameLen = 0;
    std::string deviceName = (const char *)ReadString(&reply, &deviceNameLen);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    pRsp->SetLocalDeviceName(deviceName);
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_LOCAL_DEVICE_TYPE, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::string pkgName = pBaseReq->GetPkgName();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DEVICE_TYPE, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetLocalDeviceTypeRsp> pRsp =
        std::static_pointer_cast<IpcGetLocalDeviceTypeRsp>(pBaseRsp);
    int32_t deviceType = 0;
    ReadInt32(&reply, &deviceType);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    pRsp->SetLocalDeviceType(deviceType);
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_DEVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcGetInfoByNetWorkReq> pReq =
        std::static_pointer_cast<IpcGetInfoByNetWorkReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string networkId = pReq->GetNetWorkId();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, networkId.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_DEVICE_INFO, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetDeviceInfoRsp> pRsp =
        std::static_pointer_cast<IpcGetDeviceInfoRsp>(pBaseRsp);
    DmDeviceInfo deviceInfo;
    DecodeDmDeviceInfo(reply, deviceInfo);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    pRsp->SetDeviceInfo(deviceInfo);
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_AVAILABLE_DEVICE_LIST, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::string pkgName = pBaseReq->GetPkgName();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_AVAILABLE_DEVICE_LIST, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetAvailableDeviceRsp> pRsp = std::static_pointer_cast<IpcGetAvailableDeviceRsp>(pBaseRsp);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    int32_t deviceNum = 0;
    ReadInt32(&reply, &deviceNum);

    if (ret == DM_OK && deviceNum > 0) {
        uint32_t deviceTotalSize = deviceNum * (int32_t)sizeof(DmDeviceBasicInfo);
        DmDeviceBasicInfo *pDmDeviceinfo = (DmDeviceBasicInfo *)ReadRawData(&reply, deviceTotalSize);
        if (pDmDeviceinfo == nullptr) {
            LOGE("failed to read trusted device node info.");
            pRsp->SetErrCode(ERR_DM_IPC_READ_FAILED);
            return ERR_DM_IPC_READ_FAILED;
        }
        std::vector<DmDeviceBasicInfo> dmDeviceInfoVec;
        for (int32_t i = 0; i < deviceNum; i++) {
            dmDeviceInfoVec.push_back(*pDmDeviceinfo);
            pDmDeviceinfo = ++pDmDeviceinfo;
        }
        pRsp->SetDeviceVec(dmDeviceInfoVec);
    }
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_TRUST_DEVICE_LIST, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcGetTrustDeviceReq> req = std::static_pointer_cast<IpcGetTrustDeviceReq>(pBaseReq);
    std::string pkgName = req->GetPkgName();
    std::string extra = req->GetExtra();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, extra.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_TRUST_DEVICE_LIST, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcGetTrustDeviceRsp> pRsp = std::static_pointer_cast<IpcGetTrustDeviceRsp>(pBaseRsp);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    int32_t deviceNum = 0;
    ReadInt32(&reply, &deviceNum);

    if (ret == DM_OK && deviceNum > 0) {
        std::vector<DmDeviceInfo> deviceInfoVec;
        for (int32_t i = 0; i < deviceNum; ++i) {
            DmDeviceInfo deviceInfo;
            DecodeDmDeviceInfo(reply, deviceInfo);
            deviceInfoVec.emplace_back(deviceInfo);
        }
        pRsp->SetDeviceVec(deviceInfoVec);
    }
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(START_DEVICE_DISCOVERY, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcStartDevDiscoveryByIdReq> pReq = std::static_pointer_cast<IpcStartDevDiscoveryByIdReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string extra = pReq->GetFilterOption();
    uint16_t subscribeId = pReq->GetSubscribeId();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, extra.c_str());
    WriteUint16(&request, subscribeId);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(START_DEVICE_DISCOVERY, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return SetRspErrCode(reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(STOP_DEVICE_DISCOVER, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcStopDiscoveryReq> pReq = std::static_pointer_cast<IpcStopDiscoveryReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteUint16(&request, subscribeId);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(STOP_DEVICE_DISCOVER, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return SetRspErrCode(reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(REQUEST_CREDENTIAL, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcSetCredentialReq> pReq = std::static_pointer_cast<IpcSetCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string requestJsonStr = pReq->GetCredentialParam();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, requestJsonStr.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REQUEST_CREDENTIAL, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcSetCredentialRsp> pRsp = std::static_pointer_cast<IpcSetCredentialRsp>(pBaseRsp);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    if (ret == DM_OK) {
        std::string returnJsonStr = (const char *)ReadString(&reply, nullptr);
        pRsp->SetCredentialResult(returnJsonStr);
    }
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_GET_DMFA_INFO, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcSetCredentialReq> pReq = std::static_pointer_cast<IpcSetCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string reqJsonStr = pReq->GetCredentialParam();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, reqJsonStr.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_GET_DMFA_INFO, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcSetCredentialRsp> pRsp = std::static_pointer_cast<IpcSetCredentialRsp>(pBaseRsp);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    if (ret == DM_OK) {
        std::string returnJsonStr = (const char *)ReadString(&reply, nullptr);
        pRsp->SetCredentialResult(returnJsonStr);
    }
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(IMPORT_CREDENTIAL, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcSetCredentialReq> pReq = std::static_pointer_cast<IpcSetCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string credentialInfo = pReq->GetCredentialParam();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, credentialInfo.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(IMPORT_CREDENTIAL, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcSetCredentialRsp> pRsp = std::static_pointer_cast<IpcSetCredentialRsp>(pBaseRsp);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    if (ret == DM_OK) {
        std::string outParaStr = (const char *)ReadString(&reply, nullptr);
        std::map<std::string, std::string> outputResult;
        ParseMapFromJsonString(outParaStr, outputResult);
        if (outputResult[DM_CREDENTIAL_TYPE] == DM_TYPE_MINE) {
            if (ret == DM_OK) {
                pRsp->SetCredentialResult(outputResult[DM_CREDENTIAL_RETURNJSONSTR]);
            }
        }
    }
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_SET_REQUEST(DELETE_CREDENTIAL, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcSetCredentialReq> pReq = std::static_pointer_cast<IpcSetCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deleteInfo = pReq->GetCredentialParam();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, deleteInfo.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(DELETE_CREDENTIAL, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    std::shared_ptr<IpcSetCredentialRsp> pRsp = std::static_pointer_cast<IpcSetCredentialRsp>(pBaseRsp);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    if (ret == DM_OK) {
        std::string outParaStr = (const char *)ReadString(&reply, nullptr);
        std::map<std::string, std::string> outputResult;
        ParseMapFromJsonString(outParaStr, outputResult);
        if (outputResult[DM_CREDENTIAL_TYPE] == DM_TYPE_MINE) {
            pRsp->SetCredentialResult(outputResult[DM_CREDENTIAL_RETURNJSONSTR]);
        }
    }
    pRsp->SetErrCode(ret);
    return DM_OK;
}

ON_IPC_CMD(SERVER_DEVICE_STATE_NOTIFY, IpcIo &reply)
{
    size_t len = 0;
    std::string pkgName = (const char *)ReadString(&reply, &len);
    int32_t state = 0;
    ReadInt32(&reply, &state);
    DmDeviceState deviceState = static_cast<DmDeviceState>(state);
    DmDeviceInfo deviceInfo;
    DecodeDmDeviceInfo(reply, deviceInfo);
    switch (deviceState) {
        case DEVICE_STATE_ONLINE:
            DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, deviceInfo);
            break;
        case DEVICE_STATE_OFFLINE:
            DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, deviceInfo);
            break;
        case DEVICE_INFO_CHANGED:
            DeviceManagerNotify::GetInstance().OnDeviceChanged(pkgName, deviceInfo);
            break;
        default:
            LOGE("unknown device state:%{public}d", deviceState);
            break;
    }
}

ON_IPC_CMD(SERVER_DEVICE_FOUND, IpcIo &reply)
{
    size_t len = 0;
    std::string pkgName = (const char *)ReadString(&reply, &len);
    uint16_t subscribeId = 0;
    ReadUint16(&reply, &subscribeId);
    DmDeviceInfo deviceInfo;
    DecodeDmDeviceInfo(reply, deviceInfo);
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, deviceInfo);
}

ON_IPC_CMD(SERVER_DISCOVER_FINISH, IpcIo &reply)
{
    size_t len = 0;
    std::string pkgName = (const char *)ReadString(&reply, &len);
    uint16_t subscribeId = 0;
    ReadUint16(&reply, &subscribeId);
    int32_t failedReason = 0;
    ReadInt32(&reply, &failedReason);

    if (pkgName == "" || len == 0) {
        LOGE("OnDiscoverySuccess, get para failed");
        return;
    }
    if (failedReason == DM_OK) {
        DeviceManagerNotify::GetInstance().OnDiscoverySuccess(pkgName, subscribeId);
    } else {
        DeviceManagerNotify::GetInstance().OnDiscoveryFailed(pkgName, subscribeId, failedReason);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
