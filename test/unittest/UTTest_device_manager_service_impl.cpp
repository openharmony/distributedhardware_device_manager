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

#include "UTTest_device_manager_service_impl.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceImplTest::SetUp()
{
}

void DeviceManagerServiceImplTest::TearDown()
{
}

void DeviceManagerServiceImplTest::SetUpTestCase()
{
}

void DeviceManagerServiceImplTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: Initialize_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, Initialize_001, testing::ext::TestSize.Level0)
{
    auto ideviceManagerServiceListener = std::shared_ptr<IDeviceManagerServiceListener>();
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>
    ();
    deviceManagerServiceImpl->commonEventManager_ = std::make_shared<DmCommonEventManager>();
    int ret = deviceManagerServiceImpl->Initialize(ideviceManagerServiceListener);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: Initialize_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, Initialize_002, testing::ext::TestSize.Level0)
{
    auto ideviceManagerServiceListener = std::shared_ptr<IDeviceManagerServiceListener>();
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>
    ();
    deviceManagerServiceImpl->commonEventManager_ = nullptr;
    int ret = deviceManagerServiceImpl->Initialize(ideviceManagerServiceListener);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterDevStateCallback_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterDevStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string extra;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->RegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterDevStateCallback_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterDevStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string extra;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->RegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: PraseNotifyEventJson_001
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_001, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra": {"deviceId": "123"})";
    nlohmann::json jsonObject;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_002, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra": {"deviceid": "123"})";
    nlohmann::json jsonObject;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_003
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_003, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra": {"deviceaId": 123})";
    nlohmann::json jsonObject;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_004
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_004, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra", {"deviceaId", 123})";
    nlohmann::json jsonObject;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_005
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_005, testing::ext::TestSize.Level0)
{
    std::string event = R"({"Extra", {"deviceaId", 123})";
    nlohmann::json jsonObject;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_006
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_006, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra":"123"})";
    nlohmann::json jsonObject;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_007
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_007, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra", {"deviceaId", 123})";
    nlohmann::json jsonObject;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: NotifyEvent_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, NotifyEvent_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t eventId = DM_NOTIFY_EVENT_START;
    std::string event = R"({"extra": {"deviceId": "123"})";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: NotifyEvent_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, NotifyEvent_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t eventId = DM_NOTIFY_EVENT_BUTT;
    std::string event = R"({"extra": {"deviceId": "123"})";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: NotifyEvent_003
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, NotifyEvent_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t eventId = DM_NOTIFY_EVENT_ONDEVICEREADY;
    std::string event = R"({"extra": {"deviceId": "123"}})";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
