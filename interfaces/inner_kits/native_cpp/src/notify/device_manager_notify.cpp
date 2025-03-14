/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "device_manager_notify.h"
#include <thread>
#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_device_info.h"
#include "dm_log.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ipc_model_codec.h"
#endif

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr const char* UK_SEPARATOR = "#";
constexpr int32_t MAX_CONTAINER_SIZE = 500;
}
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerNotify);
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
constexpr const char* DEVICE_ONLINE = "deviceOnline";
constexpr const char* DEVICE_OFFLINE = "deviceOffline";
constexpr const char* DEVICEINFO_CHANGE = "deviceInfoChange";
constexpr const char* DEVICE_READY = "deviceReady";
constexpr const char* DEVICE_TRUST_CHANGE = "deviceTrustChange";
#endif
const uint16_t DM_INVALID_FLAG_ID = 0;
void DeviceManagerNotify::RegisterDeathRecipientCallback(const std::string &pkgName,
                                                         std::shared_ptr<DmInitCallback> dmInitCallback)
{
    if (pkgName.empty() || dmInitCallback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    dmInitCallback_[pkgName] = dmInitCallback;
}

void DeviceManagerNotify::UnRegisterDeathRecipientCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    dmInitCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDeviceStateCallback(const std::string &pkgName,
                                                      std::shared_ptr<DeviceStateCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterDeviceStateCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_.erase(pkgName);
}

void DeviceManagerNotify::UnRegisterDeviceStatusCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStatusCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDeviceStatusCallback(const std::string &pkgName,
    std::shared_ptr<DeviceStatusCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStatusCallback_[pkgName] = callback;
}

void DeviceManagerNotify::RegisterDiscoveryCallback(const std::string &pkgName, uint16_t subscribeId,
                                                    std::shared_ptr<DiscoveryCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
        deviceDiscoveryCallbacks_[pkgName] = std::map<uint16_t, std::shared_ptr<DiscoveryCallback>>();
    }
    deviceDiscoveryCallbacks_[pkgName][subscribeId] = callback;
}

void DeviceManagerNotify::UnRegisterDiscoveryCallback(const std::string &pkgName, uint16_t subscribeId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoveryCallbacks_.count(pkgName) > 0) {
        deviceDiscoveryCallbacks_[pkgName].erase(subscribeId);
        if (deviceDiscoveryCallbacks_[pkgName].empty()) {
            deviceDiscoveryCallbacks_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::RegisterPublishCallback(const std::string &pkgName,
                                                  int32_t publishId,
                                                  std::shared_ptr<PublishCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (devicePublishCallbacks_.count(pkgName) == 0) {
        devicePublishCallbacks_[pkgName] = std::map<int32_t, std::shared_ptr<PublishCallback>>();
    }
    devicePublishCallbacks_[pkgName][publishId] = callback;
}

void DeviceManagerNotify::UnRegisterPublishCallback(const std::string &pkgName, int32_t publishId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (devicePublishCallbacks_.count(pkgName) > 0) {
        devicePublishCallbacks_[pkgName].erase(publishId);
        if (devicePublishCallbacks_[pkgName].empty()) {
            devicePublishCallbacks_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::RegisterAuthenticateCallback(const std::string &pkgName, const std::string &deviceId,
                                                       std::shared_ptr<AuthenticateCallback> callback)
{
    if (pkgName.empty() || deviceId.empty() || callback == nullptr) {
        LOGE("DeviceManagerNotify::RegisterAuthenticateCallback error: Invalid parameter, pkgName: %{public}s,"
            "deviceId: %{public}s", pkgName.c_str(), GetAnonyString(deviceId).c_str());
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (authenticateCallback_.count(pkgName) == 0) {
        authenticateCallback_[pkgName] = std::map<std::string, std::shared_ptr<AuthenticateCallback>>();
    }
    authenticateCallback_[pkgName][deviceId] = callback;
}

void DeviceManagerNotify::UnRegisterAuthenticateCallback(const std::string &pkgName, const std::string &deviceId)
{
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerNotify::UnRegisterAuthenticateCallback error: Invalid parameter, pkgName: %{public}s,"
            "deviceId: %{public}s", pkgName.c_str(), GetAnonyString(deviceId).c_str());
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (authenticateCallback_.count(pkgName) > 0) {
        authenticateCallback_[pkgName].erase(deviceId);
        if (authenticateCallback_[pkgName].empty()) {
            authenticateCallback_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::UnRegisterPackageCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_.erase(pkgName);
    deviceStatusCallback_.erase(pkgName);
    devicePublishCallbacks_.erase(pkgName);
    authenticateCallback_.erase(pkgName);
    dmInitCallback_.erase(pkgName);
    for (auto it = deviceDiscoveryCallbacks_.begin(); it != deviceDiscoveryCallbacks_.end();) {
        if (it->first.find(pkgName) != std::string::npos) {
            it = deviceDiscoveryCallbacks_.erase(it);
        } else {
            ++it;
        }
    }
}

void DeviceManagerNotify::RegisterDeviceManagerFaCallback(const std::string &pkgName,
                                                          std::shared_ptr<DeviceManagerUiCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmUiCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterDeviceManagerFaCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    dmUiCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterCredentialCallback(const std::string &pkgName,
                                                     std::shared_ptr<CredentialCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    credentialCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    credentialCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterPinHolderCallback(const std::string &pkgName,
    std::shared_ptr<PinHolderCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    pinHolderCallback_[pkgName] = callback;
}

void DeviceManagerNotify::OnRemoteDied()
{
    LOGW("DeviceManagerNotify::OnRemoteDied");
    std::map<std::string, std::shared_ptr<DmInitCallback>> dmInitCallback = GetDmInitCallback();
    for (auto iter : dmInitCallback) {
        LOGI("DeviceManagerNotify::OnRemoteDied, pkgName:%{public}s", iter.first.c_str());
        if (iter.second != nullptr) {
            iter.second->OnRemoteDied();
        }
    }
}

void DeviceManagerNotify::OnDeviceOnline(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("Online with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStateCallback_.find(pkgName);
        if (iter == deviceStateCallback_.end()) {
            LOGE("OnDeviceOnline error, device state callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceOnline error, registered device state callback is nullptr.");
        return;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceInfoOnline(deviceInfo, tempCbk); });
#else
    std::thread deviceOnline([=]() { DeviceInfoOnline(deviceInfo, tempCbk); });
    if (pthread_setname_np(deviceOnline.native_handle(), DEVICE_ONLINE) != DM_OK) {
        LOGE("DeviceInfoOnline set name failed.");
    }
    deviceOnline.detach();
#endif
}

void DeviceManagerNotify::OnDeviceOnline(const std::string &pkgName, const DmDeviceBasicInfo &deviceBasicInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("Online with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStatusCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStatusCallback_.find(pkgName);
        if (iter == deviceStatusCallback_.end()) {
            LOGE("Error, device status callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("Error, registered device status callback is nullptr.");
        return;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceBasicInfoOnline(deviceBasicInfo, tempCbk); });
#else
    std::thread deviceOnline([=]() { DeviceBasicInfoOnline(deviceBasicInfo, tempCbk); });
    if (pthread_setname_np(deviceOnline.native_handle(), DEVICE_ONLINE) != DM_OK) {
        LOGE("DeviceInfoOnline set name failed.");
    }
    deviceOnline.detach();
#endif
}

void DeviceManagerNotify::OnDeviceOffline(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("Offline with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStateCallback_.find(pkgName);
        if (iter == deviceStateCallback_.end()) {
            LOGE("Error, device state callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("Error, registered device state callback is nullptr.");
        return;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceInfoOffline(deviceInfo, tempCbk); });
    LOGI("Completed, Offline with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
#else
    std::thread deviceOffline([=]() { DeviceInfoOffline(deviceInfo, tempCbk); });
    if (pthread_setname_np(deviceOffline.native_handle(), DEVICE_OFFLINE) != DM_OK) {
        LOGE("DeviceInfoOffline set name failed.");
    }
    deviceOffline.detach();
#endif
}

void DeviceManagerNotify::OnDeviceOffline(const std::string &pkgName, const DmDeviceBasicInfo &deviceBasicInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("Offline with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStatusCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStatusCallback_.find(pkgName);
        if (iter == deviceStatusCallback_.end()) {
            LOGE("Error, device status callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("Error, registered device status callback is nullptr.");
        return;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceBasicInfoOffline(deviceBasicInfo, tempCbk); });
#else
    std::thread deviceOffline([=]() { DeviceBasicInfoOffline(deviceBasicInfo, tempCbk); });
    if (pthread_setname_np(deviceOffline.native_handle(), DEVICE_OFFLINE) != DM_OK) {
        LOGE("DeviceInfoOffline set name failed.");
    }
    deviceOffline.detach();
#endif
}

void DeviceManagerNotify::OnDeviceChanged(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }

    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStateCallback_.find(pkgName);
        if (iter == deviceStateCallback_.end()) {
            LOGE("OnDeviceChanged error, device state callback not register, pkgName:%{public}s", pkgName.c_str());
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceChanged error, registered device state callback is nullptr, pkgName:%{public}s", pkgName.c_str());
        return;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceInfoChanged(deviceInfo, tempCbk); });
#else
    std::thread deviceChanged([=]() { DeviceInfoChanged(deviceInfo, tempCbk); });
    if (pthread_setname_np(deviceChanged.native_handle(), DEVICEINFO_CHANGE) != DM_OK) {
        LOGE("deviceChanged set name failed.");
    }
    deviceChanged.detach();
#endif
}

void DeviceManagerNotify::OnDeviceChanged(const std::string &pkgName, const DmDeviceBasicInfo &deviceBasicInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }

    std::shared_ptr<DeviceStatusCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStatusCallback_.find(pkgName);
        if (iter == deviceStatusCallback_.end()) {
            LOGE("OnDeviceChanged error, device state callback not register, pkgName:%{public}s", pkgName.c_str());
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceChanged error, registered device state callback is nullptr, pkgName:%{public}s", pkgName.c_str());
        return;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceBasicInfoChanged(deviceBasicInfo, tempCbk); });
#else
    std::thread deviceChanged([=]() { DeviceBasicInfoChanged(deviceBasicInfo, tempCbk); });
    if (pthread_setname_np(deviceChanged.native_handle(), DEVICEINFO_CHANGE) != DM_OK) {
        LOGE("deviceChanged set name failed.");
    }
    deviceChanged.detach();
#endif
}

void DeviceManagerNotify::OnDeviceReady(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }

    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStateCallback_.find(pkgName);
        if (iter == deviceStateCallback_.end()) {
            LOGE("OnDeviceReady error, device state callback not register, pkgName:%{public}s", pkgName.c_str());
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceReady error, registered device state callback is nullptr, pkgName:%{public}s", pkgName.c_str());
        return;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceInfoReady(deviceInfo, tempCbk); });
#else
    std::thread deviceReady([=]() { DeviceInfoReady(deviceInfo, tempCbk); });
    if (pthread_setname_np(deviceReady.native_handle(), DEVICE_READY) != DM_OK) {
        LOGE("deviceReady set name failed.");
    }
    deviceReady.detach();
#endif
}

void DeviceManagerNotify::OnDeviceReady(const std::string &pkgName, const DmDeviceBasicInfo &deviceBasicInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceReady with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStatusCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStatusCallback_.find(pkgName);
        if (iter == deviceStatusCallback_.end()) {
            LOGE("OnDeviceReady error, device status callback not register, pkgName:%{public}s", pkgName.c_str());
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceReady error, registered device status callback is nullptr.");
        return;
    }
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceBasicInfoReady(deviceBasicInfo, tempCbk); });
#else
    std::thread deviceReady([=]() { DeviceBasicInfoReady(deviceBasicInfo, tempCbk); });
    if (pthread_setname_np(deviceReady.native_handle(), DEVICE_READY) != DM_OK) {
        LOGE("deviceReady set name failed.");
    }
    deviceReady.detach();
#endif
}

void DeviceManagerNotify::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                        const DmDeviceInfo &deviceInfo)
{
    LOGD("pkgName:%{public}s, subscribeId:%{public}d.",
         GetAnonyString(pkgName).c_str(), (int32_t)subscribeId);
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::shared_ptr<DiscoveryCallback> tempCbk = GetDiscoveryCallback(pkgName, subscribeId);
    if (tempCbk == nullptr) {
        LOGE("OnDeviceFound error, registered device discovery callback is nullptr.");
        return;
    }
    tempCbk->OnDeviceFound(subscribeId, deviceInfo);
}

void DeviceManagerNotify::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                        const DmDeviceBasicInfo &deviceBasicInfo)
{
    LOGD("pkgName:%{public}s, subscribeId:%{public}d.",
         GetAnonyString(pkgName).c_str(), (int32_t)subscribeId);
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::shared_ptr<DiscoveryCallback> tempCbk = GetDiscoveryCallback(pkgName, subscribeId);
    if (tempCbk == nullptr) {
        LOGE("OnDeviceFound error, registered device discovery callback is nullptr.");
        return;
    }
    tempCbk->OnDeviceFound(subscribeId, deviceBasicInfo);
}

void DeviceManagerNotify::OnDiscoveryFailed(const std::string &pkgName, uint16_t subscribeId, int32_t failedReason)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDiscoveryFailed in, pkgName:%{public}s, subscribeId %{public}d, failed"
        "reason %{public}d", pkgName.c_str(), (int32_t)subscribeId, failedReason);
    std::shared_ptr<DiscoveryCallback> tempCbk = GetDiscoveryCallback(pkgName, subscribeId);
    if (tempCbk == nullptr) {
        LOGE("OnDiscoveryFailed error, registered device discovery callback is nullptr.");
        return;
    }
    tempCbk->OnDiscoveryFailed(subscribeId, failedReason);
}

void DeviceManagerNotify::OnDiscoverySuccess(const std::string &pkgName, uint16_t subscribeId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("PkgName:%{public}s, subscribeId:%{public}d.", GetAnonyString(pkgName).c_str(), subscribeId);
    std::shared_ptr<DiscoveryCallback> tempCbk = GetDiscoveryCallback(pkgName, subscribeId);
    if (tempCbk == nullptr) {
        LOGE("OnDiscoverySuccess error, registered device discovery callback is nullptr.");
        return;
    }
    tempCbk->OnDiscoverySuccess(subscribeId);
}

void DeviceManagerNotify::OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnPublishResult in, pkgName:%{public}s, publishId %{public}d, publishResult %{public}d",
        pkgName.c_str(), publishId, publishResult);
    std::shared_ptr<PublishCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (devicePublishCallbacks_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnPublishResult error, device publish callback not register for"
                "pkgName %{public}s.", pkgName.c_str());
            return;
        }
        std::map<int32_t, std::shared_ptr<PublishCallback>> &publishCallMap = devicePublishCallbacks_[pkgName];
        auto iter = publishCallMap.find(publishId);
        if (iter == publishCallMap.end()) {
            LOGE("OnPublishResult error, device publish callback not register for publishId %{public}d.", publishId);
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnPublishResult error, registered device publish callback is nullptr.");
        return;
    }
    tempCbk->OnPublishResult(publishId, publishResult);
}

void DeviceManagerNotify::OnAuthResult(const std::string &pkgName, const std::string &deviceId,
                                       const std::string &token, int32_t status, int32_t reason)
{
    if (pkgName.empty() || token.empty() || deviceId.empty()) {
        LOGE("Invalid para, pkgName: %{public}s, token: %{public}s", pkgName.c_str(), GetAnonyString(token).c_str());
        return;
    }
    LOGI("DeviceManagerNotify::OnAuthResult in, pkgName:%{public}s, status:%{public}d, reason:%{public}d",
        pkgName.c_str(), status, reason);
    std::shared_ptr<AuthenticateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (authenticateCallback_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnAuthResult error, authenticate callback not register for pkgName %{public}s.",
                pkgName.c_str());
            return;
        }
        std::map<std::string, std::shared_ptr<AuthenticateCallback>> &authCallMap = authenticateCallback_[pkgName];
        auto iter = authCallMap.find(deviceId);
        if (iter == authCallMap.end()) {
            LOGE("OnAuthResult error, authenticate callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnAuthResult error, registered authenticate callback is nullptr.");
        return;
    }
    tempCbk->OnAuthResult(deviceId, token, status, reason);
    if (reason == DM_OK && (status <= STATUS_DM_CLOSE_PIN_INPUT_UI && status >= STATUS_DM_SHOW_AUTHORIZE_UI)) {
        LOGI("update ui change, status: %{public}d, reason: %{public}d", status, reason);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        authenticateCallback_[pkgName].erase(deviceId);
        if (authenticateCallback_[pkgName].empty()) {
            authenticateCallback_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::OnUiCall(std::string &pkgName, std::string &paramJson)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerNotify::OnUiCall error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return;
    }
    LOGI("DeviceManagerNotify::OnUiCall in, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceManagerUiCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (dmUiCallback_.count(pkgName) == 0) {
            LOGE("OnUiCall error, dm Ui callback not register for pkgName %{public}s.", pkgName.c_str());
            return;
        }
        tempCbk = dmUiCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnUiCall error, registered dm Ui callback is nullptr.");
        return;
    }
    tempCbk->OnCall(paramJson);
}

void DeviceManagerNotify::OnCredentialResult(const std::string &pkgName, int32_t &action,
                                             const std::string &credentialResult)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerNotify::OnCredentialResult error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return;
    }
    LOGI("DeviceManagerNotify::OnCredentialResult in, pkgName:%{public}s, action:%{public}d", pkgName.c_str(), action);
    std::shared_ptr<CredentialCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (credentialCallback_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnCredentialResult error, credential callback not register for"
                "pkgName %{public}s.", pkgName.c_str());
            return;
        }
        tempCbk = credentialCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnCredentialResult error, registered credential callback is nullptr.");
        return;
    }
    tempCbk->OnCredentialResult(action, credentialResult);
}

void DeviceManagerNotify::RegisterBindCallback(const std::string &pkgName, const PeerTargetId &targetId,
    std::shared_ptr<BindTargetCallback> callback)
{
    if (pkgName.empty() || IsInvalidPeerTargetId(targetId) || (callback == nullptr)) {
        LOGE("DeviceManagerNotify::RegisterBindCallback error: Invalid parameter, pkgName: %{public}s.",
            pkgName.c_str());
        return;
    }
    std::lock_guard<std::mutex> autoLock(bindLock_);
    if (bindCallback_.count(pkgName) == 0) {
        bindCallback_[pkgName] = std::map<PeerTargetId, std::shared_ptr<BindTargetCallback>>();
    }
    bindCallback_[pkgName][targetId] = callback;
}

void DeviceManagerNotify::RegisterUnbindCallback(const std::string &pkgName, const PeerTargetId &targetId,
    std::shared_ptr<UnbindTargetCallback> callback)
{
    if (pkgName.empty() || IsInvalidPeerTargetId(targetId) || (callback == nullptr)) {
        LOGE("DeviceManagerNotify::RegisterUnbindCallback error: Invalid parameter, pkgName: %{public}s.",
            pkgName.c_str());
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (unbindCallback_.count(pkgName) == 0) {
        unbindCallback_[pkgName] = std::map<PeerTargetId, std::shared_ptr<UnbindTargetCallback>>();
    }
    unbindCallback_[pkgName][targetId] = callback;
}

void DeviceManagerNotify::OnBindResult(const std::string &pkgName, const PeerTargetId &targetId,
    int32_t result, int32_t status, std::string content)
{
    if (pkgName.empty() || IsInvalidPeerTargetId(targetId)) {
        LOGE("Invalid para, pkgName: %{public}s.", pkgName.c_str());
        return;
    }
    LOGI("DeviceManagerNotify::OnBindResult in, pkgName:%{public}s, result:%{public}d", pkgName.c_str(), result);
    std::shared_ptr<BindTargetCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(bindLock_);
        if (bindCallback_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnBindResult error, callback not register for pkgName %{public}s.",
                pkgName.c_str());
            return;
        }
        std::map<PeerTargetId, std::shared_ptr<BindTargetCallback>> &bindCbkMap = bindCallback_[pkgName];
        auto iter = bindCbkMap.find(targetId);
        if (iter == bindCbkMap.end()) {
            LOGE("OnBindResult error, bind callback not register for targetId.");
            return;
        }
        tempCbk = iter->second;
        if (result != DM_OK || status == STATUS_DM_AUTH_FINISH || status == STATUS_DM_AUTH_DEFAULT) {
            LOGI("notify end, result: %{public}d, status: %{public}d", result, status);
            bindCallback_[pkgName].erase(targetId);
            if (bindCallback_[pkgName].empty()) {
                bindCallback_.erase(pkgName);
            }
        }
    }
    if (tempCbk == nullptr) {
        LOGE("OnBindResult error, registered bind callback is nullptr.");
        return;
    }
    tempCbk->OnBindResult(targetId, result, status, content);
}

void DeviceManagerNotify::OnUnbindResult(const std::string &pkgName, const PeerTargetId &targetId,
    int32_t result, std::string content)
{
    if (pkgName.empty() || IsInvalidPeerTargetId(targetId)) {
        LOGE("Invalid para, pkgName: %{public}s.", pkgName.c_str());
        return;
    }
    LOGI("DeviceManagerNotify::OnUnbindResult in, pkgName:%{public}s, result:%{public}d", pkgName.c_str(), result);
    std::shared_ptr<UnbindTargetCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (unbindCallback_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnUnbindResult error, callback not register for pkgName %{public}s.",
                pkgName.c_str());
            return;
        }
        std::map<PeerTargetId, std::shared_ptr<UnbindTargetCallback>> &unbindCbkMap = unbindCallback_[pkgName];
        auto iter = unbindCbkMap.find(targetId);
        if (iter == unbindCbkMap.end()) {
            LOGE("OnUnbindResult error, unbind callback not register for targetId.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnUnbindResult error, registered unbind callback is nullptr.");
        return;
    }
    tempCbk->OnUnbindResult(targetId, result, content);
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        unbindCallback_[pkgName].erase(targetId);
        if (unbindCallback_[pkgName].empty()) {
            unbindCallback_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::OnPinHolderCreate(const std::string &pkgName, const std::string &deviceId,
    DmPinType pinType, const std::string &payload)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnPinHolderCreate in, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<PinHolderCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (pinHolderCallback_.count(pkgName) == 0) {
            LOGE("OnPinHolderCreate error, device state callback not register.");
            return;
        }
        tempCbk = pinHolderCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnPinHolderCreate error, registered device state callback is nullptr.");
        return;
    }
    tempCbk->OnPinHolderCreate(deviceId, pinType, payload);
}

void DeviceManagerNotify::OnPinHolderDestroy(const std::string &pkgName, DmPinType pinType,
    const std::string &payload)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnPinHolderDestroy in, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<PinHolderCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (pinHolderCallback_.count(pkgName) == 0) {
            LOGE("OnPinHolderDestroy error, device state callback not register.");
            return;
        }
        tempCbk = pinHolderCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnPinHolderDestroy error, registered device state callback is nullptr.");
        return;
    }
    tempCbk->OnPinHolderDestroy(pinType, payload);
}

void DeviceManagerNotify::OnCreateResult(const std::string &pkgName, int32_t result)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnCreateResult in, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<PinHolderCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (pinHolderCallback_.count(pkgName) == 0) {
            LOGE("OnCreateResult error, device state callback not register.");
            return;
        }
        tempCbk = pinHolderCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnCreateResult error, registered device state callback is nullptr.");
        return;
    }
    tempCbk->OnCreateResult(result);
}

void DeviceManagerNotify::OnDestroyResult(const std::string &pkgName, int32_t result)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDestroyResult in, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<PinHolderCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (pinHolderCallback_.count(pkgName) == 0) {
            LOGE("OnDestroyResult error, device state callback not register.");
            return;
        }
        tempCbk = pinHolderCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnDestroyResult error, registered device state callback is nullptr.");
        return;
    }
    tempCbk->OnDestroyResult(result);
}

void DeviceManagerNotify::OnPinHolderEvent(const std::string &pkgName, DmPinHolderEvent event, int32_t result,
                                           const std::string &content)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnPinHolderEvent in, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<PinHolderCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (pinHolderCallback_.count(pkgName) == 0) {
            LOGE("OnPinHolderEvent error, pin holder callback not register.");
            return;
        }
        tempCbk = pinHolderCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnPinHolderEvent error, registered pin holder callback is nullptr.");
        return;
    }
    tempCbk->OnPinHolderEvent(event, result, content);
}

std::map<std::string, std::shared_ptr<DmInitCallback>> DeviceManagerNotify::GetDmInitCallback()
{
    std::lock_guard<std::mutex> autoLock(lock_);
    std::map<std::string, std::shared_ptr<DmInitCallback>> currentDmInitCallback = dmInitCallback_;
    return currentDmInitCallback;
}

void DeviceManagerNotify::DeviceInfoOnline(const DmDeviceInfo &deviceInfo, std::shared_ptr<DeviceStateCallback> tempCbk)
{
    tempCbk->OnDeviceOnline(deviceInfo);
}

void DeviceManagerNotify::DeviceInfoOffline(const DmDeviceInfo &deviceInfo,
    std::shared_ptr<DeviceStateCallback> tempCbk)
{
    tempCbk->OnDeviceOffline(deviceInfo);
}

void DeviceManagerNotify::DeviceInfoChanged(const DmDeviceInfo &deviceInfo,
    std::shared_ptr<DeviceStateCallback> tempCbk)
{
    tempCbk->OnDeviceChanged(deviceInfo);
}

void DeviceManagerNotify::DeviceInfoReady(const DmDeviceInfo &deviceInfo, std::shared_ptr<DeviceStateCallback> tempCbk)
{
    tempCbk->OnDeviceReady(deviceInfo);
}

void DeviceManagerNotify::DeviceBasicInfoOnline(const DmDeviceBasicInfo &deviceBasicInfo,
    std::shared_ptr<DeviceStatusCallback> tempCbk)
{
    tempCbk->OnDeviceOnline(deviceBasicInfo);
}

void DeviceManagerNotify::DeviceBasicInfoOffline(const DmDeviceBasicInfo &deviceBasicInfo,
    std::shared_ptr<DeviceStatusCallback> tempCbk)
{
    tempCbk->OnDeviceOffline(deviceBasicInfo);
}

void DeviceManagerNotify::DeviceBasicInfoChanged(const DmDeviceBasicInfo &deviceBasicInfo,
    std::shared_ptr<DeviceStatusCallback> tempCbk)
{
    tempCbk->OnDeviceChanged(deviceBasicInfo);
}

void DeviceManagerNotify::DeviceBasicInfoReady(const DmDeviceBasicInfo &deviceBasicInfo,
    std::shared_ptr<DeviceStatusCallback> tempCbk)
{
    tempCbk->OnDeviceReady(deviceBasicInfo);
}

void DeviceManagerNotify::RegDevTrustChangeCallback(const std::string &pkgName,
    std::shared_ptr<DevTrustChangeCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    devTrustChangeCallback_[pkgName] = callback;
}

void DeviceManagerNotify::OnDeviceTrustChange(const std::string &pkgName, const std::string &udid,
    const std::string &uuid, int32_t authForm)
{
    LOGI("PkgName %{public}s, udid %{public}s, uuid %{public}s, authForm %{public}d", pkgName.c_str(),
        GetAnonyString(udid).c_str(), GetAnonyString(uuid).c_str(), authForm);
    if (pkgName.empty() || authForm < static_cast<int32_t>(INVALID_TYPE) ||
        authForm > static_cast<int32_t>(ACROSS_ACCOUNT)) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::shared_ptr<DevTrustChangeCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = devTrustChangeCallback_.find(pkgName);
        if (iter == devTrustChangeCallback_.end()) {
            LOGE("PkgName %{public}s device_trust_change callback not register.", pkgName.c_str());
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceReady error, registered device status callback is nullptr.");
        return;
    }
    DmAuthForm dmAuthForm = static_cast<DmAuthForm>(authForm);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { DeviceTrustChange(udid, uuid, dmAuthForm, tempCbk); });
#else
    std::thread deviceTrustChange([=]() { DeviceTrustChange(udid, uuid, dmAuthForm, tempCbk); });
    if (pthread_setname_np(deviceTrustChange.native_handle(), DEVICE_TRUST_CHANGE) != DM_OK) {
        LOGE("deviceTrustChange set name failed.");
    }
    deviceTrustChange.detach();
#endif
}

void DeviceManagerNotify::DeviceTrustChange(const std::string &udid, const std::string &uuid, DmAuthForm authForm,
    std::shared_ptr<DevTrustChangeCallback> tempCbk)
{
    if (tempCbk == nullptr) {
        LOGE("Callback ptr is nullptr.");
        return;
    }
    tempCbk->OnDeviceTrustChange(udid, uuid, authForm);
}

void DeviceManagerNotify::RegisterDeviceScreenStatusCallback(const std::string &pkgName,
    std::shared_ptr<DeviceScreenStatusCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceScreenStatusCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterDeviceScreenStatusCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceScreenStatusCallback_.erase(pkgName);
}

void DeviceManagerNotify::OnDeviceScreenStatus(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("In, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceScreenStatusCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceScreenStatusCallback_.count(pkgName) == 0) {
            LOGE("error, device screen status not register.");
            return;
        }
        tempCbk = deviceScreenStatusCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("error, registered device screen status callback is nullptr.");
        return;
    }
    tempCbk->OnDeviceScreenStatus(deviceInfo);
}

void DeviceManagerNotify::RegisterCredentialAuthStatusCallback(const std::string &pkgName,
    std::shared_ptr<CredentialAuthStatusCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    credentialAuthStatusCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterCredentialAuthStatusCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    credentialAuthStatusCallback_.erase(pkgName);
}

void DeviceManagerNotify::OnCredentialAuthStatus(const std::string &pkgName, const std::string &deviceList,
                                                 uint16_t deviceTypeId, int32_t errcode)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("In, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<CredentialAuthStatusCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (credentialAuthStatusCallback_.find(pkgName) == credentialAuthStatusCallback_.end()) {
            LOGE("error, credential auth statusnot register.");
            return;
        }
        tempCbk = credentialAuthStatusCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("error, registered credential auth status callback is nullptr.");
        return;
    }
    tempCbk->OnCredentialAuthStatus(deviceList, deviceTypeId, errcode);
}

void DeviceManagerNotify::RegisterSinkBindCallback(const std::string &pkgName,
    std::shared_ptr<BindTargetCallback> callback)
{
    if (pkgName.empty() || callback == nullptr) {
        LOGE("Invalid parameter, pkgName is empty or callback is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    sinkBindTargetCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterSinkBindCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    sinkBindTargetCallback_.erase(pkgName);
}

void DeviceManagerNotify::OnSinkBindResult(const std::string &pkgName, const PeerTargetId &targetId,
    int32_t result, int32_t status, std::string content)
{
    if (pkgName.empty()) {
        LOGE("Invalid para, pkgName: %{public}s.", pkgName.c_str());
        return;
    }
    LOGI("DeviceManagerNotify::OnSinkBindResult in, pkgName:%{public}s, result:%{public}d", pkgName.c_str(), result);
    std::shared_ptr<BindTargetCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (sinkBindTargetCallback_.find(pkgName) == sinkBindTargetCallback_.end()) {
            LOGE("error, sink bind callback not register.");
            return;
        }
        tempCbk = sinkBindTargetCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("error, registered sink bind callback is nullptr.");
        return;
    }
    tempCbk->OnBindResult(targetId, result, status, content);
}

std::shared_ptr<DiscoveryCallback> DeviceManagerNotify::GetDiscoveryCallback(const std::string &pkgName,
    uint16_t subscribeId)
{
    std::string discWithSubscribeId = ComposeStr(pkgName, subscribeId);
    std::lock_guard<std::mutex> autoLock(lock_);
    auto iter = deviceDiscoveryCallbacks_.find(discWithSubscribeId);
    if (iter != deviceDiscoveryCallbacks_.end()) {
        auto subIter = iter->second.find(subscribeId);
        if (subIter != iter->second.end()) {
            return subIter->second;
        }
        return nullptr;
    }
    std::string discNoSubscribeId = ComposeStr(pkgName, DM_INVALID_FLAG_ID);
    iter = deviceDiscoveryCallbacks_.find(discNoSubscribeId);
    if (iter != deviceDiscoveryCallbacks_.end()) {
        auto subIter = iter->second.find(subscribeId);
        if (subIter != iter->second.end()) {
            return subIter->second;
        }
    }
    return nullptr;
}

void DeviceManagerNotify::GetCallBack(std::map<DmCommonNotifyEvent, std::set<std::string>> &callbackMap)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    std::set<std::string> statePkgnameSet;
    for (auto it : deviceStateCallback_) {
        statePkgnameSet.insert(it.first);
    }
    for (auto it : deviceStatusCallback_) {
        statePkgnameSet.insert(it.first);
    }
    if (statePkgnameSet.size() > 0) {
        callbackMap[DmCommonNotifyEvent::REG_DEVICE_STATE] = statePkgnameSet;
    }

    std::set<std::string> trustChangePkgnameSet;
    for (auto it : devTrustChangeCallback_) {
        trustChangePkgnameSet.insert(it.first);
    }
    if (trustChangePkgnameSet.size() > 0) {
        callbackMap[DmCommonNotifyEvent::REG_REMOTE_DEVICE_TRUST_CHANGE] = trustChangePkgnameSet;
    }

    std::set<std::string> screenStatusPkgnameSet;
    for (auto it : deviceScreenStatusCallback_) {
        screenStatusPkgnameSet.insert(it.first);
    }
    if (screenStatusPkgnameSet.size() > 0) {
        callbackMap[DmCommonNotifyEvent::REG_DEVICE_SCREEN_STATE] = screenStatusPkgnameSet;
    }

    std::set<std::string> authStatusPkgnameSet;
    for (auto it : credentialAuthStatusCallback_) {
        authStatusPkgnameSet.insert(it.first);
    }
    if (authStatusPkgnameSet.size() > 0) {
        callbackMap[DmCommonNotifyEvent::REG_CREDENTIAL_AUTH_STATUS_NOTIFY] = authStatusPkgnameSet;
    }
}

int32_t DeviceManagerNotify::RegisterGetDeviceProfileInfoListCallback(const std::string &pkgName,
    std::shared_ptr<GetDeviceProfileInfoListCallback> callback)
{
    LOGI("In, pkgName: %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(bindLock_);
    if (getDeviceProfileInfoCallback_.size() > MAX_CONTAINER_SIZE) {
        LOGE("callback map size is more than max size");
        return ERR_DM_MAX_SIZE_FAIL;
    }
    getDeviceProfileInfoCallback_[pkgName] = callback;
    return DM_OK;
}

int32_t DeviceManagerNotify::UnRegisterGetDeviceProfileInfoListCallback(const std::string &pkgName)
{
    LOGI("In, pkgName: %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(bindLock_);
    getDeviceProfileInfoCallback_.erase(pkgName);
    return DM_OK;
}

void DeviceManagerNotify::OnGetDeviceProfileInfoListResult(const std::string &pkgName,
    const std::vector<DmDeviceProfileInfo> &deviceProfileInfos, int32_t code)
{
    if (pkgName.empty()) {
        LOGE("Invalid para, pkgName: %{public}s.", pkgName.c_str());
        return;
    }
    LOGI("In, pkgName:%{public}s, code:%{public}d", pkgName.c_str(), code);
    std::shared_ptr<GetDeviceProfileInfoListCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(bindLock_);
        if (getDeviceProfileInfoCallback_.count(pkgName) == 0) {
            LOGE("error, callback not register for pkgName %{public}s.", pkgName.c_str());
            return;
        }
        tempCbk = getDeviceProfileInfoCallback_[pkgName];
        getDeviceProfileInfoCallback_.erase(pkgName);
    }
    if (tempCbk == nullptr) {
        LOGE("error, registered GetDeviceProfileInfoList callback is nullptr.");
        return;
    }
    tempCbk->OnResult(deviceProfileInfos, code);
}

int32_t DeviceManagerNotify::RegisterGetDeviceIconInfoCallback(const std::string &pkgName, const std::string &uk,
    std::shared_ptr<GetDeviceIconInfoCallback> callback)
{
    LOGI("In, pkgName: %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(bindLock_);
    if (getDeviceIconInfoCallback_.size() > MAX_CONTAINER_SIZE) {
        LOGE("callback map size is more than max size");
        return ERR_DM_MAX_SIZE_FAIL;
    }
    auto iter = getDeviceIconInfoCallback_.find(pkgName);
    if (iter == getDeviceIconInfoCallback_.end()) {
        getDeviceIconInfoCallback_[pkgName][uk] = {callback};
        return DM_OK;
    }
    if (iter->second.size() > MAX_CONTAINER_SIZE) {
        LOGE("callback map for pkg size is more than max size");
        return ERR_DM_MAX_SIZE_FAIL;
    }
    if (iter->second[uk].size() > MAX_CONTAINER_SIZE) {
        LOGE("callback set size is more than max size");
        return ERR_DM_MAX_SIZE_FAIL;
    }
    iter->second[uk].insert(callback);
    return DM_OK;
}

int32_t DeviceManagerNotify::UnRegisterGetDeviceIconInfoCallback(const std::string &pkgName, const std::string &uk)
{
    LOGI("In, pkgName: %{public}s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(bindLock_);
    auto iter = getDeviceIconInfoCallback_.find(pkgName);
    if (iter == getDeviceIconInfoCallback_.end()) {
        return DM_OK;
    }
    iter->second.erase(uk);
    if (iter->second.empty()) {
        getDeviceIconInfoCallback_.erase(pkgName);
    }
    return DM_OK;
}

void DeviceManagerNotify::OnGetDeviceIconInfoResult(const std::string &pkgName, const DmDeviceIconInfo &deviceIconInfo,
    int32_t code)
{
    if (pkgName.empty()) {
        LOGE("Invalid para, pkgName: %{public}s.", pkgName.c_str());
        return;
    }
    LOGI("In, pkgName:%{public}s, code:%{public}d", pkgName.c_str(), code);
    std::map<std::string, std::set<std::shared_ptr<GetDeviceIconInfoCallback>>> tempCbks;
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    std::string uk = IpcModelCodec::GetDeviceIconInfoUniqueKey(deviceIconInfo);
    {
        std::lock_guard<std::mutex> autoLock(bindLock_);
        auto iter = getDeviceIconInfoCallback_.find(pkgName);
        if (iter == getDeviceIconInfoCallback_.end()) {
            LOGE("error, callback not register for pkgName %{public}s.", pkgName.c_str());
            return;
        }
        if (ERR_DM_HILINKSVC_DISCONNECT == code) {
            tempCbks = iter->second;
            getDeviceIconInfoCallback_.erase(pkgName);
        } else if (iter->second.count(uk) != 0) {
            tempCbks[uk] = iter->second[uk];
            iter->second.erase(uk);
        }
    }
#endif
    if (tempCbks.empty()) {
        LOGE("error, registered GetDeviceIconInfoResult callback is nullptr.");
        return;
    }
    for (const auto &[key, callbacks] : tempCbks) {
        for (auto callback : callbacks) {
            if (callback != nullptr) {
                callback->OnResult(deviceIconInfo, code);
            }
        }
    }
}

int32_t DeviceManagerNotify::RegisterSetLocalDeviceNameCallback(const std::string &pkgName,
    std::shared_ptr<SetLocalDeviceNameCallback> callback)
{
    LOGI("In, pkgName: %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(bindLock_);
    if (setLocalDeviceNameCallback_.size() > MAX_CONTAINER_SIZE) {
        LOGI("callback map size is more than max size");
        return ERR_DM_MAX_SIZE_FAIL;
    }
    setLocalDeviceNameCallback_[pkgName] = callback;
    return DM_OK;
}

int32_t DeviceManagerNotify::UnRegisterSetLocalDeviceNameCallback(const std::string &pkgName)
{
    LOGI("In, pkgName: %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(bindLock_);
    setLocalDeviceNameCallback_.erase(pkgName);
    return DM_OK;
}

int32_t DeviceManagerNotify::RegisterSetRemoteDeviceNameCallback(const std::string &pkgName,
    const std::string &deviceId, std::shared_ptr<SetRemoteDeviceNameCallback> callback)
{
    LOGI("In, pkgName: %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(bindLock_);
    if (setRemoteDeviceNameCallback_.size() > MAX_CONTAINER_SIZE) {
        LOGI("callback map size is more than max size");
        return ERR_DM_MAX_SIZE_FAIL;
    }
    auto iter = setRemoteDeviceNameCallback_.find(pkgName);
    if (iter == setRemoteDeviceNameCallback_.end()) {
        setRemoteDeviceNameCallback_[pkgName][deviceId] = {callback};
        return DM_OK;
    }
    if (iter->second.size() > MAX_CONTAINER_SIZE) {
        LOGI("callback map size is more than max size");
        return ERR_DM_MAX_SIZE_FAIL;
    }
    if (iter->second[deviceId].size() > MAX_CONTAINER_SIZE) {
        LOGI("callback set size is more than max size");
        return ERR_DM_MAX_SIZE_FAIL;
    }
    iter->second[deviceId].insert(callback);
    return DM_OK;
}

int32_t DeviceManagerNotify::UnRegisterSetRemoteDeviceNameCallback(const std::string &pkgName,
    const std::string &deviceId)
{
    LOGI("In, pkgName: %{public}s.", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(bindLock_);
    auto iter = setRemoteDeviceNameCallback_.find(pkgName);
    if (iter == setRemoteDeviceNameCallback_.end()) {
        return DM_OK;
    }
    iter->second.erase(deviceId);
    if (iter->second.empty()) {
        getDeviceIconInfoCallback_.erase(pkgName);
    }
    return DM_OK;
}

void DeviceManagerNotify::OnSetLocalDeviceNameResult(const std::string &pkgName, int32_t code)
{
    if (pkgName.empty()) {
        LOGE("Invalid para, pkgName : %{public}s", pkgName.c_str());
        return;
    }
    LOGI("In, pkgName:%{public}s, code:%{public}d", pkgName.c_str(), code);
    std::shared_ptr<SetLocalDeviceNameCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(bindLock_);
        if (setLocalDeviceNameCallback_.count(pkgName) == 0) {
            LOGE("error, callback not register for pkgName %{public}s.", pkgName.c_str());
            return;
        }
        tempCbk = setLocalDeviceNameCallback_[pkgName];
        setLocalDeviceNameCallback_.erase(pkgName);
    }
    if (tempCbk == nullptr) {
        LOGE("error, registered SetLocalDeviceName callback is nullptr.");
        return;
    }
    tempCbk->OnResult(code);
}

void DeviceManagerNotify::OnSetRemoteDeviceNameResult(const std::string &pkgName, const std::string &deviceId,
    int32_t code)
{
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("Invalid para, pkgName : %{public}s, deviceId : %{public}s",
            pkgName.c_str(), GetAnonyString(deviceId).c_str());
        return;
    }
    LOGI("In, pkgName:%{public}s, code:%{public}d", pkgName.c_str(), code);
    std::map<std::string, std::set<std::shared_ptr<SetRemoteDeviceNameCallback>>> tempCbks;
    {
        std::lock_guard<std::mutex> autoLock(bindLock_);
        auto iter = setRemoteDeviceNameCallback_.find(pkgName);
        if (iter == setRemoteDeviceNameCallback_.end()) {
            LOGE("error, callback not register for pkgName %{public}s.", pkgName.c_str());
            return;
        }
        if (ERR_DM_HILINKSVC_DISCONNECT == code) {
            tempCbks = iter->second;
            setRemoteDeviceNameCallback_.erase(pkgName);
        } else if (iter->second.count(deviceId) != 0) {
            tempCbks[deviceId] = iter->second[deviceId];
            iter->second.erase(deviceId);
        }
    }
    if (tempCbks.empty()) {
        LOGE("error, registered GetDeviceIconInfoResult callback is nullptr.");
        return;
    }
    for (const auto &[key, callbacks] : tempCbks) {
        for (auto callback : callbacks) {
            if (callback != nullptr) {
                callback->OnResult(code);
            }
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS