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

#include "device_manager_service.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_verify_auth_result_req.h"
#include "ipc_server_stub.h"

namespace OHOS {
namespace DistributedHardware {
ON_IPC_SET_REQUEST(SERVER_DEVICE_STATE_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::static_pointer_cast<IpcNotifyDeviceStateReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t deviceState = pReq->GetDeviceState();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteInt32(&request, deviceState);
    bool ret = WriteRawData(&request, &deviceInfo, sizeof(DmDeviceInfo));
    if (!ret) {
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_STATE_NOTIFY, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
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

ON_IPC_SET_REQUEST(SERVER_DEVICE_FOUND, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::static_pointer_cast<IpcNotifyDeviceFoundReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteUint16(&request, subscribeId);
    bool ret = WriteRawData(&request, &deviceInfo, sizeof(DmDeviceInfo));
    if (!ret) {
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_FOUND, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
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

ON_IPC_SET_REQUEST(SERVER_DISCOVER_FINISH, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::static_pointer_cast<IpcNotifyDiscoverResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    int32_t result = pReq->GetResult();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteUint16(&request, subscribeId);
    WriteInt32(&request, result);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DISCOVER_FINISH, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
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

ON_IPC_SET_REQUEST(SERVER_AUTH_RESULT, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcNotifyAuthResultReq> pReq = std::static_pointer_cast<IpcNotifyAuthResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    std::string token = pReq->GetPinToken();
    int32_t status = pReq->GetStatus();
    int32_t reason = pReq->GetReason();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, deviceId.c_str());
    WriteString(&request, token.c_str());
    WriteInt32(&request, status);
    WriteInt32(&request, reason);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_AUTH_RESULT, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
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

ON_IPC_SET_REQUEST(SERVER_VERIFY_AUTH_RESULT, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcNotifyVerifyAuthResultReq> pReq =
        std::static_pointer_cast<IpcNotifyVerifyAuthResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    int32_t result = pReq->GetResult();
    int32_t flag = pReq->GetFlag();

    IpcIoInit(&request, buffer, buffLen, 0);
    WriteString(&request, pkgName.c_str());
    WriteString(&request, deviceId.c_str());
    WriteInt32(&request, result);
    WriteInt32(&request, flag);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_VERIFY_AUTH_RESULT, IpcIo &reply, std::shared_ptr<IpcRsp> pBaseRsp)
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

ON_IPC_SERVER_CMD(REGISTER_DEVICE_MANAGER_LISTENER, IpcIo &req, IpcIo &reply)
{
    int32_t errCode = RegisterDeviceManagerListener(&req, &reply);
    WriteInt32(&reply, errCode);
}

ON_IPC_SERVER_CMD(UNREGISTER_DEVICE_MANAGER_LISTENER, IpcIo &req, IpcIo &reply)
{
    int32_t errCode = UnRegisterDeviceManagerListener(&req, &reply);
    WriteInt32(&reply, errCode);
}

ON_IPC_SERVER_CMD(GET_TRUST_DEVICE_LIST, IpcIo &req, IpcIo &reply)
{
    LOGI("enter GetTrustedDeviceList.");
    std::string pkgName = (const char *)ReadString(&req, nullptr);
    std::string extra = (const char *)ReadString(&req, nullptr);

    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    WriteInt32(&reply, deviceList.size());
    if (deviceList.size() > 0) {
        bool value = WriteRawData(&reply, deviceList.data(), sizeof(DmDeviceInfo) * deviceList.size());
        if (!value) {
            return;
        }
    }
    WriteInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(GET_LOCAL_DEVICE_INFO, IpcIo &req, IpcIo &reply)
{
    LOGI("enter GetLocalDeviceInfo.");
    DmDeviceInfo dmDeviceInfo;
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(dmDeviceInfo);
    bool value = WriteRawData(&reply, &dmDeviceInfo, sizeof(DmDeviceInfo));
    if (!value) {
        return;
    }
    WriteInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(START_DEVICE_DISCOVER, IpcIo &req, IpcIo &reply)
{
    LOGI("StartDeviceDiscovery service listener.");
    std::string pkgName = (const char *)ReadString(&req, nullptr);
    std::string extra = (const char *)ReadString(&req, nullptr);
    DmSubscribeInfo *pDmSubscribeInfo = (DmSubscribeInfo *)ReadRawData(&req, sizeof(DmSubscribeInfo));
    int32_t ret = DeviceManagerService::GetInstance().StartDeviceDiscovery(pkgName, *pDmSubscribeInfo, extra);
    WriteInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(STOP_DEVICE_DISCOVER, IpcIo &req, IpcIo &reply)
{
    LOGI("StopDeviceDiscovery service listener.");
    std::string pkgName = (const char *)ReadString(&req, nullptr);
    uint16_t subscribeId = 0;
    ReadUint16(&reply, &subscribeId);
    int32_t ret = DeviceManagerService::GetInstance().StopDeviceDiscovery(pkgName, subscribeId);
    WriteInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(AUTHENTICATE_DEVICE, IpcIo &req, IpcIo &reply)
{
    LOGI("AuthenticateDevice service listener.");
    std::string pkgName = (const char *)ReadString(&req, nullptr);
    std::string extra = (const char *)ReadString(&req, nullptr);
    std::string deviceId = (const char *)ReadString(&req, nullptr);
    int32_t authType = 0;
    ReadInt32(&reply, &authType);
    int32_t ret = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    WriteInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(UNAUTHENTICATE_DEVICE, IpcIo &req, IpcIo &reply)
{
    LOGI("UnAuthenticateDevice service listener.");
    std::string pkgName = (const char *)ReadString(&req, nullptr);
    std::string deviceId = (const char *)ReadString(&req, nullptr);

    int32_t ret = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, deviceId);
    WriteInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(VERIFY_AUTHENTICATION, IpcIo &req, IpcIo &reply)
{
    LOGI("VerifyAuthentication service listener.");
    std::string authParam = (const char *)ReadString(&req, nullptr);
    int32_t ret = DeviceManagerService::GetInstance().VerifyAuthentication(authParam);
    WriteInt32(&reply, ret);
}

ON_IPC_SERVER_CMD(SERVER_USER_AUTH_OPERATION, IpcIo &req, IpcIo &reply)
{
    size_t len = 0;
    std::string packName = (const char *)ReadString(&req, &len);
    int32_t action = 0;
    ReadInt32(&reply, &action);
    DeviceManagerService::GetInstance().SetUserOperation(packName, action);
    WriteInt32(&reply, action);
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_FA_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, IpcIo &request, uint8_t *buffer,
                   size_t buffLen)
{
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::static_pointer_cast<IpcNotifyDMFAResultReq>(pBaseReq);
    std::string packagname = pReq->GetPkgName();
    std::string paramJson = pReq->GetJsonParam();
    WriteString(&request, packagname.c_str());
    WriteString(&request, paramJson.c_str());
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
