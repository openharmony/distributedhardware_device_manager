/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "device_manager_impl_3rd.h"

#include <memory>
#include <mutex>

#include "device_manager_notify_3rd.h"
#include "dm_auth_info_3rd.h"
#include "dm_error_type_3rd.h"
#include "ipc_utils_3rd.h"
#include "iremote_object.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t USLEEP_TIME_US_100000 = 100000; // 100ms
const int32_t SERVICE_INIT_MAX_NUM = 20;
constexpr int32_t DM_MIN_PINCODE_SIZE = 6;
constexpr int32_t DM_MAX_PINCODE_SIZE = 1024;
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;
constexpr int32_t MAX_TRUST_3RD_DEVICE_SIZE = 500;
}

DeviceManagerImpl3rd &DeviceManagerImpl3rd::GetInstance()
{
    static DeviceManagerImpl3rd instance;
    return instance;
}

int32_t DeviceManagerImpl3rd::RegisterAuthCallback(const std::string &businessName,
    std::shared_ptr<DmAuthCallback> dmAuthCallback)
{
    if (businessName.empty() || dmAuthCallback == nullptr) {
        LOGE("Invalid parameter, businessName is empty or dmAuthCallback is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify3rd::GetInstance().RegisterAuthCallback(businessName, dmAuthCallback);
    LOGI("completed");
    return DM_OK;
}

int32_t DeviceManagerImpl3rd::UnRegisterAuthCallback(const std::string &businessName)
{
    if (businessName.empty()) {
        LOGE("Invalid parameter, businessName is empty .");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DeviceManagerNotify3rd::GetInstance().UnRegisterAuthCallback(businessName);
    LOGI("completed");
    return DM_OK;
}

void DeviceManagerImpl3rd::SubscribeDMSAChangeListener()
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        LOGE("get system ability manager failed.");
        return;
    }
    saListenerCallback_ = sptr<SystemAbilityListener3rd>(new (std::nothrow) SystemAbilityListener3rd());
    if (saListenerCallback_ == nullptr) {
        LOGE("saListenerCallback_ is nullptr.");
        return;
    }
    if (!isSubscribeDMSAChangeListener_.load()) {
        LOGI("try subscribe source sa change listener, sa id: %{public}d", DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        int32_t ret = systemAbilityManager->SubscribeSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID,
            saListenerCallback_);
        if (ret != DM_OK) {
            LOGE("subscribe source sa change failed: %{public}d", ret);
            saListenerCallback_ = nullptr;
            return;
        }
        isSubscribeDMSAChangeListener_.store(true);
    }
}

int32_t DeviceManagerImpl3rd::ClientInit()
{
    LOGI("Start");
    if (dm3rdremoteObject_ != nullptr) {
        LOGI("DeviceManagerService Already Init");
        return DM_OK;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Get SystemAbilityManager Failed");
        return ERR_DM_INIT_FAILED;
    }

    auto object = samgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (object == nullptr) {
        LOGE("Get DeviceManager SystemAbility Failed");
        DmServiceLoad3rd::GetInstance().LoadDMService();
        return ERR_DM_INIT_FAILED;
    }

    dm3rdremoteObject_ = object;
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl3rd::SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    CHECK_NULL_RETURN(dm3rdremoteObject_, ERR_DM_POINT_NULL);
    MessageOption option;
    return dm3rdremoteObject_->SendRequest(code, data, reply, option);
}

int32_t DeviceManagerImpl3rd::InitDeviceManager(const std::string &businessName,
    std::shared_ptr<DmInit3rdCallback> dmInit3rdCallback)
{
    if (businessName.empty()) {
        LOGE("Invalid parameter, businessName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, businessName: %{public}s", businessName.c_str());
    SubscribeDMSAChangeListener();
    int32_t errcode = DM_OK;
    int32_t retryNum = 0;
    while (retryNum < SERVICE_INIT_MAX_NUM) {
        errcode = ClientInit();
        if (errcode == DM_OK) {
            break;
        }
        usleep(USLEEP_TIME_US_100000);
        retryNum++;
        if (retryNum == SERVICE_INIT_MAX_NUM) {
            LOGE("InitDeviceManager error, wait for device manager service starting timeout.");
            return ERR_DM_NOT_INIT;
        }
    }
    sptr<IpcClientStub3rd> listener = sptr<IpcClientStub3rd>(new (std::nothrow) IpcClientStub3rd());
    CHECK_NULL_RETURN(listener, ERR_DM_POINT_NULL);

    MessageParcel data;
    WRITE_INTERFACE_TOKEN(data, ERR_DM_IPC_WRITE_FAILED);
    WRITE_HELPER_RET(data, String, businessName, ERR_DM_IPC_WRITE_FAILED);
    if (!data.WriteRemoteObject(listener)) {
        LOGE("write listener failed");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    MessageParcel reply;
    int32_t sendRet = SendRequest(INIT_DEVICE_MANAGER, data, reply);
    if (sendRet != DM_OK) {
        LOGE("SendRequest failed, ret: %{public}d", sendRet);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    READ_HELPER_RET(reply, Int32, errcode, ERR_DM_IPC_READ_FAILED);
    if (errcode != DM_OK) {
        LOGE("InitDeviceManager failed, result: %{public}d", errcode);
        return errcode;
    }
    DeviceManagerNotify3rd::GetInstance().RegisterDeathRecipientCallback(businessName, dmInit3rdCallback);

    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl3rd::ImportPinCode3rd(const std::string &businessName, const std::string &pinCode)
{
    if (businessName.empty() || pinCode.empty()) {
        LOGE("Invalid parameter, businessName or pinCode is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, businessName: %{public}s", businessName.c_str());

    MessageParcel data;
    WRITE_INTERFACE_TOKEN(data, ERR_DM_IPC_WRITE_FAILED);
    WRITE_HELPER_RET(data, String, businessName, ERR_DM_IPC_WRITE_FAILED);
    WRITE_HELPER_RET(data, String, pinCode, ERR_DM_IPC_WRITE_FAILED);

    MessageParcel reply;
    int32_t ret = SendRequest(IMPORT_PINCODE_3RD, data, reply);
    if (ret != DM_OK) {
        LOGE("SendRequest failed, ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    READ_HELPER_RET(reply, Int32, ret, ERR_DM_IPC_READ_FAILED);
    if (ret != DM_OK) {
        LOGE("ImportPinCode3rd failed, result: %{public}d", ret);
        return ret;
    }
    LOGI("Completed");
    return DM_OK;
}

int32_t DeviceManagerImpl3rd::GeneratePinCode(uint32_t pinLength, std::string &pincode)
{
    if (pinLength < DM_MIN_PINCODE_SIZE || pinLength > DM_MAX_PINCODE_SIZE) {
        LOGE("Invalid pinLength: %{public}u", pinLength);
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("Start, pinLength: %{public}u", pinLength);

    MessageParcel data;
    WRITE_INTERFACE_TOKEN(data, ERR_DM_IPC_WRITE_FAILED);
    WRITE_HELPER_RET(data, Uint32, pinLength, ERR_DM_IPC_WRITE_FAILED);

    MessageParcel reply;
    int32_t ret = SendRequest(GENERATE_PINCODE_3RD, data, reply);
    if (ret != DM_OK) {
        LOGE("SendRequest failed, ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    READ_HELPER_RET(reply, Int32, ret, ERR_DM_IPC_READ_FAILED);
    if (ret != DM_OK) {
        LOGE("GeneratePinCode failed, result: %{public}d", ret);
        return ret;
    }
    READ_HELPER_RET(reply, String, pincode, ERR_DM_IPC_READ_FAILED);

    LOGI("Completed, pincode length: %{public}zu", pincode.length());
    return DM_OK;
}

bool DeviceManagerImpl3rd::IsInvalidPeerTargetId(const PeerTargetId3rd &targetId)
{
    return targetId.deviceId.empty() && targetId.brMac.empty() && targetId.bleMac.empty() && targetId.wifiIp.empty();
}

bool EncodePeerTargetId(const PeerTargetId3rd &targetId, MessageParcel &parcel)
{
    bool bRet = true;
    bRet = (bRet && parcel.WriteString(targetId.deviceId));
    bRet = (bRet && parcel.WriteString(targetId.brMac));
    bRet = (bRet && parcel.WriteString(targetId.bleMac));
    bRet = (bRet && parcel.WriteString(targetId.wifiIp));
    bRet = (bRet && parcel.WriteUint16(targetId.wifiPort));
    return bRet;
}

int32_t DeviceManagerImpl3rd::AuthPincode(const PeerTargetId3rd &targetId,
    std::map<std::string, std::string> &authParam)
{
    if (IsInvalidPeerTargetId(targetId)) {
        LOGE("DeviceManagerImpl3rd::AuthPincode failed: input targetId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, deviceId: %{public}s", GetAnonyString(targetId.deviceId).c_str());

    MessageParcel data;
    WRITE_INTERFACE_TOKEN(data, ERR_DM_IPC_WRITE_FAILED);
    EncodePeerTargetId(targetId, data);

    std::string authParamStr = ConvertMapToJsonString(authParam);
    WRITE_HELPER_RET(data, String, authParamStr, ERR_DM_IPC_WRITE_FAILED);

    MessageParcel reply;
    int32_t ret = SendRequest(AUTH_PINCODE_3RD, data, reply);
    if (ret != DM_OK) {
        LOGE("SendRequest failed, ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    READ_HELPER_RET(reply, Int32, ret, ERR_DM_IPC_READ_FAILED);
    if (ret != DM_OK) {
        LOGE("AuthPincode failed, ret: %{public}d", ret);
        return ret;
    }

    LOGI("Completed");
    return DM_OK;
}

DeviceManagerImpl3rd::SystemAbilityListener3rd::SystemAbilityListener3rd()
{
    LOGI("SystemAbilityListener3rd create");
}
 
DeviceManagerImpl3rd::SystemAbilityListener3rd::~SystemAbilityListener3rd()
{
    LOGI("SystemAbilityListener3rd destroy");
}

void DeviceManagerImpl3rd::SystemAbilityListener3rd::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    LOGI("sa %{public}d is added.", systemAbilityId);
}

void DeviceManagerImpl3rd::SystemAbilityListener3rd::OnRemoveSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    LOGI("sa %{public}d is removed.", systemAbilityId);
    if (systemAbilityId == DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID) {
        DeviceManagerNotify3rd::GetInstance().OnRemoteDied();
    }
}

int32_t DeviceManagerImpl3rd::AuthDevice3rd(const PeerTargetId3rd &targetId, std::map<std::string,
    std::string> &authParam)
{
    if (IsInvalidPeerTargetId(targetId)) {
        LOGE("AuthDevice3rd failed: invalid targetId.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::string authParamStr = ConvertMapToJsonString(authParam);
    if (!authParam.empty() && authParamStr.empty()) {
        LOGE("AuthDevice3rd failed: authParam convert error.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    MessageParcel data;
    WRITE_INTERFACE_TOKEN(data, ERR_DM_IPC_WRITE_FAILED);
    if (!data.WriteString(targetId.deviceId) ||
        !data.WriteString(targetId.brMac) ||
        !data.WriteString(targetId.bleMac) ||
        !data.WriteString(targetId.wifiIp) ||
        !data.WriteUint16(targetId.wifiPort) ||
        !data.WriteString(authParamStr)) {
        LOGE("AuthDevice3rd write request failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }
    MessageParcel reply;
    int32_t sendRet = SendRequest(AUTH_DEVICE_3RD, data, reply);
    if (sendRet != DM_OK) {
        LOGE("AuthDevice3rd send request failed.");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    return reply.ReadInt32();
}

int32_t DeviceManagerImpl3rd::QueryTrustRelation(const std::string &businessName,
    std::vector<TrustDeviceInfo3rd> &trustedDeviceList)
{
    if (businessName.empty()) {
        LOGE("failed: businessName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    auto &dm3rdImpl = DeviceManagerImpl3rd::GetInstance();
    CHECK_NULL_RETURN(dm3rdImpl.dm3rdremoteObject_, ERR_DM_POINT_NULL);

    MessageParcel data;
    WRITE_INTERFACE_TOKEN(data, ERR_DM_IPC_WRITE_FAILED);
    if (!data.WriteString(businessName)) {
        LOGE("write businessName failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }

    MessageParcel reply;
    int32_t sendRet = SendRequest(QUERY_TRUST_RELATION_3RD, data, reply);
    if (sendRet != DM_OK) {
        LOGE("send request failed.");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    trustedDeviceList.clear();
    int32_t ret = reply.ReadInt32();
    if (ret != DM_OK) {
        return ret;
    }

    int32_t listSize = reply.ReadInt32();
    if (listSize < 0 || listSize > MAX_TRUST_3RD_DEVICE_SIZE) {
        LOGE("invalid list size.");
        return ERR_DM_IPC_READ_FAILED;
    }
    for (int32_t i = 0; i < listSize; ++i) {
        TrustDeviceInfo3rd deviceInfo;
        deviceInfo.trustDeviceId = reply.ReadString();
        deviceInfo.sessionKeyId = reply.ReadInt32();
        deviceInfo.createTime = reply.ReadInt64();
        deviceInfo.userId = reply.ReadInt32();
        deviceInfo.extra = reply.ReadString();
        std::string keyStr = reply.ReadString();
        BuildSessionKey(keyStr, deviceInfo);
        trustedDeviceList.push_back(deviceInfo);
    }
    return DM_OK;
}

int32_t DeviceManagerImpl3rd::BuildSessionKey(const std::string &keyStr, TrustDeviceInfo3rd &deviceInfo)
{
    size_t keyLen = keyStr.size();
    if (keyLen > MAX_SESSION_KEY_LENGTH) {
        LOGE("SessionKey too long, len: %{public}zu", keyLen);
        return ERR_DM_SAVE_SESSION_KEY_FAILED;
    }
    deviceInfo.sessionKey.key = (uint8_t*)calloc(keyLen, sizeof(uint8_t));
    if (deviceInfo.sessionKey.key == nullptr) {
        LOGE("calloc sessionKey failed");
        return ERR_DM_FAILED;
    }
    if (memcpy_s(deviceInfo.sessionKey.key, keyLen, keyStr.c_str(), keyLen) != DM_OK) {
        LOGE("copy key data failed.");
        if (deviceInfo.sessionKey.key != nullptr) {
            (void)memset_s(deviceInfo.sessionKey.key, keyLen, 0, keyLen);
            free(deviceInfo.sessionKey.key);
            deviceInfo.sessionKey.key = nullptr;
            deviceInfo.sessionKey.keyLen = 0;
        }
        return ERR_DM_FAILED;
    }
    deviceInfo.sessionKey.keyLen = keyLen;
    return DM_OK;
}

int32_t DeviceManagerImpl3rd::DeleteTrustRelation(const std::string &businessName, const std::string &peerDeviceId,
    std::map<std::string, std::string> &unbindParam)
{
    if (businessName.empty() || peerDeviceId.empty()) {
        LOGE("DeleteTrustRelation failed: businessName or peerDeviceId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string unbindParamStr = ConvertMapToJsonString(unbindParam);
    if (!unbindParam.empty() && unbindParamStr.empty()) {
        LOGE("DeleteTrustRelation failed: unbindParam convert error.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    MessageParcel data;
    WRITE_INTERFACE_TOKEN(data, ERR_DM_IPC_WRITE_FAILED);
    if (!data.WriteString(businessName) || !data.WriteString(peerDeviceId) || !data.WriteString(unbindParamStr)) {
        LOGE("DeleteTrustRelation write request failed.");
        return ERR_DM_IPC_WRITE_FAILED;
    }

    MessageParcel reply;
    int32_t sendRet = SendRequest(DELETE_TRUST_RELATION_3RD, data, reply);
    if (sendRet != DM_OK) {
        LOGE("DeleteTrustRelation send request failed.");
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    return reply.ReadInt32();
}
} // namespace DistributedHardware
} // namespace OHOS