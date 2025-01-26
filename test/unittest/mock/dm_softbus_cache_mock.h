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
#ifndef OHOS_DM_SOFTBUS_CACHE_MOCK_H
#define OHOS_DM_SOFTBUS_CACHE_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "dm_softbus_cache.h"

namespace OHOS {
namespace DistributedHardware {
class DmSoftbusCache {
public:
    virtual ~DmSoftbusCache() = default;
public:
    virtual int32_t GetUdidByUdidHash(const std::string &udidHash, std::string &udid) = 0;
    virtual int32_t GetUdidFromCache(const char *networkId, std::string &udid) = 0;
    virtual int32_t GetNetworkIdFromCache(const std::string &udid, std::string &networkId) = 0;
    virtual int32_t GetLocalDeviceInfo(DmDeviceInfo &nodeInfo) = 0;
public:
    static inline std::shared_ptr<DmSoftbusCache> dmSoftbusCache = nullptr;
};

class SoftbusCacheMock : public DmSoftbusCache {
public:
    MOCK_METHOD(int32_t, GetUdidByUdidHash, (const std::string &, std::string &));
    MOCK_METHOD(int32_t, GetUdidFromCache, (const char *, std::string &));
    MOCK_METHOD(int32_t, GetNetworkIdFromCache, (const std::string &, std::string &));
    MOCK_METHOD(int32_t, GetLocalDeviceInfo, (DmDeviceInfo &));
};
}
}
#endif
