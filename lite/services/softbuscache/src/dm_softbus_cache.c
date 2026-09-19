/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "dm_softbus_cache.h"

#include <stdlib.h>
#include <string.h>
#include <securec.h>
#include <stdio.h>
#include "cJSON.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info_c.h"
#include "dm_log.h"

DM_IMPLEMENT_SINGLE_INSTANCE(SoftbusCache);

DM_HMAP_IMPL(DmString_DmDeviceInfoPair, DmString, DmPair_DmString_DmDeviceInfo, DmHashDmString, DmCmpDmString)
DM_HMAP_IMPL(DmString_int, DmString, int, DmHashDmString, DmCmpDmString)

static void DmSoftbusCacheDeepCopyDeviceInfo(DmDeviceInfo* dst, const DmDeviceInfo* src)
{
    (void)memcpy_s(dst->deviceId, sizeof(dst->deviceId), src->deviceId, sizeof(dst->deviceId));
    (void)memcpy_s(dst->deviceName, sizeof(dst->deviceName), src->deviceName, sizeof(dst->deviceName));
    dst->deviceTypeId = src->deviceTypeId;
    (void)memcpy_s(dst->networkId, sizeof(dst->networkId), src->networkId, sizeof(dst->networkId));
    dst->range = src->range;
    dst->networkType = src->networkType;
    dst->authForm = src->authForm;
    dst->extraData = DmStringCopy(&src->extraData);
}

void DmSoftbusCacheInit(SoftbusCache* cache)
{
    DmMutexInit(&cache->deviceInfosMutex);
    DmMutexInit(&cache->deviceSecurityLevelMutex);
    DmMutexInit(&cache->localDevInfoMutex);
    DmHmap_DmString_DmDeviceInfoPair_Init(&cache->deviceInfo);
    DmHmap_DmString_int_Init(&cache->deviceSecurityLevel);
    DmDeviceInfoInit(&cache->localDeviceInfo);
    cache->online = false;
    cache->getLocalDevInfo = false;
}

static int32_t DmSoftbusCacheGetUdidByNetworkId(const char* networkId, DmString* udid)
{
    uint8_t mUdid[UDID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NODE_KEY_UDID, mUdid, sizeof(mUdid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %{public}d.", ret);
        return ret;
    }
    *udid = DmStringCreate((char*)mUdid);
    return ret;
}

static int32_t DmSoftbusCacheGetUuidByNetworkId(const char* networkId, DmString* uuid)
{
    uint8_t mUuid[UUID_BUF_LEN] = {0};
    int32_t ret = GetNodeKeyInfo(DM_PKG_NAME, networkId, NODE_KEY_UUID, mUuid, sizeof(mUuid));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo failed, ret: %{public}d.", ret);
        return ret;
    }
    *uuid = DmStringCreate((char*)mUuid);
    return ret;
}

static int32_t DmSoftbusCacheConvertNodeBasicInfoToDmDevice(const NodeBasicInfo* nodeInfo,
    DmDeviceInfo* devInfo)
{
    DmDeviceInfoInit(devInfo);

    if (memcpy_s(devInfo->networkId, sizeof(devInfo->networkId), nodeInfo->networkId,
        sizeof(nodeInfo->networkId)) != DM_OK) {
        LOGE("copy networkId data failed.");
        DmDeviceInfoDestroy(devInfo);
        return ERR_DM_FAILED;
    }

    if (memcpy_s(devInfo->deviceName, sizeof(devInfo->deviceName), nodeInfo->deviceName,
        sizeof(nodeInfo->deviceName)) != DM_OK) {
        LOGE("copy deviceName data failed.");
        DmDeviceInfoDestroy(devInfo);
        return ERR_DM_FAILED;
    }

    devInfo->deviceTypeId = nodeInfo->deviceTypeId;

    cJSON* extraJson = cJSON_CreateObject();
    if (extraJson == NULL) {
        LOGE("cJSON_CreateObject failed.");
        DmDeviceInfoDestroy(devInfo);
        return ERR_DM_FAILED;
    }
    cJSON_AddNumberToObject(extraJson, DM_PARAM_KEY_OS_TYPE, (double)nodeInfo->osType);
    DmString osVersionStr = DmConvertCharArrayToString(nodeInfo->osVersion, OS_VERSION_BUF_LEN);
    cJSON_AddStringToObject(extraJson, DM_PARAM_KEY_OS_VERSION, DmStringCstr(&osVersionStr));
    DmStringDestroy(&osVersionStr);
    char* extraStr = cJSON_PrintUnformatted(extraJson);
    if (extraStr != NULL) {
        DmStringDestroy(&devInfo->extraData);
        devInfo->extraData = DmStringCreate(extraStr);
        cJSON_free(extraStr);
    }
    cJSON_Delete(extraJson);
    return DM_OK;
}

static int32_t DmSoftbusCacheGetDevInfoFromBus(const DmString* networkId, DmDeviceInfo* devInfo)
{
    int32_t nodeInfoCount = 0;
    NodeBasicInfo* nodeInfo = NULL;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &nodeInfoCount);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %{public}d.", ret);
        return ret;
    }
    for (int32_t i = 0; i < nodeInfoCount; ++i) {
        NodeBasicInfo* nodeBasicInfo = nodeInfo + i;
        if (strcmp(DmStringCstr(networkId), nodeBasicInfo->networkId) == 0) {
            DmSoftbusCacheConvertNodeBasicInfoToDmDevice(nodeBasicInfo, devInfo);
            break;
        }
    }
    FreeNodeInfo(nodeInfo);
    DmString _anon_devExtra = DmGetAnonyString(&devInfo->extraData);
    LOGI("GetDeviceInfo complete, deviceName : %{public}s, deviceTypeId : %{public}d.",
        DmStringCstr(&_anon_devExtra), devInfo->deviceTypeId);
    DmStringDestroy(&_anon_devExtra);
    return ret;
}

void DmSoftbusCacheSaveLocalDeviceInfo(SoftbusCache* cache)
{
    LOGI("start");
    DmMutexLock(&cache->localDevInfoMutex);
    if (cache->online) {
        DmMutexUnlock(&cache->localDevInfoMutex);
        return;
    }
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetLocalNodeDeviceInfo failed, ret: %{public}d.", ret);
        DmMutexUnlock(&cache->localDevInfoMutex);
        return;
    }
    DmDeviceInfoDestroy(&cache->localDeviceInfo);
    DmSoftbusCacheConvertNodeBasicInfoToDmDevice(&nodeBasicInfo, &cache->localDeviceInfo);
    DmString networkIdStr = DmStringCreate(cache->localDeviceInfo.networkId);
    DmString _anon_1 = DmGetAnonyString(&networkIdStr);
    LOGI("networkid %{public}s.", DmStringCstr(&_anon_1));
    DmStringDestroy(&_anon_1);
    DmStringDestroy(&networkIdStr);
    DmSoftbusCacheSaveDeviceInfo(cache, &cache->localDeviceInfo);
    DmSoftbusCacheSaveDeviceSecurityLevel(cache, cache->localDeviceInfo.networkId);
    cache->online = true;
    cache->getLocalDevInfo = true;
    DmMutexUnlock(&cache->localDevInfoMutex);
}

static void DmCacheLookupUdidUuid(SoftbusCache* cache, const DmDeviceInfo* deviceInfo, DmString* udid, DmString* uuid)
{
    (void)cache;
    if (deviceInfo->networkId[0] == '\0') {
        LOGE("networkId is empty.");
        return;
    }
    DmSoftbusCacheGetUdidByNetworkId(deviceInfo->networkId, udid);
    DmSoftbusCacheGetUuidByNetworkId(deviceInfo->networkId, uuid);
    if (DmStringEmpty(udid)) {
        LOGE("udid is empty.");
        return;
    }
}

static bool DmPrepareCacheEntry(SoftbusCache* cache, const DmDeviceInfo* deviceInfo,
    DmString* udid, DmString* uuid, DmDeviceInfo* devInfoCopy)
{
    DmCacheLookupUdidUuid(cache, deviceInfo, udid, uuid);
    if (DmStringEmpty(udid)) {
        return false;
    }
    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (DmGetUdidHashBuf(udid, (unsigned char*)udidHash) != DM_OK) {
        DmString anon = DmGetAnonyString(udid);
        LOGE("get udidhash by udid: %{public}s failed.", DmStringCstr(&anon));
        DmStringDestroy(&anon);
        return false;
    }
    DmDeviceInfoInit(devInfoCopy);
    DmSoftbusCacheDeepCopyDeviceInfo(devInfoCopy, deviceInfo);
    if (memcpy_s(devInfoCopy->deviceId, sizeof(devInfoCopy->deviceId), udidHash,
        sizeof(devInfoCopy->deviceId) < sizeof(udidHash) ? sizeof(devInfoCopy->deviceId) : sizeof(udidHash)) != DM_OK) {
        LOGE("copy deviceId failed.");
        DmDeviceInfoDestroy(devInfoCopy);
        return false;
    }
    return true;
}

static bool DmInsertCacheEntry(SoftbusCache* cache, DmString* udid, DmString* uuid, DmDeviceInfo* devInfoCopy)
{
    DmMutexLock(&cache->deviceInfosMutex);
    if (cache->deviceInfo.size >= DM_MAX_CONTAINER_SIZE) {
        LOGE("container size is more than max size");
        DmMutexUnlock(&cache->deviceInfosMutex);
        return false;
    }
    DmPair_DmString_DmDeviceInfo pair;
    pair.first = *uuid;
    pair.second = *devInfoCopy;
    DmHmap_DmString_DmDeviceInfoPair_Insert(&cache->deviceInfo, *udid, pair);
    DmMutexUnlock(&cache->deviceInfosMutex);
    return true;
}

void DmSoftbusCacheSaveDeviceInfo(SoftbusCache* cache, const DmDeviceInfo* deviceInfo)
{
    LOGI("start");
    DmString udid = DmStringCreateEmpty();
    DmString uuid = DmStringCreateEmpty();
    DmDeviceInfo devInfoCopy;
    if (!DmPrepareCacheEntry(cache, deviceInfo, &udid, &uuid, &devInfoCopy)) {
        DmStringDestroy(&udid);
        DmStringDestroy(&uuid);
        return;
    }
    if (!DmInsertCacheEntry(cache, &udid, &uuid, &devInfoCopy)) {
        DmStringDestroy(&udid);
        DmStringDestroy(&uuid);
        DmDeviceInfoDestroy(&devInfoCopy);
        return;
    }
    DmString networkIdStr = DmStringCreate(deviceInfo->networkId);
    DmString anon1 = DmGetAnonyString(&udid);
    DmString anon2 = DmGetAnonyString(&networkIdStr);
    LOGI("success udid %{public}s, networkId %{public}s", DmStringCstr(&anon1), DmStringCstr(&anon2));
    DmStringDestroy(&anon1);
    DmStringDestroy(&anon2);
    DmStringDestroy(&networkIdStr);
}

void DmSoftbusCacheDeleteDeviceInfoByNode(SoftbusCache* cache, const DmDeviceInfo* nodeInfo)
{
    DmString networkIdStr = DmStringCreate(nodeInfo->networkId);
    DmString _anon_6 = DmGetAnonyString(&networkIdStr);
    LOGI("networkId %{public}s", DmStringCstr(&_anon_6));
    DmStringDestroy(&_anon_6);
    DmStringDestroy(&networkIdStr);
    DmMutexLock(&cache->deviceInfosMutex);
    for (int i = 0; i < cache->deviceInfo.cap; i++) {
        if (cache->deviceInfo.data[i].state == 1) {
            if (strcmp(cache->deviceInfo.data[i].val.second.networkId, nodeInfo->networkId) == 0) {
                DmString key = cache->deviceInfo.data[i].key;
    DmString _anon_7 = DmGetAnonyString(&key);
                LOGI("success udid %{public}s", DmStringCstr(&_anon_7));
    DmStringDestroy(&_anon_7);
                DmStringDestroy(&cache->deviceInfo.data[i].val.first);
                DmDeviceInfoDestroy(&cache->deviceInfo.data[i].val.second);
                DmHmap_DmString_DmDeviceInfoPair_Erase(&cache->deviceInfo, key);
                break;
            }
        }
    }
    DmMutexUnlock(&cache->deviceInfosMutex);
}

bool DmSoftbusCacheGetDeviceInfoByDeviceId(SoftbusCache* cache, const DmString* deviceId,
    DmString* uuid, DmDeviceInfo* devInfo)
{
    DmString _anon_10 = DmGetAnonyString(deviceId);
    LOGI("deviceId: %{public}s", DmStringCstr(&_anon_10));
    DmStringDestroy(&_anon_10);
    DmMutexLock(&cache->deviceInfosMutex);
    DmPair_DmString_DmDeviceInfo* found = DmHmap_DmString_DmDeviceInfoPair_Find(&cache->deviceInfo, *deviceId);
    if (found != NULL) {
        *uuid = DmStringCopy(&found->first);
        DmSoftbusCacheDeepCopyDeviceInfo(devInfo, &found->second);
        DmString _anon_11 = DmGetAnonyString(uuid);
        DmString _anon_12 = DmGetAnonyString(deviceId);
        LOGI("uuid %{public}s, udid %{public}s",
            DmStringCstr(&_anon_11), DmStringCstr(&_anon_12));
        DmStringDestroy(&_anon_11);
        DmStringDestroy(&_anon_12);
        DmMutexUnlock(&cache->deviceInfosMutex);
        return true;
    }
    DmMutexUnlock(&cache->deviceInfosMutex);
    return false;
}

void DmSoftbusCacheUpdateDeviceInfoCache(SoftbusCache* cache)
{
    LOGI("start");
    int32_t deviceCount = 0;
    NodeBasicInfo* nodeInfo = NULL;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &deviceCount);
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]GetAllNodeDeviceInfo failed, ret: %{public}d.", ret);
        return;
    }
    DmSoftbusCacheSaveLocalDeviceInfo(cache);
    for (int32_t i = 0; i < deviceCount; ++i) {
        NodeBasicInfo* nodeBasicInfo = nodeInfo + i;
        DmDeviceInfo deviceInfo;
        DmDeviceInfoInit(&deviceInfo);
        DmSoftbusCacheConvertNodeBasicInfoToDmDevice(nodeBasicInfo, &deviceInfo);
        DmSoftbusCacheSaveDeviceInfo(cache, &deviceInfo);
        DmDeviceInfoDestroy(&deviceInfo);
    }
    FreeNodeInfo(nodeInfo);
    LOGI("success, deviceCount: %{public}d.", deviceCount);
}

void DmSoftbusCacheSaveDeviceSecurityLevel(SoftbusCache* cache, const char* networkId)
{
    DmString networkIdStr = DmStringCreate(networkId);
    DmString _anon_21 = DmGetAnonyString(&networkIdStr);
    LOGI("networkId %{public}s.", DmStringCstr(&_anon_21));
    DmStringDestroy(&_anon_21);
    DmMutexLock(&cache->deviceSecurityLevelMutex);
    int* found = DmHmap_DmString_int_Find(&cache->deviceSecurityLevel, networkIdStr);
    if (found != NULL) {
        DmStringDestroy(&networkIdStr);
        DmMutexUnlock(&cache->deviceSecurityLevelMutex);
        return;
    }
    if (cache->deviceSecurityLevel.size >= DM_MAX_CONTAINER_SIZE) {
        LOGE("container size is more than max size");
        DmStringDestroy(&networkIdStr);
        DmMutexUnlock(&cache->deviceSecurityLevelMutex);
        return;
    }
    int32_t tempSecurityLevel = -1;
    if (GetNodeKeyInfo(DM_PKG_NAME, networkId, NODE_KEY_DEVICE_SECURITY_LEVEL,
        (uint8_t*)&tempSecurityLevel, LNN_COMMON_LEN) != DM_OK) {
        LOGE("[SOFTBUS]GetNodeKeyInfo networkType failed.");
        DmStringDestroy(&networkIdStr);
        DmMutexUnlock(&cache->deviceSecurityLevelMutex);
        return;
    }
    DmHmap_DmString_int_Insert(&cache->deviceSecurityLevel, networkIdStr, tempSecurityLevel);
    DmMutexUnlock(&cache->deviceSecurityLevelMutex);
}

void DmSoftbusCacheDeleteDeviceSecurityLevel(SoftbusCache* cache, const char* networkId)
{
    DmString networkIdStr = DmStringCreate(networkId);
    DmString _anon_22 = DmGetAnonyString(&networkIdStr);
    LOGI("networkId %{public}s.", DmStringCstr(&_anon_22));
    DmStringDestroy(&_anon_22);
    DmStringDestroy(&networkIdStr);
    DmMutexLock(&cache->deviceSecurityLevelMutex);
    DmString eraseKey = DmStringCreate(networkId);
    DmHmap_DmString_int_Erase(&cache->deviceSecurityLevel, eraseKey);
    DmStringDestroy(&eraseKey);
    DmMutexUnlock(&cache->deviceSecurityLevelMutex);
}

int32_t DmSoftbusCacheGetDevInfoByNetworkId(SoftbusCache* cache, const DmString* networkId,
    DmDeviceInfo* nodeInfo)
{
    DmMutexLock(&cache->deviceInfosMutex);
    for (int i = 0; i < cache->deviceInfo.cap; i++) {
        if (cache->deviceInfo.data[i].state == 1) {
            if (strcmp(cache->deviceInfo.data[i].val.second.networkId, DmStringCstr(networkId)) == 0) {
                DmSoftbusCacheDeepCopyDeviceInfo(nodeInfo, &cache->deviceInfo.data[i].val.second);
                DmString _anon_24 = DmGetAnonyString(networkId);
                DmString _anon_key = DmGetAnonyString(&cache->deviceInfo.data[i].key);
                LOGI("success networkId %{public}s, udid %{public}s.",
                    DmStringCstr(&_anon_24),
                    DmStringCstr(&_anon_key));
                DmStringDestroy(&_anon_24);
                DmStringDestroy(&_anon_key);
                DmMutexUnlock(&cache->deviceInfosMutex);
                return DM_OK;
            }
        }
    }
    DmMutexUnlock(&cache->deviceInfosMutex);
    int32_t ret = DmSoftbusCacheGetDevInfoFromBus(networkId, nodeInfo);
    if (ret != DM_OK) {
        LOGE("GetDevInfoFromBus failed.");
        return ret;
    }
    DmSoftbusCacheSaveDeviceInfo(cache, nodeInfo);
    return DM_OK;
}

int32_t DmSoftbusCacheGetUdidByUdidHash(SoftbusCache* cache, const DmString* udidHash, DmString* udid)
{
    DmString _anon_25 = DmGetAnonyString(udidHash);
    LOGI("udidHash %{public}s.", DmStringCstr(&_anon_25));
    DmStringDestroy(&_anon_25);
    DmMutexLock(&cache->deviceInfosMutex);
    for (int i = 0; i < cache->deviceInfo.cap; i++) {
        if (cache->deviceInfo.data[i].state == 1) {
            if (strcmp(cache->deviceInfo.data[i].val.second.deviceId, DmStringCstr(udidHash)) == 0) {
                *udid = DmStringCopy(&cache->deviceInfo.data[i].key);
    DmString _anon_26 = DmGetAnonyString(udid);
                LOGI("success udid %{public}s.", DmStringCstr(&_anon_26));
    DmStringDestroy(&_anon_26);
                DmMutexUnlock(&cache->deviceInfosMutex);
                return DM_OK;
            }
        }
    }
    DmMutexUnlock(&cache->deviceInfosMutex);
    DmString _anon_27 = DmGetAnonyString(udidHash);
    LOGI("failed udidHash %{public}s.", DmStringCstr(&_anon_27));
    DmStringDestroy(&_anon_27);
    return ERR_DM_FAILED;
}

int32_t DmSoftbusCacheGetNetworkIdFromCache(SoftbusCache* cache, const DmString* udid, DmString* networkId)
{
    DmString _anon_30 = DmGetAnonyString(udid);
    LOGI("udid %{public}s.", DmStringCstr(&_anon_30));
    DmStringDestroy(&_anon_30);
    DmMutexLock(&cache->deviceInfosMutex);
    DmPair_DmString_DmDeviceInfo* found = DmHmap_DmString_DmDeviceInfoPair_Find(&cache->deviceInfo, *udid);
    if (found != NULL) {
        *networkId = DmStringCreate(found->second.networkId);
        DmString _anon_31 = DmGetAnonyString(networkId);
        DmString _anon_32 = DmGetAnonyString(udid);
        LOGI("success networkId %{public}s, udid %{public}s.",
            DmStringCstr(&_anon_31), DmStringCstr(&_anon_32));
        DmStringDestroy(&_anon_31);
        DmStringDestroy(&_anon_32);
        DmMutexUnlock(&cache->deviceInfosMutex);
        return DM_OK;
    }
    DmMutexUnlock(&cache->deviceInfosMutex);
    DmString _anon_33 = DmGetAnonyString(udid);
    LOGI("failed udid %{public}s.", DmStringCstr(&_anon_33));
    DmStringDestroy(&_anon_33);
    return ERR_DM_FAILED;
}

bool DmSoftbusCacheCheckIsOnlineByPeerUdid(SoftbusCache* cache, const DmString* peerUdid)
{
    DmMutexLock(&cache->deviceInfosMutex);
    DmPair_DmString_DmDeviceInfo* found = DmHmap_DmString_DmDeviceInfoPair_Find(&cache->deviceInfo, *peerUdid);
    if (found != NULL) {
    DmString _anon_37 = DmGetAnonyString(peerUdid);
        LOGI("peerUdid %{public}s is online.", DmStringCstr(&_anon_37));
    DmStringDestroy(&_anon_37);
        DmMutexUnlock(&cache->deviceInfosMutex);
        return true;
    }
    DmMutexUnlock(&cache->deviceInfosMutex);
    DmString _anon_38 = DmGetAnonyString(peerUdid);
    LOGI("peerUdid %{public}s is not online.", DmStringCstr(&_anon_38));
    DmStringDestroy(&_anon_38);
    return false;
}
