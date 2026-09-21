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


#ifndef DM_CONTAINER_H
#define DM_CONTAINER_H

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "securec.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void DmContainerMemmoveHelper(void* dst, size_t dstSize, const void* src, size_t srcSize)
{
    if (memmove_s(dst, dstSize, src, srcSize) != 0) {
        return;
    }
}

typedef char* DmCharPtr;
typedef void* DmVoidPtr;

typedef struct {
    char* data;
    int size;
    int cap;
} DmString;

DmString DmStringCreate(const char* src);
DmString DmStringCreateEmpty(void);
DmString DmStringCreateLen(const char* src, int len);
void DmStringDestroy(DmString* s);
DmString DmStringAppend(DmString* s, const char* str);
DmString DmStringAppendChar(DmString* s, char c);
int DmStringCmp(const DmString* a, const DmString* b);
int DmStringCmpCstr(const DmString* a, const char* b);
const char* DmStringCstr(const DmString* s);
int DmStringSize(const DmString* s);
bool DmStringEmpty(const DmString* s);
DmString DmStringSubstr(const DmString* s, int start, int len);
int DmStringFind(const DmString* s, const char* sub);
DmString DmStringReplace(DmString* s, const char* oldStr, const char* newStr);
void DmStringClear(DmString* s);
DmString DmStringCopy(const DmString* src);
void DmStringCopyTo(DmString* dest, const DmString* src);
unsigned int DmStringHash(const DmString* s);
void DmStringAssign(DmString* dest, const DmString* src);
void DmStringAssignCstr(DmString* dest, const char* src);

int DmCmpInt(int a, int b);
int DmCmpInt32T(int32_t a, int32_t b);
int DmCmpInt64T(int64_t a, int64_t b);
int DmCmpUint64T(uint64_t a, uint64_t b);
int DmCmpDmString(DmString a, DmString b);
unsigned int DmHashInt(int key);
unsigned int DmHashDmString(DmString key);

#define DM_VEC_CAT(a, b) a##b
#define DM_VEC_DEFINE(T) \
typedef struct { __typeof__(T)* data; int size; int cap; } DmVec_##T; \
int DM_VEC_CAT(DmVec_##T, _Init)(DmVec_##T* v); \
int DM_VEC_CAT(DmVec_##T, _Push)(DmVec_##T* v, __typeof__(T) item); \
__typeof__(T)* DM_VEC_CAT(DmVec_##T, _At)(const DmVec_##T* v, int idx); \
int DM_VEC_CAT(DmVec_##T, _Size)(const DmVec_##T* v); \
void DM_VEC_CAT(DmVec_##T, _Clear)(DmVec_##T* v); \
void DM_VEC_CAT(DmVec_##T, _Destroy)(DmVec_##T* v); \
int DM_VEC_CAT(DmVec_##T, _Erase)(DmVec_##T* v, int idx); \
bool DM_VEC_CAT(DmVec_##T, _Empty)(const DmVec_##T* v)

#define DM_VEC_IMPL_1(T) \
int DM_VEC_CAT(DmVec_##T, _Init)(DmVec_##T* v) { v->data = NULL; v->size = 0; v->cap = 0; return 0; } \
int DM_VEC_CAT(DmVec_##T, _Push)(DmVec_##T* v, __typeof__(T) item) { \
    if (v->size >= v->cap) { \
        int nc = v->cap == 0 ? 8 : v->cap * 2; \
        __typeof__(T)* nd = (T*)realloc(v->data, nc * sizeof(T)); \
        if (!nd) return -1; v->data = nd; v->cap = nc; \
    } v->data[v->size++] = item; return 0; \
}

#define DM_VEC_IMPL_2(T) \
__typeof__(T)* DM_VEC_CAT(DmVec_##T, _At)(const DmVec_##T* v, int idx) { \
    return (idx >= 0 && idx < v->size) ? &v->data[idx] : NULL; \
} \
int DM_VEC_CAT(DmVec_##T, _Size)(const DmVec_##T* v) { return v->size; } \
int DmVec_##T##_Erase(DmVec_##T* v, int idx) { \
    if (idx < 0 || idx >= v->size) return -1; \
    DmContainerMemmoveHelper(&v->data[idx], (v->size - idx) * sizeof(T), \
    &v->data[idx+1], (v->size - idx - 1) * sizeof(T)); v->size--; return 0; \
}

#define DM_VEC_IMPL_3(T) \
bool DmVec_##T##_Empty(const DmVec_##T* v) { return v->size == 0; } \
void DmVec_##T##_Clear(DmVec_##T* v) { v->size = 0; } \
void DmVec_##T##_Destroy(DmVec_##T* v) { free(v->data); v->data = NULL; v->size = 0; v->cap = 0; }

#define DM_VEC_IMPL(T) \
DM_VEC_IMPL_1(T) \
DM_VEC_IMPL_2(T) \
DM_VEC_IMPL_3(T)

#define DM_VEC_IMPL_ND_1(T) \
int DmVec_##T##_Init(DmVec_##T* v) { v->data = NULL; v->size = 0; v->cap = 0; return 0; } \
int DmVec_##T##_Push(DmVec_##T* v, __typeof__(T) item) { \
    if (v->size >= v->cap) { \
        int nc = v->cap == 0 ? 8 : v->cap * 2; \
        __typeof__(T)* nd = (T*)realloc(v->data, nc * sizeof(T)); \
        if (!nd) return -1; v->data = nd; v->cap = nc; \
    } v->data[v->size++] = item; return 0; \
}

#define DM_VEC_IMPL_ND_2(T) \
__typeof__(T)* DmVec_##T##_At(const DmVec_##T* v, int idx) { \
    return (idx >= 0 && idx < v->size) ? &v->data[idx] : NULL; \
} \
int DmVec_##T##_Size(const DmVec_##T* v) { return v->size; } \
int DmVec_##T##_Erase(DmVec_##T* v, int idx) { \
    if (idx < 0 || idx >= v->size) return -1; \
    DmContainerMemmoveHelper(&v->data[idx], (v->size - idx) * sizeof(T), \
    &v->data[idx+1], (v->size - idx - 1) * sizeof(T)); v->size--; return 0; \
}

#define DM_VEC_IMPL_ND_3(T) \
bool DmVec_##T##_Empty(const DmVec_##T* v) { return v->size == 0; }

#define DM_VEC_IMPL_NO_DESTROY(T) \
DM_VEC_IMPL_ND_1(T) \
DM_VEC_IMPL_ND_2(T) \
DM_VEC_IMPL_ND_3(T)

DM_VEC_DEFINE(int);
DM_VEC_DEFINE(int32_t);
DM_VEC_DEFINE(int64_t);
DM_VEC_DEFINE(uint8_t);
DM_VEC_DEFINE(uint32_t);
DM_VEC_DEFINE(DmString);
DM_VEC_DEFINE(DmCharPtr);
DM_VEC_DEFINE(DmVoidPtr);

typedef DmVec_int64_t DmVecInt64_t;

#define DmVecInt64_t_Init(v)       DmVec_int64_t_Init(v)
#define DmVecInt64_t_Push(v, i)    DmVec_int64_t_Push(v, i)
#define DmVecInt64_t_At(v, idx)    DmVec_int64_t_At(v, idx)
#define DmVecInt64_t_Size(v)       DmVec_int64_t_Size(v)
#define DmVecInt64_t_Clear(v)      DmVec_int64_t_Clear(v)
#define DmVecInt64_t_Destroy(v)    DmVec_int64_t_Destroy(v)
#define DmVecInt64_t_Erase(v, i)   DmVec_int64_t_Erase(v, i)
#define DmVecInt64_t_Empty(v)      DmVec_int64_t_Empty(v)

#define DM_MAP_DEFINE(NAME, K, V) \
typedef struct { __typeof__(K) key; __typeof__(V) val; } DmMapEntry_##NAME; \
typedef struct { DmMapEntry_##NAME* data; int size; int cap; } DmMap_##NAME; \
int DmMap_##NAME##_Init(DmMap_##NAME* m); \
int DmMap_##NAME##_Insert(DmMap_##NAME* m, __typeof__(K) key, __typeof__(V) val); \
__typeof__(V)* DmMap_##NAME##_Find(DmMap_##NAME* m, __typeof__(K) key); \
int DmMap_##NAME##_Size(const DmMap_##NAME* m); \
void DmMap_##NAME##_Erase(DmMap_##NAME* m, __typeof__(K) key); \
void DmMap_##NAME##_Clear(DmMap_##NAME* m); \
void DmMap_##NAME##_Destroy(DmMap_##NAME* m)

#define DM_MAP_IMPL_1(NAME, K, V, CMP) \
int DmMap_##NAME##_Init(DmMap_##NAME* m) { m->data = NULL; m->size = 0; m->cap = 0; return 0; } \
int DmMap_##NAME##_Insert(DmMap_##NAME* m, __typeof__(K) key, __typeof__(V) val) { \
    for (int i = 0; i < m->size; i++) { \
        if ((CMP)(key, m->data[i].key) == 0) { m->data[i].val = val; return 0; } \
    } if (m->size >= m->cap) { \
        int nc = m->cap == 0 ? 8 : m->cap * 2; \
        DmMapEntry_##NAME* nd = (DmMapEntry_##NAME*)realloc(m->data, nc * sizeof(DmMapEntry_##NAME)); \
        if (!nd) return -1; m->data = nd; m->cap = nc; \
    } m->data[m->size].key = key; m->data[m->size].val = val; m->size++; return 0; }

#define DM_MAP_IMPL_2(NAME, K, V, CMP) \
__typeof__(V)* DmMap_##NAME##_Find(DmMap_##NAME* m, __typeof__(K) key) { \
    for (int i = 0; i < m->size; i++) { \
        if ((CMP)(key, m->data[i].key) == 0) return &m->data[i].val; \
    } \
    return NULL; \
} \
int DmMap_##NAME##_Size(const DmMap_##NAME* m) { return m->size; }

#define DM_MAP_IMPL_3(NAME, K, V, CMP) \
void DmMap_##NAME##_Erase(DmMap_##NAME* m, __typeof__(K) key) { \
    for (int i = 0; i < m->size; i++) { \
        if ((CMP)(key, m->data[i].key) == 0) { \
            DmContainerMemmoveHelper(&m->data[i], (m->size - i) * sizeof(DmMapEntry_##NAME), \
            &m->data[i+1], (m->size - i - 1) * sizeof(DmMapEntry_##NAME)); \
            m->size--; return; \
        } \
    } \
}

#define DM_MAP_IMPL(NAME, K, V, CMP) \
DM_MAP_IMPL_1(NAME, K, V, CMP) \
DM_MAP_IMPL_2(NAME, K, V, CMP) \
DM_MAP_IMPL_3(NAME, K, V, CMP)

DM_MAP_DEFINE(int_int, int, int);
DM_MAP_DEFINE(DmString_DmString, DmString, DmString);
DM_MAP_DEFINE(int_DmString, int, DmString);
DM_MAP_DEFINE(DmString_DmVec_DmString, DmString, DmVec_DmString);
DM_MAP_DEFINE(uint64_t_DmVoidPtr, uint64_t, DmVoidPtr);
DM_MAP_DEFINE(uint64_t_uint64_t, uint64_t, uint64_t);
DM_MAP_DEFINE(uint64_t_int, uint64_t, int);
DM_MAP_DEFINE(int_DmVoidPtr, int, DmVoidPtr);

#define DM_HMAP_DEFINE(NAME, K, V) \
typedef struct { __typeof__(K) key; __typeof__(V) val; unsigned int hash; int state; } DmHmapEntry_##NAME; \
typedef struct { DmHmapEntry_##NAME* data; int size; int cap; } DmHmap_##NAME; \
int DmHmap_##NAME##_Init(DmHmap_##NAME* m); \
int DmHmap_##NAME##_Insert(DmHmap_##NAME* m, __typeof__(K) key, __typeof__(V) val); \
__typeof__(V)* DmHmap_##NAME##_Find(DmHmap_##NAME* m, __typeof__(K) key); \
int DmHmap_##NAME##_Size(const DmHmap_##NAME* m); \
void DmHmap_##NAME##_Erase(DmHmap_##NAME* m, __typeof__(K) key); \
void DmHmap_##NAME##_Clear(DmHmap_##NAME* m); \
void DmHmap_##NAME##_Destroy(DmHmap_##NAME* m)

#define DM_HMAP_IMPL_INIT(NAME) \
int DmHmap_##NAME##_Init(DmHmap_##NAME* m) { \
    m->cap = 16; \
    m->data = (DmHmapEntry_##NAME*)calloc(m->cap, sizeof(DmHmapEntry_##NAME)); \
    m->size = 0; \
    return m->data ? 0 : -1; \
}

#define DM_HMAP_RESIZE(NAME) \
    if (m->size * 2 >= m->cap) { \
        int nc = m->cap * 2; \
        DmHmapEntry_##NAME* nd = (DmHmapEntry_##NAME*)calloc(nc, sizeof(DmHmapEntry_##NAME)); \
        if (!nd) return -1; \
        for (int i = 0; i < m->cap; i++) { if (m->data[i].state == 1) { \
            unsigned int h = m->data[i].hash % nc; \
            while (nd[h].state == 1) { h = (h + 1) % nc; } nd[h] = m->data[i]; } } \
        free(m->data); m->data = nd; m->cap = nc; \
    }

#define DM_HMAP_IMPL_INSERT(NAME, K, V, HASH, CMP) \
int DmHmap_##NAME##_Insert(DmHmap_##NAME* m, __typeof__(K) key, __typeof__(V) val) { \
    unsigned int hval = (HASH)(key); DM_HMAP_RESIZE(NAME) \
    unsigned int h = hval % m->cap; int fd = -1; \
    while (m->data[h].state != 0) { \
        if (m->data[h].state == 1 && (CMP)(m->data[h].key, key) == 0) { m->data[h].val = val; return 0; } \
        if (m->data[h].state == 2 && fd == -1) fd = (int)h; h = (h + 1) % m->cap; \
    } int slot = (fd != -1) ? fd : (int)h; m->data[slot].key = key; m->data[slot].val = val; \
    m->data[slot].hash = hval; m->data[slot].state = 1; m->size++; return 0; \
}

#define DM_HMAP_IMPL_FIND(NAME, K, V, HASH, CMP) \
__typeof__(V)* DmHmap_##NAME##_Find(DmHmap_##NAME* m, __typeof__(K) key) { \
    if (m->cap == 0 || m->size == 0) return NULL; \
    unsigned int h = (HASH)(key) % m->cap; \
    while (m->data[h].state != 0) { \
        if (m->data[h].state == 1 && (CMP)(m->data[h].key, key) == 0) return &m->data[h].val; \
        h = (h + 1) % m->cap; \
    } \
    return NULL; \
}

#define DM_HMAP_IMPL_ERASE(NAME, K, V, HASH, CMP) \
int DmHmap_##NAME##_Size(const DmHmap_##NAME* m) { return m->size; } \
void DmHmap_##NAME##_Erase(DmHmap_##NAME* m, __typeof__(K) key) { \
    if (m->cap == 0 || m->size == 0) return; \
    unsigned int h = (HASH)(key) % m->cap; \
    while (m->data[h].state != 0) { \
        if (m->data[h].state == 1 && (CMP)(m->data[h].key, key) == 0) { m->data[h].state = 2; m->size--; return; } \
        h = (h + 1) % m->cap; \
    } \
}

#define DM_HMAP_IMPL(NAME, K, V, HASH, CMP) \
DM_HMAP_IMPL_INIT(NAME) \
DM_HMAP_IMPL_INSERT(NAME, K, V, HASH, CMP) \
DM_HMAP_IMPL_FIND(NAME, K, V, HASH, CMP) \
DM_HMAP_IMPL_ERASE(NAME, K, V, HASH, CMP)

DM_HMAP_DEFINE(DmString_DmString, DmString, DmString);
DM_HMAP_DEFINE(int_DmVoidPtr, int, DmVoidPtr);


typedef struct { void** data; int size; int cap; } DmVecVoid;

int DmVecVoidInit(DmVecVoid* v);
int DmVecVoidPush(DmVecVoid* v, void* item);
void** DmVecVoidAt(DmVecVoid* v, int idx);
int DmVecVoidSize(DmVecVoid* v);
void DmVecVoidClear(DmVecVoid* v);
void DmVecVoidDestroy(DmVecVoid* v);

#define DM_SET_DEFINE(T) \
typedef struct { __typeof__(T)* data; int size; int cap; } DmSet_##T; \
int DmSet_##T##_Init(DmSet_##T* s); \
int DmSet_##T##_Insert(DmSet_##T* s, __typeof__(T) item); \
bool DmSet_##T##_Contains(DmSet_##T* s, __typeof__(T) item); \
int DmSet_##T##_Size(DmSet_##T* s); \
void DmSet_##T##_Erase(DmSet_##T* s, __typeof__(T) item); \
void DmSet_##T##_Clear(DmSet_##T* s); \
void DmSet_##T##_Destroy(DmSet_##T* s)

#define DM_SET_IMPL_1(T, CMP) \
int DmSet_##T##_Init(DmSet_##T* s) { s->data = NULL; s->size = 0; s->cap = 0; return 0; } \
int DmSet_##T##_Insert(DmSet_##T* s, __typeof__(T) item) { \
    if (DmSet_##T##_Contains(s, item)) return 0; \
    if (s->size >= s->cap) { \
        int nc = s->cap == 0 ? 8 : s->cap * 2; \
        __typeof__(T)* nd = (T*)realloc(s->data, nc * sizeof(T)); \
        if (!nd) return -1; s->data = nd; s->cap = nc; \
    } s->data[s->size++] = item; return 0; \
}

#define DM_SET_IMPL_2(T, CMP) \
bool DmSet_##T##_Contains(DmSet_##T* s, __typeof__(T) item) { \
    for (int i = 0; i < s->size; i++) { \
        if ((CMP)(s->data[i], item) == 0) return true; \
    } \
    return false; \
} \
int DmSet_##T##_Size(DmSet_##T* s) { return s->size; }

#define DM_SET_IMPL_3(T, CMP) \
void DmSet_##T##_Erase(DmSet_##T* s, __typeof__(T) item) { \
    for (int i = 0; i < s->size; i++) { \
        if ((CMP)(s->data[i], item) == 0) { \
            DmContainerMemmoveHelper(&s->data[i], (s->size - i) * sizeof(T), \
            &s->data[i+1], (s->size - i - 1) * sizeof(T)); \
            s->size--; return; \
        } \
    } \
}

#define DM_SET_IMPL(T, CMP) \
DM_SET_IMPL_1(T, CMP) \
DM_SET_IMPL_2(T, CMP) \
DM_SET_IMPL_3(T, CMP)

typedef struct { DmString* data; int size; int cap; } DmSetDmString;
int DmSetDmStringInit(DmSetDmString* s);
int DmSetDmStringInsert(DmSetDmString* s, const DmString* item);
bool DmSetDmStringContains(DmSetDmString* s, const DmString* item);
int DmSetDmStringSize(DmSetDmString* s);
void DmSetDmStringErase(DmSetDmString* s, const DmString* item);
void DmSetDmStringClear(DmSetDmString* s);
void DmSetDmStringDestroy(DmSetDmString* s);

DM_SET_DEFINE(int);
DM_SET_DEFINE(uint16_t);
DM_SET_DEFINE(uint32_t);
DM_SET_DEFINE(uint64_t);
DM_SET_DEFINE(int32_t);

DM_HMAP_DEFINE(DmString_uint16_t, DmString, uint16_t);
DM_HMAP_DEFINE(DmString_DmVoidPtr, DmString, DmVoidPtr);
DM_MAP_DEFINE(DmString_int, DmString, int);
DM_MAP_DEFINE(DmString_uint16_t, DmString, uint16_t);
DM_MAP_DEFINE(int_DmSetDmString, int, DmSetDmString);
DM_MAP_DEFINE(uint16_uint16, uint16_t, uint16_t);
DM_MAP_DEFINE(DmString_DmMap_int_int, DmString, DmMap_int_int);
DM_MAP_DEFINE(DmString_DmMap_uint16_uint16, DmString, DmMap_uint16_uint16);
DM_MAP_DEFINE(DmString_DmSet_int, DmString, DmSet_int);


#define DM_QUEUE_DEFINE(T) \
typedef struct { __typeof__(T)* data; int head; int tail; int size; int cap; } DmQueue_##T; \
int DmQueue_##T##_Init(DmQueue_##T* q, int cap); \
int DmQueue_##T##_Push(DmQueue_##T* q, __typeof__(T) item); \
bool DmQueue_##T##_Pop(DmQueue_##T* q, __typeof__(T)* out); \
int DmQueue_##T##_Size(DmQueue_##T* q); \
bool DmQueue_##T##_Empty(DmQueue_##T* q); \
void DmQueue_##T##_Destroy(DmQueue_##T* q)

#define DM_QUEUE_IMPL_1(T) \
int DmQueue_##T##_Init(DmQueue_##T* q, int cap) { \
    q->data = (T*)malloc(cap * sizeof(T)); if (!q->data) return -1; \
    q->head = 0; q->tail = 0; q->size = 0; q->cap = cap; return 0; \
} \
int DmQueue_##T##_Push(DmQueue_##T* q, __typeof__(T) item) { \
    if (q->size >= q->cap) return -1; \
    q->data[q->tail] = item; q->tail = (q->tail + 1) % q->cap; q->size++; return 0; \
}

#define DM_QUEUE_IMPL_2(T) \
bool DmQueue_##T##_Pop(DmQueue_##T* q, __typeof__(T)* out) { \
    if (q->size == 0) return false; \
    *out = q->data[q->head]; \
    q->head = (q->head + 1) % q->cap; \
    q->size--; return true; \
} \
int DmQueue_##T##_Size(DmQueue_##T* q) { return q->size; } \
bool DmQueue_##T##_Empty(DmQueue_##T* q) { return q->size == 0; }

#define DM_QUEUE_IMPL(T) \
DM_QUEUE_IMPL_1(T) \
DM_QUEUE_IMPL_2(T)

DM_QUEUE_DEFINE(int);
DM_QUEUE_DEFINE(DmString);
DM_QUEUE_DEFINE(DmVoidPtr);

#define DM_PAIR_DEFINE(NAME, K, V) \
typedef struct { __typeof__(K) first; __typeof__(V) second; } DmPair_##NAME

DM_PAIR_DEFINE(DmString_DmString, DmString, DmString);
DM_PAIR_DEFINE(int_int, int, int);
DM_PAIR_DEFINE(int_DmVoidPtr, int, DmVoidPtr);
DM_PAIR_DEFINE(int64_t_int64_t, int64_t, int64_t);

DM_VEC_DEFINE(DmPair_int64_t_int64_t);

#define DM_MMAP_DEFINE(NAME, K, V) \
typedef struct { __typeof__(K) key; __typeof__(V) val; } DmMmapEntry_##NAME; \
typedef struct { DmMmapEntry_##NAME* data; int size; int cap; } DmMmap_##NAME; \
int DmMmap_##NAME##_Init(DmMmap_##NAME* m); \
int DmMmap_##NAME##_Insert(DmMmap_##NAME* m, __typeof__(K) key, __typeof__(V) val); \
int DmMmap_##NAME##_Size(DmMmap_##NAME* m); \
void DmMmap_##NAME##_Clear(DmMmap_##NAME* m); \
void DmMmap_##NAME##_Destroy(DmMmap_##NAME* m); \
DmVec_##V DmMmap_##NAME##_FindAll(DmMmap_##NAME* m, __typeof__(K) key)

#define DM_MMAP_IMPL_1(NAME, K, V, CMP) \
int DmMmap_##NAME##_Init(DmMmap_##NAME* m) { m->data = NULL; m->size = 0; m->cap = 0; return 0; } \
int DmMmap_##NAME##_Insert(DmMmap_##NAME* m, __typeof__(K) key, __typeof__(V) val) { \
    if (m->size >= m->cap) { \
        int nc = m->cap == 0 ? 8 : m->cap * 2; \
        DmMmapEntry_##NAME* nd = (DmMmapEntry_##NAME*)realloc(m->data, nc * sizeof(DmMmapEntry_##NAME)); \
        if (!nd) return -1; m->data = nd; m->cap = nc; \
    } m->data[m->size].key = key; m->data[m->size].val = val; m->size++; return 0; \
}

#define DM_MMAP_IMPL_2(NAME, K, V, CMP) \
int DmMmap_##NAME##_Size(DmMmap_##NAME* m) { return m->size; } \
DmVec_##V DmMmap_##NAME##_FindAll(DmMmap_##NAME* m, __typeof__(K) key) { \
    DmVec_##V result; DmVec_##V##_Init(&result); \
    for (int i = 0; i < m->size; i++) { \
        if ((CMP)(m->data[i].key, key) == 0) { DmVec_##V##_Push(&result, m->data[i].val); } \
    } \
    return result; \
}

#define DM_MMAP_IMPL(NAME, K, V, CMP) \
DM_MMAP_IMPL_1(NAME, K, V, CMP) \
DM_MMAP_IMPL_2(NAME, K, V, CMP)

DM_MMAP_DEFINE(DmString_DmString, DmString, DmString);

DM_MMAP_DEFINE(DmString_int_mmap, DmString, int);

#ifdef __cplusplus
}
#endif

#endif
