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

#include <thread>
#include "dm_constants.h"
#include "dm_softbus_cache.h"
#include "parameter.h"
#include "permission_manager.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "dm_constraints_manager.h"
#include "common_event_support.h"
#include "deviceprofile_connector.h"
#include "device_name_manager.h"
#include "dm_comm_tool.h"
#include "dm_jsonstr_handle.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "kv_adapter_manager.h"
#include "multiple_user_connector.h"
#include "relationship_sync_mgr.h"
#include "openssl/sha.h"
#include "system_ability_definition.h"
#endif

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr const char* USER_SWITCH_BY_WIFI_TIMEOUT_TASK = "deviceManagerTimer:userSwitchByWifi";
    constexpr const char* USER_STOP_BY_WIFI_TIMEOUT_TASK = "deviceManagerTimer:userStopByWifi";
    constexpr const char* APP_UNINSTALL_BY_WIFI_TIMEOUT_TASK = "deviceManagerTimer:appUninstallByWifi";
    constexpr const char* APP_UNBIND_BY_WIFI_TIMEOUT_TASK = "deviceManagerTimer:appUnbindByWifi";
    constexpr const char* ACCOUNT_COMMON_EVENT_BY_WIFI_TIMEOUT_TASK = "deviceManagerTimer:accountCommonEventByWifi";
    constexpr const char* SERVICE_UNBIND_PROXY_BY_WIFI_TIMEOUT_TASK = "deviceManagerTimer:serviceUnbindProxyByWifi";
    const int32_t USER_SWITCH_BY_WIFI_TIMEOUT_S = 2;
    constexpr const char* LOCAL_ALL_USERID = "local_all_userId";
    constexpr const char* LOCAL_FOREGROUND_USERID = "local_foreground_userId";
    constexpr const char* LOCAL_BACKGROUND_USERID = "local_background_userId";
    constexpr const char* HANDLE_ACCOUNT_LOGOUT_EVENT_CALLBACK_TASK = "HandleAccountLogoutEventCallbackTask";
    constexpr const char* HANDLE_USER_REMOVED_TASK = "HandleUserRemovedTask";
    constexpr const char* HANDLE_ACCOUNT_LOGOUT_EVENT_TASK = "HandleAccountLogoutEventTask";
    constexpr const char* HANDLE_COMMON_EVENT_BROAD_CAST_TASK = "HandleCommonEventBroadCastTask";
    constexpr const char* HANDLE_USER_IDS_BROAD_CAST_TASK = "HandleUserIdsBroadCastTask";
    constexpr const char* HANDLE_REMOTE_USER_REMOVED_TASK = "HandleRemoteUserRemovedTask";
    constexpr const char* PEER_UDID = "peerUdid";
    constexpr const char* PEER_OSTYPE = "peerOsType";
}

static std::string GetLocalDeviceUdid()
{
    char localUdidTemp[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    return std::string(localUdidTemp);
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
DM_EXPORT void DeviceManagerService::AccountCommonEventCallback(
    const std::string commonEventType, int32_t currentUserId, int32_t beforeUserId)
{
    MultipleUserConnector::UpdateForgroundUserId();
    if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        HandleUserSwitchEventCallback(commonEventType, currentUserId, beforeUserId);
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN) {
        DeviceNameManager::GetInstance().InitDeviceNameWhenLogin();
        MultipleUserConnector::SetAccountInfo(currentUserId, MultipleUserConnector::GetCurrentDMAccountInfo());
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT) {
        ffrt::submit([=]() {
            HandleAccountLogoutEventCallback(commonEventType, currentUserId, beforeUserId);
        },
            ffrt::task_attr().name(HANDLE_ACCOUNT_LOGOUT_EVENT_CALLBACK_TASK));
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        ffrt::submit([=]() {
            HandleUserRemoved(beforeUserId);
        },
            ffrt::task_attr().name(HANDLE_USER_REMOVED_TASK));
        MultipleUserConnector::DeleteAccountInfoByUserId(beforeUserId);
        MultipleUserConnector::SetAccountInfo(MultipleUserConnector::GetCurrentAccountUserID(),
            MultipleUserConnector::GetCurrentDMAccountInfo());
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_INFO_UPDATED) {
        DeviceNameManager::GetInstance().InitDeviceNameWhenNickChange();
    } else if ((commonEventType == CommonEventSupport::COMMON_EVENT_USER_STOPPED && IsPC()) ||
        commonEventType == CommonEventSupport::COMMON_EVENT_USER_FOREGROUND ||
        commonEventType == CommonEventSupport::COMMON_EVENT_USER_BACKGROUND) {
        CHECK_NULL_VOID(DMCommTool::GetInstance());
        DMCommTool::GetInstance()->StartCommonEvent(commonEventType,
            [this, commonEventType] () {
                DeviceManagerService::HandleAccountCommonEvent(commonEventType);
            });
    } else if (commonEventType == CommonEventSupport::COMMON_EVENT_USER_UNLOCKED && IsPC()) {
        DeviceNameManager::GetInstance().AccountSysReady(beforeUserId);
        CHECK_NULL_VOID(DMCommTool::GetInstance());
        DMCommTool::GetInstance()->StartCommonEvent(commonEventType,
            [this, commonEventType] () {
                DeviceManagerService::HandleAccountCommonEvent(commonEventType);
            });
        DeleteInvalidSkIdAcl();
    } else {
        LOGE("Invalied account common event.");
    }
    return;
}

void DeviceManagerService::GetLocalUserIdFromDataBase(std::vector<int32_t> &foregroundUsers,
    std::vector<int32_t> &backgroundUsers)
{
    std::string userIdStr;
    KVAdapterManager::GetInstance().GetLocalUserIdData(LOCAL_ALL_USERID, userIdStr);
    if (userIdStr.empty()) {
        LOGE("result is empty");
        return;
    }
    JsonObject userIdJson(userIdStr);
    if (userIdJson.IsDiscarded()) {
        LOGE("userIdJson parse failed");
        return;
    }
    if (IsArray(userIdJson, LOCAL_FOREGROUND_USERID)) {
        userIdJson[LOCAL_FOREGROUND_USERID].Get(foregroundUsers);
    }
    if (IsArray(userIdJson, LOCAL_BACKGROUND_USERID)) {
        userIdJson[LOCAL_BACKGROUND_USERID].Get(backgroundUsers);
    }
}

void DeviceManagerService::PutLocalUserIdToDataBase(const std::vector<int32_t> &foregroundUsers,
    const std::vector<int32_t> &backgroundUsers)
{
    JsonObject jsonObj;
    jsonObj[LOCAL_FOREGROUND_USERID] = foregroundUsers;
    jsonObj[LOCAL_BACKGROUND_USERID] = backgroundUsers;
    std::string localUserIdStr = jsonObj.Dump();
    KVAdapterManager::GetInstance().PutLocalUserIdData(LOCAL_ALL_USERID, localUserIdStr);
}

bool DeviceManagerService::IsUserStatusChanged(std::vector<int32_t> foregroundUserVec,
    std::vector<int32_t> backgroundUserVec)
{
    LOGI("foregroundUserVec: %{public}s, backgroundUserVec: %{public}s",
        GetIntegerList(foregroundUserVec).c_str(), GetIntegerList(backgroundUserVec).c_str());
    std::vector<int32_t> dBForegroundUserIds;
    std::vector<int32_t> dBBackgroundUserIds;
    GetLocalUserIdFromDataBase(dBForegroundUserIds, dBBackgroundUserIds);
    LOGI("dBForegroundUserIds: %{public}s, dBBackgroundUserIds: %{public}s",
        GetIntegerList(dBForegroundUserIds).c_str(), GetIntegerList(dBBackgroundUserIds).c_str());
    std::sort(foregroundUserVec.begin(), foregroundUserVec.end());
    std::sort(backgroundUserVec.begin(), backgroundUserVec.end());
    std::sort(dBForegroundUserIds.begin(), dBForegroundUserIds.end());
    std::sort(dBBackgroundUserIds.begin(), dBBackgroundUserIds.end());
    if (foregroundUserVec == dBForegroundUserIds && backgroundUserVec == dBBackgroundUserIds) {
        LOGI("User status has not changed.");
        return false;
    }
    dBForegroundUserIds = foregroundUserVec;
    dBBackgroundUserIds = backgroundUserVec;
    PutLocalUserIdToDataBase(dBForegroundUserIds, dBBackgroundUserIds);
    return true;
}

void DeviceManagerService::HandleAccountCommonEvent(const std::string commonEventType)
{
    LOGI("commonEventType: %{public}s.", commonEventType.c_str());
    std::vector<int32_t> foregroundUserVec;
    int32_t retFront = MultipleUserConnector::GetForegroundUserIds(foregroundUserVec);
    std::vector<int32_t> backgroundUserVec;
    int32_t retBack = MultipleUserConnector::GetBackgroundUserIds(backgroundUserVec);
    MultipleUserConnector::ClearLockedUser(foregroundUserVec, backgroundUserVec);
    if (retFront != DM_OK || retBack != DM_OK) {
        LOGE("retFront: %{public}d, retBack: %{public}d, frontuserids: %{public}s, backuserids: %{public}s",
            retFront, retBack, GetIntegerList(foregroundUserVec).c_str(), GetIntegerList(backgroundUserVec).c_str());
        return;
    }
    if (!IsUserStatusChanged(foregroundUserVec, backgroundUserVec)) {
        LOGI("User status has not changed.");
        return;
    }
    std::string localUdid = GetLocalDeviceUdid();
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    if (!discoveryMgr_->GetCommonDependencyObj()->CheckAclStatusAndForegroundNotMatch(localUdid, foregroundUserVec,
        backgroundUserVec)) {
        LOGI("no unreasonable data.");
        return;
    }
    std::map<std::string, int32_t> curUserDeviceMap =
        discoveryMgr_->GetCommonDependencyObj()->GetDeviceIdAndBindLevel(foregroundUserVec, localUdid);
    std::map<std::string, int32_t> preUserDeviceMap =
        discoveryMgr_->GetCommonDependencyObj()->GetDeviceIdAndBindLevel(backgroundUserVec, localUdid);
    std::vector<std::string> peerUdids;
    PushPeerUdids(curUserDeviceMap, preUserDeviceMap, peerUdids);
    if (peerUdids.empty()) {
        LOGE("peerUdids is empty");
        return;
    }
    NotifyRemoteAccountCommonEvent(commonEventType, localUdid, peerUdids, foregroundUserVec, backgroundUserVec);
}

void DeviceManagerService::PushPeerUdids(const std::map<std::string, int32_t> &curUserDeviceMap,
    const std::map<std::string, int32_t> &preUserDeviceMap, std::vector<std::string> &peerUdids)
{
    for (const auto &item : curUserDeviceMap) {
        peerUdids.push_back(item.first);
    }
    for (const auto &item : preUserDeviceMap) {
        if (find(peerUdids.begin(), peerUdids.end(), item.first) == peerUdids.end()) {
            peerUdids.push_back(item.first);
        }
    }
}

void DeviceManagerService::NotifyRemoteAccountCommonEvent(const std::string commonEventType,
    const std::string &localUdid, const std::vector<std::string> &peerUdids,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("foregroundUserIds: %{public}s, backgroundUserIds: %{public}s",
        GetIntegerList<int32_t>(foregroundUserIds).c_str(), GetIntegerList<int32_t>(backgroundUserIds).c_str());
    if (peerUdids.empty()) {
        return;
    }
    if (softbusListener_ == nullptr) {
        UpdateAcl(localUdid, peerUdids, foregroundUserIds, backgroundUserIds);
        LOGE("softbusListener_ is null");
        return;
    }
    std::vector<std::string> bleUdids;
    std::map<std::string, std::string> wifiDevices;
    for (const auto &udid : peerUdids) {
        std::string netWorkId = "";
        SoftbusCache::GetInstance().GetNetworkIdFromCache(udid, netWorkId);
        if (netWorkId.empty()) {
            LOGI("netWorkId is empty: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        int32_t networkType = 0;
        if (softbusListener_->GetNetworkTypeByNetworkId(netWorkId.c_str(), networkType) != DM_OK || networkType <= 0) {
            LOGI("get networkType failed: %{public}s", GetAnonyString(udid).c_str());
            bleUdids.push_back(udid);
            continue;
        }
        uint32_t addrTypeMask = 1 << NetworkType::BIT_NETWORK_TYPE_BLE;
        if ((static_cast<uint32_t>(networkType) & addrTypeMask) != 0x0) {
            bleUdids.push_back(udid);
        } else {
            wifiDevices.insert(std::pair<std::string, std::string>(udid, netWorkId));
        }
    }
    if (!bleUdids.empty()) {
        UpdateAcl(localUdid, bleUdids, foregroundUserIds, backgroundUserIds);
        SendCommonEventBroadCast(commonEventType, bleUdids, foregroundUserIds, backgroundUserIds);
    }
    if (!wifiDevices.empty()) {
        NotifyRemoteAccountCommonEventByWifi(localUdid, wifiDevices, foregroundUserIds, backgroundUserIds);
    }
}

void DeviceManagerService::NotifyRemoteAccountCommonEventByWifi(const std::string &localUdid,
    const std::map<std::string, std::string> &wifiDevices, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    for (const auto &it : wifiDevices) {
        int32_t result = SendAccountCommonEventByWifi(it.second, foregroundUserIds, backgroundUserIds);
        if (result != DM_OK) {
            LOGE("by wifi failed: %{public}s", GetAnonyString(it.first).c_str());
            std::vector<std::string> updateUdids;
            updateUdids.push_back(it.first);
            UpdateAcl(localUdid, updateUdids, foregroundUserIds, backgroundUserIds);
            continue;
        }
        std::lock_guard<std::mutex> autoLock(timerLocks_);
        if (timer_ == nullptr) {
            timer_ = std::make_shared<DmTimer>();
        }
        std::string udid = it.first;
        timer_->StartTimer(std::string(ACCOUNT_COMMON_EVENT_BY_WIFI_TIMEOUT_TASK) + Crypto::Sha256(udid),
            USER_SWITCH_BY_WIFI_TIMEOUT_S,
            [this, localUdid, foregroundUserIds, backgroundUserIds, udid] (std::string name) {
                DeviceManagerService::HandleCommonEventTimeout(localUdid, foregroundUserIds, backgroundUserIds, udid);
            });
    }
}

int32_t DeviceManagerService::SendAccountCommonEventByWifi(const std::string &networkId,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("start");
    std::vector<uint32_t> foregroundUserIdsUInt;
    for (auto const &u : foregroundUserIds) {
        foregroundUserIdsUInt.push_back(static_cast<uint32_t>(u));
    }
    std::vector<uint32_t> backgroundUserIdsUInt;
    for (auto const &u : backgroundUserIds) {
        backgroundUserIdsUInt.push_back(static_cast<uint32_t>(u));
    }
    CHECK_NULL_RETURN(DMCommTool::GetInstance(), ERR_DM_POINT_NULL);
    return DMCommTool::GetInstance()->SendUserIds(networkId, foregroundUserIdsUInt, backgroundUserIdsUInt);
}

void DeviceManagerService::HandleCommonEventTimeout(const std::string &localUdid,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds,
    const std::string &udid)
{
    LOGI("start udid: %{public}s", GetAnonyString(udid).c_str());
    std::vector<std::string> updateUdids;
    updateUdids.push_back(udid);
    UpdateAcl(localUdid, updateUdids, foregroundUserIds, backgroundUserIds);
}

void DeviceManagerService::UpdateAcl(const std::string &localUdid,
    const std::vector<std::string> &peerUdids, const std::vector<int32_t> &foregroundUserIds,
    const std::vector<int32_t> &backgroundUserIds)
{
    CHECK_NULL_VOID(discoveryMgr_);
    if (!discoveryMgr_->IsCommonDependencyReady() || discoveryMgr_->GetCommonDependencyObj() == nullptr) {
        LOGE("IsCommonDependencyReady failed or GetCommonDependencyObj() is nullptr.");
        return;
    }
    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    discoveryMgr_->GetCommonDependencyObj()->HandleAccountCommonEvent(localUdid, peerUdids, foregroundUserIds,
        backgroundUserIds, serviceInfos);
    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("IsDMServiceAdapterResidentLoad failed.");
        return;
    }
    for (auto &item : serviceInfos) {
        if (item.isActive == true) {
            for (size_t i = 0; i < item.serviceIds.size(); i++) {
                ServiceStateBindParameter bindParam = {
                    item.localTokenId,
                    item.localPkgName,
                    item.bindType,
                    item.peerUdid,
                    item.peerUserId,
                    item.serviceIds[i]
                };
                dmServiceImplExtResident_->BindServiceOnline(bindParam);
            }
        } else {
            for (size_t i = 0; i < item.serviceIds.size(); i++) {
                DistributedDeviceProfile::ServiceInfo dpServiceInfo;
                DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(
                    item.peerUdid, item.serviceIds[i], dpServiceInfo);
                dmServiceImplExtResident_->BindServiceOffline(item.localTokenId, item.localPkgName,
                    item.bindType, item.peerUdid, dpServiceInfo);
            }
        }
    }
}

void DeviceManagerService::HandleAccountLogout(int32_t userId, const std::string &accountId,
    const std::string &accountName)
{
    LOGI("UserId: %{public}d, accountId: %{public}s, accountName: %{public}s", userId,
        GetAnonyString(accountId).c_str(), GetAnonyString(accountName).c_str());

    if (IsDMServiceAdapterResidentLoad()) {
        dmServiceImplExtResident_->ClearCacheWhenLogout(userId, accountId);
    } else {
        LOGW("ClearCacheWhenLogout fail, adapter instance not init or init failed.");
    }
    if (!IsDMServiceImplReady()) {
        LOGE("Init impl failed.");
        return;
    }

    std::string localUdid = GetLocalDeviceUdid();
    auto deviceMap = dmServiceImpl_->GetDeviceIdAndUserId(userId, accountId);

    std::vector<std::string> peerHOUdids;
    GetHoOsTypeUdids(peerHOUdids);
    std::vector<std::string> peerUdids;
    std::vector<std::string> dualPeerUdids;
    for (const auto &item : deviceMap) {
        if (find(peerHOUdids.begin(), peerHOUdids.end(), item.first) != peerHOUdids.end()) {
            LOGI("dualUdid: %{public}s", GetAnonyString(item.first).c_str());
            dualPeerUdids.emplace_back(item.first);
        } else {
            peerUdids.emplace_back(item.first);
        }
    }

    if (!dualPeerUdids.empty() && IsDMServiceAdapterResidentLoad()) {
        LOGI("logout notify userId: %{public}d, accountId: %{public}s", userId, GetAnonyString(accountId).c_str());
        dmServiceImplExtResident_->AccountIdLogout(userId, accountId, dualPeerUdids);
    }
    HandleRegularPeerLogout(userId, accountId, accountName, peerUdids);
    ProcessDeviceMapForLogout(userId, deviceMap, localUdid, accountId);
}

void DeviceManagerService::HandleRegularPeerLogout(int32_t userId, const std::string &accountId,
    const std::string &accountName, const std::vector<std::string>& peerUdids)
{
    if (!peerUdids.empty()) {
        char accountIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (Crypto::GetAccountIdHash(accountId, reinterpret_cast<uint8_t *>(accountIdHash)) != DM_OK) {
            LOGE("GetAccountHash failed, userId: %{public}d, accountId: %{public}s, accountName: %{public}s",
                userId, GetAnonyString(accountId).c_str(), GetAnonyString(accountName).c_str());
            return;
        }
        NotifyRemoteLocalLogout(peerUdids, std::string(accountIdHash), accountName, userId);
    }
}

void DeviceManagerService::ProcessDeviceMapForLogout(int32_t userId,
    const std::multimap<std::string, int32_t> &deviceMap, const std::string &localUdid,
    const std::string &accountId)
{
    for (const auto &item : deviceMap) {
        DMAclQuadInfo info = {localUdid, userId, item.first, item.second};
        std::vector<DmUserRemovedServiceInfo> serviceInfos;
        dmServiceImpl_->HandleIdentAccountLogout(info, accountId, serviceInfos);
        if (!IsDMServiceAdapterResidentLoad()) {
            LOGE("IsDMServiceAdapterResidentLoad failed.");
            return;
        }
        LogoutProcessServiceInfos(serviceInfos, localUdid);
    }
}

void DeviceManagerService::LogoutProcessServiceInfos(const std::vector<DmUserRemovedServiceInfo> &serviceInfos,
    const std::string& localUdid)
{
    for (const auto &item : serviceInfos) {
        for (size_t i = 0; i < item.serviceIds.size(); i++) {
            DistributedDeviceProfile::ServiceInfo dpServiceInfo;
            DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(
                item.peerUdid, item.serviceIds[i], dpServiceInfo);
            dmServiceImplExtResident_->BindServiceOffline(item.localTokenId, item.localPkgName,
                item.bindType, item.peerUdid, dpServiceInfo);
            dmServiceImplExtResident_->DeleteServiceInfoForAccountEvent(item.peerUserId, item.peerUdid,
                localUdid);
        }
    }
}

void DeviceManagerService::HandleUserRemoved(int32_t removedUserId)
{
    LOGI("PreUserId %{public}d.", removedUserId);
    if (!IsDMServiceImplReady()) {
        LOGE("Init impl failed.");
        return;
    }
    std::multimap<std::string, int32_t> deviceMap = dmServiceImpl_->GetDeviceIdAndUserId(removedUserId);
    std::vector<std::string> peerUdids;
    for (const auto &item : deviceMap) {
        if (find(peerUdids.begin(), peerUdids.end(), item.first) == peerUdids.end()) {
            peerUdids.emplace_back(item.first);
        }
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    if (!peerUdids.empty()) {
        SendUserRemovedBroadCast(peerUdids, removedUserId);
        std::vector<DmUserRemovedServiceInfo> serviceInfos;
        dmServiceImpl_->HandleUserRemoved(peerUdids, removedUserId, serviceInfos);
        if (!IsDMServiceAdapterResidentLoad()) {
            LOGE("IsDMServiceAdapterResidentLoad failed.");
            return;
        }
        for (auto &item : serviceInfos) {
            for (size_t i = 0; i < item.serviceIds.size(); i++) {
                DistributedDeviceProfile::ServiceInfo dpServiceInfo;
                DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(
                    item.peerUdid, item.serviceIds[i], dpServiceInfo);
                dmServiceImplExtResident_->BindServiceOffline(item.localTokenId, item.localPkgName,
                    item.bindType, item.peerUdid, dpServiceInfo);
                dmServiceImplExtResident_->DeleteServiceInfoForAccountEvent(item.peerUserId, item.peerUdid,
                    localUdid);
            }
        }
    }
    IpcServerStub::GetInstance().HandleUserRemoved(removedUserId);
}

void DeviceManagerService::SendUserRemovedBroadCast(const std::vector<std::string> &peerUdids, int32_t userId)
{
    LOGI("peerUdids: %{public}s, userId %{public}d.", GetAnonyStringList(peerUdids).c_str(), userId);
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::DEL_USER;
    msg.userId = static_cast<uint32_t>(userId);
    msg.peerUdids = peerUdids;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}

void DeviceManagerService::SendAccountLogoutBroadCast(const std::vector<std::string> &peerUdids,
    const std::string &accountId, const std::string &accountName, int32_t userId)
{
    LOGI("accountId %{public}s, accountName %{public}s, userId %{public}d.", GetAnonyString(accountId).c_str(),
        GetAnonyString(accountName).c_str(), userId);
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::ACCOUNT_LOGOUT;
    msg.userId = static_cast<uint32_t>(userId);
    msg.peerUdids = peerUdids;
    msg.accountId = accountId;
    msg.accountName = accountName;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}
#endif

void DeviceManagerService::HandleUserIdsBroadCast(const std::vector<UserIdInfo> &remoteUserIdInfos,
    const std::string &remoteUdid, bool isNeedResponse)
{
    LOGI("rmtUdid: %{public}s, rmtUserIds: %{public}s, isNeedResponse: %{public}s,",
        GetAnonyString(remoteUdid).c_str(), GetUserIdInfoList(remoteUserIdInfos).c_str(),
        isNeedResponse ? "true" : "false");

    if (isNeedResponse) {
        std::vector<int32_t> foregroundUserVec;
        std::vector<int32_t> backgroundUserVec;
        int32_t retFront = MultipleUserConnector::GetForegroundUserIds(foregroundUserVec);
        int32_t retBack = MultipleUserConnector::GetBackgroundUserIds(backgroundUserVec);

        if (IsPC()) {
            MultipleUserConnector::ClearLockedUser(foregroundUserVec, backgroundUserVec);
        }

        if (retFront != DM_OK || retBack != DM_OK) {
            LOGE("Get userid failed, retFront: %{public}d, retBack: %{public}d, frontUserNum:%{public}d,"
                "backUserNum: %{public}d", retFront, retBack,
                static_cast<int32_t>(foregroundUserVec.size()),
                static_cast<int32_t>(backgroundUserVec.size()));
        } else {
            LOGI("Send back local frontuserids: %{public}s, backuserids: %{public}s",
                GetIntegerList(foregroundUserVec).c_str(), GetIntegerList(backgroundUserVec).c_str());
            std::vector<std::string> remoteUdids = { remoteUdid };
            SendUserIdsBroadCast(remoteUdids, foregroundUserVec, backgroundUserVec, false);
        }
    }

    ProcessReceivedUserIds(remoteUserIdInfos, remoteUdid);
    ProcessServiceBindings(remoteUserIdInfos, remoteUdid);
}

static std::vector<uint32_t> ConvertToUserIdVector(const std::vector<UserIdInfo> &userIdInfos)
{
    std::vector<uint32_t> userIds;
    for (const auto &u : userIdInfos) {
        userIds.push_back(static_cast<uint32_t>(u.userId));
    }
    return userIds;
}

void DeviceManagerService::ProcessReceivedUserIds(const std::vector<UserIdInfo> &remoteUserIdInfos,
    const std::string &remoteUdid)
{
    std::vector<UserIdInfo> foregroundUserIdInfos;
    std::vector<UserIdInfo> backgroundUserIdInfos;
    GetFrontAndBackUserIdInfos(remoteUserIdInfos, foregroundUserIdInfos, backgroundUserIdInfos);

    LOGI("process foreground and background userids");

    std::vector<uint32_t> foregroundUserIds = ConvertToUserIdVector(foregroundUserIdInfos);
    std::vector<uint32_t> backgroundUserIds = ConvertToUserIdVector(backgroundUserIdInfos);

    if (softbusListener_ != nullptr) {
        softbusListener_->SetForegroundUserIdsToDSoftBus(remoteUdid, foregroundUserIds);
    }
}

void DeviceManagerService::ProcessServiceBindings(const std::vector<UserIdInfo> &remoteUserIdInfos,
    const std::string &remoteUdid)
{
    if (!IsDMServiceImplReady()) {
        return;
    }

    std::vector<UserIdInfo> foregroundUserIdInfos;
    std::vector<UserIdInfo> backgroundUserIdInfos;
    GetFrontAndBackUserIdInfos(remoteUserIdInfos, foregroundUserIdInfos, backgroundUserIdInfos);

    std::vector<uint32_t> foregroundUserIds = ConvertToUserIdVector(foregroundUserIdInfos);
    std::vector<uint32_t> backgroundUserIds = ConvertToUserIdVector(backgroundUserIdInfos);

    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);

    std::vector<DmUserRemovedServiceInfo> serviceInfos;
    dmServiceImpl_->HandleSyncUserIdEvent(foregroundUserIds, backgroundUserIds, remoteUdid, IsPC(), serviceInfos);

    if (!IsDMServiceAdapterResidentLoad()) {
        LOGE("IsDMServiceAdapterResidentLoad failed.");
        return;
    }
    for (auto &item : serviceInfos) {
        if (item.isActive) {
            ProcessActiveServices(item);
        } else {
            ProcessInactiveServices(item);
        }
    }
}

void DeviceManagerService::ProcessActiveServices(const DmUserRemovedServiceInfo &serviceInfo)
{
    for (size_t i = 0; i < serviceInfo.serviceIds.size(); i++) {
        ServiceStateBindParameter bindParam = {
            serviceInfo.localTokenId,
            serviceInfo.localPkgName,
            serviceInfo.bindType,
            serviceInfo.peerUdid,
            serviceInfo.peerUserId,
            serviceInfo.serviceIds[i]
        };
        dmServiceImplExtResident_->BindServiceOnline(bindParam);
    }
}

void DeviceManagerService::ProcessInactiveServices(const DmUserRemovedServiceInfo &serviceInfo)
{
    for (size_t i = 0; i < serviceInfo.serviceIds.size(); i++) {
        DistributedDeviceProfile::ServiceInfo dpServiceInfo;
        DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(serviceInfo.peerUdid,
            serviceInfo.serviceIds[i], dpServiceInfo);
        dmServiceImplExtResident_->BindServiceOffline(
            serviceInfo.localTokenId,
            serviceInfo.localPkgName,
            serviceInfo.bindType,
            serviceInfo.peerUdid,
            dpServiceInfo);
    }
}

void DeviceManagerService::ProcessSyncUserIdsInner(std::vector<DmUserRemovedServiceInfo> &serviceInfos)
{
    for (auto &item : serviceInfos) {
        if (item.isActive == true) {
            for (size_t i = 0; i < item.serviceIds.size(); i++) {
                ServiceStateBindParameter bindParam = {
                    item.localTokenId,
                    item.localPkgName,
                    item.bindType,
                    item.peerUdid,
                    item.peerUserId,
                    item.serviceIds[i]
                };
                dmServiceImplExtResident_->BindServiceOnline(bindParam);
            }
        } else {
            for (size_t i = 0; i < item.serviceIds.size(); i++) {
                DistributedDeviceProfile::ServiceInfo dpServiceInfo;
                DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(
                    item.peerUdid, item.serviceIds[i], dpServiceInfo);
                dmServiceImplExtResident_->BindServiceOffline(item.localTokenId, item.localPkgName,
                    item.bindType, item.peerUdid, dpServiceInfo);
            }
        }
    }
}

void DeviceManagerService::ProcessSyncUserIds(const std::vector<uint32_t> &foregroundUserIds,
    const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid)
{
    LOGI("process sync foregroundUserIds: %{public}s, backgroundUserIds: %{public}s, remote udid: %{public}s",
        GetIntegerList<uint32_t>(foregroundUserIds).c_str(), GetIntegerList<uint32_t>(backgroundUserIds).c_str(),
        GetAnonyString(remoteUdid).c_str());

    if (softbusListener_ != nullptr) {
        softbusListener_->SetForegroundUserIdsToDSoftBus(remoteUdid, foregroundUserIds);
    }
    std::lock_guard<std::mutex> autoLock(timerLocks_);
    if (timer_ != nullptr) {
        timer_->DeleteTimer(std::string(USER_SWITCH_BY_WIFI_TIMEOUT_TASK) + Crypto::Sha256(remoteUdid));
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    if (IsDMServiceImplReady()) {
        std::vector<DmUserRemovedServiceInfo> serviceInfos;
        dmServiceImpl_->HandleSyncUserIdEvent(foregroundUserIds, backgroundUserIds, remoteUdid, IsPC(), serviceInfos);
        if (!IsDMServiceAdapterResidentLoad()) {
            LOGE("IsDMServiceAdapterResidentLoad failed.");
            return;
        }
        ProcessSyncUserIdsInner(serviceInfos);
    }
}

void DeviceManagerService::ProcessUninstApp(int32_t userId, int32_t tokenId)
{
    LOGI("userId: %{public}s, tokenId: %{public}s",
        GetAnonyInt32(userId).c_str(), GetAnonyInt32(tokenId).c_str());
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->ProcessAppUninstall(userId, tokenId);
    }
}

void DeviceManagerService::ProcessUnBindApp(int32_t userId, int32_t tokenId, const std::string &extra,
    const std::string &udid)
{
    LOGI("userId: %{public}s, tokenId: %{public}s, udid: %{public}s",
        GetAnonyInt32(userId).c_str(), GetAnonyInt32(tokenId).c_str(), GetAnonyString(udid).c_str());
    if (IsDMServiceImplReady()) {
        dmServiceImpl_->ProcessUnBindApp(userId, tokenId, extra, udid);
    }
}

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
void DeviceManagerService::HandleAccountLogoutEventCallback(const std::string &commonEventType, int32_t currentUserId,
    int32_t beforeUserId)
{
    LOGI("commonEventType %{public}s, currentUserId %{public}d, beforeUserId %{public}d.", commonEventType.c_str(),
        currentUserId, beforeUserId);
    DeviceProfileConnector::GetInstance().DeleteDpInvalidAcl();
    DeviceNameManager::GetInstance().InitDeviceNameWhenLogout();
    DMAccountInfo dmAccountInfo = MultipleUserConnector::GetAccountInfoByUserId(beforeUserId);
    if (dmAccountInfo.accountId.empty()) {
        LOGE("dmAccountInfo accountId empty.");
        return;
    }
    HandleAccountLogout(currentUserId, dmAccountInfo.accountId, dmAccountInfo.accountName);
    MultipleUserConnector::DeleteAccountInfoByUserId(currentUserId);
    MultipleUserConnector::SetAccountInfo(MultipleUserConnector::GetCurrentAccountUserID(),
        MultipleUserConnector::GetCurrentDMAccountInfo());
    IpcServerStub::GetInstance().HandleAccountLogoutEvent(currentUserId, dmAccountInfo.accountId);
}

void DeviceManagerService::GetHoOsTypeUdids(std::vector<std::string> &peerUdids)
{
    std::vector<std::string> osTypeStrs;
    if (KVAdapterManager::GetInstance().GetAllOstypeData(osTypeStrs) != DM_OK) {
        LOGE("Get all ostype failed.");
        return;
    }
    for (const auto &item : osTypeStrs) {
        JsonObject osTypeObj(item);
        if (osTypeObj.IsDiscarded()) {
            LOGE("msg prase error.");
            continue;
        }
        if (!IsString(osTypeObj, PEER_UDID) || !IsInt32(osTypeObj, PEER_OSTYPE)) {
            LOGE("osTypeObj value invalid.");
            continue;
        }
        if (osTypeObj[PEER_OSTYPE].Get<int32_t>() == OLD_DM_HO_OSTYPE ||
            osTypeObj[PEER_OSTYPE].Get<int32_t>() == NEW_DM_HO_OSTYPE) {
            peerUdids.push_back(osTypeObj[PEER_UDID].Get<std::string>());
        }
    }
}

void DeviceManagerService::DeleteInvalidSkIdAcl()
{
    std::map<std::string, DmOfflineParam> invalidAclMap;
    DeviceProfileConnector::GetInstance().GetInvalidSkIdAcl(invalidAclMap);
    for (const auto &iter : invalidAclMap) {
        if (!IsDMServiceImplReady()) {
            LOGE("instance not init or init failed.");
            return;
        }
        dmServiceImpl_->DeleteSkCredAndAcl(iter.second.needDelAclInfos);
        if (iter.second.allLeftAppOrSvrAclInfos.empty() && iter.second.allUserAclInfos.empty()) {
            LOGI("No acl exist, clear lnn acl");
            dmServiceImpl_->DeleteSkCredAndAcl(iter.second.allLnnAclInfos);
        }
        DmOfflineParam dmParam = iter.second;
        dmServiceImpl_->NotifyDeviceOrAppOffline(dmParam, iter.first);
    }
}
#endif

void DeviceManagerService::SendUserIdsBroadCast(const std::vector<std::string> &peerUdids,
    const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds, bool isNeedResponse)
{
    LOGI("peerUdids: %{public}s, foregroundUserIds: %{public}s, backgroundUserIds: %{public}s, isNeedRsp: %{public}s",
        GetAnonyStringList(peerUdids).c_str(), GetIntegerList<int32_t>(foregroundUserIds).c_str(),
        GetIntegerList<int32_t>(backgroundUserIds).c_str(), isNeedResponse ? "true" : "false");
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::SYNC_USERID;
    msg.peerUdids = peerUdids;
    msg.syncUserIdFlag = isNeedResponse;
    for (const auto &userId : foregroundUserIds) {
        msg.userIdInfos.push_back({ true, static_cast<uint16_t>(userId) });
    }
    for (auto const &userId : backgroundUserIds) {
        msg.userIdInfos.push_back({ false, static_cast<uint16_t>(userId) });
    }
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    CHECK_NULL_VOID(softbusListener_);
    softbusListener_->SendAclChangedBroadcast(broadCastMsg);
}
} // namespace DistributedHardware
} // namespace OHOS