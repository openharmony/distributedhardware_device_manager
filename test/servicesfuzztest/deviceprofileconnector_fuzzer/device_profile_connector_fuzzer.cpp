/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include <random>
#include <string>

#include "deviceprofile_connector.h"
#include "device_profile_connector_fuzzer.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t NUMBER = 7;
void DeviceProfileConnectorFuzzTest(FuzzedDataProvider &fdp)
{
    std::string trustDeviceId = fdp.ConsumeRandomLengthString();
    std::string requestDeviceId = fdp.ConsumeRandomLengthString();
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string trustUdid = fdp.ConsumeRandomLengthString();
    std::string localDeviceId = fdp.ConsumeRandomLengthString();
    std::string targetDeviceId = fdp.ConsumeRandomLengthString();
    std::string requestAccountId = fdp.ConsumeRandomLengthString();
    std::string deviceIdHash = fdp.ConsumeRandomLengthString();
    std::string trustBundleName = fdp.ConsumeRandomLengthString();
    DmAclInfo aclInfo;
    aclInfo.bindType = fdp.ConsumeIntegral<int32_t>();
    aclInfo.bindLevel = fdp.ConsumeIntegral<int32_t>();
    aclInfo.trustDeviceId = trustDeviceId;
    aclInfo.deviceIdHash = deviceIdHash;
    aclInfo.authenticationType = fdp.ConsumeIntegral<int32_t>();
    DmAccesser dmAccesser;
    dmAccesser.requestDeviceId = requestDeviceId;
    dmAccesser.requestUserId = fdp.ConsumeIntegral<int32_t>();
    dmAccesser.requestAccountId = requestAccountId;
    DmAccessee dmAccessee;
    dmAccessee.trustDeviceId = trustDeviceId;
    dmAccessee.trustUserId = fdp.ConsumeIntegral<int32_t>();
    dmAccessee.trustBundleName = trustBundleName;
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string accountId = fdp.ConsumeRandomLengthString();
    DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId, targetDeviceId, userId);
    DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, dmAccesser, dmAccessee);
    DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, accountId, accountId);
    DeviceProfileConnector::GetInstance().CheckDevIdInAclForDevBind(pkgName, localDeviceId);
    DeviceProfileConnector::GetInstance().GetTrustNumber(localDeviceId);
}

void GetServiceInfoProfileByServiceIdFuzzTest(FuzzedDataProvider &fdp)
{
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    int32_t maxStringLength = 1000;
    ServiceInfoProfile serviceInfoProfile;
    serviceInfoProfile.regServiceId = fdp.ConsumeIntegral<int32_t>();
    serviceInfoProfile.deviceId = fdp.ConsumeRandomLengthString(maxStringLength);
    serviceInfoProfile.userId = fdp.ConsumeIntegral<int32_t>();
    serviceInfoProfile.tokenId = fdp.ConsumeIntegral<int64_t>();
    serviceInfoProfile.publishState = fdp.ConsumeIntegral<int8_t>();
    serviceInfoProfile.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceInfoProfile.serviceType = fdp.ConsumeRandomLengthString(maxStringLength);
    serviceInfoProfile.serviceName = fdp.ConsumeRandomLengthString(maxStringLength);
    serviceInfoProfile.serviceDisplayName = fdp.ConsumeRandomLengthString(maxStringLength);

    DeviceProfileConnector::GetInstance().GetServiceInfoProfileByServiceId(serviceId, serviceInfoProfile);
}

void PutServiceInfoProfileFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t maxStringLen = 64;
    ServiceInfoProfile serviceInfoProfile;
    serviceInfoProfile.regServiceId = fdp.ConsumeIntegral<int32_t>();
    serviceInfoProfile.deviceId = fdp.ConsumeRandomLengthString(maxStringLen);
    serviceInfoProfile.userId = fdp.ConsumeIntegral<int32_t>();
    serviceInfoProfile.tokenId = fdp.ConsumeIntegral<int64_t>();
    serviceInfoProfile.publishState = fdp.ConsumeIntegral<int8_t>();
    serviceInfoProfile.serviceId = fdp.ConsumeIntegral<int64_t>();
    serviceInfoProfile.serviceType = fdp.ConsumeRandomLengthString(maxStringLen);
    serviceInfoProfile.serviceName = fdp.ConsumeRandomLengthString(maxStringLen);
    serviceInfoProfile.serviceDisplayName = fdp.ConsumeRandomLengthString(maxStringLen);

    DeviceProfileConnector::GetInstance().PutServiceInfoProfile(serviceInfoProfile);
}

void DeleteServiceInfoProfileFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t regServiceId = fdp.ConsumeIntegral<int32_t>();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();

    DeviceProfileConnector::GetInstance().DeleteServiceInfoProfile(regServiceId, userId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const size_t minSize = sizeof(int32_t) + sizeof(int32_t) + sizeof(bool);
    if ((data == nullptr) || (size < minSize)) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    /* Run your code on data */
    OHOS::DistributedHardware::DeviceProfileConnectorFuzzTest(fdp);
    OHOS::DistributedHardware::GetServiceInfoProfileByServiceIdFuzzTest(fdp);
    OHOS::DistributedHardware::PutServiceInfoProfileFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteServiceInfoProfileFuzzTest(fdp);

    return 0;
}