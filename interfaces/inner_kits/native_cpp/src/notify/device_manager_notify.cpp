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

#include "device_manager_notify.h"

#include "device_manager.h"
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerNotify);

void DeviceManagerNotify::RegisterDeathRecipientCallback(const std::string &pkgName,
                                                         std::shared_ptr<DmInitCallback> dmInitCallback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmInitCallback_[pkgName] = dmInitCallback;
}

void DeviceManagerNotify::UnRegisterDeathRecipientCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmInitCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDeviceStateCallback(const std::string &pkgName,
                                                      std::shared_ptr<DeviceStateCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterDeviceStateCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    deviceStateCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDiscoveryCallback(const std::string &pkgName, uint16_t subscribeId,
                                                    std::shared_ptr<DiscoveryCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
        deviceDiscoveryCallbacks_[pkgName] = std::map<uint16_t, std::shared_ptr<DiscoveryCallback>>();
    }
    deviceDiscoveryCallbacks_[pkgName][subscribeId] = callback;
}

void DeviceManagerNotify::UnRegisterDiscoveryCallback(const std::string &pkgName, uint16_t subscribeId)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (deviceDiscoveryCallbacks_.count(pkgName) > 0) {
        deviceDiscoveryCallbacks_[pkgName].erase(subscribeId);
        if (deviceDiscoveryCallbacks_[pkgName].empty()) {
            deviceDiscoveryCallbacks_.erase(pkgName);
        }
    }
}

void DeviceManagerNotify::RegisterPublishCallback(const std::string               &pkgName,
                                                  int32_t                          publishId,
                                                  std::shared_ptr<PublishCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    if (devicePublishCallbacks_.count(pkgName) == 0) {
        devicePublishCallbacks_[pkgName] = std::map<int32_t, std::shared_ptr<PublishCallback>>();
    }
    devicePublishCallbacks_[pkgName][publishId] = callback;
}

void DeviceManagerNotify::UnRegisterPublishCallback(const std::string &pkgName, int32_t publishId)
{
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
    std::lock_guard<std::mutex> autoLock(lock_);
    if (authenticateCallback_.count(pkgName) == 0) {
        authenticateCallback_[pkgName] = std::map<std::string, std::shared_ptr<AuthenticateCallback>>();
    }
    authenticateCallback_[pkgName][deviceId] = callback;
}

void DeviceManagerNotify::UnRegisterAuthenticateCallback(const std::string &pkgName, const std::string &deviceId)
{
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
    std::lock_guard<std::mutex> autoLock(lock_);
    verifyAuthCallback_.erase(pkgName);
}

void DeviceManagerNotify::RegisterDeviceManagerFaCallback(const std::string &pkgName,
                                                          std::shared_ptr<DeviceManagerFaCallback> callback)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmFaCallback_[pkgName] = callback;
}

void DeviceManagerNotify::UnRegisterDeviceManagerFaCallback(const std::string &pkgName)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    dmFaCallback_.erase(pkgName);
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
        if (iter.second != nullptr) {
            iter.second->OnRemoteDied();
        }
    }
}

void DeviceManagerNotify::OnDeviceOnline(const std::string &pkgName, const DmDeviceInfo &deviceInfo)
{
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
    LOGI("DeviceManagerNotify::OnDeviceFound in, pkgName:%s, subscribeId:%d.", pkgName.c_str(), (int32_t)subscribeId);
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("OnDeviceFound error, device discovery callback not register for pkgName %d.", pkgName.c_str());
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
    LOGI("DeviceManagerNotify::OnDiscoveryFailed in, pkgName:%s, subscribeId %d, failed reason %d", pkgName.c_str(),
        subscribeId, failedReason);
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("OnDiscoveryFailed error, device discovery callback not register for pkgName %d.", pkgName.c_str());
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
    LOGI("DeviceManagerNotify::OnDiscoverySuccess in, pkgName:%s, subscribeId:%d.", pkgName.c_str(), subscribeId);
    std::shared_ptr<DiscoveryCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (deviceDiscoveryCallbacks_.count(pkgName) == 0) {
            LOGE("OnDiscoverySuccess error, device discovery callback not register for pkgName %d.", pkgName.c_str());
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
    LOGI("DeviceManagerNotify::OnPublishResult in, pkgName:%s, publishId %d, publishResult %d", pkgName.c_str(),
        publishId, publishResult);
    std::shared_ptr<PublishCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (devicePublishCallbacks_.count(pkgName) == 0) {
            LOGE("OnPublishResult error, device publish callback not register for pkgName %d.", pkgName.c_str());
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
    LOGI("DeviceManagerNotify::OnAuthResult in, pkgName:%s, status:%d, reason:%u", pkgName.c_str(), status, reason);
    std::shared_ptr<AuthenticateCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (authenticateCallback_.count(pkgName) == 0) {
            LOGE("OnAuthResult error, authenticate callback not register for pkgName %d.", pkgName.c_str());
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
    LOGI("DeviceManagerNotify::OnVerifyAuthResult in, pkgName:%s, resultCode:%d, flag:%d", pkgName.c_str(),
        resultCode, flag);
    std::shared_ptr<VerifyAuthCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (verifyAuthCallback_.count(pkgName) == 0) {
            LOGE("OnVerifyAuthResult error, verify auth callback not register for pkgName %d.", pkgName.c_str());
            return;
        }
        tempCbk = verifyAuthCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnVerifyAuthResult error, registered verify auth callback is nullptr.");
        return;
    }
    tempCbk->OnVerifyAuthResult(deviceId, resultCode, flag);
    verifyAuthCallback_.erase(pkgName);
}

void DeviceManagerNotify::OnFaCall(std::string &pkgName, std::string &paramJson)
{
    LOGI("DeviceManagerNotify::OnFaCall in, pkgName:%s", pkgName.c_str());
    std::shared_ptr<DeviceManagerFaCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (dmFaCallback_.count(pkgName) == 0) {
            LOGE("OnFaCall error, dm fa callback not register for pkgName %d.", pkgName.c_str());
            return;
        }
        tempCbk = dmFaCallback_[pkgName];
    }
    if (tempCbk == nullptr) {
        LOGE("OnFaCall error, registered dm fa callback is nullptr.");
        return;
    }
    tempCbk->OnCall(paramJson);
}

void DeviceManagerNotify::OnCredentialResult(const std::string &pkgName, int32_t &action,
                                             const std::string &credentialResult)
{
    LOGI("DeviceManagerNotify::OnCredentialResult in, pkgName:%s, action:%d", pkgName.c_str(), action);
    std::shared_ptr<CredentialCallback> tempCbk;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (credentialCallback_.count(pkgName) == 0) {
            LOGE("OnCredentialResult error, credential callback not register for pkgName %d.", pkgName.c_str());
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
