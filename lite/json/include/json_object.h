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


#ifndef DM_JSON_OBJECT_H
#define DM_JSON_OBJECT_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_container.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

typedef enum DmJsonCreateType {
    DM_JSON_CREATE_TYPE_OBJECT = 0,
    DM_JSON_CREATE_TYPE_ARRAY = 1
} DmJsonCreateType;

typedef struct DmJsonItemObject {
    void* item;
    void* parent;
    int32_t itemIndex;
    bool needDeleteItem;
    bool beValid;
    DmString itemName;
} DmJsonItemObject;

DM_VEC_DEFINE(DmJsonItemObject);

typedef struct DmJsonObject {
    DmJsonItemObject base;
    DmJsonCreateType createType;
} DmJsonObject;

DM_EXPORT void DmJsonItemObjectInit(DmJsonItemObject* obj);
DM_EXPORT void DmJsonItemObjectDestroy(DmJsonItemObject* obj);
DM_EXPORT bool DmJsonItemObjectIsString(const DmJsonItemObject* obj);
DM_EXPORT bool DmJsonItemObjectIsNumber(const DmJsonItemObject* obj);
DM_EXPORT bool DmJsonItemObjectIsNumberInteger(const DmJsonItemObject* obj);
DM_EXPORT bool DmJsonItemObjectIsArray(const DmJsonItemObject* obj);
DM_EXPORT bool DmJsonItemObjectIsBoolean(const DmJsonItemObject* obj);
DM_EXPORT bool DmJsonItemObjectIsObject(const DmJsonItemObject* obj);
DM_EXPORT bool DmJsonItemObjectIsDiscarded(const DmJsonItemObject* obj);
DM_EXPORT bool DmJsonItemObjectContains(const DmJsonItemObject* obj, const char* key);
DM_EXPORT DmString DmJsonItemObjectDumpFormatted(const DmJsonItemObject* obj);
DM_EXPORT DmString DmJsonItemObjectDump(const DmJsonItemObject* obj);
DM_EXPORT bool DmJsonItemObjectPushBackString(DmJsonItemObject* obj, const char* strValue);
DM_EXPORT bool DmJsonItemObjectPushBackDouble(DmJsonItemObject* obj, double value);
DM_EXPORT bool DmJsonItemObjectPushBackInt64(DmJsonItemObject* obj, int64_t value);
DM_EXPORT bool DmJsonItemObjectPushBackItem(DmJsonItemObject* obj, const DmJsonItemObject* item);
DM_EXPORT DmString DmJsonItemObjectKey(const DmJsonItemObject* obj);
DM_EXPORT void DmJsonItemObjectInsert(DmJsonItemObject* obj, const char* key, const DmJsonItemObject* itemObj);
DM_EXPORT DmJsonItemObject DmJsonItemObjectAt(const DmJsonItemObject* obj, const char* key);
DM_EXPORT DmJsonItemObject DmJsonItemObjectAdd(DmJsonItemObject* obj, const char* key);
DM_EXPORT void DmJsonItemObjectGetToString(const DmJsonItemObject* obj, DmString* strValue);
DM_EXPORT void DmJsonItemObjectGetToDouble(const DmJsonItemObject* obj, double* value);
DM_EXPORT void DmJsonItemObjectGetToInt32(const DmJsonItemObject* obj, int32_t* value);
DM_EXPORT void DmJsonItemObjectGetToUint32(const DmJsonItemObject* obj, uint32_t* value);
DM_EXPORT void DmJsonItemObjectGetToInt64(const DmJsonItemObject* obj, int64_t* value);
DM_EXPORT void DmJsonItemObjectGetToBool(const DmJsonItemObject* obj, bool* value);
DM_EXPORT void DmJsonItemObjectGetToUint64(const DmJsonItemObject* obj, uint64_t* value);
DM_EXPORT DmVec_DmJsonItemObject DmJsonItemObjectItems(const DmJsonItemObject* obj);
DM_EXPORT void DmJsonItemObjectErase(DmJsonItemObject* obj, const char* key);

DM_EXPORT void DmJsonItemObjectSetKey(DmJsonItemObject* obj, const char* key, const char* value);
DM_EXPORT void DmJsonItemObjectSetKeyInt(DmJsonItemObject* obj, const char* key, int32_t value);
DM_EXPORT void DmJsonItemObjectSetKeyInt64(DmJsonItemObject* obj, const char* key, int64_t value);
DM_EXPORT void DmJsonItemObjectSetKeyUint32(DmJsonItemObject* obj, const char* key, uint32_t value);
DM_EXPORT void DmJsonItemObjectSetKeyBool(DmJsonItemObject* obj, const char* key, bool value);

DM_EXPORT bool DmJsonItemObjectIsBool(const DmJsonItemObject* obj, const char* key);
DM_EXPORT bool DmJsonItemObjectGetBool(const DmJsonItemObject* obj, const char* key);
DM_EXPORT bool DmJsonItemObjectIsInt32(const DmJsonItemObject* obj, const char* key);
DM_EXPORT int32_t DmJsonItemObjectGetInt32(const DmJsonItemObject* obj, const char* key);
DM_EXPORT bool DmJsonItemObjectContainsAt(const DmJsonItemObject* obj, const char* parentKey, const char* childKey);
DM_EXPORT bool DmJsonItemObjectIsInt32At(const DmJsonItemObject* obj, const char* parentKey, const char* childKey);
DM_EXPORT int32_t DmJsonItemObjectGetInt32At(const DmJsonItemObject* obj, const char* parentKey, const char* childKey);
DM_EXPORT void DmJsonItemObjectParse(DmJsonItemObject* obj, const char* strJson);
DM_EXPORT DmString DmJsonItemObjectDumpArrayString(const DmJsonItemObject* obj);
DM_EXPORT int DmJsonItemObjectArraySize(const DmJsonItemObject* obj, const char* key);
DM_EXPORT int64_t DmJsonItemObjectGetInt64FromArray(const DmJsonItemObject* obj, const char* key, int index);
DM_EXPORT void DmJsonItemObjectPutInt64Array(DmJsonItemObject* obj, const char* key, DmVecInt64_t* arr);
DM_EXPORT void DmJsonItemObjectPutStringArray(DmJsonItemObject* obj, const char* key, DmVec_DmString* arr);
DM_EXPORT void DmJsonItemObjectGetInt64Array(const DmJsonItemObject* obj, const char* key, DmVecInt64_t* arr);
DM_EXPORT bool DmJsonItemObjectIsUint64(const DmJsonItemObject* obj, const char* key);
DM_EXPORT uint64_t DmJsonItemObjectGetUint64(const DmJsonItemObject* obj, const char* key);

DM_EXPORT DmString DmJsonGetString(const DmJsonItemObject* obj);
DM_EXPORT int32_t DmJsonGetInt(const DmJsonItemObject* obj);
DM_EXPORT int64_t DmJsonGetInt64(const DmJsonItemObject* obj);
DM_EXPORT uint64_t DmJsonGetUint64(const DmJsonItemObject* obj);
DM_EXPORT bool DmJsonGetBool(const DmJsonItemObject* obj);
DM_EXPORT double DmJsonGetDouble(const DmJsonItemObject* obj);
DM_EXPORT DmJsonItemObject DmJsonGetObject(const DmJsonItemObject* obj, const char* key);

DM_EXPORT void DmJsonToJsonString(DmJsonItemObject* obj, const char* value);
DM_EXPORT void DmJsonToJsonDouble(DmJsonItemObject* obj, double value);
DM_EXPORT void DmJsonToJsonBool(DmJsonItemObject* obj, bool value);
DM_EXPORT void DmJsonToJsonInt32(DmJsonItemObject* obj, int32_t value);
DM_EXPORT void DmJsonToJsonUint32(DmJsonItemObject* obj, uint32_t value);
DM_EXPORT void DmJsonToJsonInt64(DmJsonItemObject* obj, int64_t value);
DM_EXPORT void DmJsonToJsonUint64(DmJsonItemObject* obj, uint64_t value);

DM_EXPORT DmString DmJsonToString(const DmJsonItemObject* jsonItem);

DM_EXPORT void DmJsonObjectInit(DmJsonObject* obj, DmJsonCreateType type);
DM_EXPORT void DmJsonObjectInitFromStr(DmJsonObject* obj, const char* strJson);
DM_EXPORT void DmJsonObjectDestroy(DmJsonObject* obj);
DM_EXPORT bool DmJsonObjectParse(DmJsonObject* obj, const char* strJson);
DM_EXPORT void DmJsonObjectDuplicate(DmJsonObject* obj, const DmJsonObject* source);
DM_EXPORT DmJsonItemObject* DmJsonObjectGetItem(DmJsonObject* obj);
DM_EXPORT DmJsonItemObject* DmJsonObjectGetByKey(DmJsonObject* obj, const char* key);

#ifdef __cplusplus
}
#endif

#endif
