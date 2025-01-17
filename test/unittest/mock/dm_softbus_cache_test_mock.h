/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_SOFTBUS_CACHE_TEST_MOCK_H
#define OHOS_DM_SOFTBUS_CACHE_TEST_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "dm_softbus_cache.h"

namespace OHOS {
namespace DistributedHardware {
int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info);
int32_t GetAllNodeDeviceInfo(const char *pkgName, NodeBasicInfo **info, int32_t *infoNum);
int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId,
    NodeDeviceInfoKey key, uint8_t *info, int32_t infoLen);
}
}
#endif
