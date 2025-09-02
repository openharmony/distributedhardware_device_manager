/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "dm_dialog_manager.h"

#include <pthread.h>
#include <thread>

#include "ability_manager_client.h"
#include "auth_message_processor.h"
#include "dm_anonymous.h"
#include "dm_log.h"
#include "dm_crypto.h"
#include "json_object.h"
#include "parameter.h"
#include "dm_single_instance.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t INVALID_USERID = -1;
constexpr int32_t MESSAGE_PARCEL_KEY_SIZE = 3;
constexpr int32_t WINDOW_LEVEL_UPPER = 2;
constexpr int32_t WINDOW_LEVEL_DEFAULT = 1;
constexpr const char* CONNECT_PIN_DIALOG = "pinDialog";
constexpr const char* DM_UI_BUNDLE_NAME = "com.ohos.devicemanagerui";
constexpr const char* CONFIRM_ABILITY_NAME = "com.ohos.devicemanagerui.ConfirmUIExtAbility";
constexpr const char* PIN_ABILITY_NAME = "com.ohos.devicemanagerui.PincodeUIExtAbility";
constexpr const char* INPUT_ABILITY_NAME = "com.ohos.devicemanagerui.InputUIExtAbility";
constexpr uint32_t CLOSE_DIALOG_CMD_CODE = 3;
constexpr uint32_t SHOW_DIALOG_CMD_CODE = 1;
}
DM_IMPLEMENT_SINGLE_INSTANCE(DmDialogManager);

DmDialogManager::DmDialogManager()
{
    LOGI("DmDialogManager constructor");
}

DmDialogManager::~DmDialogManager()
{
    LOGI("DmDialogManager destructor");
}

void DmDialogManager::ShowConfirmDialog(const std::string param)
{
    std::string deviceName = "";
    std::string appOperationStr = "";
    std::string customDescriptionStr = "";
    std::string hostPkgLabel = "";
    int32_t deviceType = -1;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        JsonObject jsonObject(param);
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
            if (IsString(jsonObject, TAG_HOST_PKGLABEL)) {
                hostPkgLabel = jsonObject[TAG_HOST_PKGLABEL].Get<std::string>();
            }
            if (IsBool(jsonObject, PARAM_KEY_IS_PROXY_BIND)) {
                isProxyBind_ = jsonObject[PARAM_KEY_IS_PROXY_BIND].Get<bool>();
            }
            if (IsString(jsonObject, APP_USER_DATA)) {
                appUserData_ = jsonObject[APP_USER_DATA].Get<std::string>();
            }
            if (IsString(jsonObject, TITLE)) {
                title_ = jsonObject[TITLE].Get<std::string>();
            }
        }

        bundleName_ = DM_UI_BUNDLE_NAME;
        abilityName_ = CONFIRM_ABILITY_NAME;
        deviceName_ = deviceName;
        appOperationStr_ = appOperationStr;
        customDescriptionStr_ = customDescriptionStr;
        deviceType_ = deviceType;
        hostPkgLabel_ = hostPkgLabel;
    }
    ConnectExtension();
}

void DmDialogManager::ShowPinDialog(const std::string param)
{
    LOGI("pinCode: %{public}s", GetAnonyString(param).c_str());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        bundleName_ = DM_UI_BUNDLE_NAME;
        abilityName_ = PIN_ABILITY_NAME;
        pinCode_ = param;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([]() { DmDialogManager::GetInstance().ConnectExtension(); });
#else
    std::thread pinDilog([]() { DmDialogManager::GetInstance().ConnectExtension(); });
    int32_t ret = pthread_setname_np(pinDilog.native_handle(), CONNECT_PIN_DIALOG.c_str());
    if (ret != DM_OK) {
        LOGE("pinDilog setname failed.");
    }
    pinDilog.detach();
#endif
}

void DmDialogManager::ShowInputDialog(const std::string param)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        targetDeviceName_ = param;
        bundleName_ = DM_UI_BUNDLE_NAME;
        abilityName_ = INPUT_ABILITY_NAME;
    }
    ConnectExtension();
}

void DmDialogManager::CloseDialog()
{
    LOGI("In");
    std::unique_lock<std::mutex> lock(mutex_);
    if (g_remoteObject == nullptr) {
        LOGW("g_remoteObject is nullptr");
        isCloseDialog_.store(true);
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = g_remoteObject->SendRequest(CLOSE_DIALOG_CMD_CODE, data, reply, option);
    if (ret != ERR_OK) {
        LOGE("close dm dialog is failed: %{public}d", ret);
        return;
    }
}

void DmDialogManager::ConnectExtension()
{
    LOGI("DmDialogManager::ConnectExtension start.");
    isCloseDialog_.store(false);
    if (isConnectSystemUI_.load() && dialogConnectionCallback_ != nullptr && g_remoteObject != nullptr) {
        AppExecFwk::ElementName element;
        OnAbilityConnectDone(element, g_remoteObject, INVALID_USERID);
        LOGI("DmDialogManager::ConnectExtension dialog has been show.");
        return;
    }
    AAFwk::Want want;
    std::string bundleName = "com.ohos.sceneboard";
    std::string abilityName = "com.ohos.sceneboard.systemdialog";
    want.SetElementName(bundleName, abilityName);
    auto abilityManager = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityManager == nullptr) {
        LOGE("AbilityManagerClient is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (dialogConnectionCallback_ == nullptr) {
        dialogConnectionCallback_ = new (std::nothrow) DialogAbilityConnection();
    }
    if (dialogConnectionCallback_ == nullptr) {
        LOGE("create dialogConnectionCallback_ failed.");
        return;
    }
    LOGI("DmDialogManager::ConnectExtension abilityManager ConnectAbility begin.");
    auto ret = abilityManager->ConnectAbility(want, dialogConnectionCallback_, INVALID_USERID);
    if (ret != ERR_OK) {
        LOGE("ConnectExtensionAbility sceneboard failed.");
        bundleName = "com.ohos.systemui";
        abilityName = "com.ohos.systemui.dialog";
        want.SetElementName(bundleName, abilityName);
        ret = abilityManager->ConnectAbility(want, dialogConnectionCallback_, INVALID_USERID);
        if (ret != ERR_OK) {
            LOGE("ConnectExtensionAbility systemui failed again.");
        }
    }
}

void DmDialogManager::OnAbilityConnectDone(
    const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    LOGI("OnAbilityConnectDone");
    std::unique_lock<std::mutex> lock(mutex_);
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return;
    }
    if (g_remoteObject == nullptr) {
        g_remoteObject = remoteObject;
    }
    isConnectSystemUI_.store(true);
    if (isCloseDialog_.load()) {
        LOGW("isCloseDialog_  is true");
        isCloseDialog_.store(false);
        return;
    }
    SendMsgRequest(remoteObject);
}

void DmDialogManager::SendMsgRequest(const sptr<IRemoteObject>& remoteObject)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(MESSAGE_PARCEL_KEY_SIZE);
    data.WriteString16(u"bundleName");
    data.WriteString16(Str8ToStr16(bundleName_));
    data.WriteString16(u"abilityName");
    data.WriteString16(Str8ToStr16(abilityName_));
    data.WriteString16(u"parameters");
    JsonObject param;
    param["ability.want.params.uiExtensionType"] = "sysDialog/common";
    param["sysDialogZOrder"] = WINDOW_LEVEL_DEFAULT;
    if (abilityName_ == INPUT_ABILITY_NAME) {
        param["sysDialogZOrder"] = WINDOW_LEVEL_UPPER;
    }
    std::string pinCodeHash = GetAnonyString(Crypto::Sha256(pinCode_));
    LOGI("OnAbilityConnectDone pinCodeHash: %{public}s", pinCodeHash.c_str());
    param["isProxyBind"] = isProxyBind_;
    param["appUserData"] = appUserData_;
    param["title"] = title_;
    param["pinCode"] = pinCode_;
    param["deviceName"] = deviceName_;
    param["appOperationStr"] = appOperationStr_;
    param["customDescriptionStr"] = customDescriptionStr_;
    param["deviceType"] = deviceType_;
    param[TAG_TARGET_DEVICE_NAME] = targetDeviceName_;
    param[TAG_HOST_PKGLABEL] = hostPkgLabel_;
    param["disableUpGesture"] = 1;
    std::string paramStr = param.Dump();
    data.WriteString16(Str8ToStr16(paramStr));
    LOGI("show dm dialog is begin");
    int32_t ret = remoteObject->SendRequest(SHOW_DIALOG_CMD_CODE, data, reply, option);
    if (ret != ERR_OK) {
        LOGE("show dm dialog is failed: %{public}d", ret);
        return;
    }
    LOGI("show dm dialog is success");
}

void DmDialogManager::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode)
{
    LOGI("OnAbilityDisconnectDone");
    std::lock_guard<std::mutex> lock(mutex_);
    g_remoteObject = nullptr;
    isConnectSystemUI_.store(false);
    isCloseDialog_.store(false);
}

void DmDialogManager::DialogAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    DmDialogManager::GetInstance().OnAbilityConnectDone(element, remoteObject, resultCode);
}

void DmDialogManager::DialogAbilityConnection::OnAbilityDisconnectDone(
    const AppExecFwk::ElementName& element, int resultCode)
{
    DmDialogManager::GetInstance().OnAbilityDisconnectDone(element, resultCode);
}
}
}