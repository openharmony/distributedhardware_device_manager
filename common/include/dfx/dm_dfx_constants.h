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
const int DM_HISYEVENT_FAULT = 1;
const int DM_HISYEVENT_STATISTIC = 2;
const int DM_HISYEVENT_SECURITY = 3;
const int DM_HISYEVENT_BEHAVIOR = 4;

// HiDumper Flag
enum class HidumperFlag {
    HIDUMPER_UNKNOWN = 0,
    HIDUMPER_GET_HELP,
    HIDUMPER_GET_TRUSTED_LIST,
    HIDUMPER_GET_DEVICE_STATE,
};

// HiDumper info
constexpr const char* ARGS_HELP_INFO = "-help";
constexpr const char* HIDUMPER_GET_TRUSTED_LIST_INFO = "-getTrustlist";

// HiDumper command
const std::unordered_map<std::string, HidumperFlag> MAP_ARGS = {
    { std::string(ARGS_HELP_INFO), HidumperFlag::HIDUMPER_GET_HELP },
    { std::string(HIDUMPER_GET_TRUSTED_LIST_INFO), HidumperFlag::HIDUMPER_GET_TRUSTED_LIST },
};

// HiDumper device type
typedef struct DumperInfo {
    DmDeviceType deviceTypeId;
    std::string deviceTypeInfo;
} DumperInfo;

static DumperInfo dumperDeviceType[] = {
    {DEVICE_TYPE_UNKNOWN, "DEVICE_TYPE_UNKNOWN"},
    {DEVICE_TYPE_WIFI_CAMERA, "DEVICE_TYPE_WIFI_CAMERA"},
    {DEVICE_TYPE_AUDIO, "DEVICE_TYPE_AUDIO"},
    {DEVICE_TYPE_PC, "DEVICE_TYPE_PC"},
    {DEVICE_TYPE_PHONE, "DEVICE_TYPE_PHONE"},
    {DEVICE_TYPE_PAD, "DEVICE_TYPE_PAD"},
    {DEVICE_TYPE_WATCH, "DEVICE_TYPE_WATCH"},
    {DEVICE_TYPE_CAR, "DEVICE_TYPE_CAR"},
    {DEVICE_TYPE_TV, "DEVICE_TYPE_TV"},
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DFX_CONSTANTS_H
