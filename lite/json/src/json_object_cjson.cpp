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


#include "json_object.h"

#include "cJSON.h"
#include "dm_log.h"
#include <string>
#include <cstdlib>
#include <cstring>

static const int DM_JSON_BASE_TEN = 10;

static cJSON* DmGetCjsonPointer(void* pointer)
{
    return static_cast<cJSON*>(pointer);
}

static bool DmAddToArray(cJSON* item, cJSON* newItem)
{
    if (item == nullptr) {
        LOGE("item is nullptr");
        return false;
    }
    if (!cJSON_AddItemToArray(item, newItem)) {
        LOGE("add item to array fail");
        cJSON_Delete(newItem);
        return false;
    }
    return true;
}

static cJSON* DmCreateInt64Item(int64_t value)
{
    return cJSON_CreateString(std::to_string(value).c_str());
}

static cJSON* DmCreateUint64Item(uint64_t value)
{
    return cJSON_CreateString(std::to_string(value).c_str());
}

static int64_t DmItemToInt64(const cJSON* item)
{
    if (item == nullptr) {
        return 0;
    }
    if (cJSON_IsString(item)) {
        return std::strtoll(item->valuestring, nullptr, DM_JSON_BASE_TEN);
    }
    if (cJSON_IsNumber(item)) {
        return static_cast<int64_t>(item->valuedouble);
    }
    return 0;
}

static uint64_t DmItemToUint64(const cJSON* item)
{
    if (item == nullptr) {
        return 0;
    }
    if (cJSON_IsString(item)) {
        return std::strtoull(item->valuestring, nullptr, DM_JSON_BASE_TEN);
    }
    if (cJSON_IsNumber(item)) {
        return static_cast<uint64_t>(item->valuedouble);
    }
    return 0;
}

void DmJsonToJsonString(DmJsonItemObject* obj, const char* value)
{
    if (obj->item != nullptr) {
        cJSON_Delete(DmGetCjsonPointer(obj->item));
    }
    obj->item = cJSON_CreateString(value);
}

void DmJsonToJsonDouble(DmJsonItemObject* obj, double value)
{
    if (obj->item != nullptr) {
        cJSON_Delete(DmGetCjsonPointer(obj->item));
    }
    obj->item = cJSON_CreateNumber(value);
}

void DmJsonToJsonBool(DmJsonItemObject* obj, bool value)
{
    if (obj->item != nullptr) {
        cJSON_Delete(DmGetCjsonPointer(obj->item));
    }
    obj->item = value ? cJSON_CreateTrue() : cJSON_CreateFalse();
}

void DmJsonToJsonInt32(DmJsonItemObject* obj, int32_t value)
{
    if (obj->item != nullptr) {
        cJSON_Delete(DmGetCjsonPointer(obj->item));
    }
    obj->item = cJSON_CreateNumber(static_cast<double>(value));
}

void DmJsonToJsonUint32(DmJsonItemObject* obj, uint32_t value)
{
    if (obj->item != nullptr) {
        cJSON_Delete(DmGetCjsonPointer(obj->item));
    }
    obj->item = cJSON_CreateNumber(static_cast<double>(value));
}

void DmJsonToJsonInt64(DmJsonItemObject* obj, int64_t value)
{
    if (obj->item != nullptr) {
        cJSON_Delete(DmGetCjsonPointer(obj->item));
    }
    obj->item = DmCreateInt64Item(value);
}

void DmJsonToJsonUint64(DmJsonItemObject* obj, uint64_t value)
{
    if (obj->item != nullptr) {
        cJSON_Delete(DmGetCjsonPointer(obj->item));
    }
    obj->item = DmCreateUint64Item(value);
}

void DmJsonItemObjectInit(DmJsonItemObject* obj)
{
    obj->item = nullptr;
    obj->parent = nullptr;
    obj->itemIndex = -1;
    obj->needDeleteItem = false;
    obj->beValid = false;
    obj->itemName = DmStringCreateEmpty();
}

void DmJsonItemObjectDestroy(DmJsonItemObject* obj)
{
    if (obj->needDeleteItem && obj->item != nullptr) {
        cJSON_Delete(DmGetCjsonPointer(obj->item));
    }
    obj->item = nullptr;
    DmStringDestroy(&obj->itemName);
}

bool DmJsonItemObjectIsString(const DmJsonItemObject* obj)
{
    if (obj->item == nullptr) {
        return false;
    }
    return cJSON_IsString(DmGetCjsonPointer(obj->item));
}

bool DmJsonItemObjectIsNumber(const DmJsonItemObject* obj)
{
    if (obj->item == nullptr) {
        return false;
    }
    return cJSON_IsNumber(DmGetCjsonPointer(obj->item));
}

bool DmJsonItemObjectIsNumberInteger(const DmJsonItemObject* obj)
{
    if (obj->item == nullptr) {
        return false;
    }
    if (cJSON_IsString(DmGetCjsonPointer(obj->item))) {
        return true;
    }
    if (!DmJsonItemObjectIsNumber(obj)) {
        return false;
    }
    double value = cJSON_GetNumberValue(DmGetCjsonPointer(obj->item));
    return ((value - static_cast<int64_t>(value)) == 0);
}

bool DmJsonItemObjectIsArray(const DmJsonItemObject* obj)
{
    if (obj->item == nullptr) {
        return false;
    }
    return cJSON_IsArray(DmGetCjsonPointer(obj->item));
}

bool DmJsonItemObjectIsBoolean(const DmJsonItemObject* obj)
{
    if (obj->item == nullptr) {
        return false;
    }
    return cJSON_IsBool(DmGetCjsonPointer(obj->item));
}

bool DmJsonItemObjectIsObject(const DmJsonItemObject* obj)
{
    if (obj->item == nullptr) {
        return false;
    }
    return cJSON_IsObject(DmGetCjsonPointer(obj->item));
}

bool DmJsonItemObjectIsDiscarded(const DmJsonItemObject* obj)
{
    return (obj->item == nullptr);
}

bool DmJsonItemObjectContains(const DmJsonItemObject* obj, const char* key)
{
    if (obj->item == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    cJSON* item = cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key);
    return (item != nullptr);
}

void DmJsonItemObjectInsert(DmJsonItemObject* obj, const char* key, const DmJsonItemObject* itemObj)
{
    if (obj->item == nullptr || itemObj->item == nullptr) {
        LOGE("invalid item or object item");
        return;
    }
    cJSON* newItem = cJSON_Duplicate(DmGetCjsonPointer(itemObj->item), cJSON_True);
    if (newItem == nullptr) {
        LOGE("copy item fail");
        return;
    }
    if (cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(DmGetCjsonPointer(obj->item), key);
    }
    if (!cJSON_AddItemToObject(DmGetCjsonPointer(obj->item), key, newItem)) {
        LOGE("add new item to object fail");
        cJSON_Delete(newItem);
    }
}

DmString DmJsonItemObjectDumpFormatted(const DmJsonItemObject* obj)
{
    if (obj->item == nullptr) {
        LOGE("item_ is nullptr");
        return DmStringCreateEmpty();
    }
    char* jsonString = cJSON_Print(DmGetCjsonPointer(obj->item));
    if (jsonString == nullptr) {
        return DmStringCreateEmpty();
    }
    DmString result = DmStringCreate(jsonString);
    cJSON_free(jsonString);
    return result;
}

DmString DmJsonItemObjectDump(const DmJsonItemObject* obj)
{
    if (obj->item == nullptr) {
        LOGE("item_ is nullptr");
        return DmStringCreateEmpty();
    }
    char* jsonString = cJSON_PrintUnformatted(DmGetCjsonPointer(obj->item));
    if (jsonString == nullptr) {
        return DmStringCreateEmpty();
    }
    DmString result = DmStringCreate(jsonString);
    cJSON_free(jsonString);
    return result;
}

bool DmJsonItemObjectPushBackString(DmJsonItemObject* obj, const char* strValue)
{
    if (obj->item == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (DmGetCjsonPointer(obj->item)->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON* newItem = cJSON_CreateString(strValue);
    return DmAddToArray(DmGetCjsonPointer(obj->item), newItem);
}

bool DmJsonItemObjectPushBackDouble(DmJsonItemObject* obj, double value)
{
    if (obj->item == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (DmGetCjsonPointer(obj->item)->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON* newItem = cJSON_CreateNumber(value);
    return DmAddToArray(DmGetCjsonPointer(obj->item), newItem);
}

bool DmJsonItemObjectPushBackInt64(DmJsonItemObject* obj, int64_t value)
{
    if (obj->item == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (DmGetCjsonPointer(obj->item)->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON* newItem = DmCreateInt64Item(value);
    return DmAddToArray(DmGetCjsonPointer(obj->item), newItem);
}

bool DmJsonItemObjectPushBackItem(DmJsonItemObject* obj, const DmJsonItemObject* item)
{
    if (obj->item == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (!item->beValid) {
        return false;
    }
    if (DmGetCjsonPointer(obj->item)->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON* newItem = cJSON_Duplicate(DmGetCjsonPointer(item->item), cJSON_True);
    return DmAddToArray(DmGetCjsonPointer(obj->item), newItem);
}

DmString DmJsonItemObjectKey(const DmJsonItemObject* obj)
{
    if (obj->item != nullptr) {
        cJSON* citem = DmGetCjsonPointer(obj->item);
        if (citem->string != nullptr) {
            return DmStringCreate(citem->string);
        }
    }
    return DmStringCopy(&obj->itemName);
}

DmJsonItemObject DmJsonItemObjectAt(const DmJsonItemObject* obj, const char* key)
{
    DmJsonItemObject operationItem;
    DmJsonItemObjectInit(&operationItem);
    if (obj->item == nullptr) {
        return operationItem;
    }
    operationItem.item = cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key);
    if (operationItem.item != nullptr) {
        operationItem.beValid = true;
    }
    operationItem.parent = obj->item;
    operationItem.itemName = DmStringCreate(key);
    return operationItem;
}

DmJsonItemObject DmJsonItemObjectAdd(DmJsonItemObject* obj, const char* key)
{
    DmJsonItemObject itemObject = DmJsonItemObjectAt(obj, key);
    if (itemObject.item == nullptr) {
        itemObject.item = cJSON_CreateNull();
        if (itemObject.item == nullptr) {
            return itemObject;
        }
        if (!cJSON_AddItemToObject(DmGetCjsonPointer(obj->item), key, DmGetCjsonPointer(itemObject.item))) {
            LOGE("add item to object fail");
            cJSON_Delete(DmGetCjsonPointer(itemObject.item));
            itemObject.item = nullptr;
        } else {
            itemObject.beValid = true;
        }
    }
    return itemObject;
}

void DmJsonItemObjectGetToString(const DmJsonItemObject* obj, DmString* strValue)
{
    *strValue = DmStringCreateEmpty();
    if (obj->item == nullptr) {
        return;
    }
    if (!DmJsonItemObjectIsString(obj)) {
        return;
    }
    const char* strVal = cJSON_GetStringValue(DmGetCjsonPointer(obj->item));
    if (strVal == nullptr) {
        return;
    }
    *strValue = DmStringCreate(strVal);
}

void DmJsonItemObjectGetToDouble(const DmJsonItemObject* obj, double* value)
{
    *value = 0.0;
    if (obj->item == nullptr) {
        return;
    }
    if (!DmJsonItemObjectIsNumber(obj)) {
        return;
    }
    *value = cJSON_GetNumberValue(DmGetCjsonPointer(obj->item));
}

void DmJsonItemObjectGetToInt32(const DmJsonItemObject* obj, int32_t* value)
{
    int64_t tmpValue = 0;
    DmJsonItemObjectGetToInt64(obj, &tmpValue);
    *value = static_cast<int32_t>(tmpValue);
}

void DmJsonItemObjectGetToUint32(const DmJsonItemObject* obj, uint32_t* value)
{
    int64_t tmpValue = 0;
    DmJsonItemObjectGetToInt64(obj, &tmpValue);
    *value = static_cast<uint32_t>(tmpValue);
}

void DmJsonItemObjectGetToInt64(const DmJsonItemObject* obj, int64_t* value)
{
    *value = 0;
    if (obj->item == nullptr) {
        return;
    }
    if (!DmJsonItemObjectIsNumberInteger(obj)) {
        return;
    }
    *value = DmItemToInt64(DmGetCjsonPointer(obj->item));
}

void DmJsonItemObjectGetToBool(const DmJsonItemObject* obj, bool* value)
{
    *value = false;
    if (obj->item == nullptr) {
        return;
    }
    if (!DmJsonItemObjectIsBoolean(obj)) {
        return;
    }
    *value = cJSON_IsTrue(DmGetCjsonPointer(obj->item));
}

void DmJsonItemObjectGetToUint64(const DmJsonItemObject* obj, uint64_t* value)
{
    *value = 0;
    if (obj->item == nullptr) {
        return;
    }
    if (!DmJsonItemObjectIsNumberInteger(obj)) {
        return;
    }
    *value = DmItemToUint64(DmGetCjsonPointer(obj->item));
}

DmVec_DmJsonItemObject DmJsonItemObjectItems(const DmJsonItemObject* obj)
{
    DmVec_DmJsonItemObject items;
    DmVec_DmJsonItemObject_Init(&items);
    if (obj->item == nullptr) {
        return items;
    }
    cJSON* jsonItem = DmGetCjsonPointer(obj->item);
    if (jsonItem->type == cJSON_Object || jsonItem->type == cJSON_Array) {
        cJSON* current = nullptr;
        cJSON_ArrayForEach(current, jsonItem) {
            DmJsonItemObject child;
            DmJsonItemObjectInit(&child);
            child.item = current;
            child.parent = obj->item;
            DmVec_DmJsonItemObject_Push(&items, child);
        }
    }
    return items;
}

void DmJsonItemObjectErase(DmJsonItemObject* obj, const char* key)
{
    if (obj->item == nullptr) {
        return;
    }
    if (DmJsonItemObjectIsObject(obj)) {
        cJSON_DeleteItemFromObjectCaseSensitive(DmGetCjsonPointer(obj->item), key);
    }
}

void DmJsonItemObjectSetKey(DmJsonItemObject* obj, const char* key, const char* value)
{
    if (obj->item == nullptr) {
        return;
    }
    if (cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(DmGetCjsonPointer(obj->item), key);
    }
    cJSON_AddItemToObject(DmGetCjsonPointer(obj->item), key, cJSON_CreateString(value));
}

void DmJsonItemObjectSetKeyInt(DmJsonItemObject* obj, const char* key, int32_t value)
{
    if (obj->item == nullptr) {
        return;
    }
    if (cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(DmGetCjsonPointer(obj->item), key);
    }
    cJSON_AddItemToObject(DmGetCjsonPointer(obj->item), key, cJSON_CreateNumber(static_cast<double>(value)));
}

void DmJsonItemObjectSetKeyUint32(DmJsonItemObject* obj, const char* key, uint32_t value)
{
    if (obj->item == nullptr) {
        return;
    }
    if (cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(DmGetCjsonPointer(obj->item), key);
    }
    cJSON_AddItemToObject(DmGetCjsonPointer(obj->item), key, cJSON_CreateNumber(static_cast<double>(value)));
}

void DmJsonItemObjectSetKeyInt64(DmJsonItemObject* obj, const char* key, int64_t value)
{
    if (obj->item == nullptr) {
        return;
    }
    if (cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(DmGetCjsonPointer(obj->item), key);
    }
    cJSON_AddItemToObject(DmGetCjsonPointer(obj->item), key, DmCreateInt64Item(value));
}

void DmJsonItemObjectSetKeyBool(DmJsonItemObject* obj, const char* key, bool value)
{
    if (obj->item == nullptr) {
        return;
    }
    if (cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(DmGetCjsonPointer(obj->item), key);
    }
    cJSON_AddItemToObject(DmGetCjsonPointer(obj->item), key, value ? cJSON_CreateTrue() : cJSON_CreateFalse());
}

bool DmJsonItemObjectIsBool(const DmJsonItemObject* obj, const char* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(obj, key);
    return DmJsonItemObjectIsBoolean(&item);
}

bool DmJsonItemObjectGetBool(const DmJsonItemObject* obj, const char* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(obj, key);
    bool value = false;
    DmJsonItemObjectGetToBool(&item, &value);
    return value;
}

bool DmJsonItemObjectIsInt32(const DmJsonItemObject* obj, const char* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(obj, key);
    return DmJsonItemObjectIsNumberInteger(&item);
}

int32_t DmJsonItemObjectGetInt32(const DmJsonItemObject* obj, const char* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(obj, key);
    return DmJsonGetInt(&item);
}

bool DmJsonItemObjectContainsAt(const DmJsonItemObject* obj, const char* parentKey, const char* childKey)
{
    DmJsonItemObject parent = DmJsonItemObjectAt(obj, parentKey);
    return DmJsonItemObjectContains(&parent, childKey);
}

bool DmJsonItemObjectIsInt32At(const DmJsonItemObject* obj, const char* parentKey, const char* childKey)
{
    DmJsonItemObject parent = DmJsonItemObjectAt(obj, parentKey);
    return DmJsonItemObjectIsInt32(&parent, childKey);
}

int32_t DmJsonItemObjectGetInt32At(const DmJsonItemObject* obj, const char* parentKey, const char* childKey)
{
    DmJsonItemObject parent = DmJsonItemObjectAt(obj, parentKey);
    return DmJsonItemObjectGetInt32(&parent, childKey);
}

void DmJsonItemObjectParse(DmJsonItemObject* obj, const char* strJson)
{
    DmJsonItemObjectDestroy(obj);
    obj->itemName = DmStringCreateEmpty();
    if (strJson != nullptr && strlen(strJson) > 0) {
        obj->item = cJSON_Parse(strJson);
        obj->needDeleteItem = true;
        obj->beValid = true;
    }
}

DmString DmJsonItemObjectDumpArrayString(const DmJsonItemObject* obj)
{
    return DmJsonItemObjectDump(obj);
}

int DmJsonItemObjectArraySize(const DmJsonItemObject* obj, const char* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(obj, key);
    if (item.item == nullptr) {
        return 0;
    }
    return cJSON_GetArraySize(DmGetCjsonPointer(item.item));
}

int64_t DmJsonItemObjectGetInt64FromArray(const DmJsonItemObject* obj, const char* key, int index)
{
    DmJsonItemObject arr = DmJsonItemObjectAt(obj, key);
    if (arr.item == nullptr) {
        return 0;
    }
    cJSON* arrItem = DmGetCjsonPointer(arr.item);
    if (index < 0 || index >= cJSON_GetArraySize(arrItem)) {
        return 0;
    }
    cJSON* elem = cJSON_GetArrayItem(arrItem, index);
    return DmItemToInt64(elem);
}

void DmJsonItemObjectPutInt64Array(DmJsonItemObject* obj, const char* key, DmVecInt64_t* arr)
{
    if (obj->item == nullptr) {
        return;
    }
    cJSON* newArray = cJSON_CreateArray();
    for (int i = 0; i < static_cast<int>(DmVecInt64_t_Size(arr)); i++) {
        int64_t* val = DmVecInt64_t_At(arr, i);
        cJSON_AddItemToArray(newArray, val ? DmCreateInt64Item(*val) : DmCreateInt64Item(0));
    }
    if (cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(DmGetCjsonPointer(obj->item), key);
    }
    cJSON_AddItemToObject(DmGetCjsonPointer(obj->item), key, newArray);
}

void DmJsonItemObjectPutStringArray(DmJsonItemObject* obj, const char* key, DmVec_DmString* arr)
{
    if (obj->item == nullptr) {
        return;
    }
    cJSON* newArray = cJSON_CreateArray();
    for (int i = 0; i < static_cast<int>(DmVec_DmString_Size(arr)); i++) {
        const DmString* s = DmVec_DmString_At(arr, i);
        cJSON_AddItemToArray(newArray, cJSON_CreateString(DmStringCstr(s)));
    }
    if (cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(obj->item), key) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(DmGetCjsonPointer(obj->item), key);
    }
    cJSON_AddItemToObject(DmGetCjsonPointer(obj->item), key, newArray);
}

void DmJsonItemObjectGetInt64Array(const DmJsonItemObject* obj, const char* key, DmVecInt64_t* arr)
{
    DmVecInt64_t_Clear(arr);
    DmJsonItemObject arrItem = DmJsonItemObjectAt(obj, key);
    if (arrItem.item == nullptr) {
        return;
    }
    cJSON* arrPtr = DmGetCjsonPointer(arrItem.item);
    if (!cJSON_IsArray(arrPtr)) {
        return;
    }
    int size = cJSON_GetArraySize(arrPtr);
    for (int i = 0; i < size; i++) {
        cJSON* elem = cJSON_GetArrayItem(arrPtr, i);
        DmVecInt64_t_Push(arr, DmItemToInt64(elem));
    }
}

bool DmJsonItemObjectIsUint64(const DmJsonItemObject* obj, const char* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(obj, key);
    return DmJsonItemObjectIsNumberInteger(&item);
}

uint64_t DmJsonItemObjectGetUint64(const DmJsonItemObject* obj, const char* key)
{
    DmJsonItemObject item = DmJsonItemObjectAt(obj, key);
    return DmJsonGetUint64(&item);
}

DmString DmJsonGetString(const DmJsonItemObject* obj)
{
    DmString value;
    DmJsonItemObjectGetToString(obj, &value);
    return value;
}

int32_t DmJsonGetInt(const DmJsonItemObject* obj)
{
    int32_t value = 0;
    DmJsonItemObjectGetToInt32(obj, &value);
    return value;
}

int64_t DmJsonGetInt64(const DmJsonItemObject* obj)
{
    int64_t value = 0;
    DmJsonItemObjectGetToInt64(obj, &value);
    return value;
}

uint64_t DmJsonGetUint64(const DmJsonItemObject* obj)
{
    uint64_t value = 0;
    DmJsonItemObjectGetToUint64(obj, &value);
    return value;
}

bool DmJsonGetBool(const DmJsonItemObject* obj)
{
    bool value = false;
    DmJsonItemObjectGetToBool(obj, &value);
    return value;
}

double DmJsonGetDouble(const DmJsonItemObject* obj)
{
    double value = 0.0;
    DmJsonItemObjectGetToDouble(obj, &value);
    return value;
}

DmJsonItemObject DmJsonGetObject(const DmJsonItemObject* obj, const char* key)
{
    return DmJsonItemObjectAt(obj, key);
}

DmString DmJsonToString(const DmJsonItemObject* jsonItem)
{
    return DmJsonItemObjectDump(jsonItem);
}

void DmJsonObjectInit(DmJsonObject* obj, DmJsonCreateType type)
{
    DmJsonItemObjectInit(&obj->base);
    obj->createType = type;
    obj->base.needDeleteItem = true;
    obj->base.beValid = true;
    if (type == DM_JSON_CREATE_TYPE_OBJECT) {
        obj->base.item = cJSON_CreateObject();
    } else {
        obj->base.item = cJSON_CreateArray();
    }
}

void DmJsonObjectInitFromStr(DmJsonObject* obj, const char* strJson)
{
    DmJsonItemObjectInit(&obj->base);
    obj->base.needDeleteItem = true;
    obj->base.beValid = true;
    DmJsonObjectParse(obj, strJson);
}

void DmJsonObjectDestroy(DmJsonObject* obj)
{
    DmJsonItemObjectDestroy(&obj->base);
}

bool DmJsonObjectParse(DmJsonObject* obj, const char* strJson)
{
    DmJsonItemObjectDestroy(&obj->base);
    obj->base.itemName = DmStringCreateEmpty();
    if (strJson != nullptr && strlen(strJson) > 0) {
        obj->base.item = cJSON_Parse(strJson);
        obj->base.needDeleteItem = true;
        return true;
    }
    LOGE("strJson is empty");
    return false;
}

void DmJsonObjectDuplicate(DmJsonObject* obj, const DmJsonObject* source)
{
    DmJsonItemObjectDestroy(&obj->base);
    obj->base.item = cJSON_Duplicate(DmGetCjsonPointer(source->base.item), cJSON_True);
    obj->base.needDeleteItem = true;
}

DmJsonItemObject* DmJsonObjectGetItem(DmJsonObject* obj)
{
    return &obj->base;
}

DmJsonItemObject* DmJsonObjectGetByKey(DmJsonObject* obj, const char* key)
{
    if (obj == nullptr || key == nullptr) {
        return nullptr;
    }
    DmJsonItemObject* item = DmJsonObjectGetItem(obj);
    if (item == nullptr || item->item == nullptr) {
        return nullptr;
    }
    cJSON* found = cJSON_GetObjectItemCaseSensitive(DmGetCjsonPointer(item->item), key);
    if (found == nullptr) {
        return nullptr;
    }
    static DmJsonItemObject result;
    DmJsonItemObjectInit(&result);
    result.item = found;
    result.parent = item->item;
    result.beValid = true;
    result.itemName = DmStringCreate(key);
    result.needDeleteItem = false;
    return &result;
}

DM_VEC_IMPL(DmJsonItemObject)
