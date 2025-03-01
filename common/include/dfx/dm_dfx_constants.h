/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_DFX_CONSTANTS_H
#define OHOS_DM_DFX_CONSTANTS_H

#include "dm_device_info.h"

#include <string>
#include <unordered_map>

namespace OHOS {
namespace DistributedHardware {
// HisysEvent Type
enum HisysEventType {
    DM_HISYEVENT_FAULT = 1,
    DM_HISYEVENT_STATISTIC = 2,
    DM_HISYEVENT_SECURITY = 3,
    DM_HISYEVENT_BEHAVIOR = 4
};

// HiDumper Flag
enum class HidumperFlag {
    HIDUMPER_UNKNOWN = 0,
    HIDUMPER_GET_HELP,
    HIDUMPER_GET_TRUSTED_LIST,
    HIDUMPER_GET_DEVICE_STATE,
};

// HiDumper device type
typedef struct DumperInfo {
    DmDeviceType deviceTypeId;
    std::string deviceTypeInfo;
} DumperInfo;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DFX_CONSTANTS_H
