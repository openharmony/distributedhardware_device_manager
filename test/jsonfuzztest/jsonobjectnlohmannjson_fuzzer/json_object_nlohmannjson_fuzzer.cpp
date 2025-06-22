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
    std::string strValue = fdp.ConsumeRandomLengthString();
    int16_t int16Value = fdp.ConsumeIntegral<int16_t>();
    double doubleValue = fdp.ConsumeFloatingPoint<double>();
    bool boolValue = fdp.ConsumeBool();
    uint8_t uint8Value = fdp.ConsumeIntegral<uint8_t>();
    uint16_t uint16Value = fdp.ConsumeIntegral<uint16_t>();
    uint32_t uint32Value = fdp.ConsumeIntegral<uint32_t>();
    int64_t int64Value = fdp.ConsumeIntegral<int64_t>();
    ToJson(itemObject, doubleValue);
    ToJson(itemObject, int16Value);
    FromJson(itemObject, doubleValue);
    FromJson(itemObject, boolValue);
    FromJson(itemObject, uint8Value);
    FromJson(itemObject, uint16Value);
    itemObject.IsNumber();
    itemObjectNew = itemObject;
    itemObjectNew.PushBack(strValue);
    itemObjectNew.PushBack(doubleValue);
    itemObjectNew.PushBack(int64Value);
    itemObjectNew.GetTo(doubleValue);
    itemObjectNew.GetTo(uint32Value);
    itemObjectNew.GetTo(boolValue);
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
