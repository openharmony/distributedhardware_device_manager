/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "UTTest_softbus_publish.h"

#include <atomic>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "json_object.h"
#include "softbus_connector.h"
#include "softbus_error_code.h"
#include "system_ability_definition.h"

extern int32_t g_mockPublishLnnRet;
extern int32_t g_mockStopPublishLnnRet;
extern int32_t g_mockPublishLnnCallCount;
extern int32_t g_mockStopPublishLnnCallCount;
extern int32_t g_mockPublishLnnLastPublishId;
extern int32_t g_mockStopPublishLnnLastPublishId;

namespace OHOS {
namespace DistributedHardware {
extern std::atomic<bool> g_getLocalDevInfo;
extern DmDeviceInfo localDeviceInfo_;

void SoftbusPublishTest::SetUp()
{
    g_mockPublishLnnRet = DM_OK;
    g_mockStopPublishLnnRet = DM_OK;
    g_mockPublishLnnCallCount = 0;
    g_mockStopPublishLnnCallCount = 0;
    g_mockPublishLnnLastPublishId = -1;
    g_mockStopPublishLnnLastPublishId = -1;
    g_getLocalDevInfo = true;
    localDeviceInfo_ = {};
    localDeviceInfo_.deviceTypeId = DmDeviceType::DEVICE_TYPE_PHONE;
}
void SoftbusPublishTest::TearDown()
{
}
void SoftbusPublishTest::SetUpTestCase()
{
}
void SoftbusPublishTest::TearDownTestCase()
{
}

namespace {
HWTEST_F(SoftbusPublishTest, PublishSoftbusLNN_001, testing::ext::TestSize.Level1)
{
    g_mockPublishLnnRet = SOFTBUS_ERR;
    SoftbusPublish spftbusPublish;
    int32_t ret = spftbusPublish.PublishSoftbusLNN();
    int publishId = 1;
    SoftbusPublish::OnSoftbusPublishResult(publishId, PUBLISH_LNN_SUCCESS);
    EXPECT_EQ(ret, ERR_DM_PUBLISH_FAILED);
    EXPECT_EQ(g_mockPublishLnnCallCount, 1);
    EXPECT_EQ(g_mockPublishLnnLastPublishId, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
}

HWTEST_F(SoftbusPublishTest, PublishSoftbusLNN_002, testing::ext::TestSize.Level1)
{
    g_mockPublishLnnRet = DM_OK;
    SoftbusPublish spftbusPublish;
    int32_t ret = spftbusPublish.PublishSoftbusLNN();
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(g_mockPublishLnnCallCount, 1);
    EXPECT_EQ(g_mockPublishLnnLastPublishId, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
}

HWTEST_F(SoftbusPublishTest, StopPublishSoftbusLNN_001, testing::ext::TestSize.Level1)
{
    g_mockStopPublishLnnRet = SOFTBUS_ERR;
    SoftbusPublish spftbusPublish;
    int publishId = 2;
    int32_t ret = spftbusPublish.StopPublishSoftbusLNN(publishId);
    EXPECT_EQ(ret, ERR_DM_STOP_PUBLISH_LNN_FAILED);
    EXPECT_EQ(g_mockStopPublishLnnCallCount, 1);
    EXPECT_EQ(g_mockStopPublishLnnLastPublishId, publishId);
}

HWTEST_F(SoftbusPublishTest, StopPublishSoftbusLNN_002, testing::ext::TestSize.Level1)
{
    g_mockStopPublishLnnRet = DM_OK;
    SoftbusPublish spftbusPublish;
    int publishId = 2;
    int32_t ret = spftbusPublish.StopPublishSoftbusLNN(publishId);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(g_mockStopPublishLnnCallCount, 1);
    EXPECT_EQ(g_mockStopPublishLnnLastPublishId, publishId);
}

HWTEST_F(SoftbusPublishTest, PublishCommonEventCallback_001, testing::ext::TestSize.Level1)
{
    localDeviceInfo_.deviceTypeId = DmDeviceType::DEVICE_TYPE_WATCH;
    PublishCommonEventCallback(0, 0, DM_SCREEN_ON);
    EXPECT_EQ(g_mockPublishLnnCallCount, 0);
    EXPECT_EQ(g_mockStopPublishLnnCallCount, 0);
}

HWTEST_F(SoftbusPublishTest, PublishCommonEventCallback_002, testing::ext::TestSize.Level1)
{
    localDeviceInfo_.deviceTypeId = DmDeviceType::DEVICE_TYPE_PHONE;
    PublishCommonEventCallback(0, 0, DM_SCREEN_OFF);
    EXPECT_EQ(g_mockPublishLnnCallCount, 0);
    EXPECT_EQ(g_mockStopPublishLnnCallCount, 1);
    EXPECT_EQ(g_mockStopPublishLnnLastPublishId, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
