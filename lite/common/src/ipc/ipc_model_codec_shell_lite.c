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


#include "ipc_model_codec.h"
#include "dm_container.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "securec.h"

#define DM_LITE_PARCEL_INIT_CAP 64
#define DM_LITE_PARCEL_GROW_FACTOR 2

typedef struct {
    uint8_t* data;
    int size;
    int cap;
    int readPos;
} DmLiteParcel;

static bool LiteParcelGrow(DmLiteParcel* p, int needed)
{
    int newCap = p->cap == 0 ? DM_LITE_PARCEL_INIT_CAP : p->cap;
    while (newCap < p->size + needed) {
        newCap *= DM_LITE_PARCEL_GROW_FACTOR;
    }
    uint8_t* nd = (uint8_t*)malloc(newCap);
    if (nd) {
        if (p->data && p->size > 0) {
            if (memcpy_s(nd, newCap, p->data, p->size) != 0) {
                free(nd);
                return false;
            }
            free(p->data);
        }
    }
    if (!nd) {
        return false;
    }
    p->data = nd;
    p->cap = newCap;
    return true;
}

static bool LiteParcelWriteBuf(DmLiteParcel* p, const uint8_t* buf, int len)
{
    if (!LiteParcelGrow(p, len)) {
        return false;
    }
    if (memcpy_s(p->data + p->size, p->cap - p->size, buf, len) != 0) {
        return false;
    }
    p->size += len;
    return true;
}

static bool LiteWriteString(void* parcel, const char* str)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (!str) {
        str = "";
    }
    int32_t len = (int32_t)strlen(str);
    if (!LiteParcelWriteBuf(p, (const uint8_t*)&len, sizeof(int32_t))) {
        return false;
    }
    if (!LiteParcelWriteBuf(p, (const uint8_t*)str, len)) {
        return false;
    }
    uint8_t zero = 0;
    if (!LiteParcelWriteBuf(p, &zero, 1)) {
        return false;
    }
    return true;
}

static bool LiteWriteInt32(void* parcel, int32_t val)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    return LiteParcelWriteBuf(p, (const uint8_t*)&val, sizeof(int32_t));
}

static bool LiteWriteInt64(void* parcel, int64_t val)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    return LiteParcelWriteBuf(p, (const uint8_t*)&val, sizeof(int64_t));
}

static bool LiteWriteUint16(void* parcel, uint16_t val)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    return LiteParcelWriteBuf(p, (const uint8_t*)&val, sizeof(uint16_t));
}

static bool LiteWriteUint32(void* parcel, uint32_t val)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    return LiteParcelWriteBuf(p, (const uint8_t*)&val, sizeof(uint32_t));
}

static bool LiteWriteUint64(void* parcel, uint64_t val)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    return LiteParcelWriteBuf(p, (const uint8_t*)&val, sizeof(uint64_t));
}

static bool LiteWriteBool(void* parcel, bool val)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    uint8_t b = val ? 1 : 0;
    return LiteParcelWriteBuf(p, &b, 1);
}

static bool LiteWriteRawData(void* parcel, const uint8_t* data, int32_t len)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (!LiteParcelWriteBuf(p, (const uint8_t*)&len, sizeof(int32_t))) {
        return false;
    }
    if (len > 0 && data) {
        if (!LiteParcelWriteBuf(p, data, len)) {
            return false;
        }
    }
    return true;
}

static bool LiteReadString(void* parcel, DmString* out)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (p->readPos + sizeof(int32_t) > p->size) {
        return false;
    }
    int32_t len;
    if (memcpy_s(&len, sizeof(int32_t), p->data + p->readPos, sizeof(int32_t)) != 0) {
        return false;
    }
    p->readPos += sizeof(int32_t);
    if (p->readPos + len + 1 > p->size) {
        return false;
    }
    *out = DmStringCreateLen((const char*)(p->data + p->readPos), len);
    p->readPos += len + 1;
    return true;
}

static int32_t LiteReadInt32(void* parcel)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (p->readPos + sizeof(int32_t) > p->size) {
        return 0;
    }
    int32_t val;
    if (memcpy_s(&val, sizeof(int32_t), p->data + p->readPos, sizeof(int32_t)) != 0) {
        return 0;
    }
    p->readPos += sizeof(int32_t);
    return val;
}

static int64_t LiteReadInt64(void* parcel)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (p->readPos + sizeof(int64_t) > p->size) {
        return 0;
    }
    int64_t val;
    if (memcpy_s(&val, sizeof(int64_t), p->data + p->readPos, sizeof(int64_t)) != 0) {
        return 0;
    }
    p->readPos += sizeof(int64_t);
    return val;
}

static uint16_t LiteReadUint16(void* parcel)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (p->readPos + sizeof(uint16_t) > p->size) {
        return 0;
    }
    uint16_t val;
    if (memcpy_s(&val, sizeof(uint16_t), p->data + p->readPos, sizeof(uint16_t)) != 0) {
        return 0;
    }
    p->readPos += sizeof(uint16_t);
    return val;
}

static uint32_t LiteReadUint32(void* parcel)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (p->readPos + sizeof(uint32_t) > p->size) {
        return 0;
    }
    uint32_t val;
    if (memcpy_s(&val, sizeof(uint32_t), p->data + p->readPos, sizeof(uint32_t)) != 0) {
        return 0;
    }
    p->readPos += sizeof(uint32_t);
    return val;
}

static uint64_t LiteReadUint64(void* parcel)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (p->readPos + sizeof(uint64_t) > p->size) {
        return 0;
    }
    uint64_t val;
    if (memcpy_s(&val, sizeof(uint64_t), p->data + p->readPos, sizeof(uint64_t)) != 0) {
        return 0;
    }
    p->readPos += sizeof(uint64_t);
    return val;
}

static bool LiteReadBool(void* parcel)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (p->readPos + 1 > p->size) {
        return false;
    }
    uint8_t b = p->data[p->readPos];
    p->readPos += 1;
    return b != 0;
}

static const uint8_t* LiteReadRawData(void* parcel, int32_t len)
{
    DmLiteParcel* p = (DmLiteParcel*)parcel;
    if (p->readPos + sizeof(int32_t) > p->size) {
        return NULL;
    }
    int32_t storedLen;
    if (memcpy_s(&storedLen, sizeof(int32_t), p->data + p->readPos, sizeof(int32_t)) != 0) {
        return NULL;
    }
    p->readPos += sizeof(int32_t);
    if (storedLen != len || p->readPos + len > p->size) {
        return NULL;
    }
    const uint8_t* ptr = p->data + p->readPos;
    p->readPos += len;
    return ptr;
}

DmParcelWriteOps* DmParcelWriteOpsCreate(void)
{
    DmParcelWriteOps* ops = (DmParcelWriteOps*)malloc(sizeof(DmParcelWriteOps));
    if (!ops) {
        return NULL;
    }
    ops->write_string = LiteWriteString;
    ops->write_int32 = LiteWriteInt32;
    ops->write_int64 = LiteWriteInt64;
    ops->write_uint16 = LiteWriteUint16;
    ops->write_uint32 = LiteWriteUint32;
    ops->write_uint64 = LiteWriteUint64;
    ops->write_bool = LiteWriteBool;
    ops->write_raw_data = LiteWriteRawData;
    return ops;
}

void DmParcelWriteOpsDestroy(DmParcelWriteOps* ops)
{
    free(ops);
}

DmParcelReadOps* DmParcelReadOpsCreate(void)
{
    DmParcelReadOps* ops = (DmParcelReadOps*)malloc(sizeof(DmParcelReadOps));
    if (!ops) {
        return NULL;
    }
    ops->read_string = LiteReadString;
    ops->read_int32 = LiteReadInt32;
    ops->read_int64 = LiteReadInt64;
    ops->read_uint16 = LiteReadUint16;
    ops->read_uint32 = LiteReadUint32;
    ops->read_uint64 = LiteReadUint64;
    ops->read_bool = LiteReadBool;
    ops->read_raw_data = LiteReadRawData;
    return ops;
}

void DmParcelReadOpsDestroy(DmParcelReadOps* ops)
{
    free(ops);
}
