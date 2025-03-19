/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "UTTest_discovery_filter.h"

#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
void DiscoveryFilterTest::SetUp()
{
}
void DiscoveryFilterTest::TearDown()
{
}
void DiscoveryFilterTest::SetUpTestCase()
{
}
void DiscoveryFilterTest::TearDownTestCase()
{
}

const std::string FILTERS_KEY = "filters";
const std::string FILTER_OP_KEY = "filter_op";
namespace {
HWTEST_F(DiscoveryFilterTest, ParseFilterJson_001, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    std::string str;
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterJson_002, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    std::string str = SafetyDump(jsonObject);
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterJson_003, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    jsonObject[FILTERS_KEY] = "filters";
    std::string str = SafetyDump(jsonObject);
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterJson_004, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    std::vector<int> myArray;
    jsonObject[FILTERS_KEY] = myArray;
    std::string str = SafetyDump(jsonObject);
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterJson_005, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    std::vector<int> myArray = {1, 2, 3, 4, 5};
    jsonObject[FILTERS_KEY] = myArray;
    std::string str = SafetyDump(jsonObject);
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterJson_006, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    std::vector<int> myArray = {1, 2, 3, 4, 5};
    jsonObject[FILTERS_KEY] = myArray;
    jsonObject[FILTER_OP_KEY] = 12345;
    std::string str = SafetyDump(jsonObject);
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterJson_007, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    std::vector<int> myArray = {1, 2, 3, 4, 5};
    jsonObject[FILTERS_KEY] = myArray;
    jsonObject[FILTER_OP_KEY] = "filter_op";
    std::string str = SafetyDump(jsonObject);
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterJson_008, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    nlohmann::json object;
    object["type"] = 1;
    std::vector<nlohmann::json>myArray;
    myArray.push_back(object);
    jsonObject[FILTERS_KEY] = myArray;
    jsonObject[FILTER_OP_KEY] = "filter_op";
    std::string str = jsonObject.dump();
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterJson_009, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    nlohmann::json object;
    object["type"] = "1";
    object["value"] = "1";
    std::vector<nlohmann::json>myArray;
    myArray.push_back(object);
    jsonObject[FILTERS_KEY] = myArray;
    jsonObject[FILTER_OP_KEY] = "filter_op";
    std::string str = jsonObject.dump();
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterJson_0010, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    nlohmann::json object;
    object["type"] = "1";
    object["value"] = 1;
    std::vector<nlohmann::json>myArray;
    myArray.push_back(object);
    jsonObject[FILTERS_KEY] = myArray;
    jsonObject[FILTER_OP_KEY] = "filter_op";
    std::string str = jsonObject.dump();
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterOptionJson_001, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    std::string str;
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ != "AND", true);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterOptionJson_002, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    std::string str = SafetyDump(jsonObject);
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterOptionJson_003, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    jsonObject["credible"] = "123";
    std::string str = SafetyDump(jsonObject);
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterOptionJson_004, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    jsonObject["credible"] = 1;
    jsonObject["isTrusted"] = "123";
    std::string str = SafetyDump(jsonObject);
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterOptionJson_005, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    jsonObject["credible"] = 1;
    jsonObject["isTrusted"] = 2;
    jsonObject["authForm"] = "3";
    std::string str = SafetyDump(jsonObject);
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterOptionJson_006, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    jsonObject["credible"] = 1;
    jsonObject["isTrusted"] = 2;
    jsonObject["authForm"] = 3;
    jsonObject["deviceType"] = "4";
    std::string str = SafetyDump(jsonObject);
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

HWTEST_F(DiscoveryFilterTest, ParseFilterOptionJson_007, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    JsonObject jsonObject;
    jsonObject["credible"] = 1;
    jsonObject["isTrusted"] = 2;
    jsonObject["authForm"] = 3;
    jsonObject["deviceType"] = 4;
    std::string str = SafetyDump(jsonObject);
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

HWTEST_F(DiscoveryFilterTest, TransformToFilter_001, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    std::string filterOptions;
    int32_t ret = filterOption.TransformToFilter(filterOptions);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DiscoveryFilterTest, TransformToFilter_002, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    std::string filterOptions = "filterOptions";
    int32_t ret = filterOption.TransformToFilter(filterOptions);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DiscoveryFilterTest, TransformFilterOption_001, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    std::string filterOptions;
    filterOption.TransformFilterOption(filterOptions);
    EXPECT_EQ(filterOptions.empty(), true);
}

HWTEST_F(DiscoveryFilterTest, TransformFilterOption_002, testing::ext::TestSize.Level0)
{
    DeviceFilterOption filterOption;
    std::string filterOptions = "filterOptions";
    filterOption.TransformFilterOption(filterOptions);
    EXPECT_EQ(filterOptions.empty(), false);
}

HWTEST_F(DiscoveryFilterTest, FilterByDeviceState_001, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = 0;
    bool isOnline = false;
    bool ret = filter.FilterByDeviceState(value, isOnline);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterByDeviceState_002, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = 1;
    bool isOnline = false;
    bool ret = filter.FilterByDeviceState(value, isOnline);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DiscoveryFilterTest, FilterByDeviceState_003, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = 2;
    bool isOnline = false;
    bool ret = filter.FilterByDeviceState(value, isOnline);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterByRange_001, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = 0;
    bool isActive = true;
    bool ret = filter.FilterByRange(value, isActive);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DiscoveryFilterTest, FilterByRange_002, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = 1;
    bool isActive = true;
    bool ret = filter.FilterByRange(value, isActive);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterByRange_003, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = 2;
    bool isActive = true;
    bool ret = filter.FilterByRange(value, isActive);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterByRange_004, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = -1;
    int32_t range = 1;
    bool ret = filter.FilterByRange(value, range);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DiscoveryFilterTest, FilterByRange_005, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = 0;
    int32_t range = -1;
    bool ret = filter.FilterByRange(value, range);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DiscoveryFilterTest, FilterByRange_006, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = 1;
    int32_t range = 1;
    bool ret = filter.FilterByRange(value, range);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterByRange_007, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    int32_t value = 2;
    int32_t deviceType = 2;
    bool ret = filter.FilterByRange(value, deviceType);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterOr_001, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    DeviceFilters filters;
    filters.type = "typeTest";
    std::vector<DeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DeviceFilterPara filterPara;
    bool ret = filter.FilterOr(filtersVec, filterPara);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DiscoveryFilterTest, FilterOr_002, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    DeviceFilters filters;
    filters.type = "range";
    filters.value = 1;
    std::vector<DeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DeviceFilterPara filterPara;
    filterPara.range = 1;
    bool ret = filter.FilterOr(filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterOr_003, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    DeviceFilters filters;
    filters.type = "credible";
    filters.value = 0;
    std::vector<DeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DeviceFilterPara filterPara;
    filterPara.isOnline = false;
    bool ret = filter.FilterOr(filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterAnd_001, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    DeviceFilters filters;
    filters.type = "typeTest";
    std::vector<DeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DeviceFilterPara filterPara;
    bool ret = filter.FilterAnd(filtersVec, filterPara);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DiscoveryFilterTest, FilterAnd_002, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    DeviceFilters filters;
    filters.type = "range";
    filters.value = 1;
    std::vector<DeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DeviceFilterPara filterPara;
    filterPara.range = 1;
    bool ret = filter.FilterAnd(filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterAnd_003, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    DeviceFilters filters;
    filters.type = "credible";
    filters.value = 0;
    std::vector<DeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DeviceFilterPara filterPara;
    filterPara.isOnline = false;
    bool ret = filter.FilterAnd(filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterByType_001, testing::ext::TestSize.Level0)
{
    DeviceFilters filters = {
        .type = "credible",
        .value = 0,
    };
    DeviceFilterPara filterPara = {
        .isOnline = true,
    };
    DiscoveryFilter filter;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DiscoveryFilterTest, FilterByType_002, testing::ext::TestSize.Level0)
{
    DeviceFilters filters = {
        .type = "range",
        .value = 1,
    };
    DeviceFilterPara filterPara = {
        .range = 1,
    };
    DiscoveryFilter filter;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterByType_003, testing::ext::TestSize.Level0)
{
    DeviceFilters filters = {
        .type = "isTrusted",
        .value = 1,
    };
    DeviceFilterPara filterPara = {
        .isTrusted = true,
    };
    DiscoveryFilter filter;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterByType_004, testing::ext::TestSize.Level0)
{
    DeviceFilters filters = {
        .type = "deviceType",
        .value = 1,
    };
    DeviceFilterPara filterPara = {
        .deviceType = 1,
    };
    DiscoveryFilter filter;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, FilterByType_005, testing::ext::TestSize.Level0)
{
    DeviceFilters filters = {
        .type = "filterByType",
        .value = 1,
    };
    DeviceFilterPara filterPara = {
        .deviceType = 1,
    };
    DiscoveryFilter filter;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DiscoveryFilterTest, IsValidDevice_001, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    const std::string filterOp = "OR";
    DeviceFilters filters;
    filters.type = "credible";
    filters.value = 0;
    std::vector<DeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DeviceFilterPara filterPara;
    filterPara.isOnline = false;
    bool ret = filter.IsValidDevice(filterOp, filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, IsValidDevice_002, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    const std::string filterOp = "AND";
    DeviceFilters filters;
    filters.type = "range";
    filters.value = 1;
    std::vector<DeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DeviceFilterPara filterPara;
    filterPara.range = 1;
    bool ret = filter.IsValidDevice(filterOp, filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryFilterTest, IsValidDevice_003, testing::ext::TestSize.Level0)
{
    DiscoveryFilter filter;
    const std::string filterOp = "filterOpTest";
    std::vector<DeviceFilters> filtersVec;
    DeviceFilterPara filterPara;
    bool ret = filter.IsValidDevice(filterOp, filtersVec, filterPara);
    EXPECT_EQ(ret, false);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS