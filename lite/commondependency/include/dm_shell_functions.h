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


#ifndef DM_SHELL_FUNCTIONS_H
#define DM_SHELL_FUNCTIONS_H

#include "deviceprofile_connector.h"
#include "multiple_user_connector.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t DmShellGetAccessControlProfileByUserId(DmVecVoid* profiles, int32_t userId);
int32_t DmShellGetAclProfileByUserId(DmVecVoid* profiles, const char* localUdid,
    int32_t userId, const char* remoteUdid);
int32_t DmShellGetAllAccessControlProfile(DmVecVoid* profiles);
int32_t DmShellGetAllAclIncludeLnnAcl(DmVecVoid* profiles);
typedef struct DmFilterAclCtx {
    const char* localDeviceId;
    uint32_t localTokenId;
    const char* extra;
} DmFilterAclCtx;

void DmShellFilterNeedDeleteAclInfos(DmVecVoid* profiles, const char* remoteDeviceId,
    DmFilterAclCtx* ctx, DmOfflineParam* offlineParam);
void DmShellFilterNeedDeleteAclInfosByPeer(DmVecVoid* profiles, const char* localUdid,
    const char* peerUdid, DmOfflineParam* offlineParam);
uint32_t DmShellGetAuthFormPriority(void* profilePtr, const char* peerUdid, const char* localUdid);
int32_t DmShellPutAccessControlList(DmAclInfo* aclInfo, DmAccesser* dmAccesser, DmAccessee* dmAccessee);
void DmShellGetUdidHash(const char* udid, uint8_t* hash);
int32_t DmShellHandleDmAuthForm(void* profilePtr, DmDiscoveryInfo* discoveryInfo);
bool DmShellIsLnnAcl(void* profilePtr);
void DmShellCacheAcerAclId(void* profilePtr, DmAclIdParam* param);
void DmShellCacheAceeAclId(void* profilePtr, DmAclIdParam* param);
void DmShellUpdateAccessControlProfile(void* profilePtr);
DmString DmShellAccessToStr(void* acl);
int32_t DmShellSubscribeDeviceProfileInited(void* dpInitedCallback);
int32_t DmShellUnSubscribeDeviceProfileInited(void);
int32_t DmShellPutAllTrustedDevices(DmVecVoid* deviceInfos);
void DmShellGetAppTrustDeviceListC(DmHmap_DmString_DmAuthForm* resultMap,
    const char* pkgName, const char* deviceId);
int32_t DmShellGetForegroundUserIds(DmVec_int* userVec);
DmString DmShellImplGetAccesserExtraData(void* profilePtr);
DmString DmShellImplGetAccesseeExtraData(void* profilePtr);
void DmShellImplSetAccesserExtraData(void* profilePtr, const char* extraData);
void DmShellImplSetAccesseeExtraData(void* profilePtr, const char* extraData);
int32_t DmShellImplUpdateAccessControlProfile(void* profilePtr);

int32_t DmShellGetCurrentAccountUserId(void);
int32_t DmShellQueryActiveOsAccountIds(int32_t* userId);
int32_t DmShellCheckOsAccountConstraintEnabled(int32_t userId, const char* constraint, bool* isEnabled);
DmString DmShellGetOhosAccountId(void);
DmString DmShellGetOhosAccountIdByUserId(int32_t userId);
DmString DmShellGetOhosAccountNameByUserId(int32_t userId);
DmString DmShellGetOhosAccountName(void);
void DmShellGetCallingTokenId(uint32_t* tokenId);
void DmShellGetCallerUserId(int32_t* userId);
int32_t DmShellGetBackgroundUserIds(DmVec_int* userIdVec);
int32_t DmShellGetAllUserIds(DmVec_int* userIdVec);
DmString DmShellGetAccountNickName(int32_t userId);
bool DmShellIsUserUnlocked(int32_t userId);
void DmShellGetForegroundOsAccountLocalIdByDisplayId(int32_t displayId, int32_t* userId);

#ifdef __cplusplus
}
#endif

#endif
