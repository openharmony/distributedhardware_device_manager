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

#ifndef OHOS_DM_MULTIPLE_USER_CONNECTOR_3RD_H
#define OHOS_DM_MULTIPLE_USER_CONNECTOR_3RD_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace OHOS {
namespace DistributedHardware {

class MultipleUserConnector3rd {
public:
    static void GetCallerUserId(int32_t &userId);
    static void GetCallingTokenId(uint32_t &tokenId);
    static int32_t GetCurrentAccountUserID(void);
    static std::string GetOhosAccountIdByUserId(int32_t userId);
    static int32_t GetFirstForegroundUserId(void);
    static int32_t GetForegroundUserIds(std::vector<int32_t> &userVec);
private:
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_MULTIPLE_USER_CONNECTOR_3RD_H