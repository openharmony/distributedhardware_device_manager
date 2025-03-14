/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISCOVERY_FILTER_H
#define OHOS_DISCOVERY_FILTER_H

#include <string>

#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
struct DeviceFilters {
    std::string type;
    int32_t value;
};

struct DeviceFilterOption {
    std::string filterOp_;
    std::vector<DeviceFilters> filters_;
    int32_t ParseFilterJson(const std::string &str);
    int32_t ParseFilterOptionJson(const std::string &str);
    int32_t TransformToFilter(const std::string &filterOptions);
    int32_t TransformFilterOption(const std::string &filterOptions);
};

struct DeviceFilterPara {
    bool isOnline;
    int32_t range;
    bool isTrusted;
    int32_t authForm;
    int32_t deviceType;
};

class DiscoveryFilter {
public:
    bool IsValidDevice(const std::string &filterOp, const std::vector<DeviceFilters> &filters,
        const DeviceFilterPara &filterPara);
private:
    bool FilterByDeviceState(int32_t value, bool isActive);
    bool FilterByRange(int32_t value, int32_t range);
    bool FilterByDeviceType(int32_t value, int32_t deviceType);
    bool FilterByType(const DeviceFilters &filters, const DeviceFilterPara &filterPara);
    bool FilterOr(const std::vector<DeviceFilters> &filters, const DeviceFilterPara &filterPara);
    bool FilterAnd(const std::vector<DeviceFilters> &filters, const DeviceFilterPara &filterPara);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISCOVERY_FILTER_H
