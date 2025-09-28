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
#include "dm_common_util.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_subscribe_info.h"
#include "ipc_cmd_register.h"
#include "ipc_common_param_req.h"
#include "ipc_def.h"
#include "ipc_get_device_info_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_register_listener_req.h"
#include "ipc_set_credential_req.h"
#include "ipc_set_credential_rsp.h"
#include "ipc_sync_callback_req.h"
#include "securec.h"

namespace OHOS {
namespace DistributedHardware {
void DecodeDmDeviceInfo(IpcIo &reply, DmDeviceInfo &devInfo)
{
    size_t len = 0;
    std::string deviceIdStr = (const char *)ReadString(&reply, &len);
    if (strcpy_s(devInfo.deviceId, deviceIdStr.size() + 1, deviceIdStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceId failed!");
        return;
    }
    std::string deviceNameStr = (const char *)ReadString(&reply, &len);
    if (strcpy_s(devInfo.deviceName, deviceNameStr.size() + 1, deviceNameStr.c_str()) != DM_OK) {
        LOGE("strcpy_s deviceName failed!");
        return;
    }
    ReadUint16(&reply, &devInfo.deviceTypeId);
    std::string networkIdStr = (const char *)ReadString(&reply, &len);
    if (strcpy_s(devInfo.networkId, networkIdStr.size() + 1, networkIdStr.c_str()) != DM_OK) {
        LOGE("strcpy_s networkId failed!");
        return;
    }
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

ON_IPC_SET_REQUEST(SYNC_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcSyncCallbackReq> pReq = std::static_pointer_cast<IpcSyncCallbackReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t dmCommonNotifyEvent = pReq->GetDmCommonNotifyEvent();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteInt32(&request, dmCommonNotifyEvent);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SYNC_CALLBACK, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return SetRspErrCode(reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(START_DISCOVERING, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    LOGI("start.");
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string discParaStr = pReq->GetFirstParam();
    std::string filterOpStr = pReq->GetSecondParam();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, discParaStr.c_str());
    WriteString(&request, filterOpStr.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(START_DISCOVERING, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    LOGI("start.");
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    return SetRspErrCode(reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(STOP_DISCOVERING, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    LOGI("start.");
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string discParaStr = pReq->GetFirstParam();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, discParaStr.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(STOP_DISCOVERING, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    LOGI("start.");
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    return SetRspErrCode(reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(GET_LOCAL_DEVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DEVICE_INFO, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcGetLocalDeviceInfoRsp> pRsp = std::static_pointer_cast<IpcGetLocalDeviceInfoRsp>(pBaseRsp);
    DmDeviceInfo deviceInfo;
    DecodeDmDeviceInfo(reply, deviceInfo);
    int32_t ret = 0;
    ReadInt32(&reply, &ret);
    pRsp->SetLocalDeviceInfo(deviceInfo);
    pRsp->SetErrCode(ret);
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
