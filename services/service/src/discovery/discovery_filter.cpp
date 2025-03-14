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

#include "discovery_filter.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
const std::string FILTERS_KEY = "filters";
const std::string FILTER_OP_KEY = "filter_op";
const std::string FILTERS_TYPE_OR = "OR";
const std::string FILTERS_TYPE_AND = "AND";
const int32_t DM_OK = 0;
const int32_t ERR_DM_INPUT_PARA_INVALID = 96929749;
enum class DmDiscoveryDeviceFilter : int32_t {
    DM_INVALID_DEVICE = 0,
    DM_VALID_DEVICE = 1,
    DM_ALL_DEVICE = 2
};

int32_t DeviceFilterOption::ParseFilterJson(const std::string &str)
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
        DeviceFilters deviceFilters;
        deviceFilters.type = object["type"];
        deviceFilters.value = object["value"];
        filters_.push_back(deviceFilters);
    }
    return DM_OK;
}

int32_t DeviceFilterOption::ParseFilterOptionJson(const std::string &str)
{
    nlohmann::json object = nlohmann::json::parse(str, nullptr, false);
    if (object.is_discarded()) {
        LOGE("ParseFilterOptionJson parse error.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    filterOp_ = FILTERS_TYPE_AND;
    DeviceFilters deviceFilters;
    if (object.contains("credible") && object["credible"].is_number_integer()) {
        deviceFilters.type = "credible";
        deviceFilters.value = object["credible"];
        filters_.push_back(deviceFilters);
    }
    if (object.contains("range") && object["range"].is_number_integer()) {
        deviceFilters.type = "range";
        deviceFilters.value = object["range"];
        filters_.push_back(deviceFilters);
    }
    if (object.contains("isTrusted") && object["isTrusted"].is_number_integer()) {
        deviceFilters.type = "isTrusted";
        deviceFilters.value = object["isTrusted"];
        filters_.push_back(deviceFilters);
    }
    if (object.contains("authForm") && object["authForm"].is_number_integer()) {
        deviceFilters.type = "authForm";
        deviceFilters.value = object["authForm"];
        filters_.push_back(deviceFilters);
    }
    if (object.contains("deviceType") && object["deviceType"].is_number_integer()) {
        deviceFilters.type = "deviceType";
        deviceFilters.value = object["deviceType"];
        filters_.push_back(deviceFilters);
    }
    return DM_OK;
}

int32_t DeviceFilterOption::TransformToFilter(const std::string &filterOptions)
{
    if (filterOptions.empty()) {
        LOGI("DeviceFilterOption::filterOptions empty");
        filterOp_ = FILTERS_TYPE_OR;
        DeviceFilters deviceFilters;
        deviceFilters.type = "credible";
        deviceFilters.value = static_cast<int32_t>(DmDiscoveryDeviceFilter::DM_INVALID_DEVICE);
        filters_.push_back(deviceFilters);
        return DM_OK;
    }
    return ParseFilterJson(filterOptions);
}

int32_t DeviceFilterOption::TransformFilterOption(const std::string &filterOptions)
{
    if (filterOptions.empty()) {
        LOGI("DeviceFilterOption::filterOptions empty");
        filterOp_ = FILTERS_TYPE_OR;
        DeviceFilters deviceFilters;
        deviceFilters.type = "credible";
        deviceFilters.value = static_cast<int32_t>(DmDiscoveryDeviceFilter::DM_INVALID_DEVICE);
        filters_.push_back(deviceFilters);
        return DM_OK;
    }
    return ParseFilterOptionJson(filterOptions);
}

bool DiscoveryFilter::FilterByDeviceState(int32_t value, bool isActive)
{
    if (value == static_cast<int32_t>(DmDiscoveryDeviceFilter::DM_INVALID_DEVICE)) {
        return !isActive;
    }
    if (value == static_cast<int32_t>(DmDiscoveryDeviceFilter::DM_VALID_DEVICE)) {
        return isActive;
    }
    return (value == static_cast<int32_t>(DmDiscoveryDeviceFilter::DM_ALL_DEVICE));
}

bool DiscoveryFilter::FilterByRange(int32_t value, int32_t range)
{
    return ((range >= 0) && (range <= value));
}

bool DiscoveryFilter::FilterByDeviceType(int32_t value, int32_t deviceType)
{
    return (value == deviceType);
}

bool DiscoveryFilter::FilterByType(const DeviceFilters &filters, const DeviceFilterPara &filterPara)
{
    if (filters.type == "credible") {
        return FilterByDeviceState(filters.value, filterPara.isOnline);
    }
    if (filters.type == "range") {
        return FilterByRange(filters.value, filterPara.range);
    }
    if (filters.type == "isTrusted") {
        return FilterByDeviceState(filters.value, filterPara.isTrusted);
    }
    if (filters.type == "deviceType") {
        return FilterByDeviceType(filters.value, filterPara.deviceType);
    }
    return false;
}

bool DiscoveryFilter::FilterOr(const std::vector<DeviceFilters> &filters, const DeviceFilterPara &filterPara)
{
    for (auto &iter : filters) {
        if (FilterByType(iter, filterPara) == true) {
            return true;
        }
    }
    return false;
}

bool DiscoveryFilter::FilterAnd(const std::vector<DeviceFilters> &filters, const DeviceFilterPara &filterPara)
{
    for (auto &iter : filters) {
        if (FilterByType(iter, filterPara) == false) {
            return false;
        }
    }
    return true;
}

bool DiscoveryFilter::IsValidDevice(const std::string &filterOp, const std::vector<DeviceFilters> &filters,
    const DeviceFilterPara &filterPara)
{
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