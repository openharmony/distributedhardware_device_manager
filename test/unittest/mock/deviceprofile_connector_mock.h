/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_DEVICEPROFILE_CONNECTOR_MOCK_H
#define OHOS_DEVICEPROFILE_CONNECTOR_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "deviceprofile_connector.h"

namespace OHOS {
namespace DistributedHardware {
class DmDeviceProfileConnector {
public:
    virtual ~DmDeviceProfileConnector() = default;
public:
    virtual std::vector<DistributedDeviceProfile::AccessControlProfile> GetAllAccessControlProfile() = 0;
    virtual int32_t HandleDevUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid,
        const std::string &localUdid) = 0;
    virtual std::string HandleAppUnBindEvent(int32_t remoteUserId, const std::string &remoteUdid, int32_t tokenId,
        const std::string &localUdid) = 0;
    virtual int32_t HandleAccountLogoutEvent(int32_t remoteUserId, const std::string &remoteAccountHash,
        const std::string &remoteUdid, const std::string &localUdid) = 0;
    virtual uint32_t CheckBindType(std::string trustDeviceId, std::string requestDeviceId) = 0;
    virtual std::vector<std::string> GetPkgNameFromAcl(std::string &localDeviceId, std::string &targetDeviceId) = 0;
    virtual DmOfflineParam GetOfflineParamFromAcl(std::string trustDeviceId, std::string requestDeviceId) = 0;
public:
    static inline std::shared_ptr<DmDeviceProfileConnector> dmDeviceProfileConnector = nullptr;
};

class DeviceProfileConnectorMock : public DmDeviceProfileConnector {
public:
    MOCK_METHOD(std::vector<DistributedDeviceProfile::AccessControlProfile>, GetAllAccessControlProfile, ());
    MOCK_METHOD(int32_t, HandleDevUnBindEvent, (int32_t, const std::string &, const std::string &));
    MOCK_METHOD(std::string, HandleAppUnBindEvent, (int32_t, const std::string &, int32_t, const std::string &));
    MOCK_METHOD(int32_t, HandleAccountLogoutEvent, (int32_t, const std::string &, const std::string &,
        const std::string &));
    MOCK_METHOD(uint32_t, CheckBindType, (std::string, std::string));
    MOCK_METHOD(std::vector<std::string>, GetPkgNameFromAcl, (std::string &, std::string &));
    MOCK_METHOD(DmOfflineParam, GetOfflineParamFromAcl, (std::string, std::string));
};
}
}
#endif