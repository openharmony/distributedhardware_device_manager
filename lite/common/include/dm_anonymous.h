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


#ifndef DM_ANONYMOUS_H
#define DM_ANONYMOUS_H

#include "dm_container.h"
#include "dm_log.h"
#include "dm_thread.h"
#include "dm_device_info_c.h"
#include "json_object.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

#define DM_ANONY_MAX_MESSAGE_LEN (40 * 1024 * 1024)
#define DM_ANONY_MAX_MAP_LEN 1000
#define DM_ANONY_MAX_INT_LEN 20
#define DM_ANONY_MAX_ID_LEN 256

extern DM_EXPORT const char* DM_PRINT_LIST_SPLIT;
extern const int32_t DM_LIST_SPLIT_LEN;

typedef struct {
    DmString key;
    int32_t val;
} DmMmapEntryDmStringInt;

typedef struct {
    DmMmapEntryDmStringInt* data;
    int size;
    int cap;
} DmMmapDmStringInt;

int DmMmapDmStringIntInit(DmMmapDmStringInt* m);
int DmMmapDmStringIntInsert(DmMmapDmStringInt* m, DmString key, int32_t val);
int DmMmapDmStringIntSize(DmMmapDmStringInt* m);
void DmMmapDmStringIntClear(DmMmapDmStringInt* m);
void DmMmapDmStringIntDestroy(DmMmapDmStringInt* m);

DM_EXPORT DmString DmGetAnonyString(const DmString* value);
DmString DmGetAnonyStringList(DmVec_DmString* values);
DmString DmGetAnonyInt(int value);
DmString DmGetAnonyInt32(int32_t value);
DmString DmGetAnonyInt64(int64_t value);
DmString DmGetAnonyUint64(uint64_t value);
DmString DmGetAnonyInt32List(DmVec_int* values);
DmString DmGetAnonyIntList(DmVec_int* values);
DmString DmGetAnonyUint64List(DmVec_uint64_t* values);
DmString DmGetIntList(DmVec_int* values);
DmString DmGetInt32List(DmVec_int* values);
bool DmIsNumberString(const DmString* inputString);
bool DmIsString(const DmJsonItemObject* jsonObj, const DmString* key);
DM_EXPORT bool DmIsUint16(const DmJsonItemObject* jsonObj, const DmString* key);
bool DmIsInt32(const DmJsonItemObject* jsonObj, const DmString* key);
bool DmIsUint32(const DmJsonItemObject* jsonObj, const DmString* key);
bool DmIsInt64(const DmJsonItemObject* jsonObj, const DmString* key);
bool DmIsUint64(const DmJsonItemObject* jsonObj, const DmString* key);
bool DmIsArray(const DmJsonItemObject* jsonObj, const DmString* key);
bool DmIsBool(const DmJsonItemObject* jsonObj, const DmString* key);
DmString DmConvertMapToJsonString(DmMap_DmString_DmString* paramMap);
void DmParseMapFromJsonString(const DmString* jsonStr, DmMap_DmString_DmString* paramMap);
bool DmIsInvalidPeerTargetId(const DmPeerTargetId* targetId);
DmString DmConvertCharArrayToString(const char* srcData, uint32_t srcLen);
int32_t DmStringToInt(const DmString* str, int32_t base);
int64_t DmStringToInt64(const DmString* str, int32_t base);
void DmVersionSplitToInt(const DmString* str, char split, DmVec_int* numVec);
bool DmCompareVecNum(DmVec_int* srcVecNum, DmVec_int* sinkVecNum);
bool DmCompareVersion(const DmString* remoteVersion, const DmString* oldVersion);
bool DmGetVersionNumber(const DmString* dmVersion, int32_t* versionNum);
DmString DmComposeStrU16(const DmString* pkgName, uint16_t subscribeId);
DmString DmComposeStrI32(const DmString* pkgName, int32_t id);
DmString DmComposeStrU32(const DmString* pkgName, uint32_t id);
DmString DmGetCallerPkgName(const DmString* pkgName);
uint16_t DmGetSubscribeId(const DmString* pkgName);
bool DmIsIdLengthValid(const DmString* inputID);
bool DmIsMessageLengthValid(const DmString* inputMessage);
bool DmIsValueExist(DmMmapDmStringInt* unorderedmap, const DmString* udid, int32_t userId);
bool DmIsCommonNotifyEventValid(DmCommonNotifyEvent dmCommonNotifyEvent);
DmString DmGetSubStr(const DmString* rawStr, const DmString* separator, int32_t index);
bool DmIsJsonValIntegerString(const DmJsonItemObject* jsonObj, const DmString* key);
DmString DmGetAnonyJsonString(const DmString* value);
int64_t DmGetCurrentTimestamp(void);
int32_t DmConvertStrToInt(const DmString* inputStr);

#ifdef __cplusplus
}
#endif

#endif
