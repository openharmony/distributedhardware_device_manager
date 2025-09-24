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
#include "json_object.h"
#include "taihe/runtime.hpp"
#include "dm_device_info.h"
#include "ohos.distributedDeviceManager.proj.hpp"
#include "ohos.distributedDeviceManager.impl.hpp"

namespace ani_dmutils {

std::string GetDeviceTypeById(OHOS::DistributedHardware::DmDeviceType type);
void InsertMapParames(OHOS::DistributedHardware::JsonObject &bindParamObj,
    std::map<std::string, std::string> &bindParamMap);

} //namespace ani_dmutils
#endif

