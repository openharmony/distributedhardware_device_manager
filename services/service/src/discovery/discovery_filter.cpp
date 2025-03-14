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
#include "json_object.h"

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
    JsonObject jsonObject(str);
    if (jsonObject.IsDiscarded()) {
        LOGE("FilterOptions parse error.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!jsonObject.Contains(FILTERS_KEY) || !jsonObject[FILTERS_KEY].IsArray()) {
        LOGE("Filters invalid.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::vector<JsonItemObject> children = jsonObject[FILTERS_KEY].Items();
    if (children.empty()) {
        LOGE("Filters invalid.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (jsonObject.Contains(FILTER_OP_KEY) && !jsonObject[FILTER_OP_KEY].IsString()) {
        LOGE("Filters_op invalid.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!jsonObject.Contains(FILTER_OP_KEY)) {
        filterOp_ = FILTERS_TYPE_OR; // filterOp optional, "OR" default
    } else {
        jsonObject[FILTER_OP_KEY].GetTo(filterOp_);
    }

    for (const auto &object : children) {
        if (!object.Contains("type") || !object["type"].IsString()) {
            LOGE("Filters type invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        if (!object.Contains("value") || !object["value"].IsNumberInteger()) {
            LOGE("Filters value invalid");
            return ERR_DM_INPUT_PARA_INVALID;
        }
        DeviceFilters deviceFilters;
        deviceFilters.type = object["type"].Get<std::string>();
        deviceFilters.value = object["value"].Get<int32_t>();
        filters_.push_back(deviceFilters);
    }
    return DM_OK;
}

int32_t DeviceFilterOption::ParseFilterOptionJson(const std::string &str)
{
    JsonObject object(str);
    if (object.IsDiscarded()) {
        LOGE("ParseFilterOptionJson parse error.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    filterOp_ = FILTERS_TYPE_AND;
    DeviceFilters deviceFilters;
    if (object.Contains("credible") && object["credible"].IsNumberInteger()) {
        deviceFilters.type = "credible";
        deviceFilters.value = object["credible"].Get<int32_t>();
        filters_.push_back(deviceFilters);
    }
    if (object.Contains("range") && object["range"].IsNumberInteger()) {
        deviceFilters.type = "range";
        deviceFilters.value = object["range"].Get<int32_t>();
        filters_.push_back(deviceFilters);
    }
    if (object.Contains("isTrusted") && object["isTrusted"].IsNumberInteger()) {
        deviceFilters.type = "isTrusted";
        deviceFilters.value = object["isTrusted"].Get<int32_t>();
        filters_.push_back(deviceFilters);
    }
    if (object.Contains("authForm") && object["authForm"].IsNumberInteger()) {
        deviceFilters.type = "authForm";
        deviceFilters.value = object["authForm"].Get<int32_t>();
        filters_.push_back(deviceFilters);
    }
    if (object.Contains("deviceType") && object["deviceType"].IsNumberInteger()) {
        deviceFilters.type = "deviceType";
        deviceFilters.value = object["deviceType"].Get<int32_t>();
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