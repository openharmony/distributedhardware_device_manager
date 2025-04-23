/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "softbus_connector_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {

int32_t SoftbusConnector::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    return DmSoftbusConnector::dmSoftbusConnector->GetUdidByNetworkId(networkId, udid);
}

bool SoftbusConnector::CheckIsOnline(const std::string &targetDeviceId)
{
    return DmSoftbusConnector::dmSoftbusConnector->CheckIsOnline(targetDeviceId);
}

std::vector<ProcessInfo> SoftbusConnector::GetProcessInfo()
{
    return DmSoftbusConnector::dmSoftbusConnector->GetProcessInfo();
}

DmDeviceInfo SoftbusConnector::GetDeviceInfoByDeviceId(const std::string &deviceId)
{
    return DmSoftbusConnector::dmSoftbusConnector->GetDeviceInfoByDeviceId(deviceId);
}

std::shared_ptr<SoftbusSession> SoftbusConnector::GetSoftbusSession()
{
    return DmSoftbusConnector::dmSoftbusConnector->GetSoftbusSession();
}

int32_t SoftbusConnector::SyncLocalAclListProcess(const DevUserInfo &localDevUserInfo,
                                                  const DevUserInfo &remoteDevUserInfo, std::string remoteAclList)
{
    return DmSoftbusConnector::dmSoftbusConnector->SyncLocalAclListProcess(localDevUserInfo, remoteDevUserInfo,
                                                                           remoteAclList);
}
} // namespace DistributedHardware
} // namespace OHOS