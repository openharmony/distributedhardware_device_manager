/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_advertise_manager.h"
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t VALUABLE_PUBLISH_ID = 10;
constexpr int32_t DEFAULT_META_TYPE = 10;
constexpr int32_t DEFAULT_DISC_MODE = 20;
constexpr int32_t DEFAULT_DISC_FREQ = 30;
const std::string PKG_NAME = "pkgName";
const std::string DEFAULT_CUSTOM_DATA = "CUSTOM_DATA";
} // namespace
void AdvertiseManagerTest::SetUp()
{}

void AdvertiseManagerTest::TearDown()
{}

void AdvertiseManagerTest::SetUpTestCase()
{}

void AdvertiseManagerTest::TearDownTestCase()
{}

HWTEST_F(AdvertiseManagerTest, StartAdvertising_001, testing::ext::TestSize.Level1)
{
    std::string stopTime = std::to_string(100);
    auto softbusListener = std::make_shared<SoftbusListener>();
    auto advertiseManager = std::make_shared<AdvertiseManager>(softbusListener);

    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_DISC_CAPABILITY] = DM_CAPABILITY_APPROACH;
    advertiseParam[PARAM_KEY_CUSTOM_DATA] = DEFAULT_CUSTOM_DATA;
    advertiseParam[PARAM_KEY_AUTO_STOP_ADVERTISE] = stopTime;
    advertiseParam[PARAM_KEY_META_TYPE] = std::to_string(DEFAULT_META_TYPE);
    advertiseParam[PARAM_KEY_PUBLISH_ID] = std::to_string(VALUABLE_PUBLISH_ID);
    advertiseParam[PARAM_KEY_DISC_MODE] = std::to_string(DEFAULT_DISC_MODE);
    advertiseParam[PARAM_KEY_DISC_FREQ] = std::to_string(DEFAULT_DISC_FREQ);
    auto ret = advertiseManager->StartAdvertising(PKG_NAME, advertiseParam);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AdvertiseManagerTest, StartAdvertising_002, testing::ext::TestSize.Level1)
{
    std::string stopTime = std::to_string(-5);
    auto softbusListener = std::make_shared<SoftbusListener>();
    auto advertiseManager = std::make_shared<AdvertiseManager>(softbusListener);

    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_DISC_CAPABILITY] = DM_CAPABILITY_APPROACH;
    advertiseParam[PARAM_KEY_CUSTOM_DATA] = DEFAULT_CUSTOM_DATA;
    advertiseParam[PARAM_KEY_AUTO_STOP_ADVERTISE] = stopTime;
    advertiseParam[PARAM_KEY_META_TYPE] = std::to_string(DEFAULT_META_TYPE);
    advertiseParam[PARAM_KEY_PUBLISH_ID] = std::to_string(VALUABLE_PUBLISH_ID);
    advertiseParam[PARAM_KEY_DISC_MODE] = std::to_string(DEFAULT_DISC_MODE);
    advertiseParam[PARAM_KEY_DISC_FREQ] = std::to_string(DEFAULT_DISC_FREQ);
    auto ret = advertiseManager->StartAdvertising(PKG_NAME, advertiseParam);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(AdvertiseManagerTest, StartAdvertising_003, testing::ext::TestSize.Level2)
{
    std::string emptyPackageName;
    std::string stopTime = std::to_string(100);
    auto softbusListener = std::make_shared<SoftbusListener>();
    auto advertiseManager = std::make_shared<AdvertiseManager>(softbusListener);

    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_DISC_CAPABILITY] = DM_CAPABILITY_APPROACH;
    advertiseParam[PARAM_KEY_CUSTOM_DATA] = DEFAULT_CUSTOM_DATA;
    advertiseParam[PARAM_KEY_AUTO_STOP_ADVERTISE] = stopTime;
    advertiseParam[PARAM_KEY_META_TYPE] = std::to_string(DEFAULT_META_TYPE);
    advertiseParam[PARAM_KEY_PUBLISH_ID] = std::to_string(VALUABLE_PUBLISH_ID);
    advertiseParam[PARAM_KEY_DISC_MODE] = std::to_string(DEFAULT_DISC_MODE);
    advertiseParam[PARAM_KEY_DISC_FREQ] = std::to_string(DEFAULT_DISC_FREQ);
    auto ret = advertiseManager->StartAdvertising(emptyPackageName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(AdvertiseManagerTest, HandleAutoStopAdvertise_001, testing::ext::TestSize.Level1)
{
    auto softbusListener = std::make_shared<SoftbusListener>();
    auto advertiseManager = std::make_shared<AdvertiseManager>(softbusListener);
    std::string timerName = "timer";
    advertiseManager->HandleAutoStopAdvertise(timerName, PKG_NAME, VALUABLE_PUBLISH_ID);
    ASSERT_TRUE(advertiseManager->softbusListener_ != nullptr);
}

HWTEST_F(AdvertiseManagerTest, StopAdvertising_001, testing::ext::TestSize.Level2)
{
    std::string emptyPackageName;
    auto softbusListener = std::make_shared<SoftbusListener>();
    auto advertiseManager = std::make_shared<AdvertiseManager>(softbusListener);
    auto ret = advertiseManager->StopAdvertising(emptyPackageName, VALUABLE_PUBLISH_ID);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(AdvertiseManagerTest, StopAdvertising_002, testing::ext::TestSize.Level1)
{
    auto softbusListener = std::make_shared<SoftbusListener>();
    auto advertiseManager = std::make_shared<AdvertiseManager>(softbusListener);
    auto innerPublishId = advertiseManager->GenInnerPublishId(PKG_NAME, VALUABLE_PUBLISH_ID);
    auto ret = advertiseManager->StopAdvertising(PKG_NAME, VALUABLE_PUBLISH_ID);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
