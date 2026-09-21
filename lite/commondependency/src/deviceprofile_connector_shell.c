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


#include "dm_crypto.h"
#include "deviceprofile_connector.h"
#include "dm_shell_functions.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_container.h"
#include "dm_constants.h"
#include "dm_local_acl_profile.h"
#include "dm_jsonstr_handle.h"
#include "json_object.h"
#include "cJSON.h"
#include "securec.h"
#include <stdlib.h>
#include <string.h>

#define DM_AUTH_PRIORITY_IDENTICAL_ACCOUNT 8
#define DM_AUTH_PRIORITY_SHARE 5
#define DM_AUTH_PRIORITY_P2P_USER 6
#define DM_AUTH_PRIORITY_P2P_SERVICE 4
#define DM_AUTH_PRIORITY_P2P_APP 3
#define DM_AUTH_PRIORITY_ACROSS_SERVICE 2
#define DM_AUTH_PRIORITY_ACROSS_APP 1

int32_t DmShellGetAccessControlProfileByUserId(DmVecVoid* profiles, int32_t userId)
{
    return DmAclStoreGetAll(profiles);
}

int32_t DmShellGetAclProfileByUserId(DmVecVoid* profiles, const char* localUdid,
    int32_t userId, const char* remoteUdid)
{
    return DmAclStoreGetByUdid(profiles, localUdid, userId, remoteUdid);
}

int32_t DmShellGetAllAccessControlProfile(DmVecVoid* profiles)
{
    return DmAclStoreGetAll(profiles);
}

int32_t DmShellGetAllAclIncludeLnnAcl(DmVecVoid* profiles)
{
    return DmAclStoreGetAll(profiles);
}


void DmShellFilterNeedDeleteAclInfos(DmVecVoid* profiles, const char* remoteDeviceId,
    DmFilterAclCtx* ctx, DmOfflineParam* offlineParam)
{
    (void)ctx;
    for (int32_t i = 0; i < DmVecVoidSize(profiles); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(profiles, i);
        if (strcmp(DmStringCstr(&p->trustDeviceId), remoteDeviceId) == 0) {
            DmOfflineParamInit(offlineParam);
            offlineParam->bindType = p->bindType;
            if (p->bindType == DM_IDENTICAL_ACCOUNT_VAL) {
                offlineParam->hasUserAcl = true;
            } else if (p->bindType == DM_POINT_TO_POINT_VAL) {
                offlineParam->hasUserAcl = true;
            } else if (p->bindType == DM_LNN_VAL) {
                offlineParam->hasLnnAcl = true;
            }
            DmAclIdParam aclParam;
            DmAclIdParamInit(&aclParam);
            DmStringAssignCstr(&aclParam.udid, DmStringCstr(&p->trustDeviceId));
            aclParam.userId = p->accesser.userId;
            DmStringAssignCstr(&aclParam.credId, DmStringCstr(&p->accesser.credentialId));
            DmVec_DmAclIdParam_Push(&offlineParam->needDelAclInfos, aclParam);
        }
    }
}

void DmShellFilterNeedDeleteAclInfosByPeer(DmVecVoid* profiles, const char* localUdid,
    const char* peerUdid, DmOfflineParam* offlineParam)
{
    for (int32_t i = 0; i < DmVecVoidSize(profiles); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(profiles, i);
        bool isAccesserLocal = strcmp(DmStringCstr(&p->accesser.deviceId), localUdid) == 0;
    const char* peerDevId = isAccesserLocal ?
        DmStringCstr(&p->accessee.deviceId) : DmStringCstr(&p->accesser.deviceId);
        if (strcmp(peerDevId, peerUdid) == 0) {
            DmOfflineParamInit(offlineParam);
            offlineParam->bindType = p->bindType;
            if (p->bindType == DM_IDENTICAL_ACCOUNT_VAL) {
                offlineParam->hasUserAcl = true;
            } else if (p->bindType == DM_POINT_TO_POINT_VAL) {
                offlineParam->hasUserAcl = true;
            } else if (p->bindType == DM_LNN_VAL) {
                offlineParam->hasLnnAcl = true;
            }
        }
    }
}

DmString DmShellProfileGetTrustDeviceId(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DmStringCreateEmpty();
    }
    DmLocalAclProfile* p = (DmLocalAclProfile*)profilePtr;
    return DmStringCreate(DmStringCstr(&p->trustDeviceId));
}

uint32_t DmShellGetAuthFormPriority(void* profilePtr, const char* peerUdid, const char* localUdid)
{
    if (profilePtr == NULL) {
        return 0;
    }
    DmLocalAclProfile* p = (DmLocalAclProfile*)profilePtr;
    int32_t bindType = p->bindType;
    uint32_t bindLevel = p->bindLevel;
    if (bindType == DM_IDENTICAL_ACCOUNT_VAL) {
        return DM_AUTH_PRIORITY_IDENTICAL_ACCOUNT;
    }
    if (bindType == DM_SHARE_VAL) {
        return DM_AUTH_PRIORITY_SHARE;
    }
    if (bindType == DM_POINT_TO_POINT_VAL) {
        if (bindLevel == DM_BIND_LEVEL_USER) {
            return DM_AUTH_PRIORITY_P2P_USER;
        }
        if (bindLevel == DM_SERVICE_CONST) {
            return DM_AUTH_PRIORITY_P2P_SERVICE;
        }
        if (bindLevel == DM_APP_CONST) {
            return DM_AUTH_PRIORITY_P2P_APP;
        }
    }
    if (bindType == DM_ACROSS_ACCOUNT_VAL) {
        if (bindLevel == DM_SERVICE_CONST) {
            return DM_AUTH_PRIORITY_ACROSS_SERVICE;
        }
        if (bindLevel == DM_APP_CONST) {
            return DM_AUTH_PRIORITY_ACROSS_APP;
        }
    }
    return 0;
}

int32_t DmShellPutAccessControlList(DmAclInfo* aclInfo, DmAccesser* dmAccesser, DmAccessee* dmAccessee)
{
    if (aclInfo == NULL || dmAccesser == NULL || dmAccessee == NULL) {
        return ERR_DM_POINT_NULL;
    }
    DmLocalAclProfile* profile = DmLocalAclProfileCreate();
    profile->bindType = aclInfo->bindType;
    DmStringAssignCstr(&profile->trustDeviceId, DmStringCstr(&aclInfo->trustDeviceId));
    profile->bindLevel = aclInfo->bindLevel;
    profile->authenticationType = aclInfo->authenticationType;

    DmStringAssignCstr(&profile->accesser.deviceId, DmStringCstr(&dmAccesser->requestDeviceId));
    profile->accesser.userId = dmAccesser->requestUserId;
    DmStringAssignCstr(&profile->accesser.accountId, DmStringCstr(&dmAccesser->requestAccountId));
    profile->accesser.tokenId = dmAccesser->requestTokenId;
    DmStringAssignCstr(&profile->accesser.bundleName, DmStringCstr(&dmAccesser->requestBundleName));
    DmStringAssignCstr(&profile->accesser.credentialId, DmStringCstr(&dmAccesser->requestCredentialId));

    DmStringAssignCstr(&profile->accessee.deviceId, DmStringCstr(&dmAccessee->trustDeviceId));
    profile->accessee.userId = dmAccessee->trustUserId;
    DmStringAssignCstr(&profile->accessee.accountId, DmStringCstr(&dmAccessee->trustAccountId));
    profile->accessee.tokenId = dmAccessee->trustTokenId;
    DmStringAssignCstr(&profile->accessee.bundleName, DmStringCstr(&dmAccessee->trustBundleName));
    DmStringAssignCstr(&profile->accessee.credentialId, DmStringCstr(&dmAccessee->trustCredentialId));

    int32_t ret = DmAclStorePut(profile);
    DmLocalAclProfileDelete(profile);
    return ret;
}

void DmShellGetUdidHash(const char* udid, uint8_t* hash)
{
    DmString udidStr = DmStringCreate(udid);
    DmGetUdidHashBuf(&udidStr, hash);
    DmStringDestroy(&udidStr);
}

int32_t DmShellProfileGetStatus(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DM_INACTIVE;
    }
    return DM_ACTIVE;
}

int32_t DmShellProfileGetBindType(void* profilePtr)
{
    if (profilePtr == NULL) {
        return 0;
    }
    return ((DmLocalAclProfile*)profilePtr)->bindType;
}

uint32_t DmShellProfileGetBindLevel(void* profilePtr)
{
    if (profilePtr == NULL) {
        return 0;
    }
    return ((DmLocalAclProfile*)profilePtr)->bindLevel;
}

DmString DmShellProfileGetAccesserDeviceId(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DmStringCreateEmpty();
    }
    return DmStringCreate(DmStringCstr(&((DmLocalAclProfile*)profilePtr)->accesser.deviceId));
}

int32_t DmShellProfileGetAccesserUserId(void* profilePtr)
{
    if (profilePtr == NULL) {
        return 0;
    }
    return ((DmLocalAclProfile*)profilePtr)->accesser.userId;
}

DmString DmShellProfileGetAccesseeDeviceId(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DmStringCreateEmpty();
    }
    return DmStringCreate(DmStringCstr(&((DmLocalAclProfile*)profilePtr)->accessee.deviceId));
}

int32_t DmShellProfileGetAccesseeUserId(void* profilePtr)
{
    if (profilePtr == NULL) {
        return 0;
    }
    return ((DmLocalAclProfile*)profilePtr)->accessee.userId;
}

int32_t DmShellHandleDmAuthForm(void* profilePtr, DmDiscoveryInfo* discoveryInfo)
{
    if (profilePtr == NULL) {
        return DM_INVALIED_TYPE;
    }
    DmLocalAclProfile* p = (DmLocalAclProfile*)profilePtr;
    if (p->bindType == DM_IDENTICAL_ACCOUNT_VAL) {
        return DM_IDENTICAL_ACCOUNT_TYPE;
    }
    if (p->bindType == DM_POINT_TO_POINT_VAL) {
        if (p->bindLevel == DM_BIND_LEVEL_USER) {
            return DM_DEVICE_PEER_TO_PEER_TYPE;
        }
        if (p->bindLevel == DM_SERVICE_CONST) {
            return DM_SERVICE_PEER_TO_PEER_TYPE;
        }
        if (p->bindLevel == DM_APP_CONST) {
            return DM_APP_PEER_TO_PEER_TYPE;
        }
    }
    if (p->bindType == DM_ACROSS_ACCOUNT_VAL) {
        return DM_DEVICE_ACROSS_ACCOUNT_TYPE;
    }
    if (p->bindType == DM_SHARE_VAL) {
        return DM_SHARE_TYPE;
    }
    return DM_INVALIED_TYPE;
}

bool DmShellIsLnnAcl(void* profilePtr)
{
    (void)profilePtr;
    return false;
}

void DmShellCacheAcerAclId(void* profilePtr, DmAclIdParam* param)
{
    if (profilePtr == NULL || param == NULL) {
        return;
    }
    DmLocalAclProfile* p = (DmLocalAclProfile*)profilePtr;
    DmAclIdParamInit(param);
    DmStringAssignCstr(&param->udid, DmStringCstr(&p->trustDeviceId));
    param->userId = p->accesser.userId;
    DmStringAssignCstr(&param->credId, DmStringCstr(&p->accesser.credentialId));
    DmVecInt64_t_Push(&param->tokenIds, p->accesser.tokenId);
    DmStringAssignCstr(&param->pkgName, DmStringCstr(&p->accesser.bundleName));
}

void DmShellCacheAceeAclId(void* profilePtr, DmAclIdParam* param)
{
    if (profilePtr == NULL || param == NULL) {
        return;
    }
    DmLocalAclProfile* p = (DmLocalAclProfile*)profilePtr;
    DmAclIdParamInit(param);
    DmStringAssignCstr(&param->udid, DmStringCstr(&p->trustDeviceId));
    param->userId = p->accessee.userId;
    DmStringAssignCstr(&param->credId, DmStringCstr(&p->accessee.credentialId));
    DmVecInt64_t_Push(&param->tokenIds, p->accessee.tokenId);
    DmStringAssignCstr(&param->pkgName, DmStringCstr(&p->accessee.bundleName));
}

void DmShellProfileSetStatus(void* profilePtr, int32_t status)
{
    (void)profilePtr;
    (void)status;
}

void DmShellUpdateAccessControlProfile(void* profilePtr)
{
    if (profilePtr == NULL) {
        return;
    }
    DmAclStorePut((DmLocalAclProfile*)profilePtr);
}

DmString DmShellAccessToStr(void* acl)
{
    if (acl == NULL) {
        return DmStringCreateEmpty();
    }
    DmLocalAclProfile* p = (DmLocalAclProfile*)acl;
    char buf[256];
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%s:%d:%u",
        DmStringCstr(&p->accesser.deviceId), p->bindType, p->accesser.tokenId) < 0) {
        return DmStringCreateEmpty();
    }
    return DmStringCreate(buf);
}

int32_t DmShellSubscribeDeviceProfileInited(void* dpInitedCallback)
{
    (void)dpInitedCallback;
    DmAclStoreInit();
    return DM_OK;
}
int32_t DmShellUnSubscribeDeviceProfileInited(void)
{
    return DM_OK;
}

int32_t DmShellPutAllTrustedDevices(DmVecVoid* deviceInfos)
{
    return DM_OK;
}

void DmShellGetAppTrustDeviceListC(DmHmap_DmString_DmAuthForm* resultMap,
    const char* pkgName, const char* deviceId)
{
    bool isAllPkg = (pkgName != NULL && strcmp(pkgName, DM_ALL_PKGNAME) == 0);
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmAclStoreGetAll(&profiles);
    for (int32_t i = 0; i < DmVecVoidSize(&profiles); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&profiles, i);
        /* filter 1: only keep ACLs involving the local device */
        if (deviceId != NULL && strlen(deviceId) > 0 &&
            strcmp(DmStringCstr(&p->accesser.deviceId), deviceId) != 0 &&
            strcmp(DmStringCstr(&p->accessee.deviceId), deviceId) != 0) {
            DmLocalAclProfileDestroy(p);
            continue;
        }
        /* filter 2: pkgName filter */
        bool needPkgFilter = (p->bindLevel == DM_APP_CONST || p->bindLevel == DM_SERVICE_CONST);
        if (needPkgFilter && !isAllPkg &&
            strcmp(DmStringCstr(&p->accesser.bundleName), pkgName) != 0 &&
            strcmp(DmStringCstr(&p->accessee.bundleName), pkgName) != 0) {
            DmLocalAclProfileDestroy(p);
            continue;
        }
        DmString key = DmStringCreate(DmStringCstr(&p->trustDeviceId));
        DmAuthForm form = DM_AUTH_FORM_PEER_TO_PEER;
        DmHmap_DmString_DmAuthForm_Insert(resultMap, key, form);
        DmStringDestroy(&key);
        DmLocalAclProfileDestroy(p);
    }
    DmVecVoidDestroy(&profiles);
}

int32_t DmShellGetForegroundUserIds(DmVec_int* userVec)
{
    if (userVec == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmVec_int_Push(userVec, 0);
    return 0;
}

DmString DmShellLocalServiceInfoGetExtraInfo(void* localServiceInfoPtr)
{
    return DmStringCreateEmpty();
}

DmString DmShellImplGetAccesserExtraData(void* profilePtr)
{
    (void)profilePtr;
    return DmStringCreateEmpty();
}

DmString DmShellImplGetAccesseeExtraData(void* profilePtr)
{
    (void)profilePtr;
    return DmStringCreateEmpty();
}

void DmShellImplSetAccesserExtraData(void* profilePtr, const char* extraData)
{
    (void)profilePtr;
    (void)extraData;
}

void DmShellImplSetAccesseeExtraData(void* profilePtr, const char* extraData)
{
    (void)profilePtr;
    (void)extraData;
}

int32_t DmShellImplUpdateAccessControlProfile(void* profilePtr)
{
    if (profilePtr == NULL) {
        return ERR_DM_POINT_NULL;
    }
    return DmAclStorePut((DmLocalAclProfile*)profilePtr);
}

DmString DmShellProfileGetAccesserExtraData(void* profilePtr)
{
    (void)profilePtr;
    return DmStringCreateEmpty();
}

DmString DmShellProfileGetAccesseeExtraData(void* profilePtr)
{
    (void)profilePtr;
    return DmStringCreateEmpty();
}

void DmShellProfileSetAccesserExtraData(void* profilePtr, const char* extraData)
{
    (void)profilePtr;
    (void)extraData;
}

void DmShellProfileSetAccesseeExtraData(void* profilePtr, const char* extraData)
{
    (void)profilePtr;
    (void)extraData;
}

int32_t DmDpConnectorUpdateAccessControlProfile(void* profilePtr)
{
    if (profilePtr == NULL) {
        return ERR_DM_POINT_NULL;
    }
    return DmAclStorePut((DmLocalAclProfile*)profilePtr);
}

int64_t DmShellProfileGetAccesserTokenId(void* profilePtr)
{
    if (profilePtr == NULL) {
        return 0;
    }
    return ((DmLocalAclProfile*)profilePtr)->accesser.tokenId;
}

DmString DmShellProfileGetAccesserBundleName(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DmStringCreateEmpty();
    }
    return DmStringCreate(DmStringCstr(&((DmLocalAclProfile*)profilePtr)->accesser.bundleName));
}

DmString DmShellProfileGetAccesserAccountId(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DmStringCreateEmpty();
    }
    return DmStringCreate(DmStringCstr(&((DmLocalAclProfile*)profilePtr)->accesser.accountId));
}

DmString DmShellProfileGetAccesserCredentialIdStr(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DmStringCreateEmpty();
    }
    return DmStringCreate(DmStringCstr(&((DmLocalAclProfile*)profilePtr)->accesser.credentialId));
}

int64_t DmShellProfileGetAccesseeTokenId(void* profilePtr)
{
    if (profilePtr == NULL) {
        return 0;
    }
    return ((DmLocalAclProfile*)profilePtr)->accessee.tokenId;
}

DmString DmShellProfileGetAccesseeBundleName(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DmStringCreateEmpty();
    }
    return DmStringCreate(DmStringCstr(&((DmLocalAclProfile*)profilePtr)->accessee.bundleName));
}

DmString DmShellProfileGetAccesseeAccountId(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DmStringCreateEmpty();
    }
    return DmStringCreate(DmStringCstr(&((DmLocalAclProfile*)profilePtr)->accessee.accountId));
}

DmString DmShellProfileGetAccesseeCredentialIdStr(void* profilePtr)
{
    if (profilePtr == NULL) {
        return DmStringCreateEmpty();
    }
    return DmStringCreate(DmStringCstr(&((DmLocalAclProfile*)profilePtr)->accessee.credentialId));
}


int32_t DmShellGetBusinessEvent(const char* businessKey, DmString* businessValue)
{
    return ERR_DM_FAILED;
}

int32_t DmShellGetServiceInfoByUdidAndServiceId(void* serviceInfoPtr,
    const char* udid, int64_t serviceId)
{
    return ERR_DM_FAILED;
}

int64_t DmShellServiceInfoGetServiceOwnerTokenId(void* serviceInfoPtr)
{
    return 0;
}

int64_t DmShellServiceInfoGetServiceRegisterTokenId(void* serviceInfoPtr)
{
    return 0;
}

int32_t DmShellServiceInfoGetPublishState(void* serviceInfoPtr)
{
    return 0;
}

int32_t DmShellServiceInfoGetUserId(void* serviceInfoPtr)
{
    return 0;
}

int32_t DmShellServiceInfoGetDisplayId(void* serviceInfoPtr)
{
    return 0;
}

DmString DmShellServiceInfoGetServiceOwnerPkgName(void* serviceInfoPtr)
{
    return DmStringCreateEmpty();
}

int32_t DmShellGetLocalServiceInfoByBundleNameAndPinExchangeType(
    void* localServiceInfoPtr, const char* bundleName, int32_t pinExchangeType)
{
    return ERR_DM_FAILED;
}

int32_t DmDpConnectorGetAllAclIncludeLnnAcl(DmVecVoid* profiles)
{
    return DmAclStoreGetAll(profiles);
}

int32_t DmDpConnectorGetAclProfileByDeviceIdAndUserIdRemote(DmVecVoid* profiles,
    const char* deviceId, int32_t userId, const char* remoteDeviceId)
{
    return DmAclStoreGetByUdid(profiles, deviceId, userId, remoteDeviceId);
}


int32_t DmDpConnectorGetAclList(DmVecVoid* profiles, const char* localUdid,
    int32_t localUserId, const char* remoteUdid, int32_t remoteUserId)
{
    DmVecVoid all;
    DmVecVoidInit(&all);
    DmAclStoreGetAll(&all);
    for (int32_t i = 0; i < DmVecVoidSize(&all); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&all, i);
        bool match = false;
        if (strcmp(DmStringCstr(&p->accesser.deviceId), localUdid) == 0 && p->accesser.userId == localUserId &&
            strcmp(DmStringCstr(&p->accessee.deviceId), remoteUdid) == 0 && p->accessee.userId == remoteUserId) {
            match = true;
        }
        if (strcmp(DmStringCstr(&p->accessee.deviceId), localUdid) == 0 && p->accessee.userId == localUserId &&
            strcmp(DmStringCstr(&p->accesser.deviceId), remoteUdid) == 0 && p->accesser.userId == remoteUserId) {
            match = true;
        }
        if (match) {
            DmVecVoidPush(profiles, p);
        } else {
            DmLocalAclProfileDelete(p);
        }
    }
    free(all.data);
    return DM_OK;
}

int32_t DmDpConnectorGetLocalServiceInfoByBundleNameAndPinExchangeType(
    const char* bundleName, int32_t pinExchangeType, void* localServiceInfo)
{
    return ERR_DM_FAILED;
}

int32_t DmDpConnectorGetServiceInfosByUdid(DmVecVoid* serviceInfos, const char* udid)
{
    return ERR_DM_FAILED;
}


bool DmDpConnectorIsAllowAuthAlways(const char* localUdid, int32_t userId,
    const char* peerUdid, const char* pkgName, int64_t tokenId)
{
    return false;
}


void DmDpConnectorUpdateAclStatus(void* profilePtr)
{
    if (profilePtr == NULL) {
        return;
    }
    DmAclStorePut((DmLocalAclProfile*)profilePtr);
}

bool DmDpConnectorCheckAccessControl(const DmAccessCaller* caller,
    const char* srcUdid, const DmAccessCallee* callee, const char* sinkUdid)
{
    if (caller == NULL || callee == NULL || srcUdid == NULL) {
        return false;
    }
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmAclStoreGetAll(&profiles);
    bool found = false;
    for (int32_t i = 0; i < DmVecVoidSize(&profiles); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&profiles, i);
        if ((strcmp(DmStringCstr(&p->accesser.deviceId), srcUdid) == 0 &&
             (strcmp(DmStringCstr(&p->accesser.bundleName), DmStringCstr(&caller->pkgName)) == 0 ||
              p->accesser.tokenId == caller->tokenId)) ||
            (strcmp(DmStringCstr(&p->accessee.deviceId), srcUdid) == 0 &&
             (strcmp(DmStringCstr(&p->accessee.bundleName), DmStringCstr(&callee->pkgName)) == 0 ||
              p->accessee.tokenId == callee->tokenId))) {
            found = true;
        }
        DmLocalAclProfileDestroy(p);
    }
    DmVecVoidDestroy(&profiles);
    return found;
}

bool DmDpConnectorCheckIsSameAccount(const DmAccessCaller* caller,
    const char* srcUdid, const DmAccessCallee* callee, const char* sinkUdid)
{
    if (caller == NULL || callee == NULL || srcUdid == NULL) {
        return false;
    }
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmAclStoreGetAll(&profiles);
    bool found = false;
    for (int32_t i = 0; i < DmVecVoidSize(&profiles); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&profiles, i);
        if ((strcmp(DmStringCstr(&p->accesser.deviceId), srcUdid) == 0 ||
            strcmp(DmStringCstr(&p->accessee.deviceId), srcUdid) == 0)) {
            found = true;
        }
        DmLocalAclProfileDestroy(p);
    }
    DmVecVoidDestroy(&profiles);
    return found;
}

int32_t DmDpConnectorUpdateLocalServiceInfo(void* localServiceInfo)
{
    return ERR_DM_FAILED;
}

void DmShellDeleteAccessControlProfile(const char* trustDeviceId)
{
    if (trustDeviceId == NULL) {
        return;
    }
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmAclStoreGetAll(&profiles);
    for (int32_t i = 0; i < DmVecVoidSize(&profiles); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&profiles, i);
        if (p == NULL) {
            continue;
        }
        if (strcmp(DmStringCstr(&p->trustDeviceId), trustDeviceId) == 0) {
            char key[32];
            DmAclKeyFromProfile(key, sizeof(key), p);
            DmAclStoreDeleteByKey(key);
        }
        DmLocalAclProfileDelete(p);
        profiles.data[i] = NULL;
    }
    free(profiles.data);
}
