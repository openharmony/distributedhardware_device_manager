/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_KV_INFO_H
#define OHOS_DM_KV_INFO_H

#include <string>

namespace OHOS {
namespace DistributedHardware {

typedef struct DmKVValue {
    std::string udidHash;
    std::string appID;
    std::string anoyDeviceId;
    std::string salt;
    int64_t lastModifyTime;
    explicit DmKVValue() : udidHash(""), appID(""), anoyDeviceId(""), salt(""), lastModifyTime(0) {}
} DmKVValue;
void ConvertDmKVValueToJson(const DmKVValue &kvValue, std::string &result);
void ConvertJsonToDmKVValue(const std::string &result, DmKVValue &kvValue);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_KV_INFO_H
