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

#include <string>

namespace OHOS {
namespace DistributedHardware {
// HisysEvent Type
const int DM_HISYEVENT_FAULT = 1;
const int DM_HISYEVENT_STATISTIC = 2;
const int DM_HISYEVENT_SECURITY  = 3;
const int DM_HISYEVENT_BEHAVIOR  = 4;

// state
const std::string DM_INIT_DEVICE_MANAGER_SUCCESS = "DM_INIT_DEVICE_MANAGER_SUCCESS";
const std::string DM_INIT_DEVICE_MANAGER_FAILED = "DM_INIT_DEVICE_MANAGER_FAILED";
const std::string START_DEVICE_DISCOVERY_SUCCESS = "START_DEVICE_DISCOVERY_SUCCESS";
const std::string START_DEVICE_DISCOVERY_FAILED = "START_DEVICE_DISCOVERY_FAILED";
const std::string GET_LOCAL_DEVICE_INFO_SUCCESS = "GET_LOCAL_DEVICE_INFO_SUCCESS";
const std::string GET_LOCAL_DEVICE_INFO_FAILED = "GET_LOCAL_DEVICE_INFO_FAILED";
const std::string DM_SEND_REQUEST_SUCCESS = "DM_SEND_REQUEST_SUCCESS";
const std::string DM_SEND_REQUEST_FAILED = "DM_SEND_REQUEST_FAILED";
const std::string ADD_HICHAIN_GROUP_SUCCESS = "ADD_HICHAIN_GROUP_SUCCESS";
const std::string ADD_HICHAIN_GROUP_FAILED = "ADD_HICHAIN_GROUP_FAILED";
const std::string DM_CREATE_GROUP_SUCCESS = "DM_CREATE_GROUP_SUCCESS";
const std::string DM_CREATE_GROUP_FAILED = "DM_CREATE_GROUP_FAILED";
const std::string UNAUTHENTICATE_DEVICE_SUCCESS = "UNAUTHENTICATE_DEVICE_SUCCESS";
const std::string UNAUTHENTICATE_DEVICE_FAILED = "UNAUTHENTICATE_DEVICE_FAILED";

// msg
const std::string DM_INIT_DEVICE_MANAGER_SUCCESS_MSG = "init devicemanager success.";
const std::string DM_INIT_DEVICE_MANAGER_FAILED_MSG = "init devicemanager failed.";
const std::string START_DEVICE_DISCOVERY_SUCCESS_MSG = "device manager discovery success.";
const std::string START_DEVICE_DISCOVERY_FAILED_MSG = "device manager discovery failed.";
const std::string GET_LOCAL_DEVICE_INFO_SUCCESS_MSG = "get local device info success.";
const std::string GET_LOCAL_DEVICE_INFO_FAILED_MSG = "get local device info failed.";
const std::string DM_SEND_REQUEST_SUCCESS_MSG = "send request success.";
const std::string DM_SEND_REQUEST_FAILED_MSG = "send request failed.";
const std::string ADD_HICHAIN_GROUP_SUCCESS_MSG = "dm add member to group success.";
const std::string ADD_HICHAIN_GROUP_FAILED_MSG = "dm add member to group failed.";
const std::string DM_CREATE_GROUP_SUCCESS_MSG = "dm create group success.";
const std::string DM_CREATE_GROUP_FAILED_MSG = "dm create group failed.";
const std::string UNAUTHENTICATE_DEVICE_SUCCESS_MSG = "unauthenticate device success.";
const std::string UNAUTHENTICATE_DEVICE_FAILED_MSG = "unauthenticate device failed.";

// dfx hitrace
const std::string DM_HITRACE_START_DEVICE = "DM_HITRACE_START_DEVICE";
const std::string DM_HITRACE_GET_LOCAL_DEVICE_INFO = "DM_HITRACE_GET_LOCAL_DEVICE_INFO";
const std::string DM_HITRACE_AUTH_TO_CONSULT = "DM_HITRACE_AUTH_TO_CONSULT";
const std::string DM_HITRACE_AUTH_TO_OPPEN_SESSION = "DM_HITRACE_AUTH_TO_OPPEN_SESSION";
const std::string DM_HITRACE_DEVICE_ONLINE = "DM_HITRACE_DEVICE_ONLINE";
const std::string DM_HITRACE_INIT = "DM_HITRACE_INIT";
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DFX_CONSTANTS_H
