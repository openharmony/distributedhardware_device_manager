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
#include "UTTest_softbus_adapter.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"
#include "softbus_adapter.h"
#include "softbus_connector.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusAdapterTest::SetUp()
{
}
void SoftbusAdapterTest::TearDown()
{
}
void SoftbusAdapterTest::SetUpTestCase()
{
}
void SoftbusAdapterTest::TearDownTestCase()
{
}

namespace {
HWTEST_F(SoftbusAdapterTest, CreateSoftbusSessionServer_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string sessionName;
    int32_t ret = SoftbusAdapter::GetInstance().CreateSoftbusSessionServer(pkgName, sessionName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(SoftbusAdapterTest, RemoveSoftbusSessionServer_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string sessionName;
    int32_t ret = SoftbusAdapter::GetInstance().RemoveSoftbusSessionServer(pkgName, sessionName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(SoftbusAdapterTest, OnSoftbusSessionOpened_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    std::string str = "132131321345";
    PeerSocketInfo info = {
        .name = const_cast<char*>(str.c_str()),
        .pkgName = const_cast<char*>(str.c_str()),
        .networkId = const_cast<char*>(str.c_str()),
    };
    SoftbusAdapter::GetInstance().OnSoftbusSessionOpened(socket, info);
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, OnSoftbusSessionClosed_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_UNKNOWN;
    SoftbusAdapter::GetInstance().OnSoftbusSessionClosed(socket, reason);
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, OnBytesReceived_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    std::string data = "data";
    SoftbusAdapter::GetInstance().OnBytesReceived(socket, data.c_str(), data.size());
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, OnStreamReceived_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    StreamData data;
    StreamData exta;
    StreamFrameInfo frameInfo;
    SoftbusAdapter::GetInstance().OnStreamReceived(socket, &data, &exta, &frameInfo);
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, OnMessageReceived_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    std::string data = "data";
    SoftbusAdapter::GetInstance().OnMessageReceived(socket, data.c_str(), data.size());
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, OnQosEvent_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    QoSEvent eventId = static_cast<QoSEvent>(1);
    QosTV qosInfo[] = {
        { .qos = QOS_TYPE_MIN_BW, .value = 64 * 1024 },
        { .qos = QOS_TYPE_MAX_LATENCY, .value = 10000 },
        { .qos = QOS_TYPE_MIN_LATENCY, .value = 2500 },
    };
    uint32_t qosCount = 1;
    SoftbusAdapter::GetInstance().OnQosEvent(socket, eventId, qosInfo, qosCount);
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, DmOnSoftbusSessionBind_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    std::string str = "132131321345";
    PeerSocketInfo info = {
        .name = const_cast<char*>(str.c_str()),
        .pkgName = const_cast<char*>(str.c_str()),
        .networkId = const_cast<char*>(str.c_str()),
    };
    SoftbusAdapter::GetInstance().iSocketListener_.OnBind(socket, info);
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, DmOnSoftbusSessionClosed_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_UNKNOWN;
    SoftbusAdapter::GetInstance().iSocketListener_.OnShutdown(socket, reason);
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, DmOnBytesReceived_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    std::string data = "data";
    SoftbusAdapter::GetInstance().iSocketListener_.OnBytes(socket, data.c_str(), data.size());
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, DmOnStreamReceived_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    StreamData data;
    StreamData exta;
    StreamFrameInfo frameInfo;
    SoftbusAdapter::GetInstance().iSocketListener_.OnStream(socket, &data, &exta, &frameInfo);
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, DmOnMessageReceived_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    std::string data = "data";
    SoftbusAdapter::GetInstance().iSocketListener_.OnMessage(socket, data.c_str(), data.size());
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}

HWTEST_F(SoftbusAdapterTest, DmOnQosEvent_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    QoSEvent eventId = static_cast<QoSEvent>(1);
    QosTV qosInfo[] = {
        { .qos = QOS_TYPE_MIN_BW, .value = 64 * 1024 },
        { .qos = QOS_TYPE_MAX_LATENCY, .value = 10000 },
        { .qos = QOS_TYPE_MIN_LATENCY, .value = 2500 },
    };
    uint32_t qosCount = 1;
    SoftbusAdapter::GetInstance().iSocketListener_.OnQos(socket, eventId, qosInfo, qosCount);
    EXPECT_EQ(SoftbusAdapter::GetInstance().iSocketListener_.OnFile, nullptr);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
