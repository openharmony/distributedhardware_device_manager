/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_ANONYMOUS_H
#define OHOS_DM_ANONYMOUS_H

#include <map>
#include <string>

#include "dm_device_info.h"

#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
std::string GetAnonyString(const std::string &value);
std::string GetAnonyInt32(const int32_t value);
bool IsNumberString(const std::string &inputString);
bool IsString(const nlohmann::json &jsonObj, const std::string &key);
bool IsInt32(const nlohmann::json &jsonObj, const std::string &key);
bool IsUint32(const nlohmann::json &jsonObj, const std::string &key);
bool IsInt64(const nlohmann::json &jsonObj, const std::string &key);
bool IsArray(const nlohmann::json &jsonObj, const std::string &key);
bool IsBool(const nlohmann::json &jsonObj, const std::string &key);
std::string ConvertMapToJsonString(const std::map<std::string, std::string> &paramMap);
void ParseMapFromJsonString(const std::string &jsonStr, std::map<std::string, std::string> &paramMap);
bool IsInvalidPeerTargetId(const PeerTargetId &targetId);
std::string ConvertCharArray2String(const char *srcData, uint32_t srcLen);
int32_t StringToInt(const std::string &str, int32_t base);
int64_t StringToInt64(const std::string &str, int32_t base);
void VersionSplitToInt(const std::string &str, const char split, std::vector<int32_t> &numVec);
bool CompareVecNum(const std::vector<int32_t> &srcVecNum, const std::vector<int32_t> &sinkVecNum);
bool CompareVersion(const std::string &remoteVersion, const std::string &oldVersion);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_ANONYMOUS_H
