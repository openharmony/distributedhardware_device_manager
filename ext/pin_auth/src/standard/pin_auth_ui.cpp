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

#include "pin_auth_ui.h"

#include "dm_ability_manager.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
constexpr const char* VERIFY_FAILED = "verifyFailed";

PinAuthUi::PinAuthUi()
{
    LOGI("AuthUi constructor");
}

int32_t PinAuthUi::ShowPinDialog(int32_t code, std::shared_ptr<DmAuthManager> authManager)
{
    LOGI("ShowPinDialog start");
    if (authManager == nullptr) {
        LOGE("authManager is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<DmAbilityManager> dmAbilityMgr = std::make_shared<DmAbilityManager>();
    AAFwk::Want want;
    want.SetParam("PinCode", std::to_string(code));
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    const std::string deviceId = localDeviceId;
    const std::string bundleUiName = "com.ohos.devicemanagerui";
    const std::string abilityUiName = "com.ohos.devicemanagerui.PincodeServiceExtAbility";
    AppExecFwk::ElementName element(deviceId, bundleUiName, abilityUiName);
    want.SetElement(element);
    AbilityStatus status = dmAbilityMgr->StartAbility(want);
    if (status != AbilityStatus::ABILITY_STATUS_SUCCESS) {
        LOGE("ShowConfirm::start ui service fail");
        return ERR_DM_FAILED;
    }
    LOGI("ShowPinDialog end");
    return DM_OK;
}

int32_t PinAuthUi::InputPinDialog(std::shared_ptr<DmAuthManager> authManager)
{
    LOGI("InputPinDialog start");
    if (authManager == nullptr) {
        LOGE("authManager is null");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<DmAbilityManager> dmAbilityMgr = std::make_shared<DmAbilityManager>();

    AAFwk::Want want;
    want.SetParam(VERIFY_FAILED, false);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    const std::string deviceId = localDeviceId;
    const std::string bundleUiName = "com.ohos.devicemanagerui";
    const std::string abilityUiName = "com.ohos.devicemanagerui.InputServiceExtAbility";
    AppExecFwk::ElementName element(deviceId, bundleUiName, abilityUiName);
    want.SetElement(element);
    AbilityStatus status = dmAbilityMgr->StartAbility(want);
    if (status != AbilityStatus::ABILITY_STATUS_SUCCESS) {
        LOGE("ShowConfirm::start ui service success");
        return ERR_DM_FAILED;
    }
    LOGI("InputPinDialog end");
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
