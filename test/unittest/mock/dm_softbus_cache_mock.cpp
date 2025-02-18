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

#include "dm_softbus_cache_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(SoftbusCache);
int32_t SoftbusCache::GetUdidByUdidHash(const std::string &udidHash, std::string &udid)
{
    return DmSoftbusCache::dmSoftbusCache->GetUdidByUdidHash(udidHash, udid);
}

int32_t SoftbusCache::GetUdidFromCache(const char *networkId, std::string &udid)
{
    return DmSoftbusCache::dmSoftbusCache->GetUdidFromCache(networkId, udid);
}

int32_t SoftbusCache::GetNetworkIdFromCache(const std::string &udid, std::string &networkId)
{
    return DmSoftbusCache::dmSoftbusCache->GetNetworkIdFromCache(udid, networkId);
}

int32_t SoftbusCache::GetLocalDeviceInfo(DmDeviceInfo &nodeInfo)
{
    return DmSoftbusCache::dmSoftbusCache->GetLocalDeviceInfo(nodeInfo);
}

int32_t SoftbusCache::GetUuidFromCache(const char *networkId, std::string &uuid)
{
    return DmSoftbusCache::dmSoftbusCache->GetUuidFromCache(networkId, uuid);
}

bool SoftbusCache::CheckIsOnline(const std::string &deviceId)
{
    return DmSoftbusCache::dmSoftbusCache->CheckIsOnline(deviceId);
}

int32_t SoftbusCache::GetDeviceInfoFromCache(std::vector<DmDeviceInfo> &deviceInfoList)
{
    return DmSoftbusCache::dmSoftbusCache->GetDeviceInfoFromCache(deviceInfoList);
}

int32_t SoftbusCache::GetDeviceNameFromCache(const std::string &udid, std::string &deviceName)
{
    return DmSoftbusCache::dmSoftbusCache->GetDeviceNameFromCache(udid, deviceName);
}
} // namespace DistributedHardware
} // namespace OHOS