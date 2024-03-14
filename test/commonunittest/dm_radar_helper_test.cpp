/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dm_radar_helper_test.h"

#include "dm_radar_helper.h"

namespace OHOS {
namespace DistributedHardware {
void DmRadarHelperTest::SetUp()
{
}

void DmRadarHelperTest::TearDown()
{
}

void DmRadarHelperTest::SetUpTestCase()
{
}

void DmRadarHelperTest::TearDownTestCase()
{
}

HWTEST_F(DmRadarHelperTest, ReportDiscoverRegCallback_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportDiscoverRegCallback(info);
    EXPECT_EQ(res, true);
}

HWTEST_F(DmRadarHelperTest, ReportDiscoverRegCallback_002, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_CANCEL);
    bool res = DmRadarHelper::GetInstance().ReportDiscoverRegCallback(info);
    EXPECT_EQ(res, true);
}

HWTEST_F(DmRadarHelperTest, ReportDiscoverResCallback_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportDiscoverResCallback(info);
    EXPECT_EQ(res, true);
}

HWTEST_F(DmRadarHelperTest, ReportDiscoverUserRes_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_CANCEL);
    bool res = DmRadarHelper::GetInstance().ReportDiscoverUserRes(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportDiscoverUserRes_002, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportDiscoverUserRes(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthStart_001, testing::ext::TestSize.Level0)
{
    bool res = DmRadarHelper::GetInstance().ReportAuthStart("udid", "pkgname");
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthOpenSession_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthOpenSession(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthOpenSession_002, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    bool res = DmRadarHelper::GetInstance().ReportAuthOpenSession(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthSessionOpenCb_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthSessionOpenCb(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthSendRequest_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthSendRequest(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthPullAuthBox_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthPullAuthBox(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthConfirmBox_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    bool res = DmRadarHelper::GetInstance().ReportAuthConfirmBox(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthConfirmBox_002, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthConfirmBox(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthCreateGroup_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthCreateGroup(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthCreateGroup_002, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    bool res = DmRadarHelper::GetInstance().ReportAuthCreateGroup(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthCreateGroupCb_001, testing::ext::TestSize.Level0)
{
    int32_t stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    bool res = DmRadarHelper::GetInstance().ReportAuthCreateGroupCb("funcName", stageRes);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthCreateGroupCb_002, testing::ext::TestSize.Level0)
{
    int32_t stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthCreateGroupCb("funcName", stageRes);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthPullPinBox_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthPullPinBox(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthInputPinBox_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    bool res = DmRadarHelper::GetInstance().ReportAuthInputPinBox(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthInputPinBox_002, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthInputPinBox(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthAddGroup_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthAddGroup(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthAddGroup_002, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    bool res = DmRadarHelper::GetInstance().ReportAuthAddGroup(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthAddGroupCb_001, testing::ext::TestSize.Level0)
{
    int32_t stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    bool res = DmRadarHelper::GetInstance().ReportAuthAddGroupCb("func", stageRes);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportAuthAddGroupCb_002, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    int32_t stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportAuthAddGroupCb("func", stageRes);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportNetworkOnline_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportNetworkOnline(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportNetworkOffline_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportNetworkOffline(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportDeleteTrustRelation_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportDeleteTrustRelation(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportGetTrustDeviceList_001, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_SUCC);
    bool res = DmRadarHelper::GetInstance().ReportGetTrustDeviceList(info);
    EXPECT_EQ(res, true);
}
HWTEST_F(DmRadarHelperTest, ReportGetTrustDeviceList_002, testing::ext::TestSize.Level0)
{
    struct RadarInfo info = { 0 };
    info.stageRes = static_cast<int32_t>(StageRes::STAGE_IDLE);
    bool res = DmRadarHelper::GetInstance().ReportGetTrustDeviceList(info);
    EXPECT_EQ(res, true);
}
} // namespace DistributedHardware
} // namespace OHOS
