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

void ToJsonDoubleFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    double value = fdp.ConsumeFloatingPoint<double>();

    ToJson(itemObject, value);
}
void ToJsonUint8FuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    uint8_t value = fdp.ConsumeIntegral<uint8_t>();

    ToJson(itemObject, value);
}

void ToJsonInt16FuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    int16_t value = fdp.ConsumeIntegral<int16_t>();

    ToJson(itemObject, value);
}

void ToJsonUint16FuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    uint16_t value = fdp.ConsumeIntegral<uint16_t>();

    ToJson(itemObject, value);
}

void ToJsonUint64FuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    uint64_t value = fdp.ConsumeIntegral<uint64_t>();

    ToJson(itemObject, value);
}

void FromJsonDoubleFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    double result = fdp.ConsumeFloatingPoint<double>();

    double testValue = fdp.ConsumeFloatingPoint<double>();
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonBoolFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    bool result = fdp.ConsumeBool();

    bool testValue = fdp.ConsumeBool();
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonUint8FuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    uint8_t result = fdp.ConsumeIntegral<uint8_t>();

    int32_t testValue = fdp.ConsumeIntegralInRange<int32_t>(
        std::numeric_limits<uint8_t>::min(),
        std::numeric_limits<uint8_t>::max());
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonInt16FuzzTest(FuzzedDataProvider &fdp)
{
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

void FromJsonUint16FuzzTest(FuzzedDataProvider &fdp)
{
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

void FromJsonInt32FuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    int32_t result = fdp.ConsumeIntegral<int32_t>();

    int32_t testValue = fdp.ConsumeIntegral<int32_t>();
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void PushBackDoubleFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;

    double value = fdp.ConsumeFloatingPoint<double>();
    object.PushBack(value);
}

void PushBackInt64FuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;

    int64_t value = fdp.ConsumeIntegral<int64_t>();
    object.PushBack(value);
}

void GetToBoolFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;
    bool value = fdp.ConsumeBool();
    object.GetTo(value);
}

void EraseFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;
    std::string key = fdp.ConsumeRandomLengthString();
    object.Erase(key);
}

void JsonModelFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::ToJsonDoubleFuzzTest(fdp);
    OHOS::DistributedHardware::ToJsonUint8FuzzTest(fdp);
    OHOS::DistributedHardware::ToJsonInt16FuzzTest(fdp);
    OHOS::DistributedHardware::ToJsonUint16FuzzTest(fdp);
    OHOS::DistributedHardware::ToJsonUint64FuzzTest(fdp);
    OHOS::DistributedHardware::FromJsonDoubleFuzzTest(fdp);
    OHOS::DistributedHardware::FromJsonBoolFuzzTest(fdp);
    OHOS::DistributedHardware::FromJsonUint8FuzzTest(fdp);
    OHOS::DistributedHardware::FromJsonInt16FuzzTest(fdp);
    OHOS::DistributedHardware::FromJsonUint16FuzzTest(fdp);
    OHOS::DistributedHardware::FromJsonInt32FuzzTest(fdp);
    OHOS::DistributedHardware::PushBackDoubleFuzzTest(fdp);
    OHOS::DistributedHardware::PushBackInt64FuzzTest(fdp);
    OHOS::DistributedHardware::GetToBoolFuzzTest(fdp);
    OHOS::DistributedHardware::EraseFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::JsonModelFuzzTest(data, size);

    return 0;
}
