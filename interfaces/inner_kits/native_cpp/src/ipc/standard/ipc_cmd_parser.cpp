/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "dm_device_profile_info.h"
#include "dm_log.h"
#include "ipc_acl_profile_req.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_bind_device_req.h"
#include "ipc_bind_target_req.h"
#include "ipc_check_access_control.h"
#include "ipc_cmd_register.h"
#include "ipc_common_param_req.h"
#include "ipc_create_pin_holder_req.h"
#include "ipc_destroy_pin_holder_req.h"
#include "ipc_def.h"
#include "ipc_export_auth_code_rsp.h"
#include "ipc_generate_encrypted_uuid_req.h"
#include "ipc_get_anony_local_udid_rsp.h"
#include "ipc_get_authType_by_udidhash_req.h"
#include "ipc_get_authType_by_udidhash_rsp.h"
#include "ipc_get_device_icon_info_req.h"
#include "ipc_get_device_info_rsp.h"
#include "ipc_get_device_network_id_list_req.h"
#include "ipc_get_device_network_id_list_rsp.h"
#include "ipc_get_device_profile_info_list_req.h"
#include "ipc_get_device_screen_status_req.h"
#include "ipc_get_device_screen_status_rsp.h"
#include "ipc_get_encrypted_uuid_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_local_device_name_rsp.h"
#include "ipc_get_local_display_device_name_req.h"
#include "ipc_get_local_display_device_name_rsp.h"
#include "ipc_get_localserviceinfo_rsp.h"
#include "ipc_get_local_serviceinfo_ext_rsp.h"
#include "ipc_get_peerserviceinfo_by_serviceid_req.h"
#include "ipc_get_register_serviceinfo_rsp.h"
#include "ipc_notify_dmfa_result_req.h"
#include "ipc_get_trust_service_info_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_get_udids_by_deviceIds_req.h"
#include "ipc_get_udids_by_deviceIds_rsp.h"
#include "ipc_import_auth_code_req.h"
#include "ipc_model_codec.h"
#include "ipc_notify_event_req.h"
#include "ipc_put_device_profile_info_list_req.h"
#include "ipc_register_listener_req.h"
#include "ipc_register_serviceinfo_req.h"
#include "ipc_req.h"
#include "ipc_rsp.h"
#include "ipc_set_credential_req.h"
#include "ipc_set_credential_rsp.h"
#include "ipc_set_local_device_name_req.h"
#include "ipc_set_remote_device_name_req.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_start_service_discovery_req.h"
#include "ipc_sync_callback_req.h"
#include "ipc_sync_service_callback_req.h"
#include "ipc_start_publish_service_req.h"
#include "ipc_stop_publish_service_req.h"
#include "ipc_permission_req.h"
#include "ipc_publish_req.h"
#include "ipc_publish_service_info_rsp.h"
#include "ipc_unbind_device_req.h"
#include "ipc_unbind_service_target_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_unregister_service_info_req.h"
#include "ipc_update_service_info_req.h"
#include "securec.h"
#include "ipc_auth_info_req.h"
#include "ipc_auth_info_rsp.h"
#include "ipc_register_service_info_req.h"
#include "ipc_sync_service_info_req.h"
#include "ipc_sync_service_info_result_req.h"
namespace OHOS { class IRemoteObject; }

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t DM_MAX_TRUST_DEVICE_NUM = 200;
}
int32_t SetRequest(const DMIpcCmdInterfaceCode &ipcCode, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    LOGI("start ipcCode %{public}d.", static_cast<int32_t>(ipcCode));
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCheckAcl> pReq = std::static_pointer_cast<IpcCheckAcl>(pBaseReq);
    DmAccessCaller caller = pReq->GetAccessCaller();
    DmAccessCallee callee = pReq->GetAccessCallee();
    if (!IpcModelCodec::EncodeDmAccessCaller(caller, data)) {
        LOGE("write caller failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodeDmAccessCallee(callee, data)) {
        LOGE("write caller failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

int32_t ReadResponse(const DMIpcCmdInterfaceCode &ipcCode, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    LOGI("start ipcCode %{public}d.", static_cast<int32_t>(ipcCode));
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(static_cast<int32_t>(reply.ReadBool()));
    return DM_OK;
}

ON_IPC_SET_REQUEST(REGISTER_DEVICE_MANAGER_LISTENER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcRegisterListenerReq> pReq = std::static_pointer_cast<IpcRegisterListenerReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    sptr<IRemoteObject> listener = pReq->GetListener();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteRemoteObject(listener)) {
        LOGE("write listener failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNREGISTER_DEVICE_MANAGER_LISTENER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::string pkgName = pBaseReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write papam failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNREGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_TRUST_DEVICE_LIST, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetTrustDeviceReq> pReq = std::static_pointer_cast<IpcGetTrustDeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string extra = pReq->GetExtra();
    bool isRefresh = pReq->GetRefresh();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkg failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(extra)) {
        LOGE("write extra failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteBool(isRefresh)) {
        LOGE("write isRefresh failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_TRUST_DEVICE_LIST, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetTrustDeviceRsp> pRsp = std::static_pointer_cast<IpcGetTrustDeviceRsp>(pBaseRsp);
    int32_t deviceNum = reply.ReadInt32();
    if (deviceNum > 0 && deviceNum <= DM_MAX_TRUST_DEVICE_NUM) {
        std::vector<DmDeviceInfo> deviceInfoVec;
        for (int32_t i = 0; i < deviceNum; ++i) {
            DmDeviceInfo deviceInfo;
            IpcModelCodec::DecodeDmDeviceInfo(reply, deviceInfo);
            deviceInfoVec.emplace_back(deviceInfo);
        }
        pRsp->SetDeviceVec(deviceInfoVec);
    }
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_DEVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkReq> pReq = std::static_pointer_cast<IpcGetInfoByNetWorkReq>(pBaseReq);
    std::string networkId = pReq->GetNetWorkId();
    if (!data.WriteString(networkId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_ALL_TRUST_DEVICE_LIST, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetTrustDeviceReq> pReq = std::static_pointer_cast<IpcGetTrustDeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string extra = pReq->GetExtra();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkg failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(extra)) {
        LOGE("write extra failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_ALL_TRUST_DEVICE_LIST, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetTrustDeviceRsp> pRsp = std::static_pointer_cast<IpcGetTrustDeviceRsp>(pBaseRsp);
    int32_t deviceNum = reply.ReadInt32();
    if (deviceNum > 0 && deviceNum <= DM_MAX_TRUST_DEVICE_NUM) {
        std::vector<DmDeviceInfo> deviceInfoVec;
        for (int32_t i = 0; i < deviceNum; ++i) {
            DmDeviceInfo deviceInfo;
            IpcModelCodec::DecodeDmDeviceInfo(reply, deviceInfo);
            deviceInfoVec.emplace_back(deviceInfo);
        }
        pRsp->SetDeviceVec(deviceInfoVec);
    }
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_DEVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetDeviceInfoRsp> pRsp = std::static_pointer_cast<IpcGetDeviceInfoRsp>(pBaseRsp);
    DmDeviceInfo deviceInfo;
    IpcModelCodec::DecodeDmDeviceInfo(reply, deviceInfo);
    pRsp->SetDeviceInfo(deviceInfo);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_LOCAL_DEVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DEVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetLocalDeviceInfoRsp> pRsp = std::static_pointer_cast<IpcGetLocalDeviceInfoRsp>(pBaseRsp);
    DmDeviceInfo localDeviceInfo;
    IpcModelCodec::DecodeDmDeviceInfo(reply, localDeviceInfo);
    pRsp->SetLocalDeviceInfo(localDeviceInfo);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_UDID_BY_NETWORK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkReq> pReq = std::static_pointer_cast<IpcGetInfoByNetWorkReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string netWorkId = pReq->GetNetWorkId();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(netWorkId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_UDID_BY_NETWORK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkRsp> pRsp = std::static_pointer_cast<IpcGetInfoByNetWorkRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetUdid(reply.ReadString());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_UUID_BY_NETWORK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkReq> pReq = std::static_pointer_cast<IpcGetInfoByNetWorkReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string netWorkId = pReq->GetNetWorkId();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(netWorkId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_UUID_BY_NETWORK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkRsp> pRsp = std::static_pointer_cast<IpcGetInfoByNetWorkRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetUuid(reply.ReadString());
    return DM_OK;
}

ON_IPC_SET_REQUEST(PUBLISH_DEVICE_DISCOVER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcPublishReq> pReq = std::static_pointer_cast<IpcPublishReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    const DmPublishInfo dmPublishInfo = pReq->GetPublishInfo();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteRawData(&dmPublishInfo, sizeof(DmPublishInfo))) {
        LOGE("write dmPublishInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(PUBLISH_DEVICE_DISCOVER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNPUBLISH_DEVICE_DISCOVER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcUnPublishReq> pReq = std::static_pointer_cast<IpcUnPublishReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t publishId = pReq->GetPublishId();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(publishId)) {
        LOGE("write publishId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNPUBLISH_DEVICE_DISCOVER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(AUTHENTICATE_DEVICE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcAuthenticateDeviceReq> pReq = std::static_pointer_cast<IpcAuthenticateDeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string extra = pReq->GetExtra();
    int32_t authType = pReq->GetAuthType();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    std::string deviceId = deviceInfo.deviceId;

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(extra)) {
        LOGE("write extra failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(deviceId)) {
        LOGE("write deviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(authType)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(AUTHENTICATE_DEVICE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNAUTHENTICATE_DEVICE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcUnAuthenticateDeviceReq> pReq = std::static_pointer_cast<IpcUnAuthenticateDeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    std::string networkId = deviceInfo.networkId;
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(networkId)) {
        LOGE("write extra failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNAUTHENTICATE_DEVICE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_USER_AUTH_OPERATION, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetOperationReq> pReq = std::static_pointer_cast<IpcGetOperationReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t action = pReq->GetOperation();
    std::string params = pReq->GetParams();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(action)) {
        LOGE("write action failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(params)) {
        LOGE("write params failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }

    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_USER_AUTH_OPERATION, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(SERVER_DEVICE_STATE_NOTIFY, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmDeviceState deviceState = static_cast<DmDeviceState>(data.ReadInt32());
    DmDeviceInfo dmDeviceInfo;
    IpcModelCodec::DecodeDmDeviceInfo(data, dmDeviceInfo);

    DmDeviceBasicInfo dmDeviceBasicInfo;
    IpcModelCodec::DecodeDmDeviceBasicInfo(data, dmDeviceBasicInfo);
    std::vector<int64_t> serviceIds;
    IpcModelCodec::DecodeServiceIds(serviceIds, data);
    switch (deviceState) {
        case DEVICE_STATE_ONLINE:
            LOGI("Online pkgName:%{public}s", pkgName.c_str());
            DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, dmDeviceInfo);
            DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, dmDeviceBasicInfo);
            break;
        case DEVICE_STATE_OFFLINE:
            LOGI("Offline pkgName:%{public}s", pkgName.c_str());
            DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, dmDeviceInfo);
            DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, dmDeviceBasicInfo);
            break;
        case DEVICE_INFO_CHANGED:
            DeviceManagerNotify::GetInstance().OnDeviceChanged(pkgName, dmDeviceInfo);
            DeviceManagerNotify::GetInstance().OnDeviceChanged(pkgName, dmDeviceBasicInfo);
            break;
        case DEVICE_INFO_READY:
            LOGI("OnDeviceReady in, pkgName:%{public}s, networkId: %{public}s.",
                pkgName.c_str(), GetAnonyString(dmDeviceInfo.networkId).c_str());
            DeviceManagerNotify::GetInstance().OnDeviceReady(pkgName, dmDeviceInfo);
            DeviceManagerNotify::GetInstance().OnDeviceReady(pkgName, dmDeviceBasicInfo);
            break;
        default:
            LOGE("unknown device state:%{public}d", deviceState);
            break;
    }
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SERVER_DEVICE_FOUND, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int16_t subscribeId = data.ReadInt16();
    DmDeviceInfo dmDeviceInfo;
    IpcModelCodec::DecodeDmDeviceInfo(data, dmDeviceInfo);
    DmDeviceBasicInfo devBasicInfo;
    IpcModelCodec::DecodeDmDeviceBasicInfo(data, devBasicInfo);
    LOGD("pkgName:%{public}s, subscribeId:%{public}d.", GetAnonyString(pkgName).c_str(), (int32_t)subscribeId);
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, dmDeviceInfo);
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, devBasicInfo);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SERVER_DEVICE_DISCOVERY, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int16_t subscribeId = data.ReadInt16();
    DmDeviceBasicInfo dmDeviceBasicInfo;
    IpcModelCodec::DecodeDmDeviceBasicInfo(data, dmDeviceBasicInfo);
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, dmDeviceBasicInfo);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SERVER_DISCOVER_FINISH, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int16_t subscribeId = data.ReadInt16();
    int32_t failedReason = data.ReadInt32();

    if (failedReason == DM_OK) {
        DeviceManagerNotify::GetInstance().OnDiscoverySuccess(pkgName, subscribeId);
    } else {
        DeviceManagerNotify::GetInstance().OnDiscoveryFailed(pkgName, subscribeId, failedReason);
    }
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SERVER_PUBLISH_FINISH, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t publishId = data.ReadInt32();
    int32_t publishResult = data.ReadInt32();

    DeviceManagerNotify::GetInstance().OnPublishResult(pkgName, publishId, publishResult);
    if (!reply.WriteInt32(DM_OK)) {
        LOGE("write return failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(SERVER_AUTH_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string deviceId = data.ReadString();
    std::string token = data.ReadString();
    int32_t status = data.ReadInt32();
    int32_t reason = data.ReadInt32();
    DeviceManagerNotify::GetInstance().OnAuthResult(pkgName, deviceId, token, (uint32_t)status, reason);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SERVER_DEVICE_FA_NOTIFY, MessageParcel &data, MessageParcel &reply)
{
    std::string packagename = data.ReadString();
    std::string paramJson = data.ReadString();
    DeviceManagerNotify::GetInstance().OnUiCall(packagename, paramJson);
    if (!reply.WriteInt32(DM_OK)) {
        LOGE("write return failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(REQUEST_CREDENTIAL, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcSetCredentialReq> pReq = std::static_pointer_cast<IpcSetCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string requestJsonStr = pReq->GetCredentialParam();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkg failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(requestJsonStr)) {
        LOGE("write requestJsonStr failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REQUEST_CREDENTIAL, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcSetCredentialRsp> pRsp = std::static_pointer_cast<IpcSetCredentialRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    if (pRsp->GetErrCode() == DM_OK) {
        std::string returnJsonStr = reply.ReadString();
        pRsp->SetCredentialResult(returnJsonStr);
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_GET_DMFA_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcSetCredentialReq> pReq = std::static_pointer_cast<IpcSetCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string reqJsonStr = pReq->GetCredentialParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkg failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(reqJsonStr)) {
        LOGE("write returnJsonStr failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_GET_DMFA_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcSetCredentialRsp> pRsp = std::static_pointer_cast<IpcSetCredentialRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    if (pRsp->GetErrCode() == DM_OK) {
        std::string returnJsonStr = reply.ReadString();
        pRsp->SetCredentialResult(returnJsonStr);
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(IMPORT_CREDENTIAL, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcSetCredentialReq> pReq = std::static_pointer_cast<IpcSetCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string credentialInfo = pReq->GetCredentialParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkg failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(credentialInfo)) {
        LOGE("write credentialInfo failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(IMPORT_CREDENTIAL, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::string outParaStr = reply.ReadString();
    std::map<std::string, std::string> outputResult;
    ParseMapFromJsonString(outParaStr, outputResult);
    if (outputResult[DM_CREDENTIAL_TYPE] == DM_TYPE_OH) {
        pBaseRsp->SetErrCode(reply.ReadInt32());
    }
    if (outputResult[DM_CREDENTIAL_TYPE] == DM_TYPE_MINE) {
        std::shared_ptr<IpcSetCredentialRsp> pRsp = std::static_pointer_cast<IpcSetCredentialRsp>(pBaseRsp);
        pRsp->SetErrCode(reply.ReadInt32());
        if (pRsp->GetErrCode() == DM_OK) {
            pRsp->SetCredentialResult(outputResult[DM_CREDENTIAL_RETURNJSONSTR]);
        }
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(DELETE_CREDENTIAL, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcSetCredentialReq> pReq = std::static_pointer_cast<IpcSetCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deleteInfo = pReq->GetCredentialParam();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkg failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(deleteInfo)) {
        LOGE("write deleteInfo failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(DELETE_CREDENTIAL, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::string outParaStr = reply.ReadString();
    std::map<std::string, std::string> outputResult;
    ParseMapFromJsonString(outParaStr, outputResult);
    if (outputResult[DM_CREDENTIAL_TYPE] == DM_TYPE_OH) {
        pBaseRsp->SetErrCode(reply.ReadInt32());
    }
    if (outputResult[DM_CREDENTIAL_TYPE] == DM_TYPE_MINE) {
        std::shared_ptr<IpcSetCredentialRsp> pRsp = std::static_pointer_cast<IpcSetCredentialRsp>(pBaseRsp);
        pRsp->SetErrCode(reply.ReadInt32());
        if (pRsp->GetErrCode() == DM_OK) {
            pRsp->SetCredentialResult(outputResult[DM_CREDENTIAL_RETURNJSONSTR]);
        }
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(REGISTER_CREDENTIAL_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }

    return DM_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_CREDENTIAL_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNREGISTER_CREDENTIAL_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }

    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNREGISTER_CREDENTIAL_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(SERVER_CREDENTIAL_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t action = data.ReadInt32();
    std::string credentialResult = data.ReadString();

    DeviceManagerNotify::GetInstance().OnCredentialResult(pkgName, action, credentialResult);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(NOTIFY_EVENT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyEventReq> pReq = std::static_pointer_cast<IpcNotifyEventReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t eventId = pReq->GetEventId();
    std::string event = pReq->GetEvent();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(eventId)) {
        LOGE("write eventId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(event)) {
        LOGE("write event failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(NOTIFY_EVENT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_ENCRYPTED_UUID_BY_NETWOEKID, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetEncryptedUuidReq> pReq = std::static_pointer_cast<IpcGetEncryptedUuidReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string netWorkId = pReq->GetNetworkId();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(netWorkId)) {
        LOGE("write netWorkId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_ENCRYPTED_UUID_BY_NETWOEKID, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkRsp> pRsp = std::static_pointer_cast<IpcGetInfoByNetWorkRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetUuid(reply.ReadString());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GENERATE_ENCRYPTED_UUID, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGenerateEncryptedUuidReq> pReq = std::static_pointer_cast<IpcGenerateEncryptedUuidReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string uuid = pReq->GetUuid();
    std::string appId = pReq->GetAppId();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(uuid)) {
        LOGE("write uuid failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(appId)) {
        LOGE("write appId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GENERATE_ENCRYPTED_UUID, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkRsp> pRsp = std::static_pointer_cast<IpcGetInfoByNetWorkRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetUuid(reply.ReadString());
    return DM_OK;
}

ON_IPC_SET_REQUEST(BIND_DEVICE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcBindDeviceReq> pReq = std::static_pointer_cast<IpcBindDeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string bindParam = pReq->GetBindParam();
    int32_t bindType = pReq->GetBindType();
    std::string deviceId = pReq->GetDeviceId();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(bindParam)) {
        LOGE("write bindParam failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(deviceId)) {
        LOGE("write deviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(bindType)) {
        LOGE("write bindType failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(BIND_DEVICE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNBIND_DEVICE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcUnBindDeviceReq> pReq = std::static_pointer_cast<IpcUnBindDeviceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    std::string extra = pReq->GetExtraInfo();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(deviceId)) {
        LOGE("write deviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(extra)) {
        LOGE("write extra failed");
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNBIND_DEVICE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_NETWORKTYPE_BY_NETWORK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkReq> pReq = std::static_pointer_cast<IpcGetInfoByNetWorkReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string netWorkId = pReq->GetNetWorkId();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(netWorkId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_NETWORKTYPE_BY_NETWORK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkRsp> pRsp = std::static_pointer_cast<IpcGetInfoByNetWorkRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetNetworkType(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(REGISTER_UI_STATE_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_UI_STATE_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNREGISTER_UI_STATE_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNREGISTER_UI_STATE_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(IMPORT_AUTH_CODE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcImportAuthCodeReq> pReq = std::static_pointer_cast<IpcImportAuthCodeReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string authCode = pReq->GetAuthCode();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(authCode)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(IMPORT_AUTH_CODE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(EXPORT_AUTH_CODE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    LOGI("send export auth code request!");
    return DM_OK;
}

ON_IPC_READ_RESPONSE(EXPORT_AUTH_CODE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcExportAuthCodeRsp> pRsp = std::static_pointer_cast<IpcExportAuthCodeRsp>(pBaseRsp);
    CHECK_NULL_RETURN(pRsp, ERR_DM_FAILED);
    std::string authCode = reply.ReadString();
    pRsp->SetAuthCode(authCode);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(IMPORT_AUTH_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcAuthInfoReq> pReq = std::static_pointer_cast<IpcAuthInfoReq>(pBaseReq);
    const DmAuthInfo &info = pReq->GetDmAuthInfo();
    if (!IpcModelCodec::EncodeDmAuthInfo(info, data)) {
        LOGE("EncodeDmAuthInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(IMPORT_AUTH_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(EXPORT_AUTH_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcAuthInfoReq> pReq = std::static_pointer_cast<IpcAuthInfoReq>(pBaseReq);
    const DmAuthInfo &info = pReq->GetDmAuthInfo();
    uint32_t pinLength = pReq->GetPinLength();
    if (!IpcModelCodec::EncodeDmAuthInfo(info, data)) {
        LOGE("EncodeDmAuthInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteUint32(pinLength)) {
        LOGE("WriteUint32 length failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(EXPORT_AUTH_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcAuthInfoRsp> pRsp = std::static_pointer_cast<IpcAuthInfoRsp>(pBaseRsp);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    if (pBaseRsp->GetErrCode() == DM_OK) {
        DmAuthInfo info;
        bool ret = IpcModelCodec::DecodeDmAuthInfo(reply, info);
        if (!ret) {
            LOGE("DecodeDmAuthInfo failed");
            pRsp->SetErrCode(ERR_DM_IPC_READ_FAILED);
            return ERR_DM_IPC_READ_FAILED;
        }
        pRsp->SetDmAuthInfo(info);
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(REGISTER_DISCOVERY_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string discParaStr = pReq->GetFirstParam();
    std::string filterOpStr = pReq->GetSecondParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(discParaStr)) {
        LOGE("write discovery parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(filterOpStr)) {
        LOGE("write filter option parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_DISCOVERY_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNREGISTER_DISCOVERY_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string extraParaStr = pReq->GetFirstParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(extraParaStr)) {
        LOGE("write extra parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNREGISTER_DISCOVERY_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(START_DISCOVERING, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string discParaStr = pReq->GetFirstParam();
    std::string filterOpStr = pReq->GetSecondParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(discParaStr)) {
        LOGE("write discovery parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(filterOpStr)) {
        LOGE("write filter option parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(START_DISCOVERING, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(STOP_DISCOVERING, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string discParaStr = pReq->GetFirstParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(discParaStr)) {
        LOGE("write discovery parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(STOP_DISCOVERING, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(START_ADVERTISING, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string adverParaStr = pReq->GetFirstParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(adverParaStr)) {
        LOGE("write advertising parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(START_ADVERTISING, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(STOP_ADVERTISING, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string adverParaStr = pReq->GetFirstParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(adverParaStr)) {
        LOGE("write advertising parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(STOP_ADVERTISING, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(BIND_TARGET, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcBindTargetReq> pReq = std::static_pointer_cast<IpcBindTargetReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    PeerTargetId targetId = pReq->GetPeerTargetId();
    std::string bindParam = pReq->GetBindParam();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodePeerTargetId(targetId, data)) {
        LOGE("write peer target id failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(bindParam)) {
        LOGE("write bind parameter string failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(BIND_TARGET, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNBIND_TARGET, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcBindTargetReq> pReq = std::static_pointer_cast<IpcBindTargetReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    PeerTargetId targetId = pReq->GetPeerTargetId();
    std::string unbindParam = pReq->GetBindParam();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodePeerTargetId(targetId, data)) {
        LOGE("write peer target id failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(unbindParam)) {
        LOGE("write unbind parameter string failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNBIND_TARGET, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(BIND_TARGET_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    PeerTargetId targetId;
    IpcModelCodec::DecodePeerTargetId(data, targetId);
    int32_t result = data.ReadInt32();
    int32_t status = data.ReadInt32();
    std::string content = data.ReadString();

    DeviceManagerNotify::GetInstance().OnBindResult(pkgName, targetId, result, status, content);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(UNBIND_TARGET_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    PeerTargetId targetId;
    IpcModelCodec::DecodePeerTargetId(data, targetId);
    int32_t result = data.ReadInt32();
    std::string content = data.ReadString();

    DeviceManagerNotify::GetInstance().OnUnbindResult(pkgName, targetId, result, content);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(REGISTER_PIN_HOLDER_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_PIN_HOLDER_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(CREATE_PIN_HOLDER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCreatePinHolderReq> pReq = std::static_pointer_cast<IpcCreatePinHolderReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    PeerTargetId targetId = pReq->GetPeerTargetId();
    std::string payload = pReq->GetPayload();
    int32_t pinType = pReq->GetPinType();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodePeerTargetId(targetId, data)) {
        LOGE("write peer target id failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(payload)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pinType)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(CREATE_PIN_HOLDER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(DESTROY_PIN_HOLDER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcDestroyPinHolderReq> pReq = std::static_pointer_cast<IpcDestroyPinHolderReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    PeerTargetId targetId = pReq->GetPeerTargetId();
    int32_t pinType = pReq->GetPinType();
    std::string payload = pReq->GetPayload();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodePeerTargetId(targetId, data)) {
        LOGE("write peer target id failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pinType)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(payload)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(DESTROY_PIN_HOLDER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SET_DN_POLICY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string policy = pReq->GetFirstParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(policy)) {
        LOGE("write setDnPolicy parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SET_DN_POLICY, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(STOP_AUTHENTICATE_DEVICE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(STOP_AUTHENTICATE_DEVICE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(SERVER_CREATE_PIN_HOLDER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string deviceId = data.ReadString();
    DmPinType pinType = static_cast<DmPinType>(data.ReadInt32());
    std::string payload = data.ReadString();

    DeviceManagerNotify::GetInstance().OnPinHolderCreate(pkgName, deviceId, pinType, payload);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SERVER_DESTROY_PIN_HOLDER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmPinType pinType = static_cast<DmPinType>(data.ReadInt32());
    std::string payload = data.ReadString();

    DeviceManagerNotify::GetInstance().OnPinHolderDestroy(pkgName, pinType, payload);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SERVER_CREATE_PIN_HOLDER_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = data.ReadInt32();

    DeviceManagerNotify::GetInstance().OnCreateResult(pkgName, result);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SERVER_DESTROY_PIN_HOLDER_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = data.ReadInt32();

    DeviceManagerNotify::GetInstance().OnDestroyResult(pkgName, result);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(DP_ACL_ADD, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcAclProfileReq> pReq = std::static_pointer_cast<IpcAclProfileReq>(pBaseReq);
    std::string udid = pReq->GetStr();
    int64_t accessControlId = pReq->GetAccessControlId();
    if (!data.WriteString(udid)) {
        LOGE("write udid failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt64(accessControlId)) {
        LOGE("write accessControlId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(DP_ACL_ADD, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_SECURITY_LEVEL, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkReq> pReq = std::static_pointer_cast<IpcGetInfoByNetWorkReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string networkId = pReq->GetNetWorkId();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(networkId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_SECURITY_LEVEL, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkRsp> pRsp = std::static_pointer_cast<IpcGetInfoByNetWorkRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetSecurityLevel(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(SERVER_ON_PIN_HOLDER_EVENT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = data.ReadInt32();
    DmPinHolderEvent pinHolderEvent = static_cast<DmPinHolderEvent>(data.ReadInt32());
    std::string content = data.ReadString();

    DeviceManagerNotify::GetInstance().OnPinHolderEvent(pkgName, pinHolderEvent, result, content);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(IS_SAME_ACCOUNT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcAclProfileReq> pReq = std::static_pointer_cast<IpcAclProfileReq>(pBaseReq);
    std::string netWorkId = pReq->GetStr();
    if (!data.WriteString(netWorkId)) {
        LOGE("write netWorkId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(IS_SAME_ACCOUNT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(CHECK_API_PERMISSION, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcPermissionReq> pReq = std::static_pointer_cast<IpcPermissionReq>(pBaseReq);
    if (!data.WriteInt32(pReq->GetPermissionLevel())) {
        LOGE("write permissionLevel failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(CHECK_API_PERMISSION, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(CHECK_ACCESS_CONTROL, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    return SetRequest(CHECK_ACCESS_CONTROL, pBaseReq, data);
}

ON_IPC_READ_RESPONSE(CHECK_ACCESS_CONTROL, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return ReadResponse(CHECK_ACCESS_CONTROL, reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(CHECK_SAME_ACCOUNT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    return SetRequest(CHECK_SAME_ACCOUNT, pBaseReq, data);
}

ON_IPC_READ_RESPONSE(CHECK_SAME_ACCOUNT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return ReadResponse(CHECK_SAME_ACCOUNT, reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(SHIFT_LNN_GEAR, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SHIFT_LNN_GEAR, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(REMOTE_DEVICE_TRUST_CHANGE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string udid = data.ReadString();
    int32_t authForm = data.ReadInt32();
    std::string uuid = data.ReadString();
    DeviceManagerNotify::GetInstance().OnDeviceTrustChange(pkgName, udid, uuid, authForm);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SERVER_DEVICE_SCREEN_STATE_NOTIFY, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmDeviceInfo dmDeviceInfo;
    IpcModelCodec::DecodeDmDeviceInfo(data, dmDeviceInfo);
    DeviceManagerNotify::GetInstance().OnDeviceScreenStatus(pkgName, dmDeviceInfo);

    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_DEVICE_SCREEN_STATUS, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetDeviceScreenStatusReq> pReq = std::static_pointer_cast<IpcGetDeviceScreenStatusReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string networkId = pReq->GetNetWorkId();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(networkId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_DEVICE_SCREEN_STATUS, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetDeviceScreenStatusRsp> pRsp = std::static_pointer_cast<IpcGetDeviceScreenStatusRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetScreenStatus(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_ANONY_LOCAL_UDID, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_ANONY_LOCAL_UDID, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetAnonyLocalUdidRsp> pRsp = std::static_pointer_cast<IpcGetAnonyLocalUdidRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetAnonyUdid(reply.ReadString());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_NETWORKID_BY_UDID, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkReq> pReq = std::static_pointer_cast<IpcGetInfoByNetWorkReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string udid = pReq->GetUdid();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(udid)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_NETWORKID_BY_UDID, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetInfoByNetWorkRsp> pRsp = std::static_pointer_cast<IpcGetInfoByNetWorkRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetNetWorkId(reply.ReadString());
    return DM_OK;
}

ON_IPC_CMD(SERVICE_CREDENTIAL_AUTH_STATUS_NOTIFY, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string deviceList = data.ReadString();
    uint16_t deviceTypeId = data.ReadUint16();
    int32_t errCode = data.ReadInt32();
    DeviceManagerNotify::GetInstance().OnCredentialAuthStatus(pkgName, deviceList, deviceTypeId, errCode);

    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_CMD(SINK_BIND_TARGET_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    PeerTargetId targetId;
    IpcModelCodec::DecodePeerTargetId(data, targetId);
    int32_t result = data.ReadInt32();
    int32_t status = data.ReadInt32();
    std::string content = data.ReadString();

    DeviceManagerNotify::GetInstance().OnSinkBindResult(pkgName, targetId, result, status, content);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(REGISTER_DEV_STATE_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_DEV_STATE_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SYNC_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcSyncCallbackReq> pReq = std::static_pointer_cast<IpcSyncCallbackReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pReq->GetDmCommonNotifyEvent())) {
        LOGE("write notify event failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SYNC_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}
ON_IPC_SET_REQUEST(REG_AUTHENTICATION_TYPE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string authTypeStr = pReq->GetFirstParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(authTypeStr)) {
        LOGE("write authTypeStr parameter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REG_AUTHENTICATION_TYPE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_DEVICE_PROFILE_INFO_LIST, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetDeviceProfileInfoListReq> pReq =
        std::static_pointer_cast<IpcGetDeviceProfileInfoListReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodeDmDeviceProfileInfoFilterOptions(pReq->GetFilterOptions(), data)) {
        LOGE("write filterOptions failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_DEVICE_PROFILE_INFO_LIST, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(GET_DEVICE_PROFILE_INFO_LIST_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t code = data.ReadInt32();
    int32_t deviceNum = data.ReadInt32();
    std::vector<DmDeviceProfileInfo> deviceProfileInfos;
    if (deviceNum > 0 && deviceNum <= MAX_DEVICE_PROFILE_SIZE) {
        for (int32_t i = 0; i < deviceNum; ++i) {
            DmDeviceProfileInfo deviceInfo;
            IpcModelCodec::DecodeDmDeviceProfileInfo(data, deviceInfo);
            deviceProfileInfos.emplace_back(deviceInfo);
        }
    }

    DeviceManagerNotify::GetInstance().OnGetDeviceProfileInfoListResult(pkgName, deviceProfileInfos, code);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_DEVICE_ICON_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetDeviceIconInfoReq> pReq = std::static_pointer_cast<IpcGetDeviceIconInfoReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodeDmDeviceIconInfoFilterOptions(pReq->GetFilterOptions(), data)) {
        LOGE("write filterOptions failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_DEVICE_ICON_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(GET_DEVICE_ICON_INFO_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t code = data.ReadInt32();
    DmDeviceIconInfo deviceIconInfo;
    IpcModelCodec::DecodeDmDeviceIconInfo(data, deviceIconInfo);
    DeviceManagerNotify::GetInstance().OnGetDeviceIconInfoResult(pkgName, deviceIconInfo, code);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(PUT_DEVICE_PROFILE_INFO_LIST, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcPutDeviceProfileInfoListReq> pReq =
        std::static_pointer_cast<IpcPutDeviceProfileInfoListReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    std::vector<DmDeviceProfileInfo> deviceProfileInfos = pReq->GetDeviceProfileInfoList();
    if (!data.WriteInt32((int32_t)deviceProfileInfos.size())) {
        LOGE("write device list size failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    for (const auto &devInfo : deviceProfileInfos) {
        if (!IpcModelCodec::EncodeDmDeviceProfileInfo(devInfo, data)) {
            LOGE("write dm device profile info failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(PUT_DEVICE_PROFILE_INFO_LIST, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_LOCAL_DISPLAY_DEVICE_NAME, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetLocalDisplayDeviceNameReq> pReq =
        std::static_pointer_cast<IpcGetLocalDisplayDeviceNameReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pReq->GetMaxNameLength())) {
        LOGE("write maxNameLength failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DISPLAY_DEVICE_NAME, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetLocalDisplayDeviceNameRsp> pRsp =
        std::static_pointer_cast<IpcGetLocalDisplayDeviceNameRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetDisplayName(reply.ReadString());
    return DM_OK;
}

ON_IPC_SET_REQUEST(REG_LOCALSERVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcRegServiceInfoReq> pReq = std::static_pointer_cast<IpcRegServiceInfoReq>(pBaseReq);
    const DMLocalServiceInfo& info = pReq->GetLocalServiceInfo();
    if (!IpcModelCodec::EncodeLocalServiceInfo(info, data)) {
        LOGE("EncodeLocalServiceInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REG_LOCALSERVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNREG_LOCALSERVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string bundleName = pReq->GetFirstParam();
    int32_t pinExchangeType = pReq->GetInt32Param();
    if (!data.WriteString(bundleName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pinExchangeType)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNREG_LOCALSERVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UPDATE_LOCALSERVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcUpdateServiceInfoReq> pReq = std::static_pointer_cast<IpcUpdateServiceInfoReq>(pBaseReq);
    int64_t serviceId = pReq->GetServiceId();
    const DmRegisterServiceInfo& info = pReq->GetRegisterServiceInfo();
    if (!data.WriteInt64(serviceId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodeDmRegServiceInfo(info, data)) {
        LOGE("EncodeDmRegServiceInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UPDATE_LOCALSERVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_SERVICEINFO_BYBUNDLENAME_PINEXCHANGETYPE, std::shared_ptr<IpcReq> pBaseReq,
    MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcCommonParamReq> pReq = std::static_pointer_cast<IpcCommonParamReq>(pBaseReq);
    std::string bundleName = pReq->GetFirstParam();
    int32_t pinExchangeType = pReq->GetInt32Param();
    if (!data.WriteString(bundleName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pinExchangeType)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_SERVICEINFO_BYBUNDLENAME_PINEXCHANGETYPE, MessageParcel &reply,
    std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    if (pBaseRsp->GetErrCode() == DM_OK) {
        std::shared_ptr<IpcGetLocalServiceInfoRsp> pRsp =
            std::static_pointer_cast<IpcGetLocalServiceInfoRsp>(pBaseRsp);
        DMLocalServiceInfo info;
        bool ret = IpcModelCodec::DecodeLocalServiceInfo(reply, info);
        if (!ret) {
            LOGE("DecodeLocalServiceInfo failed");
            pBaseRsp->SetErrCode(ERR_DM_IPC_READ_FAILED);
        }
        pRsp->SetLocalServiceInfo(info);
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(SET_LOCAL_DEVICE_NAME, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcSetLocalDeviceNameReq> pReq = std::static_pointer_cast<IpcSetLocalDeviceNameReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    std::string deviceName = pReq->GetDeviceName();
    if (!data.WriteString(deviceName)) {
        LOGE("write deviceName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SET_LOCAL_DEVICE_NAME, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(SET_LOCAL_DEVICE_NAME_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t code = data.ReadInt32();
    DeviceManagerNotify::GetInstance().OnSetLocalDeviceNameResult(pkgName, code);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SET_REMOTE_DEVICE_NAME, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SET_REMOTE_DEVICE_NAME, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcSetRemoteDeviceNameReq> pReq = std::static_pointer_cast<IpcSetRemoteDeviceNameReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    std::string deviceId = pReq->GetDeviceId();
    if (!data.WriteString(deviceId)) {
        LOGE("write deviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    std::string deviceName = pReq->GetDeviceName();
    if (!data.WriteString(deviceName)) {
        LOGE("write deviceName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(SET_REMOTE_DEVICE_NAME_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string deviceId = data.ReadString();
    int32_t code = data.ReadInt32();
    DeviceManagerNotify::GetInstance().OnSetRemoteDeviceNameResult(pkgName, deviceId, code);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(RESTORE_LOCAL_DEVICE_NAME, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(RESTORE_LOCAL_DEVICE_NAME, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::string pkgName = pBaseReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_DEVICE_NETWORK_ID_LIST, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    if (pBaseReq == nullptr) {
        LOGE("pBaseReq is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcGetDeviceNetworkIdListReq> pReq =
        std::static_pointer_cast<IpcGetDeviceNetworkIdListReq>(pBaseReq);
    if (!data.WriteString(pReq->GetPkgName())) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodeNetworkIdQueryFilter(pReq->GetQueryFilter(), data)) {
        LOGE("write query filter failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_DEVICE_NETWORK_ID_LIST, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    if (pBaseRsp == nullptr) {
        LOGE("pBaseRsp is null");
        return ERR_DM_FAILED;
    }
    pBaseRsp->SetErrCode(reply.ReadInt32());
    if (pBaseRsp->GetErrCode() == DM_OK) {
        std::shared_ptr<IpcGetDeviceNetworkIdListRsp> pRsp =
            std::static_pointer_cast<IpcGetDeviceNetworkIdListRsp>(pBaseRsp);
        std::vector<std::string> networkIds;
        bool ret = IpcModelCodec::DecodeStringVector(reply, networkIds);
        if (!ret) {
            LOGE("DecodeLocalServiceInfo failed");
            pBaseRsp->SetErrCode(ERR_DM_IPC_READ_FAILED);
        }
        pRsp->SetNetworkIds(networkIds);
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNREGISTER_PIN_HOLDER_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNREGISTER_PIN_HOLDER_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_LOCAL_DEVICE_NAME, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DEVICE_NAME, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetLocalDeviceNameRsp> pRsp = std::static_pointer_cast<IpcGetLocalDeviceNameRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetLocalDeviceName(reply.ReadString());
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_LOCAL_DEVICE_NAME_OLD, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcReq> pReq = std::static_pointer_cast<IpcReq>(pBaseReq);
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_LOCAL_DEVICE_NAME_OLD, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetLocalDeviceNameRsp> pRsp = std::static_pointer_cast<IpcGetLocalDeviceNameRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetLocalDeviceName(reply.ReadString());
    return DM_OK;
}

ON_IPC_SET_REQUEST(CHECK_SRC_ACCESS_CONTROL, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    return SetRequest(CHECK_SRC_ACCESS_CONTROL, pBaseReq, data);
}

ON_IPC_READ_RESPONSE(CHECK_SRC_ACCESS_CONTROL, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return ReadResponse(CHECK_SRC_ACCESS_CONTROL, reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(CHECK_SINK_ACCESS_CONTROL, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    return SetRequest(CHECK_SINK_ACCESS_CONTROL, pBaseReq, data);
}

ON_IPC_READ_RESPONSE(CHECK_SINK_ACCESS_CONTROL, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return ReadResponse(CHECK_SINK_ACCESS_CONTROL, reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(CHECK_SRC_SAME_ACCOUNT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    return SetRequest(CHECK_SRC_SAME_ACCOUNT, pBaseReq, data);
}

ON_IPC_READ_RESPONSE(CHECK_SRC_SAME_ACCOUNT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return ReadResponse(CHECK_SRC_SAME_ACCOUNT, reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(CHECK_SINK_SAME_ACCOUNT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    return SetRequest(CHECK_SINK_SAME_ACCOUNT, pBaseReq, data);
}

ON_IPC_READ_RESPONSE(CHECK_SINK_SAME_ACCOUNT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    return ReadResponse(CHECK_SINK_SAME_ACCOUNT, reply, pBaseRsp);
}

ON_IPC_SET_REQUEST(GET_UDIDS_BY_DEVICEIDS, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetUdidsByDeviceIdsReq> pReq =
        std::static_pointer_cast<IpcGetUdidsByDeviceIdsReq>(pBaseReq);
    if (!data.WriteString(pReq->GetPkgName())) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    std::vector<std::string> deviceIdList = pReq->GetDeviceIdList();
    if (!IpcModelCodec::EncodeStringVector(deviceIdList, data)) {
        LOGE("write deviceIdList failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_UDIDS_BY_DEVICEIDS, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetUdidsByDeviceIdsRsp> pRsp = std::static_pointer_cast<IpcGetUdidsByDeviceIdsRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    std::string outParaStr = reply.ReadString();
    std::map<std::string, std::string> outputResult;
    ParseMapFromJsonString(outParaStr, outputResult);
    pRsp->SetDeviceIdToUdidMap(outputResult);
    return DM_OK;
}

ON_IPC_CMD(NOTIFY_SERVICE_DISCOVERY_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string serviceType = data.ReadString();
    int32_t reason = data.ReadInt32();
    DeviceManagerNotify::GetInstance().OnServiceDiscoveryResult(pkgName, serviceType, reason);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(BIND_SERVICE_TARGET, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcBindTargetReq> pReq = std::static_pointer_cast<IpcBindTargetReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    PeerTargetId targetId = pReq->GetPeerTargetId();
    std::string bindParam = pReq->GetBindParam();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodePeerTargetId(targetId, data)) {
        LOGE("write peer target id failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(bindParam)) {
        LOGE("write bind parameter string failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(BIND_SERVICE_TARGET, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(SERVICE_PUBLISH_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int64_t serviceId = data.ReadInt64();
    int32_t publishResult = data.ReadInt32();
    DeviceManagerNotify::GetInstance().OnServicePublishResult(pkgName, serviceId, publishResult);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(LEAVE_LNN, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetDeviceScreenStatusReq> pReq =
        std::static_pointer_cast<IpcGetDeviceScreenStatusReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string networkId = pReq->GetNetWorkId();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(networkId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(LEAVE_LNN, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(LEAVE_LNN_RESULT, MessageParcel &data, MessageParcel &reply)
{
    std::string networkId = data.ReadString();
    int32_t code = data.ReadInt32();
    DeviceManagerNotify::GetInstance().OnLeaveLNNResult(networkId, code);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_AUTHTYPE_BY_UDIDHASH, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetAuthTypeByUdidHashReq> pReq =
        std::static_pointer_cast<IpcGetAuthTypeByUdidHashReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string udidHash = pReq->GetUdidHash();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(udidHash)) {
        LOGE("write udidHash failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_AUTHTYPE_BY_UDIDHASH, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetAuthTypeByUdidHashRsp> pRsp =
        std::static_pointer_cast<IpcGetAuthTypeByUdidHashRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetAuthType(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(ON_AUTH_CODE_INVALID, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DeviceManagerNotify::GetInstance().OnAuthCodeInvalid(pkgName);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(REGISTER_SERVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcRegisterServiceInfoReq> pReq =
        std::static_pointer_cast<IpcRegisterServiceInfoReq>(pBaseReq);
    const DmRegisterServiceInfo regServiceInfo = pReq->GetRegisterServiceInfo();
    if (!IpcModelCodec::EncodeDmRegServiceInfo(regServiceInfo, data)) {
        LOGE("write serviceRegInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REGISTER_SERVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcPublishServiceInfoRsp> pRsp = std::static_pointer_cast<IpcPublishServiceInfoRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    pRsp->SetServiceId(reply.ReadInt64());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNREGISTER_SERVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcUnRegisterServiceInfoReq> pReq = std::static_pointer_cast<IpcUnRegisterServiceInfoReq>(pBaseReq);
    int64_t serviceId = pReq->GetServiceId();
    if (!data.WriteInt64(serviceId)) {
        LOGE("write regServiceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNREGISTER_SERVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(START_PUBLISH_SERVICE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcStartPublishServiceReq> pReq = std::static_pointer_cast<IpcStartPublishServiceReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    int64_t serviceId = pReq->GetServiceId();
    if (!data.WriteInt64(serviceId)) {
        LOGE("write serviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    DmPublishServiceParam param = pReq->GetDmPublishServiceParam();
    if (!IpcModelCodec::EncodeDmPublishServiceParam(param, data)) {
        LOGE("write publishServiceParam failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(START_PUBLISH_SERVICE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcPublishServiceInfoRsp> pRsp = std::static_pointer_cast<IpcPublishServiceInfoRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(STOP_PUBLISH_SERVICE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcStopPublishServiceReq> pReq = std::static_pointer_cast<IpcStopPublishServiceReq>(pBaseReq);
    int64_t serviceId = pReq->GetServiceId();
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt64(serviceId)) {
        LOGE("write GetServiceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(STOP_PUBLISH_SERVICE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(START_SERVICE_DISCOVERING, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcStartServiceDiscoveryReq> pReq =
        std::static_pointer_cast<IpcStartServiceDiscoveryReq>(pBaseReq);
    if (!data.WriteString(pReq->GetPkgName())) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    DmDiscoveryServiceParam discParam = pReq->GetDmDiscParam();
    if (!IpcModelCodec::EncodeDmSrvDiscParam(discParam, data)) {
        LOGE("write GetServiceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(START_SERVICE_DISCOVERING, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(STOP_SERVICE_DISCOVERING, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcStartServiceDiscoveryReq> pReq =
        std::static_pointer_cast<IpcStartServiceDiscoveryReq>(pBaseReq);
    if (!data.WriteString(pReq->GetPkgName())) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    DmDiscoveryServiceParam discParam = pReq->GetDmDiscParam();
    if (!IpcModelCodec::EncodeDmSrvDiscParam(discParam, data)) {
        LOGE("write GetServiceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(STOP_SERVICE_DISCOVERING, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcRsp> pRsp = std::static_pointer_cast<IpcRsp>(pBaseRsp);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(NOTIFY_SERVICE_FOUND, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmServiceInfo dmServiceInfo;
    if (!IpcModelCodec::DecodeDmServiceInfo(data, dmServiceInfo)) {
        LOGE("DecodeDmServiceInfo failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    DeviceManagerNotify::GetInstance().OnServiceFound(pkgName, dmServiceInfo);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}
// zl online&offline
ON_IPC_SET_REQUEST(SYNC_SERVICE_CALLBACK, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcSyncServiceCallbackReq> pReq = std::static_pointer_cast<IpcSyncServiceCallbackReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int64_t serviceId= pReq->GetServiceId();
    int32_t notifyEvent = pReq->GetDmCommonNotifyEvent();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt64(serviceId)) {
        LOGE("write serviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(notifyEvent)) {
        LOGE("write notify event failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}
// zl online&offline
ON_IPC_READ_RESPONSE(SYNC_SERVICE_CALLBACK, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}
// zl online&offline
ON_IPC_CMD(SERVER_SERVICE_STATE_NOTIFY, MessageParcel &data, MessageParcel &reply)
{
    LOGI("SERVER_SERVICE_STATE_NOTIFY ON_IPC_CMD start");

    DmRegisterServiceState registerServiceState;
    IpcModelCodec::DecodeDmRegisterServiceState(data, registerServiceState);
    DmServiceInfo dmServiceInfo;
    IpcModelCodec::DecodeDmServiceInfo(data, dmServiceInfo);
    DmServiceState deviceState = static_cast<DmServiceState>(data.ReadInt32());
 
    switch (deviceState) {
        case SERVICE_STATE_ONLINE:
            LOGI("SERVER_SERVICE_STATE_NOTIFY ON_IPC_CMD OnServiceOnline");
            DeviceManagerNotify::GetInstance().OnServiceOnline(registerServiceState, dmServiceInfo);
            break;
        case SERVICE_STATE_OFFLINE:
            DeviceManagerNotify::GetInstance().OnServiceOffline(registerServiceState, dmServiceInfo);
            break;
        case DEVICE_INFO_CHANGED:
            break;
        default:
            LOGE("unknown service state:%{public}d", deviceState);
            break;
    }
    reply.WriteInt32(DM_OK);
    LOGI("SERVER_SERVICE_STATE_NOTIFY ON_IPC_CMD sucessed.");

    return DM_OK;
}
//add by zqz
ON_IPC_SET_REQUEST(SYNC_SERVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcSyncServiceInfoReq> pReq = std::static_pointer_cast<IpcSyncServiceInfoReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t localUserId = pReq->GetLocalUserId();
    std::string networkId = pReq->GetNetworkId();
    int64_t serviceId = pReq->GetServiceId();
    if (!data.WriteString(pkgName)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(localUserId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(networkId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt64(serviceId)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}
//add by zqz
ON_IPC_READ_RESPONSE(SYNC_SERVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}
//add by zqz
ON_IPC_CMD(SYNC_SERVICE_INFO_RESULT, MessageParcel &data, MessageParcel &reply)
{
    ServiceSyncInfo serviceSyncInfo;
    IpcModelCodec::DecodeServiceSyncInfo(data, serviceSyncInfo);
    int32_t result = data.ReadInt32();
    std::string content = data.ReadString();
    DeviceManagerNotify::GetInstance().OnSyncServiceInfoResult(serviceSyncInfo, result, content);
    reply.WriteInt32(DM_OK);
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_LOCAL_SERVICEINFO_BY_SERVICEID, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcStopPublishServiceReq> pReq = std::static_pointer_cast<IpcStopPublishServiceReq>(pBaseReq);
    int64_t serviceId = pReq->GetServiceId();
    if (!data.WriteInt64(serviceId)) {
        LOGE("write serviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}
 
ON_IPC_READ_RESPONSE(GET_LOCAL_SERVICEINFO_BY_SERVICEID, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetLocalServiceInfoExtRsp> pRsp = std::static_pointer_cast<IpcGetLocalServiceInfoExtRsp>(pBaseRsp);
 
    DmRegisterServiceInfo serviceInfo;
    IpcModelCodec::DecodeDmRegServiceInfo(reply, serviceInfo);
 
    pRsp->SetServiceInfo(serviceInfo);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}
 
ON_IPC_SET_REQUEST(GET_TRUST_SERVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::static_pointer_cast<IpcNotifyDMFAResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string jsonParam = pReq->GetJsonParam();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkg failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(jsonParam)) {
        LOGE("write jsonParam failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}
 
ON_IPC_READ_RESPONSE(GET_TRUST_SERVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetTrustServiceInfoRsp> pRsp = std::static_pointer_cast<IpcGetTrustServiceInfoRsp>(pBaseRsp);
    int32_t serviceInfoSize = reply.ReadInt32();
    if (serviceInfoSize > 0 && serviceInfoSize <= DM_MAX_TRUST_DEVICE_NUM) {
        std::vector<DmServiceInfo> serviceInfoVec;
        for (int32_t i = 0; i < serviceInfoSize; ++i) {
            DmServiceInfo serviceInfo;
            IpcModelCodec::DecodeDmServiceInfo(reply, serviceInfo);
            serviceInfoVec.emplace_back(serviceInfo);
        }
        pRsp->SetServiceInfoVec(serviceInfoVec);
    }
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}
 
ON_IPC_SET_REQUEST(GET_REGISTER_SERVICE_INFO, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::static_pointer_cast<IpcNotifyDMFAResultReq>(pBaseReq);
    std::string jsonParam = pReq->GetJsonParam();
    if (!data.WriteString(jsonParam)) {
        LOGE("write jsonParam failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}
 
ON_IPC_READ_RESPONSE(GET_REGISTER_SERVICE_INFO, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetRegisterServiceInfoRsp> pRsp = std::static_pointer_cast<IpcGetRegisterServiceInfoRsp>(pBaseRsp);
 
    int32_t regServiceInfoSize = reply.ReadInt32();
    if (regServiceInfoSize > 0 && regServiceInfoSize <= DM_MAX_TRUST_DEVICE_NUM) {
        std::vector<DmRegisterServiceInfo> regServiceInfoVec;
        for (int32_t i = 0; i < regServiceInfoSize; ++i) {
            DmRegisterServiceInfo regServiceInfo;
            IpcModelCodec::DecodeDmRegServiceInfo(reply, regServiceInfo);
            regServiceInfoVec.emplace_back(regServiceInfo);
        }
        pRsp->SetRegisterServiceInfos(regServiceInfoVec);
    }
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}
 
ON_IPC_SET_REQUEST(GET_PEER_SERVICEINFO_BY_SERVICEID, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcGetPeerServiceInfoByServiceIdReq> pReq =
        std::static_pointer_cast<IpcGetPeerServiceInfoByServiceIdReq>(pBaseReq);
    std::string networkId = pReq->GetNetworkId();
    int64_t serviceId = pReq->GetServiceId();
    if (!data.WriteString(networkId)) {
        LOGE("write networkId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt64(serviceId)) {
        LOGE("write serviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}
 
ON_IPC_READ_RESPONSE(GET_PEER_SERVICEINFO_BY_SERVICEID, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    std::shared_ptr<IpcGetLocalServiceInfoExtRsp> pRsp =
        std::static_pointer_cast<IpcGetLocalServiceInfoExtRsp>(pBaseRsp);
    DmRegisterServiceInfo serviceInfo;
    IpcModelCodec::DecodeDmRegServiceInfo(reply, serviceInfo);
 
    pRsp->SetServiceInfo(serviceInfo);
    pRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNBIND_SERVICE_TARGET, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcBindTargetReq> pReq = std::static_pointer_cast<IpcBindTargetReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string unbindParam = pReq->GetBindParam();
    std::string networkId = pReq->GetNetWorkId();
    int64_t serviceId = pReq->GetServiceId();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(unbindParam)) {
        LOGE("write unbind parameter string failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(networkId)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt64(serviceId)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNBIND_SERVICE_TARGET, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
