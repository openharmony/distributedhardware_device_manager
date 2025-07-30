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

#ifndef JSON_OBJECT_H
#define JSON_OBJECT_H

#include <string>
#include <vector>

namespace OHOS {
namespace DistributedHardware {
class JsonObject;
class JsonItemObject {
    friend void ToJson(JsonItemObject &itemObject, const std::string &value);
    friend void ToJson(JsonItemObject &itemObject, const char *value);
    friend void ToJson(JsonItemObject &itemObject, const double &value);
    friend void ToJson(JsonItemObject &itemObject, const bool &value);
    friend void ToJson(JsonItemObject &itemObject, const uint8_t &value);
    friend void ToJson(JsonItemObject &itemObject, const int16_t &value);
    friend void ToJson(JsonItemObject &itemObject, const uint16_t &value);
    friend void ToJson(JsonItemObject &itemObject, const int32_t &value);
    friend void ToJson(JsonItemObject &itemObject, const uint32_t &value);
    friend void ToJson(JsonItemObject &itemObject, const int64_t &value);
    friend void ToJson(JsonItemObject &itemObject, const uint64_t &value);
public:
    JsonItemObject(const JsonItemObject &object);
    JsonItemObject(const JsonObject &object) = delete;
    virtual ~JsonItemObject();
    bool IsString() const;
    bool IsNumber() const;
    bool IsNumberInteger() const;
    bool IsArray() const;
    bool IsBoolean() const;
    bool IsObject() const;
    JsonItemObject& operator=(const JsonItemObject &object);
    JsonItemObject& operator=(const JsonObject &object) = delete;
    JsonItemObject operator[](const std::string &key);
    const JsonItemObject operator[](const std::string &key) const;
    std::string DumpFormated() const;
    std::string Dump() const;
    bool Contains(const std::string &key) const;
    bool IsDiscarded() const;
    bool PushBack(const std::string &strValue);
    bool PushBack(const double &value);
    bool PushBack(const int64_t &value);
    bool PushBack(const JsonItemObject &item);
    std::string Key() const;
    void Insert(const std::string &key, const JsonItemObject &object);
    JsonItemObject At(const std::string &key) const;
    void GetTo(std::string &strValue) const;
    void GetTo(double &value) const;
    void GetTo(int32_t &value) const;
    void GetTo(uint32_t &value) const;
    void GetTo(int64_t &value) const;
    void GetTo(bool &value) const;
    std::vector<JsonItemObject> Items() const;
    void Erase(const std::string &key);

    template<typename T>
    T Get() const
    {
        T value;
        FromJson(*this, value);
        return value;
    }

    template<typename T>
    void Get(std::vector<T> &dataList) const
    {
        dataList.clear();
        if (!IsArray()) {
            return;
        }
        for (auto& item : this->Items()) {
            T element;
            FromJson(item, element);
            dataList.push_back(element);
        }
    }

    template<typename T>
    JsonItemObject& operator=(const T &value)
    {
        JsonItemObject newItem;
        if (!InitItem(newItem)) {
            return *this;
        }
        ToJson(newItem, value);
        if (!ReplaceItem(newItem.item_)) {
            newItem.needDeleteItem_ = true;
            return *this;
        }
        return *this;
    }

    template<typename T>
    JsonItemObject& operator=(const std::vector<T> &values)
    {
        if (!InitArray()) {
            return *this;
        }
        for (const auto& value : values) {
            JsonItemObject operationItem;
            operationItem.parent_ = item_;
            operationItem.beValid_ = true;
            operationItem = value;
            AddItemToArray(operationItem);
        }
        return *this;
    }
protected:
    JsonItemObject();
    void Delete();
    std::string Dump(bool formatFlag, bool isIgnoreError) const;
    void AddItemToArray(JsonItemObject &item);
    bool InitItem(JsonItemObject &item);
    bool InitArray();
    bool ReplaceItem(void *newItem);
protected:
    void *item_ = nullptr;
    void *parent_ = nullptr;
    int32_t itemIndex_ = -1;
    bool needDeleteItem_ = false;
    bool beValid_ = false;
    std::string itemName_ = "";
};

enum class JsonCreateType {
    JSON_CREATE_TYPE_OBJECT = 0,
    JSON_CREATE_TYPE_ARRAY
};

class JsonObject : public JsonItemObject {
public:
    using JsonItemObject::operator=;
    JsonObject(JsonCreateType type = JsonCreateType::JSON_CREATE_TYPE_OBJECT);
    JsonObject(const std::string &strJson);
    JsonObject(const JsonObject &object) = delete;
    JsonObject& operator=(const JsonObject &object) = delete;
    JsonObject& operator=(const JsonItemObject &object) = delete;
    ~JsonObject();
    bool Parse(const std::string &strJson);
    void Duplicate(const JsonObject &object);
};

void ToJson(JsonItemObject &itemObject, const std::string &value);
void ToJson(JsonItemObject &itemObject, const char *value);
void ToJson(JsonItemObject &itemObject, const double &value);
void ToJson(JsonItemObject &itemObject, const bool &value);
void ToJson(JsonItemObject &itemObject, const uint8_t &value);
void ToJson(JsonItemObject &itemObject, const int16_t &value);
void ToJson(JsonItemObject &itemObject, const uint16_t &value);
void ToJson(JsonItemObject &itemObject, const int32_t &value);
void ToJson(JsonItemObject &itemObject, const uint32_t &value);
void ToJson(JsonItemObject &itemObject, const int64_t &value);
void ToJson(JsonItemObject &itemObject, const uint64_t &value);
void FromJson(const JsonItemObject &itemObject, std::string &value);
void FromJson(const JsonItemObject &itemObject, double &value);
void FromJson(const JsonItemObject &itemObject, bool &value);
void FromJson(const JsonItemObject &itemObject, uint8_t &value);
void FromJson(const JsonItemObject &itemObject, int16_t &value);
void FromJson(const JsonItemObject &itemObject, uint16_t &value);
void FromJson(const JsonItemObject &itemObject, int32_t &value);
void FromJson(const JsonItemObject &itemObject, uint32_t &value);
void FromJson(const JsonItemObject &itemObject, int64_t &value);
void FromJson(const JsonItemObject &itemObject, uint64_t &value);
std::string ToString(const JsonItemObject &jsonItem);
} // namespace DistributedHardware
} // namespace OHOS
#endif // JSON_OBJECT_H