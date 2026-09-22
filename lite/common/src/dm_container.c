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


#include "dm_container.h"
#include "dm_device_info_c.h"
#include "securec.h"

#define DM_HASH_SEED 5381
#define DM_HASH_MULTIPLIER 0x45d9f3b
#define DM_HASH_SHIFT_BITS 16
#define DM_STRING_HASH_SHIFT 5
const int DM_STRING_EXTRA_SPACE = 2;

DmString DmStringCreate(const char* src)
{
    DmString s;
    if (!src) {
        s.data = malloc(1);
        if (s.data) {
            s.data[0] = '\0';
        }
        s.size = 0;
        s.cap = 1;
        return s;
    }
    int len = strlen(src);
    s.cap = len + 1;
    s.data = malloc(s.cap);
    if (s.data) {
        if (memcpy_s(s.data, s.cap, src, len + 1) != 0) {
            free(s.data);
            s.data = NULL;
            s.size = 0;
            s.cap = 0;
            return s;
        }
        s.size = len;
    } else {
        s.size = 0;
        s.cap = 0;
    }
    return s;
}

DmString DmStringCreateEmpty(void)
{
    DmString s;
    s.data = malloc(1);
    if (s.data) {
        s.data[0] = '\0';
    }
    s.size = 0;
    s.cap = 1;
    return s;
}

DmString DmStringCreateLen(const char* src, int len)
{
    DmString s;
    if (!src || len <= 0) {
        return DmStringCreateEmpty();
    }
    s.cap = len + 1;
    s.data = malloc(s.cap);
    if (s.data) {
        if (memcpy_s(s.data, s.cap, src, len) != 0) {
            free(s.data);
            s.data = NULL;
            s.size = 0;
            s.cap = 0;
            return s;
        }
        s.data[len] = '\0';
        s.size = len;
    } else {
        s.size = 0;
        s.cap = 0;
    }
    return s;
}

void DmStringDestroy(DmString* s)
{
    if (s) {
        free(s->data);
        s->data = NULL;
        s->size = 0;
        s->cap = 0;
    }
}

static int DmStringGrow(DmString* s, int needCap)
{
    if (needCap <= s->cap) {
        return 0;
    }
    int nc = s->cap == 0 ? (needCap > 8 ? needCap : 8) : (needCap > s->cap * 2 ? needCap : s->cap * 2);
    char* nd = (char*)malloc(nc);
    if (!nd) {
        return -1;
    }
    if (s->data && s->size > 0) {
        if (memcpy_s(nd, nc, s->data, s->size) != 0) {
            free(nd);
            return -1;
        }
        free(s->data);
    }
    s->data = nd;
    s->cap = nc;
    return 0;
}

DmString DmStringAppend(DmString* s, const char* str)
{
    if (!s || !str) {
        return s ? *s : DmStringCreateEmpty();
    }
    int addLen = strlen(str);
    if (addLen == 0) {
        return *s;
    }
    int needCap = s->size + addLen + 1;
    if (DmStringGrow(s, needCap) != 0) {
        return *s;
    }
    if (memcpy_s(s->data + s->size, s->cap - s->size, str, addLen + 1) != 0) {
        return *s;
    }
    s->size += addLen;
    return *s;
}

DmString DmStringAppendChar(DmString* s, char c)
{
    if (!s) {
        return DmStringCreateEmpty();
    }
    if (DmStringGrow(s, s->size + DM_STRING_EXTRA_SPACE) != 0) {
        return *s;
    }
    s->data[s->size] = c;
    s->data[s->size + 1] = '\0';
    s->size++;
    return *s;
}

int DmStringCmp(const DmString* a, const DmString* b)
{
    return strcmp(DmStringCstr(a), DmStringCstr(b));
}

int DmStringCmpCstr(const DmString* a, const char* b)
{
    return strcmp(DmStringCstr(a), b ? b : "");
}

const char* DmStringCstr(const DmString* s)
{
    return s->data ? s->data : "";
}

int DmStringSize(const DmString* s)
{
    return (int)s->size;
}

bool DmStringEmpty(const DmString* s)
{
    return s->size == 0;
}

DmString DmStringSubstr(const DmString* s, int start, int len)
{
    if (start < 0 || len < 0 || start >= (int)s->size) {
        return DmStringCreateEmpty();
    }
    int actualLen = len;
    if (start + actualLen > (int)s->size) {
        actualLen = (int)s->size - start;
    }
    DmString result;
    result.cap = actualLen + 1;
    result.data = malloc(result.cap);
    if (result.data) {
        if (memcpy_s(result.data, result.cap, s->data + start, actualLen) != 0) {
            free(result.data);
            result.data = NULL;
            result.size = 0;
            result.cap = 0;
            return result;
        }
        result.data[actualLen] = '\0';
        result.size = actualLen;
    } else {
        result.size = 0;
        result.cap = 0;
    }
    return result;
}

int DmStringFind(const DmString* s, const char* sub)
{
    if (!s->data || s->size == 0 || !sub) {
        return -1;
    }
    const char* found = strstr(s->data, sub);
    if (!found) {
        return -1;
    }
    return found - s->data;
}

DmString DmStringReplace(DmString* s, const char* old_str, const char* newStr)
{
    if (!s) {
        return DmStringCreateEmpty();
    }
    if (!old_str || !s->data || s->size == 0 || strlen(old_str) == 0) {
        return *s;
    }
    int oldLen = strlen(old_str);
    DmString result = DmStringCreateEmpty();
    const char* pos = s->data;
    const char* end = s->data + s->size;
    while (pos < end) {
        const char* found = strstr(pos, old_str);
        if (!found) {
            DmStringAppend(&result, pos);
            break;
        }
        if (found > pos) {
            int beforeLen = found - pos;
            char* before = malloc(beforeLen + 1);
            if (before != NULL && memcpy_s(before, beforeLen + 1, pos, beforeLen) == 0) {
                before[beforeLen] = '\0';
                DmStringAppend(&result, before);
            }
            if (before != NULL) {
                free(before);
            }
        }
        DmStringAppend(&result, newStr);
        pos = found + oldLen;
    }
    DmStringDestroy(s);
    *s = result;
    return *s;
}

void DmStringClear(DmString* s)
{
    if (!s) {
        return;
    }
    free(s->data);
    s->data = malloc(1);
    if (s->data) {
        s->data[0] = '\0';
    }
    s->size = 0;
    s->cap = 1;
}

DmString DmStringCopy(const DmString* src)
{
    return DmStringCreate(DmStringCstr(src));
}

void DmStringCopyTo(DmString* dest, const DmString* src)
{
    DmStringDestroy(dest);
    *dest = DmStringCopy(src);
}

void DmStringAssign(DmString* dest, const DmString* src)
{
    DmStringDestroy(dest);
    *dest = DmStringCopy(src);
}

void DmStringAssignCstr(DmString* dest, const char* src)
{
    DmStringDestroy(dest);
    *dest = DmStringCreate(src);
}

unsigned int DmStringHash(const DmString* s)
{
    unsigned int hash = DM_HASH_SEED;
    const char* str = DmStringCstr(s);
    while (*str) {
        hash = ((hash << DM_STRING_HASH_SHIFT) + hash) + (unsigned char)(*str);
        str++;
    }
    return hash;
}

int DmCmpInt(int a, int b)
{
    return (a > b) - (a < b);
}

int DmCmpInt32T(int32_t a, int32_t b)
{
    return (a > b) - (a < b);
}

int DmCmpInt64T(int64_t a, int64_t b)
{
    return (a > b) - (a < b);
}

int DmCmpUint64T(uint64_t a, uint64_t b)
{
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

int DmCmpDmString(DmString a, DmString b)
{
    return DmStringCmp(&a, &b);
}

unsigned int DmHashInt(int key)
{
    unsigned int k = (unsigned int)key;
    k = ((k >> DM_HASH_SHIFT_BITS) ^ k) * DM_HASH_MULTIPLIER;
    k = ((k >> DM_HASH_SHIFT_BITS) ^ k) * DM_HASH_MULTIPLIER;
    k = (k >> DM_HASH_SHIFT_BITS) ^ k;
    return k;
}

unsigned int DmHashDmString(DmString key)
{
    return DmStringHash(&key);
}

DM_VEC_IMPL(int)
DM_VEC_IMPL(int32_t)
DM_VEC_IMPL(uint8_t)
DM_VEC_IMPL(uint32_t)
DM_VEC_IMPL(int64_t)
DM_VEC_IMPL(uint64_t)
DM_VEC_IMPL(DmPair_int64_t_int64_t)
DM_VEC_IMPL_NO_DESTROY(DmString)
DM_VEC_IMPL_NO_DESTROY(DmCharPtr)
DM_VEC_IMPL(DmVoidPtr)
DM_VEC_IMPL(ProcessInfo)

void DmVec_DmString_Clear(DmVec_DmString* v)
{
    for (int i = 0; i < v->size; i++) {
        DmStringDestroy(&v->data[i]);
    }
    v->size = 0;
}
void DmVec_DmString_Destroy(DmVec_DmString* v)
{
    DmVec_DmString_Clear(v);
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->cap = 0;
}

void DmVec_DmCharPtr_Clear(DmVec_DmCharPtr* v)
{
    for (int i = 0; i < v->size; i++) {
        free(v->data[i]);
    }
    v->size = 0;
}
void DmVec_DmCharPtr_Destroy(DmVec_DmCharPtr* v)
{
    DmVec_DmCharPtr_Clear(v);
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->cap = 0;
}

DM_MAP_IMPL(int_int, int, int, DmCmpInt)
void DmMap_int_int_Clear(DmMap_int_int* m)
{
    m->size = 0;
}
void DmMap_int_int_Destroy(DmMap_int_int* m)
{
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}
DM_MAP_IMPL(DmString_DmDeviceInfo, DmString, DmDeviceInfo, DmCmpDmString)
DM_MAP_IMPL(DmString_DmString, DmString, DmString, DmCmpDmString)
DM_MAP_IMPL(int_DmString, int, DmString, DmCmpInt)
DM_MAP_IMPL(DmString_int, DmString, int, DmCmpDmString)
DM_MAP_IMPL(DmString_uint16_t, DmString, uint16_t, DmCmpDmString)
DM_MAP_IMPL(uint64_t_DmVoidPtr, uint64_t, DmVoidPtr, DmCmpUint64T)
DM_MAP_IMPL(int_DmVoidPtr, int, DmVoidPtr, DmCmpInt)

void DmMap_int_DmVoidPtr_Clear(DmMap_int_DmVoidPtr* m)
{
    m->size = 0;
}
void DmMap_int_DmVoidPtr_Destroy(DmMap_int_DmVoidPtr* m)
{
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

DM_MAP_IMPL(int_DmSetDmString, int, DmSetDmString, DmCmpInt)

void DmMap_uint64_t_uint64_t_Clear(DmMap_uint64_t_uint64_t* m)
{
    m->size = 0;
}
void DmMap_uint64_t_uint64_t_Destroy(DmMap_uint64_t_uint64_t* m)
{
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}
DM_MAP_IMPL(uint64_t_uint64_t, uint64_t, uint64_t, DmCmpUint64T)

void DmMap_uint64_t_int_Clear(DmMap_uint64_t_int* m)
{
    m->size = 0;
}
void DmMap_uint64_t_int_Destroy(DmMap_uint64_t_int* m)
{
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}
DM_MAP_IMPL(uint64_t_int, uint64_t, int, DmCmpUint64T)

void DmMap_int_DmSetDmStringClear(DmMap_int_DmSetDmString* m)
{
    for (int i = 0; i < m->size; i++) {
        DmSetDmStringDestroy(&m->data[i].val);
    }
    m->size = 0;
}
void DmMap_int_DmSetDmStringDestroy(DmMap_int_DmSetDmString* m)
{
    DmMap_int_DmSetDmStringClear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

DM_MAP_IMPL(uint16_uint16, uint16_t, uint16_t, DmCmpInt)
DM_MAP_IMPL(DmString_DmMap_int_int, DmString, DmMap_int_int, DmCmpDmString)
DM_MAP_IMPL(DmString_DmMap_uint16_uint16, DmString, DmMap_uint16_uint16, DmCmpDmString)
DM_MAP_IMPL(DmString_DmSet_int, DmString, DmSet_int, DmCmpDmString)


void DmMap_DmString_DmSet_int_Clear(DmMap_DmString_DmSet_int* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].key);
        DmSet_int_Destroy(&m->data[i].val);
    }
    m->size = 0;
}
void DmMap_DmString_DmSet_int_Destroy(DmMap_DmString_DmSet_int* m)
{
    DmMap_DmString_DmSet_int_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMapIntIntClear(DmMap_int_int* m)
{
    m->size = 0;
}
void DmMapIntIntDestroy(DmMap_int_int* m)
{
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMap_DmString_DmString_Clear(DmMap_DmString_DmString* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].key);
        DmStringDestroy(&m->data[i].val);
    }
    m->size = 0;
}
void DmMap_DmString_DmString_Destroy(DmMap_DmString_DmString* m)
{
    DmMap_DmString_DmString_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMap_int_DmString_Clear(DmMap_int_DmString* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].val);
    }
    m->size = 0;
}
void DmMap_int_DmString_Destroy(DmMap_int_DmString* m)
{
    DmMap_int_DmString_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMap_DmString_int_Clear(DmMap_DmString_int* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].key);
    }
    m->size = 0;
}
void DmMap_DmString_int_Destroy(DmMap_DmString_int* m)
{
    DmMap_DmString_int_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMap_DmString_uint16_t_Clear(DmMap_DmString_uint16_t* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].key);
    }
    m->size = 0;
}
void DmMap_DmString_uint16_t_Destroy(DmMap_DmString_uint16_t* m)
{
    DmMap_DmString_uint16_t_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMap_uint64_t_DmVoidPtr_Clear(DmMap_uint64_t_DmVoidPtr* m)
{
    m->size = 0;
}
void DmMap_uint64_t_DmVoidPtr_Destroy(DmMap_uint64_t_DmVoidPtr* m)
{
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMap_DmString_DmDeviceInfo_Clear(DmMap_DmString_DmDeviceInfo* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].key);
    }
    m->size = 0;
}
void DmMap_DmString_DmDeviceInfo_Destroy(DmMap_DmString_DmDeviceInfo* m)
{
    DmMap_DmString_DmDeviceInfo_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMap_DmString_DmMap_int_int_Clear(DmMap_DmString_DmMap_int_int* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].key);
        DmMapIntIntDestroy(&m->data[i].val);
    }
    m->size = 0;
}
void DmMap_DmString_DmMap_int_int_Destroy(DmMap_DmString_DmMap_int_int* m)
{
    DmMap_DmString_DmMap_int_int_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMap_uint16_uint16_Clear(DmMap_uint16_uint16* m)
{
    m->size = 0;
}
void DmMap_uint16_uint16_Destroy(DmMap_uint16_uint16* m)
{
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmMap_DmString_DmMap_uint16_uint16_Clear(DmMap_DmString_DmMap_uint16_uint16* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].key);
        DmMap_uint16_uint16_Destroy(&m->data[i].val);
    }
    m->size = 0;
}
void DmMap_DmString_DmMap_uint16_uint16_Destroy(DmMap_DmString_DmMap_uint16_uint16* m)
{
    DmMap_DmString_DmMap_uint16_uint16_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

DM_HMAP_IMPL(DmString_DmString, DmString, DmString, DmHashDmString, DmCmpDmString)
DM_HMAP_IMPL(DmString_DmVoidPtr, DmString, DmVoidPtr, DmHashDmString, DmCmpDmString)
DM_HMAP_IMPL(DmString_uint16_t, DmString, uint16_t, DmHashDmString, DmCmpDmString)
DM_HMAP_IMPL(int_DmVoidPtr, int, DmVoidPtr, DmHashInt, DmCmpInt)

void DmHmap_DmString_DmString_Clear(DmHmap_DmString_DmString* m)
{
    for (int i = 0; i < m->cap; i++) {
        if (m->data[i].state == 1) {
            DmStringDestroy(&m->data[i].key);
            DmStringDestroy(&m->data[i].val);
        }
        m->data[i].state = 0;
    }
    m->size = 0;
}
void DmHmap_DmString_DmString_Destroy(DmHmap_DmString_DmString* m)
{
    DmHmap_DmString_DmString_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmHmap_int_DmVoidPtr_Clear(DmHmap_int_DmVoidPtr* m)
{
    for (int i = 0; i < m->cap; i++) {
        m->data[i].state = 0;
    }
    m->size = 0;
}
void DmHmap_int_DmVoidPtr_Destroy(DmHmap_int_DmVoidPtr* m)
{
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmHmap_DmString_DmVoidPtr_Clear(DmHmap_DmString_DmVoidPtr* m)
{
    for (int i = 0; i < m->cap; i++) {
        if (m->data[i].state == 1) {
            DmStringDestroy(&m->data[i].key);
        }
        m->data[i].state = 0;
    }
    m->size = 0;
}
void DmHmap_DmString_DmVoidPtr_Destroy(DmHmap_DmString_DmVoidPtr* m)
{
    DmHmap_DmString_DmVoidPtr_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmHmap_DmString_uint16_t_Clear(DmHmap_DmString_uint16_t* m)
{
    for (int i = 0; i < m->cap; i++) {
        if (m->data[i].state == 1) {
            DmStringDestroy(&m->data[i].key);
        }
        m->data[i].state = 0;
    }
    m->size = 0;
}
void DmHmap_DmString_uint16_t_Destroy(DmHmap_DmString_uint16_t* m)
{
    DmHmap_DmString_uint16_t_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

DM_HMAP_IMPL(DmString_DmDeviceInfo, DmString, DmDeviceInfo, DmHashDmString, DmCmpDmString)

void DmHmap_DmString_DmDeviceInfo_Clear(DmHmap_DmString_DmDeviceInfo* m)
{
    for (int i = 0; i < m->cap; i++) {
        if (m->data[i].state == 1) {
            DmStringDestroy(&m->data[i].key);
            DmStringDestroy(&m->data[i].val.extraData);
        }
        m->data[i].state = 0;
    }
    m->size = 0;
}
void DmHmap_DmString_DmDeviceInfo_Destroy(DmHmap_DmString_DmDeviceInfo* m)
{
    DmHmap_DmString_DmDeviceInfo_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

int DmSetDmStringInit(DmSetDmString* s)
{
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
    return 0;
}

void DmSetDmStringClear(DmSetDmString* s)
{
    for (int i = 0; i < s->size; i++) {
        DmStringDestroy(&s->data[i]);
    }
    s->size = 0;
}
void DmSetDmStringDestroy(DmSetDmString* s)
{
    DmSetDmStringClear(s);
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

int DmSetDmStringInsert(DmSetDmString* s, const DmString* item)
{
    if (DmSetDmStringContains(s, item)) {
        return 0;
    }
    if (s->size >= s->cap) {
        int nc = s->cap == 0 ? 8 : s->cap * 2;
        DmString* nd = (DmString*)malloc(nc * sizeof(DmString));
        if (!nd) {
            return -1;
        }
        if (s->data && s->size > 0) {
            if (memcpy_s(nd, nc * sizeof(DmString), s->data, s->size * sizeof(DmString)) != 0) {
                free(nd);
                return -1;
            }
            free(s->data);
        }
        s->data = nd;
        s->cap = nc;
    }
    s->data[s->size] = DmStringCopy(item);
    s->size++;
    return 0;
}
bool DmSetDmStringContains(DmSetDmString* s, const DmString* item)
{
    for (int i = 0; i < s->size; i++) {
        if (DmCmpDmString(s->data[i], *item) == 0) {
            return true;
        }
    }
    return false;
}
int DmSetDmStringSize(DmSetDmString* s)
{
    return s->size;
}
void DmSetDmStringErase(DmSetDmString* s, const DmString* item)
{
    for (int i = 0; i < s->size; i++) {
        if (DmCmpDmString(s->data[i], *item) == 0) {
            DmStringDestroy(&s->data[i]);
            if (memmove_s(&s->data[i], (s->cap - i) * sizeof(DmString),
                &s->data[i + 1], (s->size - i - 1) * sizeof(DmString)) != 0) {
                return;
            }
            s->size--;
            return;
        }
    }
}
DM_SET_IMPL(int, DmCmpInt)
DM_SET_IMPL(uint16_t, DmCmpInt)
DM_SET_IMPL(uint32_t, DmCmpInt)
DM_SET_IMPL(uint64_t, DmCmpUint64T)
DM_SET_IMPL(int32_t, DmCmpInt)

void DmSet_int_Clear(DmSet_int* s)
{
    s->size = 0;
}
void DmSet_int_Destroy(DmSet_int* s)
{
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

void DmSet_uint16_t_Clear(DmSet_uint16_t* s)
{
    s->size = 0;
}
void DmSet_uint16_t_Destroy(DmSet_uint16_t* s)
{
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

void DmSet_uint32_t_Clear(DmSet_uint32_t* s)
{
    s->size = 0;
}
void DmSet_uint32_t_Destroy(DmSet_uint32_t* s)
{
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

void DmSet_uint64_t_Clear(DmSet_uint64_t* s)
{
    s->size = 0;
}
void DmSet_uint64_t_Destroy(DmSet_uint64_t* s)
{
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

void DmSet_int32_t_Clear(DmSet_int32_t* s)
{
    s->size = 0;
}
void DmSet_int32_t_Destroy(DmSet_int32_t* s)
{
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}


DM_QUEUE_IMPL(int)
void DmQueue_int_Destroy(DmQueue_int* q)
{
    free(q->data);
    q->data = NULL;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->cap = 0;
}
DM_QUEUE_IMPL(DmString)
DM_QUEUE_IMPL(DmVoidPtr)

void DmQueueIntDestroy(DmQueue_int* q)
{
    free(q->data);
    q->data = NULL;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->cap = 0;
}

void DmQueue_DmString_Destroy(DmQueue_DmString* q)
{
    if (q->cap > 0) {
        for (int i = 0; i < q->size; i++) {
            DmStringDestroy(&q->data[(q->head + i) % q->cap]);
        }
    }
    free(q->data);
    q->data = NULL;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->cap = 0;
}

void DmQueue_DmVoidPtr_Destroy(DmQueue_DmVoidPtr* q)
{
    free(q->data);
    q->data = NULL;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->cap = 0;
}

DM_MMAP_IMPL(DmString_DmString, DmString, DmString, DmCmpDmString)

void DmMmap_DmString_DmString_Clear(DmMmap_DmString_DmString* m)
{
    for (int i = 0; i < m->size; i++) {
        DmStringDestroy(&m->data[i].key);
        DmStringDestroy(&m->data[i].val);
    }
    m->size = 0;
}
void DmMmap_DmString_DmString_Destroy(DmMmap_DmString_DmString* m)
{
    DmMmap_DmString_DmString_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}
