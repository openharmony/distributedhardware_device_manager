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

void UnRegisterServiceStateCallbackFuzzTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
}

void StopServiceDiscoveryFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t maxStringLength = 64;
    std::string pkgName = fdp.ConsumeRandomLengthString(maxStringLength);
    int32_t discoveryServiceId = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerImpl::GetInstance().StopServiceDiscovery(pkgName, discoveryServiceId);
}

void BindServiceTargetTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::map<std::string, std::string> bindParam;
    std::shared_ptr<BindTargetCallback> callback;
    PeerTargetId targetId;
    targetId.deviceId = fdp.ConsumeRandomLengthString();
    targetId.brMac = fdp.ConsumeRandomLengthString();
    targetId.bleMac = fdp.ConsumeRandomLengthString();
    targetId.wifiIp = fdp.ConsumeRandomLengthString();
    targetId.wifiPort = fdp.ConsumeIntegral<uint16_t>();
    targetId.serviceId = fdp.ConsumeIntegral<int64_t>();
    DeviceManagerImpl::GetInstance().BindServiceTarget(pkgName, targetId, bindParam, callback);
}

void UnbindServiceTargetTest(FuzzedDataProvider &fdp)
{
    std::string pkgName = fdp.ConsumeRandomLengthString();
    int64_t targetId = fdp.ConsumeIntegral<int64_t>();
    DeviceManagerImpl::GetInstance().UnbindServiceTarget(pkgName, targetId);
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
    UnRegisterServiceStateCallbackFuzzTest(fdp);
    StopServiceDiscoveryFuzzTest(fdp);
    BindServiceTargetTest(fdp);
    UnbindServiceTargetTest(fdp);
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
