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

#ifndef OHOS_DM_PIN_AUTH_UI_H
#define OHOS_DM_PIN_AUTH_UI_H

#include <cstdint>

#include "dm_auth_manager.h"
#include "dm_ability_manager.h"

namespace OHOS {
namespace DistributedHardware {
class PinAuthUi {
public:
    /**
     * @tc.name: PinAuthUi::PinAuthUi
     * @tc.desc: AuthUi constructor
     * @tc.type: FUNC
     */
    PinAuthUi();
    /**
     * @tc.name: PinAuthUi::ShowPinDialog
     * @tc.desc: Show Pin Dialog
     * @tc.type: FUNC
     */
    int32_t ShowPinDialog(int32_t code, std::shared_ptr<DmAuthManager> authManager);
    /**
     * @tc.name: PinAuthUi::InputPinDialog
     * @tc.desc: Show Input PinDialog
     * @tc.type: FUNC
     */
    int32_t InputPinDialog(std::shared_ptr<DmAuthManager> authManager);
    /**
     * @tc.name: PinAuthUi::ClosePage
     * @tc.desc: Close Page
     * @tc.type: FUNC
     */
    int32_t ClosePage(const int32_t &pageId, std::shared_ptr<DmAuthManager> authManager);

    /**
     * @tc.name: PinAuth::UpdateAuthInfo
     * @tc.desc: Update the page information
     * @tc.type: FUNC
     */
    int32_t UpdatePinDialog(int32_t pageId);

private:
    int32_t StartFaUiService(std::shared_ptr<DmAbilityManager> dmAbilityManager);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PIN_AUTH_UI_H
