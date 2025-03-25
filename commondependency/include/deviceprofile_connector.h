/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_DEVICEPROFILE_CONNECTOR_H
#define OHOS_DM_DEVICEPROFILE_CONNECTOR_H
#include <algorithm>
#include <string>
#include <unordered_set>
#include "access_control_profile.h"
#include "dm_device_info.h"
#include "dm_single_instance.h"
#include "i_dp_inited_callback.h"
#include "local_service_info.h"
#include "trusted_device_info.h"

enum AllowAuthType {
    ALLOW_AUTH_ONCE = 1,
    ALLOW_AUTH_ALWAYS = 2
};

__attribute__ ((visibility ("default")))extern const uint32_t INVALIED_TYPE;
__attribute__ ((visibility ("default")))extern const uint32_t APP_PEER_TO_PEER_TYPE;
__attribute__ ((visibility ("default")))extern const uint32_t APP_ACROSS_ACCOUNT_TYPE;
__attribute__ ((visibility ("default")))extern const uint32_t DEVICE_PEER_TO_PEER_TYPE;
__attribute__ ((visibility ("default")))extern const uint32_t DEVICE_ACROSS_ACCOUNT_TYPE;
__attribute__ ((visibility ("default")))extern const uint32_t IDENTICAL_ACCOUNT_TYPE;

__attribute__ ((visibility ("default")))extern const uint32_t DM_IDENTICAL_ACCOUNT;
__attribute__ ((visibility ("default")))extern const uint32_t DM_POINT_TO_POINT;
__attribute__ ((visibility ("default")))extern const uint32_t DM_ACROSS_ACCOUNT;
__attribute__ ((visibility ("default")))extern const uint32_t DM_INVALIED_BINDTYPE;
__attribute__ ((visibility ("default")))extern const uint32_t DEVICE;
__attribute__ ((visibility ("default")))extern const uint32_t SERVICE;
__attribute__ ((visibility ("default")))extern const uint32_t APP;

enum ProfileState {
    INACTIVE = 0,
    ACTIVE = 1
};

typedef struct DmDiscoveryInfo {
    std::string pkgname;
    std::string localDeviceId;
    int32_t userId;
    std::string remoteDeviceIdHash;
} DmDiscoveryInfo;

typedef struct DmAclInfo {
    std::string sessionKey;
    int32_t bindType;
    int32_t state;
    std::string trustDeviceId;
    int32_t bindLevel;
    int32_t authenticationType;
    std::string deviceIdHash;
} DmAclInfo;

typedef struct DmAccesser {
    uint64_t requestTokenId;
    std::string requestBundleName;
    int32_t requestUserId;
    std::string requestAccountId;
    std::string requestDeviceId;
    int32_t requestTargetClass;
    std::string requestDeviceName;
} DmAccesser;

typedef struct DmAccessee {
    uint64_t trustTokenId;
    std::string trustBundleName;
    int32_t trustUserId;
    std::string trustAccountId;
    std::string trustDeviceId;
    int32_t trustTargetClass;
    std::string trustDeviceName;
} DmAccessee;

typedef struct DmOfflineParam {
    uint32_t bindType;
    std::vector<OHOS::DistributedHardware::ProcessInfo> processVec;
    int32_t leftAclNumber;
} DmOfflineParam;

namespace OHOS {
namespace DistributedHardware {
class IDeviceProfileConnector {
public:
    virtual ~IDeviceProfileConnector() {}
    virtual int32_t GetDeviceAclParam(DmDiscoveryInfo discoveryInfo, bool &isOnline, int32_t &authForm) = 0;
    virtual std::map<std::string, int32_t> GetDeviceIdAndBindLevel(std::vector<int32_t> userIds,
        const std::string &localUdid) = 0;
    virtual int32_t HandleUserSwitched(const std::string &localUdid, const std::vector<std::string> &deviceVec,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds) = 0;
    virtual bool CheckAclStatusAndForegroundNotMatch(const std::string &localUdid,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds) = 0;
    virtual int32_t HandleUserStop(int32_t stopUserId, const std::string &stopEventUdid) = 0;
    virtual int32_t HandleUserStop(int32_t stopUserId, const std::string &localUdid,
        const std::vector<std::string> &acceptEventUdids) = 0;
};

class DeviceProfileConnector : public IDeviceProfileConnector {
    DM_DECLARE_SINGLE_INSTANCE(DeviceProfileConnector);
public:
    __attribute__ ((visibility ("default")))std::vector<DistributedDeviceProfile::AccessControlProfile>
        GetAccessControlProfile();
    std::vector<DistributedDeviceProfile::AccessControlProfile> GetAccessControlProfileByUserId(int32_t userId);
    std::vector<DistributedDeviceProfile::AccessControlProfile> GetAclProfileByDeviceIdAndUserId(
        const std::string &deviceId, int32_t userId);
    __attribute__ ((visibility ("default")))uint32_t CheckBindType(std::string peerUdid, std::string localUdid);
    __attribute__ ((visibility ("default")))int32_t PutAccessControlList(DmAclInfo aclInfo, DmAccesser dmAccesser,
        DmAccessee dmAccessee);
    int32_t UpdateAccessControlList(int32_t userId, std::string &oldAccountId, std::string &newAccountId);
    __attribute__ ((visibility ("default")))std::unordered_map<std::string, DmAuthForm> GetAppTrustDeviceList(
        const std::string &pkgName, const std::string &deviceId);
    __attribute__ ((visibility ("default")))std::vector<int32_t> GetBindTypeByPkgName(std::string pkgName,
        std::string requestDeviceId, std::string trustUdid);
    __attribute__ ((visibility ("default")))uint64_t GetTokenIdByNameAndDeviceId(std::string pkgName,
        std::string requestDeviceId);
    __attribute__ ((visibility ("default")))std::vector<int32_t> SyncAclByBindType(std::string pkgName,
        std::vector<int32_t> bindTypeVec, std::string localDeviceId, std::string targetDeviceId);
    int32_t GetDeviceAclParam(DmDiscoveryInfo discoveryInfo, bool &isOnline, int32_t &authForm);
    __attribute__ ((visibility ("default")))bool DeleteAclForAccountLogOut(const std::string &localUdid,
        int32_t localUserId, const std::string &peerUdid, int32_t peerUserId);
    __attribute__ ((visibility ("default")))void DeleteAclForUserRemoved(std::string localUdid, int32_t userId);
    __attribute__ ((visibility ("default")))void DeleteAclForRemoteUserRemoved(std::string peerUdid,
        int32_t peerUserId, std::vector<int32_t> &userIds);
    __attribute__ ((visibility ("default")))DmOfflineParam DeleteAccessControlList(const std::string &pkgName,
        const std::string &localDeviceId, const std::string &remoteDeviceId, int32_t bindLevel,
        const std::string &extra);
    __attribute__ ((visibility ("default")))std::vector<OHOS::DistributedHardware::ProcessInfo>
        GetProcessInfoFromAclByUserId(const std::string &localDeviceId, const std::string &targetDeviceId,
        int32_t userId);
    bool CheckIdenticalAccount(int32_t userId, const std::string &accountId);
    __attribute__ ((visibility ("default")))bool CheckSrcDevIdInAclForDevBind(const std::string &pkgName,
        const std::string &deviceId);
    __attribute__ ((visibility ("default")))bool CheckSinkDevIdInAclForDevBind(const std::string &pkgName,
        const std::string &deviceId);
    __attribute__ ((visibility ("default")))uint32_t DeleteTimeOutAcl(const std::string &deviceId);
    __attribute__ ((visibility ("default")))int32_t GetTrustNumber(const std::string &deviceId);
    bool CheckDevIdInAclForDevBind(const std::string &pkgName, const std::string &deviceId);
    std::vector<int32_t> CompareBindType(std::vector<DistributedDeviceProfile::AccessControlProfile> profiles,
        std::string pkgName, std::vector<int32_t> &sinkBindType, std::string localDeviceId, std::string targetDeviceId);
    __attribute__ ((visibility ("default")))int32_t IsSameAccount(const std::string &udid);
    __attribute__ ((visibility ("default")))int32_t CheckAccessControl(const DmAccessCaller &caller,
        const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid);
    __attribute__ ((visibility ("default")))int32_t CheckIsSameAccount(const DmAccessCaller &caller,
        const std::string &srcUdid, const DmAccessCallee &callee, const std::string &sinkUdid);
    __attribute__ ((visibility ("default")))void DeleteAccessControlList(const std::string &udid);
    __attribute__ ((visibility ("default")))int32_t GetBindLevel(const std::string &pkgName,
        const std::string &localUdid, const std::string &udid, uint64_t &tokenId);
    std::map<std::string, int32_t> GetDeviceIdAndBindLevel(std::vector<int32_t> userIds, const std::string &localUdid);
    __attribute__ ((visibility ("default")))std::multimap<std::string, int32_t> GetDeviceIdAndUserId(
        int32_t userId, const std::string &accountId, const std::string &localUdid);
    int32_t HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
        const std::string &remoteUdid, const std::string &localUdid);
    __attribute__ ((visibility ("default")))int32_t HandleDevUnBindEvent(int32_t remoteUserId,
        const std::string &remoteUdid, const std::string &localUdid);
    __attribute__ ((visibility ("default")))OHOS::DistributedHardware::ProcessInfo HandleAppUnBindEvent(
        int32_t remoteUserId, const std::string &remoteUdid, int32_t tokenId, const std::string &localUdid);
    __attribute__ ((visibility ("default")))OHOS::DistributedHardware::ProcessInfo HandleAppUnBindEvent(
        int32_t remoteUserId, const std::string &remoteUdid, int32_t tokenId, const std::string &localUdid,
        int32_t peerTokenId);
    __attribute__ ((visibility ("default")))std::vector<DistributedDeviceProfile::AccessControlProfile>
        GetAllAccessControlProfile();
    __attribute__ ((visibility ("default")))void DeleteAccessControlById(int64_t accessControlId);
    __attribute__ ((visibility ("default")))int32_t HandleUserSwitched(const std::string &localUdid,
        const std::vector<std::string> &deviceVec, int32_t currentUserId, int32_t beforeUserId);
    __attribute__ ((visibility ("default")))int32_t HandleUserSwitched(const std::string &localUdid,
        const std::vector<std::string> &deviceVec, const std::vector<int32_t> &foregroundUserIds,
        const std::vector<int32_t> &backgroundUserIds);
    bool CheckAclStatusAndForegroundNotMatch(const std::string &localUdid,
        const std::vector<int32_t> &foregroundUserIds, const std::vector<int32_t> &backgroundUserIds);
    __attribute__ ((visibility ("default")))void HandleUserSwitched(
        const std::vector<DistributedDeviceProfile::AccessControlProfile> &activeProfiles,
        const std::vector<DistributedDeviceProfile::AccessControlProfile> &inActiveProfiles,
        const std::vector<DistributedDeviceProfile::AccessControlProfile> &delActiveProfiles);
    __attribute__ ((visibility ("default")))void HandleSyncForegroundUserIdEvent(
        const std::vector<int32_t> &remoteUserIds, const std::string &remoteUdid,
        const std::vector<int32_t> &localUserIds, std::string &localUdid);
    std::vector<ProcessInfo> GetOfflineProcessInfo(std::string &localUdid, const std::vector<int32_t> &localUserIds,
        const std::string &remoteUdid, const std::vector<int32_t> &remoteUserIds);
    __attribute__ ((visibility ("default")))std::map<int32_t, int32_t> GetUserIdAndBindLevel(
        const std::string &localUdid, const std::string &peerUdid);
    __attribute__ ((visibility ("default")))void UpdateACL(std::string &localUdid,
        const std::vector<int32_t> &localUserIds, const std::string &remoteUdid,
        const std::vector<int32_t> &remoteFrontUserIds, const std::vector<int32_t> &remoteBackUserIds);
    __attribute__ ((visibility ("default")))std::multimap<std::string, int32_t> GetDevIdAndUserIdByActHash(
        const std::string &localUdid, const std::string &peerUdid, int32_t peerUserId,
        const std::string &peerAccountHash);
    __attribute__ ((visibility ("default")))std::multimap<std::string, int32_t> GetDeviceIdAndUserId(
        const std::string &localUdid, int32_t localUserId);
    __attribute__ ((visibility ("default")))void HandleSyncBackgroundUserIdEvent(
        const std::vector<int32_t> &remoteUserIds, const std::string &remoteUdid,
        const std::vector<int32_t> &localUserIds, std::string &localUdid);
    __attribute__ ((visibility ("default")))void HandleDeviceUnBind(int32_t bindType, const std::string &peerUdid,
        const std::string &localUdid, int32_t localUserId, const std::string &localAccountId);
    __attribute__ ((visibility ("default")))int32_t SubscribeDeviceProfileInited(
        sptr<DistributedDeviceProfile::IDpInitedCallback> dpInitedCallback);
    __attribute__ ((visibility ("default")))int32_t UnSubscribeDeviceProfileInited();
    __attribute__ ((visibility ("default")))int32_t PutAllTrustedDevices(
        const std::vector<DistributedDeviceProfile::TrustedDeviceInfo> &deviceInfos);
    __attribute__ ((visibility ("default")))int32_t CheckDeviceInfoPermission(const std::string &localUdid,
        const std::string &peerDeviceId);
    __attribute__ ((visibility ("default")))int32_t UpdateAclDeviceName(const std::string &udid,
        const std::string &newDeviceName);
    __attribute__ ((visibility ("default")))int32_t PutLocalServiceInfo(
        const DistributedDeviceProfile::LocalServiceInfo &localServiceInfo);
    __attribute__ ((visibility ("default")))int32_t DeleteLocalServiceInfo(const std::string &bundleName,
        int32_t pinExchangeType);
    __attribute__ ((visibility ("default")))int32_t UpdateLocalServiceInfo(
        const DistributedDeviceProfile::LocalServiceInfo &localServiceInfo);
    __attribute__ ((visibility ("default")))int32_t GetLocalServiceInfoByBundleNameAndPinExchangeType(
        const std::string &bundleName, int32_t pinExchangeType,
        DistributedDeviceProfile::LocalServiceInfo &localServiceInfo);
    __attribute__ ((visibility ("default")))int32_t PutSessionKey(const std::vector<unsigned char> &sessionKeyArray,
        int32_t &sessionKeyId);
    int32_t HandleUserStop(int32_t stopUserId, const std::string &stopEventUdid);
    int32_t HandleUserStop(int32_t stopUserId, const std::string &localUdid,
        const std::vector<std::string> &acceptEventUdids);

private:
    int32_t HandleDmAuthForm(DistributedDeviceProfile::AccessControlProfile profiles, DmDiscoveryInfo discoveryInfo);
    void GetParamBindTypeVec(DistributedDeviceProfile::AccessControlProfile profiles, std::string requestDeviceId,
        std::vector<int32_t> &bindTypeVec, std::string trustUdid);
    void ProcessBindType(DistributedDeviceProfile::AccessControlProfile profiles, std::string localDeviceId,
        std::vector<int32_t> &sinkBindType, std::vector<int32_t> &bindTypeIndex,
        uint32_t index, std::string targetDeviceId);
    bool CheckAppLevelAccess(const DistributedDeviceProfile::AccessControlProfile &profile,
        const DmAccessCaller &caller, const DmAccessCallee &callee);
    int32_t GetAuthForm(DistributedDeviceProfile::AccessControlProfile profiles, const std::string &trustDev,
        const std::string &reqDev);
    int32_t CheckAuthForm(DmAuthForm form, DistributedDeviceProfile::AccessControlProfile profiles,
        DmDiscoveryInfo discoveryInfo);
    bool SingleUserProcess(const DistributedDeviceProfile::AccessControlProfile &profile, const DmAccessCaller &caller,
        const DmAccessCallee &callee);
    void DeleteAppBindLevel(DmOfflineParam &offlineParam, const std::string &pkgName,
        const std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles, const std::string &localUdid,
        const std::string &remoteUdid);
    void DeleteAppBindLevel(DmOfflineParam &offlineParam, const std::string &pkgName,
        const std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles, const std::string &localUdid,
        const std::string &remoteUdid, const std::string &extra);
    void DeleteDeviceBindLevel(DmOfflineParam &offlineParam,
        const std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles, const std::string &localUdid,
        const std::string &remoteUdid);
    void DeleteServiceBindLevel(DmOfflineParam &offlineParam, const std::string &pkgName,
        const std::vector<DistributedDeviceProfile::AccessControlProfile> &profiles, const std::string &localUdid,
        const std::string &remoteUdid);
    void UpdateBindType(const std::string &udid, int32_t compareParam, std::map<std::string, int32_t> &deviceMap);
    std::vector<DistributedDeviceProfile::AccessControlProfile> GetAclProfileByUserId(const std::string &localUdid,
        int32_t userId, const std::string &remoteUdid);
    void DeleteSigTrustACL(DistributedDeviceProfile::AccessControlProfile profile, const std::string &remoteUdid,
        const std::vector<int32_t> &remoteFrontUserIds, const std::vector<int32_t> &remoteBackUserIds);
    void UpdatePeerUserId(DistributedDeviceProfile::AccessControlProfile profile, std::string &localUdid,
        const std::vector<int32_t> &localUserIds, const std::string &remoteUdid,
        const std::vector<int32_t> &remoteFrontUserIds);
    bool CheckAclStatusNotMatch(const DistributedDeviceProfile::AccessControlProfile &profile,
        const std::string &localUdid, const std::vector<int32_t> &foregroundUserIds,
        const std::vector<int32_t> &backgroundUserIds);
};

extern "C" IDeviceProfileConnector *CreateDpConnectorInstance();
using CreateDpConnectorFuncPtr = IDeviceProfileConnector *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICEPROFILE_CONNECTOR_H
