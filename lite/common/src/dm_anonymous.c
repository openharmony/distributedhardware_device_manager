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


#include "dm_anonymous.h"
#include "securec.h"
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>

const char* DM_PRINT_LIST_SPLIT = ", ";
const int32_t DM_LIST_SPLIT_LEN = 2;
const int DM_ANONY_MIN_LEN_FOR_MASK = 2;


int DmMmapDmStringIntInit(DmMmapDmStringInt* m)
{
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
    return 0;
}

int DmMmapDmStringIntInsert(DmMmapDmStringInt* m, DmString key, int32_t val)
{
    if (m->size >= m->cap) {
        int nc = m->cap == 0 ? 8 : m->cap * 2;
        DmMmapEntryDmStringInt* nd = (DmMmapEntryDmStringInt*)malloc(nc * sizeof(DmMmapEntryDmStringInt));
        if (nd && m->data && m->size > 0) {
            if (memcpy_s(nd, nc * sizeof(DmMmapEntryDmStringInt), m->data,
                m->size * sizeof(DmMmapEntryDmStringInt)) != 0) {
                free(nd);
                return -1;
            }
            free(m->data);
        }
        if (!nd) {
            return -1;
        }
        m->data = nd;
        m->cap = nc;
    }
    m->data[m->size].key = key;
    m->data[m->size].val = val;
    m->size++;
    return 0;
}

int DmMmapDmStringIntSize(DmMmapDmStringInt* m)
{
    return m->size;
}

void DmMmapDmStringIntClear(DmMmapDmStringInt* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].key);
    }
    m->size = 0;
}

void DmMmapDmStringIntDestroy(DmMmapDmStringInt* m)
{
    DmMmapDmStringIntClear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

DmString DmGetAnonyString(const DmString* value)
{
    const int32_t shortIdLength = 20;
    const int32_t plaintextLength = 4;
    const int32_t minIdLength = 3;
    int strLen = DmStringSize(value);
    if (strLen < minIdLength) {
        return DmStringCreate("******");
    }
    DmString res = DmStringCreateEmpty();
    const char* v = DmStringCstr(value);
    if (strLen <= shortIdLength) {
        DmStringAppendChar(&res, v[0]);
        DmStringAppend(&res, "******");
        DmStringAppendChar(&res, v[strLen - 1]);
    } else {
        DmString prefix = DmStringSubstr(value, 0, plaintextLength);
        DmStringAppend(&res, DmStringCstr(&prefix));
        DmStringDestroy(&prefix);
        DmStringAppend(&res, "******");
        DmString suffix = DmStringSubstr(value, strLen - plaintextLength, plaintextLength);
        DmStringAppend(&res, DmStringCstr(&suffix));
        DmStringDestroy(&suffix);
    }
    return res;
}

DmString DmGetAnonyStringList(DmVec_DmString* values)
{
    DmString temp = DmStringCreate("[ ");
    bool flag = false;
    for (int i = 0; i < DmVec_DmString_Size(values); i++) {
        DmString* v = DmVec_DmString_At(values, i);
        DmString anony = DmGetAnonyString(v);
        DmStringAppend(&temp, DmStringCstr(&anony));
        DmStringAppend(&temp, DM_PRINT_LIST_SPLIT);
        DmStringDestroy(&anony);
        flag = true;
    }
    if (flag) {
        temp.data[temp.size - DM_LIST_SPLIT_LEN] = '\0';
        temp.size -= DM_LIST_SPLIT_LEN;
    }
    DmStringAppend(&temp, " ]");
    return temp;
}

DmString DmGetAnonyInt32(int32_t value)
{
    char buf[32];
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%d", value) < 0) {
        return DmStringCreate("******");
    }
    int len = (int)strlen(buf);
    if (len == 1) {
        buf[0] = '*';
        return DmStringCreate(buf);
    }
    for (int i = 1; i < len - 1; i++) {
        buf[i] = '*';
    }
    return DmStringCreate(buf);
}

DmString DmGetAnonyInt64(int64_t value)
{
    char buf[32];
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%" PRId64, value) < 0) {
        return DmStringCreate("******");
    }
    int len = (int)strlen(buf);
    if (len == 1) {
        buf[0] = '*';
        return DmStringCreate(buf);
    }
    if (len == DM_ANONY_MIN_LEN_FOR_MASK) {
        buf[1] = '*';
        return DmStringCreate(buf);
    }
    for (int i = 1; i < len - 1; i++) {
        buf[i] = '*';
    }
    return DmStringCreate(buf);
}

DmString DmGetAnonyUint64(uint64_t value)
{
    char buf[32];
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%" PRIu64, value) < 0) {
        return DmStringCreate("******");
    }
    int len = (int)strlen(buf);
    if (len == 1) {
        buf[0] = '*';
        return DmStringCreate(buf);
    }
    if (len == DM_ANONY_MIN_LEN_FOR_MASK) {
        buf[1] = '*';
        return DmStringCreate(buf);
    }
    for (int i = 1; i < len - 1; i++) {
        buf[i] = '*';
    }
    return DmStringCreate(buf);
}

DmString DmGetAnonyInt(int value)
{
    char buf[32];
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%d", value) < 0) {
        return DmStringCreate("******");
    }
    int len = (int)strlen(buf);
    if (len == 1) {
        buf[0] = '*';
        return DmStringCreate(buf);
    }
    for (int i = 1; i < len - 1; i++) {
        buf[i] = '*';
    }
    return DmStringCreate(buf);
}

DmString DmGetAnonyInt32List(DmVec_int* values)
{
    DmString temp = DmStringCreate("[ ");
    bool flag = false;
    for (int i = 0; i < DmVec_int_Size(values); i++) {
        int32_t* v = DmVec_int_At(values, i);
        DmString anony = DmGetAnonyInt32(*v);
        DmStringAppend(&temp, DmStringCstr(&anony));
        DmStringAppend(&temp, DM_PRINT_LIST_SPLIT);
        DmStringDestroy(&anony);
        flag = true;
    }
    if (flag) {
        temp.data[temp.size - DM_LIST_SPLIT_LEN] = '\0';
        temp.size -= DM_LIST_SPLIT_LEN;
    }
    DmStringAppend(&temp, " ]");
    return temp;
}

DmString DmGetAnonyIntList(DmVec_int* values)
{
    DmString temp = DmStringCreate("[ ");
    bool flag = false;
    for (int i = 0; i < DmVec_int_Size(values); i++) {
        int32_t* v = DmVec_int_At(values, i);
        DmString anony = DmGetAnonyInt(*v);
        DmStringAppend(&temp, DmStringCstr(&anony));
        DmStringAppend(&temp, DM_PRINT_LIST_SPLIT);
        DmStringDestroy(&anony);
        flag = true;
    }
    if (flag) {
        temp.data[temp.size - DM_LIST_SPLIT_LEN] = '\0';
        temp.size -= DM_LIST_SPLIT_LEN;
    }
    DmStringAppend(&temp, " ]");
    return temp;
}

DmString DmGetAnonyUint64List(DmVec_uint64_t* values)
{
    DmString temp = DmStringCreate("[ ");
    bool flag = false;
    for (int i = 0; i < DmVec_uint64_t_Size(values); i++) {
        uint64_t* v = DmVec_uint64_t_At(values, i);
        DmString anony = DmGetAnonyUint64(*v);
        DmStringAppend(&temp, DmStringCstr(&anony));
        DmStringAppend(&temp, DM_PRINT_LIST_SPLIT);
        DmStringDestroy(&anony);
        flag = true;
    }
    if (flag) {
        temp.data[temp.size - DM_LIST_SPLIT_LEN] = '\0';
        temp.size -= DM_LIST_SPLIT_LEN;
    }
    DmStringAppend(&temp, " ]");
    return temp;
}

DmString DmGetIntList(DmVec_int* values)
{
    DmString temp = DmStringCreate("[ ");
    bool flag = false;
    for (int i = 0; i < DmVec_int_Size(values); i++) {
        int32_t* v = DmVec_int_At(values, i);
        char buf[32];
        if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%d", *v) < 0) {
            DmStringAppend(&temp, "******");
            flag = true;
            DmStringAppend(&temp, DM_PRINT_LIST_SPLIT);
            continue;
        }
        DmStringAppend(&temp, buf);
        DmStringAppend(&temp, DM_PRINT_LIST_SPLIT);
        flag = true;
    }
    if (flag) {
        temp.data[temp.size - DM_LIST_SPLIT_LEN] = '\0';
        temp.size -= DM_LIST_SPLIT_LEN;
    }
    DmStringAppend(&temp, " ]");
    return temp;
}

DmString DmGetInt32List(DmVec_int* values)
{
    DmString temp = DmStringCreate("[ ");
    bool flag = false;
    for (int i = 0; i < DmVec_int_Size(values); i++) {
        int32_t* v = DmVec_int_At(values, i);
        char buf[32];
        if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%d", *v) < 0) {
            DmStringAppend(&temp, "******");
            DmStringAppend(&temp, DM_PRINT_LIST_SPLIT);
            flag = true;
            continue;
        }
        DmStringAppend(&temp, buf);
        DmStringAppend(&temp, DM_PRINT_LIST_SPLIT);
        flag = true;
    }
    if (flag) {
        temp.data[temp.size - DM_LIST_SPLIT_LEN] = '\0';
        temp.size -= DM_LIST_SPLIT_LEN;
    }
    DmStringAppend(&temp, " ]");
    return temp;
}

bool DmIsNumberString(const DmString* inputString)
{
    if (DmStringSize(inputString) == 0 || DmStringSize(inputString) > DM_ANONY_MAX_INT_LEN) {
        LOGE("inputString is Null or inputString length is too long");
        return false;
    }
    const int32_t minAsciiNum = 48;
    const int32_t maxAsciiNum = 57;
    const char* s = DmStringCstr(inputString);
    for (int i = 0; i < DmStringSize(inputString); i++) {
        int num = (int)s[i];
        if (num >= minAsciiNum && num <= maxAsciiNum) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool DmIsInvalidPeerTargetId(const DmPeerTargetId* targetId)
{
    return DmStringEmpty(&targetId->deviceId) && DmStringEmpty(&targetId->brMac) &&
        DmStringEmpty(&targetId->bleMac) && DmStringEmpty(&targetId->wifiIp) &&
        (targetId->serviceId == 0);
}

DmString DmConvertCharArrayToString(const char* srcData, uint32_t srcLen)
{
    if (srcData == NULL || srcLen == 0 || srcLen >= DM_ANONY_MAX_MESSAGE_LEN) {
        LOGE("Invalid parameter.");
        return DmStringCreateEmpty();
    }
    char* dstData = (char*)calloc(srcLen + 1, 1);
    if (memcpy_s(dstData, srcLen + 1, srcData, srcLen) != 0) {
        LOGE("memcpy_s failed.");
        free(dstData);
        return DmStringCreateEmpty();
    }
    DmString temp = DmStringCreate(dstData);
    free(dstData);
    return temp;
}

int32_t DmStringToInt(const DmString* str, int32_t base)
{
    if (DmStringEmpty(str)) {
        LOGE("Str is empty.");
        return 0;
    }
    char* nextPtr = NULL;
    long result = strtol(DmStringCstr(str), &nextPtr, base);
    if (errno == ERANGE || *nextPtr != '\0') {
        LOGE("parse int error");
        return 0;
    }
    return (int32_t)result;
}

int64_t DmStringToInt64(const DmString* str, int32_t base)
{
    if (DmStringEmpty(str)) {
        LOGE("Str is empty.");
        return 0;
    }
    char* nextPtr = NULL;
    int64_t result = strtoll(DmStringCstr(str), &nextPtr, base);
    if (errno == ERANGE || nextPtr == NULL || nextPtr == DmStringCstr(str) || *nextPtr != '\0') {
        LOGE("parse int error");
        return 0;
    }
    return result;
}

void DmVersionSplitToInt(const DmString* str, char split, DmVec_int* numVec)
{
    if (DmStringEmpty(str)) {
        return;
    }
    const char* s = DmStringCstr(str);
    int len = DmStringSize(str);
    if (len < 0 || len >= DM_ANONY_MAX_MESSAGE_LEN) {
        return;
    }
    char* copy = (char*)malloc(len + 1);
    if (!copy) {
        return;
    }
    if (memcpy_s(copy, len + 1, s, len + 1) != 0) {
        free(copy);
        return;
    }
    int pos = 0;
    for (int i = 0; i <= len; i++) {
        if (i == len || copy[i] == split) {
            copy[i] = '\0';
            DmVec_int_Push(numVec, atoi(copy + pos));
            pos = i + 1;
        }
    }
    free(copy);
}

bool DmCompareVecNum(DmVec_int* srcVecNum, DmVec_int* sinkVecNum)
{
    int minSize = DmVec_int_Size(srcVecNum) < DmVec_int_Size(sinkVecNum) ?
        DmVec_int_Size(srcVecNum) : DmVec_int_Size(sinkVecNum);
    for (int index = 0; index < minSize; index++) {
        int32_t* sv = DmVec_int_At(srcVecNum, index);
        int32_t* sk = DmVec_int_At(sinkVecNum, index);
        if (*sv > *sk) {
            return true;
        } else if (*sv < *sk) {
            return false;
        }
    }
    if (DmVec_int_Size(srcVecNum) > DmVec_int_Size(sinkVecNum)) {
        return true;
    }
    return false;
}

bool DmCompareVersion(const DmString* remoteVersion, const DmString* oldVersion)
{
    LOGI("remoteVersion %{public}s, oldVersion %{public}s.", DmStringCstr(remoteVersion), DmStringCstr(oldVersion));
    DmVec_int remoteVersionVec;
    DmVec_int oldVersionVec;
    DmVec_int_Init(&remoteVersionVec);
    DmVec_int_Init(&oldVersionVec);
    DmVersionSplitToInt(remoteVersion, '.', &remoteVersionVec);
    DmVersionSplitToInt(oldVersion, '.', &oldVersionVec);
    bool result = DmCompareVecNum(&remoteVersionVec, &oldVersionVec);
    DmVec_int_Destroy(&remoteVersionVec);
    DmVec_int_Destroy(&oldVersionVec);
    return result;
}

bool DmGetVersionNumber(const DmString* dmVersion, int32_t* versionNum)
{
    LOGI("dmVersion %{public}s,", DmStringCstr(dmVersion));
    DmString number = DmStringCreateEmpty();
    const char* s = DmStringCstr(dmVersion);
    int len = DmStringSize(dmVersion);
    if (len < 0 || len >= DM_ANONY_MAX_MESSAGE_LEN) {
        DmStringDestroy(&number);
        return false;
    }
    char* copy = (char*)malloc(len + 1);
    if (!copy) {
        DmStringDestroy(&number);
        return false;
    }
    if (memcpy_s(copy, len + 1, s, len + 1) != 0) {
        free(copy);
        DmStringDestroy(&number);
        return false;
    }
    int pos = 0;
    for (int i = 0; i <= len; i++) {
        if (i == len || copy[i] == '.') {
            copy[i] = '\0';
            DmStringAppend(&number, copy + pos);
            pos = i + 1;
        }
    }
    free(copy);
    LOGI("number %{public}s,", DmStringCstr(&number));
    *versionNum = atoi(DmStringCstr(&number));
    if (*versionNum <= 0) {
        LOGE("convert failed, number: %{public}s,", DmStringCstr(&number));
        DmStringDestroy(&number);
        return false;
    }
    DmStringDestroy(&number);
    return true;
}

DmString DmComposeStrU16(const DmString* pkgName, uint16_t subscribeId)
{
    DmString res = DmStringCreate(DmStringCstr(pkgName));
    DmStringAppend(&res, "#");
    char buf[32];
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%u", subscribeId) < 0) {
        return res;
    }
    DmStringAppend(&res, buf);
    return res;
}

DmString DmComposeStrI32(const DmString* pkgName, int32_t id)
{
    DmString res = DmStringCreate(DmStringCstr(pkgName));
    DmStringAppend(&res, "#");
    char buf[32];
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%d", id) < 0) {
        return res;
    }
    DmStringAppend(&res, buf);
    return res;
}

DmString DmComposeStrU32(const DmString* pkgName, uint32_t id)
{
    DmString res = DmStringCreate(DmStringCstr(pkgName));
    DmStringAppend(&res, "#");
    char buf[32];
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%u", id) < 0) {
        return res;
    }
    DmStringAppend(&res, buf);
    return res;
}

DmString DmGetCallerPkgName(const DmString* pkgName)
{
    const char* s = DmStringCstr(pkgName);
    int len = DmStringSize(pkgName);
    int end = 0;
    while (end < len && s[end] != '#') {
        end++;
    }
    return DmStringSubstr(pkgName, 0, end);
}

uint16_t DmGetSubscribeId(const DmString* pkgName)
{
    DmVec_DmString strVec;
    DmVec_DmString_Init(&strVec);
    int subIdIndex = 1;
    const char* s = DmStringCstr(pkgName);
    int len = DmStringSize(pkgName);
    int start = 0;
    for (int i = 0; i <= len; i++) {
        if (i == len || s[i] == '#') {
            DmString sub = DmStringSubstr(pkgName, start, i - start);
            DmVec_DmString_Push(&strVec, sub);
            start = i + 1;
        }
    }
    uint16_t result = 0;
    if (DmVec_DmString_Size(&strVec) >= subIdIndex + 1) {
        DmString* elem = DmVec_DmString_At(&strVec, subIdIndex);
        if (elem) {
            result = (uint16_t)atoi(DmStringCstr(elem));
        }
    }
    DmVec_DmString_Destroy(&strVec);
    return result;
}

bool DmIsIdLengthValid(const DmString* inputID)
{
    if (DmStringEmpty(inputID) || DmStringSize(inputID) > DM_ANONY_MAX_ID_LEN) {
        LOGE("On parameter length error, maybe empty or beyond MAX_ID_LEN!");
        return false;
    }
    return true;
}

bool DmIsMessageLengthValid(const DmString* inputMessage)
{
    if (DmStringEmpty(inputMessage) || DmStringSize(inputMessage) > DM_ANONY_MAX_MESSAGE_LEN) {
        LOGE("On parameter error, maybe empty or beyond MAX_MESSAGE_LEN!");
        return false;
    }
    return true;
}

bool DmIsValueExist(DmMmapDmStringInt* unorderedmap, const DmString* udid, int32_t userId)
{
    for (int i = 0; i < unorderedmap->size; i++) {
        if (DmStringCmp(&unorderedmap->data[i].key, udid) == 0 && unorderedmap->data[i].val == userId) {
            return true;
        }
    }
    return false;
}

bool DmIsCommonNotifyEventValid(DmCommonNotifyEvent dmCommonNotifyEvent)
{
    if (dmCommonNotifyEvent > DM_NOTIFY_EVENT_MIN && dmCommonNotifyEvent < DM_NOTIFY_EVENT_MAX) {
        return true;
    }
    return false;
}

DmString DmGetSubStr(const DmString* rawStr, const DmString* separator, int32_t index)
{
    if (DmStringEmpty(rawStr) || DmStringEmpty(separator) || index < 0) {
        LOGE("param invalid");
        return DmStringCreateEmpty();
    }
    DmVec_DmString strVec;
    DmVec_DmString_Init(&strVec);
    const char* s = DmStringCstr(rawStr);
    const char* sep = DmStringCstr(separator);
    int sepLen = DmStringSize(separator);
    int rawLen = DmStringSize(rawStr);
    int start = 0;
    while (start < rawLen) {
        const char* found = strstr(s + start, sep);
        if (found) {
            int end = (int)(found - s);
            DmString sub = DmStringSubstr(rawStr, start, end - start);
            DmVec_DmString_Push(&strVec, sub);
            start = end + sepLen;
        } else {
            break;
        }
    }
    DmString last = DmStringSubstr(rawStr, start, rawLen - start);
    DmVec_DmString_Push(&strVec, last);
    DmString result = DmStringCreateEmpty();
    if (DmVec_DmString_Size(&strVec) >= index + 1) {
        DmString* elem = DmVec_DmString_At(&strVec, index);
        if (elem) {
            result = DmStringCopy(elem);
        }
    } else {
        LOGE("get failed");
    }
    DmVec_DmString_Destroy(&strVec);
    return result;
}

int64_t DmGetCurrentTimestamp(void)
{
    return (int64_t)DmGetTimestampMs();
}

int32_t DmConvertStrToInt(const DmString* inputStr)
{
    if (!DmIsNumberString(inputStr)) {
        LOGE("Invalid parameter, param is not number.");
        return 0;
    }
    int32_t outputNumber = atoi(DmStringCstr(inputStr));
    if (outputNumber < 0) {
        LOGE("Invalid parameter, param out of range.");
        return 0;
    }
    return outputNumber;
}

bool DmIsString(const DmJsonItemObject* jsonObj, const DmString* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(jsonObj, DmStringCstr(key));
    return DmJsonItemObjectIsString(&item);
}

bool DmIsUint16(const DmJsonItemObject* jsonObj, const DmString* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(jsonObj, DmStringCstr(key));
    return DmJsonItemObjectIsNumberInteger(&item);
}

bool DmIsInt32(const DmJsonItemObject* jsonObj, const DmString* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(jsonObj, DmStringCstr(key));
    return DmJsonItemObjectIsNumberInteger(&item);
}

bool DmIsUint32(const DmJsonItemObject* jsonObj, const DmString* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(jsonObj, DmStringCstr(key));
    return DmJsonItemObjectIsNumberInteger(&item);
}

bool DmIsInt64(const DmJsonItemObject* jsonObj, const DmString* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(jsonObj, DmStringCstr(key));
    return DmJsonItemObjectIsNumberInteger(&item);
}

bool DmIsUint64(const DmJsonItemObject* jsonObj, const DmString* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(jsonObj, DmStringCstr(key));
    return DmJsonItemObjectIsNumberInteger(&item);
}

bool DmIsArray(const DmJsonItemObject* jsonObj, const DmString* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(jsonObj, DmStringCstr(key));
    return DmJsonItemObjectIsArray(&item);
}

bool DmIsBool(const DmJsonItemObject* jsonObj, const DmString* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(jsonObj, DmStringCstr(key));
    return DmJsonItemObjectIsBoolean(&item);
}

DmString DmConvertMapToJsonString(DmMap_DmString_DmString* paramMap)
{
    if (paramMap == NULL || paramMap->size == 0) {
        return DmStringCreate("{}");
    }
    DmString result = DmStringCreate("{");
    for (int i = 0; i < paramMap->size; i++) {
        DmStringAppend(&result, "\"");
        DmStringAppend(&result, DmStringCstr(&paramMap->data[i].key));
        DmStringAppend(&result, "\":\"");
        DmStringAppend(&result, DmStringCstr(&paramMap->data[i].val));
        DmStringAppend(&result, "\"");
        if (i < paramMap->size - 1) {
            DmStringAppend(&result, ",");
        }
    }
    DmStringAppend(&result, "}");
    return result;
}

void DmParseMapFromJsonString(const DmString* jsonStr, DmMap_DmString_DmString* paramMap)
{
    if (jsonStr == NULL || paramMap == NULL) {
        return;
    }
    DmMap_DmString_DmString_Clear(paramMap);
    DmJsonItemObject obj;
    DmJsonItemObjectInit(&obj);
    DmJsonItemObjectParse(&obj, DmStringCstr(jsonStr));
    DmVec_DmJsonItemObject items = DmJsonItemObjectItems(&obj);
    for (int i = 0; i < DmVec_DmJsonItemObject_Size(&items); i++) {
        DmJsonItemObject* item = DmVec_DmJsonItemObject_At(&items, i);
        DmString key = DmJsonItemObjectKey(item);
        DmString val = DmJsonGetString(item);
        DmMap_DmString_DmString_Insert(paramMap, key, val);
    }
    DmVec_DmJsonItemObject_Destroy(&items);
    DmJsonItemObjectDestroy(&obj);
}

DmString DmGetAnonyJsonString(const DmString* value)
{
    CHECK_NULL_RETURN(value, DmStringCreateEmpty());
    return DmStringCopy(value);
}
