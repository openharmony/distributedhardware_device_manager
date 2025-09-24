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
#define LOG_TAG "AniDMUtils"
#include "ani_dm_utils.h"
#include "ani_utils.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_constants.h"
#include "dm_anonymous.h"

namespace ani_dmutils {

using namespace OHOS::DistributedHardware;

void InsertMapParames(JsonObject &bindParamObj, std::map<std::string, std::string> &bindParamMap)
{
    if (IsInt32(bindParamObj, AUTH_TYPE)) {
        int32_t authType = bindParamObj[AUTH_TYPE].Get<int32_t>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_AUTH_TYPE, std::to_string(authType)));
    }
    if (IsString(bindParamObj, APP_OPERATION)) {
        std::string appOperation = bindParamObj[APP_OPERATION].Get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_APP_OPER, appOperation));
    }
    if (IsString(bindParamObj, CUSTOM_DESCRIPTION)) {
        std::string appDescription = bindParamObj[CUSTOM_DESCRIPTION].Get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_APP_DESC, appDescription));
    }
    if (IsString(bindParamObj, PARAM_KEY_TARGET_PKG_NAME)) {
        std::string targetPkgName = bindParamObj[PARAM_KEY_TARGET_PKG_NAME].Get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_TARGET_PKG_NAME, targetPkgName));
    }
    if (IsString(bindParamObj, PARAM_KEY_META_TYPE)) {
        std::string metaType = bindParamObj[PARAM_KEY_META_TYPE].Get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_META_TYPE, metaType));
    }
    if (IsString(bindParamObj, PARAM_KEY_PIN_CODE)) {
        std::string pinCode = bindParamObj[PARAM_KEY_PIN_CODE].Get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_PIN_CODE, pinCode));
    }
    if (IsString(bindParamObj, PARAM_KEY_AUTH_TOKEN)) {
        std::string authToken = bindParamObj[PARAM_KEY_AUTH_TOKEN].Get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_AUTH_TOKEN, authToken));
    }
    if (IsString(bindParamObj, PARAM_KEY_IS_SHOW_TRUST_DIALOG)) {
        std::string isShowTrustDialog = bindParamObj[PARAM_KEY_IS_SHOW_TRUST_DIALOG].Get<std::string>();
        bindParamMap.insert(std::pair<std::string, std::string>(PARAM_KEY_IS_SHOW_TRUST_DIALOG, isShowTrustDialog));
    }
    if (IsInt32(bindParamObj, BIND_LEVEL)) {
        int32_t bindLevel = bindParamObj[BIND_LEVEL].Get<std::int32_t>();
        bindParamMap.insert(std::pair<std::string, std::string>(BIND_LEVEL, std::to_string(bindLevel)));
    }
}

std::string GetDeviceTypeById(DmDeviceType type)
{
    const static std::pair<DmDeviceType, std::string> mapArray[] = {
        {DEVICE_TYPE_UNKNOWN, DEVICE_TYPE_UNKNOWN_STRING},
        {DEVICE_TYPE_PHONE, DEVICE_TYPE_PHONE_STRING},
        {DEVICE_TYPE_PAD, DEVICE_TYPE_PAD_STRING},
        {DEVICE_TYPE_TV, DEVICE_TYPE_TV_STRING},
        {DEVICE_TYPE_CAR, DEVICE_TYPE_CAR_STRING},
        {DEVICE_TYPE_WATCH, DEVICE_TYPE_WATCH_STRING},
        {DEVICE_TYPE_WIFI_CAMERA, DEVICE_TYPE_WIFICAMERA_STRING},
        {DEVICE_TYPE_PC, DEVICE_TYPE_PC_STRING},
        {DEVICE_TYPE_SMART_DISPLAY, DEVICE_TYPE_SMART_DISPLAY_STRING},
        {DEVICE_TYPE_2IN1, DEVICE_TYPE_2IN1_STRING},
    };
    for (const auto& item : mapArray) {
        if (item.first == type) {
            return item.second;
        }
    }
    return DEVICE_TYPE_UNKNOWN_STRING;
}

} //namespace ani_utils