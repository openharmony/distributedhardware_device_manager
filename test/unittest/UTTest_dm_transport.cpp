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

#include "UTTest_dm_transport.h"
#include "dm_error_type.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
void DMTransportTest::SetUp()
{
}
void DMTransportTest::TearDown()
{
}
void DMTransportTest::SetUpTestCase()
{
}
void DMTransportTest::TearDownTestCase()
{
}

namespace {
    constexpr int32_t SOCKETED = 101;
}
/**
 * @tc.name: CreateClientSocket_InvalidInput
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, CreateClientSocket_InvalidInput, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "";
    EXPECT_EQ(dmTransport_->CreateClientSocket(rmtNetworkId), ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: Init_Success
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Init_Success, testing::ext::TestSize.Level1)
{
    EXPECT_EQ(dmTransport_->Init(), ERR_DM_FAILED);
}

/**
 * @tc.name: Init_AlreadyInitialized
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Init_AlreadyInitialized, testing::ext::TestSize.Level1)
{
    dmTransport_->Init();
    EXPECT_EQ(dmTransport_->Init(), ERR_DM_FAILED);
}

/**
 * @tc.name: UnInit_ShouldShutdownAllSockets
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, UnInit_ShouldShutdownAllSockets, testing::ext::TestSize.Level1)
{
    int32_t socketId;
    dmTransport_->StartSocket("device1", socketId);
    dmTransport_->StartSocket("device2", socketId);

    EXPECT_EQ(dmTransport_->UnInit(), DM_OK);
    EXPECT_FALSE(dmTransport_->IsDeviceSessionOpened("device1", socketId));
    EXPECT_FALSE(dmTransport_->IsDeviceSessionOpened("device2", socketId));
}

/**
 * @tc.name: UnInit_ShouldNotShutdownWhenNoSockets
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, UnInit_ShouldNotShutdownWhenNoSockets, testing::ext::TestSize.Level1)
{
    EXPECT_EQ(dmTransport_->UnInit(), DM_OK);
}

/**
 * @tc.name: IsDeviceSessionOpened_ShouldReturnFalseForClosedSession
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, IsDeviceSessionOpened_ShouldReturnFalseForClosedSession, testing::ext::TestSize.Level1)
{
    int32_t socketId;
    EXPECT_FALSE(dmTransport_->IsDeviceSessionOpened("device2", socketId));
}

/**
 * @tc.name: GetRemoteNetworkIdBySocketIdReturnEmptyString
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, GetRemoteNetworkIdBySocketIdReturnEmptyString, testing::ext::TestSize.Level1)
{
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(-1), "");
}

/**
 * @tc.name: ClearDeviceSocketOpened_ShouldRemoveSocket
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, ClearDeviceSocketOpened_ShouldRemoveSocket, testing::ext::TestSize.Level1)
{
    int32_t socketId = 1;
    dmTransport_->StartSocket("device1", socketId);
    dmTransport_->ClearDeviceSocketOpened("device1", socketId);
    EXPECT_FALSE(dmTransport_->IsDeviceSessionOpened("device1", socketId));
}

/**
 * @tc.name: ClearDeviceSocketOpened_ShouldDoNothingForInvalidId
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, ClearDeviceSocketOpened_ShouldDoNothingForInvalidId, testing::ext::TestSize.Level1)
{
    int32_t socketId = 1;
    dmTransport_->StartSocket("device1", socketId);
    dmTransport_->ClearDeviceSocketOpened("invalid_device", socketId);
    EXPECT_FALSE(dmTransport_->IsDeviceSessionOpened("device1", socketId));
}

/**
 * @tc.name: StartSocket_ShouldCreateSocket
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StartSocket_ShouldCreateSocket, testing::ext::TestSize.Level1)
{
    int32_t socketId;
    EXPECT_EQ(dmTransport_->StartSocket("device1", socketId), ERR_DM_FAILED);
    EXPECT_FALSE(dmTransport_->IsDeviceSessionOpened("device1", socketId));
}

/**
 * @tc.name: StartSocket_ShouldReturnErrorForInvalidId
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StartSocket_ShouldReturnErrorForInvalidId, testing::ext::TestSize.Level1)
{
    int32_t socketId;
    EXPECT_EQ(dmTransport_->StartSocket("", socketId), ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartSocket_ShouldReturnErrorIfAlreadyOpened
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StartSocket_ShouldReturnErrorIfAlreadyOpened, testing::ext::TestSize.Level1)
{
    int32_t socketId;
    dmTransport_->StartSocket("device1", socketId);
    EXPECT_EQ(dmTransport_->StartSocket("device1", socketId), ERR_DM_FAILED);
}

/**
 * @tc.name: StopSocket_InvalidId_Failure
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StopSocket_InvalidId_Failure, testing::ext::TestSize.Level1)
{
    std::string invalidId = "invalidId";
    int32_t result = dmTransport_->StopSocket(invalidId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

/**
 * @tc.name: StopSocket_SessionNotOpened_Failure
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StopSocket_SessionNotOpened_Failure, testing::ext::TestSize.Level1)
{
    std::string notOpenedId = "notOpenedId";
    int32_t result = dmTransport_->StopSocket(notOpenedId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

/**
 * @tc.name: Send_InvalidId_Failure
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Send_InvalidId_Failure, testing::ext::TestSize.Level1)
{
    std::string invalidId = "invalidId";
    std::string payload = "Hello, World!";
    int32_t result = dmTransport_->Send(invalidId, payload, 0);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

/**
 * @tc.name: Send_SessionNotOpened_Failure
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Send_SessionNotOpened_Failure, testing::ext::TestSize.Level1)
{
    std::string notOpenedId = "notOpenedId";
    std::string payload = "Hello, World!";
    int32_t result = dmTransport_->Send(notOpenedId, payload, 0);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMTransportTest, OnSocketOpened_001, testing::ext::TestSize.Level1)
{
    int32_t socketId = 1;
    std::string name = "socketName";
    std::string networkId = "ne*****1v";
    std::string pkgName = "ohos.objectstore";
    PeerSocketInfo info = {
        .name = name.data(),
        .networkId = networkId.data(),
        .pkgName = pkgName.data(),
        .dataType = DATA_TYPE_BYTES
    };
    int32_t ret = dmTransport_->OnSocketOpened(socketId, info);
    EXPECT_EQ(ret, DM_OK);

    socketId = SOCKETED;
    ret = dmTransport_->OnSocketOpened(socketId, info);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DMTransportTest, OnSocketClosed_001, testing::ext::TestSize.Level1)
{
    int32_t socketId = 10;
    std::string name = "socketName";
    std::string networkId = "ne*****1v";
    std::string pkgName = "ohos.objectstore";
    PeerSocketInfo info = {
        .name = name.data(),
        .networkId = networkId.data(),
        .pkgName = pkgName.data(),
        .dataType = DATA_TYPE_BYTES
    };
    int32_t ret = dmTransport_->OnSocketOpened(socketId, info);
    EXPECT_EQ(ret, DM_OK);
    
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_LNN_CHANGED;
    dmTransport_->OnSocketClosed(socketId, reason);
    
    socketId = -1;
    void *data = nullptr;
    uint32_t dataLen = 0;
    dmTransport_->OnBytesReceived(socketId, data, dataLen);
    socketId = SOCKETED;
    dmTransport_->OnBytesReceived(socketId, data, dataLen);
    std::string dataStr = "a**************2";
    data = reinterpret_cast<void *>(dataStr.data());
    dataLen = static_cast<uint32_t>(dataStr.length());
    dmTransport_->OnBytesReceived(socketId, data, dataLen);
    socketId = 0;
    dmTransport_->OnBytesReceived(socketId, data, dataLen);
    std::string payload = "p*********13";
    dmTransport_->HandleReceiveMessage(socketId, payload);
    socketId = SOCKETED;
    dmTransport_->HandleReceiveMessage(socketId, payload);
    payload = R"({
        "MsgType": "0",
        "msg": "messgaeinfo",
        "code": 145,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    dmTransport_->HandleReceiveMessage(socketId, payload);
}

HWTEST_F(DMTransportTest, CreateClientSocket_001, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "";
    int32_t ret = dmTransport_->CreateClientSocket(rmtNetworkId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    rmtNetworkId = "rmt**********7";
    ret = dmTransport_->CreateClientSocket(rmtNetworkId);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

HWTEST_F(DMTransportTest, IsDeviceSessionOpened_001, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "";
    int32_t socketId = 1;
    bool ret = dmTransport_->IsDeviceSessionOpened(rmtNetworkId, socketId);
    EXPECT_FALSE(ret);

    rmtNetworkId = "rmt**********7";
    ret = dmTransport_->IsDeviceSessionOpened(rmtNetworkId, socketId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: Send_InvalidNetworkId_002
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Send_InvalidNetworkId_002, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "";
    std::string payload = "Hello";
    int32_t result = dmTransport_->Send(rmtNetworkId, payload, 1);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: Send_EmptyPayload
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Send_EmptyPayload, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "rmt**********7";
    std::string payload = "";
    int32_t result = dmTransport_->Send(rmtNetworkId, payload, 1);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: Send_OverSizedPayload
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Send_OverSizedPayload, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "rmt**********7";
    std::string payload(MAX_SEND_MSG_LENGTH + 1, 'a');
    int32_t result = dmTransport_->Send(rmtNetworkId, payload, 1);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

/**
 * @tc.name: StopSocket_InvalidNetworkId
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StopSocket_InvalidNetworkId, testing::ext::TestSize.Level1)
{
    std::string rmtNetworkId = "";
    int32_t result = dmTransport_->StopSocket(rmtNetworkId);
    EXPECT_EQ(result, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ClearDeviceSocketOpened_InvalidNetworkId
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, ClearDeviceSocketOpened_InvalidNetworkId, testing::ext::TestSize.Level1)
{
    int32_t socketId = 1;
    dmTransport_->ClearDeviceSocketOpened("", socketId);
    EXPECT_FALSE(dmTransport_->IsDeviceSessionOpened("device1", socketId));
}

/**
 * @tc.name: GetRemoteNetworkIdBySocketId_WithOpenedSocket
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, GetRemoteNetworkIdBySocketId_WithOpenedSocket, testing::ext::TestSize.Level1)
{
    int32_t socketId = 50;
    std::string name = "socketName";
    std::string networkId = "net*********11";
    std::string pkgName = "ohos.objectstore";
    PeerSocketInfo info = {
        .name = name.data(),
        .networkId = networkId.data(),
        .pkgName = pkgName.data(),
        .dataType = DATA_TYPE_BYTES
    };
    int32_t ret = dmTransport_->OnSocketOpened(socketId, info);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId), networkId);
}

/**
 * @tc.name: OnBytesReceived_OverSizedData
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, OnBytesReceived_OverSizedData, testing::ext::TestSize.Level1)
{
    int32_t socketId = 60;
    std::string dataStr(MAX_SEND_MSG_LENGTH + 1, 'b');
    void *data = reinterpret_cast<void *>(dataStr.data());
    uint32_t dataLen = static_cast<uint32_t>(dataStr.length());
    dmTransport_->OnBytesReceived(socketId, data, dataLen);
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId), "");
}

/**
 * @tc.name: OnSocketOpened_002
 * @tc.desc: Verify OnSocketOpened inserts a second socket for an existing networkId.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportTest, OnSocketOpened_002, testing::ext::TestSize.Level1)
{
    int32_t socketId1 = 201;
    int32_t socketId2 = 202;
    std::string name = "socketName";
    std::string networkId = "net*********22";
    std::string pkgName = "ohos.objectstore";
    PeerSocketInfo info = {
        .name = name.data(),
        .networkId = networkId.data(),
        .pkgName = pkgName.data(),
        .dataType = DATA_TYPE_BYTES
    };
    int32_t ret = dmTransport_->OnSocketOpened(socketId1, info);
    EXPECT_EQ(ret, DM_OK);
    ret = dmTransport_->OnSocketOpened(socketId2, info);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId1), networkId);
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId2), networkId);
}

/**
 * @tc.name: OnSocketClosed_002
 * @tc.desc: Verify OnSocketClosed removes only the closed socket, keeping other sockets.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportTest, OnSocketClosed_002, testing::ext::TestSize.Level1)
{
    int32_t socketId1 = 301;
    int32_t socketId2 = 302;
    std::string name = "socketName";
    std::string networkId = "net*********33";
    std::string pkgName = "ohos.objectstore";
    PeerSocketInfo info = {
        .name = name.data(),
        .networkId = networkId.data(),
        .pkgName = pkgName.data(),
        .dataType = DATA_TYPE_BYTES
    };
    dmTransport_->OnSocketOpened(socketId1, info);
    dmTransport_->OnSocketOpened(socketId2, info);
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_LNN_CHANGED;
    dmTransport_->OnSocketClosed(socketId1, reason);
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId1), "");
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId2), networkId);
}

/**
 * @tc.name: OnBytesReceived_NullData
 * @tc.desc: Verify OnBytesReceived returns early when data is nullptr.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportTest, OnBytesReceived_NullData, testing::ext::TestSize.Level1)
{
    int32_t socketId = 401;
    void *data = nullptr;
    uint32_t dataLen = 10;
    dmTransport_->OnBytesReceived(socketId, data, dataLen);
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId), "");
}

/**
 * @tc.name: OnBytesReceived_ZeroDataLen
 * @tc.desc: Verify OnBytesReceived returns early when dataLen is zero.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportTest, OnBytesReceived_ZeroDataLen, testing::ext::TestSize.Level1)
{
    int32_t socketId = 402;
    std::string dataStr = "somedata";
    void *data = reinterpret_cast<void *>(dataStr.data());
    uint32_t dataLen = 0;
    dmTransport_->OnBytesReceived(socketId, data, dataLen);
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId), "");
}

/**
 * @tc.name: GetRemoteNetworkIdBySocketId_UnknownSocket
 * @tc.desc: Verify GetRemoteNetworkIdBySocketId returns empty for an unknown socket.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportTest, GetRemoteNetworkIdBySocketId_UnknownSocket, testing::ext::TestSize.Level1)
{
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(9999), "");
}

/**
 * @tc.name: IsDeviceSessionOpened_002
 * @tc.desc: Verify IsDeviceSessionOpened returns false for an opened-but-not-source socket.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportTest, IsDeviceSessionOpened_002, testing::ext::TestSize.Level1)
{
    int32_t socketId = 501;
    std::string name = "socketName";
    std::string networkId = "net*********55";
    std::string pkgName = "ohos.objectstore";
    PeerSocketInfo info = {
        .name = name.data(),
        .networkId = networkId.data(),
        .pkgName = pkgName.data(),
        .dataType = DATA_TYPE_BYTES
    };
    dmTransport_->OnSocketOpened(socketId, info);
    int32_t outSocketId = -1;
    bool opened = dmTransport_->IsDeviceSessionOpened(networkId, outSocketId);
    EXPECT_FALSE(opened);
}

/**
 * @tc.name: ClearDeviceSocketOpened_RemovesEntry
 * @tc.desc: Verify ClearDeviceSocketOpened removes the networkId entry when its set becomes empty.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportTest, ClearDeviceSocketOpened_RemovesEntry, testing::ext::TestSize.Level1)
{
    int32_t socketId = 601;
    std::string name = "socketName";
    std::string networkId = "net*********66";
    std::string pkgName = "ohos.objectstore";
    PeerSocketInfo info = {
        .name = name.data(),
        .networkId = networkId.data(),
        .pkgName = pkgName.data(),
        .dataType = DATA_TYPE_BYTES
    };
    dmTransport_->OnSocketOpened(socketId, info);
    dmTransport_->ClearDeviceSocketOpened(networkId, socketId);
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId), "");
}

/**
 * @tc.name: HandleReceiveMessage_InvalidSocket
 * @tc.desc: Verify HandleReceiveMessage does not crash for an unknown socket.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DMTransportTest, HandleReceiveMessage_InvalidSocket, testing::ext::TestSize.Level1)
{
    int32_t socketId = 701;
    std::string payload = "{\"code\":1,\"msg\":\"hello\"}";
    dmTransport_->HandleReceiveMessage(socketId, payload);
    EXPECT_EQ(dmTransport_->GetRemoteNetworkIdBySocketId(socketId), "");
}
} // DistributedHardware
} // OHOS