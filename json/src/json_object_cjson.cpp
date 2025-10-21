/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace DistributedHardware {

static cJSON *GetCJsonPointer(void *pointer)
{
    return static_cast<cJSON*>(pointer);
}

static bool AddToArray(cJSON *item, cJSON *newItem)
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

void ToJson(JsonItemObject &itemObject, const std::string &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
    itemObject.item_ = cJSON_CreateString(value.c_str());
}

void ToJson(JsonItemObject &itemObject, const char *value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
    itemObject.item_ = cJSON_CreateString(value);
}

void ToJson(JsonItemObject &itemObject, const double &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
    itemObject.item_ = cJSON_CreateNumber(value);
}

void ToJson(JsonItemObject &itemObject, const bool &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
    itemObject.item_ = value ? cJSON_CreateTrue() : cJSON_CreateFalse();
}

void ToJson(JsonItemObject &itemObject, const uint8_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
#ifdef __CJSON_USE_INT64
    itemObject.item_ = cJSON_CreateInt64Number(value);
#else
    itemObject.item_ = cJSON_CreateNumber(value);
#endif
}

void ToJson(JsonItemObject &itemObject, const int16_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
#ifdef __CJSON_USE_INT64
    itemObject.item_ = cJSON_CreateInt64Number(value);
#else
    itemObject.item_ = cJSON_CreateNumber(value);
#endif
}

void ToJson(JsonItemObject &itemObject, const uint16_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
#ifdef __CJSON_USE_INT64
    itemObject.item_ = cJSON_CreateInt64Number(value);
#else
    itemObject.item_ = cJSON_CreateNumber(value);
#endif
}

void ToJson(JsonItemObject &itemObject, const int32_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
#ifdef __CJSON_USE_INT64
    itemObject.item_ = cJSON_CreateInt64Number(value);
#else
    itemObject.item_ = cJSON_CreateNumber(value);
#endif
}

void ToJson(JsonItemObject &itemObject, const uint32_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
#ifdef __CJSON_USE_INT64
    itemObject.item_ = cJSON_CreateInt64Number(value);
#else
    itemObject.item_ = cJSON_CreateNumber(value);
#endif
}

void ToJson(JsonItemObject &itemObject, const int64_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
#ifdef __CJSON_USE_INT64
    itemObject.item_ = cJSON_CreateInt64Number(value);
#else
    itemObject.item_ = cJSON_CreateNumber(value);
#endif
}

void ToJson(JsonItemObject &itemObject, const uint64_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(itemObject.item_));
    }
#ifdef __CJSON_USE_INT64
    itemObject.item_ = cJSON_CreateInt64Number(value);
#else
    itemObject.item_ = cJSON_CreateNumber(value);
#endif
}

//LCOV_EXCL_START
void FromJson(const JsonItemObject &itemObject, std::string &value)
{
    itemObject.GetTo(value);
}

void FromJson(const JsonItemObject &itemObject, double &value)
{
    itemObject.GetTo(value);
}

void FromJson(const JsonItemObject &itemObject, bool &value)
{
    itemObject.GetTo(value);
}

void FromJson(const JsonItemObject &itemObject, uint8_t &value)
{
    int32_t tmpValue = 0;
    itemObject.GetTo(tmpValue);
    value = static_cast<uint8_t>(tmpValue);
}

void FromJson(const JsonItemObject &itemObject, int16_t &value)
{
    int32_t tmpValue = 0;
    itemObject.GetTo(tmpValue);
    value = static_cast<int16_t>(tmpValue);
}

void FromJson(const JsonItemObject &itemObject, uint16_t &value)
{
    int32_t tmpValue = 0;
    itemObject.GetTo(tmpValue);
    value = static_cast<uint16_t>(tmpValue);
}

void FromJson(const JsonItemObject &itemObject, int32_t &value)
{
    itemObject.GetTo(value);
}

void FromJson(const JsonItemObject &itemObject, uint32_t &value)
{
    int32_t tmpValue = 0;
    itemObject.GetTo(tmpValue);
    value = static_cast<uint32_t>(tmpValue);
}

void FromJson(const JsonItemObject &itemObject, int64_t &value)
{
    itemObject.GetTo(value);
}

void FromJson(const JsonItemObject &itemObject, uint64_t &value)
{
    int64_t tmpValue = 0;
    itemObject.GetTo(tmpValue);
    value = static_cast<uint64_t>(tmpValue);
}

std::string ToString(const JsonItemObject &jsonItem)
{
    return jsonItem.Dump();
}

JsonItemObject::JsonItemObject()
{}

JsonItemObject::JsonItemObject(const JsonItemObject &object)
{
    item_ = object.item_;
    parent_ = object.parent_;
    itemName_ = object.itemName_;
    needDeleteItem_ = object.needDeleteItem_;
    if (object.needDeleteItem_) {
        item_ = cJSON_Duplicate(GetCJsonPointer(object.item_), cJSON_True);
    }
}

JsonItemObject::~JsonItemObject()
{
    Delete();
}

void JsonItemObject::Delete()
{
    if (needDeleteItem_ && item_ != nullptr) {
        cJSON_Delete(GetCJsonPointer(item_));
    }
    item_ = nullptr;
}

bool JsonItemObject::IsString() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsString(GetCJsonPointer(item_));
}

bool JsonItemObject::IsNumber() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsNumber(GetCJsonPointer(item_));
}

#ifdef __CJSON_USE_INT64
bool JsonItemObject::IsNumberInteger() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsInt64Number(GetCJsonPointer(item_));
}
#else
bool JsonItemObject::IsNumberInteger() const
{
    if (!IsNumber()) {
        return false;
    }
    double value = 0.0;
    GetTo(value);
    return ((value - static_cast<int64_t>(value)) == 0);
}
#endif

bool JsonItemObject::IsArray() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsArray(GetCJsonPointer(item_));
}

bool JsonItemObject::IsBoolean() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsBool(GetCJsonPointer(item_));
}

bool JsonItemObject::IsObject() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsObject(GetCJsonPointer(item_));
}
//LCOV_EXCL_STOP

void JsonItemObject::Insert(const std::string &key, const JsonItemObject &object)
{
    if (item_ == nullptr || object.item_ == nullptr) {
        LOGE("invalid item or object item");
        return;
    }
    cJSON *newItem = cJSON_Duplicate(GetCJsonPointer(object.item_), cJSON_True);
    if (newItem == nullptr) {
        LOGE("copy item fail");
        return;
    }
    if (cJSON_GetObjectItemCaseSensitive(GetCJsonPointer(item_), key.c_str()) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(GetCJsonPointer(item_), key.c_str());
    }
    if (!cJSON_AddItemToObject(GetCJsonPointer(item_), key.c_str(), newItem)) {
        LOGE("add new item to object fail");
        cJSON_Delete(newItem);
    }
}

JsonItemObject& JsonItemObject::operator=(const JsonItemObject &object)
{
    item_ = object.item_;
    parent_ = object.parent_;
    itemName_ = object.itemName_;
    itemIndex_ = object.itemIndex_;
    needDeleteItem_ = object.needDeleteItem_;
    if (object.needDeleteItem_) {
        item_ = cJSON_Duplicate(GetCJsonPointer(object.item_), cJSON_True);
    }
    return *this;
}

//LCOV_EXCL_START
std::string JsonItemObject::DumpFormated() const
{
    return Dump(true, true);
}

std::string JsonItemObject::Dump() const
{
    return Dump(false, true);
}

std::string JsonItemObject::Dump(bool formatFlag, bool isIgnoreError) const
{
    (void) isIgnoreError;
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return "";
    }
    cJSON *jsonItem = GetCJsonPointer(item_);
    char* jsonString = formatFlag ? cJSON_Print(jsonItem) : cJSON_PrintUnformatted(jsonItem);
    if (jsonString == nullptr) {
        return "";
    }
    std::string out(jsonString);
    cJSON_free(jsonString);
    return out;
}

JsonItemObject JsonItemObject::operator[](const std::string &key)
{
    JsonItemObject itemObject = At(key);
    if (itemObject.item_ == nullptr) {
        itemObject.item_ = cJSON_CreateNull();
        if (itemObject.item_ == nullptr) {
            return itemObject;
        }
        if (!cJSON_AddItemToObject(GetCJsonPointer(item_), key.c_str(), GetCJsonPointer(itemObject.item_))) {
            LOGE("add item to object fail");
            cJSON_Delete(GetCJsonPointer(itemObject.item_));
            itemObject.item_ = nullptr;
        } else {
            itemObject.beValid_ = true;
        }
    }
    return itemObject;
}

const JsonItemObject JsonItemObject::operator[](const std::string &key) const
{
    return At(key);
}
//LCOV_EXCL_STOP

bool JsonItemObject::Contains(const std::string &key) const
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    cJSON* item = cJSON_GetObjectItemCaseSensitive(GetCJsonPointer(item_), key.c_str());
    return (item != nullptr);
}

//LCOV_EXCL_START
bool JsonItemObject::IsDiscarded() const
{
    return (item_ == nullptr);
}
//LCOV_EXCL_STOP

bool JsonItemObject::PushBack(const std::string &strValue)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (GetCJsonPointer(item_)->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON *newItem = cJSON_CreateString(strValue.c_str());
    return AddToArray(GetCJsonPointer(item_), newItem);
}

bool JsonItemObject::PushBack(const double &value)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (GetCJsonPointer(item_)->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON *newItem = cJSON_CreateNumber(value);
    return AddToArray(GetCJsonPointer(item_), newItem);
}

bool JsonItemObject::PushBack(const int64_t &value)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (GetCJsonPointer(item_)->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
#ifdef __CJSON_USE_INT64
    cJSON *newItem = cJSON_CreateInt64Number(value);
#else
    cJSON *newItem = cJSON_CreateNumber(static_cast<double>(value));
#endif
    return AddToArray(GetCJsonPointer(item_), newItem);
}

bool JsonItemObject::PushBack(const JsonItemObject &item)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (!item.beValid_ || !beValid_) {
        return false;
    }
    if (GetCJsonPointer(item_)->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON* newItem = cJSON_Duplicate(GetCJsonPointer(item.item_), cJSON_True);
    return AddToArray(GetCJsonPointer(item_), newItem);
}

void JsonItemObject::AddItemToArray(JsonItemObject &item)
{
    cJSON* jsonItem = GetCJsonPointer(item_);
    if (cJSON_AddItemToArray(jsonItem, GetCJsonPointer(item.item_))) {
        item.itemIndex_ = cJSON_GetArraySize(jsonItem) - 1;
    } else if (item.item_ != nullptr) {
        // item add fail, need delete
        item.needDeleteItem_ = true;
    }
}

//LCOV_EXCL_START
std::string JsonItemObject::Key() const
{
    if (item_ != nullptr) {
        return std::string(GetCJsonPointer(item_)->string);
    }
    return itemName_;
}
//LCOV_EXCL_STOP

JsonItemObject JsonItemObject::At(const std::string &key) const
{
    JsonItemObject operationItem;
    operationItem.item_ = cJSON_GetObjectItemCaseSensitive(GetCJsonPointer(item_), key.c_str());
    if (operationItem.item_ != nullptr) {
        operationItem.beValid_ = true;
    }
    operationItem.parent_ = item_;
    operationItem.itemName_ = key;
    return operationItem;
}

void JsonItemObject::GetTo(std::string &value) const
{
    value = "";
    if (item_ == nullptr) {
        LOGE("value item is null");
        return;
    }
    if (!IsString()) {
        return;
    }
    const char* strValue = cJSON_GetStringValue(GetCJsonPointer(item_));
    if (strValue == nullptr) {
        return;
    }
    value = std::string(strValue);
}

void JsonItemObject::GetTo(double &value) const
{
    value = 0.0;
    if (item_ == nullptr) {
        LOGE("value item is null");
        return;
    }
    if (!IsNumber()) {
        return;
    }
    value = cJSON_GetNumberValue(GetCJsonPointer(item_));
}

void JsonItemObject::GetTo(int32_t &value) const
{
    int64_t tmpValue = 0;
    GetTo(tmpValue);
    value = static_cast<int32_t>(tmpValue);
}

void JsonItemObject::GetTo(uint32_t &value) const
{
    int64_t tmpValue = 0;
    GetTo(tmpValue);
    value = static_cast<uint32_t>(tmpValue);
}

void JsonItemObject::GetTo(int64_t &value) const
{
    value = 0;
    if (item_ == nullptr) {
        LOGE("value item is null");
        return;
    }
    if (!IsNumberInteger()) {
        return;
    }
#ifdef __CJSON_USE_INT64
    long long *pValue = cJSON_GetInt64NumberValue(GetCJsonPointer(item_));
    if (pValue == nullptr) {
        LOGE("value is null");
        return;
    }
    value = *pValue;
#else
    double tmpValue = cJSON_GetNumberValue(GetCJsonPointer(item_));
    value = static_cast<int64_t>(tmpValue);
#endif
}

void JsonItemObject::GetTo(bool &value) const
{
    value = false;
    if (item_ == nullptr) {
        LOGE("value item is null");
        return;
    }
    if (!IsBoolean()) {
        return;
    }
    value = cJSON_IsTrue(GetCJsonPointer(item_)) ? true : false;
}

//LCOV_EXCL_START
std::vector<JsonItemObject> JsonItemObject::Items() const
{
    std::vector<JsonItemObject> items;
    if (item_ == nullptr) {
        return items;
    }
    cJSON *current = nullptr;
    cJSON *jsonItem = GetCJsonPointer(item_);
    if (jsonItem->type == cJSON_Object || jsonItem->type == cJSON_Array) {
        cJSON_ArrayForEach(current, jsonItem) {
            JsonItemObject child;
            child.item_ = current;
            child.parent_ = item_;
            items.push_back(child);
        }
    }
    return items;
}
//LCOV_EXCL_STOP

bool JsonItemObject::InitItem(JsonItemObject &item)
{
    if (!beValid_) {
        LOGE("invalid item");
        return false;
    }
    item.item_ = cJSON_CreateObject();
    if (item.item_ == nullptr) {
        LOGE("create new object item fail");
        return false;
    }
    item.parent_ = parent_;
    item.beValid_ = true;
    item.itemName_ = itemName_;
    item.itemIndex_ = itemIndex_;
    return true;
}

//LCOV_EXCL_START
bool JsonItemObject::InitArray()
{
    if (!beValid_) {
        return false;
    }
    cJSON *newItem = cJSON_CreateArray();
    if (newItem == nullptr) {
        return false;
    }
    if (!ReplaceItem(newItem)) {
        cJSON_Delete(newItem);
        return false;
    }
    return true;
}
//LCOV_EXCL_STOP

bool JsonItemObject::ReplaceItem(void *newItem)
{
    if (parent_ != nullptr) {
        cJSON *jsonParent = GetCJsonPointer(parent_);
        if (cJSON_IsObject(jsonParent)) {
            if (!cJSON_ReplaceItemInObjectCaseSensitive(jsonParent, itemName_.c_str(), GetCJsonPointer(newItem))) {
                LOGE("replace item in object fail, itemName:%{public}s", itemName_.c_str());
                return false;
            }
        } else if (cJSON_IsArray(jsonParent) && itemIndex_ >= 0 && itemIndex_ < cJSON_GetArraySize(jsonParent)) {
            if (!cJSON_ReplaceItemInArray(jsonParent, itemIndex_, GetCJsonPointer(newItem))) {
                LOGE("replace item in array fail, itemIndex:%{public}d", itemIndex_);
                return false;
            }
        }
    } else {
        cJSON_Delete(GetCJsonPointer(item_));
    }
    item_ = newItem;
    return true;
}

void JsonItemObject::Erase(const std::string &key)
{
    if (item_ == nullptr) {
        return;
    }
    if (IsObject()) {
        cJSON_DeleteItemFromObjectCaseSensitive(GetCJsonPointer(item_), key.c_str());
    }
}


JsonObject::JsonObject(JsonCreateType type)
{
    needDeleteItem_ = true;
    beValid_ = true;
    if (type == JsonCreateType::JSON_CREATE_TYPE_OBJECT) {
        item_ = cJSON_CreateObject();
    } else {
        item_ = cJSON_CreateArray();
    }
}

JsonObject::JsonObject(const std::string &strJson)
{
    needDeleteItem_ = true;
    beValid_ = true;
    Parse(strJson);
}

JsonObject::~JsonObject()
{
    Delete();
}

bool JsonObject::Parse(const std::string &strJson)
{
    Delete();
    if (!strJson.empty()) {
        item_ = cJSON_Parse(strJson.c_str());
        return true;
    }
    LOGE("strJson is empty");
    return false;
}

void JsonObject::Duplicate(const JsonObject &object)
{
    Delete();
    item_ = cJSON_Duplicate(GetCJsonPointer(object.item_), cJSON_True);
}

} // namespace DistributedHardware
} // namespace OHOS
