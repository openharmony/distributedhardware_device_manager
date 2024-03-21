/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "dm_ability_manager.h"

#include "ability_manager_client.h"
#include "auth_message_processor.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string bundleName = "com.ohos.devicemanagerui";
const std::string abilityName = "com.ohos.devicemanagerui.ConfirmServiceExtAbility";
const std::string peerDeviceName = "deviceName";
const std::string appOperation = "appOperation";
const std::string customDescription = "customDescription";
const std::string peerdeviceType = "deviceType";
} // namespace

AbilityStatus DmAbilityManager::StartAbility(const std::string &params)
{
    LOGI("Start Ability.");
    std::string deviceName = "";
    std::string appOperationStr = "";
    std::string customDescriptionStr = "";
    int32_t deviceType = -1;
    nlohmann::json jsonObject = nlohmann::json::parse(params, nullptr, false);
    if (!jsonObject.is_discarded()) {
        if (IsString(jsonObject, TAG_REQUESTER)) {
            deviceName = jsonObject[TAG_REQUESTER].get<std::string>();
        }
        if (IsString(jsonObject, TAG_APP_OPERATION)) {
            appOperationStr = jsonObject[TAG_APP_OPERATION].get<std::string>();
        }
        if (IsString(jsonObject, TAG_CUSTOM_DESCRIPTION)) {
            customDescriptionStr = jsonObject[TAG_CUSTOM_DESCRIPTION].get<std::string>();
        }
        if (IsInt32(jsonObject, TAG_LOCAL_DEVICE_TYPE)) {
            deviceType = jsonObject[TAG_LOCAL_DEVICE_TYPE].get<std::int32_t>();
        }
    }

    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string deviceId = localDeviceId;
    AAFwk::Want want;
    AppExecFwk::ElementName element(deviceId, bundleName, abilityName);
    want.SetElement(element);
    want.SetParam(peerDeviceName, deviceName);
    want.SetParam(customDescription, customDescriptionStr);
    want.SetParam(peerdeviceType, deviceType);
    if (!appOperationStr.empty()) {
        want.SetParam(appOperation, appOperationStr);
    }

    AAFwk::AbilityManagerClient::GetInstance()->Connect();
    ErrCode result = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
    if (result != 0) {
        LOGE("Start Ability failed, error value = %{public}d", (int32_t)result);
        return AbilityStatus::ABILITY_STATUS_FAILED;
    }
    return AbilityStatus::ABILITY_STATUS_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
