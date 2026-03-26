/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "dm_client_fail_to_support.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t DM_NOT_SUPPORT = 801;
DmClientFailToSupport &DmClientFailToSupport::GetInstance()
{
    static DmClientFailToSupport instance;
    return instance;
}

int32_t DmClientFailToSupport::Init()
{
    LOGI("device not support");
    return DM_NOT_SUPPORT;
}

int32_t DmClientFailToSupport::UnInit()
{
    LOGI("device not support");
    return DM_NOT_SUPPORT;
}

int32_t DmClientFailToSupport::ReInit()
{
    UnInit();
    return Init();
}

int32_t DmClientFailToSupport::GetLocalDeviceName(std::string &deviceName)
{
    LOGI("device not support");
    return DM_NOT_SUPPORT;
}
} // namespace DistributedHardware
} // namespace OHOS
