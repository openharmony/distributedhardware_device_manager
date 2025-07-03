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
#include "json_object_nlohmannjson_fuzzer.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {

void ToJsonDoubleNLFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    double value = fdp.ConsumeFloatingPoint<double>();

    ToJson(itemObject, value);
}

void ToJsonInt16NLFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int16_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    JsonItemObject itemObject;
    int16_t value = fdp.ConsumeIntegral<int16_t>();

    ToJson(itemObject, value);
}

void FromJsonDoubleNLFuzzTest(const uint8_t* data, size_t size)
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

void FromJsonBoolNLFuzzTest(const uint8_t* data, size_t size)
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

void FromJsonUint8NLFuzzTest(const uint8_t* data, size_t size)
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

void FromJsonInt16NLFuzzTest(const uint8_t* data, size_t size)
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

void IsNumberNLFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    JsonItemObject Object;
    Object.IsNumber();
}

void JsonItemObjectAssignmentNLFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    JsonItemObject sourceObj;
    JsonItemObject targetObj;
    targetObj = sourceObj;
}

void PushBackDoubleNLFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(double)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject Object;

    double value = fdp.ConsumeFloatingPoint<double>();
    Object.PushBack(value);
}

void PushBackInt64NLFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(int64_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject Object;

    int64_t value = fdp.ConsumeIntegral<int64_t>();
    Object.PushBack(value);
}

void PushBackStringNLFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject Object;

    std::string value = fdp.ConsumeRandomLengthString();
    Object.PushBack(value);
}

void GetToDoubleNLFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(double)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject Object;
    double value = fdp.ConsumeFloatingPoint<double>();
    Object.GetTo(value);
}

void GetToUint32NLFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject Object;
    uint32_t value = fdp.ConsumeIntegral<uint32_t>();
    Object.GetTo(value);
}

void GetToBoolNLFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(bool)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    JsonItemObject Object;
    bool value = fdp.ConsumeBool();;
    Object.GetTo(value);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::ToJsonDoubleNLFuzzTest(data, size);
    OHOS::DistributedHardware::ToJsonInt16NLFuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonDoubleNLFuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonBoolNLFuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonUint8NLFuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonInt16NLFuzzTest(data, size);
    OHOS::DistributedHardware::IsNumberNLFuzzTest(data, size);
    OHOS::DistributedHardware::JsonItemObjectAssignmentNLFuzzTest(data, size);
    OHOS::DistributedHardware::PushBackDoubleNLFuzzTest(data, size);
    OHOS::DistributedHardware::PushBackInt64NLFuzzTest(data, size);
    OHOS::DistributedHardware::PushBackStringNLFuzzTest(data, size);
    OHOS::DistributedHardware::GetToDoubleNLFuzzTest(data, size);
    OHOS::DistributedHardware::GetToUint32NLFuzzTest(data, size);
    OHOS::DistributedHardware::GetToBoolNLFuzzTest(data, size);

    return 0;
}
