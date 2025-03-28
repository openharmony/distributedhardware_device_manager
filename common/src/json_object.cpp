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

namespace OHOS {
namespace DistributedHardware {

EXPORT void ToJson(JsonItemObject &itemObject, const std::string &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateString(value.c_str());
}

void ToJson(JsonItemObject &itemObject, const char *value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateString(value);
}

void ToJson(JsonItemObject &itemObject, const double &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateNumber(value);
}

void ToJson(JsonItemObject &itemObject, const bool &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = value ? cJSON_CreateTrue() : cJSON_CreateFalse();
}

void ToJson(JsonItemObject &itemObject, const uint8_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateNumber(value);
}

void ToJson(JsonItemObject &itemObject, const int16_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateNumber(value);
}

void ToJson(JsonItemObject &itemObject, const uint16_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateNumber(value);
}

void ToJson(JsonItemObject &itemObject, const int32_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateNumber(value);
}

void ToJson(JsonItemObject &itemObject, const uint32_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateNumber(value);
}

EXPORT void ToJson(JsonItemObject &itemObject, const int64_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateNumber(value);
}

void ToJson(JsonItemObject &itemObject, const uint64_t &value)
{
    if (itemObject.item_ != nullptr) {
        cJSON_Delete(itemObject.item_);
    }
    itemObject.item_ = cJSON_CreateNumber(value);
}

EXPORT void FromJson(const JsonItemObject &itemObject, std::string &value)
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

EXPORT void FromJson(const JsonItemObject &itemObject, int64_t &value)
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

EXPORT JsonItemObject::JsonItemObject()
{}

EXPORT JsonItemObject::JsonItemObject(const JsonItemObject &object)
{
    item_ = object.item_;
    parent_ = object.parent_;
    itemName_ = object.itemName_;
    needDeleteItem_ = object.needDeleteItem_;
    if (object.needDeleteItem_) {
        item_ = cJSON_Duplicate(object.item_, cJSON_True);
    }
}

EXPORT JsonItemObject::~JsonItemObject()
{
    Delete();
}

void JsonItemObject::Delete()
{
    if (needDeleteItem_ && item_ != nullptr) {
        cJSON_Delete(item_);
    }
    item_ = nullptr;
}

EXPORT bool JsonItemObject::IsString() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsString(item_);
}

bool JsonItemObject::IsNumber() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsNumber(item_);
}

EXPORT bool JsonItemObject::IsNumberInteger() const
{
    if (!IsNumber()) {
        return false;
    }
    double value = 0.0;
    GetTo(value);
    return ((value - static_cast<int64_t>(value)) == 0);
}

bool JsonItemObject::IsArray() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsArray(item_);
}

bool JsonItemObject::IsBoolean() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsBool(item_);
}

bool JsonItemObject::IsObject() const
{
    if (item_ == nullptr) {
        return false;
    }
    return cJSON_IsObject(item_);
}

void JsonItemObject::Insert(const std::string &key, const JsonItemObject &object)
{
    if (item_ == nullptr || object.item_ == nullptr) {
        LOGE("invalid item or object item");
        return;
    }
    cJSON *newItem = cJSON_Duplicate(object.item_, cJSON_True);
    if (newItem == nullptr) {
        LOGE("copy item fail");
        return;
    }
    if (cJSON_GetObjectItemCaseSensitive(item_, key.c_str()) != nullptr) {
        cJSON_DeleteItemFromObjectCaseSensitive(item_, key.c_str());
    }
    if (!cJSON_AddItemToObject(item_, key.c_str(), newItem)) {
        LOGE("add new item to object fail");
        cJSON_Delete(newItem);
    }
}

JsonItemObject& JsonItemObject::operator=(const JsonItemObject &object)
{
    item_ = object.item_;
    parent_ = object.parent_;
    itemName_ = object.itemName_;
    needDeleteItem_ = object.needDeleteItem_;
    if (object.needDeleteItem_) {
        item_ = cJSON_Duplicate(object.item_, cJSON_True);
    }
    return *this;
}

std::string JsonItemObject::DumpFormated() const
{
    return Dump(true);
}

EXPORT std::string JsonItemObject::Dump() const
{
    return Dump(false);
}

std::string JsonItemObject::Dump(bool formatFlag) const
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return "";
    }
    char* jsonString = formatFlag ? cJSON_Print(item_) : cJSON_PrintUnformatted(item_);
    if (jsonString == nullptr) {
        return "";
    }
    std::string out(jsonString);
    cJSON_free(jsonString);
    return out;
}

EXPORT JsonItemObject JsonItemObject::operator[](const std::string &key)
{
    JsonItemObject itemObject = At(key);
    if (itemObject.item_ == nullptr) {
        itemObject.item_ = cJSON_CreateNull();
        if (itemObject.item_ == nullptr) {
            return itemObject;
        }
        if (!cJSON_AddItemToObject(item_, key.c_str(), itemObject.item_)) {
            LOGE("add item to object fail");
            cJSON_Delete(itemObject.item_);
            itemObject.item_ = nullptr;
        } else {
            itemObject.beValid_ = true;
        }
    }
    return itemObject;
}

EXPORT const JsonItemObject JsonItemObject::operator[](const std::string &key) const
{
    return At(key);
}

EXPORT bool JsonItemObject::Contains(const std::string &key) const
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    cJSON* item = cJSON_GetObjectItemCaseSensitive(item_, key.c_str());
    return (item != nullptr);
}

EXPORT bool JsonItemObject::IsDiscarded() const
{
    return (item_ == nullptr);
}

bool JsonItemObject::PushBack(const std::string &strValue)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (item_->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON *newItem = cJSON_CreateString(strValue.c_str());
    return AddToArray(newItem);
}

bool JsonItemObject::PushBack(const double &value)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (item_->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON *newItem = cJSON_CreateNumber(value);
    return AddToArray(newItem);
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
    if (item_->type != cJSON_Array) {
        LOGE("item_ type is not array");
        return false;
    }
    cJSON* newItem = cJSON_Duplicate(item.item_, cJSON_True);
    return AddToArray(newItem);
}

bool JsonItemObject::AddToArray(cJSON *newItem)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (!cJSON_AddItemToArray(item_, newItem)) {
        LOGE("add item to array fail");
        cJSON_Delete(newItem);
        return false;
    }
    return true;
}

std::string JsonItemObject::Key() const
{
    if (item_ != nullptr) {
        return std::string(item_->string);
    }
    return itemName_;
}

JsonItemObject JsonItemObject::At(const std::string &key) const
{
    JsonItemObject operationItem;
    operationItem.item_ = cJSON_GetObjectItemCaseSensitive(item_, key.c_str());
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
    const char* strValue = cJSON_GetStringValue(item_);
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
    value = cJSON_GetNumberValue(item_);
}

void JsonItemObject::GetTo(int32_t &value) const
{
    double tmpValue = 0.0;
    GetTo(tmpValue);
    value = static_cast<int32_t>(tmpValue);
}

void JsonItemObject::GetTo(uint32_t &value) const
{
    double tmpValue = 0.0;
    GetTo(tmpValue);
    value = static_cast<uint32_t>(tmpValue);
}

void JsonItemObject::GetTo(int64_t &value) const
{
    double tmpValue = 0.0;
    GetTo(tmpValue);
    value = static_cast<int64_t>(tmpValue);
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
    value = cJSON_IsTrue(item_) ? true : false;
}

std::vector<JsonItemObject> JsonItemObject::Items() const
{
    std::vector<JsonItemObject> items;
    if (item_ == nullptr) {
        return items;
    }
    cJSON *current = nullptr;
    if (item_->type == cJSON_Object || item_->type == cJSON_Array) {
        cJSON_ArrayForEach(current, item_) {
            JsonItemObject child;
            child.item_ = current;
            child.parent_ = item_;
            items.push_back(child);
        }
    }
    return items;
}

EXPORT bool JsonItemObject::InitItem(JsonItemObject &item)
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

EXPORT bool JsonItemObject::ReplaceItem(cJSON *newItem)
{
    if (parent_ != nullptr) {
        if (cJSON_IsObject(parent_)) {
            if (!cJSON_ReplaceItemInObjectCaseSensitive(parent_, itemName_.c_str(), newItem)) {
                LOGE("replace item in object fail, itemName:%{public}s", itemName_.c_str());
                return false;
            }
        } else if (cJSON_IsArray(parent_) && itemIndex_ >= 0 && itemIndex_ < cJSON_GetArraySize(parent_)) {
            if (!cJSON_ReplaceItemInArray(parent_, itemIndex_, newItem)) {
                LOGE("replace item in array fail, itemIndex:%{public}d", itemIndex_);
                return false;
            }
        }
    } else {
        cJSON_Delete(item_);
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
        cJSON_DeleteItemFromObjectCaseSensitive(item_, key.c_str());
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

EXPORT JsonObject::JsonObject(const std::string &strJson)
{
    needDeleteItem_ = true;
    beValid_ = true;
    Parse(strJson);
}

EXPORT JsonObject::~JsonObject()
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
    item_ = cJSON_Duplicate(object.item_, cJSON_True);
}

} // namespace DistributedHardware
} // namespace OHOS