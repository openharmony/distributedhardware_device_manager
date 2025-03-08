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
#ifndef OHOS_SOFTBUS_LISTENER_MOCK_H
#define OHOS_SOFTBUS_LISTENER_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "softbus_listener.h"

namespace OHOS {
namespace DistributedHardware {
class DmSoftbusListener {
public:
    virtual ~DmSoftbusListener() = default;
public:
    virtual int32_t ShiftLNNGear(bool isWakeUp, const std::string &callerId) = 0;
    virtual int32_t GetUdidByNetworkId(const char *networkId, std::string &udid) = 0;
    virtual int32_t GetDeviceSecurityLevel(const char *networkId, int32_t &securityLevel) = 0;
    virtual int32_t GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info) = 0;
    virtual int32_t GetUuidByNetworkId(const char *networkId, std::string &uuid) = 0;
    virtual int32_t GetTargetInfoFromCache(const std::string &deviceId, PeerTargetId &targetId,
        ConnectionAddrType &addrType) = 0;
    virtual int32_t GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList) = 0;
    virtual int32_t GetLocalDeviceInfo(DmDeviceInfo &deviceInfo) = 0;
    virtual int32_t GetDeviceScreenStatus(const char *networkId, int32_t &screenStatus) = 0;
    virtual int32_t GetIPAddrTypeFromCache(const std::string &deviceId, const std::string &ip,
        ConnectionAddrType &addrType) = 0;
    virtual int32_t GetNetworkTypeByNetworkId(const char *networkId, int32_t &networkType) = 0;
    virtual int32_t SetLocalDisplayName(const std::string &displayName) = 0;
    virtual int32_t GetUdidFromDp(const std::string &udidHash, std::string &udid) = 0;
    virtual int32_t GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
        std::vector<DmDeviceInfo> &deviceList) = 0;
    virtual int32_t StopRefreshSoftbusLNN(uint16_t subscribeId) = 0;
public:
    static inline std::shared_ptr<DmSoftbusListener> dmSoftbusListener = nullptr;
};

class SoftbusListenerMock : public DmSoftbusListener {
public:
    MOCK_METHOD(int32_t, ShiftLNNGear, (bool, const std::string &));
    MOCK_METHOD(int32_t, GetUdidByNetworkId, (const char *, std::string &));
    MOCK_METHOD(int32_t, GetDeviceSecurityLevel, (const char *, int32_t &));
    MOCK_METHOD(int32_t, GetDeviceInfo, (const std::string &, DmDeviceInfo &));
    MOCK_METHOD(int32_t, GetUuidByNetworkId, (const char *, std::string &));
    MOCK_METHOD(int32_t, GetTargetInfoFromCache, (const std::string &, PeerTargetId &, ConnectionAddrType &addrType));
    MOCK_METHOD(int32_t, GetTrustedDeviceList, (std::vector<DmDeviceInfo> &));
    MOCK_METHOD(int32_t, GetLocalDeviceInfo, (DmDeviceInfo &));
    MOCK_METHOD(int32_t, GetDeviceScreenStatus, (const char *, int32_t &));
    MOCK_METHOD(int32_t, GetIPAddrTypeFromCache, (const std::string &, const std::string &, ConnectionAddrType &));
    MOCK_METHOD(int32_t, GetNetworkTypeByNetworkId, (const char *, int32_t &));
    MOCK_METHOD(int32_t, SetLocalDisplayName, (const std::string &));
    MOCK_METHOD(int32_t, GetUdidFromDp, (const std::string &, std::string &));
    MOCK_METHOD(int32_t, GetAllTrustedDeviceList, (const std::string &, const std::string &,
        std::vector<DmDeviceInfo> &));
    MOCK_METHOD(int32_t, StopRefreshSoftbusLNN, (uint16_t));
};
}
}
#endif
