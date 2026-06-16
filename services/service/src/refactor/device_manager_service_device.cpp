/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "dm_crypto.h"
#include "dm_device_info.h"
#include "dm_hidumper.h"
#include "dm_softbus_cache.h"
#include "parameter.h"
#include "permission_manager.h"
#include "app_manager.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_constraints_manager.h"
#include "deviceprofile_connector.h"
#include "dm_comm_tool.h"
#include "ipc_skeleton.h"
#include "multiple_user_connector.h"
#include "system_ability_definition.h"
#endif

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr const char *ALL_PKGNAME = "";
    constexpr const int32_t NORMAL = 0;
    constexpr const int32_t SYSTEM_BASIC = 1;
    constexpr const int32_t SYSTEM_CORE = 2;
}

DM_EXPORT void DeviceManagerService::RegisterCallerAppId(const std::string &pkgName, const int32_t userId)
{
    AppManager::GetInstance().RegisterCallerAppId(pkgName, userId);
}

DM_EXPORT void DeviceManagerService::UnRegisterCallerAppId(const std::string &pkgName, const int32_t userId)
{
    AppManager::GetInstance().UnRegisterCallerAppId(pkgName, userId);
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                   std::vector<DmDeviceInfo> &deviceList)
{
    CHECK_EMPTY_RETURN(pkgName, ERR_DM_INPUT_PARA_INVALID);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(
        MultipleUserConnector::GetForgroundUserId(), DM_ACCOUNT_CONSTRAINT)) {
        LOGI("contraint enable is true");
        return DM_OK;
    }
#endif
    bool isOnlyShowNetworkId = !(PermissionManager::GetInstance().CheckAccessServicePermission() ||
        PermissionManager::GetInstance().CheckDataSyncPermission());
    std::vector<DmDeviceInfo> onlineDeviceList;
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    if (softbusListener_->GetTrustedDeviceList(onlineDeviceList) != DM_OK) {
        return ERR_DM_FAILED;
    }
    if (isOnlyShowNetworkId && !onlineDeviceList.empty()) {
        for (auto item : onlineDeviceList) {
            DmDeviceInfo tempInfo;
            if (memcpy_s(tempInfo.networkId, DM_MAX_DEVICE_ID_LEN, item.networkId, sizeof(item.networkId)) != 0) {
                LOGE("get networkId: %{public}s failed", GetAnonyString(item.networkId).c_str());
                return ERR_DM_SECURITY_FUNC_FAILED;
            }
            deviceList.push_back(tempInfo);
        }
        return DM_OK;
    }
    if (!onlineDeviceList.empty() && IsDMServiceImplReady()) {
        std::unordered_map<std::string, DmAuthForm> udidMap;
        if (PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName) ||
            PermissionManager::GetInstance().CheckProcessValidOnGetTrustedDeviceList()) {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(std::string(ALL_PKGNAME));
        } else {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(pkgName);
        }
        for (auto item : onlineDeviceList) {
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
            ConvertUdidHashToAnoyDeviceId(item);
#endif
            std::string udid = "";
            SoftbusListener::GetUdidByNetworkId(item.networkId, udid);
            if (udidMap.find(udid) != udidMap.end()) {
                item.authForm = udidMap[udid];
                deviceList.push_back(item);
            }
        }
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                      std::vector<DmDeviceInfo> &deviceList)
{
    CHECK_EMPTY_RETURN(pkgName, ERR_DM_INPUT_PARA_INVALID);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(
        MultipleUserConnector::GetForgroundUserId(), DM_ACCOUNT_CONSTRAINT)) {
        LOGI("contraint enable is true");
        return DM_OK;
    }
#endif
    if (!PermissionManager::GetInstance().CheckDataSyncPermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetAllTrustedDeviceList.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetAllTrustedDeviceList(pkgName, extra, deviceList);
    if (ret != DM_OK) {
        LOGE("GetAllTrustedDeviceList failed");
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerService::ShiftLNNGear(const std::string &pkgName, const std::string &callerId, bool isRefresh,
                                           bool isWakeUp)
{
    LOGD("Begin for pkgName = %{public}s, callerId = %{public}s, isRefresh ="
        "%{public}d, isWakeUp = %{public}d", pkgName.c_str(), GetAnonyString(callerId).c_str(), isRefresh, isWakeUp);
    if (!PermissionManager::GetInstance().CheckDataSyncPermission()) {
        LOGE("The caller does not have permission to call ShiftLNNGear, pkgName = %{public}s", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || callerId.empty()) {
        LOGE("Invalid parameter, parameter is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (isRefresh) {
        CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
        int32_t ret = softbusListener_->ShiftLNNGear(isWakeUp, callerId);
        if (ret != DM_OK) {
            LOGE("failed ret: %{public}d", ret);
            return ret;
        }
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info)
{
    LOGI("Begin networkId %{public}s.", GetAnonyString(networkId).c_str());
    CHECK_EMPTY_RETURN(networkId, ERR_DM_INPUT_PARA_INVALID);
    if (CheckConstraintEnabledByNetworkId(networkId)) {
        LOGI("contraint enable is true");
        return DM_OK;
    }
    if (!PermissionManager::GetInstance().CheckAccessServicePermission() &&
        !PermissionManager::GetInstance().CheckDataSyncPermission()) {
        LOGE("The caller does not have permission to call GetDeviceInfo.");
        return ERR_DM_NO_PERMISSION;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    std::string peerDeviceId = "";
    SoftbusListener::GetUdidByNetworkId(networkId.c_str(), peerDeviceId);
    int32_t ret = DM_OK;
    if (!IsDMServiceImplReady()) {
        LOGE("instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    if (localUdid == peerDeviceId) {
        ret = softbusListener_->GetDeviceInfo(networkId, info);
        if (ret != DM_OK) {
            LOGE("Get DeviceInfo By NetworkId failed, ret : %{public}d", ret);
        }
        return ret;
    }
    if (!AppManager::GetInstance().IsSystemSA()) {
        int32_t permissionRet = dmServiceImpl_->CheckDeviceInfoPermission(localUdid, peerDeviceId);
        if (permissionRet != DM_OK) {
            std::string processName = "";
            if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
                LOGE("Get caller process name failed.");
                return ret;
            }
            if (!PermissionManager::GetInstance().CheckProcessNameValidOnGetDeviceInfo(processName)) {
                LOGE("The caller: %{public}s is not in white list.", processName.c_str());
                return ret;
            }
        }
    }
    return softbusListener_->GetDeviceInfo(networkId, info);
}

int32_t DeviceManagerService::GetLocalDeviceInfo(DmDeviceInfo &info)
{
    LOGD("Begin.");
    bool isOnlyShowNetworkId = false;
    if (!PermissionManager::GetInstance().CheckDataSyncPermission()) {
        LOGE("The caller does not have permission to call GetLocalDeviceInfo.");
        isOnlyShowNetworkId = true;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetLocalDeviceInfo(info);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceInfo failed");
        return ret;
    }
    if (isOnlyShowNetworkId) {
        DmDeviceInfo tempInfo;
        if (memcpy_s(tempInfo.networkId, DM_MAX_DEVICE_ID_LEN, info.networkId, sizeof(info.networkId)) != 0) {
            LOGE("get networkId: %{public}s failed", GetAnonyString(info.networkId).c_str());
            return ERR_DM_FAILED;
        }
        info = tempInfo;
        return DM_OK;
    }
    if (localDeviceId_.empty()) {
        char localDeviceId[DEVICE_UUID_LENGTH] = {0};
        char udidHash[DEVICE_UUID_LENGTH] = {0};
        GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
        if (Crypto::GetUdidHash(localDeviceId, reinterpret_cast<uint8_t *>(udidHash)) == DM_OK) {
            localDeviceId_ = udidHash;
        }
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string udidHashTemp = "";
    if (ConvertUdidHashToAnoyDeviceId(localDeviceId_, udidHashTemp) == DM_OK) {
        if (memset_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN) != DM_OK) {
            LOGE("memset_s failed.");
            return ERR_DM_FAILED;
        }
        if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, udidHashTemp.c_str(), udidHashTemp.length()) != 0) {
            LOGE("get deviceId: %{public}s failed", GetAnonyString(udidHashTemp).c_str());
            return ERR_DM_FAILED;
        }
        return DM_OK;
    }
#endif
    if (memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, localDeviceId_.c_str(), localDeviceId_.length()) != 0) {
        LOGE("get deviceId: %{public}s failed", GetAnonyString(localDeviceId_).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetLocalDeviceNameOld(std::string &deviceName)
{
    LOGD("Begin.");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE)) && !defined(DEVICE_MANAGER_COMMON_FLAG)
    if (!PermissionManager::GetInstance().CheckDataSyncPermission() && !IsCallerInWhiteList()) {
        LOGE("The caller does not have permission to call GetLocalDeviceName.");
        return ERR_DM_NO_PERMISSION;
    }
#else
    if (!PermissionManager::GetInstance().CheckDataSyncPermission()) {
        LOGE("The caller does not have permission to call GetLocalDeviceName.");
        return ERR_DM_NO_PERMISSION;
    }
#endif
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    DmDeviceInfo info;
    int32_t ret = softbusListener_->GetLocalDeviceInfo(info);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceInfo failed");
        return ret;
    }
    deviceName = info.deviceName;
    return DM_OK;
}

int32_t DeviceManagerService::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &udid)
{
    CHECK_EMPTY_RETURN(netWorkId, ERR_DM_INPUT_PARA_INVALID);
    CHECK_EMPTY_RETURN(pkgName, ERR_DM_INPUT_PARA_INVALID);
    if (CheckConstraintEnabledByNetworkId(netWorkId)) {
        LOGI("contraint enable is true");
        return DM_OK;
    }
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetUdidByNetworkId.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    return SoftbusListener::GetUdidByNetworkId(netWorkId.c_str(), udid);
}

int32_t DeviceManagerService::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &uuid)
{
    CHECK_EMPTY_RETURN(netWorkId, ERR_DM_INPUT_PARA_INVALID);
    CHECK_EMPTY_RETURN(pkgName, ERR_DM_INPUT_PARA_INVALID);
    if (CheckConstraintEnabledByNetworkId(netWorkId)) {
        LOGI("contraint enable is true");
        return DM_OK;
    }
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetUuidByNetworkId.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    return SoftbusListener::GetUuidByNetworkId(netWorkId.c_str(), uuid);
}

int32_t DeviceManagerService::PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo)
{
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call PublishDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Begin for pkgName = %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::map<std::string, std::string> advertiseParam;
    advertiseParam.insert(std::pair<std::string, std::string>(PARAM_KEY_PUBLISH_ID,
        std::to_string(publishInfo.publishId)));
    CHECK_NULL_RETURN(advertiseMgr_, ERR_DM_POINT_NULL);
    return advertiseMgr_->StartAdvertising(pkgName, advertiseParam);
}

int32_t DeviceManagerService::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnPublishDeviceDiscovery.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(advertiseMgr_, ERR_DM_POINT_NULL);
    return advertiseMgr_->StopAdvertising(pkgName, publishId);
}

int32_t DeviceManagerService::GetDeviceSecurityLevel(const std::string &pkgName, const std::string &networkId,
                                                     int32_t &securityLevel)
{
    LOGI("Begin pkgName: %{public}s, networkId: %{public}s",
        pkgName.c_str(), GetAnonyString(networkId).c_str());
    CHECK_EMPTY_RETURN(pkgName, ERR_DM_INPUT_PARA_INVALID);
    CHECK_EMPTY_RETURN(networkId, ERR_DM_INPUT_PARA_INVALID);
    if (CheckConstraintEnabledByNetworkId(networkId)) {
        LOGI("contraint enable is true");
        return DM_OK;
    }
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetDeviceSecurityLevel.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || networkId.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, networkId: %{public}s", pkgName.c_str(),
            GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetDeviceSecurityLevel(networkId.c_str(), securityLevel);
    if (ret != DM_OK) {
        LOGE("ret = %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerService::IsSameAccount(const std::string &networkId)
{
    LOGI("NetworkId %{public}s.", GetAnonyString(networkId).c_str());
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        return ERR_DM_NO_PERMISSION;
    }
    std::string udid = "";
    if (SoftbusListener::GetUdidByNetworkId(networkId.c_str(), udid) != DM_OK) {
        LOGE("udid: %{public}s", GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->IsSameAccount(udid);
}

int32_t DeviceManagerService::GetNetworkTypeByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                        int32_t &networkType)
{
    CHECK_EMPTY_RETURN(pkgName, ERR_DM_INPUT_PARA_INVALID);
    CHECK_EMPTY_RETURN(netWorkId, ERR_DM_INPUT_PARA_INVALID);
    if (CheckConstraintEnabledByNetworkId(netWorkId)) {
        LOGI("contraint enable is true");
        return DM_OK;
    }
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetNetworkTypeByNetworkId.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    return softbusListener_->GetNetworkTypeByNetworkId(netWorkId.c_str(), networkType);
}

int32_t DeviceManagerService::GetNetworkIdByUdid(const std::string &pkgName, const std::string &udid,
                                                 std::string &networkId)
{
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetNetworkIdByUdid.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty() || udid.empty()) {
        LOGE("Invalid parameter, pkgName: %{public}s, udid: %{public}s", pkgName.c_str(), GetAnonyString(udid).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    return SoftbusListener::GetNetworkIdByUdid(udid, networkId);
}

int32_t DeviceManagerService::GetDeviceScreenStatus(const std::string &pkgName, const std::string &networkId,
    int32_t &screenStatus)
{
    LOGI("Begin pkgName: %{public}s, networkId: %{public}s", pkgName.c_str(), GetAnonyString(networkId).c_str());
    CHECK_EMPTY_RETURN(pkgName, ERR_DM_INPUT_PARA_INVALID);
    CHECK_EMPTY_RETURN(networkId, ERR_DM_INPUT_PARA_INVALID);
    if (CheckConstraintEnabledByNetworkId(networkId)) {
        LOGI("contraint enable is true");
        return DM_OK;
    }
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call GetDeviceScreenStatus.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetDeviceScreenStatus(networkId.c_str(), screenStatus);
    if (ret != DM_OK) {
        LOGE("ret = %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t DeviceManagerService::GetAnonyLocalUdid(const std::string &pkgName, std::string &anonyUdid)
{
    (void) pkgName;
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission to call GetAnonyLocalUdid.");
        return ERR_DM_NO_PERMISSION;
    }
    std::string udid = DmRadarHelper::GetInstance().GetAnonyLocalUdid();
    if (udid.empty()) {
        LOGE("Anony local udid is empty.");
        return ERR_DM_FAILED;
    }
    anonyUdid = udid;
    return DM_OK;
}

int32_t DeviceManagerService::GetDeviceNetworkIdList(const std::string &pkgName,
    const NetworkIdQueryFilter &queryFilter, std::vector<std::string> &networkIds)
{
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    LOGI("Start for pkgName = %{public}s", pkgName.c_str());
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("GetDeviceProfileInfoList failed, adapter instance not init or init failed.");
        return ERR_DM_UNSUPPORTED_METHOD;
    }
    std::vector<DmDeviceProfileInfo> dmDeviceProfileInfos;
    int32_t ret = dmServiceImplExtResident_->GetDeviceProfileInfosFromLocalCache(queryFilter, dmDeviceProfileInfos);
    if (ret != DM_OK) {
        LOGW("GetDeviceProfileInfosFromLocalCache failed, ret = %{public}d.", ret);
        return ret;
    }
    for (const auto &item : dmDeviceProfileInfos) {
        std::string networkId = "";
        SoftbusCache::GetInstance().GetNetworkIdFromCache(item.deviceId, networkId);
        if (!networkId.empty()) {
            networkIds.emplace_back(networkId);
        }
    }
    if (networkIds.empty()) {
        LOGW("networkIds is empty");
        return ERR_DM_FIND_NETWORKID_LIST_EMPTY;
    }
    return DM_OK;
}

int32_t DeviceManagerService::StartDiscovering(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckDataSyncPermission() &&
        !PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (discoverParam.find(PARAM_KEY_META_TYPE) != discoverParam.end()) {
        LOGI("input MetaType = %{public}s", (discoverParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->StartDiscovering(pkgName, discoverParam, filterOptions);
}

int32_t DeviceManagerService::StopDiscovering(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam)
{
    if (!PermissionManager::GetInstance().CheckDataSyncPermission() &&
        !PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    uint16_t subscribeId = -1;
    if (discoverParam.find(PARAM_KEY_SUBSCRIBE_ID) != discoverParam.end()) {
        subscribeId = std::atoi((discoverParam.find(PARAM_KEY_SUBSCRIBE_ID)->second).c_str());
    }
    if (discoverParam.find(PARAM_KEY_META_TYPE) != discoverParam.end()) {
        LOGI("input MetaType = %{public}s", (discoverParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->StopDiscovering(pkgName, subscribeId);
}

int32_t DeviceManagerService::EnableDiscoveryListener(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    if (!PermissionManager::GetInstance().CheckDataSyncPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SoftbusListener::SetHostPkgName(pkgName);
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
}

int32_t DeviceManagerService::DisableDiscoveryListener(const std::string &pkgName,
    const std::map<std::string, std::string> &extraParam)
{
    if (!PermissionManager::GetInstance().CheckDataSyncPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(discoveryMgr_, ERR_DM_POINT_NULL);
    return discoveryMgr_->DisableDiscoveryListener(pkgName, extraParam);
}

int32_t DeviceManagerService::StartAdvertising(const std::string &pkgName,
    const std::map<std::string, std::string> &advertiseParam)
{
    if (!PermissionManager::GetInstance().CheckDataSyncPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(advertiseMgr_, ERR_DM_POINT_NULL);
    return advertiseMgr_->StartAdvertising(pkgName, advertiseParam);
}

int32_t DeviceManagerService::StopAdvertising(const std::string &pkgName,
    const std::map<std::string, std::string> &advertiseParam)
{
    if (!PermissionManager::GetInstance().CheckDataSyncPermission()) {
        LOGE("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (advertiseParam.find(PARAM_KEY_META_TYPE) != advertiseParam.end()) {
        LOGI("input MetaType=%{public}s", (advertiseParam.find(PARAM_KEY_META_TYPE)->second).c_str());
    }
    int32_t publishId = -1;
    if (advertiseParam.find(PARAM_KEY_PUBLISH_ID) != advertiseParam.end()) {
        publishId = std::atoi((advertiseParam.find(PARAM_KEY_PUBLISH_ID)->second).c_str());
    }
    CHECK_NULL_RETURN(advertiseMgr_, ERR_DM_POINT_NULL);
    return advertiseMgr_->StopAdvertising(pkgName, publishId);
}

int32_t DeviceManagerService::DmHiDumper(const std::vector<std::string>& args, std::string &result)
{
    LOGI("HiDump GetTrustedDeviceList");
    std::vector<HidumperFlag> dumpflag;
    HiDumpHelper::GetInstance().GetArgsType(args, dumpflag);

    for (unsigned int i = 0; i < dumpflag.size(); i++) {
        if (dumpflag[i] == HidumperFlag::HIDUMPER_GET_TRUSTED_LIST) {
            std::vector<DmDeviceInfo> deviceList;
            CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
            int32_t ret = softbusListener_->GetTrustedDeviceList(deviceList);
            if (ret != DM_OK) {
                result.append("HiDumpHelper GetTrustedDeviceList failed");
                LOGE("HiDumpHelper GetTrustedDeviceList failed");
                return ret;
            }

            for (unsigned int j = 0; j < deviceList.size(); j++) {
                HiDumpHelper::GetInstance().SetNodeInfo(deviceList[j]);
                LOGI("SetNodeInfo.");
            }
        }
    }
    HiDumpHelper::GetInstance().HiDump(args, result);
    return DM_OK;
}

void DeviceManagerService::ClearDiscoveryCache(const ProcessInfo &processInfo)
{
    LOGI("PkgName: %{public}s, userId: %{public}d", processInfo.pkgName.c_str(), processInfo.userId);
    CHECK_NULL_VOID(discoveryMgr_);
    discoveryMgr_->ClearDiscoveryCache(processInfo);
}

void DeviceManagerService::ClearPublishIdCache(const ProcessInfo &processInfo)
{
    CHECK_NULL_VOID(advertiseMgr_);
    advertiseMgr_->ClearPublishIdCache(processInfo);
}

void DeviceManagerService::HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo, const bool isOnline)
{
    LOGI("start.");
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceStatusChange(devState, devInfo, isOnline);
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (IsDMServiceAdapterResidentLoad()) {
        int32_t ret = dmServiceImplExtResident_->GetServiceNodeKeyInfo(DM_PKG_NAME, devInfo.networkId);
        if (ret == DM_OK) {
            std::string localUdid = GetLocalDeviceUdid();
            std::string peerNetworkid = devInfo.networkId;
            ffrt::submit([dmServiceImplExtResident = dmServiceImplExtResident_,
                peerNetworkid = peerNetworkid, localUdid = localUdid]() {
                    dmServiceImplExtResident->SyncServiceInfoOnline(localUdid, peerNetworkid);
            },
                ffrt::task_attr().name(SYNC_SERVICE_INFO_ONLINE_TASK));
        }
        std::string peerUdid(devInfo.deviceId);
        int32_t state = static_cast<int32_t>(devState);
        dmServiceImplExtResident_->HandleServiceStatusChange(devState, peerUdid);
    }
#endif
}

void DeviceManagerService::HandleDeviceScreenStatusChange(DmDeviceInfo &deviceInfo)
{
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceScreenStatusChange(deviceInfo);
    }
}

int32_t DeviceManagerService::RegDevStateCallbackToService(const std::string &pkgName)
{
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    CHECK_NULL_RETURN(listener_, ERR_DM_POINT_NULL);
    std::vector<DmDeviceInfo> deviceList;
    GetTrustedDeviceList(pkgName, deviceList);
    if (deviceList.size() == 0) {
        return DM_OK;
    }
    int32_t userId = -1;
    MultipleUserConnector::GetCallerUserId(userId);
    ProcessInfo processInfo;
    processInfo.pkgName = pkgName;
    processInfo.userId = userId;
    processInfo.tokenId = IPCSkeleton::GetCallingTokenID();
    listener_->OnDevStateCallbackAdd(processInfo, deviceList);
    if (PermissionManager::GetInstance().CheckOnReadyRetrospectiveNotificationBlackList()) {
        return DM_OK;
    }
    std::vector<DmDeviceInfo> readyDeviceList;
    CHECK_NULL_RETURN(dmServiceImpl_, ERR_DM_POINT_NULL);
    dmServiceImpl_->GetNotifyEventInfos(readyDeviceList);
    if (readyDeviceList.size() == 0) {
        return DM_OK;
    }
    listener_->OnDevDbReadyCallbackAdd(processInfo, readyDeviceList);
#else
    (void)pkgName;
#endif
    return DM_OK;
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("Begin for pkgName = %{public}s.", pkgName.c_str());
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(
        MultipleUserConnector::GetForgroundUserId(), DM_ACCOUNT_CONSTRAINT)) {
        LOGI("contraint enable is true");
        return DM_OK;
    }
#endif
    if (pkgName.empty() || pkgName == std::string(DM_PKG_NAME)) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::vector<DmDeviceInfo> onlineDeviceList;
    CHECK_NULL_RETURN(softbusListener_, ERR_DM_POINT_NULL);
    int32_t ret = softbusListener_->GetTrustedDeviceList(onlineDeviceList);
    if (ret != DM_OK) {
        LOGE("GetTrustedDeviceList failed");
        return ret;
    }
    if (!onlineDeviceList.empty() && IsDMServiceImplReady()) {
        dmServiceImpl_->DeleteAlwaysAllowTimeOut();
        std::unordered_map<std::string, DmAuthForm> udidMap;
        if (PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName)) {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(std::string(ALL_PKGNAME));
        } else {
            udidMap = dmServiceImpl_->GetAppTrustDeviceIdList(pkgName);
        }
        for (auto item : onlineDeviceList) {
            std::string udid = "";
            SoftbusListener::GetUdidByNetworkId(item.networkId, udid);
            if (udidMap.find(udid) != udidMap.end()) {
                item.authForm = udidMap[udid];
                deviceList.push_back(item);
            }
        }
    }
    return DM_OK;
}

void DeviceManagerService::RemoveNotifyRecord(const ProcessInfo &processInfo)
{
    LOGI("start");
    CHECK_NULL_VOID(listener_);
    listener_->OnProcessRemove(processInfo);
}

void DeviceManagerService::HandleDeviceNotTrust(const std::string &msg)
{
    LOGI("Start.");
    if (msg.empty()) {
        LOGE("msg is empty.");
        return;
    }
    JsonObject msgJsonObj(msg);
    if (msgJsonObj.IsDiscarded()) {
        LOGE("msg prase error.");
        return;
    }
    if (!IsString(msgJsonObj, "NETWORK_ID")) {
        LOGE("msg not contain networkId.");
        return;
    }
    std::string networkId = msgJsonObj["NETWORK_ID"].Get<std::string>();
    std::string udid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(networkId.c_str(), udid);
    LOGI("NetworkId: %{public}s, udid: %{public}s.",
        GetAnonyString(networkId).c_str(), GetAnonyString(udid).c_str());
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->HandleDeviceNotTrust(udid);
    }
    if (IsDMServiceAdapterResidentLoad()) {
        dmServiceImplExtResident_->HandleDeviceNotTrust(udid);
    }
    return;
}

int32_t DeviceManagerService::CheckApiPermission(int32_t permissionLevel)
{
    LOGI("PermissionLevel: %{public}d", permissionLevel);
    int32_t ret = ERR_DM_NO_PERMISSION;
    switch (permissionLevel) {
        case NORMAL:
            if (PermissionManager::GetInstance().CheckDataSyncPermission()) {
                LOGI("The caller have permission to call");
                ret = DM_OK;
            }
            break;
        case SYSTEM_BASIC:
            if (PermissionManager::GetInstance().CheckAccessServicePermission()) {
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

bool DeviceManagerService::CheckConstraintEnabledByNetworkId(const std::string &networkId)
{
    DmDeviceInfo deviceInfo;
    SoftbusCache::GetInstance().GetLocalDeviceInfo(deviceInfo);
    if (networkId == std::string(deviceInfo.networkId)) {
        LOGI("get local deviceinfo, networkId %{public}s.", GetAnonyString(networkId).c_str());
        return false;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    return DmConstrainsManager::GetInstance().CheckOsAccountConstraintEnabled(
        MultipleUserConnector::GetForgroundUserId(), DM_ACCOUNT_CONSTRAINT);
#endif
    return false;
}

static std::string GetLocalDeviceUdid()
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    return std::string(localUdidTemp);
}
} // namespace DistributedHardware
} // namespace OHOS