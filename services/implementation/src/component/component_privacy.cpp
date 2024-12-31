/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "component_privacy.h"

#include "ability_manager_client.h"
#include "constants.h"
#include "component_loader.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "device_manager.h"
#include "dm_device_info.h"
#include "device_type.h"
#include "dh_utils_tool.h"
#include "event_handler.h"
#include "cJSON.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentPrivacy"

ComponentPrivacy::ComponentPrivacy()
{
    DHLOGI("ComponentPrivacy ctor.");
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<ComponentPrivacy::ComponentEventHandler>(runner, this);
}

ComponentPrivacy::~ComponentPrivacy()
{
    DHLOGI("ComponentPrivacy dtor.");
}

int32_t ComponentPrivacy::OnPrivaceResourceMessage(const ResourceEventType &type, const std::string &subtype,
    const std::string &networkId, bool &isSensitive, bool &isSameAccout)
{
    DHLOGI("OnPrivaceResourceMessage start.");
    int32_t ret = DH_FWK_SUCCESS;
    if (type == ResourceEventType::EVENT_TYPE_QUERY_RESOURCE) {
        ret = OnResourceInfoCallback(subtype, networkId, isSensitive, isSameAccout);
    } else if (type == ResourceEventType::EVENT_TYPE_PULL_UP_PAGE) {
        HandlePullUpPage(subtype, networkId);
    } else if (type == ResourceEventType::EVENT_TYPE_CLOSE_PAGE) {
        HandleClosePage(subtype);
    }
    return ret;
}

void ComponentPrivacy::HandlePullUpPage(const std::string &subtype, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return;
    }
    cJSON *jsonArrayMsg = cJSON_CreateArray();
    if (jsonArrayMsg == NULL) {
        DHLOGE("Failed to create cJSON arrary.");
        return;
    }

    cJSON *tmpJson = cJSON_CreateObject();
    if (tmpJson == NULL) {
        cJSON_Delete(jsonArrayMsg);
        DHLOGE("Failed to create cJSON object.");
        return;
    }
    if (eventHandler_ != nullptr) {
        DHLOGI("SendEvent COMP_START_PAGE");
        cJSON_AddStringToObject(tmpJson, PRIVACY_SUBTYPE.c_str(), subtype.c_str());
        cJSON_AddStringToObject(tmpJson, PRIVACY_NETWORKID.c_str(), networkId.c_str());
        cJSON_AddItemToArray(jsonArrayMsg, tmpJson);

        AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(COMP_START_PAGE,
            std::shared_ptr<cJSON>(jsonArrayMsg, cJSON_Delete), 0);
        eventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        return;
    }
    cJSON_Delete(tmpJson);
    cJSON_Delete(jsonArrayMsg);
}

void ComponentPrivacy::HandleClosePage(const std::string &subtype)
{
    cJSON *jsonArrayMsg = cJSON_CreateArray();
    if (jsonArrayMsg == NULL) {
        DHLOGE("Failed to create cJSON arrary.");
        return;
    }

    cJSON *tmpJson = cJSON_CreateObject();
    if (tmpJson == NULL) {
        cJSON_Delete(jsonArrayMsg);
        DHLOGE("Failed to create cJSON object.");
        return;
    }
    if (eventHandler_ != nullptr) {
        DHLOGI("SendEvent COMP_STOP_PAGE");
        cJSON_AddStringToObject(tmpJson, PRIVACY_SUBTYPE.c_str(), subtype.c_str());
        cJSON_AddItemToArray(jsonArrayMsg, tmpJson);

        AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(COMP_STOP_PAGE,
            std::shared_ptr<cJSON>(jsonArrayMsg, cJSON_Delete), 0);
        eventHandler_->SendEvent(msgEvent, COMP_PRIVACY_DELAY_TIME, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        return;
    }
    cJSON_Delete(tmpJson);
    cJSON_Delete(jsonArrayMsg);
}

int32_t ComponentPrivacy::OnResourceInfoCallback(const std::string &subtype, const std::string &networkId,
    bool &isSensitive, bool &isSameAccout)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("OnResourceInfoCallback start.");
    std::map<std::string, bool> resourceDesc = ComponentLoader::GetInstance().GetCompResourceDesc();
    if (resourceDesc.find(subtype) == resourceDesc.end()) {
        DHLOGE("GetCompResourceDesc failed.");
        return ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY;
    }
    isSensitive = resourceDesc[subtype];
    DmAuthForm authForm = DmAuthForm::INVALID_TYPE;
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        DHLOGE("DeviceList size is invalid!");
        return ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY;
    }
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            authForm = deviceInfo.authForm;
            break;
        }
    }
    if (authForm == DmAuthForm::IDENTICAL_ACCOUNT) {
        isSameAccout = true;
    } else {
        isSameAccout = false;
    }
    return DH_FWK_SUCCESS;
}

int32_t ComponentPrivacy::StartPrivacePage(const std::string &subtype, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("StartPrivacePage start.");
    DmDeviceInfo deviceInfo;
    DeviceManager::GetInstance().GetDeviceInfo(DH_FWK_PKG_NAME, networkId, deviceInfo);
    std::string deviceName = std::string(deviceInfo.deviceName);
    uint16_t deviceTypeId = deviceInfo.deviceTypeId;
    std::string deviceType = DeviceTypeToString(deviceTypeId);
    const std::string bundleName = "com.ohos.dhardwareui";
    const std::string abilityName = "DHardwareUIAbility";
    int32_t type = -1;
    if (subtype == "mic") {
        type = static_cast<int32_t>(DHSubtype::AUDIO_MIC);
    } else if (subtype == "camera") {
        type = static_cast<int32_t>(DHSubtype::CAMERA);
    }
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    want.SetParam("type", type);
    want.SetParam("srcNetworkId", networkId);
    want.SetParam("deviceName", deviceName);
    want.SetParam("deviceType", deviceType);
    auto abilityManager = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManager == nullptr) {
        DHLOGE("AbilityManagerClient is nullptr.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    int32_t result = abilityManager->StartAbility(want);
    DHLOGI("performance time: StartPrivacePage result = %{public}d", result);
    SetPageFlagTrue();
    return result;
}

int32_t ComponentPrivacy::StopPrivacePage(const std::string &subtype)
{
    DHLOGI("StopPrivacePage start.");
    int32_t type = -1;
    if (subtype == "mic") {
        type = static_cast<int32_t>(DHSubtype::AUDIO_MIC);
    } else if (subtype == "camera") {
        type = static_cast<int32_t>(DHSubtype::CAMERA);
    }
    const std::string bundleName = "com.ohos.dhardwareui";
    const std::string abilityName = "DHardwareUIAbility";
    int32_t returnCode = 24200102;
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    want.SetParam("type", type);
    want.SetParam("returnCode", returnCode);
    auto abilityManager = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManager == nullptr) {
        DHLOGE("AbilityManagerClient is nullptr.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    int32_t result = abilityManager->StartAbility(want);
    DHLOGI("performance time: StopPrivacePage result = %{public}d", result);
    SetPageFlagFalse();
    return result;
}

std::string ComponentPrivacy::DeviceTypeToString(uint16_t deviceTypeId)
{
    DHLOGD("DeviceTypeToString start.");
    DmDeviceType deviceType = static_cast<DmDeviceType>(deviceTypeId);
    switch (deviceType) {
        case DmDeviceType::DEVICE_TYPE_WIFI_CAMERA:
            return "camera";
        case DmDeviceType::DEVICE_TYPE_AUDIO:
            return "audio";
        case DmDeviceType::DEVICE_TYPE_PC:
            return "pc";
        case DmDeviceType::DEVICE_TYPE_PHONE:
            return "phone";
        case DmDeviceType::DEVICE_TYPE_PAD:
            return "pad";
        case DmDeviceType::DEVICE_TYPE_WATCH:
            return "watch";
        case DmDeviceType::DEVICE_TYPE_CAR:
            return "car";
        case DmDeviceType::DEVICE_TYPE_TV:
            return "tv";
        case DmDeviceType::DEVICE_TYPE_SMART_DISPLAY:
            return "display";
        case DmDeviceType::DEVICE_TYPE_2IN1:
            return "2in1";
        default:
            return "unknown";
    }
}

void ComponentPrivacy::SetPageFlagTrue()
{
    isPrivacePageOpen_.store(true);
}

void ComponentPrivacy::SetPageFlagFalse()
{
    isPrivacePageOpen_.store(false);
}

bool ComponentPrivacy::GetPageFlag()
{
    return isPrivacePageOpen_.load();
}

void ComponentPrivacy::ComponentEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        DHLOGE("event is nullptr");
        return;
    }
    switch (event->GetInnerEventId()) {
        case COMP_START_PAGE:
            ProcessStartPage(event);
            break;
        case COMP_STOP_PAGE:
            ProcessStopPage(event);
            break;
        default:
            DHLOGE("ComponentEventHandler EventId %{public}d is undefined.", event->GetInnerEventId());
            break;
    }
}

ComponentPrivacy::ComponentEventHandler::ComponentEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> runner, ComponentPrivacy *comPrivacy)
    : AppExecFwk::EventHandler(runner)
{
    comPrivacyObj_ = comPrivacy;
}

ComponentPrivacy::ComponentEventHandler::~ComponentEventHandler()
{
    comPrivacyObj_ = nullptr;
}

void ComponentPrivacy::ComponentEventHandler::ProcessStartPage(const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGI("ProcessStartPage enter.");
    std::shared_ptr<cJSON> dataMsg = event->GetSharedObject<cJSON>();
    cJSON *innerMsg = cJSON_GetArrayItem(dataMsg.get(), 0);
    cJSON *subtypeJson = cJSON_GetObjectItem(innerMsg, PRIVACY_SUBTYPE.c_str());
    if (!IsString(subtypeJson)) {
        DHLOGE("PRIVACY_SUBTYPE is invalid!");
        return;
    }
    std::string subtype = subtypeJson->valuestring;
    cJSON *networkIdJson = cJSON_GetObjectItem(innerMsg, PRIVACY_NETWORKID.c_str());
    if (!IsString(networkIdJson)) {
        DHLOGE("PRIVACY_NETWORKID is invalid!");
        return;
    }
    std::string networkId = networkIdJson->valuestring;
    if (comPrivacyObj_ == nullptr) {
        DHLOGE("comPrivacyObj_ is nullptr");
        return;
    }
    comPrivacyObj_->StartPrivacePage(subtype, networkId);
}

void ComponentPrivacy::ComponentEventHandler::ProcessStopPage(const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGI("ProcessStopPage enter.");
    std::shared_ptr<cJSON> dataMsg = event->GetSharedObject<cJSON>();
    cJSON *innerMsg = cJSON_GetArrayItem(dataMsg.get(), 0);
    if (innerMsg == NULL) {
        DHLOGE("innerMsg is nullptr");
        return;
    }
    cJSON *subtypeJson = cJSON_GetObjectItem(innerMsg, PRIVACY_SUBTYPE.c_str());
    if (!IsString(subtypeJson)) {
        DHLOGE("PRIVACY_SUBTYPE is invalid!");
        return;
    }
    std::string subtype = subtypeJson->valuestring;
    if (comPrivacyObj_ == nullptr) {
        DHLOGE("comPrivacyObj_ is nullptr");
        return;
    }
    comPrivacyObj_->StopPrivacePage(subtype);
}
} // namespace DistributedHardware
} // namespace OHOS
