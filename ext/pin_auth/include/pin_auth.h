/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_PIN_AUTH_H
#define OHOS_DM_PIN_AUTH_H

#include <cstdint>
#include <memory>

#include "authentication.h"
#include "dm_ability_manager.h"
#include "pin_auth_ui.h"

namespace OHOS {
namespace DistributedHardware {
class PinAuth : public IAuthentication {
public:
    PinAuth();
    ~PinAuth();
    int32_t ShowAuthInfo() override;
    int32_t StartAuth(std::shared_ptr<DmAbilityManager> dmAbilityManager) override;
    int32_t VerifyAuthentication(std::string pinToken, int32_t code, const std::string &authParam) override;

private:
    int32_t times_ = 0;
    std::shared_ptr<PinAuthUi> pinAuthUi_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PIN_AUTH_H
