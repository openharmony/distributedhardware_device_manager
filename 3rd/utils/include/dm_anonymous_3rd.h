/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_ANONYMOUS_3RD_H
#define OHOS_DM_ANONYMOUS_3RD_H

#include <map>
#include <string>

#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
    std::string ConvertMapToJsonString(const std::map<std::string, std::string> &paramMap);
    void ParseMapFromJsonString(const std::string &jsonStr, std::map<std::string, std::string> &paramMap);
    std::string GetAnonyInt32(const int32_t value);
    std::string GetAnonyString(const std::string &value);
    std::string GetAnonyJsonString(const std::string &value);
    bool IsString(const JsonItemObject &jsonObj, const std::string &key);
    bool IsInt32(const JsonItemObject &jsonObj, const std::string &key);
    bool IsBool(const JsonItemObject &jsonObj, const std::string &key);
    bool IsInt64(const JsonItemObject &jsonObj, const std::string &key);
    bool IsUint64(const JsonItemObject &jsonObj, const std::string &key);
    bool IsUint32(const JsonItemObject &jsonObj, const std::string &key);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_ANONYMOUS_3RD_H