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

#include "softbus_listener_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
int32_t SoftbusListener::ShiftLNNGear(bool isWakeUp, const std::string &callerId)
{
    return DmSoftbusListener::dmSoftbusListener->ShiftLNNGear(isWakeUp, callerId);
}

int32_t SoftbusListener::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    return DmSoftbusListener::dmSoftbusListener->GetUdidByNetworkId(networkId, udid);
}

int32_t SoftbusListener::GetDeviceSecurityLevel(const char *networkId, int32_t &securityLevel)
{
    return DmSoftbusListener::dmSoftbusListener->GetDeviceSecurityLevel(networkId, securityLevel);
}

int32_t SoftbusListener::GetDeviceInfo(const std::string &networkId, DmDeviceInfo &info)
{
    return DmSoftbusListener::dmSoftbusListener->GetDeviceInfo(networkId, info);
}

int32_t SoftbusListener::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    return DmSoftbusListener::dmSoftbusListener->GetUuidByNetworkId(networkId, uuid);
}

int32_t SoftbusListener::GetTargetInfoFromCache(const std::string &deviceId, PeerTargetId &targetId,
    ConnectionAddrType &addrType)
{
    return DmSoftbusListener::dmSoftbusListener->GetTargetInfoFromCache(deviceId, targetId, addrType);
}

int32_t SoftbusListener::GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList)
{
    return DmSoftbusListener::dmSoftbusListener->GetTrustedDeviceList(deviceInfoList);
}

int32_t SoftbusListener::GetLocalDeviceInfo(DmDeviceInfo &deviceInfo)
{
    return DmSoftbusListener::dmSoftbusListener->GetLocalDeviceInfo(deviceInfo);
}

int32_t SoftbusListener::GetDeviceScreenStatus(const char *networkId, int32_t &screenStatus)
{
    return DmSoftbusListener::dmSoftbusListener->GetDeviceScreenStatus(networkId, screenStatus);
}

int32_t SoftbusListener::GetIPAddrTypeFromCache(const std::string &deviceId, const std::string &ip,
    ConnectionAddrType &addrType)
{
    return DmSoftbusListener::dmSoftbusListener->GetIPAddrTypeFromCache(deviceId, ip, addrType);
}

int32_t SoftbusListener::GetNetworkTypeByNetworkId(const char *networkId, int32_t &networkType)
{
    return DmSoftbusListener::dmSoftbusListener->GetNetworkTypeByNetworkId(networkId, networkType);
}

int32_t SoftbusListener::SetLocalDisplayName(const std::string &displayName)
{
    return DmSoftbusListener::dmSoftbusListener->SetLocalDisplayName(displayName);
}

int32_t SoftbusListener::GetUdidFromDp(const std::string &udidHash, std::string &udid)
{
    return DmSoftbusListener::dmSoftbusListener->GetUdidFromDp(udidHash, udid);
}

int32_t SoftbusListener::GetAllTrustedDeviceList(const std::string &pkgName, const std::string &extra,
    std::vector<DmDeviceInfo> &deviceList)
{
    return DmSoftbusListener::dmSoftbusListener->GetAllTrustedDeviceList(pkgName, extra, deviceList);
}

int32_t SoftbusListener::StopRefreshSoftbusLNN(uint16_t subscribeId)
{
    return DmSoftbusListener::dmSoftbusListener->StopRefreshSoftbusLNN(subscribeId);
}
} // namespace DistributedHardware
} // namespace OHOS