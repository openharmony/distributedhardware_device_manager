/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_DEVICE_MANAGER_UTILS_H
#define OHOS_DEVICE_MANAGER_UTILS_H

#include <atomic>

#include "json_object.h"

#include "device_manager_callback.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {

enum DmFfiDevStatusChange { UNKNOWN = 0, AVAILABLE = 1, UNAVAILABLE = 2, CHANGE = 3};

class DmFfiInitCallback : public DmInitCallback {
public:
    explicit DmFfiInitCallback(const std::string &bundleName) : bundleName_(bundleName)
    {
    }
    ~DmFfiInitCallback() override = default;
    void OnRemoteDied() override;

private:
    std::string bundleName_;
};

class DmFfiDeviceStatusCallback : public DeviceStatusCallback {
public:
    explicit DmFfiDeviceStatusCallback(const std::string &bundleName) : bundleName_(bundleName)
    {
    }
    ~DmFfiDeviceStatusCallback() override = default;
    void OnDeviceOnline(const DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceReady(const DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceOffline(const DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDeviceChanged(const DmDeviceBasicInfo &deviceBasicInfo) override;
private:
    std::string bundleName_;
};

class DmFfiDiscoveryCallback : public DiscoveryCallback {
public:
    explicit DmFfiDiscoveryCallback(const std::string &bundleName): refCount_(0), bundleName_(bundleName)
    {
    }
    ~DmFfiDiscoveryCallback() override = default;
    void OnDeviceFound(uint16_t subscribeId,
        const OHOS::DistributedHardware::DmDeviceBasicInfo &deviceBasicInfo) override;
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override;
    void OnDiscoverySuccess(uint16_t subscribeId) override;
    void IncreaseRefCount();
    void DecreaseRefCount();
    int32_t GetRefCount();

private:
    std::atomic<int32_t> refCount_;
    std::string bundleName_;
};

class DmFfiPublishCallback : public PublishCallback {
public:
    explicit DmFfiPublishCallback(const std::string &bundleName): refCount_(0), bundleName_(bundleName)
    {
    }
    ~DmFfiPublishCallback() override = default;
    void OnPublishResult(int32_t publishId, int32_t publishResult) override;
    void IncreaseRefCount();
    void DecreaseRefCount();
    int32_t GetRefCount();

private:
    std::atomic<int32_t> refCount_;
    std::string bundleName_;
};

class DmFfiDeviceManagerUiCallback : public DeviceManagerUiCallback {
public:
    explicit DmFfiDeviceManagerUiCallback(const std::string &bundleName) : bundleName_(bundleName)
    {
    }
    ~DmFfiDeviceManagerUiCallback() override = default;
    void OnCall(const std::string &paramJson) override;

private:
    std::string bundleName_;
};

class DmFfiAuthenticateCallback : public AuthenticateCallback {
public:
    explicit DmFfiAuthenticateCallback(const std::string &bundleName) : bundleName_(bundleName)
    {
    }
    ~DmFfiAuthenticateCallback() override = default;
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status, int32_t reason) override;

private:
    std::string bundleName_;
};

class DmFfiBindTargetCallback : public BindTargetCallback {
public:
    explicit DmFfiBindTargetCallback(std::string &bundleName) : bundleName_(bundleName)
    {
    }
    ~DmFfiBindTargetCallback() override = default;
    void OnBindResult(const PeerTargetId &targetId, int32_t result, int32_t status, std::string content) override;

private:
    std::string bundleName_;
};

const std::string &GetDeviceTypeById(DmDeviceType type);
char *MallocCStr(const char *in);
void InsertMapParames(JsonObject &bindParamObj, std::map<std::string, std::string> &bindParamMap);
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DEVICE_MANAGER_IMPL_H
