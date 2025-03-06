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

#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

#include "device_manager_ipc_interface_code.h"
#include "device_manager_service.h"
#include "device_manager_service_notify.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "ipc_acl_profile_req.h"
#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "ipc_create_pin_holder_req.h"
#include "ipc_credential_auth_status_req.h"
#include "ipc_destroy_pin_holder_req.h"
#include "ipc_model_codec.h"
#include "ipc_notify_devicetrustchange_req.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_bind_result_req.h"
#include "ipc_notify_credential_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_discovery_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_get_device_icon_info_req.h"
#include "ipc_notify_get_device_profile_info_list_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_notify_pin_holder_event_req.h"
#include "ipc_server_client_proxy.h"
#include "ipc_server_stub.h"
#include "multiple_user_connector.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "multiple_user_connector.h"
#endif
namespace OHOS {
namespace DistributedHardware {
const unsigned int XCOLLIE_TIMEOUT_S = 5;
bool EncodeDmDeviceInfo(const DmDeviceInfo &devInfo, MessageParcel &parcel)
{
    bool bRet = true;
    std::string deviceIdStr(devInfo.deviceId);
    bRet = (bRet && parcel.WriteString(deviceIdStr));
    std::string deviceNameStr(devInfo.deviceName);
    bRet = (bRet && parcel.WriteString(deviceNameStr));
    bRet = (bRet && parcel.WriteUint16(devInfo.deviceTypeId));
    std::string networkIdStr(devInfo.networkId);
    bRet = (bRet && parcel.WriteString(networkIdStr));
    bRet = (bRet && parcel.WriteInt32(devInfo.range));
    bRet = (bRet && parcel.WriteInt32(devInfo.networkType));
    bRet = (bRet && parcel.WriteInt32(devInfo.authForm));
    bRet = (bRet && parcel.WriteString(devInfo.extraData));
    return bRet;
}

bool EncodeDmDeviceBasicInfo(const DmDeviceBasicInfo &devInfo, MessageParcel &parcel)
{
    bool bRet = true;
    std::string deviceIdStr(devInfo.deviceId);
    bRet = (bRet && parcel.WriteString(deviceIdStr));
    std::string deviceNameStr(devInfo.deviceName);
    bRet = (bRet && parcel.WriteString(deviceNameStr));
    bRet = (bRet && parcel.WriteUint16(devInfo.deviceTypeId));
    std::string networkIdStr(devInfo.networkId);
    bRet = (bRet && parcel.WriteString(networkIdStr));
    bRet = (bRet && parcel.WriteString(devInfo.extraData));
    return bRet;
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

void DecodePeerTargetId(MessageParcel &parcel, PeerTargetId &targetId)
{
    targetId.deviceId = parcel.ReadString();
    targetId.brMac = parcel.ReadString();
    targetId.bleMac = parcel.ReadString();
    targetId.wifiIp = parcel.ReadString();
    targetId.wifiPort = parcel.ReadUint16();
}

void DecodeDmAccessCaller(MessageParcel &parcel, DmAccessCaller &caller)
{
    caller.accountId = parcel.ReadString();
    caller.pkgName = parcel.ReadString();
    caller.networkId = parcel.ReadString();
    caller.userId = parcel.ReadInt32();
    caller.tokenId = parcel.ReadUint64();
    caller.extra = parcel.ReadString();
}

void DecodeDmAccessCallee(MessageParcel &parcel, DmAccessCallee &callee)
{
    callee.accountId = parcel.ReadString();
    callee.networkId = parcel.ReadString();
    callee.peerId = parcel.ReadString();
    callee.userId = parcel.ReadInt32();
    callee.extra = parcel.ReadString();
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_STATE_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::static_pointer_cast<IpcNotifyDeviceStateReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t deviceState = pReq->GetDeviceState();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    DmDeviceBasicInfo deviceBasicInfo = pReq->GetDeviceBasicInfo();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(deviceState)) {
        LOGE("write state failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!EncodeDmDeviceInfo(deviceInfo, data)) {
        LOGE("write dm device info failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!EncodeDmDeviceBasicInfo(deviceBasicInfo, data)) {
        LOGE("write dm device basic info failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_STATE_NOTIFY, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_FOUND, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq = std::static_pointer_cast<IpcNotifyDeviceFoundReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();
    DmDeviceBasicInfo devBasicInfo = pReq->GetDeviceBasicInfo();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt16((int16_t)subscribeId)) {
        LOGE("write subscribeId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!EncodeDmDeviceInfo(deviceInfo, data)) {
        LOGE("write dm device info failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!EncodeDmDeviceBasicInfo(devBasicInfo, data)) {
        LOGE("write dm device basic info failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_FOUND, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_DISCOVERY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyDeviceDiscoveryReq> pReq = std::static_pointer_cast<IpcNotifyDeviceDiscoveryReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    DmDeviceBasicInfo deviceBasicInfo = pReq->GetDeviceBasicInfo();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt16((int16_t)subscribeId)) {
        LOGE("write subscribeId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!EncodeDmDeviceBasicInfo(deviceBasicInfo, data)) {
        LOGE("write dm device basic info failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_DISCOVERY, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DISCOVER_FINISH, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq = std::static_pointer_cast<IpcNotifyDiscoverResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    uint16_t subscribeId = pReq->GetSubscribeId();
    int32_t result = pReq->GetResult();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt16((int16_t)subscribeId)) {
        LOGE("write subscribeId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DISCOVER_FINISH, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_PUBLISH_FINISH, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq = std::static_pointer_cast<IpcNotifyPublishResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t publishId = pReq->GetPublishId();
    int32_t result = pReq->GetResult();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(publishId)) {
        LOGE("write publishId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_PUBLISH_FINISH, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_AUTH_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyAuthResultReq> pReq = std::static_pointer_cast<IpcNotifyAuthResultReq>(pBaseReq);

    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    std::string token = pReq->GetPinToken();
    int32_t status = pReq->GetStatus();
    int32_t reason = pReq->GetReason();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(deviceId)) {
        LOGE("write deviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(token)) {
        LOGE("write token failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(status)) {
        LOGE("write status failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(reason)) {
        LOGE("write reason failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_AUTH_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_FA_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyDMFAResultReq> pReq = std::static_pointer_cast<IpcNotifyDMFAResultReq>(pBaseReq);

    std::string packagname = pReq->GetPkgName();
    std::string paramJson = pReq->GetJsonParam();
    if (!data.WriteString(packagname)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(paramJson)) {
        LOGE("write paramJson failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_FA_NOTIFY, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(GET_TRUST_DEVICE_LIST, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string extra = data.ReadString();
    bool isRefresh = data.ReadBool();
    if (isRefresh) {
        DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, pkgName, isRefresh, false);
    }
    std::vector<DmDeviceInfo> deviceList;
    int32_t result = DeviceManagerService::GetInstance().GetTrustedDeviceList(pkgName, extra, deviceList);
    if (!reply.WriteInt32((int32_t)deviceList.size())) {
        LOGE("write device list size failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    for (const auto &devInfo : deviceList) {
        if (!EncodeDmDeviceInfo(devInfo, reply)) {
            LOGE("write dm device info failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_ALL_TRUST_DEVICE_LIST, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string extra = data.ReadString();
    std::vector<DmDeviceInfo> deviceList;
    int32_t result = DeviceManagerService::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    if (!reply.WriteInt32((int32_t)deviceList.size())) {
        LOGE("write device list size failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    for (const auto &devInfo : deviceList) {
        if (!EncodeDmDeviceInfo(devInfo, reply)) {
            LOGE("write dm device info failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(REGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &data, MessageParcel &reply)
{
    int32_t id = OHOS::HiviewDFX::XCollie::GetInstance().SetTimer("RegisterDeviceManagerListener", XCOLLIE_TIMEOUT_S,
        nullptr, nullptr, OHOS::HiviewDFX::XCOLLIE_FLAG_LOG | OHOS::HiviewDFX::XCOLLIE_FLAG_RECOVERY);
    std::string pkgName = data.ReadString();
    sptr<IRemoteObject> listener = data.ReadRemoteObject();
    if (listener == nullptr) {
        LOGE("read remote object failed.");
        OHOS::HiviewDFX::XCollie::GetInstance().CancelTimer(id);
        return ERR_DM_POINT_NULL;
    }
    sptr<IpcServerClientProxy> callback(new IpcServerClientProxy(listener));
    if (callback == nullptr) {
        LOGE("create ipc server client proxy failed.");
        OHOS::HiviewDFX::XCollie::GetInstance().CancelTimer(id);
        return ERR_DM_POINT_NULL;
    }
    ProcessInfo processInfo;
    processInfo.pkgName = pkgName;
    MultipleUserConnector::GetCallerUserId(processInfo.userId);
    DeviceManagerService::GetInstance().RegisterCallerAppId(pkgName);
    int32_t result = IpcServerStub::GetInstance().RegisterDeviceManagerListener(processInfo, callback);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        OHOS::HiviewDFX::XCollie::GetInstance().CancelTimer(id);
        return ERR_DM_IPC_WRITE_FAILED;
    }
    OHOS::HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return DM_OK;
}

ON_IPC_CMD(UNREGISTER_DEVICE_MANAGER_LISTENER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    ProcessInfo processInfo;
    processInfo.pkgName = pkgName;
    MultipleUserConnector::GetCallerUserId(processInfo.userId);
    DeviceManagerService::GetInstance().UnRegisterCallerAppId(pkgName);
    int32_t result = IpcServerStub::GetInstance().UnRegisterDeviceManagerListener(processInfo);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}


ON_IPC_CMD(PUBLISH_DEVICE_DISCOVER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmPublishInfo *publishInfo =
        static_cast<DmPublishInfo *>(const_cast<void *>(data.ReadRawData(sizeof(DmPublishInfo))));
    int32_t result = ERR_DM_POINT_NULL;

    if (publishInfo != nullptr) {
        result = DeviceManagerService::GetInstance().PublishDeviceDiscovery(pkgName, *publishInfo);
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNPUBLISH_DEVICE_DISCOVER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t publishId = data.ReadInt32();
    int32_t result = DeviceManagerService::GetInstance().UnPublishDeviceDiscovery(pkgName, publishId);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(AUTHENTICATE_DEVICE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string extra = data.ReadString();
    std::string deviceId = data.ReadString();
    int32_t authType = data.ReadInt32();

    int32_t result = DM_OK;
    result = DeviceManagerService::GetInstance().AuthenticateDevice(pkgName, authType, deviceId, extra);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNAUTHENTICATE_DEVICE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string deviceId = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().UnAuthenticateDevice(pkgName, deviceId);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_DEVICE_INFO, MessageParcel &data, MessageParcel &reply)
{
    std::string networkId = data.ReadString();
    DmDeviceInfo deviceInfo;
    int32_t result = DeviceManagerService::GetInstance().GetDeviceInfo(networkId, deviceInfo);
    if (!EncodeDmDeviceInfo(deviceInfo, reply)) {
        LOGE("write dm device info failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_LOCAL_DEVICE_INFO, MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    DmDeviceInfo localDeviceInfo;
    int32_t result = DeviceManagerService::GetInstance().GetLocalDeviceInfo(localDeviceInfo);
    if (!EncodeDmDeviceInfo(localDeviceInfo, reply)) {
        LOGE("write dm device info failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_UDID_BY_NETWORK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string netWorkId = data.ReadString();
    std::string udid;
    int32_t result = DeviceManagerService::GetInstance().GetUdidByNetworkId(pkgName, netWorkId, udid);

    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(udid)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_UUID_BY_NETWORK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string netWorkId = data.ReadString();
    std::string uuid;
    int32_t result = DeviceManagerService::GetInstance().GetUuidByNetworkId(pkgName, netWorkId, uuid);

    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(uuid)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(SERVER_USER_AUTH_OPERATION, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    int32_t action = data.ReadInt32();
    std::string params = data.ReadString();
    int result = DeviceManagerService::GetInstance().SetUserOperation(packageName, action, params);
    if (!reply.WriteInt32(result)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return result;
}

ON_IPC_CMD(REQUEST_CREDENTIAL, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    std::string reqParaStr = data.ReadString();
    std::map<std::string, std::string> requestParam;
    ParseMapFromJsonString(reqParaStr, requestParam);
    std::string returnJsonStr;
    int32_t ret = ERR_DM_FAILED;
    if (requestParam[DM_CREDENTIAL_TYPE] == DM_TYPE_OH) {
        ret = DeviceManagerService::GetInstance().RequestCredential(requestParam[DM_CREDENTIAL_REQJSONSTR],
                                                                    returnJsonStr);
    }
    if (requestParam[DM_CREDENTIAL_TYPE] == DM_TYPE_MINE) {
        ret = DeviceManagerService::GetInstance().MineRequestCredential(packageName, returnJsonStr);
    }
    if (!reply.WriteInt32(ret)) {
        LOGE("write ret failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (ret == DM_OK && !returnJsonStr.empty()) {
        if (!reply.WriteString(returnJsonStr)) {
            LOGE("write returnJsonStr failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    return DM_OK;
}

ON_IPC_CMD(IMPORT_CREDENTIAL, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    std::string reqParaStr = data.ReadString();
    std::map<std::string, std::string> requestParam;
    ParseMapFromJsonString(reqParaStr, requestParam);
    std::string returnJsonStr;
    std::map<std::string, std::string> outputResult;
    int32_t ret = ERR_DM_FAILED;
    if (requestParam[DM_CREDENTIAL_TYPE] == DM_TYPE_OH) {
        ret = DeviceManagerService::GetInstance().ImportCredential(packageName, requestParam[DM_CREDENTIAL_REQJSONSTR]);
        outputResult.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_OH);
    }
    if (requestParam[DM_CREDENTIAL_TYPE] == DM_TYPE_MINE) {
        ret = DeviceManagerService::GetInstance().ImportCredential(packageName, requestParam[DM_CREDENTIAL_REQJSONSTR],
                                                                   returnJsonStr);
        outputResult.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_MINE);
        outputResult.emplace(DM_CREDENTIAL_RETURNJSONSTR, returnJsonStr);
    }
    if (!reply.WriteInt32(ret)) {
        LOGE("write ret failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (ret == DM_OK && !returnJsonStr.empty()) {
        std::string outParaStr = ConvertMapToJsonString(outputResult);
        if (!reply.WriteString(outParaStr)) {
        LOGE("write returnJsonStr failed");
        return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    return DM_OK;
}

ON_IPC_CMD(DELETE_CREDENTIAL, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    std::string reqParaStr = data.ReadString();
    std::map<std::string, std::string> requestParam;
    ParseMapFromJsonString(reqParaStr, requestParam);
    std::map<std::string, std::string> outputResult;
    std::string returnJsonStr;
    int32_t ret = ERR_DM_FAILED;
    if (requestParam[DM_CREDENTIAL_TYPE] == DM_TYPE_OH) {
        ret = DeviceManagerService::GetInstance().DeleteCredential(packageName, requestParam[DM_CREDENTIAL_REQJSONSTR]);
        outputResult.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_OH);
    }
    if (requestParam[DM_CREDENTIAL_TYPE] == DM_TYPE_MINE) {
        ret = DeviceManagerService::GetInstance().DeleteCredential(packageName, requestParam[DM_CREDENTIAL_REQJSONSTR],
                                                                   returnJsonStr);
        outputResult.emplace(DM_CREDENTIAL_TYPE, DM_TYPE_MINE);
        outputResult.emplace(DM_CREDENTIAL_RETURNJSONSTR, returnJsonStr);
    }
    if (!reply.WriteInt32(ret)) {
        LOGE("write ret failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (ret == DM_OK && !returnJsonStr.empty()) {
        std::string outParaStr = ConvertMapToJsonString(outputResult);
        if (!reply.WriteString(outParaStr)) {
            LOGE("write returnJsonStr failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    return DM_OK;
}

ON_IPC_CMD(SERVER_GET_DMFA_INFO, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    std::string reqJsonStr = data.ReadString();
    std::string returnJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().CheckCredential(packageName, reqJsonStr, returnJsonStr);
    if (!reply.WriteInt32(ret)) {
        LOGE("write ret failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (ret == DM_OK && !returnJsonStr.empty()) {
        if (!reply.WriteString(returnJsonStr)) {
            LOGE("write returnJsonStr failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    return DM_OK;
}

ON_IPC_CMD(REGISTER_CREDENTIAL_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    int result = DeviceManagerService::GetInstance().RegisterCredentialCallback(packageName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return result;
}

ON_IPC_CMD(UNREGISTER_CREDENTIAL_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string packageName = data.ReadString();
    int result = DeviceManagerService::GetInstance().UnRegisterCredentialCallback(packageName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return result;
}

ON_IPC_CMD(NOTIFY_EVENT, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t eventId = data.ReadInt32();
    std::string event = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_CREDENTIAL_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyCredentialReq> pReq = std::static_pointer_cast<IpcNotifyCredentialReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t action = pReq->GetCredentialAction();
    std::string credentialResult = pReq->GetCredentialResult();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(action)) {
        LOGE("write action failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(credentialResult)) {
        LOGE("write credentialResult failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }

    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_CREDENTIAL_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(GET_ENCRYPTED_UUID_BY_NETWOEKID, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string networkId = data.ReadString();
    std::string uuid;

    int32_t result = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(uuid)) {
        LOGE("write uuid failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GENERATE_ENCRYPTED_UUID, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string uuid = data.ReadString();
    std::string appId = data.ReadString();
    std::string encryptedUuid;

    int32_t result = DeviceManagerService::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(encryptedUuid)) {
        LOGE("write encryptedUuid failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(BIND_DEVICE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string bindParam = data.ReadString();
    std::string deviceId = data.ReadString();
    int32_t bindType = data.ReadInt32();
    int32_t result = DM_OK;
    result = DeviceManagerService::GetInstance().BindDevice(pkgName, bindType, deviceId, bindParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNBIND_DEVICE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string deviceId = data.ReadString();
    std::string extra = data.ReadString();
    int32_t result = 0;
    if (extra == "") {
        result = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    } else {
        result = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_NETWORKTYPE_BY_NETWORK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string netWorkId = data.ReadString();
    int32_t networkType = -1;
    int32_t result = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteInt32(networkType)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(REGISTER_UI_STATE_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNREGISTER_UI_STATE_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(IMPORT_AUTH_CODE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string authCode = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(EXPORT_AUTH_CODE, MessageParcel &data, MessageParcel &reply)
{
    std::string authCode = "";
    int32_t result = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    if (!reply.WriteString(authCode)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(REGISTER_DISCOVERY_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string discParaStr = data.ReadString();
    std::string filterOpStr = data.ReadString();
    std::map<std::string, std::string> discoverParam;
    ParseMapFromJsonString(discParaStr, discoverParam);
    std::map<std::string, std::string> filterOptions;
    ParseMapFromJsonString(filterOpStr, filterOptions);
    int32_t result = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNREGISTER_DISCOVERY_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string extraParaStr = data.ReadString();
    std::map<std::string, std::string> extraParam;
    ParseMapFromJsonString(extraParaStr, extraParam);
    int32_t result = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(START_DISCOVERING, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string discParaStr = data.ReadString();
    std::string filterOpStr = data.ReadString();
    std::map<std::string, std::string> discoverParam;
    ParseMapFromJsonString(discParaStr, discoverParam);
    std::map<std::string, std::string> filterOptions;
    ParseMapFromJsonString(filterOpStr, filterOptions);
    int32_t result = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(STOP_DISCOVERING, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string discParaStr = data.ReadString();
    std::map<std::string, std::string> discoverParam;
    ParseMapFromJsonString(discParaStr, discoverParam);
    int32_t result = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(START_ADVERTISING, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string adverParaStr = data.ReadString();
    std::map<std::string, std::string> advertiseParam;
    ParseMapFromJsonString(adverParaStr, advertiseParam);
    int32_t result = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(STOP_ADVERTISING, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string adverParaStr = data.ReadString();
    std::map<std::string, std::string> advertiseParam;
    ParseMapFromJsonString(adverParaStr, advertiseParam);
    int32_t result = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(BIND_TARGET, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    PeerTargetId targetId;
    DecodePeerTargetId(data, targetId);
    std::string bindParamStr = data.ReadString();
    std::map<std::string, std::string> bindParam;
    ParseMapFromJsonString(bindParamStr, bindParam);
    int32_t result = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNBIND_TARGET, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    PeerTargetId targetId;
    DecodePeerTargetId(data, targetId);
    std::string unbindParamStr = data.ReadString();
    std::map<std::string, std::string> unbindParam;
    ParseMapFromJsonString(unbindParamStr, unbindParam);
    int32_t result = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(BIND_TARGET_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::static_pointer_cast<IpcNotifyBindResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    PeerTargetId targetId = pReq->GetPeerTargetId();
    int32_t result = pReq->GetResult();
    int32_t status = pReq->GetStatus();
    std::string content = pReq->GetContent();

    if (!data.WriteString(pkgName)) {
        LOGE("write bind pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!EncodePeerTargetId(targetId, data)) {
        LOGE("write bind peer target id failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write bind result code failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(status)) {
        LOGE("write bind result status failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(content)) {
        LOGE("write bind result content failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(BIND_TARGET_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SINK_BIND_TARGET_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::static_pointer_cast<IpcNotifyBindResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    PeerTargetId targetId = pReq->GetPeerTargetId();
    int32_t result = pReq->GetResult();
    int32_t status = pReq->GetStatus();
    std::string content = pReq->GetContent();

    if (!data.WriteString(pkgName)) {
        LOGE("write bind pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!EncodePeerTargetId(targetId, data)) {
        LOGE("write bind peer target id failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write bind result code failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(status)) {
        LOGE("write bind result status failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(content)) {
        LOGE("write bind result content failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SINK_BIND_TARGET_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(UNBIND_TARGET_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyBindResultReq> pReq = std::static_pointer_cast<IpcNotifyBindResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    PeerTargetId targetId = pReq->GetPeerTargetId();
    int32_t result = pReq->GetResult();
    std::string content = pReq->GetContent();

    if (!data.WriteString(pkgName)) {
        LOGE("write unbind pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!EncodePeerTargetId(targetId, data)) {
        LOGE("write unbind peer target id failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write unbind result code failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(content)) {
        LOGE("write unbind result content failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(UNBIND_TARGET_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(REGISTER_PIN_HOLDER_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(CREATE_PIN_HOLDER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    PeerTargetId targetId;
    DecodePeerTargetId(data, targetId);
    std::string payload = data.ReadString();
    DmPinType pinType = static_cast<DmPinType>(data.ReadInt32());
    int32_t result = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(DESTROY_PIN_HOLDER, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    PeerTargetId targetId;
    DecodePeerTargetId(data, targetId);
    DmPinType pinType = static_cast<DmPinType>(data.ReadInt32());
    std::string payload = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_CREATE_PIN_HOLDER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcCreatePinHolderReq> pReq = std::static_pointer_cast<IpcCreatePinHolderReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deviceId = pReq->GetDeviceId();
    int32_t pinType = pReq->GetPinType();
    std::string payload = pReq->GetPayload();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(deviceId)) {
        LOGE("write deviceId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pinType)) {
        LOGE("write pinType failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(payload)) {
        LOGE("write payload failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_CREATE_PIN_HOLDER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DESTROY_PIN_HOLDER, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcDestroyPinHolderReq> pReq = std::static_pointer_cast<IpcDestroyPinHolderReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t pinType = pReq->GetPinType();
    std::string payload = pReq->GetPayload();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pinType)) {
        LOGE("write pinType failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(payload)) {
        LOGE("write payload failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DESTROY_PIN_HOLDER, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_CREATE_PIN_HOLDER_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq = std::static_pointer_cast<IpcNotifyPublishResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t result = pReq->GetResult();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_CREATE_PIN_HOLDER_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DESTROY_PIN_HOLDER_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyPublishResultReq> pReq = std::static_pointer_cast<IpcNotifyPublishResultReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t result = pReq->GetResult();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DESTROY_PIN_HOLDER_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(DP_ACL_ADD, MessageParcel &data, MessageParcel &reply)
{
    std::string udid = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().DpAclAdd(udid);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_SECURITY_LEVEL, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string networkId = data.ReadString();
    int32_t securityLevel = -1;
    int32_t result = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, networkId, securityLevel);
    if (!reply.WriteInt32(result)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteInt32(securityLevel)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_ON_PIN_HOLDER_EVENT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyPinHolderEventReq> pReq = std::static_pointer_cast<IpcNotifyPinHolderEventReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    int32_t pinHolderEvent = pReq->GetPinHolderEvent();
    int32_t result = pReq->GetResult();
    std::string content = pReq->GetContent();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(pinHolderEvent)) {
        LOGE("write pinHolderEvent failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(content)) {
        LOGE("write content failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_ON_PIN_HOLDER_EVENT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(IS_SAME_ACCOUNT, MessageParcel &data, MessageParcel &reply)
{
    std::string netWorkId = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().IsSameAccount(netWorkId);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(CHECK_API_PERMISSION, MessageParcel &data, MessageParcel &reply)
{
    int32_t permissionLevel = data.ReadInt32();
    int32_t result = DeviceManagerService::GetInstance().CheckApiPermission(permissionLevel);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(CHECK_ACCESS_CONTROL, MessageParcel &data, MessageParcel &reply)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    DecodeDmAccessCaller(data, caller);
    DecodeDmAccessCallee(data, callee);
    int32_t result = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(CHECK_SAME_ACCOUNT, MessageParcel &data, MessageParcel &reply)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    DecodeDmAccessCaller(data, caller);
    DecodeDmAccessCallee(data, callee);
    int32_t result = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}


ON_IPC_CMD(SHIFT_LNN_GEAR, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, pkgName, true, true);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(SET_DN_POLICY, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string policyStr = data.ReadString();
    std::map<std::string, std::string> policy;
    ParseMapFromJsonString(policyStr, policy);
    int32_t result = DeviceManagerService::GetInstance().SetDnPolicy(pkgName, policy);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(STOP_AUTHENTICATE_DEVICE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t result = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(REMOTE_DEVICE_TRUST_CHANGE, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyDevTrustChangeReq> pReq = std::static_pointer_cast<IpcNotifyDevTrustChangeReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string udid = pReq->GetUdid();
    int32_t authForm = pReq->GetAuthForm();
    std::string uuid = pReq->GetUuid();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(udid)) {
        LOGE("write udid failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(authForm)) {
        LOGE("write authForm code failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(uuid)) {
        LOGE("write uuid code failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(REMOTE_DEVICE_TRUST_CHANGE, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVER_DEVICE_SCREEN_STATE_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyDeviceStateReq> pReq = std::static_pointer_cast<IpcNotifyDeviceStateReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    DmDeviceInfo deviceInfo = pReq->GetDeviceInfo();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!EncodeDmDeviceInfo(deviceInfo, data)) {
        LOGE("write dm device info failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVER_DEVICE_SCREEN_STATE_NOTIFY, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(GET_DEVICE_SCREEN_STATUS, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string networkId = data.ReadString();
    int32_t screenStatus = -1;
    int32_t result = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    if (!reply.WriteInt32(result)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteInt32(screenStatus)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_NETWORKID_BY_UDID, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string udid = data.ReadString();
    std::string netWorkId;
    int32_t result = DeviceManagerService::GetInstance().GetNetworkIdByUdid(pkgName, udid, netWorkId);

    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(netWorkId)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_ANONY_LOCAL_UDID, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string anonyUdid;
    int32_t result = DeviceManagerService::GetInstance().GetAnonyLocalUdid(pkgName, anonyUdid);
    if (!reply.WriteInt32(result)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(anonyUdid)) {
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(SERVICE_CREDENTIAL_AUTH_STATUS_NOTIFY, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyCredentialAuthStatusReq> pReq =
        std::static_pointer_cast<IpcNotifyCredentialAuthStatusReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    std::string deviceList = pReq->GetDeviceList();
    uint16_t deviceTypeId = pReq->GetDeviceTypeId();
    int32_t errCode = pReq->GetErrCode();

    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteString(deviceList)) {
        LOGE("write deviceList failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteUint16(deviceTypeId)) {
        LOGE("write deviceTypeId failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!data.WriteInt32(errCode)) {
        LOGE("write errCode failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(SERVICE_CREDENTIAL_AUTH_STATUS_NOTIFY, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(SYNC_CALLBACK, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t dmCommonNotifyEvent = data.ReadInt32();
    ProcessInfo processInfo;
    processInfo.pkgName = pkgName;
    MultipleUserConnector::GetCallerUserId(processInfo.userId);
    int32_t result = DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo);
    if (dmCommonNotifyEvent == static_cast<int32_t>(DmCommonNotifyEvent::REG_DEVICE_STATE)) {
        DeviceManagerService::GetInstance().RegDevStateCallbackToService(pkgName);
    }
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(REG_AUTHENTICATION_TYPE, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    std::string authTypeStr = data.ReadString();
    std::map<std::string, std::string> authParam;
    ParseMapFromJsonString(authTypeStr, authParam);
    int32_t result = DeviceManagerService::GetInstance().RegisterAuthenticationType(pkgName, authParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_DEVICE_PROFILE_INFO_LIST, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmDeviceProfileInfoFilterOptions filterOptions;
    int32_t ret = IpcModelCodec::DecodeDmDeviceProfileInfoFilterOptions(data, filterOptions);
    if (ret != DM_OK) {
        LOGE("DecodeDmDeviceProfileInfoFilterOptions fail ret:%{public}d,", ret);
        return ret;
    }
    int32_t result = DeviceManagerService::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_DEVICE_PROFILE_INFO_LIST_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyGetDeviceProfileInfoListReq> pReq =
        std::static_pointer_cast<IpcNotifyGetDeviceProfileInfoListReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    int32_t result = pReq->GetResult();
    if (!data.WriteInt32(result)) {
        LOGE("write result code failed");
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

ON_IPC_READ_RESPONSE(GET_DEVICE_PROFILE_INFO_LIST_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(PUT_DEVICE_PROFILE_INFO_LIST, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t deviceNum = data.ReadInt32();
    std::vector<DmDeviceProfileInfo> deviceProfileInfoList;
    if (deviceNum > 0 && deviceNum <= MAX_DEVICE_PROFILE_SIZE) {
        for (int32_t i = 0; i < deviceNum; ++i) {
            DmDeviceProfileInfo deviceInfo;
            IpcModelCodec::DecodeDmDeviceProfileInfo(data, deviceInfo);
            deviceProfileInfoList.emplace_back(deviceInfo);
        }
    }
    int32_t result = DeviceManagerService::GetInstance().PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_DEVICE_ICON_INFO, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    DmDeviceIconInfoFilterOptions filterOptions;
    IpcModelCodec::DecodeDmDeviceIconInfoFilterOptions(data, filterOptions);
    int32_t result = DeviceManagerService::GetInstance().GetDeviceIconInfo(pkgName, filterOptions);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_SET_REQUEST(GET_DEVICE_ICON_INFO_RESULT, std::shared_ptr<IpcReq> pBaseReq, MessageParcel &data)
{
    CHECK_NULL_RETURN(pBaseReq, ERR_DM_FAILED);
    std::shared_ptr<IpcNotifyGetDeviceIconInfoReq> pReq =
        std::static_pointer_cast<IpcNotifyGetDeviceIconInfoReq>(pBaseReq);
    std::string pkgName = pReq->GetPkgName();
    if (!data.WriteString(pkgName)) {
        LOGE("write pkgName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    int32_t result = pReq->GetResult();
    if (!data.WriteInt32(result)) {
        LOGE("write result code failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!IpcModelCodec::EncodeDmDeviceIconInfo(pReq->GetDmDeviceIconInfo(), data)) {
        LOGE("write dm device icon info failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_READ_RESPONSE(GET_DEVICE_ICON_INFO_RESULT, MessageParcel &reply, std::shared_ptr<IpcRsp> pBaseRsp)
{
    CHECK_NULL_RETURN(pBaseRsp, ERR_DM_FAILED);
    pBaseRsp->SetErrCode(reply.ReadInt32());
    return DM_OK;
}

ON_IPC_CMD(GET_LOCAL_DISPLAY_DEVICE_NAME, MessageParcel &data, MessageParcel &reply)
{
    std::string pkgName = data.ReadString();
    int32_t maxNameLength = data.ReadInt32();
    std::string displayName = "";
    int32_t result = DeviceManagerService::GetInstance().GetLocalDisplayDeviceName(pkgName, maxNameLength, displayName);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(displayName)) {
        LOGE("write displayName failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(REG_LOCALSERVICE_INFO, MessageParcel &data, MessageParcel &reply)
{
    DMLocalServiceInfo serviceInfo;
    IpcModelCodec::DecodeLocalServiceInfo(data, serviceInfo);
    int32_t result = DeviceManagerService::GetInstance().RegisterLocalServiceInfo(serviceInfo);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UNREG_LOCALSERVICE_INFO, MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    int32_t pinExchangeType = data.ReadInt32();
    int32_t result = DeviceManagerService::GetInstance().UnRegisterLocalServiceInfo(bundleName, pinExchangeType);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(UPDATE_LOCALSERVICE_INFO, MessageParcel &data, MessageParcel &reply)
{
    DMLocalServiceInfo serviceInfo;
    IpcModelCodec::DecodeLocalServiceInfo(data, serviceInfo);
    int32_t result = DeviceManagerService::GetInstance().UpdateLocalServiceInfo(serviceInfo);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

ON_IPC_CMD(GET_SERVICEINFO_BYBUNDLENAME_PINEXCHANGETYPE, MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    int64_t pinExchangeType = data.ReadInt32();
    DMLocalServiceInfo serviceInfo;
    int32_t result = DeviceManagerService::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(
        bundleName, pinExchangeType, serviceInfo);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (result == DM_OK && !IpcModelCodec::EncodeLocalServiceInfo(serviceInfo, reply)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS