/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_ANI_DM_UTILS_H
#define OHOS_ANI_DM_UTILS_H

#include <mutex>
#include <map>
#include <string>
#include "ani_utils.h"
#include "json_object.h"
#include "taihe/runtime.hpp"
#include "dm_device_info.h"
#include "dm_device_profile_info.h"
#include "ohos.distributedDeviceManager.proj.hpp"
#include "ohos.distributedDeviceManager.impl.hpp"

namespace ani_dmutils {

bool CheckJsParamStringValid(const std::string &param);

std::string GetDeviceTypeById(const OHOS::DistributedHardware::DmDeviceType &type);
void InsertMapParams(const OHOS::DistributedHardware::JsonObject &bindParamObj,
    std::map<std::string, std::string> &bindParamMap);

ani_object ServiceProfileInfoToAni(ani_env *env, const OHOS::DistributedHardware::DmServiceProfileInfo &nativeObj);
ani_object ServiceProfileInfoArrayToAni(ani_env *env,
    const std::vector<OHOS::DistributedHardware::DmServiceProfileInfo> &services);
ani_object DeviceProfileInfoToAni(ani_env *env, const OHOS::DistributedHardware::DmDeviceProfileInfo &nativeObj);
ani_object DeviceProfileInfoArrayToAni(ani_env *env,
    const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &nativeList);

void DeviceProfileInfoArrayToNative(
    const ::taihe::array_view<::ohos::distributedDeviceManager::DeviceProfileInfo> &taiheList,
    std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &nativeList);

ani_object DeviceIconInfoToAni(ani_env *env, const OHOS::DistributedHardware::DmDeviceIconInfo &nativeObj);

} //namespace ani_dmutils
#endif

