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

#ifndef OHOS_DM_DEVICE_PROFILE_INFO_H
#define OHOS_DM_DEVICE_PROFILE_INFO_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace DistributedHardware {
typedef struct DmProductInfo {
    std::string prodId;
    std::string model;
    std::string prodName;
    std::string prodShortName;
    std::string imageVersion;
} DmProductInfo;

typedef struct DmServiceProfileInfo {
    std::string deviceId;
    std::string serviceId;
    std::string serviceType;
    std::map<std::string, std::string> data = {};
} DmServiceProfileInfo;

typedef struct DmDeviceProfileInfoFilterOptions {
    bool isCloud;
    std::vector<std::string> deviceIdList = {};
} DmDeviceProfileInfoFilterOptions;

typedef struct DmDeviceProfileInfo {
    std::string deviceId;
    std::string deviceSn;
    std::string mac;
    std::string model;
    std::string innerModel;
    std::string deviceType;
    std::string manufacturer;
    std::string deviceName;
    std::string productId;
    std::string subProductId;
    std::string sdkVersion;
    std::string bleMac;
    std::string brMac;
    std::string sleMac;
    std::string firmwareVersion;
    std::string hardwareVersion;
    std::string softwareVersion;
    int32_t protocolType;
    int32_t setupType;
    std::string wiseDeviceId;
    std::string wiseUserId;
    std::string registerTime;
    std::string modifyTime;
    std::string shareTime;
    bool isLocalDevice;
    std::vector<DmServiceProfileInfo> services = {};
} DmDeviceProfileInfo;

} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICE_PROFILE_INFO_H