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

#include "device_manager_notify.h"

#include "device_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerNotify);

constexpr uint32_t WAIT_BINDIND_TIME_OUT_SECOND = 1;
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
    deviceDiscoveryCallbacks_.erase(pkgName);
    devicePublishCallbacks_.erase(pkgName);
    authenticateCallback_.erase(pkgName);
    dmInitCallback_.erase(pkgName);
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
    std::lock_guard<std::mutex> autoLock(lock_);
    for (auto iter : dmInitCallback_) {
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
    LOGI("DeviceManagerNotify::OnDeviceOnline with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
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
    LOGI("DeviceManagerNotify::OnDeviceOnline complete with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
    tempCbk->OnDeviceOnline(deviceInfo);
}

void DeviceManagerNotify::OnDeviceOnline(const std::string &pkgName, const DmDeviceBasicInfo &deviceBasicInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceOnline with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStatusCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStatusCallback_.find(pkgName);
        if (iter == deviceStatusCallback_.end()) {
            LOGE("OnDeviceOnline error, device status callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceOnline error, registered device status callback is nullptr.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceOnline complete with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    tempCbk->OnDeviceOnline(deviceBasicInfo);
}

void DeviceManagerNotify::OnDeviceOffline(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceOffline with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStateCallback_.find(pkgName);
        if (iter == deviceStateCallback_.end()) {
            LOGE("OnDeviceOffline error, device state callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceOffline error, registered device state callback is nullptr.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceOffline complete with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
    tempCbk->OnDeviceOffline(deviceInfo);
}

void DeviceManagerNotify::OnDeviceOffline(const std::string &pkgName, const DmDeviceBasicInfo &deviceBasicInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceOffline with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStatusCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStatusCallback_.find(pkgName);
        if (iter == deviceStatusCallback_.end()) {
            LOGE("OnDeviceOffline error, device status callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceOffline error, registered device status callback is nullptr.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceOffline complete with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    tempCbk->OnDeviceOffline(deviceBasicInfo);
}

void DeviceManagerNotify::OnDeviceChanged(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceChanged with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStateCallback_.find(pkgName);
        if (iter == deviceStateCallback_.end()) {
            LOGE("OnDeviceChanged error, device state callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceChanged error, registered device state callback is nullptr.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceChanged complete with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
    tempCbk->OnDeviceChanged(deviceInfo);
}

void DeviceManagerNotify::OnDeviceChanged(const std::string &pkgName, const DmDeviceBasicInfo &deviceBasicInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceChanged with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStatusCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStatusCallback_.find(pkgName);
        if (iter == deviceStatusCallback_.end()) {
            LOGE("OnDeviceChanged error, device state callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceChanged error, registered device state callback is nullptr.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceChanged complete with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    tempCbk->OnDeviceChanged(deviceBasicInfo);
}

void DeviceManagerNotify::OnDeviceReady(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceReady with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        auto iter = deviceStateCallback_.find(pkgName);
        if (iter == deviceStateCallback_.end()) {
            LOGE("OnDeviceReady error, device state callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceReady error, registered device state callback is nullptr.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceReady complete with DmDeviceInfo, pkgName:%{public}s", pkgName.c_str());
    tempCbk->OnDeviceReady(deviceInfo);
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
            LOGE("OnDeviceReady error, device status callback not register.");
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceReady error, registered device status callback is nullptr.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceReady complete with DmDeviceBasicInfo, pkgName:%{public}s", pkgName.c_str());
    tempCbk->OnDeviceReady(deviceBasicInfo);
}

void DeviceManagerNotify::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                        const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceFound with DmDeviceInfo, pkgName:%{public}s, subscribeId:%{public}d.",
         pkgName.c_str(), (int32_t)subscribeId);
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnDeviceFound error, device discovery callback not register for"
                "pkgName %{public}s.", pkgName.c_str());
            return;
        }
        std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
        auto iter = discoverCallMap.find(subscribeId);
        if (iter == discoverCallMap.end()) {
            LOGE("OnDeviceFound error, no register deviceDiscoveryCallback for subscribeId %{public}d.",
                (int32_t)subscribeId);
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceFound error, registered device discovery callback is nullptr.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceFound complete with DmDeviceInfo, pkgName:%{public}s, subscribeId:%{public}d.",
         pkgName.c_str(), (int32_t)subscribeId);
    tempCbk->OnDeviceFound(subscribeId, deviceInfo);
}

void DeviceManagerNotify::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                        const DmDeviceBasicInfo &deviceBasicInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceFound with DmDeviceBasicInfo, pkgName:%{public}s, subscribeId:%{public}d.",
         pkgName.c_str(), (int32_t)subscribeId);
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnDeviceFound error, device discovery callback not register for"
                "pkgName %{public}s.", pkgName.c_str());
            return;
        }
        std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
        auto iter = discoverCallMap.find(subscribeId);
        if (iter == discoverCallMap.end()) {
            LOGE("OnDeviceFound error, no register deviceDiscoveryCallback for subscribeId %{public}d.",
                (int32_t)subscribeId);
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceFound error, registered device discovery callback is nullptr.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceFound complete with DmDeviceBasicInfo, pkgName:%{public}s,"
        "subscribeId:%{public}d.", pkgName.c_str(), (int32_t)subscribeId);
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
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnDiscoveryFailed error, device discovery callback not register for"
                "pkgName %{public}s.", pkgName.c_str());
            return;
        }
        std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
        auto iter = discoverCallMap.find(subscribeId);
        if (iter == discoverCallMap.end()) {
            LOGE("OnDiscoveryFailed error, device discovery callback not register for subscribeId %{public}d.",
                subscribeId);
            return;
        }
        tempCbk = iter->second;
    }
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
    LOGI("DeviceManagerNotify::OnDiscoverySuccess in, pkgName:%{public}s, subscribeId:%{public}d.", pkgName.c_str(),
        subscribeId);
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("OnDiscoverySuccess error, device discovery callback not register for pkgName %{public}s.",
                pkgName.c_str());
            return;
        }
        std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
        auto iter = discoverCallMap.find(subscribeId);
        if (iter == discoverCallMap.end()) {
            LOGE("OnDiscoverySuccess error, device discovery callback not register for subscribeId %{public}d.",
                (int32_t)subscribeId);
            return;
        }
        tempCbk = iter->second;
    }
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
    std::unique_lock<std::mutex> ulk(bindLock_);
    cv_.wait_for(ulk, std::chrono::seconds(WAIT_BINDIND_TIME_OUT_SECOND), [this]() {return !binding_; });
    binding_ = true;
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
        std::lock_guard<std::mutex> glk(bindLock_);
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
        if (result != DM_OK || status > STATUS_DM_CLOSE_PIN_INPUT_UI || status < STATUS_DM_SHOW_AUTHORIZE_UI) {
            LOGI("notify end, result: %{public}d, status: %{public}d", result, status);
            bindCallback_[pkgName].erase(targetId);
            if (bindCallback_[pkgName].empty()) {
                bindCallback_.erase(pkgName);
            }
            binding_ = false;
            cv_.notify_one();
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
} // namespace DistributedHardware
} // namespace OHOS
