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

#include "device_manager_impl_fuzzer.h"
#include "device_manager_impl.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 256;

// Mock callback classes for testing
class ServiceInfoStateCallbackTest : public ServiceInfoStateCallback {
public:
    ServiceInfoStateCallbackTest() = default;
    virtual ~ServiceInfoStateCallbackTest() = default;
    void OnServiceStateChange(int64_t serviceId, const std::string &udid, int32_t serviceState) override {}
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

void RegisterServiceStateCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    std::shared_ptr<ServiceInfoStateCallbackTest> callback = std::make_shared<ServiceInfoStateCallbackTest>();

    DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
}

void UnRegisterServiceStateCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
}

void GetLocalServiceInfoByBundleNameAndPinExchangeTypeFuzzTest(FuzzedDataProvider &fdp)
{
    std::string bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int32_t pinExchangeType = fdp.ConsumeIntegral<int32_t>();
    DmAuthInfo dmAuthInfo;

    DeviceManagerImpl::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(
        bundleName, pinExchangeType, dmAuthInfo);
}

void ConvertLocalServiceInfoToAuthInfoFuzzTest(FuzzedDataProvider &fdp)
{
    DMLocalServiceInfo info;
    info.bundleName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.authBoxType = fdp.ConsumeIntegral<int32_t>();
    info.authType = fdp.ConsumeIntegral<int32_t>();
    info.pinExchangeType = fdp.ConsumeIntegral<int32_t>();
    info.pinCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    info.extraInfo = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    DmAuthInfo dmAuthInfo;
    DeviceManagerImpl::GetInstance().ConvertLocalServiceInfoToAuthInfo(info, dmAuthInfo);
}

void RegisterServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    regServiceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    regServiceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    regServiceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    regServiceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    regServiceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    regServiceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    int64_t serviceId = 0;
    DeviceManagerImpl::GetInstance().RegisterServiceInfo(regServiceInfo, serviceId);
}

void UnRegisterServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(serviceId);
}

void StartPublishServiceFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DmPublishServiceParam publishServiceParam;
    publishServiceParam.discoverMode = static_cast<DMSrvDiscoveryMode>(fdp.ConsumeIntegral<int32_t>());
    publishServiceParam.media = static_cast<DMSrvMediumType>(fdp.ConsumeIntegral<int32_t>());
    publishServiceParam.freq = static_cast<DmExchangeFreq>(fdp.ConsumeIntegral<int32_t>());

    std::shared_ptr<ServicePublishCallbackTest> callback = std::make_shared<ServicePublishCallbackTest>();

    DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId, publishServiceParam, callback);
}

void StopPublishServiceFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceManagerImpl::GetInstance().StopPublishService(pkgName, serviceId);
}

void StartDiscoveryServiceFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    DmDiscoveryServiceParam discParam;
    discParam.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    discParam.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    discParam.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    discParam.freq = static_cast<DmExchangeFreq>(fdp.ConsumeIntegral<int32_t>());
    discParam.medium = static_cast<DMSrvMediumType>(fdp.ConsumeIntegral<int32_t>());
    discParam.mode = static_cast<DMSrvDiscoveryMode>(fdp.ConsumeIntegral<int32_t>());

    std::shared_ptr<ServiceDiscoveryCallbackTest> callback = std::make_shared<ServiceDiscoveryCallbackTest>();

    DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
}

void StopDiscoveryServiceFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    DmDiscoveryServiceParam discParam;
    discParam.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    discParam.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    discParam.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    discParam.freq = static_cast<DmExchangeFreq>(fdp.ConsumeIntegral<int32_t>());
    discParam.medium = static_cast<DMSrvMediumType>(fdp.ConsumeIntegral<int32_t>());
    discParam.mode = static_cast<DMSrvDiscoveryMode>(fdp.ConsumeIntegral<int32_t>());

    DeviceManagerImpl::GetInstance().StopDiscoveryService(pkgName, discParam);
}

void SyncCallbackToServiceForServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    DmCommonNotifyEvent dmCommonNotifyEvent = static_cast<DmCommonNotifyEvent>(fdp.ConsumeIntegral<int32_t>());
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(dmCommonNotifyEvent, pkgName, serviceId);
}

void SyncServiceInfoByServiceIdFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int32_t localUserId = fdp.ConsumeIntegral<int32_t>();
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    std::shared_ptr<SyncServiceInfoCallbackTest> callback = std::make_shared<SyncServiceInfoCallbackTest>();

    DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId, networkId, serviceId, callback);
}

void SyncAllServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int32_t localUserId = fdp.ConsumeIntegral<int32_t>();
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::shared_ptr<SyncServiceInfoCallbackTest> callback = std::make_shared<SyncServiceInfoCallbackTest>();

    DeviceManagerImpl::GetInstance().SyncAllServiceInfo(pkgName, localUserId, networkId, callback);
}

void GetLocalServiceInfoByServiceIdFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    DmRegisterServiceInfo serviceInfo;

    DeviceManagerImpl::GetInstance().GetLocalServiceInfoByServiceId(serviceId, serviceInfo);
}

void GetTrustServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::map<std::string, std::string> param;
    size_t paramMapCount = fdp.ConsumeIntegralInRange<size_t>(0, 5);
    for (size_t i = 0; i < paramMapCount; ++i) {
        param.insert({fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH),
                fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH)});
    }

    std::vector<DmServiceInfo> serviceInfoList;

    DeviceManagerImpl::GetInstance().GetTrustServiceInfo(pkgName, param, serviceInfoList);
}

void GetRegisterServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    std::map<std::string, std::string> param;
    size_t paramMapCount = fdp.ConsumeIntegralInRange<size_t>(0, 5);
    for (size_t i = 0; i < paramMapCount; ++i) {
        param.insert({fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH),
                fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH)});
    }

    std::vector<DmRegisterServiceInfo> regServiceInfos;

    DeviceManagerImpl::GetInstance().GetRegisterServiceInfo(param, regServiceInfos);
}

void GetPeerServiceInfoByServiceIdFuzzTest(FuzzedDataProvider &fdp)
{
    std::string networkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    DmRegisterServiceInfo serviceInfo;

    DeviceManagerImpl::GetInstance().GetPeerServiceInfoByServiceId(networkId, serviceId, serviceInfo);
}

void SyncServiceCallbacksToServiceFuzzTest(FuzzedDataProvider &fdp)
{
    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> callbackMap;

    size_t mapCount = fdp.ConsumeIntegralInRange<size_t>(0, 3);
    for (size_t i = 0; i < mapCount; ++i) {
        DmCommonNotifyEvent event = static_cast<DmCommonNotifyEvent>(fdp.ConsumeIntegral<int32_t>());
        std::set<std::pair<std::string, int64_t>> callbackSet;

        size_t setCount = fdp.ConsumeIntegralInRange<size_t>(0, 3);
        for (size_t j = 0; j < setCount; ++j) {
            callbackSet.insert({fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH), fdp.ConsumeIntegral<int64_t>()});
        }

        callbackMap[event] = callbackSet;
    }

    DeviceManagerImpl::GetInstance().SyncServiceCallbacksToService(callbackMap);
}

void UnbindServiceTargetFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::map<std::string, std::string> unbindParam;
    size_t mapCount = fdp.ConsumeIntegralInRange<size_t>(0, 5);
    for (size_t i = 0; i < mapCount; ++i) {
        unbindParam.insert({fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH),
            fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH)});
    }

    std::string netWorkId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceManagerImpl::GetInstance().UnbindServiceTarget(pkgName, unbindParam, netWorkId, serviceId);
}

void UpdateServiceInfoFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.userId = fdp.ConsumeIntegral<int32_t>();
    regServiceInfo.displayId = fdp.ConsumeIntegral<int64_t>();
    regServiceInfo.serviceOwnerTokenId = fdp.ConsumeIntegral<uint64_t>();
    regServiceInfo.serviceOwnerPkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceRegisterTokenId = fdp.ConsumeIntegral<uint64_t>();
    regServiceInfo.serviceType = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceDisplayName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.serviceCode = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.customData = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    regServiceInfo.dataLen = fdp.ConsumeIntegral<uint32_t>();
    regServiceInfo.timeStamp = fdp.ConsumeIntegral<int64_t>();
    regServiceInfo.description = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    DeviceManagerImpl::GetInstance().UpdateServiceInfo(serviceId, regServiceInfo);
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
    OHOS::DistributedHardware::RegisterServiceStateCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::UnRegisterServiceStateCallbackFuzzTest(fdp);
    OHOS::DistributedHardware::GetLocalServiceInfoByBundleNameAndPinExchangeTypeFuzzTest(fdp);
    OHOS::DistributedHardware::ConvertLocalServiceInfoToAuthInfoFuzzTest(fdp);
    OHOS::DistributedHardware::RegisterServiceInfoFuzzTest(fdp);
    OHOS::DistributedHardware::UnRegisterServiceInfoFuzzTest(fdp);
    OHOS::DistributedHardware::StartPublishServiceFuzzTest(fdp);
    OHOS::DistributedHardware::StopPublishServiceFuzzTest(fdp);
    OHOS::DistributedHardware::StartDiscoveryServiceFuzzTest(fdp);
    OHOS::DistributedHardware::StopDiscoveryServiceFuzzTest(fdp);
    OHOS::DistributedHardware::SyncCallbackToServiceForServiceInfoFuzzTest(fdp);
    OHOS::DistributedHardware::SyncServiceInfoByServiceIdFuzzTest(fdp);
    OHOS::DistributedHardware::SyncAllServiceInfoFuzzTest(fdp);
    OHOS::DistributedHardware::GetLocalServiceInfoByServiceIdFuzzTest(fdp);
    OHOS::DistributedHardware::GetTrustServiceInfoFuzzTest(fdp);
    OHOS::DistributedHardware::GetRegisterServiceInfoFuzzTest(fdp);
    OHOS::DistributedHardware::GetPeerServiceInfoByServiceIdFuzzTest(fdp);
    OHOS::DistributedHardware::SyncServiceCallbacksToServiceFuzzTest(fdp);
    OHOS::DistributedHardware::UnbindServiceTargetFuzzTest(fdp);
    OHOS::DistributedHardware::UpdateServiceInfoFuzzTest(fdp);
    return 0;
}
