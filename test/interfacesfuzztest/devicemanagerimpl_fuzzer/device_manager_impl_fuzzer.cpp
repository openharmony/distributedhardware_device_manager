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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <fuzzer/FuzzedDataProvider.h>

#include "device_manager_impl.h"

namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 256;

namespace {}
class DevTrustChangeCallbackTest : public DevTrustChangeCallback {
public:
    virtual ~DevTrustChangeCallbackTest() {}
    void OnDeviceTrustChange(const std::string &udid, const std::string &uuid, DmAuthForm authForm) override {}
};

class CredentialAuthStatusCallbackTest : public CredentialAuthStatusCallback {
public:
    virtual ~CredentialAuthStatusCallbackTest() {}
    void OnCredentialAuthStatus(const std::string &deviceList, uint16_t deviceTypeId, int32_t errcode) override {}
};
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

void UnBindDeviceTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DevTrustChangeCallback> trustCallback = std::make_shared<DevTrustChangeCallbackTest>();
    std::string udid = fdp.ConsumeRandomLengthString();
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<CredentialAuthStatusCallback> authCallback = std::make_shared<CredentialAuthStatusCallbackTest>();
    std::string extra = fdp.ConsumeRandomLengthString();
    std::vector<DmDeviceInfo> deviceList;
    std::shared_ptr<BindTargetCallback> bindCallback = nullptr;
    DmDeviceProfileInfoFilterOptions filterOptions;
    std::shared_ptr<GetDeviceProfileInfoListCallback> getDeviceCallback = nullptr;
    DeviceManagerImpl::GetInstance().UnBindDevice(pkgName, deviceId);
    DeviceManagerImpl::GetInstance().ShiftLNNGear(pkgName);
    DeviceManagerImpl::GetInstance().RegDevTrustChangeCallback(pkgName, trustCallback);
    DeviceManagerImpl::GetInstance().RegDevTrustChangeCallback(pkgName, trustCallback);
    DeviceManagerImpl::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    DeviceManagerImpl::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    DeviceManagerImpl::GetInstance().RegisterCredentialAuthStatusCallback(pkgName, authCallback);
    DeviceManagerImpl::GetInstance().RegisterCredentialAuthStatusCallback(pkgName, authCallback);
    DeviceManagerImpl::GetInstance().UnRegisterCredentialAuthStatusCallback(pkgName);
    DeviceManagerImpl::GetInstance().UnRegisterCredentialAuthStatusCallback(pkgName);
    DeviceManagerImpl::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    DeviceManagerImpl::GetInstance().RegisterSinkBindCallback(pkgName, bindCallback);
    DeviceManagerImpl::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions, getDeviceCallback);
    DeviceManagerImpl::GetInstance().StopAuthenticateDevice(pkgName);
    DeviceManagerImpl::GetInstance().StopAuthenticateDevice(pkgName);
}

void GetDeviceIconInfoTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DmDeviceIconInfoFilterOptions filterOptions;
    std::shared_ptr<GetDeviceIconInfoCallback> getDeviceCallback = nullptr;
    std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> deviceProfileInfoList;
    std::string displayName = fdp.ConsumeRandomLengthString();
    int32_t maxNameLength = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString();
    NetworkIdQueryFilter queryFilter;
    std::vector<std::string> networkIds;
    std::string deviceName = fdp.ConsumeRandomLengthString();
    std::shared_ptr<SetLocalDeviceNameCallback> setLocaCallback = nullptr;
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<SetRemoteDeviceNameCallback> setRemoteCallback = nullptr;
    DMLocalServiceInfo info;
    int32_t errCode = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerImpl::GetInstance().GetDeviceIconInfo(pkgName, filterOptions, getDeviceCallback);
    DeviceManagerImpl::GetInstance().PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
    DeviceManagerImpl::GetInstance().GetLocalDisplayDeviceName(pkgName, maxNameLength, displayName);
    DeviceManagerImpl::GetInstance().GetDeviceNetworkIdList(bundleName, queryFilter, networkIds);
    DeviceManagerImpl::GetInstance().SetLocalDeviceName(pkgName, deviceName, setLocaCallback);
    DeviceManagerImpl::GetInstance().SetRemoteDeviceName(pkgName, deviceId, deviceName, setRemoteCallback);
    DeviceManagerImpl::GetInstance().RestoreLocalDeviceName(pkgName);
    DeviceManagerImpl::GetInstance().GetErrCode(errCode);
    DeviceManagerImpl::GetInstance().UnRegisterPinHolderCallback(pkgName);
    DeviceManagerImpl::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(bundleName, maxNameLength,
        info);
}

void GetLocalDeviceNameFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    std::string bundleName = fdp.ConsumeRandomLengthString();
    int32_t pinExchangeType = fdp.ConsumeIntegral<int32_t>();
    OHOS::DistributedHardware::DmDeviceIconInfoFilterOptions filterOptions;
    DMLocalServiceInfo localServiceInfo;
    localServiceInfo.extraInfo = fdp.ConsumeRandomLengthString();
    DmAccessCaller caller;
    caller.extra = fdp.ConsumeRandomLengthString();
    DmAccessCallee callee;
    callee.extra = fdp.ConsumeRandomLengthString();
    DMIpcCmdInterfaceCode ipcCode = REGISTER_DEVICE_MANAGER_LISTENER;
    DeviceManagerImpl::GetInstance().GetLocalDeviceName(pkgName, deviceName);
    DeviceManagerImpl::GetInstance().GetLocalDeviceName(pkgName);
    DeviceManagerImpl::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    DeviceManagerImpl::GetInstance().UnRegisterSinkBindCallback(pkgName);
    DeviceManagerImpl::GetInstance().RegisterLocalServiceInfo(localServiceInfo);
    DeviceManagerImpl::GetInstance().UnRegisterLocalServiceInfo(bundleName, pinExchangeType);
    DeviceManagerImpl::GetInstance().UpdateLocalServiceInfo(localServiceInfo);
    DeviceManagerImpl::GetInstance().CheckAccessControl(caller, callee);
    DeviceManagerImpl::GetInstance().CheckIsSameAccount(caller, callee);
    DeviceManagerImpl::GetInstance().CheckSrcAccessControl(caller, callee);
    DeviceManagerImpl::GetInstance().CheckSinkAccessControl(caller, callee);
    DeviceManagerImpl::GetInstance().CheckSrcIsSameAccount(caller, callee);
    DeviceManagerImpl::GetInstance().CheckSinkIsSameAccount(caller, callee);
    DeviceManagerImpl::GetInstance().CheckAclByIpcCode(caller, callee, ipcCode);
}

void SyncCallbacksToServiceFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t maxStringLength = 64;
    std::map<DmCommonNotifyEvent, std::set<std::string>> callbackMap;
    int32_t minCount = 1;
    int32_t maxCount = 5;
    DmCommonNotifyEvent event = static_cast<DmCommonNotifyEvent>(fdp.ConsumeIntegral<int32_t>());
    std::set<std::string> pkgNames;
    callbackMap[event] = pkgNames;
    DeviceManagerImpl::GetInstance().SyncCallbacksToService(callbackMap);
    int32_t pkgCount = fdp.ConsumeIntegralInRange<int32_t>(minCount, maxCount);
    for (int32_t j = 0; j < pkgCount; ++j) {
        pkgNames.insert(fdp.ConsumeRandomLengthString(maxStringLength));
    }
    callbackMap[event] = pkgNames;
    DeviceManagerImpl::GetInstance().SyncCallbacksToService(callbackMap);
}
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

void DeviceManagerImplFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    UnBindDeviceTest(fdp);
    GetDeviceIconInfoTest(fdp);
    GetLocalDeviceNameFuzzTest(fdp);
    SyncCallbacksToServiceFuzzTest(fdp);
    RegisterServiceStateCallbackFuzzTest(fdp);
    UnRegisterServiceStateCallbackFuzzTest(fdp);
    GetLocalServiceInfoByBundleNameAndPinExchangeTypeFuzzTest(fdp);
    ConvertLocalServiceInfoToAuthInfoFuzzTest(fdp);
    RegisterServiceInfoFuzzTest(fdp);
    UnRegisterServiceInfoFuzzTest(fdp);
    StartPublishServiceFuzzTest(fdp);
    StopPublishServiceFuzzTest(fdp);
    StartDiscoveryServiceFuzzTest(fdp);
    StopDiscoveryServiceFuzzTest(fdp);
    SyncCallbackToServiceForServiceInfoFuzzTest(fdp);
    SyncServiceInfoByServiceIdFuzzTest(fdp);
    SyncAllServiceInfoFuzzTest(fdp);
    GetLocalServiceInfoByServiceIdFuzzTest(fdp);
    GetTrustServiceInfoFuzzTest(fdp);
    GetRegisterServiceInfoFuzzTest(fdp);
    GetPeerServiceInfoByServiceIdFuzzTest(fdp);
    SyncServiceCallbacksToServiceFuzzTest(fdp);
    UnbindServiceTargetFuzzTest(fdp);
    UpdateServiceInfoFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DeviceManagerImplFuzzTest(data, size);
    return 0;
}
