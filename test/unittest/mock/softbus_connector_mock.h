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
#ifndef OHOS_SOFTBUS_CONNECTOR_MOCK_H
#define OHOS_SOFTBUS_CONNECTOR_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "softbus_connector.h"

namespace OHOS {
namespace DistributedHardware {
class DmSoftbusConnector {
public:
    virtual ~DmSoftbusConnector() = default;
public:
    virtual int32_t GetUdidByNetworkId(const char *networkId, std::string &udid) = 0;
    virtual bool CheckIsOnline(const std::string &targetDeviceId) = 0;
    virtual std::vector<ProcessInfo> GetProcessInfo() = 0;
    virtual DmDeviceInfo GetDeviceInfoByDeviceId(const std::string &deviceId) = 0;
    virtual std::shared_ptr<SoftbusSession> GetSoftbusSession();
public:
    static inline std::shared_ptr<DmSoftbusConnector> dmSoftbusConnector = nullptr;
};

class SoftbusConnectorMock : public DmSoftbusConnector {
public:
    MOCK_METHOD(int32_t, GetUdidByNetworkId, (const char *, std::string &));
    MOCK_METHOD(bool, CheckIsOnline, (const std::string &));
    MOCK_METHOD(std::vector<ProcessInfo>, GetProcessInfo, ());
    MOCK_METHOD(DmDeviceInfo, GetDeviceInfoByDeviceId, (const std::string &deviceId));
    MOCK_METHOD(std::shared_ptr<SoftbusSession>, GetSoftbusSession, ());
};
}
}
#endif
