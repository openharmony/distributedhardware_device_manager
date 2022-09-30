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

// HisysEvent state
constexpr const char* DM_INIT_DEVICE_MANAGER_SUCCESS = "DM_INIT_DEVICE_MANAGER_SUCCESS";
constexpr const char* DM_INIT_DEVICE_MANAGER_FAILED = "DM_INIT_DEVICE_MANAGER_FAILED";
constexpr const char* START_DEVICE_DISCOVERY_SUCCESS = "START_DEVICE_DISCOVERY_SUCCESS";
constexpr const char* START_DEVICE_DISCOVERY_FAILED = "START_DEVICE_DISCOVERY_FAILED";
constexpr const char* GET_LOCAL_DEVICE_INFO_SUCCESS = "GET_LOCAL_DEVICE_INFO_SUCCESS";
constexpr const char* GET_LOCAL_DEVICE_INFO_FAILED = "GET_LOCAL_DEVICE_INFO_FAILED";
constexpr const char* DM_SEND_REQUEST_SUCCESS = "DM_SEND_REQUEST_SUCCESS";
constexpr const char* DM_SEND_REQUEST_FAILED = "DM_SEND_REQUEST_FAILED";
constexpr const char* ADD_HICHAIN_GROUP_SUCCESS = "ADD_HICHAIN_GROUP_SUCCESS";
constexpr const char* ADD_HICHAIN_GROUP_FAILED = "ADD_HICHAIN_GROUP_FAILED";
constexpr const char* DM_CREATE_GROUP_SUCCESS = "DM_CREATE_GROUP_SUCCESS";
constexpr const char* DM_CREATE_GROUP_FAILED = "DM_CREATE_GROUP_FAILED";
constexpr const char* UNAUTHENTICATE_DEVICE_SUCCESS = "UNAUTHENTICATE_DEVICE_SUCCESS";
constexpr const char* UNAUTHENTICATE_DEVICE_FAILED = "UNAUTHENTICATE_DEVICE_FAILED";

// HisysEvent msg
constexpr const char* DM_INIT_DEVICE_MANAGER_SUCCESS_MSG = "init devicemanager success.";
constexpr const char* DM_INIT_DEVICE_MANAGER_FAILED_MSG = "init devicemanager failed.";
constexpr const char* START_DEVICE_DISCOVERY_SUCCESS_MSG = "device manager discovery success.";
constexpr const char* START_DEVICE_DISCOVERY_FAILED_MSG = "device manager discovery failed.";
constexpr const char* GET_LOCAL_DEVICE_INFO_SUCCESS_MSG = "get local device info success.";
constexpr const char* GET_LOCAL_DEVICE_INFO_FAILED_MSG = "get local device info failed.";
constexpr const char* DM_SEND_REQUEST_SUCCESS_MSG = "send request success.";
constexpr const char* DM_SEND_REQUEST_FAILED_MSG = "send request failed.";
constexpr const char* ADD_HICHAIN_GROUP_SUCCESS_MSG = "dm add member to group success.";
constexpr const char* ADD_HICHAIN_GROUP_FAILED_MSG = "dm add member to group failed.";
constexpr const char* DM_CREATE_GROUP_SUCCESS_MSG = "dm create group success.";
constexpr const char* DM_CREATE_GROUP_FAILED_MSG = "dm create group failed.";
constexpr const char* UNAUTHENTICATE_DEVICE_SUCCESS_MSG = "unauthenticate device success.";
constexpr const char* UNAUTHENTICATE_DEVICE_FAILED_MSG = "unauthenticate device failed.";

// dfx hitrace
constexpr const char* DM_HITRACE_START_DEVICE = "DM_HITRACE_START_DEVICE";
constexpr const char* DM_HITRACE_GET_LOCAL_DEVICE_INFO = "DM_HITRACE_GET_LOCAL_DEVICE_INFO";
constexpr const char* DM_HITRACE_AUTH_TO_CONSULT = "DM_HITRACE_AUTH_TO_CONSULT";
constexpr const char* DM_HITRACE_AUTH_TO_OPPEN_SESSION = "DM_HITRACE_AUTH_TO_OPPEN_SESSION";
constexpr const char* DM_HITRACE_INIT = "DM_HITRACE_INIT";

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
