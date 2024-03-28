/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <string>
#include <cstdlib>
#include <random>

#include "deviceprofile_connector.h"
#include "device_profile_connector_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceProfileConnectorFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(DmAccesser))) {
        return;
    }

    std::string trustDeviceId(reinterpret_cast<const char*>(data), size);
    std::string requestDeviceId(reinterpret_cast<const char*>(data), size);
    std::string pkgName(reinterpret_cast<const char*>(data), size);
    std::string trustUdid(reinterpret_cast<const char*>(data), size);
    std::string localDeviceId(reinterpret_cast<const char*>(data), size);
    std::string targetDeviceId(reinterpret_cast<const char*>(data), size);
    std::string requestAccountId(reinterpret_cast<const char*>(data), size);
    std::string deviceIdHash(reinterpret_cast<const char*>(data), size);
    std::string trustBundleName(reinterpret_cast<const char*>(data), size);
    DmAclInfo aclInfo;
    aclInfo.bindType = *(reinterpret_cast<const int32_t*>(data));
    aclInfo.bindLevel = *(reinterpret_cast<const int32_t*>(data));
    aclInfo.trustDeviceId = trustDeviceId;
    aclInfo.deviceIdHash = deviceIdHash;
    aclInfo.authenticationType = *(reinterpret_cast<const int32_t*>(data));
    DmAccesser dmAccesser;
    dmAccesser.requestDeviceId = requestDeviceId;
    dmAccesser.requestUserId = *(reinterpret_cast<const int32_t*>(data));
    dmAccesser.requestAccountId = requestAccountId;
    DmAccessee dmAccessee;
    dmAccessee.trustDeviceId = trustDeviceId;
    dmAccessee.trustUserId = *(reinterpret_cast<const int32_t*>(data));
    dmAccessee.trustBundleName = trustBundleName;
    int32_t userId = *(reinterpret_cast<const int32_t*>(data));
    std::string accountId(reinterpret_cast<const char*>(data), size);

    DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    DeviceProfileConnector::GetInstance().GetPkgNameFromAcl(localDeviceId, targetDeviceId);
    DeviceProfileConnector::GetInstance().GetOfflineParamFromAcl(trustDeviceId, requestDeviceId);
    DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, dmAccesser, dmAccessee);
    DeviceProfileConnector::GetInstance().DeleteAccessControlList(userId, accountId);
    DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId, requestDeviceId);
    DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, accountId, accountId);
    DeviceProfileConnector::GetInstance().CheckIdenticalAccount(userId, accountId);
    DeviceProfileConnector::GetInstance().DeleteP2PAccessControlList(userId, accountId);
    DeviceProfileConnector::GetInstance().CheckDeviceIdInAcl(pkgName, localDeviceId);
    DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(localDeviceId);
    DeviceProfileConnector::GetInstance().GetTrustNumber(localDeviceId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DeviceProfileConnectorFuzzTest(data, size);

    return 0;
}