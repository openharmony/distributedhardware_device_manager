/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "UTTest_dm_discovery_filter.h"

namespace OHOS {
namespace DistributedHardware {
void DmDiscoveryFilterTest::SetUp()
{
}
void DmDiscoveryFilterTest::TearDown()
{
}
void DmDiscoveryFilterTest::SetUpTestCase()
{
}
void DmDiscoveryFilterTest::TearDownTestCase()
{
}

const std::string FILTERS_KEY = "filters";
const std::string FILTER_OP_KEY = "filter_op";
namespace {

/**
 * @tc.name: ParseFilterJson_001
 * @tc.desc: Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_001, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    std::string str;
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseFilterJson_002
 * @tc.desc: Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_002, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    std::string str = jsonObject.dump();
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseFilterJson_003
 * @tc.desc: Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_003, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    jsonObject[FILTERS_KEY] = "filters";
    std::string str = jsonObject.dump();
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseFilterJson_004
 * @tc.desc: Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_004, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    std::vector<int> myArray;
    jsonObject[FILTERS_KEY] = myArray;
    std::string str = jsonObject.dump();
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseFilterJson_005
 * @tc.desc: Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_005, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    int myArray[5] = {1, 2, 3, 4, 5};
    jsonObject[FILTERS_KEY] = myArray;
    std::string str = jsonObject.dump();
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseFilterJson_006
 * @tc.desc: Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_006, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    int myArray[5] = {1, 2, 3, 4, 5};
    jsonObject[FILTERS_KEY] = myArray;
    jsonObject[FILTER_OP_KEY] = 12345;
    std::string str = jsonObject.dump();
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseFilterJson_007
 * @tc.desc: Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_007, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    int myArray[5] = {1, 2, 3, 4, 5};
    jsonObject[FILTERS_KEY] = myArray;
    jsonObject[FILTER_OP_KEY] = "filter_op";
    std::string str = jsonObject.dump();
    int32_t ret = filterOption.ParseFilterJson(str);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ParseFilterJson_008
 * @tc.desc: Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_008, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
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

/**
 * @tc.name: ParseFilterJson_009
 * @tc.desc: Return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_009, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
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

/**
 * @tc.name: ParseFilterJson_0010
 * @tc.desc: Return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterJson_0010, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
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

/**
 * @tc.name: ParseFilterOptionJson_001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterOptionJson_001, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    std::string str;
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ != "AND", true);
}

/**
 * @tc.name: ParseFilterOptionJson_002
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterOptionJson_002, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    std::string str = jsonObject.dump();
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

/**
 * @tc.name: ParseFilterOptionJson_003
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterOptionJson_003, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    jsonObject["credible"] = "123";
    std::string str = jsonObject.dump();
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

/**
 * @tc.name: ParseFilterOptionJson_004
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterOptionJson_004, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    jsonObject["credible"] = 1;
    jsonObject["isTrusted"] = "123";
    std::string str = jsonObject.dump();
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

/**
 * @tc.name: ParseFilterOptionJson_005
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterOptionJson_005, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    jsonObject["credible"] = 1;
    jsonObject["isTrusted"] = 2;
    jsonObject["authForm"] = "3";
    std::string str = jsonObject.dump();
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

/**
 * @tc.name: ParseFilterOptionJson_006
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterOptionJson_006, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    jsonObject["credible"] = 1;
    jsonObject["isTrusted"] = 2;
    jsonObject["authForm"] = 3;
    jsonObject["deviceType"] = "4";
    std::string str = jsonObject.dump();
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

/**
 * @tc.name: ParseFilterOptionJson_007
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, ParseFilterOptionJson_007, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    nlohmann::json jsonObject;
    jsonObject["credible"] = 1;
    jsonObject["isTrusted"] = 2;
    jsonObject["authForm"] = 3;
    jsonObject["deviceType"] = 4;
    std::string str = jsonObject.dump();
    filterOption.ParseFilterOptionJson(str);
    EXPECT_EQ(filterOption.filterOp_ == "AND", true);
}

/**
 * @tc.name: TransformToFilter_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, TransformToFilter_001, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    std::string filterOptions;
    int32_t ret = filterOption.TransformToFilter(filterOptions);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: TransformToFilter_002
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, TransformToFilter_002, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    std::string filterOptions = "filterOptions";
    int32_t ret = filterOption.TransformToFilter(filterOptions);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: TransformFilterOption_001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, TransformFilterOption_001, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    std::string filterOptions;
    filterOption.TransformFilterOption(filterOptions);
    EXPECT_EQ(filterOptions.empty(), true);
}

/**
 * @tc.name: TransformFilterOption_002
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, TransformFilterOption_002, testing::ext::TestSize.Level0)
{
    DmDeviceFilterOption filterOption;
    std::string filterOptions = "filterOptions";
    filterOption.TransformFilterOption(filterOptions);
    EXPECT_EQ(filterOptions.empty(), false);
}

/**
 * @tc.name: FilterByDeviceState_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByDeviceState_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 0;
    bool isOnline = false;
    bool ret = filter.FilterByDeviceState(value, isOnline);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByDeviceState_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByDeviceState_002, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 1;
    bool isOnline = false;
    bool ret = filter.FilterByDeviceState(value, isOnline);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterByDeviceState_003
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByDeviceState_003, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 2;
    bool isOnline = false;
    bool ret = filter.FilterByDeviceState(value, isOnline);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByRange_001
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByRange_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 0;
    int32_t range = 1;
    bool ret = filter.FilterByRange(value, range);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterByRange_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByRange_002, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 0;
    int32_t range = -1;
    bool ret = filter.FilterByRange(value, range);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterByRange_003
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByRange_003, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 1;
    int32_t range = 1;
    bool ret = filter.FilterByRange(value, range);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByAuthForm_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByAuthForm_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 1;
    int32_t authForm = 1;
    bool ret = filter.FilterByAuthForm(value, authForm);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByAuthForm_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByAuthForm_002, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 1;
    int32_t authForm = 2;
    bool ret = filter.FilterByAuthForm(value, authForm);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterByDeviceType_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByDeviceType_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 1;
    int32_t deviceType = 1;
    bool ret = filter.FilterByDeviceType(value, deviceType);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByDeviceType_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByDeviceType, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 1;
    int32_t deviceType = 2;
    bool ret = filter.FilterByDeviceType(value, deviceType);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterByType_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByType_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "credible";
    filters.value = 0;
    DmDeviceFilterPara filterPara;
    filterPara.isOnline = false;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByType_002
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByType_002, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "range";
    filters.value = 1;
    DmDeviceFilterPara filterPara;
    filterPara.range = 1;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByType_003
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByType_003, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "isTrusted";
    filters.value = 1;
    DmDeviceFilterPara filterPara;
    filterPara.isTrusted = false;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterByType_004
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByType_004, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "authForm";
    filters.value = 1;
    DmDeviceFilterPara filterPara;
    filterPara.authForm = 1;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByType_005
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByType_005, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "deviceType";
    filters.value = 1;
    DmDeviceFilterPara filterPara;
    filterPara.deviceType = 1;
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByType_006
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByType_006, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    DmDeviceFilterPara filterPara;
    filters.type = "typeTest";
    bool ret = filter.FilterByType(filters, filterPara);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterOr_001
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterOr_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "typeTest";
    std::vector<DmDeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DmDeviceFilterPara filterPara;
    bool ret = filter.FilterOr(filtersVec, filterPara);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterOr_002
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterOr_002, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "range";
    filters.value = 1;
    std::vector<DmDeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DmDeviceFilterPara filterPara;
    filterPara.range = 1;
    bool ret = filter.FilterOr(filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterOr_003
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterOr_003, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "credible";
    filters.value = 0;
    std::vector<DmDeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DmDeviceFilterPara filterPara;
    filterPara.isOnline = false;
    bool ret = filter.FilterOr(filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterAnd_001
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterAnd_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "typeTest";
    std::vector<DmDeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DmDeviceFilterPara filterPara;
    bool ret = filter.FilterAnd(filtersVec, filterPara);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterAnd_002
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterAnd_002, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "range";
    filters.value = 1;
    std::vector<DmDeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DmDeviceFilterPara filterPara;
    filterPara.range = 1;
    bool ret = filter.FilterAnd(filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterAnd_003
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterAnd_003, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    DmDeviceFilters filters;
    filters.type = "credible";
    filters.value = 0;
    std::vector<DmDeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DmDeviceFilterPara filterPara;
    filterPara.isOnline = false;
    bool ret = filter.FilterAnd(filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsValidDevice_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, IsValidDevice_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    const std::string filterOp = "OR";
    DmDeviceFilters filters;
    filters.type = "credible";
    filters.value = 0;
    std::vector<DmDeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DmDeviceFilterPara filterPara;
    filterPara.isOnline = false;
    bool ret = filter.IsValidDevice(filterOp, filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsValidDevice_002
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, IsValidDevice_002, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    const std::string filterOp = "AND";
    DmDeviceFilters filters;
    filters.type = "range";
    filters.value = 1;
    std::vector<DmDeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DmDeviceFilterPara filterPara;
    filterPara.range = 1;
    bool ret = filter.IsValidDevice(filterOp, filtersVec, filterPara);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsValidDevice_003
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, IsValidDevice_003, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    const std::string filterOp = "filterOpTest";
    DmDeviceFilters filters;
    std::vector<DmDeviceFilters> filtersVec;
    filtersVec.push_back(filters);
    DmDeviceFilterPara filterPara;
    bool ret = filter.IsValidDevice(filterOp, filtersVec, filterPara);
    EXPECT_EQ(ret, false);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS