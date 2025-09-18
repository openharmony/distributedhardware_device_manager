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

#include "device_manager_callback.h"
#include "device_manager_impl.h"
#include "dm_device_info.h"

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

class ServiceDiscoveryCallbackTest : public ServiceDiscoveryCallback {
public:
    ServiceDiscoveryCallbackTest() = default;
    virtual ~ServiceDiscoveryCallbackTest() = default;
    void OnServiceFound(const DiscoveryServiceInfo &service) override {}
    void OnServiceDiscoveryResult(int32_t resReason) override {}
};

void StopAuthenticateDeviceTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DeviceManagerImpl::GetInstance().StopAuthenticateDevice(pkgName);
    std::string emptyPkgName = "";
    DeviceManagerImpl::GetInstance().StopAuthenticateDevice(emptyPkgName);
    DeviceManagerImpl::GetInstance().OnDmServiceDied();
}

void UnBindDeviceTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    DeviceManagerImpl::GetInstance().UnBindDevice(pkgName, deviceId);
}

void ShiftLNNGearTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DeviceManagerImpl::GetInstance().ShiftLNNGear(pkgName);
}

void RegDevTrustChangeCallbackTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<DevTrustChangeCallback> callback = nullptr;
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DeviceManagerImpl::GetInstance().RegDevTrustChangeCallback(pkgName, callback);
    callback = std::make_shared<DevTrustChangeCallbackTest>();
    DeviceManagerImpl::GetInstance().RegDevTrustChangeCallback(pkgName, callback);
}

void GetNetworkIdByUdidTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string udid = fdp.ConsumeRandomLengthString();
    std::string networkId = fdp.ConsumeRandomLengthString();
    DeviceManagerImpl::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    DeviceManagerImpl::GetInstance().GetNetworkIdByUdid("pkgName", "udid", networkId);
}

void RegisterCredentialAuthStatusCallbackTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::shared_ptr<CredentialAuthStatusCallback> callback = nullptr;
    DeviceManagerImpl::GetInstance().RegisterCredentialAuthStatusCallback(pkgName, callback);
    callback = std::make_shared<CredentialAuthStatusCallbackTest>();
    DeviceManagerImpl::GetInstance().RegisterCredentialAuthStatusCallback(pkgName, callback);
    DeviceManagerImpl::GetInstance().UnRegisterCredentialAuthStatusCallback(pkgName);
    DeviceManagerImpl::GetInstance().UnRegisterCredentialAuthStatusCallback("");
}

void GetAllTrustedDeviceListTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    std::vector<DmDeviceInfo> deviceList;
    DeviceManagerImpl::GetInstance().GetAllTrustedDeviceList(pkgName, extra, deviceList);
    std::string emptyPkgName = "";
    DeviceManagerImpl::GetInstance().GetAllTrustedDeviceList(emptyPkgName, extra, deviceList);
}

void RegisterSinkBindCallbackTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::shared_ptr<BindTargetCallback> callback = nullptr;
    DeviceManagerImpl::GetInstance().RegisterSinkBindCallback(pkgName, callback);
    std::string emptyPkgName = "";
    DeviceManagerImpl::GetInstance().RegisterSinkBindCallback(emptyPkgName, callback);
}

void GetDeviceProfileInfoListTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DmDeviceProfileInfoFilterOptions filterOptions;
    std::shared_ptr<GetDeviceProfileInfoListCallback> callback = nullptr;
    DeviceManagerImpl::GetInstance().GetDeviceProfileInfoList(pkgName, filterOptions, callback);
}

void GetDeviceIconInfoTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DmDeviceIconInfoFilterOptions filterOptions;
    std::shared_ptr<GetDeviceIconInfoCallback> callback = nullptr;
    DeviceManagerImpl::GetInstance().GetDeviceIconInfo(pkgName, filterOptions, callback);
}

void PutDeviceProfileInfoListTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> deviceProfileInfoList;
    DeviceManagerImpl::GetInstance().PutDeviceProfileInfoList(pkgName, deviceProfileInfoList);
}

void GetLocalDisplayDeviceNameTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string displayName = fdp.ConsumeRandomLengthString();
    int32_t maxNameLength = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerImpl::GetInstance().GetLocalDisplayDeviceName(pkgName, maxNameLength, displayName);
}

void GetDeviceNetworkIdListTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string bundleName = fdp.ConsumeRandomLengthString();
    NetworkIdQueryFilter queryFilter;
    std::vector<std::string> networkIds;
    DeviceManagerImpl::GetInstance().GetDeviceNetworkIdList(bundleName, queryFilter, networkIds);
}

void SetLocalDeviceNameTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceName = fdp.ConsumeRandomLengthString();
    std::shared_ptr<SetLocalDeviceNameCallback> callback = nullptr;
    DeviceManagerImpl::GetInstance().SetLocalDeviceName(pkgName, deviceName, callback);
}

void SetRemoteDeviceNameTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string deviceName = fdp.ConsumeRandomLengthString();
    std::shared_ptr<SetRemoteDeviceNameCallback> callback = nullptr;
    DeviceManagerImpl::GetInstance().SetRemoteDeviceName(pkgName, deviceId, deviceName, callback);
}

void RestoreLocalDeviceNameTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DeviceManagerImpl::GetInstance().RestoreLocalDeviceName(pkgName);
    std::string emptyPkgName = "";
    DeviceManagerImpl::GetInstance().RestoreLocalDeviceName(emptyPkgName);
}

void GetLocalServiceInfoByBundleNameAndPinExchangeTypeTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string bundleName = fdp.ConsumeRandomLengthString();
    int32_t maxNameLength = fdp.ConsumeIntegral<int32_t>();
    DMLocalServiceInfo info;
    DeviceManagerImpl::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(bundleName, maxNameLength, info);
}

void GetErrorCodeTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t errCode = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerImpl::GetInstance().GetErrCode(errCode);
}

void UnRegisterPinHolderCallbackTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DeviceManagerImpl::GetInstance().UnRegisterPinHolderCallback(pkgName);
    std::string emptyPkgName = "";
    DeviceManagerImpl::GetInstance().UnRegisterPinHolderCallback(emptyPkgName);
}

void DeviceManagerImplFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string deviceName = fdp.ConsumeRandomLengthString();
    std::string deviceId = fdp.ConsumeRandomLengthString();
    std::string extra = fdp.ConsumeRandomLengthString();
    std::string bundleName = fdp.ConsumeRandomLengthString();
    int32_t pinExchangeType = fdp.ConsumeIntegral<int32_t>();
    OHOS::DistributedHardware::DmDeviceIconInfoFilterOptions filterOptions;
    DMLocalServiceInfo localServiceInfo;
    DmAccessCaller caller;
    DmAccessCallee callee;
    DMIpcCmdInterfaceCode ipcCode = REGISTER_DEVICE_MANAGER_LISTENER;
    DeviceManagerImpl::GetInstance().GetLocalDeviceName(pkgName, deviceName);
    DeviceManagerImpl::GetInstance().GetLocalDeviceName(pkgName);
    DeviceManagerImpl::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    DeviceManagerImpl::GetInstance().UnBindDevice("pkgName", "deviceId", extra);
    DeviceManagerImpl::GetInstance().UnRegisterSinkBindCallback(pkgName);
    DeviceManagerImpl::GetInstance().UnRegisterSinkBindCallback("");
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

void SyncCallbacksToServiceFuzzTest(const uint8_t* data, size_t size)
{
    int32_t maxStringLength = 64;
    if (data == nullptr || size < sizeof(int32_t) + maxStringLength) {
        return;
    }

    FuzzedDataProvider fdp(data, size);

    std::map<DmCommonNotifyEvent, std::set<std::string>> callbackMap;
    DeviceManagerImpl::GetInstance().SyncCallbacksToService(callbackMap);

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

void RegisterServiceStateCallbackFuzzTest(const uint8_t* data, size_t size)
{
    constexpr size_t minRequiredSize = sizeof(int64_t) + 1;
    if ((data == nullptr) || (size < minRequiredSize)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    std::shared_ptr<ServiceInfoStateCallback> tempCb = std::make_shared<ServiceInfoStateCallback>();

    DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, tempCb);
}

void UnRegisterServiceStateCallbackFuzzTest(const uint8_t* data, size_t size)
{
    constexpr size_t minRequiredSize = sizeof(int64_t) + 1;
    if ((data == nullptr) || (size < minRequiredSize)) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();

    DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
}

void StartServiceDiscoveryFuzzTest(const uint8_t* data, size_t size)
{
    int32_t maxStringLength = 64;
    int32_t minDataSize = sizeof(uint32_t) + maxStringLength * 2 + 1;
    if ((data == nullptr) || size < minDataSize) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    std::string pkgName = fdp.ConsumeRandomLengthString(maxStringLength);
    DiscoveryServiceParam discParam;
    discParam.serviceType = fdp.ConsumeRandomLengthString(maxStringLength);
    discParam.discoveryServiceId = fdp.ConsumeIntegral<uint32_t>();

    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    DeviceManagerImpl::GetInstance().StartServiceDiscovery(pkgName, discParam, callback);
}

void StopServiceDiscoveryFuzzTest(const uint8_t* data, size_t size)
{
    int32_t maxStringLength = 64;
    if ((data == nullptr) || (size < sizeof(int32_t) + maxStringLength)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    std::string pkgName = fdp.ConsumeRandomLengthString(maxStringLength);
    int32_t discoveryServiceId = fdp.ConsumeIntegral<int32_t>();
    DeviceManagerImpl::GetInstance().StopServiceDiscovery(pkgName, discoveryServiceId);
}

void BindServiceTargetTest(const uint8_t *data, size_t size)
{
    size_t minLenth = sizeof(int64_t) + sizeof(uint16_t);
    if ((data == nullptr) || (size < minLenth)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
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

void UnbindServiceTargetTest(const uint8_t *data, size_t size)
{
    size_t minLenth = sizeof(int64_t) + 1;
    if ((data == nullptr) || (size < minLenth)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    int64_t targetId = fdp.ConsumeIntegral<int64_t>();
    DeviceManagerImpl::GetInstance().UnbindServiceTarget(pkgName, targetId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::StopAuthenticateDeviceTest(data, size);
    OHOS::DistributedHardware::UnBindDeviceTest(data, size);
    OHOS::DistributedHardware::ShiftLNNGearTest(data, size);
    OHOS::DistributedHardware::RegDevTrustChangeCallbackTest(data, size);
    OHOS::DistributedHardware::GetNetworkIdByUdidTest(data, size);
    OHOS::DistributedHardware::RegisterCredentialAuthStatusCallbackTest(data, size);
    OHOS::DistributedHardware::GetAllTrustedDeviceListTest(data, size);
    OHOS::DistributedHardware::RegisterSinkBindCallbackTest(data, size);
    OHOS::DistributedHardware::GetDeviceProfileInfoListTest(data, size);
    OHOS::DistributedHardware::GetDeviceIconInfoTest(data, size);
    OHOS::DistributedHardware::PutDeviceProfileInfoListTest(data, size);
    OHOS::DistributedHardware::GetLocalDisplayDeviceNameTest(data, size);
    OHOS::DistributedHardware::GetDeviceNetworkIdListTest(data, size);
    OHOS::DistributedHardware::SetLocalDeviceNameTest(data, size);
    OHOS::DistributedHardware::SetRemoteDeviceNameTest(data, size);
    OHOS::DistributedHardware::RestoreLocalDeviceNameTest(data, size);
    OHOS::DistributedHardware::GetLocalServiceInfoByBundleNameAndPinExchangeTypeTest(data, size);
    OHOS::DistributedHardware::UnRegisterPinHolderCallbackTest(data, size);
    OHOS::DistributedHardware::DeviceManagerImplFuzzTest(data, size);
    OHOS::DistributedHardware::GetErrorCodeTest(data, size);
    OHOS::DistributedHardware::SyncCallbacksToServiceFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterServiceStateCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::UnRegisterServiceStateCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::StartServiceDiscoveryFuzzTest(data, size);
    OHOS::DistributedHardware::StopServiceDiscoveryFuzzTest(data, size);
    OHOS::DistributedHardware::BindServiceTargetTest(data, size);
    OHOS::DistributedHardware::UnbindServiceTargetTest(data, size);
    return 0;
}
