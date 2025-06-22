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

#include "json_object_nlohmannjson_fuzzer.h"

#include <chrono>
#include <fuzzer/FuzzedDataProvider.h>
#include <securec.h>
#include <string>

#include "dm_constants.h"
#include "json_object.h"
namespace OHOS {
namespace DistributedHardware {

void JsonObjectNlohmannjsonFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    JsonItemObject itemObject;
    JsonItemObject itemObjectNew;
    itemObject["key1"] = fdp.ConsumeIntegral<int32_t>();
    std::string str_value = fdp.ConsumeRandomLengthString();
    int16_t int16_value = fdp.ConsumeIntegral<int16_t>();
    double double_value = fdp.ConsumeFloatingPoint<double>();
    bool bool_value = fdp.ConsumeBool();
    uint8_t uint8_value = fdp.ConsumeIntegral<uint8_t>();
    uint16_t uint16_value = fdp.ConsumeIntegral<uint16_t>();
    uint32_t uint32_value = fdp.ConsumeIntegral<uint32_t>();
    int64_t int64_value = fdp.ConsumeIntegral<int64_t>();
    ToJson(itemObject, double_value);
    ToJson(itemObject, int16_value);
    FromJson(itemObject, double_value);
    FromJson(itemObject, bool_value);
    FromJson(itemObject, uint8_value);
    FromJson(itemObject, uint16_value);
    itemObject.IsNumber();
    itemObjectNew = itemObject;
    itemObjectNew.PushBack(str_value);
    itemObjectNew.PushBack(double_value);
    itemObjectNew.PushBack(int64_value);
    itemObjectNew.GetTo(double_value);
    itemObjectNew.GetTo(uint32_value);
    itemObjectNew.GetTo(bool_value);
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
