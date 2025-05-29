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
    DMCommToolMock::dmDMCommTool = dmCommToolMock_;
}
void DMCommToolTest::TearDownTestCase()
{
    DmDMTransport::dMTransport_ = nullptr;
    dmTransportMock_ = nullptr;
    DmSoftbusCache::dmSoftbusCache = nullptr;
    softbusCacheMock_ = nullptr;
    dmCommToolMock_ = nullptr;
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

HWTEST_F(DMCommToolTest, UnInit_001, testing::ext::TestSize.Level1)
{
    dmCommTool->dmTransportPtr_ = nullptr;

    EXPECT_NO_THROW(dmCommTool->UnInit());
}

HWTEST_F(DMCommToolTest, UnInit_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*dmTransportMock_, UnInit()).Times(1);

    EXPECT_NO_THROW(dmCommTool->UnInit());
}

HWTEST_F(DMCommToolTest, SendMsg_001, testing::ext::TestSize.Level1)
{
    std::string invalidNetworkId = "";
    int32_t msgType = 1;
    std::string msg = "test message";

    int32_t ret = dmCommTool->SendMsg(invalidNetworkId, msgType, msg);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DMCommToolTest, SendMsg_002, testing::ext::TestSize.Level1)
{
    dmCommTool->dmTransportPtr_ = nullptr;
    std::string rmtNetworkId = "validNetworkId";
    int32_t msgType = 1;
    std::string msg = "test message";

    int32_t ret = dmCommTool->SendMsg(rmtNetworkId, msgType, msg);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DMCommToolTest, SendMsg_003, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t msgType = 1;
    std::string msg = "test message";

    EXPECT_CALL(*dmTransportMock_, StartSocket(rmtNetworkId, _))
        .WillOnce(Return(ERR_DM_FAILED));

    int32_t ret = dmCommTool->SendMsg(rmtNetworkId, msgType, msg);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, SendMsg_004, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t msgType = 1;
    std::string msg = "test message";

    EXPECT_CALL(*dmTransportMock_, StartSocket(rmtNetworkId, _))
        .WillOnce(DoAll(SetArgReferee<1>(-1), Return(DM_OK)));

    int32_t ret = dmCommTool->SendMsg(rmtNetworkId, msgType, msg);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, SendMsg_005, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t msgType = 1;
    std::string msg = "test message";

    EXPECT_CALL(*dmTransportMock_, StartSocket(rmtNetworkId, _))
        .WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(rmtNetworkId, _, 1))
        .WillOnce(Return(ERR_DM_FAILED));

    int32_t ret = dmCommTool->SendMsg(rmtNetworkId, msgType, msg);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, SendMsg_006, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t msgType = 1;
    std::string msg = "test message";

    EXPECT_CALL(*dmTransportMock_, StartSocket(rmtNetworkId, _))
        .WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(rmtNetworkId, _, 1))
        .WillOnce(Return(DM_OK));

    int32_t ret = dmCommTool->SendMsg(rmtNetworkId, msgType, msg);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DMCommToolTest, SendUserStop_001, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t stopUserId = 12345;

    EXPECT_CALL(*dmTransportMock_, StartSocket(rmtNetworkId, _))
        .Times(1).WillOnce(Return(DM_OK));
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).Times(0);

    int32_t ret = dmCommTool->SendUserStop(rmtNetworkId, stopUserId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, SendUserStop_002, testing::ext::TestSize.Level1)
{
    std::string invalidNetworkId = "";
    int32_t stopUserId = 12345;

    int32_t ret = dmCommTool->SendUserStop(invalidNetworkId, stopUserId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DMCommToolTest, SendUserStop_003, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t stopUserId = 12345;

    EXPECT_CALL(*dmTransportMock_, StartSocket(rmtNetworkId, _))
        .WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(rmtNetworkId, _, 1))
        .WillOnce(Return(ERR_DM_FAILED));

    int32_t ret = dmCommTool->SendUserStop(rmtNetworkId, stopUserId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, SendUserStop_004, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t stopUserId = 12345;

    EXPECT_CALL(*dmTransportMock_, StartSocket(rmtNetworkId, _))
        .WillOnce(Return(ERR_DM_FAILED));

    int32_t ret = dmCommTool->SendUserStop(rmtNetworkId, stopUserId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, SendUserStop_005, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t stopUserId = 12345;

    EXPECT_CALL(*dmTransportMock_, StartSocket(rmtNetworkId, _))
        .WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(rmtNetworkId, _, 1))
        .WillOnce(Return(DM_OK));

    int32_t ret = dmCommTool->SendUserStop(rmtNetworkId, stopUserId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DMCommToolTest, ParseUserStopMessage_001, testing::ext::TestSize.Level1)
{
    std::string invalidJson = "invalid_json";
    int32_t stopUserId = -1;

    int32_t result = DMCommTool::GetInstance()->ParseUserStopMessage(invalidJson, stopUserId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, ParseUserStopMessage_002, testing::ext::TestSize.Level1)
{
    std::string jsonWithoutKey = R"({ "otherKey": 12345 })";
    int32_t stopUserId = -1;

    int32_t result = DMCommTool::GetInstance()->ParseUserStopMessage(jsonWithoutKey, stopUserId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, ParseUserStopMessage_003, testing::ext::TestSize.Level1)
{
    std::string jsonWithInvalidValue = R"({ "stopUserId": "not_a_number" })";
    int32_t stopUserId = -1;

    int32_t result = DMCommTool::GetInstance()->ParseUserStopMessage(jsonWithInvalidValue, stopUserId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, ParseUserStopMessage_004, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "stopUserId": 12345 })";
    int32_t stopUserId = -1;

    int32_t result = DMCommTool::GetInstance()->ParseUserStopMessage(validJson, stopUserId);
    EXPECT_EQ(result, DM_OK);
    EXPECT_EQ(stopUserId, 12345);
}

HWTEST_F(DMCommToolTest, ProcessReceiveUserStopEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<InnerCommMsg> commMsg = nullptr;

    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUserStopEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUserStopEvent_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, "{}");
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(""), Return(ERR_DM_FAILED)));

    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUserStopEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUserStopEvent_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, "invalid_json");
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("validUdid"), Return(DM_OK)));

    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUserStopEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUserStopEvent_004, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "stopUserId": 12345 })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("validUdid"), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).Times(1);

    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUserStopEvent(commMsg));
}

HWTEST_F(DMCommToolTest, RspUserStop_001, testing::ext::TestSize.Level1)
{
    dmCommTool->dmTransportPtr_ = nullptr;
    std::string rmtNetworkId = "validNetworkId";
    int32_t socketId = 1;
    int32_t stopUserId = 12345;

    EXPECT_NO_THROW(dmCommTool->RspUserStop(rmtNetworkId, socketId, stopUserId));
}

HWTEST_F(DMCommToolTest, RspUserStop_002, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t invalidSocketId = -1;
    int32_t stopUserId = 12345;

    EXPECT_CALL(*dmTransportMock_, Send(rmtNetworkId, _, invalidSocketId))
    .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_NO_THROW(dmCommTool->RspUserStop(rmtNetworkId, invalidSocketId, stopUserId));
}

HWTEST_F(DMCommToolTest, RspUserStop_003, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t socketId = 1;
    int32_t stopUserId = 12345;

    EXPECT_CALL(*dmTransportMock_, Send(rmtNetworkId, _, socketId))
        .WillOnce(Return(ERR_DM_FAILED));

    EXPECT_NO_THROW(dmCommTool->RspUserStop(rmtNetworkId, socketId, stopUserId));
}

HWTEST_F(DMCommToolTest, RspUserStop_004, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "validNetworkId";
    int32_t socketId = 1;
    int32_t stopUserId = 12345;

    EXPECT_CALL(*dmTransportMock_, Send(rmtNetworkId, _, socketId))
        .WillOnce(Return(DM_OK));

    EXPECT_NO_THROW(dmCommTool->RspUserStop(rmtNetworkId, socketId, stopUserId));
}

HWTEST_F(DMCommToolTest, ProcessResponseUserStopEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<InnerCommMsg> commMsg = nullptr;

    EXPECT_NO_THROW(dmCommTool->ProcessResponseUserStopEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessResponseUserStopEvent_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, "{}");
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(""), Return(ERR_DM_FAILED)));

    EXPECT_NO_THROW(dmCommTool->ProcessResponseUserStopEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessResponseUserStopEvent_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, "invalid_json");
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("validUdid"), Return(DM_OK)));

    EXPECT_NO_THROW(dmCommTool->ProcessResponseUserStopEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessResponseUserStopEvent_004, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "stopUserId": 12345 })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);

    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("validUdid"), Return(DM_OK)));

    EXPECT_NO_THROW(dmCommTool->ProcessResponseUserStopEvent(commMsg));
}

HWTEST_F(DMCommToolTest, SendUninstAppObj_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    int32_t tokenId = 0;
    std::string networkId = "";
    int32_t result = dmCommTool->SendUninstAppObj(userId, tokenId, networkId);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DMCommToolTest, SendUninstAppObj_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    int32_t tokenId = 0;
    std::string networkId = "123456";
    dmCommTool->dmTransportPtr_ = nullptr;
    int32_t result = dmCommTool->SendUninstAppObj(userId, tokenId, networkId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, SendUninstAppObj_003, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    int32_t tokenId = 0;
    std::string networkId = "123456";

    EXPECT_CALL(*dmTransportMock_, StartSocket(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t result = dmCommTool->SendUninstAppObj(userId, tokenId, networkId);
    EXPECT_EQ(result, ERR_DM_FAILED);

    EXPECT_CALL(*dmTransportMock_, StartSocket(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    result = dmCommTool->SendUninstAppObj(userId, tokenId, networkId);
    EXPECT_EQ(result, ERR_DM_FAILED);

    EXPECT_CALL(*dmTransportMock_, StartSocket(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(DM_OK));
    result = dmCommTool->SendUninstAppObj(userId, tokenId, networkId);
    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(DMCommToolTest, RspAppUninstall_001, testing::ext::TestSize.Level1)
{
    int32_t socketId = 0;
    std::string rmtNetworkId = "";
    dmCommTool->dmTransportPtr_ = nullptr;
    int32_t result = dmCommTool->RspAppUninstall(rmtNetworkId, socketId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, RspAppUninstall_002, testing::ext::TestSize.Level1)
{
    int32_t socketId = 0;
    std::string rmtNetworkId = "";
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t result = dmCommTool->RspAppUninstall(rmtNetworkId, socketId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, RspAppUninstall_003, testing::ext::TestSize.Level1)
{
    int32_t socketId = 0;
    std::string rmtNetworkId = "";
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(DM_OK));
    int32_t result = dmCommTool->RspAppUninstall(rmtNetworkId, socketId);
    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(DMCommToolTest, RspAppUnbind_001, testing::ext::TestSize.Level1)
{
    int32_t socketId = 0;
    std::string rmtNetworkId = "";
    dmCommTool->dmTransportPtr_ = nullptr;
    int32_t result = dmCommTool->RspAppUnbind(rmtNetworkId, socketId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, RspAppUnbind_002, testing::ext::TestSize.Level1)
{
    int32_t socketId = 0;
    std::string rmtNetworkId = "";
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t result = dmCommTool->RspAppUnbind(rmtNetworkId, socketId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, RspAppUnbind_003, testing::ext::TestSize.Level1)
{
    int32_t socketId = 0;
    std::string rmtNetworkId = "";
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(DM_OK));
    int32_t result = dmCommTool->RspAppUnbind(rmtNetworkId, socketId);
    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(DMCommToolTest, SendUnBindAppObj_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    int32_t tokenId = 0;
    std::string extra = "";
    std::string networkId = "";
    std::string udid = "12211";
    int32_t result = dmCommTool->SendUnBindAppObj(userId, tokenId, extra, networkId, udid);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DMCommToolTest, SendUnBindAppObj_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    int32_t tokenId = 0;
    std::string extra = "";
    std::string networkId = "123456";
    std::string udid = "12211";
    dmCommTool->dmTransportPtr_ = nullptr;
    int32_t result = dmCommTool->SendUnBindAppObj(userId, tokenId, extra, networkId, udid);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMCommToolTest, SendUnBindAppObj_003, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    int32_t tokenId = 0;
    std::string extra = "";
    std::string networkId = "123456";
    std::string udid = "12211";

    EXPECT_CALL(*dmTransportMock_, StartSocket(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t result = dmCommTool->SendUnBindAppObj(userId, tokenId, extra, networkId, udid);
    EXPECT_EQ(result, ERR_DM_FAILED);

    EXPECT_CALL(*dmTransportMock_, StartSocket(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    result = dmCommTool->SendUnBindAppObj(userId, tokenId, extra, networkId, udid);
    EXPECT_EQ(result, ERR_DM_FAILED);

    EXPECT_CALL(*dmTransportMock_, StartSocket(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(DM_OK)));
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(DM_OK));
    result = dmCommTool->SendUnBindAppObj(userId, tokenId, extra, networkId, udid);
    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(DMCommToolTest, ProcessReceiveUninstAppEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<InnerCommMsg> commMsg = nullptr;
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUninstAppEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUninstAppEvent_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, "invalid_json");
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUninstAppEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUninstAppEvent_003, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "aaa" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUninstAppEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUninstAppEvent_004, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "1234", "tokenId": "1234" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUninstAppEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUnBindAppEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<InnerCommMsg> commMsg = nullptr;
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUnBindAppEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUnBindAppEvent_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, "invalid_json");
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUnBindAppEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUnBindAppEvent_003, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "aaa", "tokenId": "bbb" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUnBindAppEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveUnBindAppEvent_004, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "1234", "tokenId": "1234" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_CALL(*dmTransportMock_, Send(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveUnBindAppEvent(commMsg));
}

HWTEST_F(DMCommToolTest, StopSocket_001, testing::ext::TestSize.Level1)
{
    std::string networkId = "123456";
    dmCommTool->dmTransportPtr_ = nullptr;
    EXPECT_NO_THROW(dmCommTool->StopSocket(networkId));
}

HWTEST_F(DMCommToolTest, StopSocket_002, testing::ext::TestSize.Level1)
{
    std::string networkId = "123456";
    EXPECT_NO_THROW(dmCommTool->StopSocket(networkId));
}

HWTEST_F(DMCommToolTest, ProcessReceiveRspAppUninstallEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<InnerCommMsg> commMsg = nullptr;
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveRspAppUninstallEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveRspAppUninstallEvent_002, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "1234", "tokenId": "1234" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    dmCommTool->dmTransportPtr_ = nullptr;
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveRspAppUninstallEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveRspAppUninstallEvent_003, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "1234", "tokenId": "1234" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(""), Return(ERR_DM_FAILED)));
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveRspAppUninstallEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveRspAppUninstallEvent_004, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "1234", "tokenId": "1234" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("validUdid"), Return(DM_OK)));
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveRspAppUninstallEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveRspAppUnbindEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<InnerCommMsg> commMsg = nullptr;
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveRspAppUnbindEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveRspAppUnbindEvent_002, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "1234", "tokenId": "1234" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    dmCommTool->dmTransportPtr_ = nullptr;
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveRspAppUnbindEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveRspAppUnbindEvent_003, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "1234", "tokenId": "1234" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(""), Return(ERR_DM_FAILED)));
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveRspAppUnbindEvent(commMsg));
}

HWTEST_F(DMCommToolTest, ProcessReceiveRspAppUnbindEvent_004, testing::ext::TestSize.Level1)
{
    std::string validJson = R"({ "userId": "1234", "tokenId": "1234" })";
    std::shared_ptr<CommMsg> commMsg_ = std::make_shared<CommMsg>(1, validJson);
    std::shared_ptr<InnerCommMsg> commMsg = std::make_shared<InnerCommMsg>("networkId", commMsg_, 0);
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _))
        .WillOnce(DoAll(SetArgReferee<1>("validUdid"), Return(DM_OK)));
    EXPECT_NO_THROW(dmCommTool->ProcessReceiveRspAppUnbindEvent(commMsg));
}

} // DistributedHardware
} // OHOS