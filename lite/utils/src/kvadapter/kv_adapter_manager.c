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


#include "kv_adapter_manager.h"

#include <stdlib.h>
#include <string.h>

#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"

#define DM_KV_STORE_PREFIX "DM2_"
#define DM_KV_STORE_FREEZE_PREFIX "anti_ddos_local_"
#define DM_DB_KEY_DELIMITER "###"
#define DM_KV_STORE_REFRESH_TIME (24 * 60 * 60)
#define DM_MAX_SUPPORTED_EXIST_TIME (3 * 24 * 60 * 60)
#define DM_OSTYPE_PREFIX "ostype"
#define DM_UDID_PREFIX "udid"
const int DM_BASE_TEN = 10;

DM_IMPLEMENT_SINGLE_INSTANCE(KVAdapterManager);

static DmString DmComposeOsTypePrefix(void)
{
    DmString dmKey = DmStringCreate(DM_OSTYPE_PREFIX);
    DmStringAppend(&dmKey, DM_DB_KEY_DELIMITER);
    DmStringAppend(&dmKey, DM_UDID_PREFIX);
    DmStringAppend(&dmKey, DM_DB_KEY_DELIMITER);
    return dmKey;
}

static bool DmIsTimeOut(int64_t sourceTime, int64_t targetTime, int64_t timeOut)
{
    return (targetTime - sourceTime >= timeOut);
}

DM_EXPORT int32_t DmKvAdapterManagerInit(void)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmMutexLock(&mgr->idCacheMapMtx);
    (void)DmHmap_DmString_DmString_Clear(&mgr->idCacheMap);
    DmMutexUnlock(&mgr->idCacheMapMtx);
    int32_t ret = DM_OK;
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        mgr->kvAdapter = DmKvAdapterCreate();
        ret = DmKvAdapterInit(mgr->kvAdapter);
    }
    DmMutexUnlock(&mgr->kvAdapterMtx);
    return ret;
}

DM_EXPORT void DmKvAdapterManagerUninit(void)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        return;
    }
    DmKvAdapterUninit(mgr->kvAdapter);
    DmKvAdapterDestroy(mgr->kvAdapter);
    mgr->kvAdapter = NULL;
    DmMutexUnlock(&mgr->kvAdapterMtx);
}

DM_EXPORT void DmKvAdapterManagerReinit(void)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        return;
    }
    DmKvAdapterReinit(mgr->kvAdapter);
    DmMutexUnlock(&mgr->kvAdapterMtx);
}

static int32_t DmPutToAdapter(KVAdapterManager* mgr, const DmString* key, const DmString* valueStr)
{
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        return ERR_DM_POINT_NULL;
    }
    if (DmKvAdapterPut(mgr->kvAdapter, DmStringCstr(key), DmStringCstr(valueStr)) != DM_OK) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        return ERR_DM_FAILED;
    }
    DmMutexUnlock(&mgr->kvAdapterMtx);
    return DM_OK;
}

int32_t DmKvAdapterManagerPutByAnoyDeviceId(const char* key, const DmKVValue* value)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmString dmKey = DmStringCreate(DM_KV_STORE_PREFIX);
    DmStringAppend(&dmKey, key);
    DmString prefixKey = DmStringCreate(DM_KV_STORE_PREFIX);
    DmStringAppend(&prefixKey, DmStringCstr(&value->appID));
    DmStringAppend(&prefixKey, DM_DB_KEY_DELIMITER);
    DmStringAppend(&prefixKey, DmStringCstr(&value->udidHash));
    DmMutexLock(&mgr->idCacheMapMtx);
    DmString* idIter = DmHmap_DmString_DmString_Find(&mgr->idCacheMap, dmKey);
    if (idIter != NULL && !DmIsTimeOut(DmStringToInt64(idIter, DM_BASE_TEN),
        value->lastModifyTime, DM_KV_STORE_REFRESH_TIME)) {
        DmMutexUnlock(&mgr->idCacheMapMtx);
        DmStringDestroy(&dmKey);
        DmStringDestroy(&prefixKey);
        return DM_OK;
    }
    (void)DmHmap_DmString_DmString_Insert(&mgr->idCacheMap, DmStringCopy(&dmKey), DmStringCreate(""));
    (void)DmHmap_DmString_DmString_Insert(&mgr->idCacheMap, DmStringCopy(&prefixKey), DmStringCreate(""));
    DmMutexUnlock(&mgr->idCacheMapMtx);
    DmString valueStr = DmStringCreateEmpty();
    DmConvertKvValueToJson(value, &valueStr);
    int32_t ret = DmPutToAdapter(mgr, &dmKey, &valueStr);
    if (ret != DM_OK) {
        if (ret == ERR_DM_POINT_NULL) {
            LOGE("kvAdapter is NULL");
        } else {
            LOGE("Insert value to DB for dmKey failed");
        }
        DmStringDestroy(&dmKey);
        DmStringDestroy(&prefixKey);
        DmStringDestroy(&valueStr);
        return ret;
    }
    ret = DmPutToAdapter(mgr, &prefixKey, &valueStr);
    if (ret != DM_OK) {
        LOGE("Insert value to DB for prefixKey failed");
    }
    DmStringDestroy(&dmKey);
    DmStringDestroy(&prefixKey);
    DmStringDestroy(&valueStr);
    return ret;
}

DM_EXPORT int32_t DmKvAdapterManagerGet(const char* key, DmKVValue* value)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmString dmKey = DmStringCreate(DM_KV_STORE_PREFIX);
    DmStringAppend(&dmKey, key);
    DmMutexLock(&mgr->idCacheMapMtx);
    DmString* idIter = DmHmap_DmString_DmString_Find(&mgr->idCacheMap, dmKey);
    if (idIter != NULL) {
        DmMutexUnlock(&mgr->idCacheMapMtx);
        DmStringDestroy(&dmKey);
        return DM_OK;
    }
    DmMutexUnlock(&mgr->idCacheMapMtx);
    DmString valueStr = DmStringCreateEmpty();
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        DmStringDestroy(&dmKey);
        return ERR_DM_POINT_NULL;
    }
    if (DmKvAdapterGet(mgr->kvAdapter, DmStringCstr(&dmKey), &valueStr) != DM_OK) {
        DmString anonyDmKey = DmGetAnonyString(&dmKey);
        LOGE("kv value failed, dmKey: %{public}s", DmStringCstr(&anonyDmKey));
        DmStringDestroy(&anonyDmKey);
        DmMutexUnlock(&mgr->kvAdapterMtx);
        DmStringDestroy(&dmKey);
        return ERR_DM_FAILED;
    }
    DmMutexUnlock(&mgr->kvAdapterMtx);
    DmConvertJsonToKvValue(&valueStr, value);
    DmMutexLock(&mgr->idCacheMapMtx);
    DmString prefixKey = DmStringCreate(DM_KV_STORE_PREFIX);
    DmStringAppend(&prefixKey, DmStringCstr(&value->appID));
    DmStringAppend(&prefixKey, DM_DB_KEY_DELIMITER);
    DmStringAppend(&prefixKey, DmStringCstr(&value->udidHash));
    (void)DmHmap_DmString_DmString_Insert(&mgr->idCacheMap, DmStringCopy(&dmKey), DmStringCreate(""));
    (void)DmHmap_DmString_DmString_Insert(&mgr->idCacheMap, DmStringCopy(&prefixKey), DmStringCreate(""));
    DmMutexUnlock(&mgr->idCacheMapMtx);
    DmStringDestroy(&dmKey);
    DmStringDestroy(&prefixKey);
    DmStringDestroy(&valueStr);
    return DM_OK;
}

DM_EXPORT int32_t DmKvAdapterManagerDeleteAgedEntry(void)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmMutexLock(&mgr->idCacheMapMtx);
    (void)DmHmap_DmString_DmString_Clear(&mgr->idCacheMap);
    DmMutexUnlock(&mgr->idCacheMapMtx);
    return DM_OK;
}

DM_EXPORT int32_t DmKvAdapterManagerAppUninstall(const char* appId)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        return ERR_DM_POINT_NULL;
    }
    DmKvAdapterDeleteByAppId(mgr->kvAdapter, appId, DM_KV_STORE_PREFIX);
    DmMutexUnlock(&mgr->kvAdapterMtx);
    return DM_OK;
}

DM_EXPORT int32_t DmKvAdapterManagerGetFreezeData(const char* key, DmString* value)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmString dmKey = DmStringCreate(DM_KV_STORE_FREEZE_PREFIX);
    DmStringAppend(&dmKey, key);
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        DmStringDestroy(&dmKey);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmKvAdapterGet(mgr->kvAdapter, DmStringCstr(&dmKey), value);
    DmMutexUnlock(&mgr->kvAdapterMtx);
    DmStringDestroy(&dmKey);
    return ret;
}

DM_EXPORT int32_t DmKvAdapterManagerPutFreezeData(const char* key, const DmString* value)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmString dmKey = DmStringCreate(DM_KV_STORE_FREEZE_PREFIX);
    DmStringAppend(&dmKey, key);
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        DmStringDestroy(&dmKey);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmKvAdapterPut(mgr->kvAdapter, DmStringCstr(&dmKey), DmStringCstr(value));
    DmMutexUnlock(&mgr->kvAdapterMtx);
    DmStringDestroy(&dmKey);
    return ret;
}

DM_EXPORT int32_t DmKvAdapterManagerDeleteFreezeData(const char* key)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmString dmKey = DmStringCreate(DM_KV_STORE_FREEZE_PREFIX);
    DmStringAppend(&dmKey, key);
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        DmStringDestroy(&dmKey);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmKvAdapterDelete(mgr->kvAdapter, DmStringCstr(&dmKey));
    DmMutexUnlock(&mgr->kvAdapterMtx);
    DmStringDestroy(&dmKey);
    return ret;
}

DM_EXPORT int32_t DmKvAdapterManagerGetAllOstypeData(DmVec_DmString* values)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmString dmKey = DmComposeOsTypePrefix();
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        DmStringDestroy(&dmKey);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmKvAdapterGetAllOstypeData(mgr->kvAdapter, DmStringCstr(&dmKey), values);
    DmMutexUnlock(&mgr->kvAdapterMtx);
    DmStringDestroy(&dmKey);
    return ret;
}

DM_EXPORT int32_t DmKvAdapterManagerPutOstypeData(const char* key, const char* value)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmString dmKey = DmComposeOsTypePrefix();
    DmStringAppend(&dmKey, key);
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        DmStringDestroy(&dmKey);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmKvAdapterPut(mgr->kvAdapter, DmStringCstr(&dmKey), value);
    DmMutexUnlock(&mgr->kvAdapterMtx);
    DmStringDestroy(&dmKey);
    return ret;
}

DM_EXPORT int32_t DmKvAdapterManagerDeleteOstypeData(const char* key)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmString dmKey = DmComposeOsTypePrefix();
    DmStringAppend(&dmKey, key);
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        DmStringDestroy(&dmKey);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmKvAdapterDelete(mgr->kvAdapter, DmStringCstr(&dmKey));
    DmMutexUnlock(&mgr->kvAdapterMtx);
    DmStringDestroy(&dmKey);
    return ret;
}

DM_EXPORT int32_t DmKvAdapterManagerGetLocalUserIdData(const char* key, DmString* value)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmKvAdapterGet(mgr->kvAdapter, key, value);
    DmMutexUnlock(&mgr->kvAdapterMtx);
    return ret;
}

DM_EXPORT int32_t DmKvAdapterManagerPutLocalUserIdData(const char* key, const DmString* value)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmKvAdapterPut(mgr->kvAdapter, key, DmStringCstr(value));
    DmMutexUnlock(&mgr->kvAdapterMtx);
    return ret;
}

DM_EXPORT int32_t DmKvAdapterManagerGetOsTypeCount(int32_t* count)
{
    KVAdapterManager* mgr = KVAdapterManagerGetInstance();
    DmMutexLock(&mgr->kvAdapterMtx);
    if (mgr->kvAdapter == NULL) {
        DmMutexUnlock(&mgr->kvAdapterMtx);
        return ERR_DM_POINT_NULL;
    }
    DmString osTypePrefix = DmComposeOsTypePrefix();
    int32_t ret = DmKvAdapterGetOstypeCountByPrefix(mgr->kvAdapter, DmStringCstr(&osTypePrefix), count);
    DmMutexUnlock(&mgr->kvAdapterMtx);
    DmStringDestroy(&osTypePrefix);
    return ret;
}
