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
    int32_t peerUserId = fdp.ConsumeIntegral<int32_t>();
    std::string accountId = fdp.ConsumeRandomLengthString();
    DmOfflineParam offlineParam;
    offlineParam.bindType = fdp.ConsumeIntegral<uint32_t>();
    DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId, targetDeviceId, userId);
    DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, dmAccesser, dmAccessee);
    DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, accountId, accountId);
    DeviceProfileConnector::GetInstance().CheckDevIdInAclForDevBind(pkgName, localDeviceId);
    DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(trustUdid, peerUserId, userId, offlineParam);
    DeviceProfileConnector::GetInstance().GetTrustNumber(localDeviceId);
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
    return 0;
}
