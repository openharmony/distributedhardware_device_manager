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

void StopAuthenticateDeviceTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    DeviceManagerImpl::GetInstance().StopAuthenticateDevice(pkgName);
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
    return 0;
}
