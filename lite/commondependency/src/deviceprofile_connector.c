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


#include "deviceprofile_connector.h"
#include "dm_anonymous.h"
#include "dm_constants.h"

DM_VEC_IMPL(DmAclIdParam)
DM_VEC_IMPL(DmUserRemovedServiceInfo)
DM_VEC_IMPL(DmAclHashItem)
DM_HMAP_IMPL(DmString_DmAuthForm, DmString, DmAuthForm, DmHashDmString, DmCmpDmString)
static int DmCmp_Dmauthonceaclinfo(DmAuthOnceAclInfo a, DmAuthOnceAclInfo b)
{
    if (a.localUserId != b.localUserId) {
        return (a.localUserId > b.localUserId) - (a.localUserId < b.localUserId);
    }
    if (a.peerUserId != b.peerUserId) {
        return (a.peerUserId > b.peerUserId) - (a.peerUserId < b.peerUserId);
    }
    return DmStringCmp(&a.peerUdid, &b.peerUdid);
}
DM_SET_IMPL(DmAuthOnceAclInfo, DmCmp_Dmauthonceaclinfo)
DM_MAP_IMPL(DmString_DmOfflineParam, DmString, DmOfflineParam, DmCmpDmString)
#include "dm_log.h"
#include "multiple_user_connector.h"
#include "dm_jsonstr_handle.h"
#include "json_object.h"
#include "dm_shell_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint32_t DM_INVALIED_TYPE = 0;
const uint32_t DM_APP_PEER_TO_PEER_TYPE = 1;
const uint32_t DM_APP_ACROSS_ACCOUNT_TYPE = 2;
const uint32_t DM_SERVICE_PEER_TO_PEER_TYPE = 3;
const uint32_t DM_SERVICE_ACROSS_ACCOUNT_TYPE = 4;
const uint32_t DM_SHARE_TYPE = 5;
const uint32_t DM_DEVICE_PEER_TO_PEER_TYPE = 6;
const uint32_t DM_DEVICE_ACROSS_ACCOUNT_TYPE = 7;
const uint32_t DM_IDENTICAL_ACCOUNT_TYPE = 8;
const uint32_t DM_DM_INVALIED_TYPE = 2048;
const uint32_t DM_SERVICE_CONST = 2;
const uint32_t DM_APP_CONST = 3;
const uint32_t DM_BIND_LEVEL_USER = 1;
const int64_t DM_DEFAULT_SERVICE_ID = 0;
static const uint32_t DM_MAX_SESSION_KEY_LENGTH __attribute__((unused)) = 512;

const char* DM_TAG_PEER_BUNDLE_NAME = "peerBundleName";
const char* DM_TAG_PEER_TOKENID = "peerTokenId";
static const char* DM_TAG_ACL __attribute__((unused)) = "accessControlTable";
static const char* DM_TAG_DMVERSION = "dmVersion";
static const char* DM_TAG_ACL_HASH_KEY_VERSION __attribute__((unused)) = "aclVersion";
static const char* DM_TAG_ACL_HASH_KEY_ACLHASHLIST __attribute__((unused)) = "aclHashList";


static const int32_t DM_SUPPORT_ACL_AGING_VERSION_NUM __attribute__((unused)) = 1;
static const DmString DM_VERSION_STR_5_1_0 __attribute__((unused));
static const uint32_t DM_AUTH_EXT_WHITE_LIST_NUM __attribute__((unused)) = 1;
static const char* g_dmExtWhiteList[1] __attribute__((unused)) = {
    "CastEngineService",
};
typedef enum {
    DM_DEVICE_PEER_TO_PEER_BIND_TYPE = 3,
    DM_DEVICE_ACROSS_ACCOUNT_BIND_TYPE = 4,
    DM_IDENTICAL_ACCOUNT_BIND_TYPE = 5
} DmDevBindType;

void DmDiscoveryInfoInit(DmDiscoveryInfo* info)
{
    info->pkgname = DmStringCreateEmpty();
    info->localDeviceId = DmStringCreateEmpty();
    info->userId = 0;
    info->remoteDeviceIdHash = DmStringCreateEmpty();
}

void DmDiscoveryInfoDestroy(DmDiscoveryInfo* info)
{
    DmStringDestroy(&info->pkgname);
    DmStringDestroy(&info->localDeviceId);
    DmStringDestroy(&info->remoteDeviceIdHash);
}

void DmAclInfoInit(DmAclInfo* info)
{
    info->sessionKey = DmStringCreateEmpty();
    info->bindType = 0;
    info->state = 0;
    info->trustDeviceId = DmStringCreateEmpty();
    info->bindLevel = 0;
    info->authenticationType = 0;
    info->deviceIdHash = DmStringCreateEmpty();
    info->extraData = DmStringCreateEmpty();
}

void DmAclInfoDestroy(DmAclInfo* info)
{
    DmStringDestroy(&info->sessionKey);
    DmStringDestroy(&info->trustDeviceId);
    DmStringDestroy(&info->deviceIdHash);
    DmStringDestroy(&info->extraData);
}

void DmAccesserStructInit(DmAccesser* info)
{
    info->requestTokenId = 0;
    info->requestBundleName = DmStringCreateEmpty();
    info->requestUserId = 0;
    info->requestAccountId = DmStringCreateEmpty();
    info->requestDeviceId = DmStringCreateEmpty();
    info->requestTargetClass = 0;
    info->requestDeviceName = DmStringCreateEmpty();
    info->requestCredentialId = DmStringCreateEmpty();
    info->requestSkTimeStamp = 0;
    info->requestExtraData = DmStringCreateEmpty();
}

void DmAccesserStructDestroy(DmAccesser* info)
{
    DmStringDestroy(&info->requestBundleName);
    DmStringDestroy(&info->requestAccountId);
    DmStringDestroy(&info->requestDeviceId);
    DmStringDestroy(&info->requestDeviceName);
    DmStringDestroy(&info->requestCredentialId);
    DmStringDestroy(&info->requestExtraData);
}

void DmAccesserSetDeviceId(DmAccesser* info, const char* value)
{
    DmStringDestroy(&info->requestDeviceId);
    info->requestDeviceId = DmStringCreate(value);
}

void DmAccesserSetUserId(DmAccesser* info, int32_t value)
{
    info->requestUserId = value;
}

void DmAccesserSetAccountId(DmAccesser* info, const char* value)
{
    DmStringDestroy(&info->requestAccountId);
    info->requestAccountId = DmStringCreate(value);
}

void DmAccesserSetTokenId(DmAccesser* info, uint64_t value)
{
    info->requestTokenId = value;
}

void DmAccesserSetBundleName(DmAccesser* info, const char* value)
{
    DmStringDestroy(&info->requestBundleName);
    info->requestBundleName = DmStringCreate(value);
}

void DmAccesserSetDeviceName(DmAccesser* info, const char* value)
{
    DmStringDestroy(&info->requestDeviceName);
    info->requestDeviceName = DmStringCreate(value);
}

void DmAccesserSetCredentialIdStr(DmAccesser* info, const char* value)
{
    DmStringDestroy(&info->requestCredentialId);
    info->requestCredentialId = DmStringCreate(value);
}


void DmAccesserSetSkTimeStamp(DmAccesser* info, int64_t value)
{
    info->requestSkTimeStamp = value;
}

void DmAccesserSetExtraData(DmAccesser* info, const char* value)
{
    DmStringDestroy(&info->requestExtraData);
    info->requestExtraData = DmStringCreate(value);
}

void DmAccesseeStructInit(DmAccessee* info)
{
    info->trustTokenId = 0;
    info->trustBundleName = DmStringCreateEmpty();
    info->trustUserId = 0;
    info->trustAccountId = DmStringCreateEmpty();
    info->trustDeviceId = DmStringCreateEmpty();
    info->trustTargetClass = 0;
    info->trustDeviceName = DmStringCreateEmpty();
    info->trustCredentialId = DmStringCreateEmpty();
    info->trustSkTimeStamp = 0;
    info->trustExtraData = DmStringCreateEmpty();
}

void DmAccesseeStructDestroy(DmAccessee* info)
{
    DmStringDestroy(&info->trustBundleName);
    DmStringDestroy(&info->trustAccountId);
    DmStringDestroy(&info->trustDeviceId);
    DmStringDestroy(&info->trustDeviceName);
    DmStringDestroy(&info->trustCredentialId);
    DmStringDestroy(&info->trustExtraData);
}

void DmAccesseeSetDeviceId(DmAccessee* info, const char* value)
{
    DmStringDestroy(&info->trustDeviceId);
    info->trustDeviceId = DmStringCreate(value);
}

void DmAccesseeSetUserId(DmAccessee* info, int32_t value)
{
    info->trustUserId = value;
}

void DmAccesseeSetAccountId(DmAccessee* info, const char* value)
{
    DmStringDestroy(&info->trustAccountId);
    info->trustAccountId = DmStringCreate(value);
}

void DmAccesseeSetTokenId(DmAccessee* info, uint64_t value)
{
    info->trustTokenId = value;
}

void DmAccesseeSetBundleName(DmAccessee* info, const char* value)
{
    DmStringDestroy(&info->trustBundleName);
    info->trustBundleName = DmStringCreate(value);
}

void DmAccesseeSetDeviceName(DmAccessee* info, const char* value)
{
    DmStringDestroy(&info->trustDeviceName);
    info->trustDeviceName = DmStringCreate(value);
}

void DmAccesseeSetCredentialIdStr(DmAccessee* info, const char* value)
{
    DmStringDestroy(&info->trustCredentialId);
    info->trustCredentialId = DmStringCreate(value);
}


void DmAccesseeSetSkTimeStamp(DmAccessee* info, int64_t value)
{
    info->trustSkTimeStamp = value;
}

void DmAccesseeSetExtraData(DmAccessee* info, const char* value)
{
    DmStringDestroy(&info->trustExtraData);
    info->trustExtraData = DmStringCreate(value);
}

void DmAclIdParamInit(DmAclIdParam* param)
{
    param->udid = DmStringCreateEmpty();
    param->userId = 0;
    param->credId = DmStringCreateEmpty();
    param->pkgName = DmStringCreateEmpty();
    DmVecInt64_t_Init(&param->tokenIds);
}

void DmAclIdParamDestroy(DmAclIdParam* param)
{
    DmStringDestroy(&param->udid);
    DmStringDestroy(&param->credId);
    DmStringDestroy(&param->pkgName);
    DmVecInt64_t_Destroy(&param->tokenIds);
}

void DmOfflineParamInit(DmOfflineParam* param)
{
    param->bindType = 0;
    DmVec_ProcessInfo_Init(&param->processVec);
    DmVec_DmString_Init(&param->credIdVec);
    param->leftAclNumber = 0;
    param->peerUserId = 0;
    param->hasLnnAcl = false;
    param->hasUserAcl = false;
    param->isNewVersion = true;
    DmVec_DmAclIdParam_Init(&param->needDelAclInfos);
    DmVec_DmAclIdParam_Init(&param->allLnnAclInfos);
    DmVec_DmAclIdParam_Init(&param->allLeftAppOrSvrAclInfos);
    DmVec_DmAclIdParam_Init(&param->allUserAclInfos);
}

void DmOfflineParamDestroy(DmOfflineParam* param)
{
    DmVec_ProcessInfo_Destroy(&param->processVec);
    DmVec_DmString_Destroy(&param->credIdVec);
    DmVec_DmAclIdParam_Destroy(&param->needDelAclInfos);
    DmVec_DmAclIdParam_Destroy(&param->allLnnAclInfos);
    DmVec_DmAclIdParam_Destroy(&param->allLeftAppOrSvrAclInfos);
    DmVec_DmAclIdParam_Destroy(&param->allUserAclInfos);
}

void DmLocalUserRemovedInfoInit(DmLocalUserRemovedInfo* info)
{
    info->localUdid = DmStringCreateEmpty();
    info->preUserId = 0;
    DmVec_DmString_Init(&info->peerUdids);
}

void DmLocalUserRemovedInfoDestroy(DmLocalUserRemovedInfo* info)
{
    DmStringDestroy(&info->localUdid);
    DmVec_DmString_Destroy(&info->peerUdids);
}

void DmRemoteUserRemovedInfoInit(DmRemoteUserRemovedInfo* info)
{
    info->peerUdid = DmStringCreateEmpty();
    info->peerUserId = 0;
    DmVec_int_Init(&info->localUserIds);
}

void DmRemoteUserRemovedInfoDestroy(DmRemoteUserRemovedInfo* info)
{
    DmStringDestroy(&info->peerUdid);
    DmVec_int_Destroy(&info->localUserIds);
}

void DmAclHashItemInit(DmAclHashItem* item)
{
    item->version = DmStringCreateEmpty();
    DmVec_DmString_Init(&item->aclHashList);
}

void DmAclHashItemDestroy(DmAclHashItem* item)
{
    DmStringDestroy(&item->version);
    DmVec_DmString_Destroy(&item->aclHashList);
}

void DmAuthOnceAclInfoInit(DmAuthOnceAclInfo* info)
{
    info->peerUdid = DmStringCreateEmpty();
    info->peerUserId = 0;
    info->localUserId = 0;
}

void DmAuthOnceAclInfoDestroy(DmAuthOnceAclInfo* info)
{
    DmStringDestroy(&info->peerUdid);
}

static DmString DmDpConnectorGetLocalDeviceId(void)
{
    return DmStringCreateEmpty();
}

struct DmDeviceProfileConnector {
    void* cppHandle;
};

DM_IMPLEMENT_SINGLE_INSTANCE(DmDeviceProfileConnector)

DmOfflineParam DmDpConnectorFilterNeedDeleteAcl(const char* localDeviceId, uint32_t localTokenId,
    const char* remoteDeviceId, const char* extra)
{
    DmOfflineParam offlineParam;
    DmOfflineParamInit(&offlineParam);
    int32_t userId = DmMultipleUserGetCurrentAccountUserId();
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmDpConnectorGetAclProfileByDeviceIdAndUserIdRemote(&profiles, localDeviceId, userId, remoteDeviceId);
    if (DmVecVoidSize(&profiles) == 0) {
        LOGE("Acl is empty.");
        DmVecVoidDestroy(&profiles);
        return offlineParam;
    }
    DmFilterAclCtx ctx = { localDeviceId, localTokenId, extra };
    DmShellFilterNeedDeleteAclInfos(&profiles, remoteDeviceId, &ctx, &offlineParam);
    DmVecVoidDestroy(&profiles);
    return offlineParam;
}

DmOfflineParam DmDpConnectorFilterNeedDeleteAclByPeer(const char* peerUdid)
{
    DmString localUdid = DmDpConnectorGetLocalDeviceId();
    DmOfflineParam offlineParam;
    DmOfflineParamInit(&offlineParam);
    int32_t userId = DmMultipleUserGetCurrentAccountUserId();
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmDpConnectorGetAclProfileByDeviceIdAndUserIdRemote(&profiles, DmStringCstr(&localUdid),
        userId, peerUdid);
    if (DmVecVoidSize(&profiles) == 0) {
        LOGE("Acl is empty.");
        DmStringDestroy(&localUdid);
        DmVecVoidDestroy(&profiles);
        return offlineParam;
    }
    DmShellFilterNeedDeleteAclInfosByPeer(&profiles, DmStringCstr(&localUdid), peerUdid, &offlineParam);
    DmStringDestroy(&localUdid);
    DmVecVoidDestroy(&profiles);
    return offlineParam;
}

int32_t DmDpConnectorGetAccessControlProfile(DmVecVoid* profiles)
{
    int32_t userId = DmMultipleUserGetCurrentAccountUserId();
    return DmShellGetAccessControlProfileByUserId(profiles, userId);
}

uint32_t DmDpConnectorCheckBindType(const char* peerUdid, const char* localUdid)
{
    DmVecVoid filterProfiles;
    DmVecVoidInit(&filterProfiles);
    DmShellGetAclProfileByUserId(&filterProfiles, localUdid,
        DmMultipleUserGetFirstForegroundUserId(), peerUdid);
    uint32_t highestPriority = DM_INVALIED_TYPE;
    for (int i = 0; i < DmVecVoidSize(&filterProfiles); i++) {
        void** item = DmVecVoidAt(&filterProfiles, i);
        if (item == NULL) {
            continue;
        }
        bool isLnn = DmDpConnectorIsLnnAcl(*item);
        DmString trustDevId = DmShellProfileGetTrustDeviceId(*item);
        if (isLnn || DmStringCmpCstr(&trustDevId, peerUdid) != 0) {
            DmStringDestroy(&trustDevId);
            continue;
        }
        uint32_t priority = DmShellGetAuthFormPriority(*item, peerUdid, localUdid);
        if (priority > highestPriority) {
            highestPriority = priority;
        }
        DmStringDestroy(&trustDevId);
    }
    DmVecVoidDestroy(&filterProfiles);
    return highestPriority;
}

int32_t DmDpConnectorPutAccessControlList(DmAclInfo aclInfo, DmAccesser dmAccesser, DmAccessee dmAccessee)
{
    LOGI("Start.");
    return DmShellPutAccessControlList(&aclInfo, &dmAccesser, &dmAccessee);
}

static bool DmCheckProfileMatch(void* profilePtr, DmDiscoveryInfo* discoveryInfo)
{
    DmString trustDevId = DmShellProfileGetTrustDeviceId(profilePtr);
    char deviceIdHash[DM_MAX_DEVICE_ID_LEN] = {0};
    DmShellGetUdidHash(DmStringCstr(&trustDevId), (uint8_t*)deviceIdHash);
    bool isLnn = DmDpConnectorIsLnnAcl(profilePtr);
    DmString acerDevId = DmShellProfileGetAccesserDeviceId(profilePtr);
    int32_t acerUserId = DmShellProfileGetAccesserUserId(profilePtr);
    DmString aceeDevId = DmShellProfileGetAccesseeDeviceId(profilePtr);
    int32_t aceeUserId = DmShellProfileGetAccesseeUserId(profilePtr);
    bool skip = false;
    if (isLnn || strcmp(deviceIdHash, DmStringCstr(&discoveryInfo->remoteDeviceIdHash)) != 0 ||
        (DmStringCmpCstr(&discoveryInfo->localDeviceId, DmStringCstr(&acerDevId)) == 0 &&
         discoveryInfo->userId != acerUserId) ||
        (DmStringCmpCstr(&discoveryInfo->localDeviceId, DmStringCstr(&aceeDevId)) == 0 &&
         discoveryInfo->userId != aceeUserId)) {
        skip = true;
    }
    DmStringDestroy(&trustDevId);
    DmStringDestroy(&acerDevId);
    DmStringDestroy(&aceeDevId);
    return !skip;
}

static int32_t DmFindBindType(DmVec_int* bindTypes, int32_t target)
{
    for (int i = 0; i < DmVec_int_Size(bindTypes); i++) {
        int32_t* val = DmVec_int_At(bindTypes, i);
        if (val != NULL && *val == target) {
            return target;
        }
    }
    return (int32_t)DM_INVALIED_TYPE;
}

static void DmCollectBindTypes(DmVecVoid* profiles, DmDiscoveryInfo* discoveryInfo, DmVec_int* bindTypes)
{
    for (int i = 0; i < DmVecVoidSize(profiles); i++) {
        void** item = DmVecVoidAt(profiles, i);
        if (item == NULL || !DmCheckProfileMatch(*item, discoveryInfo)) {
            continue;
        }
        int32_t bindType = DmShellHandleDmAuthForm(*item, discoveryInfo);
        if (bindType != (int32_t)DM_INVALIED_TYPE) {
            DmVec_int_Push(bindTypes, bindType);
        }
    }
}

static void DmFilterProfilesByBindType(DmVecVoid* profiles, DmDiscoveryInfo* discoveryInfo,
    bool* isOnline, int32_t* authForm)
{
    DmVec_int bindTypes;
    DmVec_int_Init(&bindTypes);
    DmCollectBindTypes(profiles, discoveryInfo, &bindTypes);
    int32_t form = DmFindBindType(&bindTypes, DM_IDENTICAL_ACCOUNT_VAL);
    if (form == (int32_t)DM_INVALIED_TYPE) {
        form = DmFindBindType(&bindTypes, DM_POINT_TO_POINT_VAL);
    }
    if (form == (int32_t)DM_INVALIED_TYPE) {
        form = DmFindBindType(&bindTypes, DM_ACROSS_ACCOUNT_VAL);
    }
    if (form != (int32_t)DM_INVALIED_TYPE) {
        *isOnline = true;
        *authForm = form;
    } else {
        *authForm = (int32_t)DM_INVALIED_TYPE;
    }
    DmVec_int_Destroy(&bindTypes);
}

int32_t DmDpConnectorGetDeviceAclParam(DmDiscoveryInfo discoveryInfo, bool* isOnline, int32_t* authForm)
{
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmShellGetAccessControlProfileByUserId(&profiles, discoveryInfo.userId);
    DmFilterProfilesByBindType(&profiles, &discoveryInfo, isOnline, authForm);
    DmVecVoidDestroy(&profiles);
    return 0;
}

int32_t DmDpConnectorGetVersionByExtra(DmString* extraInfo, DmString* dmVersion)
{
    DmJsonObject extraInfoJson;
    DmJsonObjectInitFromStr(&extraInfoJson, DmStringCstr(extraInfo));
    DmJsonItemObject* itemObj = DmJsonObjectGetItem(&extraInfoJson);
    if (DmJsonItemObjectIsDiscarded(itemObj)) {
        LOGE("extraInfoJson error");
        DmJsonObjectDestroy(&extraInfoJson);
        return ERR_DM_FAILED;
    }
    DmJsonItemObject versionItem = DmJsonItemObjectAt(itemObj, DM_TAG_DMVERSION);
    if (!DmJsonItemObjectIsString(&versionItem)) {
        LOGE("PARAM_KEY_OS_VERSION error");
        DmJsonObjectDestroy(&extraInfoJson);
        return ERR_DM_FAILED;
    }
    DmJsonItemObjectGetToString(&versionItem, dmVersion);
    DmJsonObjectDestroy(&extraInfoJson);
    return 0;
}

bool DmDpConnectorIsLnnAcl(void* profilePtr)
{
    return DmShellIsLnnAcl(profilePtr);
}

void DmDpConnectorCacheAcerAclId(void* profile, DmVec_DmAclIdParam* aclInfos)
{
    DmAclIdParam param;
    DmAclIdParamInit(&param);
    DmShellCacheAcerAclId(profile, &param);
    DmVec_DmAclIdParam_Push(aclInfos, param);
}

void DmDpConnectorCacheAceeAclId(void* profile, DmVec_DmAclIdParam* aclInfos)
{
    DmAclIdParam param;
    DmAclIdParamInit(&param);
    DmShellCacheAceeAclId(profile, &param);
    DmVec_DmAclIdParam_Push(aclInfos, param);
}

int32_t DmDpConnectorIsSameAccount(const char* udid)
{
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmDpConnectorGetAccessControlProfile(&profiles);
    for (int i = 0; i < DmVecVoidSize(&profiles); i++) {
        void** item = DmVecVoidAt(&profiles, i);
        if (item == NULL) {
            continue;
        }
        DmString trustDevId = DmShellProfileGetTrustDeviceId(*item);
        int32_t status = DmShellProfileGetStatus(*item);
        if (DmStringCmpCstr(&trustDevId, udid) == 0 && status == DM_ACTIVE) {
            uint32_t bindType = DmShellProfileGetBindType(*item);
            DmStringDestroy(&trustDevId);
            if (bindType == DM_IDENTICAL_ACCOUNT_VAL) {
                DmVecVoidDestroy(&profiles);
                return 0;
            }
            continue;
        }
        DmStringDestroy(&trustDevId);
    }
    DmVecVoidDestroy(&profiles);
    return ERR_DM_FAILED;
}

int32_t DmDpConnectorCheckIsSameAccountByUdidHash(const char* udidHash)
{
    if (udidHash == NULL || strlen(udidHash) == 0) {
        LOGE("udidHash is empty!");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmDpConnectorGetAccessControlProfile(&profiles);
    for (int i = 0; i < DmVecVoidSize(&profiles); i++) {
        void** item = DmVecVoidAt(&profiles, i);
        if (item == NULL) {
            continue;
        }
        DmString trustDevId = DmShellProfileGetTrustDeviceId(*item);
        char hash[DM_MAX_DEVICE_ID_LEN] = {0};
        DmShellGetUdidHash(DmStringCstr(&trustDevId), (uint8_t*)hash);
        uint32_t bindType = DmShellProfileGetBindType(*item);
        if (strcmp(hash, udidHash) == 0 && bindType == DM_IDENTICAL_ACCOUNT_VAL) {
            DmStringDestroy(&trustDevId);
            DmVecVoidDestroy(&profiles);
            return 0;
        }
        DmStringDestroy(&trustDevId);
    }
    DmVecVoidDestroy(&profiles);
    return ERR_DM_VERIFY_SAME_ACCOUNT_FAILED;
}

int32_t DmDpConnectorGetTrustNumber(const char* deviceId)
{
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmDpConnectorGetAccessControlProfile(&profiles);
    int32_t trustNumber = 0;
    for (int i = 0; i < DmVecVoidSize(&profiles); i++) {
        void** item = DmVecVoidAt(&profiles, i);
        if (item == NULL) {
            continue;
        }
        DmString trustDevId = DmShellProfileGetTrustDeviceId(*item);
        int32_t status = DmShellProfileGetStatus(*item);
        if (DmStringCmpCstr(&trustDevId, deviceId) == 0 && status == DM_ACTIVE) {
            trustNumber++;
        }
        DmStringDestroy(&trustDevId);
    }
    DmVecVoidDestroy(&profiles);
    return trustNumber;
}

int32_t DmDpConnectorSubscribeDeviceProfileInited(void* dpInitedCallback)
{
    LOGI("In");
    return DmShellSubscribeDeviceProfileInited(dpInitedCallback);
}

int32_t DmDpConnectorUnSubscribeDeviceProfileInited(void)
{
    LOGI("In");
    return DmShellUnSubscribeDeviceProfileInited();
}

int32_t DmDpConnectorPutAllTrustedDevices(DmVecVoid* deviceInfos)
{
    LOGI("In deviceInfos.size:%{public}d", DmVecVoidSize(deviceInfos));
    return DmShellPutAllTrustedDevices(deviceInfos);
}

int32_t DmDpConnectorCheckDeviceInfoPermission(const char* localUdid, const char* peerDeviceId)
{
    LOGI("Start.");
    int32_t localUserId = 0;
    uint32_t tempLocalTokenId = 0;
    DmMultipleUserGetTokenIdAndForegroundUserId(&tempLocalTokenId, &localUserId);
    DmString localAccountId = DmMultipleUserGetOhosAccountIdByUserId(localUserId);
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmShellGetAccessControlProfileByUserId(&profiles, localUserId);
    for (int i = 0; i < DmVecVoidSize(&profiles); i++) {
        void** item = DmVecVoidAt(&profiles, i);
        if (item == NULL) {
            continue;
        }
        if (DmDpConnectorIsLnnAcl(*item)) {
            continue;
        }
        DmString trustDevId = DmShellProfileGetTrustDeviceId(*item);
        if (DmStringCmpCstr(&trustDevId, peerDeviceId) == 0) {
            uint32_t bindType = DmShellProfileGetBindType(*item);
            uint32_t bindLevel = DmShellProfileGetBindLevel(*item);
            DmStringDestroy(&trustDevId);
            if (bindType == DM_IDENTICAL_ACCOUNT_VAL || bindLevel == DM_BIND_LEVEL_USER) {
                DmVecVoidDestroy(&profiles);
                DmStringDestroy(&localAccountId);
                return 0;
            }
            continue;
        }
        DmStringDestroy(&trustDevId);
    }
    DmVecVoidDestroy(&profiles);
    DmStringDestroy(&localAccountId);
    return ERR_DM_NO_PERMISSION;
}

void DmDpConnectorDeleteAccessControlListByUdid(const char* udid)
{
    DmString udidStr = DmStringCreate(udid);
    DmString anonUdid = DmGetAnonyString(&udidStr);
    LOGI("Udid: %{public}s.", DmStringCstr(&anonUdid));
    DmStringDestroy(&anonUdid);
    DmStringDestroy(&udidStr);
    if (udid == NULL || strlen(udid) == 0) {
        LOGE("udid is empty.");
        return;
    }
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmDpConnectorGetAccessControlProfile(&profiles);
    for (int i = 0; i < DmVecVoidSize(&profiles); i++) {
        void** item = DmVecVoidAt(&profiles, i);
        if (item == NULL) {
            continue;
        }
        DmString trustDevId = DmShellProfileGetTrustDeviceId(*item);
        if (DmStringCmpCstr(&trustDevId, udid) == 0) {
            DmShellDeleteAccessControlProfile(DmStringCstr(&trustDevId));
        }
        DmStringDestroy(&trustDevId);
    }
    DmVecVoidDestroy(&profiles);
}

void DmDpConnectorDeleteAccessControlById(const char* trustDeviceId)
{
    DmShellDeleteAccessControlProfile(trustDeviceId);
}

DmString DmDpConnectorAccessToStr(void* acl)
{
    return DmShellAccessToStr(acl);
}

int32_t DmDpConnectorHandleUserStop(int32_t stopUserId, const char* stopEventUdid)
{
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmDpConnectorGetAllAclIncludeLnnAcl(&profiles);
    for (int i = 0; i < DmVecVoidSize(&profiles); i++) {
        void** item = DmVecVoidAt(&profiles, i);
        if (item == NULL) {
            continue;
        }
        DmString acerDevId = DmShellProfileGetAccesserDeviceId(*item);
        int32_t acerUserId = DmShellProfileGetAccesserUserId(*item);
        int32_t status = DmShellProfileGetStatus(*item);
        DmString aceeDevId = DmShellProfileGetAccesseeDeviceId(*item);
        int32_t aceeUserId = DmShellProfileGetAccesseeUserId(*item);
        if ((DmStringCmpCstr(&acerDevId, stopEventUdid) == 0 && acerUserId == stopUserId && status == DM_ACTIVE) ||
            (DmStringCmpCstr(&aceeDevId, stopEventUdid) == 0 && aceeUserId == stopUserId && status == DM_ACTIVE)) {
            DmShellProfileSetStatus(*item, DM_INACTIVE);
            DmShellUpdateAccessControlProfile(*item);
        }
        DmStringDestroy(&acerDevId);
        DmStringDestroy(&aceeDevId);
    }
    DmVecVoidDestroy(&profiles);
    return 0;
}

bool DmDpConnectorCheckUserIdIsForegroundUserId(int32_t userId)
{
    DmVec_int userVec;
    DmVec_int_Init(&userVec);
    int32_t ret = DmMultipleUserGetForegroundUserIds(&userVec);
    if (ret != 0) {
        LOGE("GetForegroundUserIds failed, ret = %{public}d", ret);
        DmVec_int_Destroy(&userVec);
        return false;
    }
    for (int i = 0; i < DmVec_int_Size(&userVec); i++) {
        int32_t* val = DmVec_int_At(&userVec, i);
        if (val != NULL && *val == userId) {
            DmVec_int_Destroy(&userVec);
            return true;
        }
    }
    LOGE("userId = %{public}d is not foregroundUserId.", userId);
    DmVec_int_Destroy(&userVec);
    return false;
}

DmDeviceProfileConnector* DmDpConnectorCreateInstance(void)
{
    return DmDeviceProfileConnectorGetInstance();
}

int32_t DmDpConnectorGetAppTrustDeviceList(DmHmap_DmString_DmAuthForm* resultMap,
    const char* pkgName, const char* deviceId)
{
    DmShellGetAppTrustDeviceListC(resultMap, pkgName, deviceId);
    return 0;
}

// These functions are implemented in deviceprofile_connector_shell.cpp
void DmHmap_DmString_DmAuthForm_Clear(DmHmap_DmString_DmAuthForm* m)
{
    for (int i = 0; i < m->cap; i++) {
        if (m->data[i].state == 1) {
            DmStringDestroy(&m->data[i].key);
        }
    }
    m->size = 0;
}
void DmHmap_DmString_DmAuthForm_Destroy(DmHmap_DmString_DmAuthForm* m)
{
    DmHmap_DmString_DmAuthForm_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}
