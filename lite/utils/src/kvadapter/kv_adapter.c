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


#include "kv_adapter.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "securec.h"
#include "cJSON.h"
#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"

#include "kv_store.h"
#include "kvstore_env.h"

#define DM_SYS_KV_CHUNK_SIZE 120
#define DM_SYS_KV_MAX_KEY 32
#define DM_SYS_KV_DATA_PATH "/userdata"
#define DM_KV_MAX_CHUNK_COUNT 100
#define DM_KV_DIR_MODE 0755

static void DmSysKvMakeChunkKey(char* buf, size_t bufSize, const char* baseKey, int chunkIdx)
{
    if (chunkIdx < 0) {
        if (snprintf_s(buf, bufSize, bufSize - 1, "%s_cnt", baseKey) < 0) {
            buf[0] = '\0';
        }
    } else {
        if (snprintf_s(buf, bufSize, bufSize - 1, "%s_%d", baseKey, chunkIdx) < 0) {
            buf[0] = '\0';
        }
    }
}

static void DmSysKvDeleteOldChunks(const char* key, int oldCount)
{
    for (int j = 0; j < oldCount; j++) {
        char oldChunkKey[DM_SYS_KV_MAX_KEY];
        DmSysKvMakeChunkKey(oldChunkKey, sizeof(oldChunkKey), key, j);
        if (strlen(oldChunkKey) < DM_SYS_KV_MAX_KEY) {
            (void)UtilsDeleteValue(oldChunkKey);
        }
    }
}

static int32_t DmWriteChunks(const char* key, const char* value, size_t valueLen)
{
    int chunkCount = (valueLen + DM_SYS_KV_CHUNK_SIZE - 1) / DM_SYS_KV_CHUNK_SIZE;
    for (int i = 0; i < chunkCount; i++) {
        char chunkKey[DM_SYS_KV_MAX_KEY];
        DmSysKvMakeChunkKey(chunkKey, sizeof(chunkKey), key, i);
        if (strlen(chunkKey) >= DM_SYS_KV_MAX_KEY) {
            LOGE("chunk key too long: %s", chunkKey);
            return ERR_DM_FAILED;
        }
        size_t offset = i * DM_SYS_KV_CHUNK_SIZE;
        size_t copyLen = valueLen - offset;
        if (copyLen > DM_SYS_KV_CHUNK_SIZE) {
            copyLen = DM_SYS_KV_CHUNK_SIZE;
        }
        char chunkVal[DM_SYS_KV_CHUNK_SIZE + 1];
        if (memcpy_s(chunkVal, sizeof(chunkVal), value + offset, copyLen) != 0) {
            return ERR_DM_FAILED;
        }
        chunkVal[copyLen] = '\0';
        int ret = UtilsSetValue(chunkKey, chunkVal);
        if (ret != 0) {
            LOGE("UtilsSetValue chunk failed key=%s ret=%d", chunkKey, ret);
            return ERR_DM_FAILED;
        }
    }
    return DM_OK;
}

static int32_t DmSysKvPut(const char* storeDir, const char* key, const char* value)
{
    (void)storeDir;
    size_t valueLen = strlen(value);
    if (valueLen < DM_SYS_KV_CHUNK_SIZE) {
        int ret = UtilsSetValue(key, value);
        if (ret != 0) {
            LOGE("UtilsSetValue failed key=%s ret=%d", key, ret);
            return ERR_DM_FAILED;
        }
        return DM_OK;
    }
    int chunkCount = (valueLen + DM_SYS_KV_CHUNK_SIZE - 1) / DM_SYS_KV_CHUNK_SIZE;
    char cntKey[DM_SYS_KV_MAX_KEY];
    /* Clean up old chunks before writing new ones */
    DmSysKvMakeChunkKey(cntKey, sizeof(cntKey), key, -1);
    if (strlen(cntKey) < DM_SYS_KV_MAX_KEY) {
        char oldCntBuf[8] = {0};
        int oldCntRet = UtilsGetValue(cntKey, oldCntBuf, sizeof(oldCntBuf));
        if (oldCntRet >= 0) {
            int oldCount = atoi(oldCntBuf);
            DmSysKvDeleteOldChunks(key, oldCount);
        }
    }
    char cntVal[8];
    if (snprintf_s(cntVal, sizeof(cntVal), sizeof(cntVal) - 1, "%d", chunkCount) < 0) {
        return ERR_DM_FAILED;
    }
    if (strlen(cntKey) >= DM_SYS_KV_MAX_KEY) {
        LOGE("cnt key too long: %s", cntKey);
        return ERR_DM_FAILED;
    }
    (void)UtilsSetValue(cntKey, cntVal);
    return DmWriteChunks(key, value, valueLen);
}

static int32_t DmSysKvGet(const char* storeDir, const char* key, DmString* value)
{
    (void)storeDir;
    char buf[DM_SYS_KV_CHUNK_SIZE + 1];
    int ret = UtilsGetValue(key, buf, sizeof(buf));
    if (ret >= 0) {
        *value = DmStringCreate(buf);
        return DM_OK;
    }
    char cntKey[DM_SYS_KV_MAX_KEY];
    DmSysKvMakeChunkKey(cntKey, sizeof(cntKey), key, -1);
    char cntBuf[8] = {0};
    int cntRet = UtilsGetValue(cntKey, cntBuf, sizeof(cntBuf));
    if (cntRet < 0) {
        return ERR_DM_FAILED;
    }
    int chunkCount = atoi(cntBuf);
    if (chunkCount <= 0 || chunkCount > DM_KV_MAX_CHUNK_COUNT) {
        return ERR_DM_FAILED;
    }
    size_t fullValueSize = chunkCount * (DM_SYS_KV_CHUNK_SIZE + 1);
    char* fullValue = (char*)malloc(fullValueSize);
    if (fullValue == NULL) {
        return ERR_DM_FAILED;
    }
    fullValue[0] = '\0';
    size_t totalLen = 0;
    for (int i = 0; i < chunkCount; i++) {
        char chunkKey[DM_SYS_KV_MAX_KEY];
        DmSysKvMakeChunkKey(chunkKey, sizeof(chunkKey), key, i);
        int chunkRet = UtilsGetValue(chunkKey, buf, sizeof(buf));
        if (chunkRet < 0) {
            free(fullValue);
            return ERR_DM_FAILED;
        }
        int chunkLen = (chunkRet > 0) ? chunkRet : (int)strlen(buf);
        if (memcpy_s(fullValue + totalLen, fullValueSize - totalLen, buf, chunkLen) != 0) {
            free(fullValue);
            return ERR_DM_FAILED;
        }
        totalLen += chunkLen;
    }
    fullValue[totalLen] = '\0';
    *value = DmStringCreate(fullValue);
    free(fullValue);
    return DM_OK;
}

static int32_t DmSysKvDelete(const char* storeDir, const char* key)
{
    (void)storeDir;
    (void)UtilsDeleteValue(key);
    char cntKey[DM_SYS_KV_MAX_KEY];
    DmSysKvMakeChunkKey(cntKey, sizeof(cntKey), key, -1);
    char cntBuf[8] = {0};
    int cntRet = UtilsGetValue(cntKey, cntBuf, sizeof(cntBuf));
    if (cntRet < 0) {
        return DM_OK;
    }
    int chunkCount = atoi(cntBuf);
    for (int i = 0; i < chunkCount; i++) {
        char chunkKey[DM_SYS_KV_MAX_KEY];
        DmSysKvMakeChunkKey(chunkKey, sizeof(chunkKey), key, i);
        (void)UtilsDeleteValue(chunkKey);
    }
    (void)UtilsDeleteValue(cntKey);
    return DM_OK;
}

#define DM_APP_ID "distributed_device_manager_service"
#define DM_STORE_ID "dm_kv_store"
#define DM_DATABASE_DIR "/userdata/dm_kv_store"
#define DM_ACL_DIR "/userdata/dm_acl_store"
#define DM_MAX_BATCH_SIZE 128
#define DM_MAX_STRING_LEN 4096
#define DM_MAX_INIT_RETRY_TIMES 20
#define DM_INIT_RETRY_SLEEP_INTERVAL 200000
#define DM_OSTYPE_PREFIX_LEN 16

struct KVAdapter {
    void* kvStorePtr;
    DmMutex kvAdapterMutex;
    DmMutex kvDataMgrMutex;
    volatile bool isInited;
};

KVAdapter* DmKvAdapterCreate(void)
{
    KVAdapter* adapter = (KVAdapter*)calloc(1, sizeof(KVAdapter));
    if (adapter == NULL) {
        return NULL;
    }
    DmMutexInit(&adapter->kvAdapterMutex);
    DmMutexInit(&adapter->kvDataMgrMutex);
    adapter->isInited = false;
    adapter->kvStorePtr = NULL;
    return adapter;
}

void DmKvAdapterDestroy(KVAdapter* adapter)
{
    if (adapter == NULL) {
        return;
    }
    DmKvAdapterUninit(adapter);
    DmMutexDestroy(&adapter->kvAdapterMutex);
    DmMutexDestroy(&adapter->kvDataMgrMutex);
    free(adapter);
}


static int DmEnsureDir(const char* path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return DM_OK;
        }
        return ERR_DM_FAILED;
    }
    char tmp[512];
    if (snprintf_s(tmp, sizeof(tmp), sizeof(tmp) - 1, "%s", path) < 0) {
        return ERR_DM_FAILED;
    }
    size_t len = strlen(tmp);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }
    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, DM_KV_DIR_MODE) != 0 && errno != EEXIST) {
                return ERR_DM_FAILED;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, DM_KV_DIR_MODE) != 0 && errno != EEXIST) {
        return ERR_DM_FAILED;
    }
    return DM_OK;
}


static int32_t DmLiteKvPut(const char* storeDir, const char* key, const char* value)
{
    (void)storeDir;
    return DmSysKvPut(storeDir, key, value);
}

static int32_t DmLiteKvGet(const char* storeDir, const char* key, DmString* value)
{
    (void)storeDir;
    return DmSysKvGet(storeDir, key, value);
}

static int32_t DmLiteKvDelete(const char* storeDir, const char* key)
{
    (void)storeDir;
    return DmSysKvDelete(storeDir, key);
}

static int32_t DmLiteKvDeleteStore(const char* storeDir)
{
    (void)storeDir;
    char idListBuf[DM_MAX_STRING_LEN] = {0};
    DmString idListStr = {0};
    int ret = (DmSysKvGet(NULL, "acl_key_list", &idListStr) == DM_OK &&
        DmStringSize(&idListStr) > 0) ? (int)DmStringSize(&idListStr) : -1;
    if (ret > 0) {
        if (strncpy_s(idListBuf, sizeof(idListBuf), DmStringCstr(&idListStr), sizeof(idListBuf) - 1) != 0) {
            return DM_OK;
        }
    }
    DmStringDestroy(&idListStr);
    if (ret > 0) {
        idListBuf[ret] = '\0';
        char* save = NULL;
        char* tok = strtok_r(idListBuf, ",", &save);
        while (tok != NULL) {
            char aclKey[DM_SYS_KV_MAX_KEY];
            if (snprintf_s(aclKey, sizeof(aclKey), sizeof(aclKey) - 1, "acl_%s", tok) < 0) {
                tok = strtok_r(NULL, ",", &save);
                continue;
            }
            DmSysKvDelete(NULL, aclKey);
            tok = strtok_r(NULL, ",", &save);
        }
        (void)UtilsDeleteValue("acl_key_list");
    }
    (void)UtilsDeleteValue("acl_key_list");
    free(idListBuf);
    return DM_OK;
}

static int32_t DmLiteKvDeleteByPrefix(const char* storeDir, const char* prefix)
{
    (void)storeDir;
    if (prefix == NULL) {
        return DM_OK;
    }
    if (strcmp(prefix, "acl_") == 0) {
        return DmLiteKvDeleteStore(storeDir);
    }
    if (strcmp(prefix, "sk_") == 0) {
        char skListBuf[DM_MAX_STRING_LEN] = {0};
        DmString skListStr = {0};
        int ret = -1;
        if (DmSysKvGet(NULL, "sk_id_list", &skListStr) == DM_OK && DmStringSize(&skListStr) > 0) {
            if (strncpy_s(skListBuf, sizeof(skListBuf), DmStringCstr(&skListStr), sizeof(skListBuf) - 1) != 0) {
                DmStringDestroy(&skListStr);
                return DM_OK;
            }
            ret = (int)DmStringSize(&skListStr);
        }
        DmStringDestroy(&skListStr);
        if (ret > 0) {
            skListBuf[ret] = '\0';
            char* save = NULL;
            char* tok = strtok_r(skListBuf, ",", &save);
            while (tok != NULL) {
                (void)UtilsDeleteValue(tok);
                tok = strtok_r(NULL, ",", &save);
            }
            (void)UtilsDeleteValue("sk_id_list");
        }
        free(skListBuf);
    }
    return DM_OK;
}

static int32_t DmLiteKvGetByPrefix(const char* storeDir, const char* prefix, DmVec_DmString* values)
{
    (void)storeDir;
    if (prefix == NULL) {
        return DM_OK;
    }
    if (strcmp(prefix, "acl_") != 0) {
        return DM_OK;
    }
    char listBuf[DM_MAX_STRING_LEN] = {0};
    DmString listStr = {0};
    if (DmSysKvGet(NULL, "acl_key_list", &listStr) == DM_OK && DmStringSize(&listStr) > 0) {
        if (strncpy_s(listBuf, sizeof(listBuf), DmStringCstr(&listStr), sizeof(listBuf) - 1) != 0) {
            DmStringDestroy(&listStr);
            return DM_OK;
        }
    }
    DmStringDestroy(&listStr);
    if (listBuf[0] == '\0') {
        return DM_OK;
    }
    char* save = NULL;
    char* tok = strtok_r(listBuf, ",", &save);
    while (tok != NULL) {
        char aclKey[DM_SYS_KV_MAX_KEY];
        if (snprintf_s(aclKey, sizeof(aclKey), sizeof(aclKey) - 1, "acl_%s", tok) < 0) {
            tok = strtok_r(NULL, ",", &save);
            continue;
        }
        DmString val = {0};
        if (DmSysKvGet(NULL, aclKey, &val) == DM_OK && DmStringSize(&val) > 0) {
            (void)DmVec_DmString_Push(values, val);
        } else {
            DmStringDestroy(&val);
        }
        tok = strtok_r(NULL, ",", &save);
    }
    free(listBuf);
    return DM_OK;
}

static int32_t DmLiteKvCountByPrefix(const char* storeDir, const char* prefix, int32_t* count)
{
    (void)storeDir;
    *count = 0;
    if (prefix == NULL) {
        return DM_OK;
    }
    if (strcmp(prefix, "acl_") == 0) {
        char idListBuf[DM_MAX_STRING_LEN] = {0};
        DmString idListStr = {0};
        int ret = (DmSysKvGet(NULL, "acl_key_list", &idListStr) == DM_OK &&
            DmStringSize(&idListStr) > 0) ? (int)DmStringSize(&idListStr) : -1;
        if (ret > 0) {
            if (strncpy_s(idListBuf, sizeof(idListBuf), DmStringCstr(&idListStr), sizeof(idListBuf) - 1) != 0) {
                DmStringDestroy(&idListStr);
                return DM_OK;
            }
        }
        DmStringDestroy(&idListStr);
        if (ret > 0) {
            idListBuf[ret] = '\0';
            char* save = NULL;
            char* tok = strtok_r(idListBuf, ",", &save);
            while (tok != NULL) {
                (*count)++;
                tok = strtok_r(NULL, ",", &save);
            }
        }
    }
    return DM_OK;
}

static const char* DmLiteGetStoreDir(KVAdapter* adapter)
{
    return DM_DATABASE_DIR;
}

int32_t DmKvAdapterInit(KVAdapter* adapter)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    if (adapter->isInited) {
        return DM_OK;
    }
    DmMutexLock(&adapter->kvAdapterMutex);
    (void)DmEnsureDir(DM_DATABASE_DIR);
    (void)DmEnsureDir(DM_ACL_DIR);
    /* Initialize system KV store data path */
    (void)UtilsSetEnv(DM_SYS_KV_DATA_PATH);
    LOGI("UtilsSetEnv path=%s", DM_SYS_KV_DATA_PATH);
    adapter->kvStorePtr = (void*)1;
    adapter->isInited = true;
    DmMutexUnlock(&adapter->kvAdapterMutex);
    return DM_OK;
}

void DmKvAdapterUninit(KVAdapter* adapter)
{
    if (adapter == NULL) {
        return;
    }
    if (adapter->isInited) {
        DmMutexLock(&adapter->kvAdapterMutex);
        adapter->kvStorePtr = NULL;
        adapter->isInited = false;
        DmMutexUnlock(&adapter->kvAdapterMutex);
    }
}

int32_t DmKvAdapterReinit(KVAdapter* adapter)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    DmKvAdapterUninit(adapter);
    return DmKvAdapterInit(adapter);
}

int32_t DmKvAdapterPut(KVAdapter* adapter, const char* key, const char* value)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    if (key == NULL || value == NULL) {
        return ERR_DM_FAILED;
    }
    DmMutexLock(&adapter->kvAdapterMutex);
    if (!adapter->isInited) {
        DmMutexUnlock(&adapter->kvAdapterMutex);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmLiteKvPut(DmLiteGetStoreDir(adapter), key, value);
    DmMutexUnlock(&adapter->kvAdapterMutex);
    return ret;
}

int32_t DmKvAdapterGet(KVAdapter* adapter, const char* key, DmString* value)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    if (key == NULL || value == NULL) {
        return ERR_DM_FAILED;
    }
    DmMutexLock(&adapter->kvAdapterMutex);
    if (!adapter->isInited) {
        DmMutexUnlock(&adapter->kvAdapterMutex);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmLiteKvGet(DmLiteGetStoreDir(adapter), key, value);
    DmMutexUnlock(&adapter->kvAdapterMutex);
    return ret;
}

int32_t DmKvAdapterDeleteKvStore(KVAdapter* adapter)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    DmMutexLock(&adapter->kvAdapterMutex);
    DmLiteKvDeleteStore(DmLiteGetStoreDir(adapter));
    DmLiteKvDeleteStore(DM_ACL_DIR);
    DmMutexUnlock(&adapter->kvAdapterMutex);
    return DM_OK;
}

int32_t DmKvAdapterDeleteByAppId(KVAdapter* adapter, const char* appId, const char* prefix)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    if (appId == NULL) {
        return ERR_DM_FAILED;
    }
    DmMutexLock(&adapter->kvAdapterMutex);
    DmLiteKvDeleteByPrefix(DmLiteGetStoreDir(adapter), prefix ? prefix : appId);
    DmMutexUnlock(&adapter->kvAdapterMutex);
    return DM_OK;
}

int32_t DmKvAdapterDeleteBatch(KVAdapter* adapter, const DmVec_DmString* keys)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    DmMutexLock(&adapter->kvAdapterMutex);
    for (int32_t i = 0; i < DmVec_DmString_Size(keys); i++) {
        DmLiteKvDelete(DmLiteGetStoreDir(adapter), DmStringCstr(DmVec_DmString_At(keys, i)));
    }
    DmMutexUnlock(&adapter->kvAdapterMutex);
    return DM_OK;
}

int32_t DmKvAdapterDelete(KVAdapter* adapter, const char* key)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    if (key == NULL) {
        return ERR_DM_FAILED;
    }
    DmMutexLock(&adapter->kvAdapterMutex);
    if (!adapter->isInited) {
        DmMutexUnlock(&adapter->kvAdapterMutex);
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = DmLiteKvDelete(DmLiteGetStoreDir(adapter), key);
    DmMutexUnlock(&adapter->kvAdapterMutex);
    return ret;
}

int32_t DmKvAdapterGetAllOstypeData(KVAdapter* adapter, const char* key, DmVec_DmString* values)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    if (key == NULL) {
        return ERR_DM_FAILED;
    }
    DmMutexLock(&adapter->kvAdapterMutex);
    DmLiteKvGetByPrefix(DmLiteGetStoreDir(adapter), key, values);
    DmMutexUnlock(&adapter->kvAdapterMutex);
    return DM_OK;
}

int32_t DmKvAdapterGetOstypeCountByPrefix(KVAdapter* adapter, const char* prefix, int32_t* count)
{
    if (adapter == NULL) {
        return ERR_DM_POINT_NULL;
    }
    if (prefix == NULL) {
        return ERR_DM_FAILED;
    }
    DmMutexLock(&adapter->kvAdapterMutex);
    DmLiteKvCountByPrefix(DmLiteGetStoreDir(adapter), prefix, count);
    DmMutexUnlock(&adapter->kvAdapterMutex);
    return DM_OK;
}
