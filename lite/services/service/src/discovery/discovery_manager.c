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


#include "discovery_manager_c.h"
#include <stdlib.h>
#include <string.h>
#include "securec.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_constants.h"
#include "discovery_filter.h"
#include "softbus_listener_c.h"

static const uint16_t DM_INVALID_FLAG_ID = 0;

DM_IMPLEMENT_SINGLE_INSTANCE(DiscoveryManager)

DM_MAP_IMPL(DmString_MultiUserDiscovery, DmString, MultiUserDiscovery, DmCmpDmString)
DM_MAP_IMPL(DmString_DiscoveryContext, DmString, DiscoveryContext, DmCmpDmString)

DmString DmDiscoveryManagerAddMultiUserIdentify(DiscoveryManager* mgr, const DmString* pkgName);
DmString DmDiscoveryManagerRemoveMultiUserIdentify(DiscoveryManager* mgr, const DmString* pkgName);
void DmDiscoveryManagerUpdateInfoFreq(DiscoveryManager* mgr,
    const DmMap_DmString_DmString* discoverParam, DmSubscribeInfo* dmSubInfo);
void DmDiscoveryManagerUpdateInfoMedium(DiscoveryManager* mgr,
    const DmMap_DmString_DmString* discoverParam, DmSubscribeInfo* dmSubInfo);

int32_t DmDiscoveryManagerInit(DiscoveryManager* mgr, void* softbusListener, void* listener)
{
    CHECK_NULL_RETURN(mgr, ERR_DM_FAILED);
    DmMutexInit(&mgr->locks);
    DmMutexInit(&mgr->subIdMapLocks);
    DmMutexInit(&mgr->timerLocks);
    DmMutexInit(&mgr->capabilityMapLocks);
    DmMutexInit(&mgr->multiUserDiscLocks);
    mgr->timer = NULL;
    mgr->softbusListener = softbusListener;
    mgr->listener = listener;
    DmMap_DmString_DmMap_uint16_uint16_Init(&mgr->pkgName2SubIdMap);
    DmMap_DmString_DiscoveryContext_Init(&mgr->discoveryContextMap);
    DmSetDmStringInit(&mgr->pkgNameSet);
    DmMap_DmString_DmString_Init(&mgr->capabilityMap);
    DmMap_DmString_MultiUserDiscovery_Init(&mgr->multiUserDiscMap);
    DmSet_uint16_t_Init(&mgr->randSubIdSet);
    LOGI("DiscoveryManager init.");
    return DM_OK;
}

int32_t DmDiscoveryManagerEnableDiscoveryListener(DiscoveryManager* mgr, const DmString* pkgName,
    const DmMap_DmString_DmString* discoverParam, const DmMap_DmString_DmString* filterOptions)
{
    CHECK_NULL_RETURN(mgr, ERR_DM_FAILED);
    if (DmStringEmpty(pkgName)) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("begin for pkgName.");
    DmString pkgNameTemp = DmDiscoveryManagerAddMultiUserIdentify(mgr, pkgName);
    DmSubscribeInfo dmSubInfo;
    dmSubInfo.subscribeId = DM_INVALID_FLAG_ID;
    dmSubInfo.mode = DM_DISCOVER_MODE_PASSIVE;
    dmSubInfo.freq = DM_LOW;
    dmSubInfo.isSameAccount = false;
    dmSubInfo.isWakeRemote = false;
    (void)strncpy_s(dmSubInfo.capability, sizeof(dmSubInfo.capability),
        DM_CAPABILITY_APPROACH, sizeof(dmSubInfo.capability) - 1);
    DmDiscoveryManagerUpdateInfoFreq(mgr, discoverParam, &dmSubInfo);
    DmDiscoveryManagerUpdateInfoMedium(mgr, discoverParam, &dmSubInfo);
    {
        DmMutexLock(&mgr->capabilityMapLocks);
        DmMap_DmString_DmString_Insert(&mgr->capabilityMap, pkgNameTemp, DmStringCreate(dmSubInfo.capability));
        DmMutexUnlock(&mgr->capabilityMapLocks);
    }
    int32_t ret = DmSoftbusListenerRefreshSoftbusLnn(DM_PKG_NAME, &dmSubInfo, NULL);
    if (ret != DM_OK) {
        LOGE("softbus refresh lnn ret: %d.", ret);
        DmStringDestroy(&pkgNameTemp);
        return ret;
    }
    DmSoftbusListenerRegisterSoftbusLnnOpsCbk(&pkgNameTemp);
    DmStringDestroy(&pkgNameTemp);
    return DM_OK;
}

int32_t DmDiscoveryManagerDisableDiscoveryListener(DiscoveryManager* mgr, const DmString* pkgName,
    const DmMap_DmString_DmString* extraParam)
{
    CHECK_NULL_RETURN(mgr, ERR_DM_FAILED);
    if (DmStringEmpty(pkgName)) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("begin for pkgName.");
    DmString pkgNameTemp = DmDiscoveryManagerRemoveMultiUserIdentify(mgr, pkgName);
    {
        DmMutexLock(&mgr->capabilityMapLocks);
        DmMap_DmString_DmString_Erase(&mgr->capabilityMap, pkgNameTemp);
        DmMutexUnlock(&mgr->capabilityMapLocks);
    }
    DmSoftbusListenerUnregisterSoftbusLnnOpsCbk(&pkgNameTemp);
    DmStringDestroy(&pkgNameTemp);
    return DM_OK;
}

void DmDiscoveryManagerUpdateInfoFreq(DiscoveryManager* mgr,
    const DmMap_DmString_DmString* discoverParam, DmSubscribeInfo* dmSubInfo)
{
    DmString keyFreq = DmStringCreate(DM_PARAM_KEY_DISC_FREQ);
    DmString* freqStrPtr = DmMap_DmString_DmString_Find((DmMap_DmString_DmString*)discoverParam, keyFreq);
    DmStringDestroy(&keyFreq);
    if (freqStrPtr != NULL) {
        int32_t freq = atoi(DmStringCstr(freqStrPtr));
        if (freq < DM_LOW || freq > DM_FREQ_BUTT) {
            LOGE("Invalid freq value.");
            return;
        }
        dmSubInfo->freq = freq;
    }
}

void DmDiscoveryManagerUpdateInfoMedium(DiscoveryManager* mgr,
    const DmMap_DmString_DmString* discoverParam, DmSubscribeInfo* dmSubInfo)
{
    dmSubInfo->medium = DM_COAP;
    DmString keyMedium = DmStringCreate(DM_PARAM_KEY_DISC_MEDIUM);
    DmString* mediumStrPtr = DmMap_DmString_DmString_Find((DmMap_DmString_DmString*)discoverParam, keyMedium);
    DmStringDestroy(&keyMedium);
    if (mediumStrPtr != NULL) {
        int32_t medium = atoi(DmStringCstr(mediumStrPtr));
        dmSubInfo->medium = medium;
    }
}

DmString DmDiscoveryManagerAddMultiUserIdentify(DiscoveryManager* mgr, const DmString* pkgName)
{
    DmString result = DmStringCreateEmpty();
    if (!DmStringEmpty(pkgName)) {
        DmStringAssign(&result, pkgName);
    }
    return result;
}

DmString DmDiscoveryManagerRemoveMultiUserIdentify(DiscoveryManager* mgr, const DmString* pkgName)
{
    DmString result = DmStringCreateEmpty();
    if (!DmStringEmpty(pkgName)) {
        DmStringAssign(&result, pkgName);
    }
    return result;
}

int32_t DmDiscoveryManagerStartDiscovering(DiscoveryManager* mgr, const DmString* pkgName,
    const DmMap_DmString_DmString* discoverParam, const DmMap_DmString_DmString* filterOptions)
{
    CHECK_NULL_RETURN(mgr, ERR_DM_FAILED);
    if (DmStringEmpty(pkgName)) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("StartDiscovering.");
    return DM_OK;
}

int32_t DmDiscoveryManagerStopDiscovering(DiscoveryManager* mgr, const DmString* pkgName, uint16_t subscribeId)
{
    CHECK_NULL_RETURN(mgr, ERR_DM_FAILED);
    if (DmStringEmpty(pkgName)) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("StopDiscovering.");
    return DM_OK;
}
