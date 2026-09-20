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


#ifndef DM_DEVICEPROFILE_CONNECTOR_H
#define DM_DEVICEPROFILE_CONNECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_container.h"
#include "dm_device_info_c.h"
#include "dm_single_instance.h"
#include "dm_anonymous.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

typedef enum DmAllowAuthType {
    DM_ALLOW_AUTH_ONCE = 1,
    DM_ALLOW_AUTH_ALWAYS = 2
} DmAllowAuthType;

DM_EXPORT extern const uint32_t DM_INVALIED_TYPE;
DM_EXPORT extern const uint32_t DM_APP_PEER_TO_PEER_TYPE;
DM_EXPORT extern const uint32_t DM_APP_ACROSS_ACCOUNT_TYPE;
DM_EXPORT extern const uint32_t DM_SHARE_TYPE;
DM_EXPORT extern const uint32_t DM_DEVICE_PEER_TO_PEER_TYPE;
DM_EXPORT extern const uint32_t DM_DEVICE_ACROSS_ACCOUNT_TYPE;
DM_EXPORT extern const uint32_t DM_IDENTICAL_ACCOUNT_TYPE;
DM_EXPORT extern const uint32_t DM_SERVICE_PEER_TO_PEER_TYPE;
DM_EXPORT extern const uint32_t DM_SERVICE_ACROSS_ACCOUNT_TYPE;

DM_EXPORT extern const uint32_t DM_DM_INVALIED_TYPE;
DM_EXPORT extern const uint32_t DM_BIND_LEVEL_USER;
DM_EXPORT extern const uint32_t DM_SERVICE_CONST;
DM_EXPORT extern const uint32_t DM_APP_CONST;

DM_EXPORT extern const char* DM_TAG_PEER_BUNDLE_NAME;
DM_EXPORT extern const char* DM_TAG_PEER_TOKENID;

#define DM_IDENTICAL_ACCOUNT_VAL 1
#define DM_SHARE_VAL 2
#define DM_LNN_VAL 3
#define DM_POINT_TO_POINT_VAL 256
#define DM_ACROSS_ACCOUNT_VAL 1282
#define DM_VERSION_INT_5_1_0 510

typedef enum DmProfileState {
    DM_INACTIVE = 0,
    DM_ACTIVE = 1
} DmProfileState;

typedef struct {
    DmString pkgname;
    DmString localDeviceId;
    int32_t userId;
    DmString remoteDeviceIdHash;
} DmDiscoveryInfo;

void DmDiscoveryInfoInit(DmDiscoveryInfo* info);
void DmDiscoveryInfoDestroy(DmDiscoveryInfo* info);

typedef struct {
    DmString sessionKey;
    int32_t bindType;
    int32_t state;
    DmString trustDeviceId;
    int32_t bindLevel;
    int32_t authenticationType;
    DmString deviceIdHash;
    DmString extraData;
} DmAclInfo;

void DmAclInfoInit(DmAclInfo* info);
void DmAclInfoDestroy(DmAclInfo* info);

typedef struct {
    uint64_t requestTokenId;
    DmString requestBundleName;
    int32_t requestUserId;
    DmString requestAccountId;
    DmString requestDeviceId;
    int32_t requestTargetClass;
    DmString requestDeviceName;
    DmString requestCredentialId;
    int64_t requestSkTimeStamp;
    DmString requestExtraData;
} DmAccesser;

void DmAccesserStructInit(DmAccesser* info);
void DmAccesserStructDestroy(DmAccesser* info);

void DmAccesserSetDeviceId(DmAccesser* info, const char* value);
void DmAccesserSetUserId(DmAccesser* info, int32_t value);
void DmAccesserSetAccountId(DmAccesser* info, const char* value);
void DmAccesserSetTokenId(DmAccesser* info, uint64_t value);
void DmAccesserSetBundleName(DmAccesser* info, const char* value);
void DmAccesserSetDeviceName(DmAccesser* info, const char* value);
void DmAccesserSetCredentialIdStr(DmAccesser* info, const char* value);
void DmAccesserSetSkTimeStamp(DmAccesser* info, int64_t value);
void DmAccesserSetExtraData(DmAccesser* info, const char* value);

typedef struct {
    uint64_t trustTokenId;
    DmString trustBundleName;
    int32_t trustUserId;
    DmString trustAccountId;
    DmString trustDeviceId;
    int32_t trustTargetClass;
    DmString trustDeviceName;
    DmString trustCredentialId;
    int64_t trustSkTimeStamp;
    DmString trustExtraData;
} DmAccessee;

void DmAccesseeStructInit(DmAccessee* info);
void DmAccesseeStructDestroy(DmAccessee* info);

void DmAccesseeSetDeviceId(DmAccessee* info, const char* value);
void DmAccesseeSetUserId(DmAccessee* info, int32_t value);
void DmAccesseeSetAccountId(DmAccessee* info, const char* value);
void DmAccesseeSetTokenId(DmAccessee* info, uint64_t value);
void DmAccesseeSetBundleName(DmAccessee* info, const char* value);
void DmAccesseeSetDeviceName(DmAccessee* info, const char* value);
void DmAccesseeSetCredentialIdStr(DmAccessee* info, const char* value);
void DmAccesseeSetSkTimeStamp(DmAccessee* info, int64_t value);
void DmAccesseeSetExtraData(DmAccessee* info, const char* value);

typedef struct {
    DmString udid;
    int32_t userId;
    DmString credId;
    DmString pkgName;
    DmVecInt64_t tokenIds;
} DmAclIdParam;

void DmAclIdParamInit(DmAclIdParam* param);
void DmAclIdParamDestroy(DmAclIdParam* param);

DM_VEC_DEFINE(DmAclIdParam);
DM_VEC_DEFINE(DmUserRemovedServiceInfo);

typedef struct {
    uint32_t bindType;
    DmVec_ProcessInfo processVec;
    DmVec_DmString credIdVec;
    int32_t leftAclNumber;
    int32_t peerUserId;
    bool hasLnnAcl;
    bool hasUserAcl;
    bool isNewVersion;
    DmVec_DmAclIdParam needDelAclInfos;
    DmVec_DmAclIdParam allLnnAclInfos;
    DmVec_DmAclIdParam allLeftAppOrSvrAclInfos;
    DmVec_DmAclIdParam allUserAclInfos;
} DmOfflineParam;

void DmOfflineParamInit(DmOfflineParam* param);
void DmOfflineParamDestroy(DmOfflineParam* param);

DM_VEC_DEFINE(DmOfflineParam);

typedef struct {
    DmString localUdid;
    int32_t preUserId;
    DmVec_DmString peerUdids;
} DmLocalUserRemovedInfo;

void DmLocalUserRemovedInfoInit(DmLocalUserRemovedInfo* info);
void DmLocalUserRemovedInfoDestroy(DmLocalUserRemovedInfo* info);

typedef struct {
    DmString peerUdid;
    int32_t peerUserId;
    DmVec_int localUserIds;
} DmRemoteUserRemovedInfo;

void DmRemoteUserRemovedInfoInit(DmRemoteUserRemovedInfo* info);
void DmRemoteUserRemovedInfoDestroy(DmRemoteUserRemovedInfo* info);

typedef struct {
    DmString version;
    DmVec_DmString aclHashList;
} DmAclHashItem;

void DmAclHashItemInit(DmAclHashItem* item);
void DmAclHashItemDestroy(DmAclHashItem* item);

DM_VEC_DEFINE(DmAclHashItem);

typedef struct {
    DmString peerUdid;
    int32_t peerUserId;
    int32_t localUserId;
} DmAuthOnceAclInfo;

void DmAuthOnceAclInfoInit(DmAuthOnceAclInfo* info);
void DmAuthOnceAclInfoDestroy(DmAuthOnceAclInfo* info);

DM_HMAP_DEFINE(DmAuthOnceAclInfo_int, DmAuthOnceAclInfo, int);
DM_SET_DEFINE(DmAuthOnceAclInfo);
DM_HMAP_DEFINE(DmString_DmAuthForm, DmString, DmAuthForm);
DM_MAP_DEFINE(DmString_DmOfflineParam, DmString, DmOfflineParam);

typedef struct DmDeviceProfileConnector DmDeviceProfileConnector;

DM_DECLARE_SINGLE_INSTANCE(DmDeviceProfileConnector);

DM_EXPORT DmOfflineParam DmDpConnectorFilterNeedDeleteAcl(const char* localDeviceId, uint32_t localTokenId,
    const char* remoteDeviceId, const char* extra);
DM_EXPORT DmOfflineParam DmDpConnectorFilterNeedDeleteAclByPeer(const char* peerUdid);
DM_EXPORT int32_t DmDpConnectorGetAccessControlProfile(DmVecVoid* profiles);
DM_EXPORT DmOfflineParam DmDpConnectorHandleServiceUnbindEvent(int32_t remoteUserId,
    const char* remoteUdid, const char* localUdid, int32_t tokenId);
DM_EXPORT int32_t DmDpConnectorGetAclProfileByUserId(DmVecVoid* profiles, int32_t userId);
DM_EXPORT int32_t DmDpConnectorGetAclProfileByDeviceIdAndUserId(DmVecVoid* profiles,
    const char* deviceId, int32_t userId);
DM_EXPORT uint32_t DmDpConnectorCheckBindType(const char* peerUdid, const char* localUdid);
DM_EXPORT int32_t DmDpConnectorPutAccessControlList(DmAclInfo aclInfo, DmAccesser dmAccesser,
    DmAccessee dmAccessee);
DM_EXPORT int32_t DmDpConnectorUpdateAccessControlList(int32_t userId, DmString* oldAccountId,
    DmString* newAccountId);
DM_EXPORT void DmDpConnectorUpdateAclStatus(void* profilePtr);
DM_EXPORT int32_t DmDpConnectorGetAppTrustDeviceList(DmHmap_DmString_DmAuthForm* resultMap,
    const char* pkgName, const char* deviceId);
DM_EXPORT DmVec_int DmDpConnectorGetBindTypeByPkgName(const char* pkgName,
    const char* requestDeviceId, const char* trustUdid);
DM_EXPORT uint64_t DmDpConnectorGetTokenIdByNameAndDeviceId(const char* extra,
    const char* requestDeviceId);
DM_EXPORT DmVec_int DmDpConnectorSyncAclByBindType(const char* pkgName,
    DmVec_int* bindTypeVec, const char* localDeviceId, const char* targetDeviceId);
DM_EXPORT int32_t DmDpConnectorGetDeviceAclParam(DmDiscoveryInfo discoveryInfo, bool* isOnline,
    int32_t* authForm);

DM_EXPORT bool DmDpConnectorDeleteAclForAccountLogOut(const DMAclQuadInfo* info,
    const char* accountId, DmOfflineParam* offlineParam, DmVec_DmUserRemovedServiceInfo* serviceInfos);
DM_EXPORT bool DmDpConnectorDeleteAclByActHash(const DMAclQuadInfo* info,
    const char* accountIdHash, DmOfflineParam* offlineParam, DmVec_DmUserRemovedServiceInfo* serviceInfos);
DM_EXPORT void DmDpConnectorDeleteAclForUserRemoved(const DmLocalUserRemovedInfo* userRemovedInfo,
    DmMmapDmStringInt* peerUserIdMap, DmOfflineParam* offlineParam,
    DmVec_DmUserRemovedServiceInfo* serviceInfos);
DM_EXPORT void DmDpConnectorDeleteAclForRemoteUserRemoved(DmRemoteUserRemovedInfo* userRemovedInfo,
    DmOfflineParam* offlineParam, DmVec_DmUserRemovedServiceInfo* serviceInfos);
DM_EXPORT DmOfflineParam DmDpConnectorDeleteAccessControlList(const char* pkgName,
    const char* localDeviceId, const char* remoteDeviceId, int32_t bindLevel, const char* extra);
DM_EXPORT int32_t DmDpConnectorGetProcessInfoFromAclByUserId(DmVec_ProcessInfo* resultVec,
    const char* localDeviceId, const char* targetDeviceId, int32_t userId);
DM_EXPORT void DmDpConnectorDeleteAccessControlListByUdid(const char* udid);
DM_EXPORT DmVec_DmPair_int64_t_int64_t DmDpConnectorGetAgentToProxyVecFromAclByUserId(
    const char* localDeviceId, const char* targetDeviceId, int32_t userId);
DM_EXPORT bool DmDpConnectorCheckSrcDevIdInAclForDevBind(const char* pkgName,
    const char* deviceId);
DM_EXPORT bool DmDpConnectorCheckSinkDevIdInAclForDevBind(const char* pkgName,
    const char* deviceId);
DM_EXPORT bool DmDpConnectorCheckDevIdInAclForDevBind(const char* pkgName,
    const char* deviceId);
DM_EXPORT uint32_t DmDpConnectorDeleteTimeOutAcl(const char* peerUdid, int32_t peerUserId,
    int32_t localUserId, DmOfflineParam* offlineParam);
DM_EXPORT int32_t DmDpConnectorGetTrustNumber(const char* deviceId);
DM_EXPORT int32_t DmDpConnectorIsSameAccount(const char* udid);
DM_EXPORT bool DmDpConnectorCheckAccessControl(const DmAccessCaller* caller,
    const char* srcUdid, const DmAccessCallee* callee, const char* sinkUdid);
DM_EXPORT bool DmDpConnectorCheckIsSameAccount(const DmAccessCaller* caller,
    const char* srcUdid, const DmAccessCallee* callee, const char* sinkUdid);
DM_EXPORT int32_t DmDpConnectorGetBindLevel(const char* pkgName,
    const char* localUdid, const char* udid, uint64_t* tokenId);
DM_EXPORT DmMap_DmString_int DmDpConnectorGetDeviceIdAndBindLevel(DmVec_int* userIds,
    const char* localUdid);
DM_EXPORT DmVec_DmString DmDpConnectorGetDeviceIdAndUdidListByTokenId(DmVec_int* userIds,
    const char* localUdid, int32_t tokenId);
DM_EXPORT DmMmapDmStringInt DmDpConnectorGetDeviceIdAndUserIdByAccount(
    int32_t userId, const char* accountId, const char* localUdid);
DM_EXPORT int32_t DmDpConnectorHandleDevUnbindEvent(int32_t remoteUserId, const char* remoteUdid,
    const char* localUdid, DmOfflineParam* offlineParam, int32_t tokenId);
DM_EXPORT DmOfflineParam DmDpConnectorHandleAppUnbindEvent(int32_t remoteUserId,
    const char* remoteUdid, int32_t tokenId, const char* localUdid);
DM_EXPORT DmOfflineParam DmDpConnectorHandleAppUnbindEventWithPeer(int32_t remoteUserId,
    const char* remoteUdid, int32_t tokenId, const char* localUdid, int32_t peerTokenId);
DM_EXPORT void DmDpConnectorHandleDeviceUnBind(int32_t bindType, const char* peerUdid,
    const char* localUdid, int32_t localUserId, const char* localAccountId);
DM_EXPORT void DmDpConnectorGetRemoteTokenIds(const char* localUdid, const char* udid,
    DmVecInt64_t* remoteTokenIds);
DM_EXPORT int32_t DmDpConnectorGetAllAccessControlProfile(DmVecVoid* profiles);
DM_EXPORT void DmDpConnectorDeleteAccessControlById(const char* trustDeviceId);
DM_EXPORT int32_t DmDpConnectorHandleUserSwitched(const char* localUdid,
    DmVec_DmString* deviceVec, int32_t currentUserId, int32_t beforeUserId);
DM_EXPORT int32_t DmDpConnectorHandleUserSwitchedWithIds(const char* localUdid,
    DmVec_DmString* deviceVec, DmVec_int* foregroundUserIds, DmVec_int* backgroundUserIds);
DM_EXPORT bool DmDpConnectorCheckAclStatusAndForegroundNotMatch(const char* localUdid,
    DmVec_int* foregroundUserIds, DmVec_int* backgroundUserIds);
DM_EXPORT void DmDpConnectorHandleUserSwitchedProfiles(DmVecVoid* activeProfiles,
    DmVecVoid* inActiveProfiles);
DM_EXPORT void DmDpConnectorHandleSyncForegroundUserIdEvent(DmVec_int* remoteUserIds,
    const char* remoteUdid, DmVec_int* localUserIds, DmString* localUdid,
    DmVec_DmUserRemovedServiceInfo* serviceInfos);
DM_EXPORT DmVec_ProcessInfo DmDpConnectorGetOfflineProcessInfo(DmString* localUdid,
    DmVec_int* localUserIds, const char* remoteUdid, DmVec_int* remoteUserIds);
DM_EXPORT DmMap_int_int DmDpConnectorGetUserIdAndBindLevel(const char* localUdid,
    const char* peerUdid);
DM_EXPORT DmMmapDmStringInt DmDpConnectorGetDevIdAndUserIdByActHash(
    const char* localUdid, const char* peerUdid, int32_t peerUserId, const char* peerAccountHash);
DM_EXPORT DmMmapDmStringInt DmDpConnectorGetDeviceIdAndUserIdByLocal(
    const char* localUdid, int32_t localUserId);
DM_EXPORT void DmDpConnectorHandleSyncBackgroundUserIdEvent(DmVec_int* remoteUserIds,
    const char* remoteUdid, DmVec_int* localUserIds, DmString* localUdid,
    DmVec_DmUserRemovedServiceInfo* serviceInfos);
DM_EXPORT int32_t DmDpConnectorSubscribeDeviceProfileInited(void* dpInitedCallback);
DM_EXPORT int32_t DmDpConnectorUnSubscribeDeviceProfileInited(void);
DM_EXPORT int32_t DmDpConnectorPutAllTrustedDevices(DmVecVoid* deviceInfos);
DM_EXPORT int32_t DmDpConnectorCheckDeviceInfoPermission(const char* localUdid,
    const char* peerDeviceId);
DM_EXPORT int32_t DmDpConnectorUpdateAclDeviceName(const char* udid,
    const char* newDeviceName, bool isLocal);
DM_EXPORT int32_t DmDpConnectorPutLocalServiceInfo(void* localServiceInfo);
DM_EXPORT int32_t DmDpConnectorDeleteLocalServiceInfo(const char* bundleName, int32_t pinExchangeType);
DM_EXPORT int32_t DmDpConnectorUpdateLocalServiceInfo(void* localServiceInfo);
DM_EXPORT int32_t DmDpConnectorGetLocalServiceInfoByBundleNameAndPinExchangeType(
    const char* bundleName, int32_t pinExchangeType, void* localServiceInfo);
DM_EXPORT int32_t DmDpConnectorHandleUserStop(int32_t stopUserId, const char* stopEventUdid);
DM_EXPORT int32_t DmDpConnectorHandleUserStopWithUdids(int32_t stopUserId, const char* localUdid,
    DmVec_DmString* acceptEventUdids);
DM_EXPORT DmString DmDpConnectorIsAuthNewVersion(int32_t bindLevel, const char* localUdid,
    const char* remoteUdid, int32_t tokenId, int32_t userId);
DM_EXPORT int32_t DmDpConnectorGetAclList(DmVecVoid* profiles, const char* localUdid,
    int32_t localUserId, const char* remoteUdid, int32_t remoteUserId);
DM_EXPORT bool DmDpConnectorChecksumAcl(void* acl, DmVec_DmString* aclStrList);
DM_EXPORT DmString DmDpConnectorAccessToStr(void* acl);
DM_EXPORT int32_t DmDpConnectorGetVersionByExtra(DmString* extraInfo, DmString* dmVersion);
DM_EXPORT void DmDpConnectorGetAllVersionAclMap(void* acl, DmMap_DmString_DmVec_DmString* aclMap,
    const char* dmVersion);
DM_EXPORT int32_t DmDpConnectorGetAclListHashStr(const DevUserInfo* localDevUserInfo,
    const DevUserInfo* remoteDevUserInfo, DmString* aclListHash, const char* dmVersion);
DM_EXPORT bool DmDpConnectorIsLnnAcl(void* profile);
DM_EXPORT void DmDpConnectorCacheAcerAclId(void* profile, DmVec_DmAclIdParam* aclInfos);
DM_EXPORT void DmDpConnectorCacheAceeAclId(void* profile, DmVec_DmAclIdParam* aclInfos);
DM_EXPORT int32_t DmDpConnectorHandleAccountCommonEvent(const char* localUdid,
    DmVec_DmString* deviceVec, DmVec_int* foregroundUserIds, DmVec_int* backgroundUserIds,
    DmVec_DmUserRemovedServiceInfo* serviceInfos);
DM_EXPORT bool DmDpConnectorCheckSrcAccessControl(const DmAccessCaller* caller,
    const char* srcUdid, const DmAccessCallee* callee, const char* sinkUdid);
DM_EXPORT bool DmDpConnectorCheckSinkAccessControl(const DmAccessCaller* caller,
    const char* srcUdid, const DmAccessCallee* callee, const char* sinkUdid);
DM_EXPORT bool DmDpConnectorCheckSrcIsSameAccount(const DmAccessCaller* caller,
    const char* srcUdid, const DmAccessCallee* callee, const char* sinkUdid);
DM_EXPORT bool DmDpConnectorCheckSinkIsSameAccount(const DmAccessCaller* caller,
    const char* srcUdid, const DmAccessCallee* callee, const char* sinkUdid);
DM_EXPORT void DmDpConnectorDeleteHoDevice(const char* peerUdid, DmVec_int* foreGroundUserIds,
    DmVec_int* backGroundUserIds);
DM_EXPORT bool DmDpConnectorIsAllowAuthAlways(const char* localUdid, int32_t userId,
    const char* peerUdid, const char* pkgName, int64_t tokenId);
DM_EXPORT int32_t DmDpConnectorGetAllAuthOnceAclInfos(DmHmap_DmAuthOnceAclInfo_int* aclInfos);
DM_EXPORT void DmDpConnectorDeleteDpInvalidAcl(void);
DM_EXPORT bool DmDpConnectorCheckUserIdIsForegroundUserId(int32_t userId);
DM_EXPORT DmSet_int DmDpConnectorGetActiveAuthOncePeerUserId(const char* peerUdid,
    int32_t localUserId);
DM_EXPORT DmSet_DmAuthOnceAclInfo DmDpConnectorGetAuthOnceAclInfos(const char* peerUdid);
DM_EXPORT bool DmDpConnectorAuthOnceAclIsActive(const char* peerUdid, int32_t peerUserId,
    int32_t localUserId);
DM_EXPORT bool DmDpConnectorCheckAccessControlProfileByTokenId(int32_t tokenId);
DM_EXPORT int32_t DmDpConnectorGetServiceInfosByUdid(DmVecVoid* serviceInfos, const char* udid);
DM_EXPORT int32_t DmDpConnectorGetServiceInfosByUdidAndUserId(DmVecVoid* serviceInfos,
    const char* udid, int32_t userId);
DM_EXPORT int32_t DmDpConnectorGetServiceInfoByUdidAndServiceId(void* serviceInfo,
    const char* udid, int64_t serviceId);
DM_EXPORT int32_t DmDpConnectorPutServiceInfo(void* serviceInfo);
DM_EXPORT int32_t DmDpConnectorDeleteServiceInfo(void* serviceInfo);
DM_EXPORT void DmDpConnectorGetPeerTokenIdForServiceProxyUnbind(int32_t userId,
    uint64_t localTokenId, const char* peerUdid, int64_t serviceId, DmVec_uint64_t* peerTokenId);
DM_EXPORT void DmDpConnectorGetInvalidSkIdAcl(DmMap_DmString_DmOfflineParam* invalidAclMap);
DM_EXPORT int32_t DmDpConnectorCheckIsSameAccountByUdidHash(const char* udidHash);

DM_EXPORT extern DmDeviceProfileConnector* DmDpConnectorCreateInstance(void);
typedef DmDeviceProfileConnector* (*DmCreateDpConnectorFuncPtr)(void);

DM_EXPORT DmString DmShellProfileGetAccesserExtraData(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetAccesseeExtraData(void* profilePtr);
DM_EXPORT void DmShellProfileSetAccesseeExtraData(void* profilePtr, const char* extraData);
DM_EXPORT void DmShellProfileSetAccesserExtraData(void* profilePtr, const char* extraData);
DM_EXPORT int32_t DmDpConnectorUpdateAccessControlProfile(void* profilePtr);
DM_EXPORT int64_t DmShellProfileGetAccesserTokenId(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetAccesserBundleName(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetAccesserAccountId(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetAccesserCredentialIdStr(void* profilePtr);
DM_EXPORT int64_t DmShellProfileGetAccesseeTokenId(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetAccesseeBundleName(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetAccesseeAccountId(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetTrustDeviceId(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetAccesseeCredentialIdStr(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetAccesserDeviceId(void* profilePtr);
DM_EXPORT DmString DmShellProfileGetAccesseeDeviceId(void* profilePtr);
DM_EXPORT int32_t DmShellProfileGetAccesserUserId(void* profilePtr);
DM_EXPORT int32_t DmShellProfileGetAccesseeUserId(void* profilePtr);
DM_EXPORT int32_t DmShellProfileGetStatus(void* profilePtr);
DM_EXPORT void DmShellProfileSetStatus(void* profilePtr, int32_t status);
DM_EXPORT uint32_t DmShellProfileGetBindLevel(void* profilePtr);
DM_EXPORT int32_t DmShellProfileGetBindType(void* profilePtr);
DM_EXPORT void DmShellDeleteAccessControlProfile(const char* trustDeviceId);
DM_EXPORT DmString DmShellLocalServiceInfoGetExtraInfo(void* localServiceInfoPtr);
DM_EXPORT int32_t DmShellGetBusinessEvent(const char* businessKey, DmString* businessValue);
DM_EXPORT int32_t DmShellGetServiceInfoByUdidAndServiceId(void* serviceInfoPtr,
    const char* udid, int64_t serviceId);
DM_EXPORT int64_t DmShellServiceInfoGetServiceOwnerTokenId(void* serviceInfoPtr);
DM_EXPORT int64_t DmShellServiceInfoGetServiceRegisterTokenId(void* serviceInfoPtr);
DM_EXPORT int32_t DmShellServiceInfoGetPublishState(void* serviceInfoPtr);
DM_EXPORT int32_t DmShellServiceInfoGetUserId(void* serviceInfoPtr);
DM_EXPORT int32_t DmShellServiceInfoGetDisplayId(void* serviceInfoPtr);
DM_EXPORT DmString DmShellServiceInfoGetServiceOwnerPkgName(void* serviceInfoPtr);
DM_EXPORT int32_t DmShellGetLocalServiceInfoByBundleNameAndPinExchangeType(
    void* localServiceInfoPtr, const char* bundleName, int32_t pinExchangeType);

DM_EXPORT int32_t DmDpConnectorGetAllAclIncludeLnnAcl(DmVecVoid* profiles);
DM_EXPORT int32_t DmDpConnectorGetAclProfileByDeviceIdAndUserIdRemote(DmVecVoid* profiles,
    const char* deviceId, int32_t userId, const char* remoteDeviceId);

#ifdef __cplusplus
}
#endif

#endif
