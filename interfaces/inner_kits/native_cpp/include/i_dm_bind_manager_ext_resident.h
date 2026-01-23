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

#ifndef OHOS_I_DM_BIND_MANAGER_EXT_RESIDENT_H
#define OHOS_I_DM_BIND_MANAGER_EXT_RESIDENT_H

#include <cinttypes>
#include <string>

namespace OHOS {
namespace DistributedHardware {
class IDMBindManagerExtResident {
public:
    virtual ~IDMBindManagerExtResident() = default;
    virtual int32_t SendResidentData(int32_t sessionId, std::string &message) = 0;
    virtual int32_t GetAuthInfoByMetaToken(const std::string &metaToken, DmAuthInfo &dmAuthInfo) = 0;
    virtual void ClearAuthInfoByMetaToken(const std::string &metaToken) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_BIND_MANAGER_EXT_RESIDENT_H
