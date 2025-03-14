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

#ifndef OHOS_DM_SERVICE_IMPL_H
#define OHOS_DM_SERVICE_IMPL_H

#include <string>
#include <vector>

#include "access_control_profile.h"
#include "dm_ability_manager.h"
#include "dm_auth_manager.h"
#include "dm_common_event_manager.h"
#include "dm_credential_manager.h"
#include "dm_device_info.h"
#include "dm_device_state_manager.h"
#include "dp_inited_callback.h"
#include "idevice_manager_service_impl.h"
#include "dm_single_instance.h"
#include "softbus_connector.h"
#include "mine_hichain_connector.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerServiceImpl : public IDeviceManagerServiceImpl {
public:
    DeviceManagerServiceImpl();
    virtual ~DeviceManagerServiceImpl();

    int32_t Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener);

    void Release();

    int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel);

    int32_t UnBindDevice(const std::string &pkgName, const std::string &udid, int32_t bindLevel);

    int32_t UnBindDevice(const std::string &pkgName, const std::string &udid,
        int32_t bindLevel, const std::string &extra);

    int32_t SetUserOperation(std::string &pkgName, int32_t action, const std::string &params);

    void HandleDeviceStatusChange(DmDeviceState devState, DmDeviceInfo &devInfo);

    int OnSessionOpened(int sessionId, int result);

    void OnSessionClosed(int sessionId);

    void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    int OnPinHolderSessionOpened(int sessionId, int result);

    void OnPinHolderSessionClosed(int sessionId);

    void OnPinHolderBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    int32_t RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr);

    int32_t ImportCredential(const std::string &pkgName, const std::string &credentialInfo);

    int32_t DeleteCredential(const std::string &pkgName, const std::string &deleteInfo);

    int32_t MineRequestCredential(const std::string &pkgName, std::string &returnJsonStr);

    int32_t CheckCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr);

    int32_t ImportCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr);

    int32_t DeleteCredential(const std::string &pkgName, const std::string &reqJsonStr,
        std::string &returnJsonStr);

    int32_t RegisterCredentialCallback(const std::string &pkgName);

    int32_t UnRegisterCredentialCallback(const std::string &pkgName);

    int32_t NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event);

    int32_t GetGroupType(std::vector<DmDeviceInfo> &deviceList);

    int32_t GetUdidHashByNetWorkId(const char *networkId, std::string &deviceId);

    void LoadHardwareFwkService();

    int32_t RegisterUiStateCallback(const std::string &pkgName);

    int32_t UnRegisterUiStateCallback(const std::string &pkgName);

    int32_t ImportAuthCode(const std::string &pkgName, const std::string &authCode);

    int32_t ExportAuthCode(std::string &authCode);

    int32_t BindTarget(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam);

    std::unordered_map<std::string, DmAuthForm> GetAppTrustDeviceIdList(std::string pkgname);

    int32_t DpAclAdd(const std::string &udid);
    int32_t IsSameAccount(const std::string &udid);
    uint64_t GetTokenIdByNameAndDeviceId(std::string pkgName, std::string requestDeviceId);
    void ScreenCommonEventCallback(std::string commonEventType);
    int32_t CheckIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid);
    int32_t CheckAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid);
    void HandleDeviceNotTrust(const std::string &udid);
    int32_t GetBindLevel(const std::string &pkgName, const std::string &localUdid,
        const std::string &udid, uint64_t &tokenId);
    void HandleIdentAccountLogout(const std::string &localUdid, int32_t localUserId, const std::string &peerUdid,
        int32_t peerUserId);
    void HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo);
    int32_t StopAuthenticateDevice(const std::string &pkgName);
    void HandleCredentialAuthStatus(const std::string &deviceList, uint16_t deviceTypeId, int32_t errcode);
    int32_t ProcessAppUnintall(const std::string &appId, int32_t accessTokenId);
    void HandleSyncUserIdEvent(const std::vector<uint32_t> &foregroundUserIds,
        const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid, bool isCheckUserStatus);
    void HandleUserSwitched(const std::vector<std::string> &deviceVec, int32_t currentUserId,
        int32_t beforeUserId);
    std::multimap<std::string, int32_t> GetDeviceIdAndUserId(int32_t localUserId);
    int32_t SaveOnlineDeviceInfo(const std::vector<DmDeviceInfo> &deviceList);
    void HandleDeviceUnBind(int32_t bindType, const std::string &peerUdid,
        const std::string &localUdid, int32_t localUserId, const std::string &localAccountId);
    int32_t RegisterAuthenticationType(int32_t authenticationType);
    void DeleteAlwaysAllowTimeOut();
    void CheckDeleteCredential(const std::string &remoteUdid);
    int32_t CheckDeviceInfoPermission(const std::string &localUdid, const std::string &peerDeviceId);
private:
    int32_t PraseNotifyEventJson(const std::string &event, nlohmann::json &jsonObject);
    std::string GetUdidHashByNetworkId(const std::string &networkId);
    void HandleOffline(DmDeviceState devState, DmDeviceInfo &devInfo);
    void HandleOnline(DmDeviceState devState, DmDeviceInfo &devInfo);
    void PutIdenticalAccountToAcl(std::string requestDeviceId, std::string trustDeviceId);
    std::map<std::string, int32_t> GetDeviceIdAndBindLevel(int32_t userId);
    std::multimap<std::string, int32_t> GetDeviceIdAndUserId(int32_t userId, const std::string &accountId);
    void HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
        const std::string &remoteUdid);
    void HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid);
    void HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid, int32_t tokenId);
    void HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
        int32_t tokenId, int32_t peerTokenId);
    void HandleUserRemoved(int32_t preUserId);
    void HandleRemoteUserRemoved(int32_t preUserId, const std::string &remoteUdid);
    DmAuthForm ConvertBindTypeToAuthForm(int32_t bindType);

private:
    std::shared_ptr<DmAuthManager> authMgr_;
    std::shared_ptr<DmDeviceStateManager> deviceStateMgr_;
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<DmAbilityManager> abilityMgr_;
    std::shared_ptr<HiChainConnector> hiChainConnector_;
    std::shared_ptr<MineHiChainConnector> mineHiChainConnector_;
    std::shared_ptr<DmCredentialManager> credentialMgr_;
    std::shared_ptr<DmCommonEventManager> commonEventManager_;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::atomic<bool> isCredentialType_ = false;
    sptr<DpInitedCallback> dpInitedCallback_ = nullptr;
};

using CreateDMServiceFuncPtr = IDeviceManagerServiceImpl *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_IMPL_H
