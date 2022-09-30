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

#include "dm_discovery_filter.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
const std::string FILTERS_KEY = "filters";
const std::string FILTER_OP_KEY = "filter_op";
const std::string FILTERS_TYPE_OR = "OR";
const std::string FILTERS_TYPE_AND = "AND";

enum DmDiscoveryDeviceFilter {
    DM_OFFLINE_DEVICE = 0,
    DM_ONLINE_DEVICE = 1,
    DM_ALL_DEVICE = 2
};

int32_t DmDeviceFilterOption::ParseFilterJson(const std::string &str)
{
    nlohmann::json jsonObject = nlohmann::json::parse(str, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("FilterOptions parse error.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!jsonObject.contains(FILTERS_KEY) || !jsonObject[FILTERS_KEY].is_array() || jsonObject[FILTERS_KEY].empty()) {
        LOGE("Filters invalid.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (jsonObject.contains(FILTER_OP_KEY) && !jsonObject[FILTER_OP_KEY].is_string()) {
        LOGE("Filters_op invalid.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!jsonObject.contains(FILTER_OP_KEY)) {
        filterOp_ = FILTERS_TYPE_OR; // filterOp optional, "OR" default
    } else {
        jsonObject[FILTER_OP_KEY].get_to(filterOp_);
    }

    for (const auto &object : jsonObject[FILTERS_KEY]) {
        if (!object.contains("type") || !object["type"].is_string()) {
            LOGE("Filters type invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        if (!object.contains("value") || !object["value"].is_number_integer()) {
            LOGE("Filters value invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        DmDeviceFilters deviceFilters;
        deviceFilters.type = object["type"];
        deviceFilters.value = object["value"];
        filters_.push_back(deviceFilters);
    }
    return DM_OK;
}

int32_t DmDeviceFilterOption::TransformToFilter(const std::string &filterOptions)
{
    if (filterOptions.empty()) {
        LOGI("DmDeviceFilterOption::filterOptions empty");
        filterOp_ = FILTERS_TYPE_OR;
        DmDeviceFilters deviceFilters;
        deviceFilters.type = "credible";
        deviceFilters.value = DM_OFFLINE_DEVICE;
        filters_.push_back(deviceFilters);
        return DM_OK;
    }
    return ParseFilterJson(filterOptions);
}

bool DmDiscoveryFilter::FilterByCredible(int32_t value, bool isOnline)
{
    if (value == DM_OFFLINE_DEVICE) {
        return !isOnline;
    }
    if (value == DM_ONLINE_DEVICE) {
        return isOnline;
    }
    return (value == DM_ALL_DEVICE);
}

bool DmDiscoveryFilter::FilterByRange(int32_t value, int range)
{
    return ((range > 0) && (range <= value));
}

bool DmDiscoveryFilter::FilterByType(const DmDeviceFilters &filters, const DmDeviceFilterPara &filterPara)
{
    LOGI("DmDiscoveryFilter::FilterByType: type: %s, value: %d", filters.type.c_str(), filters.value);
    if (filters.type == "credible") {
        return FilterByCredible(filters.value, filterPara.isOnline);
    }
    if (filters.type == "range") {
        return FilterByRange(filters.value, filterPara.range);
    }
    return false;
}

bool DmDiscoveryFilter::FilterOr(const std::vector<DmDeviceFilters> &filters, const DmDeviceFilterPara &filterPara)
{
    for (auto &iter : filters) {
        if (FilterByType(iter, filterPara) == true) {
            return true;
        }
    }
    return false;
}

bool DmDiscoveryFilter::FilterAnd(const std::vector<DmDeviceFilters> &filters, const DmDeviceFilterPara &filterPara)
{
    for (auto &iter : filters) {
        if (FilterByType(iter, filterPara) == false) {
            return false;
        }
    }
    return true;
}

bool DmDiscoveryFilter::IsValidDevice(const std::string &filterOp, const std::vector<DmDeviceFilters> &filters,
    const DmDeviceFilterPara &filterPara)
{
    LOGI("DmDiscoveryFilter::IsValidDevice: filterOp: %s, isOnline: %d, range: %d", filterOp.c_str(),
        filterPara.isOnline, filterPara.range);
    if (filterOp == FILTERS_TYPE_OR) {
        return FilterOr(filters, filterPara);
    }
    if (filterOp == FILTERS_TYPE_AND) {
        return FilterAnd(filters, filterPara);
    }
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS