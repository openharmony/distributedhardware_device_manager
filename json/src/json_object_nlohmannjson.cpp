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

#include "nlohmann/json.hpp"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
static nlohmann::json *GetJsonPointer(void *pointer)
{
    return static_cast<nlohmann::json*>(pointer);
}

void ToJson(JsonItemObject &itemObject, const std::string &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = value;
    }
}

void ToJson(JsonItemObject &itemObject, const char *value)
{
    if (itemObject.item_ != nullptr && value != nullptr) {
        *GetJsonPointer(itemObject.item_) = std::string(value);
    }
}

void ToJson(JsonItemObject &itemObject, const double &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = value;
    }
}

void ToJson(JsonItemObject &itemObject, const bool &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = value;
    }
}

void ToJson(JsonItemObject &itemObject, const uint8_t &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = (int)value;
    }
}

void ToJson(JsonItemObject &itemObject, const int16_t &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = value;
    }
}

void ToJson(JsonItemObject &itemObject, const uint16_t &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = value;
    }
}

void ToJson(JsonItemObject &itemObject, const int32_t &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = value;
    }
}

void ToJson(JsonItemObject &itemObject, const uint32_t &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = value;
    }
}

void ToJson(JsonItemObject &itemObject, const int64_t &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = value;
    }
}

void ToJson(JsonItemObject &itemObject, const uint64_t &value)
{
    if (itemObject.item_ != nullptr) {
        *GetJsonPointer(itemObject.item_) = value;
    }
}

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
    parent_ = object.parent_;
    itemName_ = object.itemName_;
    itemIndex_ = object.itemIndex_;
    needDeleteItem_ = object.needDeleteItem_;
    if (object.needDeleteItem_ && object.item_ != nullptr) {
        item_ = new nlohmann::json();
        if (item_ != nullptr) {
            *GetJsonPointer(item_) = *GetJsonPointer(object.item_);
        }
    } else {
        item_ = object.item_;
    }
}

JsonItemObject::~JsonItemObject()
{
    Delete();
}

void JsonItemObject::Delete()
{
    if (needDeleteItem_ && item_ != nullptr) {
        nlohmann::json *item = GetJsonPointer(item_);
        delete item;
    }
    item_ = nullptr;
}

bool JsonItemObject::IsString() const
{
    if (item_ == nullptr) {
        return false;
    }
    return GetJsonPointer(item_)->is_string();
}

bool JsonItemObject::IsNumber() const
{
    if (item_ == nullptr) {
        return false;
    }
    return GetJsonPointer(item_)->is_number();
}

bool JsonItemObject::IsNumberInteger() const
{
    if (item_ == nullptr) {
        return false;
    }
    return GetJsonPointer(item_)->is_number_integer();
}

bool JsonItemObject::IsArray() const
{
    if (item_ == nullptr) {
        return false;
    }
    return GetJsonPointer(item_)->is_array();
}

bool JsonItemObject::IsBoolean() const
{
    if (item_ == nullptr) {
        return false;
    }
    return GetJsonPointer(item_)->is_boolean();
}

bool JsonItemObject::IsObject() const
{
    if (item_ == nullptr) {
        return false;
    }
    return GetJsonPointer(item_)->is_object();
}

void JsonItemObject::Insert(const std::string &key, const JsonItemObject &object)
{
    if (item_ == nullptr || object.item_ == nullptr) {
        LOGE("invalid item or object item");
        return;
    }
    (*GetJsonPointer(item_))[key] = *GetJsonPointer(object.item_);
}

JsonItemObject& JsonItemObject::operator=(const JsonItemObject &object)
{
    parent_ = object.parent_;
    itemName_ = object.itemName_;
    itemIndex_ = object.itemIndex_;
    needDeleteItem_ = object.needDeleteItem_;
    if (object.needDeleteItem_ && object.item_ != nullptr) {
        if (item_ == nullptr) {
            item_ = new nlohmann::json();
        }
        if (item_ != nullptr) {
            *GetJsonPointer(item_) = *GetJsonPointer(object.item_);
        }
    } else {
        item_ = object.item_;
    }
    return *this;
}

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
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return "";
    }
    int indent = -1;
    char indent_char = ' ';
    bool ensure_ascii = false;
    nlohmann::detail::error_handler_t error_handler = nlohmann::detail::error_handler_t::strict;
    if (formatFlag) {
        indent = 1;
        indent_char = '\t';
    }
    if (isIgnoreError) {
        error_handler = nlohmann::detail::error_handler_t::ignore;
    }
    return GetJsonPointer(item_)->dump(indent, indent_char, ensure_ascii, error_handler);
}

JsonItemObject JsonItemObject::operator[](const std::string &key)
{
    JsonItemObject itemObject = At(key);
    if (itemObject.item_ == nullptr) {
        auto& newItem = (*GetJsonPointer(item_))[key];
        itemObject.item_ = &newItem;
        itemObject.beValid_ = true;
    }
    return itemObject;
}

const JsonItemObject JsonItemObject::operator[](const std::string &key) const
{
    return At(key);
}

bool JsonItemObject::Contains(const std::string &key) const
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    return GetJsonPointer(item_)->contains(key);
}

bool JsonItemObject::IsDiscarded() const
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return true;
    }
    return GetJsonPointer(item_)->is_discarded();
}

bool JsonItemObject::PushBack(const std::string &strValue)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (!GetJsonPointer(item_)->is_array()) {
        LOGE("item_ type is not array");
        return false;
    }
    GetJsonPointer(item_)->push_back(strValue);
    return true;
}

bool JsonItemObject::PushBack(const double &value)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (!GetJsonPointer(item_)->is_array()) {
        LOGE("item_ type is not array");
        return false;
    }
    GetJsonPointer(item_)->push_back(value);
    return true;
}

bool JsonItemObject::PushBack(const int64_t &value)
{
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return false;
    }
    if (!GetJsonPointer(item_)->is_array()) {
        LOGE("item_ type is not array");
        return false;
    }
    GetJsonPointer(item_)->push_back(value);
    return true;
}

bool JsonItemObject::PushBack(const JsonItemObject &item)
{
    if (item_ == nullptr || item.item_ == nullptr) {
        LOGE("item_ or item.item_ is nullptr");
        return false;
    }
    if (!GetJsonPointer(item_)->is_array()) {
        LOGE("item_ type is not array");
        return false;
    }
    GetJsonPointer(item_)->push_back(*GetJsonPointer(item.item_));
    return true;
}

void JsonItemObject::AddItemToArray(JsonItemObject &item)
{
    PushBack(item);
}

std::string JsonItemObject::Key() const
{
    return itemName_;
}

JsonItemObject JsonItemObject::At(const std::string &key) const
{
    JsonItemObject operationItem;
    if (item_ == nullptr) {
        LOGE("item_ is nullptr");
        return operationItem;
    }
    if (Contains(key)) {
        auto& findItem = GetJsonPointer(item_)->at(key);
        operationItem.item_ = &findItem;
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
    GetJsonPointer(item_)->get_to(value);
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
    GetJsonPointer(item_)->get_to(value);
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
    GetJsonPointer(item_)->get_to(value);
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
    GetJsonPointer(item_)->get_to(value);
}

std::vector<JsonItemObject> JsonItemObject::Items() const
{
    std::vector<JsonItemObject> items;
    if (item_ == nullptr) {
        return items;
    }
    for (auto &element : GetJsonPointer(item_)->items()) {
        JsonItemObject newItem;
        newItem.itemName_ = element.key();
        newItem.item_ = &(element.value());
        newItem.parent_ = item_;
        items.push_back(newItem);
    }
    return items;
}

bool JsonItemObject::InitItem(JsonItemObject &item)
{
    if (!beValid_) {
        LOGE("invalid item");
        return false;
    }
    if (item.item_ == nullptr) {
        item.needDeleteItem_ = true;
        item.item_ = new nlohmann::json();
        if (item.item_ == nullptr) {
            LOGE("new item fail");
            return false;
        }
    }
    item.parent_ = parent_;
    item.beValid_ = true;
    item.itemName_ = itemName_;
    item.itemIndex_ = itemIndex_;
    return true;
}

bool JsonItemObject::InitArray()
{
    if (!beValid_ || item_ == nullptr) {
        return false;
    }
    *GetJsonPointer(item_) = nlohmann::json::array({});
    return true;
}

bool JsonItemObject::ReplaceItem(void *newItem)
{
    if (newItem == nullptr) {
        LOGE("newItem is null");
        return false;
    }
    if (item_ == nullptr) {
        needDeleteItem_ = true;
        item_ = new nlohmann::json();
        if (item_ == nullptr) {
            LOGE("new item fail");
            return false;
        }
    }
    *GetJsonPointer(item_) = *GetJsonPointer(newItem);
    return true;
}

void JsonItemObject::Erase(const std::string &key)
{
    if (item_ == nullptr) {
        return;
    }
    if (IsObject()) {
        GetJsonPointer(item_)->erase(key);
    }
}


JsonObject::JsonObject(JsonCreateType type)
{
    needDeleteItem_ = true;
    item_ = new nlohmann::json();
    if (item_ != nullptr) {
        beValid_ = true;
        if (type == JsonCreateType::JSON_CREATE_TYPE_ARRAY) {
            *GetJsonPointer(item_) = nlohmann::json::array({});
        }
    }
}

JsonObject::JsonObject(const std::string &strJson)
{
    needDeleteItem_ = true;
    item_ = new nlohmann::json();
    if (item_ != nullptr) {
        beValid_ = true;
        Parse(strJson);
    }
}

JsonObject::~JsonObject()
{
    Delete();
}

bool JsonObject::Parse(const std::string &strJson)
{
    if (item_ == nullptr) {
        return false;
    }
    if (!strJson.empty()) {
        *GetJsonPointer(item_) = nlohmann::json::parse(strJson, nullptr, false);
        return true;
    }
    LOGE("strJson is empty");
    *GetJsonPointer(item_) = nlohmann::json::parse(strJson, nullptr, false);
    return false;
}

void JsonObject::Duplicate(const JsonObject &object)
{
    if (item_ != nullptr && object.item_ != nullptr) {
        *GetJsonPointer(item_) = *GetJsonPointer(object.item_);
    }
}

} // namespace DistributedHardware
} // namespace OHOS