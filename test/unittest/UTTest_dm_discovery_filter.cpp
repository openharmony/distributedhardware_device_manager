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

namespace {

/**
 * @tc.name: FilterByCredible_001
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByCredible_001, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 0;
    bool isOnline = false;
    bool ret = filter.FilterByCredible(value, isOnline);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FilterByCredible_002
 * @tc.desc: Return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByCredible_002, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 1;
    bool isOnline = false;
    bool ret = filter.FilterByCredible(value, isOnline);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: FilterByCredible_003
 * @tc.desc: Return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryFilterTest, FilterByCredible_003, testing::ext::TestSize.Level0)
{
    DmDiscoveryFilter filter;
    int32_t value = 2;
    bool isOnline = false;
    bool ret = filter.FilterByCredible(value, isOnline);
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
 * @tc.desc: Return false
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
 * @tc.desc: Return false
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
 * @tc.desc: Return false
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