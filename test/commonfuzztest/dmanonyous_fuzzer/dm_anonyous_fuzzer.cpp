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
#include <chrono>
#include <securec.h>
#include <string>

#include "device_manager_impl.h"
#include "dm_constants.h"
#include "softbus_listener.h"
#include "dm_anonymous.h"
namespace OHOS {
namespace DistributedHardware {

void DmAnonyousFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::vector<std::string> strList;
    int64_t testNumber = 123;
    std::string test1 = fdp.ConsumeRandomLengthString();
    std::string test2 = fdp.ConsumeRandomLengthString();
    strList.push_back(test1);
    strList.push_back(test2);
    GetAnonyStringList(strList);
    std::vector<int32_t> intList;
    intList.push_back(1);
    GetAnonyInt32List(intList);
    std::string key1 = fdp.ConsumeRandomLengthString();
    std::string key2 = fdp.ConsumeRandomLengthString();
    std::string key3 = fdp.ConsumeRandomLengthString();
    std::string value1 = fdp.ConsumeRandomLengthString();
    std::string value2 = fdp.ConsumeRandomLengthString();
    JsonObject jsonObj;
    jsonObj[key1] = testNumber;
    jsonObj[key2] = value2;
    jsonObj[key3] = true;
    IsUint32(jsonObj, key1);
    IsUint64(jsonObj, key1);
    IsBool(jsonObj, key3);
    std::map<std::string, std::string> paramMap;
    paramMap[key1] = value1;
    paramMap[key2] = value2;
    std::map<std::string, std::string> paramMap2 = {};
    std::string jsonStr = ConvertMapToJsonString(paramMap);
    ParseMapFromJsonString(jsonStr, paramMap2);
    IsJsonValIntegerString(jsonObj, key2);
}

void DmAnonyousFuzzTestFirst(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string str1 = fdp.ConsumeRandomLengthString();
    std::string str2 = fdp.ConsumeRandomLengthString();
    int32_t versionNum = 0;
    int64_t decimal = 10;
    int64_t testNumber = 123;
    StringToInt(str1, decimal);
    StringToInt64(str1, decimal);
    GetVersionNumber("1.0.0", versionNum);
    GetCallerPkgName("com.example.app#test");
    GetSubscribeId("123#12345");
    std::multimap<std::string, int32_t> unorderedmap;
    unorderedmap.insert(std::make_pair(str1, testNumber));
    IsValueExist(unorderedmap, str1, testNumber);
    GetSubStr("test#123", "#", 0);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmAnonyousFuzzTest(data, size);
    OHOS::DistributedHardware::DmAnonyousFuzzTestFirst(data, size);
    return 0;
}
