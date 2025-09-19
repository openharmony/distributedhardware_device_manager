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

void ToJsonDoubleNLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    double value = fdp.ConsumeFloatingPoint<double>();
    ToJson(itemObject, value);
}

void ToJsonInt16NLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    int16_t value = fdp.ConsumeIntegral<int16_t>();
    ToJson(itemObject, value);
}

void FromJsonDoubleNLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    double result = fdp.ConsumeFloatingPoint<double>();
    double testValue = fdp.ConsumeFloatingPoint<double>();
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonBoolNLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    bool result = fdp.ConsumeBool();
    bool testValue = fdp.ConsumeBool();
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonUint8NLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    uint8_t result = fdp.ConsumeIntegral<uint8_t>();
    int32_t testValue = fdp.ConsumeIntegralInRange<int32_t>(
        std::numeric_limits<uint8_t>::min(),
        std::numeric_limits<uint8_t>::max());
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void FromJsonInt16NLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject itemObject;
    int16_t result = fdp.ConsumeIntegralInRange<int16_t>(
        std::numeric_limits<int16_t>::min(),
        std::numeric_limits<int16_t>::max());

    int32_t testValue = fdp.ConsumeIntegralInRange<int32_t>(
        std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max());
    JsonItemObject targetObj = itemObject;
    ToJson(itemObject, testValue);
    FromJson(itemObject, result);
}

void PushBackDoubleNLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;

    double value = fdp.ConsumeFloatingPoint<double>();
    object.PushBack(value);
}

void PushBackInt64NLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;

    int64_t value = fdp.ConsumeIntegral<int64_t>();
    object.PushBack(value);
}

void PushBackStringNLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;

    std::string value = fdp.ConsumeRandomLengthString();
    object.PushBack(value);
}

void GetToDoubleNLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;
    double value = fdp.ConsumeFloatingPoint<double>();
    object.GetTo(value);
}

void GetToUint32NLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;
    uint32_t value = fdp.ConsumeIntegral<uint32_t>();
    object.GetTo(value);
}

void GetToBoolNLFuzzTest(FuzzedDataProvider &fdp)
{
    JsonItemObject object;
    bool value = fdp.ConsumeBool();
    object.GetTo(value);
}

void JsonObjectNlohmannjsonFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    ToJsonDoubleNLFuzzTest(fdp);
    ToJsonInt16NLFuzzTest(fdp);
    FromJsonDoubleNLFuzzTest(fdp);
    FromJsonBoolNLFuzzTest(fdp);
    FromJsonUint8NLFuzzTest(fdp);
    FromJsonInt16NLFuzzTest(fdp);
    PushBackDoubleNLFuzzTest(fdp);
    PushBackInt64NLFuzzTest(fdp);
    PushBackStringNLFuzzTest(fdp);
    GetToDoubleNLFuzzTest(fdp);
    GetToUint32NLFuzzTest(fdp);
    GetToBoolNLFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::JsonObjectNlohmannjsonFuzzTest(data, size);
    return 0;
}
