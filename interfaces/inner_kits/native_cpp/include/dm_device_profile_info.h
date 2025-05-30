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
    std::string productId = "";
    std::string externalName = "";
    std::string shortName = "";
    std::string imageVersion = "";
} DmProductInfo;

typedef struct DmDeviceIconInfoFilterOptions {
    std::string productId = "";
    std::string subProductId = "";
    std::string internalModel = "";
    std::string imageType = "";
    std::string specName = "";
} DmDeviceIconInfoFilterOptions;

typedef struct DmDeviceIconInfo {
    std::string productId = "";
    std::string subProductId = "";
    std::string internalModel = "";
    std::string imageType = "";
    std::string specName = "";
    std::string wiseVersion = "";
    std::string version = "";
    std::string url = "";
    std::vector<uint8_t> icon = {};

    void InitByDmDeviceIconInfoFilterOptions(const DmDeviceIconInfoFilterOptions &filterOptions)
    {
        productId = filterOptions.productId;
        subProductId = filterOptions.subProductId;
        internalModel = filterOptions.internalModel;
        imageType = filterOptions.imageType;
        specName = filterOptions.specName;
    }
} DmDeviceIconInfo;

typedef struct DmServiceProfileInfo {
    std::string deviceId = "";
    std::string serviceId = "";
    std::string serviceType = "";
    std::map<std::string, std::string> data = {};
} DmServiceProfileInfo;

typedef struct DmDeviceProfileInfoFilterOptions {
    bool isCloud = true;
    std::vector<std::string> deviceIdList = {};
} DmDeviceProfileInfoFilterOptions;

typedef struct DmDeviceProfileInfo {
    std::string deviceId = "";
    std::string deviceSn = "";
    std::string mac = "";
    std::string model = "";
    std::string internalModel = "";
    std::string deviceType = "";
    std::string manufacturer = "";
    std::string deviceName = "";
    std::string productName = "";
    std::string productId = "";
    std::string subProductId = "";
    std::string sdkVersion = "";
    std::string bleMac = "";
    std::string brMac = "";
    std::string sleMac = "";
    std::string firmwareVersion = "";
    std::string hardwareVersion = "";
    std::string softwareVersion = "";
    int32_t protocolType = 0;
    int32_t setupType = 0;
    std::string wiseDeviceId = "";
    std::string wiseUserId = "";
    std::string registerTime = "";
    std::string modifyTime = "";
    std::string shareTime = "";
    bool isLocalDevice = false;
    std::vector<DmServiceProfileInfo> services = {};
} DmDeviceProfileInfo;

typedef struct NetworkIdQueryFilter {
    std::string wiseDeviceId = "";
    int32_t onlineStatus = 0;
    std::string deviceType = "";
    std::string deviceProductId = "";
    std::string deviceModel = "";
} NetworkIdQueryFilter;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICE_PROFILE_INFO_H
