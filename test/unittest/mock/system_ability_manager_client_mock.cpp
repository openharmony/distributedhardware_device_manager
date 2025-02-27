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

#include "system_ability_manager_client_mock.h"

using namespace OHOS::DistributedHardware;

namespace OHOS {
SystemAbilityManagerClient& SystemAbilityManagerClient::GetInstance()
{
    static auto instance = std::make_shared<SystemAbilityManagerClient>();
    return *instance;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    return ISystemAbilityManagerClient::GetOrCreateSAMgrClient()->GetSystemAbilityManager();
}

namespace DistributedHardware {
std::shared_ptr<ISystemAbilityManagerClient> ISystemAbilityManagerClient::client_ = nullptr;

std::shared_ptr<ISystemAbilityManagerClient> ISystemAbilityManagerClient::GetOrCreateSAMgrClient()
{
    if (!client_) {
        client_ = std::make_shared<SystemAbilityManagerClientMock>();
    }
    return client_;
}

void ISystemAbilityManagerClient::ReleaseSAMgrClient()
{
    client_.reset();
    client_ = nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS
