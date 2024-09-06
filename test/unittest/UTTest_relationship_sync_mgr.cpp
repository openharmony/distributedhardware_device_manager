/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "UTTest_relationship_sync_mgr.h"

#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
void ReleationShipSyncMgrTest::SetUp()
{
}

void ReleationShipSyncMgrTest::TearDown()
{
}

void ReleationShipSyncMgrTest::SetUpTestCase()
{
}

void ReleationShipSyncMgrTest::TearDownTestCase()
{
}

namespace {

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_001, testing::ext::TestSize.Level0)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::ACCOUNT_LOGOUT;
    msg.userId = UINT32_MAX;
    msg.peerUdids = peerUdids;
    msg.accountId = "1";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_002, testing::ext::TestSize.Level0)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::ACCOUNT_LOGOUT;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_003, testing::ext::TestSize.Level0)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::ACCOUNT_LOGOUT;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_NE(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_004, testing::ext::TestSize.Level0)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::DEVICE_UNBIND;
    msg.userId = UINT32_MAX;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_005, testing::ext::TestSize.Level0)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::DEVICE_UNBIND;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_NE(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_006, testing::ext::TestSize.Level0)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = UINT32_MAX;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_007, testing::ext::TestSize.Level0)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    msg.tokenId = UINT64_MAX;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_008, testing::ext::TestSize.Level0)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::APP_UNBIND;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    msg.tokenId = 1;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_NE(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, SyncTrustRelationShip_009, testing::ext::TestSize.Level0)
{
    std::vector<std::string> peerUdids;
    RelationShipChangeMsg msg;
    msg.type = RelationShipChangeType::SERVICE_UNBIND;
    msg.userId = 1;
    msg.peerUdids = peerUdids;
    msg.accountId = "1111111";
    msg.accountName = "1";
    msg.tokenId = 1;
    std::string broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    msg.type = RelationShipChangeType::DEL_USER;
    broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    msg.type = RelationShipChangeType::APP_UNINSTALL;
    broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    msg.type = RelationShipChangeType::TYPE_MAX;
    broadCastMsg = ReleationShipSyncMgr::GetInstance().SyncTrustRelationShip(msg);
    ASSERT_EQ(broadCastMsg, "");
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_001, testing::ext::TestSize.Level0)
{
    std::string msg = "";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_002, testing::ext::TestSize.Level0)
{
    std::string msg = R"(
    {
        "xx": "xx"
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_003, testing::ext::TestSize.Level0)
{
    std::string msg = R"(
    {
        "TYPE": "xx"
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_004, testing::ext::TestSize.Level0)
{
    std::string msg = R"(
    {
        "TYPE": 0
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    msg = R"(
    {
        "TYPE": 1
    }
    )";
    relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    msg = R"(
    {
        "TYPE": 2
    }
    )";
    relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    msg = R"(
    {
        "TYPE": 3
    }
    )";
    relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}

HWTEST_F(ReleationShipSyncMgrTest, ParseTrustRelationShipChange_005, testing::ext::TestSize.Level0)
{
    std::string msg = R"(
    {
        "TYPE": 0,
        "VALUE": ""
    }
    )";
    RelationShipChangeMsg relationShipMsg =
        ReleationShipSyncMgr::GetInstance().ParseTrustRelationShipChange(msg);
    ASSERT_EQ(relationShipMsg.userId, UINT32_MAX);
}
}
} // namespace DistributedHardware
} // namespace OHOS
