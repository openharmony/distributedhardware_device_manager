/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <fuzzer/FuzzedDataProvider.h>
#include <memory>
#include <string>

#include "device_manager_notify_fuzzer.h"
#include "notify/device_manager_notify.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 256;

// Mock callback classes for testing
class AuthCodeInvalidCallbackTest : public AuthCodeInvalidCallback {
public:
    AuthCodeInvalidCallbackTest() = default;
    virtual ~AuthCodeInvalidCallbackTest() = default;
    void OnAuthCodeInvalid() override {}
};

class ServicePublishCallbackTest : public ServicePublishCallback {
public:
    ServicePublishCallbackTest() = default;
    virtual ~ServicePublishCallbackTest() = default;
    void OnServicePublishSuccess(int64_t publishId) override {}
    void OnServicePublishFailure(int64_t publishId, int32_t errorCode) override {}
};

class ServiceDiscoveryCallbackTest : public ServiceDiscoveryCallback {
public:
    ServiceDiscoveryCallbackTest() = default;
    virtual ~ServiceDiscoveryCallbackTest() = default;
    void OnServiceDiscoverySuccess(int64_t discoverId, const std::string &udid) override {}
    void OnServiceDiscoveryFailure(int64_t discoverId, int32_t errorCode) override {}
};

class SyncServiceInfoCallbackTest : public SyncServiceInfoCallback {
public:
    SyncServiceInfoCallbackTest() = default;
    virtual ~SyncServiceInfoCallbackTest() = default;
    void OnSyncServiceInfoSuccess(const std::string &networkId, int64_t serviceId) override {}
    void OnSyncServiceInfoFailure(const std::string &networkId, int64_t serviceId, int32_t errorCode) override {}
};

class ServiceInfoStateCallbackTest : public ServiceInfoStateCallback {
public:
    ServiceInfoStateCallbackTest() = default;
    virtual ~ServiceInfoStateCallbackTest() = default;
    void OnServiceStateChange(int64_t serviceId, const std::string &udid, int32_t serviceState) override {}
};

class LeaveLNNCallbackTest : public LeaveLNNCallback {
public:
    LeaveLNNCallbackTest() = default;
    virtual ~LeaveLNNCallbackTest() = default;
    void OnLeaveLNNResult(const std::string &networkId, int32_t retCode) override {}
};

void AuthCodeInvalidFuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<AuthCodeInvalidCallbackTest> callback = std::make_shared<AuthCodeInvalidCallbackTest>();
    DeviceManagerNotify::AuthCodeInvalid(callback);
}

void RegisterServicePublishCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    std::shared_ptr<ServicePublishCallbackTest> callback = std::make_shared<ServicePublishCallbackTest>();

    DeviceManagerNotify::GetInstance().RegisterServicePublishCallback(pkgName, serviceId, callback);
}

void UnRegisterServicePublishCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceManagerNotify::GetInstance().UnRegisterServicePublishCallback(pkgName, serviceId);
}

void OnServicePublishResultFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    int32_t publishResult = fdp.ConsumeIntegral<int32_t>();

    DeviceManagerNotify::GetInstance().OnServicePublishResult(pkgName, serviceId, publishResult);
}

void RegisterServiceDiscoveryCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::shared_ptr<ServiceDiscoveryCallbackTest> callback = std::make_shared<ServiceDiscoveryCallbackTest>();

    DeviceManagerNotify::GetInstance().RegisterServiceDiscoveryCallback(pkgName, serviceType, callback);
}

void OnServiceDiscoveryResultFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int32_t resReason = fdp.ConsumeIntegral<int32_t>();

    DeviceManagerNotify::GetInstance().OnServiceDiscoveryResult(pkgName, serviceType, resReason);
}

void OnServiceFoundFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    DmServiceInfo dmServiceInfo;
    dmServiceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    dmServiceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.authform = static_cast<DmAuthForm>(fdp.ConsumeIntegral<int32_t>());
    dmServiceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    dmServiceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    dmServiceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    dmServiceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.publishState = fdp.ConsumeIntegral<int8_t>();

    DeviceManagerNotify::GetInstance().OnServiceFound(pkgName, dmServiceInfo);
}

void RegisterSyncServiceInfoCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int32_t localUserId = fdp.ConsumeIntegral<int32_t>();
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    std::shared_ptr<SyncServiceInfoCallbackTest> callback = std::make_shared<SyncServiceInfoCallbackTest>();

    DeviceManagerNotify::GetInstance().RegisterSyncServiceInfoCallback(pkgName,
            localUserId, networkId, callback, serviceId);
}

void UnRegisterSyncServiceInfoCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int32_t localUserId = fdp.ConsumeIntegral<int32_t>();
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceManagerNotify::GetInstance().UnRegisterSyncServiceInfoCallback(pkgName, localUserId, networkId, serviceId);
}

void OnSyncServiceInfoResultFuzzTest(FuzzedDataProvider &fdp)
{
    ServiceSyncInfo serviceSyncInfo;
    serviceSyncInfo.pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceSyncInfo.localUserId = fdp.ConsumeIntegral<int32_t>();
    serviceSyncInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceSyncInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceSyncInfo.callerUserId = fdp.ConsumeIntegral<int32_t>();
    serviceSyncInfo.callerTokenId = fdp.ConsumeIntegral<uint32_t>();

    int32_t result = fdp.ConsumeIntegral<int32_t>();
    std::string content = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    DeviceManagerNotify::GetInstance().OnSyncServiceInfoResult(serviceSyncInfo, result, content);
}

void GetServiceCallBackFuzzTest(FuzzedDataProvider &fdp)
{
    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> serviceCallbackMap;

    size_t mapCount = fdp.ConsumeIntegralInRange<size_t>(0, 3);
    for (size_t i = 0; i < mapCount; ++i) {
        DmCommonNotifyEvent event = static_cast<DmCommonNotifyEvent>(fdp.ConsumeIntegral<int32_t>());
        std::set<std::pair<std::string, int64_t>> callbackSet;

        size_t setCount = fdp.ConsumeIntegralInRange<size_t>(0, 3);
        for (size_t j = 0; j < setCount; ++j) {
            callbackSet.insert({fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH), fdp.ConsumeIntegral<int64_t>()});
        }

        serviceCallbackMap[event] = callbackSet;
    }

    DeviceManagerNotify::GetInstance().GetServiceCallBack(serviceCallbackMap);
}

void OnServiceOnlineFuzzTest(FuzzedDataProvider &fdp)
{
    DmRegisterServiceState registerServiceState;
    registerServiceState.userId = fdp.ConsumeIntegral<int32_t>();
    registerServiceState.tokenId = fdp.ConsumeIntegral<uint64_t>();
    registerServiceState.pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    registerServiceState.serviceId = fdp.ConsumeIntegral<int64_t>();

    DmServiceInfo serviceInfo;
    serviceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.authform = static_cast<DmAuthForm>(fdp.ConsumeIntegral<int32_t>());
    serviceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    serviceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    serviceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    serviceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.publishState = fdp.ConsumeIntegral<int8_t>();

    DeviceManagerNotify::GetInstance().OnServiceOnline(registerServiceState, serviceInfo);
}

void ServiceInfoOnlineFuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<ServiceInfoStateCallbackTest> callback = std::make_shared<ServiceInfoStateCallbackTest>();

    DmServiceInfo dmServiceInfo;
    dmServiceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    dmServiceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.authform = static_cast<DmAuthForm>(fdp.ConsumeIntegral<int32_t>());
    dmServiceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    dmServiceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    dmServiceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    dmServiceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.publishState = fdp.ConsumeIntegral<int8_t>();

    DeviceManagerNotify::ServiceInfoOnline(callback, dmServiceInfo);
}

void OnServiceOfflineFuzzTest(FuzzedDataProvider &fdp)
{
    DmRegisterServiceState registerServiceState;
    registerServiceState.userId = fdp.ConsumeIntegral<int32_t>();
    registerServiceState.tokenId = fdp.ConsumeIntegral<uint64_t>();
    registerServiceState.pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    registerServiceState.serviceId = fdp.ConsumeIntegral<int64_t>();

    DmServiceInfo serviceInfo;
    serviceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.authform = static_cast<DmAuthForm>(fdp.ConsumeIntegral<int32_t>());
    serviceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    serviceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    serviceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    serviceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.publishState = fdp.ConsumeIntegral<int8_t>();

    DeviceManagerNotify::GetInstance().OnServiceOffline(registerServiceState, serviceInfo);
}

void ServiceInfoOfflineFuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<ServiceInfoStateCallbackTest> callback = std::make_shared<ServiceInfoStateCallbackTest>();

    DmServiceInfo dmServiceInfo;
    dmServiceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    dmServiceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.authform = static_cast<DmAuthForm>(fdp.ConsumeIntegral<int32_t>());
    dmServiceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    dmServiceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    dmServiceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    dmServiceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.publishState = fdp.ConsumeIntegral<int8_t>();

    DeviceManagerNotify::ServiceInfoOffline(callback, dmServiceInfo);
}

void OnServiceChangeFuzzTest(FuzzedDataProvider &fdp)
{
    DmRegisterServiceState registerServiceState;
    registerServiceState.userId = fdp.ConsumeIntegral<int32_t>();
    registerServiceState.tokenId = fdp.ConsumeIntegral<uint64_t>();
    registerServiceState.pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    registerServiceState.serviceId = fdp.ConsumeIntegral<int64_t>();

    DmServiceInfo serviceInfo;
    serviceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    serviceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.authform = static_cast<DmAuthForm>(fdp.ConsumeIntegral<int32_t>());
    serviceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    serviceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    serviceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    serviceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    serviceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    serviceInfo.publishState = fdp.ConsumeIntegral<int8_t>();

    DeviceManagerNotify::GetInstance().OnServiceChange(registerServiceState, serviceInfo);
}

void ServiceInfoChangeFuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<ServiceInfoStateCallbackTest> callback = std::make_shared<ServiceInfoStateCallbackTest>();

    DmServiceInfo dmServiceInfo;
    dmServiceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    dmServiceInfo.serviceId = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.authform = static_cast<DmAuthForm>(fdp.ConsumeIntegral<int32_t>());
    dmServiceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    dmServiceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    dmServiceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    dmServiceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    dmServiceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    dmServiceInfo.publishState = fdp.ConsumeIntegral<int8_t>();

    DeviceManagerNotify::ServiceInfoChange(callback, dmServiceInfo);
}

void RegisterServiceStateCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    std::shared_ptr<ServiceInfoStateCallbackTest> callback = std::make_shared<ServiceInfoStateCallbackTest>();

    DeviceManagerNotify::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
}

void UnRegisterServiceStateCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceManagerNotify::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
}

void RegisterLeaveLnnCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::shared_ptr<LeaveLNNCallbackTest> callback = std::make_shared<LeaveLNNCallbackTest>();

    DeviceManagerNotify::GetInstance().RegisterLeaveLnnCallback(networkId, callback);
}

void OnLeaveLNNResultFuzzTest(FuzzedDataProvider &fdp)
{
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int32_t retCode = fdp.ConsumeIntegral<int32_t>();

    DeviceManagerNotify::GetInstance().OnLeaveLNNResult(networkId, retCode);
}

void RegisterAuthCodeInvalidCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::shared_ptr<AuthCodeInvalidCallbackTest> callback = std::make_shared<AuthCodeInvalidCallbackTest>();

    DeviceManagerNotify::GetInstance().RegisterAuthCodeInvalidCallback(pkgName, callback);
}

void UnRegisterAuthCodeInvalidCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    DeviceManagerNotify::GetInstance().UnRegisterAuthCodeInvalidCallback(pkgName);
}

}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const size_t minSize = sizeof(int32_t) + sizeof(int64_t);
    if ((data == nullptr) || (size < minSize)) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    /* Run your code on data */
    OHOS::DistributedHardware::AuthCodeInvalidFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterServicePublishCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::UnRegisterServicePublishCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::OnServicePublishResultFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterServiceDiscoveryCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceDiscoveryResultFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceFoundFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterSyncServiceInfoCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::UnRegisterSyncServiceInfoCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::OnSyncServiceInfoResultFuzzTest(fdp);
    OHOS::DistributedHardware::GetServiceCallBackFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceOnlineFuzzTest(fdp);
    OHOS::DistributedHardware::ServiceInfoOnlineFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceOfflineFuzzTest(fdp);
    OHOS::DistributedHardware::ServiceInfoOfflineFuzzTest(fdp);
    OHOS::DistributedHardware::OnServiceChangeFuzzTest(fdp);
    OHOS::DistributedHardware::ServiceInfoChangeFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterServiceStateCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::UnRegisterServiceStateCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterLeaveLnnCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::OnLeaveLNNResultFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterAuthCodeInvalidCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::UnRegisterAuthCodeInvalidCallbackFuzzTest(fdp);
    return 0;
}
