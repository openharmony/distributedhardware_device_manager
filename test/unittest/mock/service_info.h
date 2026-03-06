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

#ifndef OHOS_DM_TEST_SERVICE_INFO_H
#define OHOS_DM_TEST_SERVICE_INFO_H

// Prefer real service_info.h through explicit paths to avoid self-include.
#if defined(__has_include)
#if __has_include("i_distributed_device_profile.h")
#include "i_distributed_device_profile.h"
#define OHOS_DM_TEST_HAS_REAL_SERVICE_INFO
#elif __has_include("distributed_device_profile_client.h")
#include "distributed_device_profile_client.h"
#define OHOS_DM_TEST_HAS_REAL_SERVICE_INFO
#elif __has_include("common/include/interfaces/service_info.h")
#include "common/include/interfaces/service_info.h"
#define OHOS_DM_TEST_HAS_REAL_SERVICE_INFO
#elif __has_include("deviceprofile/device_info_manager/common/include/interfaces/service_info.h")
#include "deviceprofile/device_info_manager/common/include/interfaces/service_info.h"
#define OHOS_DM_TEST_HAS_REAL_SERVICE_INFO
#elif __has_include("interfaces/service_info.h")
#include "interfaces/service_info.h"
#define OHOS_DM_TEST_HAS_REAL_SERVICE_INFO
#endif
#endif

#ifndef OHOS_DM_TEST_HAS_REAL_SERVICE_INFO
// Fallback shim when real DistributedDeviceProfile service_info.h is unavailable.
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedDeviceProfile {
using ServiceInfo = OHOS::DistributedHardware::ServiceInfo;
} // namespace DistributedDeviceProfile
} // namespace OHOS
#endif

#endif // OHOS_DM_TEST_SERVICE_INFO_H
