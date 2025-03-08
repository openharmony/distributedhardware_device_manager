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

#include "softbus_center_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {

SoftbusCenterMock::SoftbusCenterMock()
{
}
SoftbusCenterMock::~SoftbusCenterMock()
{
}

extern "C" {
int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId, NodeDeviceInfoKey key, uint8_t *info,
    int32_t infoLen)
{
    return SoftbusCenterInterface::softbusCenterInterface_->GetNodeKeyInfo(pkgName, networkId, key, info, infoLen);
}

int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info)
{
    return SoftbusCenterInterface::softbusCenterInterface_->GetLocalNodeDeviceInfo(pkgName, info);
}

int32_t GetAllNodeDeviceInfo(const char *pkgName, NodeBasicInfo **info, int32_t *infoNum)
{
    return SoftbusCenterInterface::softbusCenterInterface_->GetAllNodeDeviceInfo(pkgName, info, infoNum);
}
}
} //namespace DistributedHardware
} // namespace OHOS