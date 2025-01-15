/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dm_softbus_cache_test_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DH_SUCCESS = 0;
int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info)
{
    NodeBasicInfo nodeBasicInfo = {
        .networkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00",
        .deviceName = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591",
        .deviceTypeId = 1,
        .osType = 24,
        .osVersion = "1"
    };
    (void)pkgName;
    *info = nodeBasicInfo;
    return DH_SUCCESS;
}

int32_t GetAllNodeDeviceInfo(const char *pkgName, NodeBasicInfo **info, int32_t *infoNum)
{
    (void)pkgName;
    (void)info;
    (void)infoNum;
    return 0;
}

int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId,
    NodeDeviceInfoKey key, uint8_t *info, int32_t infoLen)
{
    (void)pkgName;
    (void)networkId;
    (void)key;
    (void)info;
    (void)infoLen;
    return DH_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS