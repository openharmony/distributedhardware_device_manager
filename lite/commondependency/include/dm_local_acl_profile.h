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


#ifndef DM_LOCAL_ACL_PROFILE_H
#define DM_LOCAL_ACL_PROFILE_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_container.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ACL_KEY_BIND_TYPE         "bindType"
#define ACL_KEY_AUTH_TYPE        "authenticationType"
#define ACL_KEY_BIND_LEVEL       "bindLevel"
#define ACL_KEY_TRUST_DEVICE_ID  "trustDeviceId"
#define ACL_KEY_ACCESSER         "accesser"
#define ACL_KEY_ACCESSEE         "accessee"
#define ACL_KEY_DEVICE_ID        "deviceId"
#define ACL_KEY_USER_ID          "userId"
#define ACL_KEY_ACCOUNT_ID      "accountId"
#define ACL_KEY_TOKEN_ID         "tokenId"
#define ACL_KEY_BUNDLE_NAME      "bundleName"
#define ACL_KEY_DEVICE_NAME      "deviceName"
#define ACL_KEY_CREDENTIAL_ID    "credentialId"

typedef struct {
    int32_t bindType;
    int32_t authenticationType;
    int32_t bindLevel;
    DmString trustDeviceId;
    struct {
        DmString deviceId;
        int32_t userId;
        DmString accountId;
        uint64_t tokenId;
        DmString bundleName;
        DmString deviceName;
        DmString credentialId;
    } accesser;
    struct {
        DmString deviceId;
        int32_t userId;
        DmString accountId;
        uint64_t tokenId;
        DmString bundleName;
        DmString deviceName;
        DmString credentialId;
    } accessee;
} DmLocalAclProfile;

void DmLocalAclProfileInit(DmLocalAclProfile* p);
void DmLocalAclProfileDestroy(DmLocalAclProfile* p);
DmLocalAclProfile* DmLocalAclProfileCreate(void);
void DmLocalAclProfileDelete(DmLocalAclProfile* p);

DmString DmLocalAclProfileSerialize(const DmLocalAclProfile* p);
bool DmLocalAclProfileDeserialize(DmLocalAclProfile* p, const char* json);

void DmAclKeyFromProfile(char* buf, size_t bufSize, const DmLocalAclProfile* p);
int32_t DmAclStoreInit(void);
int32_t DmAclStorePut(DmLocalAclProfile* profile);
int32_t DmAclStoreGetAll(DmVecVoid* profiles);
int32_t DmAclStoreGetByUdid(DmVecVoid* profiles, const char* localUdid, int32_t userId, const char* remoteUdid);
int32_t DmAclStoreDeleteByUdid(const char* localUdid, int32_t userId, const char* remoteUdid);
int32_t DmAclStoreDeleteByKey(const char* key);
int32_t DmAclStoreUpdateDeviceName(const char* localUdid, const char* newDeviceName);
int32_t DmAclStoreGetByCredId(DmVecVoid* profiles, const char* credId);

#ifdef __cplusplus
}
#endif

#endif
