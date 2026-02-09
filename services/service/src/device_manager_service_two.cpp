/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <functional>
#include "app_manager.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "parameter.h"
#include "permission_manager.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DeviceManagerService::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call ImportCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (pkgName.empty() || credentialInfo.empty()) {
        LOGE("DeviceManagerServiceImpl::ImportCredential failed, pkgName is %{public}s, credentialInfo is %{public}s",
            pkgName.c_str(), GetAnonyString(credentialInfo).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_== nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->ImportCredential(pkgName, credentialInfo);
#else
    if (!IsDMServiceImplReady()) {
        LOGE("ImportCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->ImportCredential(pkgName, credentialInfo);
#endif
}

int32_t DeviceManagerService::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call DeleteCredential.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (pkgName.empty() || deleteInfo.empty()) {
        LOGE("DeviceManagerServiceImpl::DeleteCredential failed, pkgName is %{public}s, deleteInfo is %{public}s",
            pkgName.c_str(), GetAnonyString(deleteInfo).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_== nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->DeleteCredential(pkgName, deleteInfo);
#else
    if (!IsDMServiceImplReady()) {
        LOGE("DeleteCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->DeleteCredential(pkgName, deleteInfo);
#endif
}

int32_t DeviceManagerService::RegisterCredentialCallback(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call RegisterCredentialCallback.", pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (pkgName.empty()) {
        LOGE("RegisterCredentialCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_ == nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->RegisterCredentialCallback(pkgName);
#else
    if (!IsDMServiceImplReady()) {
        LOGE("RegisterCredentialCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RegisterCredentialCallback(pkgName);
#endif
}

int32_t DeviceManagerService::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (!PermissionManager::GetInstance().CheckAccessServicePermission()) {
        LOGE("The caller: %{public}s does not have permission to call UnRegisterCredentialCallback.",
            pkgName.c_str());
        return ERR_DM_NO_PERMISSION;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    if (pkgName.empty()) {
        LOGE("UnRegisterCredentialCallback failed, pkgName is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (credentialMgr_== nullptr) {
        LOGE("credentialMgr_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    return credentialMgr_->UnRegisterCredentialCallback(pkgName);
#else
    if (!IsDMServiceImplReady()) {
        LOGE("UnRegisterCredentialCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnRegisterCredentialCallback(pkgName);
#endif
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DeviceManagerService::PrepareLogoutDeviceInfo(int32_t userId, const std::string &accountId,
    std::multimap<std::string, int32_t> &deviceMap, std::vector<std::string> &peerUdids,
    std::vector<std::string> &dualPeerUdids, std::string &localUdid)
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    localUdid = std::string(localUdidTemp);
    deviceMap = dmServiceImpl_->GetDeviceIdAndUserId(userId, accountId);
    std::vector<std::string> peerHOUdids;
    GetHoOsTypeUdids(peerHOUdids);
    for (const auto &item : deviceMap) {
        if (find(peerHOUdids.begin(), peerHOUdids.end(), item.first) != peerHOUdids.end()) {
            LOGI("dualUdid: %{public}s", GetAnonyString(item.first).c_str());
            dualPeerUdids.emplace_back(item.first);
        }
        peerUdids.emplace_back(item.first);
    }
}

void DeviceManagerService::ProcessDualPeerDeviceLogout(int32_t userId, const std::string &accountId,
    const std::vector<std::string> &dualPeerUdids, const std::vector<std::string> &peerUdids)
{
    if (!dualPeerUdids.empty()) {
        if (IsDMServiceAdapterResidentLoad()) {
            dmServiceImplExtResident_->AccountIdLogout(userId, accountId, peerUdids);
        }
    }
}

void DeviceManagerService::ProcessDeviceServiceLogout(int32_t userId, const std::string &accountId,
    const std::multimap<std::string, int32_t> &deviceMap, const std::string &localUdid)
{
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("IsDMServiceAdapterResidentLoad failed.");
        return;
    }
    for (const auto &item : deviceMap) {
        DMAclQuadInfo info = {localUdid, userId, item.first, item.second};
        std::vector<DmUserRemovedServiceInfo> serviceInfos;
        dmServiceImpl_->HandleIdentAccountLogout(info, accountId, serviceInfos);
        for (auto &serviceItem : serviceInfos) {
            for (size_t i = 0; i < serviceItem.serviceIds.size(); i++) {
                DistributedDeviceProfile::ServiceInfo dpServiceInfo;
                DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(
                    serviceItem.peerUdid, serviceItem.serviceIds[i], dpServiceInfo);
                dmServiceImplExtResident_->BindServiceOffline(serviceItem.localTokenId, serviceItem.localPkgName,
                    serviceItem.bindType, serviceItem.peerUdid, dpServiceInfo);
                dmServiceImplExtResident_->DeleteServiceInfoForAccountEvent(serviceItem.peerUserId,
                    serviceItem.peerUdid, localUdid);
            }
        }
    }
}
#endif
} // namespace DistributedHardware
} // namespace OHOS
