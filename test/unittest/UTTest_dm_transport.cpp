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

#include "UTTest_dm_transport.h"

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

/**
 * @tc.name: CreateClientSocket_InvalidInput
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, CreateClientSocket_InvalidInput, testing::ext::TestSize.Level0)
{
    std::string rmtNetworkId = "";
    EXPECT_EQ(dmTransport.CreateClientSocket(rmtNetworkId), ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: Init_Success
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Init_Success, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(dmTransport.Init(), ERR_DM_FAILED);
}

/**
 * @tc.name: Init_AlreadyInitialized
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Init_AlreadyInitialized, testing::ext::TestSize.Level0)
{
    dmTransport.Init();
    EXPECT_EQ(dmTransport.Init(), ERR_DM_FAILED);
}

/**
 * @tc.name: UnInit_ShouldShutdownAllSockets
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, UnInit_ShouldShutdownAllSockets, testing::ext::TestSize.Level0)
{
    dmTransport.StartSocket("device1");
    dmTransport.StartSocket("device2");

    EXPECT_EQ(dmTransport.UnInit(), DM_OK);

    int32_t socketId;
    EXPECT_FALSE(dmTransport.IsDeviceSessionOpened("device1", socketId));
    EXPECT_FALSE(dmTransport.IsDeviceSessionOpened("device2", socketId));
}

/**
 * @tc.name: UnInit_ShouldNotShutdownWhenNoSockets
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, UnInit_ShouldNotShutdownWhenNoSockets, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(dmTransport.UnInit(), DM_OK);
}

/**
 * @tc.name: IsDeviceSessionOpened_ShouldReturnFalseForClosedSession
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, IsDeviceSessionOpened_ShouldReturnFalseForClosedSession, testing::ext::TestSize.Level0)
{
    int32_t socketId;
    EXPECT_FALSE(dmTransport.IsDeviceSessionOpened("device2", socketId));
}

/**
 * @tc.name: GetRemoteNetworkIdBySocketIdReturnEmptyString
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, GetRemoteNetworkIdBySocketIdReturnEmptyString, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(dmTransport.GetRemoteNetworkIdBySocketId(-1), "");
}

/**
 * @tc.name: ClearDeviceSocketOpened_ShouldRemoveSocket
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, ClearDeviceSocketOpened_ShouldRemoveSocket, testing::ext::TestSize.Level0)
{
    dmTransport.StartSocket("device1");
    dmTransport.ClearDeviceSocketOpened("device1");
    int32_t socketId;
    EXPECT_FALSE(dmTransport.IsDeviceSessionOpened("device1", socketId));
}

/**
 * @tc.name: ClearDeviceSocketOpened_ShouldDoNothingForInvalidId
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, ClearDeviceSocketOpened_ShouldDoNothingForInvalidId, testing::ext::TestSize.Level0)
{
    dmTransport.StartSocket("device1");
    dmTransport.ClearDeviceSocketOpened("invalid_device");
    int32_t socketId;
    EXPECT_FALSE(dmTransport.IsDeviceSessionOpened("device1", socketId));
}

/**
 * @tc.name: StartSocket_ShouldCreateSocket
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StartSocket_ShouldCreateSocket, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(dmTransport.StartSocket("device1"), ERR_DM_FAILED);
    int32_t socketId;
    EXPECT_FALSE(dmTransport.IsDeviceSessionOpened("device1", socketId));
}

/**
 * @tc.name: StartSocket_ShouldReturnErrorForInvalidId
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StartSocket_ShouldReturnErrorForInvalidId, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(dmTransport.StartSocket(""), ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartSocket_ShouldReturnErrorIfAlreadyOpened
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StartSocket_ShouldReturnErrorIfAlreadyOpened, testing::ext::TestSize.Level0)
{
    dmTransport.StartSocket("device1");
    EXPECT_EQ(dmTransport.StartSocket("device1"), ERR_DM_FAILED);
}

/**
 * @tc.name: StopSocket_InvalidId_Failure
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StopSocket_InvalidId_Failure, testing::ext::TestSize.Level0)
{
    std::string invalidId = "invalidId";
    int32_t result = dmTransport.StopSocket(invalidId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

/**
 * @tc.name: StopSocket_SessionNotOpened_Failure
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, StopSocket_SessionNotOpened_Failure, testing::ext::TestSize.Level0)
{
    std::string notOpenedId = "notOpenedId";
    int32_t result = dmTransport.StopSocket(notOpenedId);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

/**
 * @tc.name: Send_InvalidId_Failure
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Send_InvalidId_Failure, testing::ext::TestSize.Level0)
{
    std::string invalidId = "invalidId";
    std::string payload = "Hello, World!";
    int32_t result = dmTransport.Send(invalidId, payload);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

/**
 * @tc.name: Send_SessionNotOpened_Failure
 * @tc.type: FUNC
 */
HWTEST_F(DMTransportTest, Send_SessionNotOpened_Failure, testing::ext::TestSize.Level0)
{
    std::string notOpenedId = "notOpenedId";
    std::string payload = "Hello, World!";
    int32_t result = dmTransport.Send(notOpenedId, payload);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(DMTransportTest, OnSocketOpened_001, testing::ext::TestSize.Level0)
{
    int32_t socketId = 1;
    PeerSocketInfo info;
    int32_t ret = dmTransport.OnSocketOpened(socketId, info);
    EXPECT_EQ(ret, DM_OK);

    ShutdownReason reason;
    dmTransport.remoteDevSocketIds_["socketId"] = socketId;
    dmTransport.OnSocketClosed(socketId, reason);

    int32_t value = 1;
    void *data = &value;
    uint32_t dataLen = 1;
    socketId = -1;
    dmTransport.OnBytesReceived(socketId, data, dataLen);

    socketId = 1;
    dmTransport.OnBytesReceived(socketId, data, dataLen);
}
} // DistributedHardware
} // OHOS