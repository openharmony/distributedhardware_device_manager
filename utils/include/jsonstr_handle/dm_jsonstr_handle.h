/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_JSONSTR_HANDLE_H
#define OHOS_DM_JSONSTR_HANDLE_H

#include "dm_single_instance.h"
#include <string>
#include <vector>

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__ ((visibility ("default")))
#endif // DM_EXPORT

namespace OHOS {
namespace DistributedHardware {
class JsonStrHandle {
    DM_DECLARE_SINGLE_INSTANCE(JsonStrHandle);

public:
    DM_EXPORT void GetPeerAppInfoParseExtra(const std::string &extra,
        uint64_t &peerTokenId, std::string &peerBundleName);
    DM_EXPORT std::vector<int64_t> GetProxyTokenIdByExtra(const std::string &extraInfo);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_JSONSTR_HANDLE_H
