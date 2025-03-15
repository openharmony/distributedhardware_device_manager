/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "UTTest_dm_comm_tool.h"
#include <memory>
#include "dm_error_type.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DMCommToolTest::SetUp()
{
    dmCommTool = DMCommTool::GetInstance();
    dmCommTool->Init();
}
void DMCommToolTest::TearDown()
{
}
void DMCommToolTest::SetUpTestCase()
{
    DmDMTransport::dMTransport_ = dmTransportMock_;
    DmSoftbusCache::dmSoftbusCache = softbusCacheMock_;
}
void DMCommToolTest::TearDownTestCase()
{
    DmDMTransport::dMTransport_ = nullptr;
    dmTransportMock_ = nullptr;
    DmSoftbusCache::dmSoftbusCache = nullptr;
    softbusCacheMock_ = nullptr;
}

/**
 * @tc.name: GetEventHandler_NotNull
 * @tc.type: FUNC
 */
HWTEST_F(DMCommToolTest, GetEventHandler_NotNull, testing::ext::TestSize.Level1)
{
    auto handler = dmCommTool->GetEventHandler();
    EXPECT_NE(handler, nullptr);
}

/**
 * @tc.name: GetDMTransportPtr_NotNull
 * @tc.type: FUNC
 */
HWTEST_F(DMCommToolTest, GetDMTransportPtr_NotNull, testing::ext::TestSize.Level1)
{
    auto transportPtr = dmCommTool->GetDMTransportPtr();
    EXPECT_NE(transportPtr, nullptr);
}

HWTEST_F(DMCommToolTest, SendUserIds_001, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "";
    std::vector<uint32_t> foregroundUserIds;
    std::vector<uint32_t> backgroundUserIds;
    int32_t ret = dmCommTool->SendUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    rmtNetworkId = "rmtNetworkId";
    foregroundUserIds.push_back(1);
    EXPECT_CALL(*dmTransportMock_, StartSocket(_, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = dmCommTool->SendUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    backgroundUserIds.push_back(10);
    EXPECT_CALL(*dmTransportMock_, StartSocket(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = dmCommTool->SendUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*dmTransportMock_, StartSocket(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(DM_OK));
    ret = dmCommTool->SendUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds);
    EXPECT_EQ(ret, DM_OK);

    int32_t socketId = 1;
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).Times(::testing::AtLeast(2)).WillOnce(Return(ERR_DM_FAILED));
    dmCommTool->RspLocalFrontOrBackUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds, socketId);

    std::string remoteNetworkId = "network******12";
    std::string strMsg = R"({
        "MsgType": "0",
        "userId": "12345",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, strMsg);
    socketId = 0;
    std::shared_ptr<InnerCommMsg> InnerCommMsg_ = std::make_shared<InnerCommMsg>(remoteNetworkId, commMsg_, socketId);
    std::string rmtUdid = "";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(rmtUdid), Return(ERR_DM_FAILED)));
    dmCommTool->ProcessReceiveUserIdsEvent(InnerCommMsg_);
    rmtUdid = "b*********12";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(rmtUdid), Return(ERR_DM_FAILED)));
    dmCommTool->ProcessReceiveUserIdsEvent(InnerCommMsg_);
}

HWTEST_F(DMCommToolTest, ProcessResponseUserIdsEvent_001, testing::ext::TestSize.Level1)
{
    std::string remoteNetworkId = "network******12";
    std::string strMsg = R"({
        "MsgType": "0",
        "userId": "12345",
        "syncUserIdFlag": 1,
        "foregroundUserIds": [10, 11, 12],
        "backgroundUserIds": [101, 112, 123],
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, strMsg);
    int32_t socketId = 0;
    std::shared_ptr<InnerCommMsg> InnerCommMsg_ = std::make_shared<InnerCommMsg>(remoteNetworkId, commMsg_, socketId);
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(""), Return(ERR_DM_FAILED)));
    dmCommTool->ProcessResponseUserIdsEvent(InnerCommMsg_);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("rmtUdid"), Return(ERR_DM_FAILED)));
    dmCommTool->ProcessResponseUserIdsEvent(InnerCommMsg_);

    std::string rmtNetworkId = "";
    std::vector<uint32_t> foregroundUserIds;
    std::vector<uint32_t> backgroundUserIds;
    int32_t ret = dmCommTool->SendUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
} // DistributedHardware
} // OHOS