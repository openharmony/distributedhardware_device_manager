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
#include "ani_error_utils.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_constants.h"
#include "dm_anonymous.h"
#include "json_object.h"

namespace ani_dmutils {

using namespace OHOS::DistributedHardware;

const int32_t DM_NAPI_BUF_LENGTH = 256;
const int32_t MAX_JSON_LEN = 1024 * 1024;

bool CheckJsParamStringValid(const std::string &param)
{
    if (param.size() == 0 || param.size() >= DM_NAPI_BUF_LENGTH) {
        ani_errorutils::CreateBusinessError(ERR_DM_INPUT_PARA_INVALID);
        return false;
    }
    return true;
}

void InsertMapParams(JsonObject &bindParamObj, std::map<std::string, std::string> &bindParamMap)
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

void ServiceProfileInfoToNative(const ::ohos::distributedDeviceManager::ServiceProfileInfo &taiheInfo,
    OHOS::DistributedHardware::DmServiceProfileInfo& nativeInfo)
{
    nativeInfo.deviceId = std::string(taiheInfo.deviceId);
    nativeInfo.serviceId = std::string(taiheInfo.serviceId);
    nativeInfo.serviceType = std::string(taiheInfo.serviceType);
    if (taiheInfo.data.has_value() && taiheInfo.data.value().size() > MAX_JSON_LEN) {
        LOGW("taiheInfo.data size=%{public}zu more than limit", taiheInfo.data.value().size());
        return;
    }
    if (taiheInfo.data.has_value() && taiheInfo.data.value().size() > 0) {
        auto dataStr = taiheInfo.data.value();
        OHOS::DistributedHardware::ParseMapFromJsonString(std::string(dataStr), nativeInfo.data);
    }
}

void ServiceProfileInfoArrayToNative(
    const ::taihe::array_view<::ohos::distributedDeviceManager::ServiceProfileInfo> &taiheList,
    std::vector<OHOS::DistributedHardware::DmServiceProfileInfo>& nativeList)
{
    for (auto it = taiheList.begin(); it != taiheList.end(); ++it) {
        auto const &value = *it;
        OHOS::DistributedHardware::DmServiceProfileInfo dmInfo = {};
        ServiceProfileInfoToNative(value, dmInfo);
        nativeList.emplace_back(dmInfo);
    }
}

void DeviceProfileInfoToNative(const ::ohos::distributedDeviceManager::DeviceProfileInfo &taiheInfo,
    OHOS::DistributedHardware::DmDeviceProfileInfo& nativeInfo)
{
    nativeInfo.deviceId = std::string(taiheInfo.deviceId);
    nativeInfo.deviceSn = std::string(taiheInfo.deviceSn);
    nativeInfo.mac = std::string(taiheInfo.mac);
    nativeInfo.model = std::string(taiheInfo.model);
    if (taiheInfo.internalModel.has_value()) {
        nativeInfo.internalModel = std::string(taiheInfo.internalModel.value());
    }
    nativeInfo.deviceType = std::string(taiheInfo.deviceType);
    nativeInfo.manufacturer = std::string(taiheInfo.manufacturer);
    nativeInfo.deviceName = std::string(taiheInfo.deviceName);
    if (taiheInfo.productName.has_value()) {
        nativeInfo.productName = std::string(taiheInfo.productName.value());
    }
    nativeInfo.productId = std::string(taiheInfo.productId);
    if (taiheInfo.subProductId.has_value()) {
        nativeInfo.subProductId = std::string(taiheInfo.subProductId.value());
    }
    nativeInfo.sdkVersion = std::string(taiheInfo.sdkVersion);
    nativeInfo.bleMac = std::string(taiheInfo.bleMac);
    nativeInfo.brMac = std::string(taiheInfo.brMac);
    nativeInfo.sleMac = std::string(taiheInfo.sleMac);
    nativeInfo.firmwareVersion = std::string(taiheInfo.firmwareVersion);
    nativeInfo.hardwareVersion = std::string(taiheInfo.hardwareVersion);
    nativeInfo.softwareVersion = std::string(taiheInfo.softwareVersion);
    nativeInfo.protocolType = taiheInfo.protocolType;
    nativeInfo.setupType = taiheInfo.setupType;
    nativeInfo.wiseDeviceId = std::string(taiheInfo.wiseDeviceId);
    nativeInfo.wiseUserId = std::string(taiheInfo.wiseUserId);
    nativeInfo.registerTime = std::string(taiheInfo.registerTime);
    nativeInfo.modifyTime = std::string(taiheInfo.modifyTime);
    nativeInfo.shareTime = std::string(taiheInfo.shareTime);
    nativeInfo.isLocalDevice = taiheInfo.isLocalDevice;
    if (taiheInfo.services.has_value()) {
        ServiceProfileInfoArrayToNative(taiheInfo.services.value(), nativeInfo.services);
    }
}

void DeviceProfileInfoArrayToNative(
    const ::taihe::array_view<::ohos::distributedDeviceManager::DeviceProfileInfo> &taiheList,
    std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo>& nativeList)
{
    for (auto it = taiheList.begin(); it != taiheList.end(); ++it) {
        auto const &value = *it;
        OHOS::DistributedHardware::DmDeviceProfileInfo dmInfo = {};
        DeviceProfileInfoToNative(value, dmInfo);
        nativeList.emplace_back(dmInfo);
    }
}

ani_object ServiceProfileInfoToAni(ani_env* env, const OHOS::DistributedHardware::DmServiceProfileInfo &nativeObj)
{
    if (env == nullptr) {
        return {};
    }
    ani_string ani_field_deviceId = ani_utils::AniCreateString(env, nativeObj.deviceId);
    ani_string ani_field_serviceId = ani_utils::AniCreateString(env, nativeObj.serviceId);
    ani_string ani_field_serviceType = ani_utils::AniCreateString(env, nativeObj.serviceType);
    ani_ref ani_field_data = {};
    std::string jsonDataStr;
    if (nativeObj.data.size() > 0) {
        jsonDataStr = OHOS::DistributedHardware::ConvertMapToJsonString(nativeObj.data);
    }
    if (jsonDataStr.size() > 0) {
        ani_field_data = ani_utils::AniCreateString(env, jsonDataStr);
    } else {
        env->GetUndefined(&ani_field_data);
    }
    ani_class cls = ani_utils::AniGetClass(env,
        "@ohos.distributedDeviceManager.distributedDeviceManager._taihe_ServiceProfileInfo_inner");
    ani_method method = ani_utils::AniGetClassMethod(env,
        "@ohos.distributedDeviceManager.distributedDeviceManager._taihe_ServiceProfileInfo_inner", "<ctor>", nullptr);
    ani_object ani_obj = {};
    ani_status status = env->Object_New(cls, method,
        &ani_obj, ani_field_deviceId, ani_field_serviceId, ani_field_serviceType, ani_field_data);
    if (status != ANI_OK) {
        LOGE("Failed to create ServiceProfileInfo object: %{public}d", status);
    }
    return ani_obj;
}

ani_object ServiceProfileInfoArrayToAni(ani_env* env,
    const std::vector<OHOS::DistributedHardware::DmServiceProfileInfo> &services)
{
    if (env == nullptr) {
        return {};
    }
    size_t ani_field_services_size = services.size();
    ani_array ani_field_services = {};
    ani_ref aniUndefine = {};
    env->GetUndefined(&aniUndefine);
    env->Array_New(ani_field_services_size, aniUndefine, &ani_field_services);
    for (size_t index = 0; index < ani_field_services_size; index++) {
        ani_object aniService = ServiceProfileInfoToAni(env, services[index]);
        env->Array_Set(ani_field_services, index, aniService);
    }
    return ani_field_services;
}

ani_object DeviceProfileInfoToAni(ani_env* env, const OHOS::DistributedHardware::DmDeviceProfileInfo &nativeObj)
{
    if (env == nullptr) {
        return {};
    }
    ani_string ani_field_deviceId = ani_utils::AniCreateString(env, nativeObj.deviceId);
    ani_string ani_field_deviceSn = ani_utils::AniCreateString(env, nativeObj.deviceSn);
    ani_string ani_field_mac = ani_utils::AniCreateString(env, nativeObj.mac);
    ani_string ani_field_model = ani_utils::AniCreateString(env, nativeObj.model);
    ani_string ani_field_deviceType = ani_utils::AniCreateString(env, nativeObj.deviceType);
    ani_string ani_field_manufacturer = ani_utils::AniCreateString(env, nativeObj.manufacturer);
    ani_string ani_field_deviceName = ani_utils::AniCreateString(env, nativeObj.deviceName);
    ani_string ani_field_productId = ani_utils::AniCreateString(env, nativeObj.productId);
    ani_ref ani_field_subProductId = ani_utils::AniCreateString(env, nativeObj.subProductId);
    ani_string ani_field_sdkVersion = ani_utils::AniCreateString(env, nativeObj.sdkVersion);
    ani_string ani_field_bleMac = ani_utils::AniCreateString(env, nativeObj.bleMac);
    ani_string ani_field_brMac = ani_utils::AniCreateString(env, nativeObj.brMac);
    ani_string ani_field_sleMac = ani_utils::AniCreateString(env, nativeObj.sleMac);
    ani_string ani_field_firmwareVersion = ani_utils::AniCreateString(env, nativeObj.firmwareVersion);
    ani_string ani_field_hardwareVersion = ani_utils::AniCreateString(env, nativeObj.hardwareVersion);
    ani_string ani_field_softwareVersion = ani_utils::AniCreateString(env, nativeObj.softwareVersion);
    ani_int ani_field_protocolType = static_cast<ani_int>(nativeObj.protocolType);
    ani_int ani_field_setupType = static_cast<ani_int>(nativeObj.setupType);
    ani_string ani_field_wiseDeviceId = ani_utils::AniCreateString(env, nativeObj.wiseDeviceId);
    ani_string ani_field_wiseUserId = ani_utils::AniCreateString(env, nativeObj.wiseUserId);
    ani_string ani_field_registerTime = ani_utils::AniCreateString(env, nativeObj.registerTime);
    ani_string ani_field_modifyTime = ani_utils::AniCreateString(env, nativeObj.modifyTime);
    ani_string ani_field_shareTime = ani_utils::AniCreateString(env, nativeObj.shareTime);
    ani_boolean ani_field_isLocalDevice = static_cast<ani_boolean>(nativeObj.isLocalDevice);
    ani_ref ani_field_services = {};
    if (nativeObj.services.size() == 0) {
        env->GetUndefined(&ani_field_services);
    } else {
        ani_field_services = ServiceProfileInfoArrayToAni(env, nativeObj.services);
    }
    ani_ref ani_field_productName = ani_utils::AniCreateString(env, nativeObj.productName);
    ani_ref ani_field_internalModel = ani_utils::AniCreateString(env, nativeObj.internalModel);
    ani_class cls = ani_utils::AniGetClass(env,
        "@ohos.distributedDeviceManager.distributedDeviceManager._taihe_DeviceProfileInfo_inner");
    ani_method method = ani_utils::AniGetClassMethod(env,
        "@ohos.distributedDeviceManager.distributedDeviceManager._taihe_DeviceProfileInfo_inner", "<ctor>", nullptr);
    ani_object ani_obj = {};
    CREATE_DEVICE_PROFILE_INFO_RET;
    return ani_obj;
}

ani_object DeviceProfileInfoArrayToAni(ani_env* env,
    const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &nativeList)
{
    std::vector<ani_object> aniArray;
    for (auto &item : nativeList) {
        ani_object ret = ani_dmutils::DeviceProfileInfoToAni(env, item);
        aniArray.emplace_back(ret);
    }
    return ani_utils::AniCreateArray(env, aniArray);
}

ani_object DeviceIconInfoToAni(ani_env* env, const OHOS::DistributedHardware::DmDeviceIconInfo &nativeObj)
{
    if (env == nullptr) {
        return {};
    }
    ani_string ani_field_productId = ani_utils::AniCreateString(env, nativeObj.productId);
    ani_ref ani_field_subProductId = ani_utils::AniCreateString(env, nativeObj.subProductId);
    ani_string ani_field_imageType = ani_utils::AniCreateString(env, nativeObj.imageType);
    ani_string ani_field_specName = ani_utils::AniCreateString(env, nativeObj.specName);
    ani_string ani_field_url = ani_utils::AniCreateString(env, nativeObj.url);
    void* ani_field_icon_ani_data = {};
    ani_arraybuffer ani_field_icon = {};
    ani_status status = env->CreateArrayBuffer(nativeObj.icon.size(), &ani_field_icon_ani_data, &ani_field_icon);
    if (status != ANI_OK || ani_field_icon_ani_data == nullptr) {
        LOGE("Failed to CreateArrayBuffer: %{public}d", status);
        return {};
    }
    std::copy(nativeObj.icon.begin(), nativeObj.icon.end(), reinterpret_cast<uint8_t*>(ani_field_icon_ani_data));
    ani_ref ani_field_internalModel = ani_utils::AniCreateString(env, nativeObj.internalModel);
    ani_class cls = ani_utils::AniGetClass(env,
        "@ohos.distributedDeviceManager.distributedDeviceManager._taihe_DeviceIconInfo_inner");
    ani_method method = ani_utils::AniGetClassMethod(env,
        "@ohos.distributedDeviceManager.distributedDeviceManager._taihe_DeviceIconInfo_inner", "<ctor>", nullptr);
    ani_object ani_obj = {};
    status = env->Object_New(cls, method, &ani_obj,
        ani_field_productId, ani_field_subProductId, ani_field_imageType, ani_field_specName,
        ani_field_url, ani_field_icon, ani_field_internalModel);
    if (status != ANI_OK) {
        LOGE("Failed to create DeviceIconInfo object: %{public}d", status);
    }
    return ani_obj;
}

} //namespace ani_utils