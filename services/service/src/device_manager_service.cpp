/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <dlfcn.h>
#include <functional>

#include "app_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_hidumper.h"
#include "dm_log.h"
#include "parameter.h"
#include "permission_manager.h"

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
constexpr const char* LIB_IMPL_NAME = "libdevicemanagerserviceimpl.z.so";
#else
constexpr const char* LIB_IMPL_NAME = "libdevicemanagerserviceimpl.so";
#endif
constexpr const char* LIB_DM_ADAPTER_NAME = "libdevicemanageradapter.z.so";

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);
const int32_t NORMAL = 0;
const int32_t SYSTEM_BASIC = 1;
const int32_t SYSTEM_CORE = 2;
constexpr const char* ALL_PKGNAME = "";
DeviceManagerService::~DeviceManagerService()
{
    LOGI("DeviceManagerService destructor");
    UnloadDMServiceImplSo();
    UnloadDMServiceAdapter();
}

int32_t DeviceManagerService::Init()
{
    InitSoftbusListener();
    InitDMServiceListener();
    LOGI("Init success, dm service single instance initialized.");
    return DM_OK;
}

int32_t DeviceManagerService::InitSoftbusListener()
{
    if (softbusListener_ == nullptr) {
        softbusListener_ = std::make_shared<SoftbusListener>();
    }
    LOGI("SoftbusListener init success.");
    return DM_OK;
}

void DeviceManagerService::UninitSoftbusListener()
{
    softbusListener_ = nullptr;
    LOGI("SoftbusListener uninit.");
}

int32_t DeviceManagerService::InitDMServiceListener()
{
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
    }
    if (advertiseMgr_ == nullptr) {
        advertiseMgr_ = std::make_shared<AdvertiseManager>(softbusListener_);
    }
    if (discoveryMgr_ == nullptr) {
        discoveryMgr_ = std::make_shared<DiscoveryManager>(softbusListener_, listener_);
    }
    if (pinHolder_ == nullptr) {
        pinHolder_ = std::make_shared<PinHolder>(listener_);
    }

    LOGI("DeviceManagerServiceListener init success.");
    return DM_OK;
}

void DeviceManagerService::UninitDMServiceListener()
{
    listener_ = nullptr;
    advertiseMgr_ = nullptr;
    discoveryMgr_ = nullptr;
    LOGI("DeviceManagerServiceListener uninit.");
}

void DeviceManagerService::RegisterDeviceManagerListener(const std::string &pkgName)
{
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
    }
    listener_->RegisterDmListener(pkgName, AppManager::GetInstance().GetAppId());
}

void DeviceManagerService::UnRegisterDeviceManagerListener(const std::string &pkgName)
{
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
    }
    listener_->UnRegisterDmListener(pkgName);
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                   std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("DeviceManagerService::GetTrustedDeviceList begin for pkgName = %{public}s, extra = %{public}s",
        pkgName.c_str(), extra.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::vector<DmDeviceInfo> onlineDeviceList;
    if (softbusListener_->GetTrustedDeviceList(onlineDeviceList) != DM_OK) {
        LOGE("GetTrustedDeviceList failed");
        return ERR_DM_FAILED;
    }
    if (onlineDeviceList.size() > 0 && IsDMServiceImplReady()) {
        std::map<std::string, DmAuthForm> udidMap;
        if (PermissionManager::GetInstance().CheckSA()) {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(std::string(ALL_PKGNAME));
        } else {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(pkgName);
        }
        for (auto item : onlineDeviceList) {
            std::string udid = "";
            SoftbusListener::GetUdidByNetworkId(item.networkId, udid);
            if (udidMap.find(udid) != udidMap.end()) {
                std::string deviceIdHash = "";
                dmServiceImpl_->GetUdidHashByNetWorkId(item.networkId, deviceIdHash);
                if (memcpy_s(item.deviceId, DM_MAX_DEVICE_ID_LEN, deviceIdHash.c_str(), deviceIdHash.length()) != 0) {
                    LOGE("get deviceId: %{public}s failed", GetAnonyString(deviceIdHash).c_str());
                }
                item.authForm = udidMap[udid];
                deviceList.push_back(item);
            }
        }
        LOGI("Current app available device size: %{public}zu", deviceList.size());
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetAvailableDeviceList(const std::string &pkgName,
    std::vector<DmDeviceBasicInfo> &deviceBasicInfoList)
{
    LOGI("DeviceManagerService::GetAvailableDeviceList begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::vector<DmDeviceBasicInfo> onlineDeviceList;
    if (softbusListener_->GetAvailableDeviceList(onlineDeviceList) != DM_OK) {
        LOGE("GetAvailableDeviceList failed");
        return ERR_DM_FAILED;
    }

    if (onlineDeviceList.size() > 0 && IsDMServiceImplReady()) {
        std::map<std::string, DmAuthForm> udidMap;
        if (PermissionManager::GetInstance().CheckSA()) {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(std::string(ALL_PKGNAME));
        } else {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(pkgName);
        }
        for (auto item : onlineDeviceList) {
            std::string udid = "";
            SoftbusListener::GetUdidByNetworkId(item.networkId, udid);
            if (udidMap.find(udid) != udidMap.end()) {
                std::string deviceIdHash = "";
                dmServiceImpl_->GetUdidHashByNetWorkId(item.networkId, deviceIdHash);
                if (memcpy_s(item.deviceId, DM_MAX_DEVICE_ID_LEN, deviceIdHash.c_str(),
                    deviceIdHash.length()) != 0) {
                    LOGE("get deviceId: %{public}s failed", GetAnonyString(deviceIdHash).c_str());
                }
                deviceBasicInfoList.push_back(item);
            }
        }
        LOGI("Current app available device size: %{public}zu", deviceBasicInfoList.size());
    }
    return DM_OK;
}

int32_t DeviceManagerService::ShiftLNNGear(const std::string &pkgName, const std::string &callerId, bool isRefresh)
{
    LOGI("DeviceManagerService::ShiftLNNGear begin for pkgName = %{public}s, callerId = %{public}s, isRefresh ="
        "%{public}d", pkgName.c_str(), GetAnonyString(callerId).c_str(), isRefresh);
    if (pkgName.empty() || callerId.empty()) {
        LOGE("Invalid parameter, parameter is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (isRefresh) {
        int32_t ret = softbusListener_->ShiftLNNGear();
        if (ret != DM_OK) {
            LOGE("ShiftLNNGear error, failed ret: %{public}d", ret);
            return ret;
        }
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info)
{
    if (!PermissionManager::GetInstance().CheckPermission() &&
        !PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call GetDeviceInfo.");
        return ERR_DM_NO_PERMISSION;
    }
    if (networkId.empty()) {
        LOGE("Invalid parameter, networkId is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::GetDeviceInfo begin by networkId : %{public}s.", GetAnonyString(networkId).c_str());
    int32_t ret = softbusListener_->GetDeviceInfo(networkId, info);
    if (ret != DM_OK) {
        LOGE("Get DeviceInfo By NetworkId failed, ret : %{public}d", ret);
    }
    return ret;
}

int32_t DeviceManagerService::GetLocalDeviceInfo(DmDeviceInfo &info)
{
    LOGI("DeviceManagerService::GetLocalDeviceInfo begin.");
    int32_t ret = softbusListener_->GetLocalDeviceInfo(info);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceInfo failed");
        return ret;
    }
    if (localDeviceId_.empty()) {
        char localDeviceId[DEVICE_UUID_LENGTH] = {0};
        char udidHash[DEVICE_UUID_LENGTH] = {0};
        GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
        if (Crypto::GetUdidHash(localDeviceId, reinterpret_cast<uint8_t *>(udidHash)) == DM_OK) {
            localDeviceId_ = udidHash;
        }
    }

    if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, localDeviceId_.c_str(), localDeviceId_.length()) != 0) {
        LOGE("get deviceId: %{public}s failed", GetAnonyString(localDeviceId_).c_str());
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetLocalDeviceNetworkId(std::string &networkId)
{
    LOGI("DeviceManagerService::GetLocalDeviceNetworkId begin.");
    int32_t ret = softbusListener_->GetLocalDeviceNetworkId(networkId);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceNetworkId failed");
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetLocalDeviceId(const std::string &pkgName, std::string &deviceId)
{
    LOGI("DeviceManagerService::GetLocalDeviceId begin.");
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    char udidHash[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    int32_t ret = Crypto::GetUdidHash(localDeviceId, reinterpret_cast<uint8_t *>(udidHash));
    if (ret != DM_OK) {
        LOGE("get udidhash by udid: %{public}s failed.", GetAnonyString(localDeviceId).c_str());
        deviceId = "";
        return ret;
    }
    deviceId = udidHash;
    return DM_OK;
}

int32_t DeviceManagerService::GetLocalDeviceName(std::string &deviceName)
{
    LOGI("DeviceManagerService::GetLocalDeviceName begin.");
    int32_t ret = softbusListener_->GetLocalDeviceName(deviceName);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceName failed");
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetLocalDeviceType(int32_t &deviceType)
{
    LOGI("DeviceManagerService::GetLocalDeviceType begin.");
    int32_t ret = softbusListener_->GetLocalDeviceType(deviceType);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceType failed");
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &udid)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetUdidByNetworkId.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::GetUdidByNetworkId begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, netWorkId: %{public}s", pkgName.c_str(),
            GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SoftbusListener::GetUdidByNetworkId(netWorkId.c_str(), udid);
    return DM_OK;
}

int32_t DeviceManagerService::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &uuid)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetUuidByNetworkId.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::GetUuidByNetworkId begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, netWorkId: %{public}s", pkgName.c_str(),
            GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    softbusListener_->GetUuidByNetworkId(netWorkId.c_str(), uuid);
    return DM_OK;
}

int32_t DeviceManagerService::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
    const std::string &extra)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call StartDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::StartDeviceDiscovery begin for pkgName = %{public}s, extra = %{public}s",
        pkgName.c_str(), extra.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    SoftbusListener::SetHostPkgName(pkgName);
    std::map<std::string, std::string> discParam;
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID,
        std::to_string(subscribeInfo.subscribeId)));
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_DISC_MEDIUM, std::to_string(subscribeInfo.medium)));

    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>(PARAM_KEY_FILTER_OPTIONS, extra));

    return discoveryMgr_->StartDiscovering(pkgName, discParam, filterOps);
}

int32_t DeviceManagerService::StartDeviceDiscovery(const std::string &pkgName, const uint16_t subscribeId,
                                                   const std::string &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller: %{public}s does not have permission to call StartDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("StartDeviceDiscovery begin for pkgName = %{public}s, filterOptions = %{public}s, subscribeId = %{public}d",
        pkgName.c_str(), filterOptions.c_str(), subscribeId);
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    SoftbusListener::SetHostPkgName(pkgName);
    std::map<std::string, std::string> discParam;
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID, std::to_string(subscribeId)));

    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>(PARAM_KEY_FILTER_OPTIONS, filterOptions));

    return discoveryMgr_->StartDiscovering(pkgName, discParam, filterOps);
}

int32_t DeviceManagerService::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    if (!PermissionManager::GetInstance().CheckPermission() &&
        !PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller: %{public}s does not have permission to call StopDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::StopDeviceDiscovery begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return discoveryMgr_->StopDiscovering(pkgName, subscribeId);
}

int32_t DeviceManagerService::PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call PublishDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::PublishDeviceDiscovery begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::map<std::string, std::string> advertiseParam;
    advertiseParam.insert(std::pair<std::string, std::string>(PARAM_KEY_PUBLISH_ID,
        std::to_string(publishInfo.publishId)));

    return advertiseMgr_->StartAdvertising(pkgName, advertiseParam);
}

int32_t DeviceManagerService::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnPublishDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return advertiseMgr_->StopAdvertising(pkgName, publishId);
}

int32_t DeviceManagerService::AuthenticateDevice(const std::string &pkgName, int32_t authType,
                                                 const std::string &deviceId, const std::string &extra)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call AuthenticateDevice.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerService::AuthenticateDevice error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("AuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }

    PeerTargetId targetId;
    ConnectionAddrType addrType;
    int32_t ret = SoftbusListener::GetTargetInfoFromCache(deviceId, targetId, addrType);
    if (ret != DM_OK) {
        LOGE("AuthenticateDevice failed, cannot get target info from cached discovered device map.");
        return ERR_DM_BIND_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>(PARAM_KEY_AUTH_TYPE, std::to_string(authType)));
    bindParam.insert(std::pair<std::string, std::string>(PARAM_KEY_BIND_EXTRA_DATA, extra));
    bindParam.insert(std::pair<std::string, std::string>(PARAM_KEY_CONN_ADDR_TYPE, std::to_string(addrType)));
    return dmServiceImpl_->BindTarget(pkgName, targetId, bindParam);
}

int32_t DeviceManagerService::UnAuthenticateDevice(const std::string &pkgName, const std::string &networkId)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnAuthenticateDevice.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::UnAuthenticateDevice begin for pkgName = %{public}s, networkId = %{public}s",
        pkgName.c_str(), GetAnonyString(networkId).c_str());
    if (pkgName.empty() || networkId.empty()) {
        LOGE("DeviceManagerService::UnAuthenticateDevice error: Invalid parameter, pkgName: %{public}s",
            pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnAuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnAuthenticateDevice(pkgName, networkId);
}

int32_t DeviceManagerService::BindDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
    const std::string &bindParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call BindDevice.");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerService::BindDevice error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("BindDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }

    PeerTargetId targetId;
    ConnectionAddrType addrType;
    int32_t ret = SoftbusListener::GetTargetInfoFromCache(deviceId, targetId, addrType);
    if (ret != DM_OK) {
        LOGE("BindDevice failed, cannot get target info from cached discovered device map.");
        return ERR_DM_BIND_INPUT_PARA_INVALID;
    }
    std::map<std::string, std::string> bindParamMap;
    bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_AUTH_TYPE, std::to_string(authType)));
    bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_BIND_EXTRA_DATA, bindParam));
    bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_CONN_ADDR_TYPE, std::to_string(addrType)));
    return dmServiceImpl_->BindTarget(pkgName, targetId, bindParamMap);
}

int32_t DeviceManagerService::UnBindDevice(const std::string &pkgName, const std::string &deviceId)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call UnBindDevice.");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::UnBindDevice begin for pkgName = %{public}s, deviceId = %{public}s",
        pkgName.c_str(), GetAnonyString(deviceId).c_str());
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerService::UnBindDevice error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnBindDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnBindDevice(pkgName, deviceId);
}

int32_t DeviceManagerService::SetUserOperation(std::string &pkgName, int32_t action, const std::string &params)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call SetUserOperation.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || params.empty()) {
        LOGE("DeviceManagerService::SetUserOperation error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (IsDMServiceAdapterLoad()) {
        dmServiceImplExt_->ReplyUiAction(pkgName, action, params);
    }
    if (!IsDMServiceImplReady()) {
        LOGE("SetUserOperation failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->SetUserOperation(pkgName, action, params);
}

void DeviceManagerService::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo)
{
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceStatusChange(devState, devInfo);
    }
    if (IsDMServiceAdapterLoad()) {
        dmServiceImplExt_->HandleDeviceStatusChange(devState, devInfo);
    }
}

int DeviceManagerService::OnSessionOpened(int sessionId, int result)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnSessionOpened failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->OnSessionOpened(sessionId, result);
}

void DeviceManagerService::OnSessionClosed(int sessionId)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnSessionClosed failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnSessionClosed(sessionId);
}

void DeviceManagerService::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnBytesReceived failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnBytesReceived(sessionId, data, dataLen);
}

int DeviceManagerService::OnPinHolderSessionOpened(int sessionId, int result)
{
    LOGI("DeviceManagerService::OnPinHolderSessionOpened");
    return PinHolderSession::OnSessionOpened(sessionId, result);
}

void DeviceManagerService::OnPinHolderSessionClosed(int sessionId)
{
    LOGI("DeviceManagerService::OnPinHolderSessionClosed");
    PinHolderSession::OnSessionClosed(sessionId);
}

void DeviceManagerService::OnPinHolderBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    LOGI("DeviceManagerService::OnPinHolderBytesReceived");
    PinHolderSession::OnBytesReceived(sessionId, data, dataLen);
}

int32_t DeviceManagerService::RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call RequestCredential.");
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("RequestCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RequestCredential(reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerService::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call ImportCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("ImportCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->ImportCredential(pkgName, credentialInfo);
}

int32_t DeviceManagerService::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call DeleteCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("DeleteCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->DeleteCredential(pkgName, deleteInfo);
}

int32_t DeviceManagerService::MineRequestCredential(const std::string &pkgName, std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller does not have permission to call RequestCredential.");
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("RequestCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->MineRequestCredential(pkgName, returnJsonStr);
}

int32_t DeviceManagerService::CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call CheckCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("CheckCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->CheckCredential(pkgName, reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerService::ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call ImportCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("ImportCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->ImportCredential(pkgName, reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerService::DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
    std::string &returnJsonStr)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call DeleteCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("DeleteCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerService::RegisterCredentialCallback(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call RegisterCredentialCallback.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("RegisterCredentialCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RegisterCredentialCallback(pkgName);
}

int32_t DeviceManagerService::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnRegisterCredentialCallback.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnRegisterCredentialCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnRegisterCredentialCallback(pkgName);
}

int32_t DeviceManagerService::RegisterUiStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerService::RegisterUiStateCallback error: Invalid parameter, pkgName: %{public}s",
            pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("RegisterUiStateCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RegisterUiStateCallback(pkgName);
}

int32_t DeviceManagerService::UnRegisterUiStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerService::UnRegisterUiStateCallback error: Invalid parameter, pkgName: %{public}s",
            pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnRegisterUiStateCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnRegisterUiStateCallback(pkgName);
}

bool DeviceManagerService::IsDMServiceImplReady()
{
    LOGI("DeviceManagerService::IsDMServiceImplReady");
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
    if (isImplsoLoaded_ && (dmServiceImpl_ != nullptr)) {
        return true;
    }
    char path[PATH_MAX + 1] = {0x00};
    std::string soName = std::string(DM_LIB_LOAD_PATH) + std::string(LIB_IMPL_NAME);
    if ((soName.length() == 0) || (soName.length() > PATH_MAX) || (realpath(soName.c_str(), path) == nullptr)) {
        LOGE("File %{public}s canonicalization failed.", soName.c_str());
        return false;
    }
    void *so_handle = dlopen(path, RTLD_NOW | RTLD_NODELETE);
    if (so_handle == nullptr) {
        LOGE("load libdevicemanagerserviceimpl so %{public}s failed, errMsg: %{public}s.", soName.c_str(), dlerror());
        return false;
    }
    dlerror();
    auto func = (CreateDMServiceFuncPtr)dlsym(so_handle, "CreateDMServiceObject");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(so_handle);
        LOGE("Create object function is not exist.");
        return false;
    }

    dmServiceImpl_ = std::shared_ptr<IDeviceManagerServiceImpl>(func());
    if (dmServiceImpl_->Initialize(listener_) != DM_OK) {
        dlclose(so_handle);
        dmServiceImpl_ = nullptr;
        isImplsoLoaded_ = false;
        return false;
    }
    isImplsoLoaded_ = true;
    return true;
}

int32_t DeviceManagerService::DmHiDumper(const std::vector<std::string>& args, std::string &result)
{
    LOGI("HiDump GetTrustedDeviceList");
    std::vector<HidumperFlag> dumpflag;
    HiDumpHelper::GetInstance().GetArgsType(args, dumpflag);

    for (unsigned int i = 0; i < dumpflag.size(); i++) {
        if (dumpflag[i] == HidumperFlag::HIDUMPER_GET_TRUSTED_LIST) {
            std::vector<DmDeviceInfo> deviceList;

            int32_t ret = softbusListener_->GetTrustedDeviceList(deviceList);
            if (ret != DM_OK) {
                result.append("HiDumpHelper GetTrustedDeviceList failed");
                LOGE("HiDumpHelper GetTrustedDeviceList failed");
                return ERR_DM_FAILED;
            }

            for (unsigned int j = 0; j < deviceList.size(); j++) {
                HiDumpHelper::GetInstance().SetNodeInfo(deviceList[j]);
                LOGI("DeviceManagerService::DmHiDumper SetNodeInfo.");
            }
        }
    }
    HiDumpHelper::GetInstance().HiDump(args, result);
    return DM_OK;
}

int32_t DeviceManagerService::NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call NotifyEvent.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("NotifyEvent failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    if (eventId == DM_NOTIFY_EVENT_ON_PINHOLDER_EVENT) {
        LOGI("NotifyEvent on pin holder event start.");
        return pinHolder_->NotifyPinHolderEvent(pkgName, event);
    }
    return dmServiceImpl_->NotifyEvent(pkgName, eventId, event);
}

void DeviceManagerService::LoadHardwareFwkService()
{
    std::string extra;
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = GetTrustedDeviceList(DM_PKG_NAME, extra, deviceList);
    if (ret != DM_OK) {
        LOGE("LoadHardwareFwkService failed, get trusted devicelist failed.");
        return;
    }
    if (deviceList.size() > 0) {
        dmServiceImpl_->LoadHardwareFwkService();
    }
}

int32_t DeviceManagerService::GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
    std::string &uuid)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::GetEncryptedUuidByNetworkId for pkgName = %{public}s", pkgName.c_str());
    int32_t ret = softbusListener_->GetUuidByNetworkId(networkId.c_str(), uuid);
    if (ret != DM_OK) {
        LOGE("GetUuidByNetworkId failed, ret : %{public}d", ret);
        return ret;
    }

    std::string appId = Crypto::Sha256(AppManager::GetInstance().GetAppId());
    LOGI("appId = %{public}s, uuid = %{public}s.", GetAnonyString(appId).c_str(), GetAnonyString(uuid).c_str());
    uuid = Crypto::Sha256(appId + "_" + uuid);
    LOGI("encryptedUuid = %{public}s.", GetAnonyString(uuid).c_str());
    return DM_OK;
}

int32_t DeviceManagerService::GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid,
    const std::string &appId, std::string &encryptedUuid)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    encryptedUuid = Crypto::Sha256(appId + "_" + uuid);
    LOGI("encryptedUuid = %{public}s.", GetAnonyString(encryptedUuid).c_str());
    return DM_OK;
}

int32_t DeviceManagerService::CheckApiPermission(int32_t permissionLevel)
{
    LOGI("DeviceManagerService::CheckApiPermission permissionLevel: %{public}d", permissionLevel);
    int32_t ret = ERR_DM_NO_PERMISSION;
    switch (permissionLevel) {
        case NORMAL:
            if (PermissionManager::GetInstance().CheckNewPermission()) {
                LOGI("The caller have permission to call");
                ret = DM_OK;
            }
            break;
        case SYSTEM_BASIC:
            if (PermissionManager::GetInstance().CheckPermission()) {
                LOGI("The caller have permission to call");
                ret = DM_OK;
            }
            break;
        case SYSTEM_CORE:
            if (PermissionManager::GetInstance().CheckMonitorPermission()) {
                LOGI("The caller have permission to call");
                ret = DM_OK;
            }
            break;
        default:
            LOGE("DM have not this permissionLevel.");
            break;
    }
    return ret;
}

int32_t DeviceManagerService::GetNetworkTypeByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                        int32_t &networkType)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetNetworkTypeByNetworkId.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::GetNetworkTypeByNetworkId begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, netWorkId: %{public}s", pkgName.c_str(),
            GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    softbusListener_->GetNetworkTypeByNetworkId(netWorkId.c_str(), networkType);
    return DM_OK;
}

int32_t DeviceManagerService::ImportAuthCode(const std::string &pkgName, const std::string &authCode)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller: %{public}s does not have permission to call ImportAuthCode.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::ImportAuthCode begin.");
    if (authCode.empty() || pkgName.empty()) {
        LOGE("Invalid parameter, authCode: %{public}s.", GetAnonyString(authCode).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("ImportAuthCode failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->ImportAuthCode(pkgName, authCode);
}

int32_t DeviceManagerService::ExportAuthCode(std::string &authCode)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call ExportAuthCode.");
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, processName: %{public}s.", processName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("ExportAuthCode failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    LOGI("DeviceManagerService::ExportAuthCode begin.");
    return dmServiceImpl_->ExportAuthCode(authCode);
}

void DeviceManagerService::UnloadDMServiceImplSo()
{
    LOGI("DeviceManagerService::UnloadDMServiceImplSo start.");
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
    if (dmServiceImpl_ != nullptr) {
        dmServiceImpl_->Release();
    }
    char path[PATH_MAX + 1] = {0x00};
    std::string soPathName = std::string(DM_LIB_LOAD_PATH) + std::string(LIB_IMPL_NAME);
    if ((soPathName.length() == 0) || (soPathName.length() > PATH_MAX) ||
        (realpath(soPathName.c_str(), path) == nullptr)) {
        LOGE("File %{public}s canonicalization failed.", soPathName.c_str());
        return;
    }
    void *so_handle = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
    if (so_handle != nullptr) {
        LOGI("DeviceManagerService so_handle is not nullptr.");
        dlclose(so_handle);
    }
}

bool DeviceManagerService::IsDMServiceAdapterLoad()
{
    LOGI("DeviceManagerService::IsDMServiceAdapterLoad start.");
    std::lock_guard<std::mutex> lock(isAdapterLoadLock_);
    if (isAdapterSoLoaded_ && (dmServiceImplExt_ != nullptr)) {
        return true;
    }

    char path[PATH_MAX + 1] = {0x00};
    std::string soName = std::string(DM_LIB_LOAD_PATH) + std::string(LIB_DM_ADAPTER_NAME);
    if ((soName.length() == 0) || (soName.length() > PATH_MAX) || (realpath(soName.c_str(), path) == nullptr)) {
        LOGE("File %{public}s canonicalization failed.", soName.c_str());
        return false;
    }
    void *so_handle = dlopen(path, RTLD_NOW | RTLD_NODELETE);
    if (so_handle == nullptr) {
        LOGE("load dm service adapter so %{public}s failed.", soName.c_str());
        return false;
    }
    dlerror();
    auto func = (CreateDMServiceImplExtFuncPtr)dlsym(so_handle, "CreateDMServiceImplExtObject");
    if (dlerror() != nullptr || func == nullptr) {
        dlclose(so_handle);
        LOGE("Create object function is not exist.");
        return false;
    }

    dmServiceImplExt_ = std::shared_ptr<IDMServiceImplExt>(func());
    if (dmServiceImplExt_->Initialize(listener_) != DM_OK) {
        dlclose(so_handle);
        dmServiceImplExt_ = nullptr;
        isAdapterSoLoaded_ = false;
        LOGE("dm service adapter impl ext init failed.");
        return false;
    }
    isAdapterSoLoaded_ = true;
    LOGI("DeviceManagerService::IsDMServiceAdapterLoad sucess.");
    return true;
}

void DeviceManagerService::UnloadDMServiceAdapter()
{
    LOGI("DeviceManagerService::UnloadDMServiceAdapter start.");
    std::lock_guard<std::mutex> lock(isAdapterLoadLock_);
    if (dmServiceImplExt_ != nullptr) {
        dmServiceImplExt_->Release();
    }
    dmServiceImplExt_ = nullptr;

    char path[PATH_MAX + 1] = {0x00};
    std::string soPathName = std::string(DM_LIB_LOAD_PATH) + std::string(LIB_DM_ADAPTER_NAME);
    if ((soPathName.length() == 0) || (soPathName.length() > PATH_MAX) ||
        (realpath(soPathName.c_str(), path) == nullptr)) {
        LOGE("File %{public}s canonicalization failed.", soPathName.c_str());
        return;
    }
    void *so_handle = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
    if (so_handle != nullptr) {
        LOGI("dm service adapter so_handle is not nullptr.");
        dlclose(so_handle);
    }
}

int32_t DeviceManagerService::StartDiscovering(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::StartDiscovering for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (discoverParam.find(PARAM_KEY_META_TYPE) != discoverParam.end()) {
        LOGI("StartDiscovering input MetaType = %{public}s", (discoverParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    return discoveryMgr_->StartDiscovering(pkgName, discoverParam, filterOptions);
}

int32_t DeviceManagerService::StopDiscovering(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::StopDiscovering for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    uint16_t subscribeId = -1;
    if (discoverParam.find(PARAM_KEY_SUBSCRIBE_ID) != discoverParam.end()) {
        subscribeId = std::atoi((discoverParam.find(PARAM_KEY_SUBSCRIBE_ID)->second).c_str());
    }
    if (discoverParam.find(PARAM_KEY_META_TYPE) != discoverParam.end()) {
        LOGI("StopDiscovering input MetaType = %{public}s", (discoverParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    return discoveryMgr_->StopDiscovering(pkgName, subscribeId);
}

int32_t DeviceManagerService::EnableDiscoveryListener(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::EnableDiscoveryListener for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SoftbusListener::SetHostPkgName(pkgName);
    return discoveryMgr_->EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
}

int32_t DeviceManagerService::DisableDiscoveryListener(const std::string &pkgName,
    const std::map<std::string, std::string> &extraParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::DisableDiscoveryListener for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return discoveryMgr_->DisableDiscoveryListener(pkgName, extraParam);
}

int32_t DeviceManagerService::StartAdvertising(const std::string &pkgName,
    const std::map<std::string, std::string> &advertiseParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::StartAdvertising for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return advertiseMgr_->StartAdvertising(pkgName, advertiseParam);
}

int32_t DeviceManagerService::StopAdvertising(const std::string &pkgName,
    const std::map<std::string, std::string> &advertiseParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::StopAdvertising for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (advertiseParam.find(PARAM_KEY_META_TYPE) != advertiseParam.end()) {
        LOGI("StopAdvertising input MetaType=%{public}s", (advertiseParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    int32_t publishId = -1;
    if (advertiseParam.find(PARAM_KEY_PUBLISH_ID) != advertiseParam.end()) {
        publishId = std::atoi((advertiseParam.find(PARAM_KEY_PUBLISH_ID)->second).c_str());
    }
    return advertiseMgr_->StopAdvertising(pkgName, publishId);
}

int32_t DeviceManagerService::BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &bindParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::BindTarget for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("BindTarget failed, DMServiceImpl instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    if (bindParam.find(PARAM_KEY_META_TYPE) == bindParam.end()) {
        LOGI("BindTarget stardard begin.");
        if (targetId.wifiIp.empty() || targetId.wifiIp.length() > IP_STR_MAX_LEN) {
            return dmServiceImpl_->BindTarget(pkgName, targetId, bindParam);
        }
        ConnectionAddrType ipAddrType;
        std::map<std::string, std::string> &noConstBindParam =
            const_cast<std::map<std::string, std::string> &>(bindParam);
        if (SoftbusListener::GetIPAddrTypeFromCache(targetId.deviceId, targetId.wifiIp, ipAddrType) == DM_OK) {
            noConstBindParam.insert(std::pair<std::string, std::string>(PARAM_KEY_CONN_ADDR_TYPE,
                std::to_string(ipAddrType)));
        }
        const std::map<std::string, std::string> &constBindParam =
            const_cast<const std::map<std::string, std::string> &>(noConstBindParam);
        return dmServiceImpl_->BindTarget(pkgName, targetId, constBindParam);
    }
    if (!IsDMServiceAdapterLoad()) {
        LOGE("BindTarget failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    LOGI("BindTarget unstardard begin.");
    return dmServiceImplExt_->BindTargetExt(pkgName, targetId, bindParam);
}

int32_t DeviceManagerService::UnbindTarget(const std::string &pkgName, const PeerTargetId &targetId,
    const std::map<std::string, std::string> &unbindParam)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService::UnbindTarget for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceAdapterLoad()) {
        LOGE("UnbindTarget failed, instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    if (unbindParam.find(PARAM_KEY_META_TYPE) == unbindParam.end()) {
        LOGE("input unbind parameter not contains META_TYPE, dm service adapter not supported.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return dmServiceImplExt_->UnbindTargetExt(pkgName, targetId, unbindParam);
}

int32_t DeviceManagerService::RegisterPinHolderCallback(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call ImportAuthCode.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::RegisterPinHolderCallback begin.");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s.", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return pinHolder_->RegisterPinHolderCallback(pkgName);
}

int32_t DeviceManagerService::CreatePinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call CreatePinHolder.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::CreatePinHolder begin.");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s.", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return pinHolder_->CreatePinHolder(pkgName, targetId, pinType, payload);
}

int32_t DeviceManagerService::DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId,
    DmPinType pinType, const std::string &payload)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call DestroyPinHolder.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed, pkgname: %{public}s.", pkgName.c_str());
        return ERR_DM_FAILED;
    }
    if (!PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(processName)) {
        LOGE("The caller: %{public}s is not in white list.", processName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::DestroyPinHolder begin.");
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s.", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return pinHolder_->DestroyPinHolder(pkgName, targetId, pinType, payload);
}

void DeviceManagerService::OnUnbindSessionOpened(int32_t socket, PeerSocketInfo info)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnUnbindSessionOpened failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnUnbindSessionOpened(socket, info);
}

void DeviceManagerService::OnUnbindSessionCloseed(int32_t socket)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnUnbindSessionCloseed failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnUnbindSessionCloseed(socket);
}

void DeviceManagerService::OnUnbindBytesReceived(int32_t socket, const void *data, uint32_t dataLen)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnUnbindBytesReceived failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnUnbindBytesReceived(socket, data, dataLen);
}

int32_t DeviceManagerService::DpAclAdd(const std::string &udid)
{
    if (!PermissionManager::GetInstance().CheckNewPermission()) {
        LOGE("The caller does not have permission to call DpAclAdd.");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService DpAclAdd start.");
    if (!IsDMServiceImplReady()) {
        LOGE("DpAclAdd failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    dmServiceImpl_->DpAclAdd(udid);
    LOGI("DeviceManagerService::DpAclAdd completed");
    return DM_OK;
}

int32_t DeviceManagerService::GetDeviceSecurityLevel(const std::string &pkgName, const std::string &networkId,
                                                     int32_t &securityLevel)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetDeviceSecurityLevel.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || networkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, networkId: %{public}s", pkgName.c_str(),
            GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService::GetDeviceSecurityLevel begin for pkgName = %{public}s", pkgName.c_str());
    int32_t ret = softbusListener_->GetDeviceSecurityLevel(networkId.c_str(), securityLevel);
    if (ret != DM_OK) {
        LOGE("GetDeviceSecurityLevel failed, ret = %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerService::IsSameAccount(const std::string &udid)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call IsSameAccount.", GetAnonyString(udid).c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("DeviceManagerService IsSameAccount start for udid = %{public}s", GetAnonyString(udid).c_str());
    if (udid.empty()) {
        LOGE("DeviceManagerService::IsSameAccount error: udid: %{public}s", GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("IsSameAccount failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->IsSameAccount(udid);
}

int32_t DeviceManagerService::CheckRelatedDevice(const std::string &udid, const std::string &bundleName)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGE("The caller: %{public}s does not have permission to call CheckRelatedDevice.", bundleName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (udid.empty() || bundleName.empty()) {
        LOGE("DeviceManagerService::CheckRelatedDevice error: udid: %{public}s bundleName: %{public}s",
            GetAnonyString(udid).c_str(), bundleName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("DeviceManagerService CheckRelatedDevice start for udid: %{public}s bundleName: %{public}s",
        GetAnonyString(udid).c_str(), bundleName.c_str());

    if (!IsDMServiceImplReady()) {
        LOGE("CheckRelatedDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->CheckRelatedDevice(udid, bundleName);
}
} // namespace DistributedHardware
} // namespace OHOS