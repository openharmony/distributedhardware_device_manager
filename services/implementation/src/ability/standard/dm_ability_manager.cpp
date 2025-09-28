/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "json_object.h"
#include "parameter.h"
#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr const char* BUNDLE_NAME = "com.ohos.devicemanagerui";
constexpr const char* ABILITY_NAME = "com.ohos.devicemanagerui.ConfirmServiceExtAbility";
constexpr const char* PEER_DEVICE_NAME = "deviceName";
constexpr const char* PEER_DEVICE_TYPE = "deviceType";
} // namespace

AbilityStatus DmAbilityManager::StartAbility(const std::string &params)
{
    LOGI("Start Ability.");
    std::string deviceName = "";
    std::string appOperationStr = "";
    std::string customDescriptionStr = "";
    int32_t deviceType = -1;
    JsonObject jsonObject(params);
    if (!jsonObject.IsDiscarded()) {
        if (IsString(jsonObject, TAG_REQUESTER)) {
            deviceName = jsonObject[TAG_REQUESTER].Get<std::string>();
        }
        if (IsString(jsonObject, TAG_APP_OPERATION)) {
            appOperationStr = jsonObject[TAG_APP_OPERATION].Get<std::string>();
        }
        if (IsString(jsonObject, TAG_CUSTOM_DESCRIPTION)) {
            customDescriptionStr = jsonObject[TAG_CUSTOM_DESCRIPTION].Get<std::string>();
        }
        if (IsInt32(jsonObject, TAG_LOCAL_DEVICE_TYPE)) {
            deviceType = jsonObject[TAG_LOCAL_DEVICE_TYPE].Get<std::int32_t>();
        }
    }

    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string deviceId = localDeviceId;
    AAFwk::Want want;
    AppExecFwk::ElementName element(deviceId, BUNDLE_NAME, ABILITY_NAME);
    want.SetElement(element);
    want.SetParam(PEER_DEVICE_NAME, deviceName);
    want.SetParam(CUSTOM_DESCRIPTION, customDescriptionStr);
    want.SetParam(PEER_DEVICE_TYPE, deviceType);
    if (!appOperationStr.empty()) {
        want.SetParam(APP_OPERATION, appOperationStr);
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
