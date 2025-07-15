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

#include <map>
#include <queue>
#include <semaphore>
#include <set>
#include <string>
#include <vector>

#include "access_control_profile.h"
#include "auth_manager.h"
#include "dm_ability_manager.h"
#include "dm_auth_manager.h"
#include "dm_auth_manager_base.h"
#include "dm_common_event_manager.h"
#include "dm_credential_manager.h"
#include "dm_device_info.h"
#include "dm_device_state_manager.h"
#include "dm_single_instance.h"
#include "dp_inited_callback.h"
#include "idevice_manager_service_impl.h"
#include "ipc_skeleton.h"
#include "mine_hichain_connector.h"
#include "softbus_connector.h"
#include "deviceprofile_connector.h"

namespace OHOS {
namespace DistributedHardware {

class Session {
public:
    Session(int sessionId, std::string deviceId);
    int sessionId_;
    std::string deviceId_;
    std::string version_{""};
    std::atomic<bool> flag_{false}; // Only one session is allowed
    std::set<uint64_t> logicalSessionSet_;  // Logical Session Set
    std::atomic<int> logicalSessionCnt_{0};
};

struct Config {
    std::string pkgName;
    std::string authCode;
    int32_t authenticationType{0};
};

typedef struct DmBindCallerInfo {
    int32_t userId = -1;
    int32_t tokenId = -1;
    int32_t bindLevel = -1;
    bool isSystemSA = false;
    std::string bundleName = "";
    std::string hostPkgLabel = "";
    std::string processName = "";
} DmBindCallerInfo;

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
    uint64_t GetTokenIdByNameAndDeviceId(std::string extra, std::string requestDeviceId);
    void ScreenCommonEventCallback(std::string commonEventType);
    bool CheckIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid);
    bool CheckAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid);
    void HandleDeviceNotTrust(const std::string &udid);
    int32_t GetBindLevel(const std::string &pkgName, const std::string &localUdid,
        const std::string &udid, uint64_t &tokenId);
    void HandleIdentAccountLogout(const DMAclQuadInfo &info, const std::string &accountId);
    void HandleDeviceScreenStatusChange(DmDeviceInfo &devInfo);
    int32_t StopAuthenticateDevice(const std::string &pkgName);
    void HandleCredentialAuthStatus(const std::string &deviceList, uint16_t deviceTypeId, int32_t errcode);
    int32_t SyncLocalAclListProcess(const DevUserInfo &localDevUserInfo,
        const DevUserInfo &remoteDevUserInfo, std::string remoteAclList);
    int32_t GetAclListHash(const DevUserInfo &localDevUserInfo,
        const DevUserInfo &remoteDevUserInfo, std::string &aclList);
    int32_t ProcessAppUnintall(const std::string &appId, int32_t accessTokenId);
    int32_t ProcessAppUninstall(int32_t userId, int32_t accessTokenId);
    void ProcessUnBindApp(int32_t userId, int32_t accessTokenId, const std::string &extra, const std::string &udid);
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
    void CheckDeleteCredential(const std::string &remoteUdid, int32_t remoteUserId);
    void HandleCredentialDeleted(const char *credId, const char *credInfo, const std::string &localUdid,
        std::string &remoteUdid);
    void HandleShareUnbindBroadCast(const std::string &credId, const int32_t &userId, const std::string &localUdid);
    int32_t CheckDeviceInfoPermission(const std::string &localUdid, const std::string &peerDeviceId);
    int32_t DeleteAcl(const std::string &sessionName, const std::string &localUdid, const std::string &remoteUdid,
        int32_t bindLevel, const std::string &extra);
    bool IsProxyUnbind(const std::string &extra);
    int32_t DeleteAclV2(const std::string &sessionName, const std::string &localUdid, const std::string &remoteUdid,
        int32_t bindLevel, const std::string &extra);
    void NotifyCleanEvent(uint64_t logicalSessionId);
    void HandleServiceUnBindEvent(int32_t userId, const std::string &remoteUdid,
        int32_t remoteTokenId);
    int32_t DeleteGroup(const std::string &pkgName, const std::string &deviceId);
    int32_t InitAndRegisterAuthMgr(bool isSrcSide, uint64_t tokenId, std::shared_ptr<Session> session,
        uint64_t logicalSessionId, const std::string &pkgName);
    void HandleCommonEventBroadCast(const std::vector<uint32_t> &foregroundUserIds,
        const std::vector<uint32_t> &backgroundUserIds, const std::string &remoteUdid);
    std::vector<std::string> GetDeviceIdByUserIdAndTokenId(int32_t userId, int32_t tokenId);
    bool CheckSrcAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid);
    bool CheckSinkAccessControl(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid);
    bool CheckSrcIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid);
    bool CheckSinkIsSameAccount(const DmAccessCaller &caller, const std::string &srcUdid,
        const DmAccessCallee &callee, const std::string &sinkUdid);
    void DeleteHoDevice(const std::string &peerUdid, const std::vector<int32_t> &foreGroundUserIds,
        const std::vector<int32_t> &backGroundUserIds);
private:
    int32_t PraseNotifyEventJson(const std::string &event, JsonObject &jsonObject);
    std::string GetUdidHashByNetworkId(const std::string &networkId);
    void SetOnlineProcessInfo(const uint32_t &bindType, ProcessInfo &processInfo, DmDeviceInfo &devInfo,
        const std::string &requestDeviceId, const std::string &trustDeviceId, DmDeviceState devState);
    void HandleDeletedAclOffline(const std::string &trustDeviceId,
        const std::string &requestDeviceId, DmDeviceInfo &devInfo, ProcessInfo &processInfo, DmDeviceState &devState);
    void HandleOffline(DmDeviceState devState, DmDeviceInfo &devInfo);
    void HandleOnline(DmDeviceState devState, DmDeviceInfo &devInfo);
    bool CheckSharePeerSrc(const std::string &peerUdid, const std::string &localUdid);
    std::map<std::string, int32_t> GetDeviceIdAndBindLevel(int32_t userId);
    std::multimap<std::string, int32_t> GetDeviceIdAndUserId(int32_t userId, const std::string &accountId);
    void HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
        const std::string &remoteUdid);
    void HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid);
    void HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid, int32_t tokenId);
    void HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
        int32_t tokenId, int32_t peerTokenId);
    void HandleUserRemoved(std::vector<std::string> peerUdids, int32_t preUserId);
    void HandleRemoteUserRemoved(int32_t preUserId, const std::string &remoteUdid);
    DmAuthForm ConvertBindTypeToAuthForm(int32_t bindType);
    std::shared_ptr<AuthManagerBase> GetAuthMgr();
    std::shared_ptr<AuthManagerBase> GetAuthMgrByTokenId(uint64_t tokenId);
    std::shared_ptr<AuthManagerBase> GetCurrentAuthMgr();
    void CreateGlobalClassicalAuthMgr();
    std::shared_ptr<Session> GetCurSession(int sessionId);
    std::shared_ptr<Session> GetOrCreateSession(const std::string& deviceId,
        const std::map<std::string, std::string> &bindParam);
    int32_t ParseConnectAddr(const PeerTargetId &targetId, std::string &deviceId,
        const std::map<std::string, std::string> &bindParam);
    std::shared_ptr<Config> GetConfigByTokenId();
    int OpenAuthSession(const std::string& deviceId, const std::map<std::string, std::string> &bindParam);
    int32_t ChangeUltrasonicTypeToPin(std::map<std::string, std::string> &bindParam);
    int32_t TransferByAuthType(int32_t authType,
        std::shared_ptr<Session> curSession, std::shared_ptr<AuthManagerBase> authMgr,
        std::map<std::string, std::string> &bindParam, uint64_t logicalSessionId);

    std::shared_ptr<AuthManagerBase> GetAuthMgrByMessage(int32_t msgType, uint64_t logicalSessionId,
        const JsonObject &jsonObject, std::shared_ptr<Session> curSession);
    int32_t TransferOldAuthMgr(int32_t msgType, const JsonObject &jsonObject,
        std::shared_ptr<Session> curSession);
    int32_t TransferSrcOldAuthMgr(std::shared_ptr<Session> curSession);
    int32_t TransferSinkOldAuthMgr(const JsonObject &jsonObject, std::shared_ptr<Session> curSession);
    int32_t GetDeviceInfo(const PeerTargetId &targetId, std::string &addrType, std::string &deviceId,
        std::shared_ptr<DeviceInfo> deviceInfo, int32_t &index);
    bool IsAuthNewVersion(int32_t bindLevel, std::string localUdid, std::string remoteUdid,
        int32_t tokenId, int32_t userId);
    void ImportConfig(std::shared_ptr<AuthManagerBase> authMgr, uint64_t tokenId, const std::string &pkgName);
    void ImportAuthCodeToConfig(std::shared_ptr<AuthManagerBase> authMgr, uint64_t tokenId);

    void CleanAuthMgrByLogicalSessionId(uint64_t logicalSessionId);
    void CleanSessionMap(int sessionId, std::shared_ptr<Session> session);
    void CleanSessionMapByLogicalSessionId(uint64_t logicalSessionId);
    int32_t DeleteAclForProcV2(const std::string &localUdid, uint32_t localTokenId, const std::string &remoteUdid,
        int32_t bindLevel, const std::string &extra, int32_t userId);
    int32_t DeleteSkCredAndAcl(const std::vector<DmAclIdParam> &acls);
    void DeleteCredential(DmAclIdParam &acl);
    void DeleteAclByTokenId(const int32_t accessTokenId,
        std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles,
        std::map<int64_t, DistributedDeviceProfile::AccessControlProfile> &delProfileMap,
        std::vector<std::pair<int32_t, std::string>> &delACLInfoVec, std::vector<int32_t> &userIdVec);

    void GetDelACLInfoVec(const int32_t &accessTokenId,
        std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles,
        std::vector<std::pair<int32_t, std::string>> &delACLInfoVec, std::vector<int32_t> &userIdVec,
        const uint32_t &userId);

    void DeleteAclByDelProfileMap(
        const std::map<int64_t, DistributedDeviceProfile::AccessControlProfile> &delProfileMap,
        std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles, const std::string &localUdid);

    bool CheckLnnAcl(DistributedDeviceProfile::AccessControlProfile delProfile,
        DistributedDeviceProfile::AccessControlProfile lastprofile);
    void CheckIsLastLnnAcl(DistributedDeviceProfile::AccessControlProfile profile,
        DistributedDeviceProfile::AccessControlProfile delProfile, DmOfflineParam &lnnAclParam,
        bool &isLastLnnAcl, const std::string &localUdid);
    void BindTargetImpl(uint64_t tokenId, const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam);
    void GetBindCallerInfo(DmBindCallerInfo &bindCallerInfo, const std::string &pkgName);
    void SetBindCallerInfoToBindParam(const std::map<std::string, std::string> &bindParam,
        std::map<std::string, std::string> &bindParamTmp, const DmBindCallerInfo &bindCallerInfo);
    std::string GetBundleLable(const std::string &bundleName);
    int32_t GetLogicalIdAndTokenIdBySessionId(uint64_t &logicalSessionId, uint64_t &tokenId, int32_t sessionId);
    void SaveTokenIdAndSessionId(uint64_t &tokenId, int32_t &sessionId, uint64_t &logicalSessionId);
    void ReleaseMaps();
    int32_t InitNewProtocolAuthMgr(bool isSrcSide, uint64_t tokenId, uint64_t logicalSessionId,
        const std::string &pkgName);
    int32_t InitOldProtocolAuthMgr(uint64_t tokenId, const std::string &pkgName);
    bool ParseConnectAddrAndSetProcessInfo(const PeerTargetId &targetId,
        PeerTargetId &targetIdTmp, const std::map<std::string, std::string> &bindParam, ProcessInfo &processInfo,
        const std::string &pkgName);
    void OnAuthResultAndOnBindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId,
        const std::string &deviceId, int32_t reason);
    void GetBundleName(const DMAclQuadInfo &info, std::set<std::string> &pkgNameSet);
private:
    std::shared_ptr<AuthManagerBase> authMgr_;     // Old protocol only

    std::shared_ptr<HiChainConnector> hiChainConnector_;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector_;
    std::shared_ptr<DmDeviceStateManager> deviceStateMgr_;
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<DmAbilityManager> abilityMgr_;
    std::shared_ptr<MineHiChainConnector> mineHiChainConnector_;
    std::shared_ptr<DmCredentialManager> credentialMgr_;
    std::shared_ptr<DmCommonEventManager> commonEventManager_;
    std::shared_ptr<IDeviceManagerServiceListener> listener_;
    std::atomic<bool> isCredentialType_ = false;
    sptr<DpInitedCallback> dpInitedCallback_ = nullptr;

    // The session ID corresponding to the device ID, used only on the src side
    std::map<std::string, int> deviceId2SessionIdMap_;
    std::map<int, std::shared_ptr<Session>> sessionsMap_;  // sessionId corresponds to the session object
    std::map<std::string, std::mutex> deviceIdMutexMap_;  // Lock corresponding to the device ID
    std::mutex mapMutex_;  // sessionsMap_ lock
    std::map<int, std::condition_variable> sessionEnableCvMap_;  // Condition variable corresponding to the session
    std::map<int, std::mutex> sessionEnableMutexMap_;      // Lock corresponding to the session
    std::map<int, bool> sessionEnableCvReadyMap_;  // Condition variable ready flag
    std::mutex logicalSessionId2TokenIdMapMtx_;
    std::map<uint64_t, uint64_t> logicalSessionId2TokenIdMap_;  // The relationship between logicalSessionId and tokenId
    std::mutex logicalSessionId2SessionIdMapMtx_;
    std::map<uint64_t, int> logicalSessionId2SessionIdMap_;  // The relationship logicalSessionId and physical sessionId
    std::mutex configsMapMutex_;
    std::map<uint64_t, std::shared_ptr<Config>> configsMap_;    // Import when authMgr is not initialized
    std::mutex authMgrMapMtx_;
    std::map<uint64_t, std::shared_ptr<AuthManagerBase>> authMgrMap_;  // New protocol sharing
};

using CreateDMServiceFuncPtr = IDeviceManagerServiceImpl *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_IMPL_H
