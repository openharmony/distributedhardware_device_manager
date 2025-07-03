/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fuzzer/FuzzedDataProvider.h>
#include "json_object_cjson_fuzzer.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {

void ToJsonDoubleFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(double)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    double value = fdp.ConsumeFloatingPoint<double>();

    ToJson(itemObject, value);
}
void ToJsonUint8FuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(uint8_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;

    uint8_t value = fdp.ConsumeIntegral<uint8_t>();

    ToJson(itemObject, value);
}

void ToJsonInt16FuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int16_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    int16_t value = fdp.ConsumeIntegral<int16_t>();

    ToJson(itemObject, value);
}

void ToJsonUint16FuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(uint16_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    uint16_t value = fdp.ConsumeIntegral<uint16_t>();

    ToJson(itemObject, value);
}

void ToJsonUint64FuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(uint64_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    uint64_t value = fdp.ConsumeIntegral<uint64_t>();

    ToJson(itemObject, value);
}

void FromJsonDoubleFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(double) + sizeof(double)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    double result = fdp.ConsumeFloatingPoint<double>();

    double testValue = fdp.ConsumeFloatingPoint<double>();
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonBoolFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(bool) + sizeof(bool)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    bool result = fdp.ConsumeBool();

    bool testValue = fdp.ConsumeBool();
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonUint8FuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t) + sizeof(uint8_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    uint8_t result = fdp.ConsumeIntegral<uint8_t>();

    int32_t testValue = fdp.ConsumeIntegralInRange<int32_t>(
        std::numeric_limits<uint8_t>::min(),
        std::numeric_limits<uint8_t>::max());
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonInt16FuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t) + sizeof(int16_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    int16_t result = fdp.ConsumeIntegralInRange<int16_t>(
        std::numeric_limits<int16_t>::min(),
        std::numeric_limits<int16_t>::max());

    int32_t testValue = fdp.ConsumeIntegralInRange<int32_t>(
        std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max());
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonUint16FuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t) + sizeof(uint16_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    uint16_t result = fdp.ConsumeIntegralInRange<uint16_t>(
        std::numeric_limits<uint16_t>::min(),
        std::numeric_limits<uint16_t>::max());

    int32_t testValue = fdp.ConsumeIntegralInRange<int32_t>(
        std::numeric_limits<uint16_t>::min(),
        std::numeric_limits<uint16_t>::max());
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonInt32FuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t) + sizeof(int32_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    int32_t result = fdp.ConsumeIntegral<int32_t>();

    int32_t testValue = fdp.ConsumeIntegral<int32_t>();
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void ToStringFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;

    JsonItemObject object;

    ToString(object);
}

void IsArrayFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    JsonItemObject object;
    object.IsArray();
}

void IsObjectFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    JsonItemObject object;
    object.IsObject();
}

void JsonItemObjectAssignmentFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    JsonItemObject sourceObj;
    JsonItemObject targetObj = sourceObj;
}

void PushBackDoubleFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(double)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject object;

    double value = fdp.ConsumeFloatingPoint<double>();
    object.PushBack(value);
}

void PushBackInt64FuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int64_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject object;

    int64_t value = fdp.ConsumeIntegral<int64_t>();
    object.PushBack(value);
}

void PushBackJsonItemObjectFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    JsonItemObject object;
    JsonItemObject objectTemp;
    object.PushBack(objectTemp);
}

void AddItemToArrayFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    JsonItemObject object;
    JsonItemObject objectTemp;
    object.AddItemToArray(objectTemp);
}

void GetToBoolFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(bool)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject object;
    bool value = fdp.ConsumeBool();
    object.GetTo(value);
}

void InitArrayFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    JsonItemObject object;
    object.InitArray();
}

void EraseFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject object;
    std::string key = fdp.ConsumeRandomLengthString();
    object.Erase(key);
}

void DuplicateFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    JsonObject object;
    JsonObject objectTemp;
    object.Duplicate(objectTemp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::ToJsonDoubleFuzzTest(data, size);
    OHOS::DistributedHardware::ToJsonUint8FuzzTest(data, size);
    OHOS::DistributedHardware::ToJsonInt16FuzzTest(data, size);
    OHOS::DistributedHardware::ToJsonUint16FuzzTest(data, size);
    OHOS::DistributedHardware::ToJsonUint64FuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonDoubleFuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonBoolFuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonUint8FuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonInt16FuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonUint16FuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonInt32FuzzTest(data, size);
    OHOS::DistributedHardware::ToStringFuzzTest(data, size);
    OHOS::DistributedHardware::IsArrayFuzzTest(data, size);
    OHOS::DistributedHardware::IsObjectFuzzTest(data, size);
    OHOS::DistributedHardware::JsonItemObjectAssignmentFuzzTest(data, size);
    OHOS::DistributedHardware::PushBackDoubleFuzzTest(data, size);
    OHOS::DistributedHardware::PushBackInt64FuzzTest(data, size);
    OHOS::DistributedHardware::PushBackJsonItemObjectFuzzTest(data, size);
    OHOS::DistributedHardware::AddItemToArrayFuzzTest(data, size);
    OHOS::DistributedHardware::GetToBoolFuzzTest(data, size);
    OHOS::DistributedHardware::InitArrayFuzzTest(data, size);
    OHOS::DistributedHardware::EraseFuzzTest(data, size);
    OHOS::DistributedHardware::DuplicateFuzzTest(data, size);

    return 0;
}
