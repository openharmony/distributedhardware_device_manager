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


#include "dm_local_acl_profile.h"
#include "kv_adapter.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "cJSON.h"
#include "securec.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DM_ACL_LIST_MAX_LEN 4095
#define DM_ACL_BUF_SIZE 4096

int DmVecVoidInit(DmVecVoid* v)
{
    v->data = NULL;
    v->size = 0;
    v->cap = 0;
    return 0;
}

int DmVecVoidPush(DmVecVoid* v, void* item)
{
    if (v->size >= v->cap) {
        int nc = v->cap == 0 ? 8 : v->cap * 2;
        void** nd = (void**)malloc(nc * sizeof(void*));
        if (!nd) {
            return -1;
        }
        if (v->data && v->size > 0) {
            if (memcpy_s(nd, nc * sizeof(void*), v->data, v->size * sizeof(void*)) != 0) {
                free(nd);
                return -1;
            }
            free(v->data);
        }
        v->data = nd;
        v->cap = nc;
    }
    v->data[v->size++] = item;
    return 0;
}

void** DmVecVoidAt(DmVecVoid* v, int idx)
{
    return (idx >= 0 && idx < v->size) ? &v->data[idx] : NULL;
}

int DmVecVoidSize(DmVecVoid* v)
{
    return v ? v->size : 0;
}

void DmVecVoidDestroy(DmVecVoid* v)
{
    for (int i = 0; i < v->size; i++) {
        if (v->data[i] != NULL) {
            free(v->data[i]);
        }
    }
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->cap = 0;
}

#define DM_ACL_KV_PREFIX "acl_"
#define DM_ACL_KEY_LIST "acl_key_list"

static KVAdapter* g_aclKvAdapter = NULL;

void DmLocalAclProfileInit(DmLocalAclProfile* p)
{
    if (p == NULL) {
        return;
    }
    if (memset_s(p, sizeof(DmLocalAclProfile), 0, sizeof(DmLocalAclProfile)) != 0) {
        return;
    }
    p->bindType = -1;
    p->authenticationType = -1;
    p->bindLevel = -1;
    p->accesser.userId = -1;
    p->accesser.tokenId = (uint64_t)-1;
    p->accessee.userId = -1;
    p->accessee.tokenId = (uint64_t)-1;
    DmStringAssignCstr(&p->accesser.accountId, "-1");
    DmStringAssignCstr(&p->accesser.bundleName, "-1");
    DmStringAssignCstr(&p->accesser.deviceName, "-1");
    DmStringAssignCstr(&p->accesser.credentialId, "-1");
    DmStringAssignCstr(&p->accessee.accountId, "-1");
    DmStringAssignCstr(&p->accessee.bundleName, "-1");
    DmStringAssignCstr(&p->accessee.deviceName, "-1");
    DmStringAssignCstr(&p->accessee.credentialId, "-1");
}

void DmLocalAclProfileDestroy(DmLocalAclProfile* p)
{
    if (p == NULL) {
        return;
    }
    DmStringDestroy(&p->trustDeviceId);
    DmStringDestroy(&p->accesser.deviceId);
    DmStringDestroy(&p->accesser.accountId);
    DmStringDestroy(&p->accesser.bundleName);
    DmStringDestroy(&p->accesser.deviceName);
    DmStringDestroy(&p->accesser.credentialId);
    DmStringDestroy(&p->accessee.deviceId);
    DmStringDestroy(&p->accessee.accountId);
    DmStringDestroy(&p->accessee.bundleName);
    DmStringDestroy(&p->accessee.deviceName);
    DmStringDestroy(&p->accessee.credentialId);
}

DmLocalAclProfile* DmLocalAclProfileCreate(void)
{
    DmLocalAclProfile* p = (DmLocalAclProfile*)calloc(1, sizeof(DmLocalAclProfile));
    if (p) {
        DmLocalAclProfileInit(p);
    }
    return p;
}

void DmLocalAclProfileDelete(DmLocalAclProfile* p)
{
    if (p == NULL) {
        return;
    }
    DmLocalAclProfileDestroy(p);
    free(p);
}

DmString DmLocalAclProfileSerialize(const DmLocalAclProfile* p)
{
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, ACL_KEY_BIND_TYPE, p->bindType);
    cJSON_AddNumberToObject(root, ACL_KEY_AUTH_TYPE, p->authenticationType);
    cJSON_AddNumberToObject(root, ACL_KEY_BIND_LEVEL, p->bindLevel);
    cJSON_AddStringToObject(root, ACL_KEY_TRUST_DEVICE_ID, DmStringCstr(&p->trustDeviceId));

    cJSON* accesser = cJSON_CreateObject();
    cJSON_AddStringToObject(accesser, ACL_KEY_DEVICE_ID, DmStringCstr(&p->accesser.deviceId));
    cJSON_AddNumberToObject(accesser, ACL_KEY_USER_ID, p->accesser.userId);
    cJSON_AddStringToObject(accesser, ACL_KEY_ACCOUNT_ID, DmStringCstr(&p->accesser.accountId));
    cJSON_AddNumberToObject(accesser, ACL_KEY_TOKEN_ID, (double)p->accesser.tokenId);
    cJSON_AddStringToObject(accesser, ACL_KEY_BUNDLE_NAME, DmStringCstr(&p->accesser.bundleName));
    cJSON_AddStringToObject(accesser, ACL_KEY_DEVICE_NAME, DmStringCstr(&p->accesser.deviceName));
    cJSON_AddStringToObject(accesser, ACL_KEY_CREDENTIAL_ID, DmStringCstr(&p->accesser.credentialId));
    cJSON_AddItemToObject(root, ACL_KEY_ACCESSER, accesser);

    cJSON* accessee = cJSON_CreateObject();
    cJSON_AddStringToObject(accessee, ACL_KEY_DEVICE_ID, DmStringCstr(&p->accessee.deviceId));
    cJSON_AddNumberToObject(accessee, ACL_KEY_USER_ID, p->accessee.userId);
    cJSON_AddStringToObject(accessee, ACL_KEY_ACCOUNT_ID, DmStringCstr(&p->accessee.accountId));
    cJSON_AddNumberToObject(accessee, ACL_KEY_TOKEN_ID, (double)p->accessee.tokenId);
    cJSON_AddStringToObject(accessee, ACL_KEY_BUNDLE_NAME, DmStringCstr(&p->accessee.bundleName));
    cJSON_AddStringToObject(accessee, ACL_KEY_DEVICE_NAME, DmStringCstr(&p->accessee.deviceName));
    cJSON_AddStringToObject(accessee, ACL_KEY_CREDENTIAL_ID, DmStringCstr(&p->accessee.credentialId));
    cJSON_AddItemToObject(root, ACL_KEY_ACCESSEE, accessee);

    char* jsonStr = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    DmString result = DmStringCreate(jsonStr ? jsonStr : "");
    if (jsonStr) {
        cJSON_free(jsonStr);
    }
    return result;
}

static void DmJsonStringToDmString(cJSON* obj, const char* key, DmString* target)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (item && cJSON_IsString(item) && item->valuestring) {
        DmStringAssignCstr(target, item->valuestring);
    }
}

static void DmParseAclFields(DmLocalAclProfile* p, cJSON* root)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(root, ACL_KEY_BIND_TYPE);
    if (item && cJSON_IsNumber(item)) {
        p->bindType = item->valueint;
    }
    item = cJSON_GetObjectItemCaseSensitive(root, ACL_KEY_AUTH_TYPE);
    if (item && cJSON_IsNumber(item)) {
        p->authenticationType = item->valueint;
    }
    item = cJSON_GetObjectItemCaseSensitive(root, ACL_KEY_BIND_LEVEL);
    if (item && cJSON_IsNumber(item)) {
        p->bindLevel = (int32_t)item->valuedouble;
    }

    DmJsonStringToDmString(root, ACL_KEY_TRUST_DEVICE_ID, &p->trustDeviceId);

    cJSON* accesser = cJSON_GetObjectItemCaseSensitive(root, ACL_KEY_ACCESSER);
    if (accesser) {
        DmJsonStringToDmString(accesser, ACL_KEY_DEVICE_ID, &p->accesser.deviceId);
        item = cJSON_GetObjectItemCaseSensitive(accesser, ACL_KEY_USER_ID);
        if (item && cJSON_IsNumber(item)) {
            p->accesser.userId = item->valueint;
        }
        DmJsonStringToDmString(accesser, ACL_KEY_ACCOUNT_ID, &p->accesser.accountId);
        item = cJSON_GetObjectItemCaseSensitive(accesser, ACL_KEY_TOKEN_ID);
        if (item && cJSON_IsNumber(item)) {
            p->accesser.tokenId = (uint64_t)item->valuedouble;
        }
        DmJsonStringToDmString(accesser, ACL_KEY_BUNDLE_NAME, &p->accesser.bundleName);
        DmJsonStringToDmString(accesser, ACL_KEY_DEVICE_NAME, &p->accesser.deviceName);
        DmJsonStringToDmString(accesser, ACL_KEY_CREDENTIAL_ID, &p->accesser.credentialId);
    }

    cJSON* accessee = cJSON_GetObjectItemCaseSensitive(root, ACL_KEY_ACCESSEE);
    if (accessee) {
        DmJsonStringToDmString(accessee, ACL_KEY_DEVICE_ID, &p->accessee.deviceId);
        item = cJSON_GetObjectItemCaseSensitive(accessee, ACL_KEY_USER_ID);
        if (item && cJSON_IsNumber(item)) {
            p->accessee.userId = item->valueint;
        }
        DmJsonStringToDmString(accessee, ACL_KEY_ACCOUNT_ID, &p->accessee.accountId);
        item = cJSON_GetObjectItemCaseSensitive(accessee, ACL_KEY_TOKEN_ID);
        if (item && cJSON_IsNumber(item)) {
            p->accessee.tokenId = (uint64_t)item->valuedouble;
        }
        DmJsonStringToDmString(accessee, ACL_KEY_BUNDLE_NAME, &p->accessee.bundleName);
        DmJsonStringToDmString(accessee, ACL_KEY_DEVICE_NAME, &p->accessee.deviceName);
        DmJsonStringToDmString(accessee, ACL_KEY_CREDENTIAL_ID, &p->accessee.credentialId);
    }
}

bool DmLocalAclProfileDeserialize(DmLocalAclProfile* p, const char* json)
{
    if (p == NULL || json == NULL) {
        return false;
    }
    cJSON* root = cJSON_Parse(json);
    if (root == NULL) {
        return false;
    }
    DmParseAclFields(p, root);
    cJSON_Delete(root);
    return true;
}

void DmAclKeyFromProfile(char* buf, size_t bufSize, const DmLocalAclProfile* p)
{
    if (buf == NULL || bufSize == 0 || p == NULL) {
        return;
    }

    char composite[1024];
    if (snprintf_s(composite, sizeof(composite), sizeof(composite) - 1, "%s#%d#%s#%llu#%s#%s#%d#%s#%llu#%s",
        DmStringCstr(&p->accesser.deviceId), p->accesser.userId,
        DmStringCstr(&p->accesser.accountId), p->accesser.tokenId,
        DmStringCstr(&p->accesser.bundleName),
        DmStringCstr(&p->accessee.deviceId), p->accessee.userId,
        DmStringCstr(&p->accessee.accountId), p->accessee.tokenId,
        DmStringCstr(&p->accessee.bundleName)) < 0) {
        buf[0] = '\0';
        return;
    }

    LOGW("[ACL_KEY] composite=%s", composite);

    uint32_t h1 = 2166136261u;
    uint32_t h2 = 1469347982u;
    for (int i = 0; composite[i] != '\0'; i++) {
        h1 ^= (uint32_t)(unsigned char)composite[i];
        h1 *= 16777619u;
        h2 ^= (uint32_t)(unsigned char)composite[i];
        h2 *= 1099511628211u;
    }
    if (snprintf_s(buf, bufSize, bufSize - 1, "%s%08x%08x", DM_ACL_KV_PREFIX, h1, h2) < 0) {
        buf[0] = '\0';
    }
}

int32_t DmAclStoreInit(void)
{
    if (g_aclKvAdapter != NULL) {
        return DM_OK;
    }
    g_aclKvAdapter = DmKvAdapterCreate();
    if (g_aclKvAdapter == NULL) {
        return ERR_DM_FAILED;
    }
    return DmKvAdapterInit(g_aclKvAdapter);
}

static void DmAclUpdateKeyList(const char* key, bool add)
{
    if (g_aclKvAdapter == NULL || key == NULL) {
        return;
    }
    char listBuf[DM_ACL_BUF_SIZE] = {0};
    char newList[DM_ACL_BUF_SIZE] = {0};
    DmString oldList = {0};
    if (DmKvAdapterGet(g_aclKvAdapter, DM_ACL_KEY_LIST, &oldList) == DM_OK && DmStringSize(&oldList) > 0) {
        if (strncpy_s(listBuf, sizeof(listBuf), DmStringCstr(&oldList), DM_ACL_LIST_MAX_LEN) != 0) {
            DmStringDestroy(&oldList);
            return;
        }
    }
    DmStringDestroy(&oldList);
    const char* keyPart = key + strlen(DM_ACL_KV_PREFIX);
    char* save = NULL;
    char* tok = strtok_r(listBuf, ",", &save);
    while (tok != NULL) {
        if (strcmp(tok, keyPart) != 0) {
            if (newList[0] != '\0') {
                (void)strncat_s(newList, sizeof(newList), ",", DM_ACL_LIST_MAX_LEN - strlen(newList));
            }
            (void)strncat_s(newList, sizeof(newList), tok, DM_ACL_LIST_MAX_LEN - strlen(newList));
        }
        tok = strtok_r(NULL, ",", &save);
    }
    if (add) {
        if (newList[0] != '\0') {
            (void)strncat_s(newList, sizeof(newList), ",", DM_ACL_LIST_MAX_LEN - strlen(newList));
        }
        (void)strncat_s(newList, sizeof(newList), keyPart, DM_ACL_LIST_MAX_LEN - strlen(newList));
    }
    if (newList[0] != '\0') {
        DmKvAdapterPut(g_aclKvAdapter, DM_ACL_KEY_LIST, newList);
    } else {
        DmKvAdapterDelete(g_aclKvAdapter, DM_ACL_KEY_LIST);
    }
}

int32_t DmAclStorePut(DmLocalAclProfile* profile)
{
    if (profile == NULL) {
        return ERR_DM_POINT_NULL;
    }
    DmAclStoreInit();
    if (g_aclKvAdapter != NULL) {
        DmKvAdapterReinit(g_aclKvAdapter);
    }
    char key[32];
    DmAclKeyFromProfile(key, sizeof(key), profile);
    DmString json = DmLocalAclProfileSerialize(profile);
    int32_t ret = DmKvAdapterPut(g_aclKvAdapter, key, DmStringCstr(&json));
    DmStringDestroy(&json);
    if (ret == DM_OK) {
        DmAclUpdateKeyList(key, true);
    }
    return ret;
}

int32_t DmAclStoreGetAll(DmVecVoid* profiles)
{
    DmAclStoreInit();
    char listBuf[DM_ACL_BUF_SIZE] = {0};
    DmString listStr = {0};
    if (DmKvAdapterGet(g_aclKvAdapter, DM_ACL_KEY_LIST, &listStr) == DM_OK && DmStringSize(&listStr) > 0) {
        if (strncpy_s(listBuf, sizeof(listBuf), DmStringCstr(&listStr), DM_ACL_LIST_MAX_LEN) != 0) {
            DmStringDestroy(&listStr);
            return DM_OK;
        }
    }
    DmStringDestroy(&listStr);
    char* save = NULL;
    char* tok = strtok_r(listBuf, ",", &save);
    while (tok != NULL) {
        char key[32];
        if (snprintf_s(key, sizeof(key), sizeof(key) - 1, "%s%s", DM_ACL_KV_PREFIX, tok) < 0) {
            tok = strtok_r(NULL, ",", &save);
            continue;
        }
        DmString val = {0};
        if (DmKvAdapterGet(g_aclKvAdapter, key, &val) == DM_OK && DmStringSize(&val) > 0) {
            DmLocalAclProfile* p = DmLocalAclProfileCreate();
            if (DmLocalAclProfileDeserialize(p, DmStringCstr(&val))) {
                DmVecVoidPush(profiles, p);
            } else {
                DmLocalAclProfileDelete(p);
            }
        }
        DmStringDestroy(&val);
        tok = strtok_r(NULL, ",", &save);
    }
    return DM_OK;
}

int32_t DmAclStoreGetByUdid(DmVecVoid* profiles, const char* localUdid, int32_t userId, const char* remoteUdid)
{
    DmVecVoid all;
    DmVecVoidInit(&all);
    DmAclStoreGetAll(&all);
    for (int32_t i = 0; i < DmVecVoidSize(&all); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&all, i);
        if (p == NULL) {
            continue;
        }
        bool match = false;
        if (strcmp(DmStringCstr(&p->accesser.deviceId), localUdid) == 0 &&
            (userId < 0 || p->accesser.userId == userId) &&
            strcmp(DmStringCstr(&p->accessee.deviceId), remoteUdid) == 0) {
            match = true;
        }
        if (strcmp(DmStringCstr(&p->accessee.deviceId), localUdid) == 0 &&
            (userId < 0 || p->accessee.userId == userId) &&
            strcmp(DmStringCstr(&p->accesser.deviceId), remoteUdid) == 0) {
            match = true;
        }
        if (match) {
            DmVecVoidPush(profiles, p);
            all.data[i] = NULL;
        } else {
            DmLocalAclProfileDelete(p);
            all.data[i] = NULL;
        }
    }
    free(all.data);
    return DM_OK;
}

int32_t DmAclStoreDeleteByKey(const char* key)
{
    if (key == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmAclStoreInit();
    int32_t ret = DmKvAdapterDelete(g_aclKvAdapter, key);
    DmAclUpdateKeyList(key, false);
    return ret;
}

int32_t DmAclStoreDeleteByUdid(const char* localUdid, int32_t userId, const char* remoteUdid)
{
    if (localUdid == NULL || remoteUdid == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmAclStoreGetByUdid(&profiles, localUdid, userId, remoteUdid);
    int32_t count = DmVecVoidSize(&profiles);
    for (int32_t i = 0; i < count; i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&profiles, i);
        if (p != NULL) {
            char key[32];
            DmAclKeyFromProfile(key, sizeof(key), p);
            DmAclStoreDeleteByKey(key);
            DmLocalAclProfileDelete(p);
            profiles.data[i] = NULL;
        }
    }
    free(profiles.data);
    return count;
}

int32_t DmAclStoreUpdateDeviceName(const char* localUdid, const char* newDeviceName)
{
    if (localUdid == NULL || newDeviceName == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmAclStoreInit();
    DmVecVoid all;
    DmVecVoidInit(&all);
    DmAclStoreGetAll(&all);
    int32_t updated = 0;
    for (int32_t i = 0; i < DmVecVoidSize(&all); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&all, i);
        if (p == NULL) {
            continue;
        }
        bool changed = false;
        if (strcmp(DmStringCstr(&p->accesser.deviceId), localUdid) == 0) {
            DmStringDestroy(&p->accesser.deviceName);
            p->accesser.deviceName = DmStringCreate(newDeviceName);
            changed = true;
        }
        if (strcmp(DmStringCstr(&p->accessee.deviceId), localUdid) == 0) {
            DmStringDestroy(&p->accessee.deviceName);
            p->accessee.deviceName = DmStringCreate(newDeviceName);
            changed = true;
        }
        if (changed) {
            char key[32];
            DmAclKeyFromProfile(key, sizeof(key), p);
            DmString json = DmLocalAclProfileSerialize(p);
            DmKvAdapterPut(g_aclKvAdapter, key, DmStringCstr(&json));
            DmStringDestroy(&json);
            updated++;
        }
        DmLocalAclProfileDelete(p);
        all.data[i] = NULL;
    }
    free(all.data);
    (void)updated;
    return DM_OK;
}

int32_t DmAclStoreGetByCredId(DmVecVoid* profiles, const char* credId)
{
    if (credId == NULL) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DmAclStoreInit();
    DmVecVoid all;
    DmVecVoidInit(&all);
    DmAclStoreGetAll(&all);
    for (int32_t i = 0; i < DmVecVoidSize(&all); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&all, i);
        if (p == NULL) {
            continue;
        }
        if (strcmp(DmStringCstr(&p->accesser.credentialId), credId) == 0) {
            DmVecVoidPush(profiles, p);
            all.data[i] = NULL;
        } else {
            DmLocalAclProfileDelete(p);
            all.data[i] = NULL;
        }
    }
    free(all.data);
    return DM_OK;
}
