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

#ifndef OHOS_DM_DISCOVERY_FILTER_H
#define OHOS_DM_DISCOVERY_FILTER_H

#include <vector>
#include <string>

#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
struct DmDeviceFilters {
    std::string type;
    int32_t value;
};

struct DmDeviceFilterOption {
    std::string filterOp_;
    std::vector<DmDeviceFilters> filters_;
    int32_t ParseFilterJson(const std::string &str);
    int32_t TransformToFilter(const std::string &filterOptions);
};

struct DmDeviceFilterPara {
    bool isOnline;
    int32_t range;
};

class DmDiscoveryFilter {
public:
    bool IsValidDevice(const std::string &filterOp, const std::vector<DmDeviceFilters> &filters,
        const DmDeviceFilterPara &filterPara);
private:
    bool FilterByCredible(int32_t value, bool isOnline);
    bool FilterByRange(int32_t value, int32_t range);
    bool FilterByType(const DmDeviceFilters &filters, const DmDeviceFilterPara &filterPara);
    bool FilterOr(const std::vector<DmDeviceFilters> &filters, const DmDeviceFilterPara &filterPara);
    bool FilterAnd(const std::vector<DmDeviceFilters> &filters, const DmDeviceFilterPara &filterPara);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DISCOVERY_FILTER_H
