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

#include "device_manager_notify.h"

#include "device_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerNotify);

void DeviceManagerNotify::RegisterDeathRecipientCallback(const std::string &pkgName,
                                                         std::shared_ptr<DmInitCallback> dmInitCallback)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
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
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
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

void DeviceManagerNotify::RegisterDiscoveryCallback(const std::string &pkgName, uint16_t subscribeId,
                                                    std::shared_ptr<DiscoveryCallback> callback)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
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
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
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
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerNotify::RegisterAuthenticateCallback error: Invalid parameter, pkgName: %s, deviceId: %s",
            pkgName.c_str(), GetAnonyString(deviceId).c_str());
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
        LOGE("DeviceManagerNotify::UnRegisterAuthenticateCallback error: Invalid parameter, pkgName: %s, deviceId: %s",
            pkgName.c_str(), GetAnonyString(deviceId).c_str());
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
    deviceDiscoveryCallbacks_.erase(pkgName);
    devicePublishCallbacks_.erase(pkgName);
    authenticateCallback_.erase(pkgName);
    dmInitCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterVerifyAuthenticationCallback(const std::string &pkgName, const std::string &authPara,
                                                               std::shared_ptr<VerifyAuthCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    verifyAuthCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterVerifyAuthenticationCallback(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    verifyAuthCallback_.erase(pkgName);
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
    std::lock_guard<std::mutex> autoLock(lock_);
    credentialCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterCredentialCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    credentialCallback_.erase(pkgName);
}

void DeviceManagerNotify::OnRemoteDied()
{
    LOGW("DeviceManagerNotify::OnRemoteDied");
    for (auto iter : dmInitCallback_) {
        LOGI("DeviceManagerNotify::OnRemoteDied, pkgName:%s", iter.first.c_str());
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
    LOGI("DeviceManagerNotify::OnDeviceOnline in, pkgName:%s", pkgName.c_str());
    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceStateCallback_.count(pkgName) == 0) {
            LOGE("OnDeviceOnline error, device state callback not register.");
            return;
        }
        tempCbk = deviceStateCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceOnline error, registered device state callback is nullptr.");
        return;
    }
    tempCbk->OnDeviceOnline(deviceInfo);
}

void DeviceManagerNotify::OnDeviceOffline(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceOffline in, pkgName:%s", pkgName.c_str());
    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceStateCallback_.count(pkgName) == 0) {
            LOGE("OnDeviceOffline error, device state callback not register.");
            return;
        }
        tempCbk = deviceStateCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceOffline error, registered device state callback is nullptr.");
        return;
    }
    tempCbk->OnDeviceOffline(deviceInfo);
}

void DeviceManagerNotify::OnDeviceChanged(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceChanged in, pkgName:%s", pkgName.c_str());
    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceStateCallback_.count(pkgName) == 0) {
            LOGE("OnDeviceChanged error, device state callback not register.");
            return;
        }
        tempCbk = deviceStateCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceChanged error, registered device state callback is nullptr.");
        return;
    }
    tempCbk->OnDeviceChanged(deviceInfo);
}

void DeviceManagerNotify::OnDeviceReady(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceReady in, pkgName:%s", pkgName.c_str());
    std::shared_ptr<DeviceStateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceStateCallback_.count(pkgName) == 0) {
            LOGE("OnDeviceReady error, device state callback not register.");
            return;
        }
        tempCbk = deviceStateCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceReady error, registered device state callback is nullptr.");
        return;
    }
    tempCbk->OnDeviceReady(deviceInfo);
}

void DeviceManagerNotify::OnDeviceFound(const std::string &pkgName, uint16_t subscribeId,
                                        const DmDeviceInfo &deviceInfo)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDeviceFound in, pkgName:%s, subscribeId:%d.", pkgName.c_str(), (int32_t)subscribeId);
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnDeviceFound error, device discovery callback not register for pkgName %s.",
                pkgName.c_str());
            return;
        }
        std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
        auto iter = discoverCallMap.find(subscribeId);
        if (iter == discoverCallMap.end()) {
            LOGE("OnDeviceFound error, device discovery callback not register for subscribeId %d.", subscribeId);
            return;
        }
        tempCbk = iter->second;
    }
    if (tempCbk == nullptr) {
        LOGE("OnDeviceFound error, registered device discovery callback is nullptr.");
        return;
    }
    tempCbk->OnDeviceFound(subscribeId, deviceInfo);
}

void DeviceManagerNotify::OnDiscoveryFailed(const std::string &pkgName, uint16_t subscribeId, int32_t failedReason)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnDiscoveryFailed in, pkgName:%s, subscribeId %d, failed reason %d", pkgName.c_str(),
        subscribeId, failedReason);
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnDiscoveryFailed error, device discovery callback not register for pkgName %s.",
                pkgName.c_str());
            return;
        }
        std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
        auto iter = discoverCallMap.find(subscribeId);
        if (iter == discoverCallMap.end()) {
            LOGE("OnDiscoveryFailed error, device discovery callback not register for subscribeId %d.", subscribeId);
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
    LOGI("DeviceManagerNotify::OnDiscoverySuccess in, pkgName:%s, subscribeId:%d.", pkgName.c_str(), subscribeId);
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("OnDiscoverySuccess error, device discovery callback not register for pkgName %s.", pkgName.c_str());
            return;
        }
        std::map<uint16_t, std::shared_ptr<DiscoveryCallback>> &discoverCallMap = deviceDiscoveryCallbacks_[pkgName];
        auto iter = discoverCallMap.find(subscribeId);
        if (iter == discoverCallMap.end()) {
            LOGE("OnDiscoverySuccess error, device discovery callback not register for subscribeId %d.", subscribeId);
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
    LOGI("DeviceManagerNotify::OnPublishResult in, pkgName:%s, publishId %d, publishResult %d", pkgName.c_str(),
        publishId, publishResult);
    std::shared_ptr<PublishCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (devicePublishCallbacks_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnPublishResult error, device publish callback not register for pkgName %s.",
                pkgName.c_str());
            return;
        }
        std::map<int32_t, std::shared_ptr<PublishCallback>> &publishCallMap = devicePublishCallbacks_[pkgName];
        auto iter = publishCallMap.find(publishId);
        if (iter == publishCallMap.end()) {
            LOGE("OnPublishResult error, device publish callback not register for publishId %d.", publishId);
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
                                       const std::string &token, uint32_t status, uint32_t reason)
{
    if (pkgName.empty() || token.empty() || deviceId.empty()) {
        LOGE("Invalid para, pkgName: %s, token: %s", pkgName.c_str(), token.c_str());
        return;
    }
    LOGI("DeviceManagerNotify::OnAuthResult in, pkgName:%s, status:%d, reason:%u", pkgName.c_str(), status, reason);
    std::shared_ptr<AuthenticateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (authenticateCallback_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnAuthResult error, authenticate callback not register for pkgName %s.",
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
    tempCbk->OnAuthResult(deviceId, token, status, (int32_t)reason);
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        authenticateCallback_[pkgName].erase(deviceId);
        if (authenticateCallback_[pkgName].empty()) {
            authenticateCallback_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::OnVerifyAuthResult(const std::string &pkgName, const std::string &deviceId,
                                             int32_t resultCode, int32_t flag)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return;
    }
    LOGI("DeviceManagerNotify::OnVerifyAuthResult in, pkgName:%s, resultCode:%d, flag:%d", pkgName.c_str(),
        resultCode, flag);
    std::shared_ptr<VerifyAuthCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (verifyAuthCallback_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnVerifyAuthResult error, verify auth callback not register for pkgName %s.",
                pkgName.c_str());
            return;
        }
        tempCbk = verifyAuthCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnVerifyAuthResult error, registered verify auth callback is nullptr.");
        return;
    }
    tempCbk->OnVerifyAuthResult(deviceId, resultCode, flag);
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        verifyAuthCallback_.erase(pkgName);
    }
}

void DeviceManagerNotify::OnUiCall(std::string &pkgName, std::string &paramJson)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerNotify::OnUiCall error: Invalid parameter, pkgName: %s", pkgName.c_str());
        return;
    }
    LOGI("DeviceManagerNotify::OnUiCall in, pkgName:%s", pkgName.c_str());
    std::shared_ptr<DeviceManagerUiCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (dmUiCallback_.count(pkgName) == 0) {
            LOGE("OnUiCall error, dm Ui callback not register for pkgName %d.", pkgName.c_str());
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
        LOGE("DeviceManagerNotify::OnCredentialResult error: Invalid parameter, pkgName: %s", pkgName.c_str());
        return;
    }
    LOGI("DeviceManagerNotify::OnCredentialResult in, pkgName:%s, action:%d", pkgName.c_str(), action);
    std::shared_ptr<CredentialCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (credentialCallback_.count(pkgName) == 0) {
            LOGE("DeviceManagerNotify::OnCredentialResult error, credential callback not register for pkgName %s.",
                pkgName.c_str());
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
} // namespace DistributedHardware
} // namespace OHOS
