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

#include "deviceprofile_connector_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceProfileConnector);
std::vector<DistributedDeviceProfile::AccessControlProfile> DeviceProfileConnector::GetAllAccessControlProfile()
{
    return DmDeviceProfileConnector::dmDeviceProfileConnector->GetAllAccessControlProfile();
}

int32_t DeviceProfileConnector::HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
    const std::string &localUdid)
{
    return DmDeviceProfileConnector::dmDeviceProfileConnector->HandleDevUnBindEvent(remoteUserId, remoteUdid,
        localUdid);
}

int32_t DeviceProfileConnector::HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
    const std::string &remoteUdid, const std::string &localUdid)
{
    return DmDeviceProfileConnector::dmDeviceProfileConnector->HandleAccountLogoutEvent(remoteUserId, remoteAccountHash,
        remoteUdid, localUdid);
}

uint32_t DeviceProfileConnector::CheckBindType(std::string trustDeviceId, std::string requestDeviceId)
{
    return DmDeviceProfileConnector::dmDeviceProfileConnector->CheckBindType(trustDeviceId, requestDeviceId);
}
} // namespace DistributedHardware
} // namespace OHOS