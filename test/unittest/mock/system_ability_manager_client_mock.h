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
#ifndef OHOS_DM_SYSTEM_ABILITY_MANAGER_CLIENT_MOCK_H
#define OHOS_DM_SYSTEM_ABILITY_MANAGER_CLIENT_MOCK_H

#include <gmock/gmock.h>
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
class ISystemAbilityManagerClient {
public:
    ISystemAbilityManagerClient() = default;
    virtual ~ISystemAbilityManagerClient() = default;

    virtual sptr<ISystemAbilityManager> GetSystemAbilityManager() = 0;
    static std::shared_ptr<ISystemAbilityManagerClient> GetOrCreateSAMgrClient();
    static void ReleaseSAMgrClient();
private:
    static std::shared_ptr<ISystemAbilityManagerClient> client_;
};

class SystemAbilityManagerClientMock : public ISystemAbilityManagerClient {
public:
    MOCK_METHOD0(GetSystemAbilityManager, sptr<ISystemAbilityManager>());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SYSTEM_ABILITY_MANAGER_CLIENT_MOCK_H
