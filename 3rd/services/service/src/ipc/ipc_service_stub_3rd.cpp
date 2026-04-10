/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ipc_service_stub_3rd.h"

#include <cstring>
#include <securec.h>

#include "ipc_skeleton.h"
#include "ipc_object_stub.h"   // for IPCObjectStub

#include "permission_manager_3rd.h"
#include "dm_error_type_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "ipc_utils_3rd.h"
#include "dm_anonymous_3rd.h"

namespace OHOS { class MessageOption; }
namespace OHOS { class MessageParcel; }

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE_3RD(IpcServiceStub3rd);
namespace {
    const std::u16string descriptorStr = u"ohos.distributedhardware.devicemanager";
    constexpr int32_t MAX_CALLBACK_NUM = 5000;
}

void AppDeathRecipient3rd::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    ProcessInfo3rd processInfo3rd = IpcServiceStub3rd::GetInstance().GetDmListenerPkgName(remote);
    LOGI("AppDeathRecipient3rd, OnRemoteDied for %{public}s", processInfo3rd.businessName.c_str());
}

int32_t IpcServiceStub3rd::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::u16string remoteDesc = data.ReadInterfaceToken();
    if (descriptorStr != remoteDesc) {
        LOGE("OnRemoteRequest remoteDesc is invalid!:");
        return ERR_DM_FAILED;
    }
    LOGI("code: %{public}d", code);
    switch (code) {
        case INIT_DEVICE_MANAGER:
            return InitDeviceManager(data, reply, option);
        case IMPORT_PINCODE_3RD:
            return ImportPinCode3rd(data, reply, option);
        case GENERATE_PINCODE_3RD:
            return GeneratePinCode(data, reply, option);
        case AUTH_PINCODE_3RD:
            return AuthPincode(data, reply, option);
        case AUTH_DEVICE_3RD:
            return AuthDevice3rd(data, reply, option);
        case QUERY_TRUST_RELATION_3RD:
            return QueryTrustRelation(data, reply, option);
        case DELETE_TRUST_RELATION_3RD:
            return DeleteTrustRelation(data, reply, option);
        default:
            LOGE("invalid request code.");
            IPCObjectStub iPCObjectStub;
            return iPCObjectStub.OnRemoteRequest(code, data, reply, option);
    }
}

int32_t IpcServiceStub3rd::InitDeviceManager(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string businessName = data.ReadString();
    sptr<IRemoteObject> listener = data.ReadRemoteObject();
    if (listener == nullptr) {
        LOGE("read remote object failed.");
        return ERR_DM_POINT_NULL;
    }
    sptr<IpcServerClientProxy3rd> callback(new IpcServerClientProxy3rd(listener));
    if (callback == nullptr) {
        LOGE("create ipc server client proxy failed.");
        return ERR_DM_POINT_NULL;
    }

    std::string processName = "";
    if (PermissionManager3rd::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed.");
        return ERR_DM_FAILED;
    }
    ProcessInfo3rd processInfo3rd;
    processInfo3rd.tokenId = OHOS::IPCSkeleton::GetCallingTokenID();
    processInfo3rd.uid = OHOS::IPCSkeleton::GetCallingUid();
    processInfo3rd.processName = processName;
    processInfo3rd.businessName = businessName;
    MultipleUserConnector3rd::GetCallerUserId(processInfo3rd.userId);
    int32_t result = RegisterDeviceManagerListener(processInfo3rd, callback);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

const sptr<IpcRemoteBroker3rd> IpcServiceStub3rd::GetDmListener(ProcessInfo3rd processInfo3rd)
{
    if (processInfo3rd.businessName.empty()) {
        LOGE("Invalid parameter, businessName is empty.");
        return nullptr;
    }
    SetSaUserId(processInfo3rd);
    std::lock_guard<ffrt::mutex> autoLock(listenerLock_);
    auto iter = dmListener_.find(processInfo3rd);
    if (iter == dmListener_.end()) {
        return nullptr;
    }
    return iter->second;
}

const ProcessInfo3rd IpcServiceStub3rd::GetDmListenerPkgName(const wptr<IRemoteObject> &remote)
{
    ProcessInfo3rd processInfo3rd;
    std::lock_guard<ffrt::mutex> autoLock(listenerLock_);
    for (const auto &iter : dmListener_) {
        if ((iter.second)->AsObject() == remote.promote()) {
            processInfo3rd = iter.first;
            break;
        }
    }
    return processInfo3rd;
}

int32_t IpcServiceStub3rd::RegisterDeviceManagerListener(const ProcessInfo3rd &processInfo3rd,
    sptr<IpcRemoteBroker3rd> listener)
{
    LOGI("pkgName: %{public}s, userId: %{public}d", processInfo3rd.businessName.c_str(), processInfo3rd.uid);
    if (processInfo3rd.businessName.empty() || listener == nullptr) {
        LOGE("error: input parameter invalid.");
        return ERR_DM_POINT_NULL;
    }
    std::lock_guard<ffrt::mutex> autoLock(listenerLock_);
    auto iter = dmListener_.find(processInfo3rd);
    if (iter != dmListener_.end()) {
        LOGI("Listener already exists");
        auto recipientIter = appRecipient3rd_.find(processInfo3rd);
        if (recipientIter == appRecipient3rd_.end()) {
            LOGI("AppRecipient not exists");
            dmListener_.erase(processInfo3rd);
        } else {
            auto listener = iter->second;
            auto appRecipient3rd = recipientIter->second;
            listener->AsObject()->RemoveDeathRecipient(appRecipient3rd);
            appRecipient3rd_.erase(processInfo3rd);
            dmListener_.erase(processInfo3rd);
        }
    }
    sptr<AppDeathRecipient3rd> appRecipient3rd = sptr<AppDeathRecipient3rd>(new AppDeathRecipient3rd());
    LOGI("Add death recipient.");
    if (!listener->AsObject()->AddDeathRecipient(appRecipient3rd)) {
        LOGE("AddDeathRecipient Failed");
    }
    if (dmListener_.size() >= MAX_CALLBACK_NUM || appRecipient3rd_.size() >= MAX_CALLBACK_NUM) {
        LOGE("dmListener_ or appRecipient3rd_ size exceed the limit!");
        return ERR_DM_FAILED;
    }
    dmListener_[processInfo3rd] = listener;
    appRecipient3rd_[processInfo3rd] = appRecipient3rd;
    AddSystemSA(processInfo3rd.processName);
    LOGI("complete.");
    return DM_OK;
}

void IpcServiceStub3rd::AddSystemSA(const std::string &pkgName)
{
    if (!PermissionManager3rd::GetInstance().CheckSystemSA(pkgName)) {
        return;
    }
    std::lock_guard<ffrt::mutex> lock(systemSAMtx_);
    systemSA_.insert(pkgName);
}

void IpcServiceStub3rd::SetSaUserId(ProcessInfo3rd &processInfo3rd)
{
    std::lock_guard<ffrt::mutex> lock(systemSAMtx_);
    if (systemSA_.find(processInfo3rd.processName) == systemSA_.end()) {
        return;
    }
    processInfo3rd.userId = 0;
}

int32_t IpcServiceStub3rd::ImportPinCode3rd(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string businessName = data.ReadString();
    std::string pincode = data.ReadString();
    int32_t result = DeviceManagerService3rd::GetInstance().ImportPinCode3rd(businessName, pincode);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

int32_t IpcServiceStub3rd::GeneratePinCode(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    uint32_t pinLength = data.ReadUint32();
    std::string pincode;
    int32_t result = DeviceManagerService3rd::GetInstance().GeneratePinCode(pinLength, pincode);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (!reply.WriteString(pincode)) {
        LOGE("write pincode failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

void DecodePeerTargetId(MessageParcel &parcel, PeerTargetId3rd &targetId)
{
    targetId.deviceId = parcel.ReadString();
    targetId.brMac = parcel.ReadString();
    targetId.bleMac = parcel.ReadString();
    targetId.wifiIp = parcel.ReadString();
    targetId.wifiPort = parcel.ReadUint16();
}

int32_t IpcServiceStub3rd::AuthPincode(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    PeerTargetId3rd targetId;
    DecodePeerTargetId(data, targetId);
    std::string bindParamStr = data.ReadString();
    std::map<std::string, std::string> bindParam;
    ParseMapFromJsonString(bindParamStr, bindParam);
    int32_t result = DeviceManagerService3rd::GetInstance().AuthPincode(targetId, bindParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

int IpcServiceStub3rd::OnAuth3rdAclSessionOpened(int sessionId, int result)
{
    return DeviceManagerService3rd::GetInstance().OnAuth3rdAclSessionOpened(sessionId, result);
}

void IpcServiceStub3rd::OnAuth3rdAclSessionClosed(int sessionId)
{
    DeviceManagerService3rd::GetInstance().OnAuth3rdAclSessionClosed(sessionId);
}

void IpcServiceStub3rd::OnAuth3rdAclBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    DeviceManagerService3rd::GetInstance().OnAuth3rdAclBytesReceived(sessionId, data, dataLen);
}

int IpcServiceStub3rd::OnAuth3rdSessionOpened(int sessionId, int result)
{
    return DeviceManagerService3rd::GetInstance().OnAuth3rdSessionOpened(sessionId, result);
}

void IpcServiceStub3rd::OnAuth3rdSessionClosed(int sessionId)
{
    DeviceManagerService3rd::GetInstance().OnAuth3rdSessionClosed(sessionId);
}

void IpcServiceStub3rd::OnAuth3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    DeviceManagerService3rd::GetInstance().OnAuth3rdBytesReceived(sessionId, data, dataLen);
}

int32_t IpcServiceStub3rd::AuthDevice3rd(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    (void)option;
    PeerTargetId3rd targetId;
    targetId.deviceId = data.ReadString();
    targetId.brMac = data.ReadString();
    targetId.bleMac = data.ReadString();
    targetId.wifiIp = data.ReadString();
    targetId.wifiPort = data.ReadUint16();
    std::string authParamStr = data.ReadString();
    std::map<std::string, std::string> authParam;
    ParseMapFromJsonString(authParamStr, authParam);

    int32_t result = DeviceManagerService3rd::GetInstance().AuthDevice3rd(targetId, authParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

int32_t IpcServiceStub3rd::QueryTrustRelation(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    (void)option;
    std::string businessName = data.ReadString();
    std::vector<TrustDeviceInfo3rd> trustedDeviceList;
    int32_t result = DeviceManagerService3rd::GetInstance().QueryTrustRelation(businessName, trustedDeviceList);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    if (result != DM_OK) {
        return result;
    }
    if (!reply.WriteInt32(static_cast<int32_t>(trustedDeviceList.size()))) {
        LOGE("write trustedDeviceList size failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    for (auto &deviceInfo : trustedDeviceList) {
        std::vector<unsigned char> sessionKeyVec = std::vector<unsigned char>(deviceInfo.sessionKey.key,
            deviceInfo.sessionKey.key + deviceInfo.sessionKey.keyLen);
        std::string skStr(sessionKeyVec.begin(), sessionKeyVec.end());
        if (deviceInfo.sessionKey.key != nullptr) {
            (void)memset_s(deviceInfo.sessionKey.key, deviceInfo.sessionKey.keyLen, 0, deviceInfo.sessionKey.keyLen);
            free(deviceInfo.sessionKey.key);
            deviceInfo.sessionKey.key = nullptr;
            deviceInfo.sessionKey.keyLen = 0;
        }
        if (!reply.WriteString(deviceInfo.trustDeviceId) ||
            !reply.WriteInt32(deviceInfo.sessionKeyId) ||
            !reply.WriteInt64(deviceInfo.createTime) ||
            !reply.WriteInt32(deviceInfo.userId) ||
            !reply.WriteString(deviceInfo.extra) ||
            !reply.WriteInt32(deviceInfo.bindLevel) ||
            !reply.WriteInt32(deviceInfo.bindType) ||
            !reply.WriteString(skStr)) {
            LOGE("write trustedDeviceList item failed");
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    return DM_OK;
}

int32_t IpcServiceStub3rd::DeleteTrustRelation(MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    (void)option;
    std::string businessName = data.ReadString();
    std::string peerDeviceId = data.ReadString();
    std::string unbindParamStr = data.ReadString();
    std::map<std::string, std::string> unbindParam;
    ParseMapFromJsonString(unbindParamStr, unbindParam);
    int32_t result = DeviceManagerService3rd::GetInstance().DeleteTrustRelation(businessName, peerDeviceId,
        unbindParam);
    if (!reply.WriteInt32(result)) {
        LOGE("write result failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    return DM_OK;
}

int32_t IpcServiceStub3rd::HandleUserRemoved(int32_t removedUserId)
{
    return DeviceManagerService3rd::GetInstance().HandleUserRemoved(removedUserId);
}

int32_t IpcServiceStub3rd::HandleAccountLogoutEvent(int32_t userId, const std::string &accountId)
{
    return DeviceManagerService3rd::GetInstance().HandleAccountLogoutEvent(userId, accountId);
}

extern "C" IIpcServiceStub3rd *CreateIpcServiceStub3rdObject(void)
{
    return &IpcServiceStub3rd::GetInstance();
}
}
}