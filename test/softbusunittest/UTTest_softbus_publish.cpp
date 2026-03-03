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

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "json_object.h"
#include "softbus_connector.h"
#include "softbus_bus_center_mock.h"
#include "softbus_error_code.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
NodeBasicInfo BuildLocalNodeInfo(uint16_t deviceTypeId)
{
    NodeBasicInfo localNodeInfo = {};
    localNodeInfo.deviceTypeId = deviceTypeId;
    localNodeInfo.networkId[0] = '1';
    localNodeInfo.networkId[1] = '\0';
    localNodeInfo.deviceName[0] = 'd';
    localNodeInfo.deviceName[1] = '\0';
    return localNodeInfo;
}

void PrepareLocalDeviceInfoMock(uint16_t deviceTypeId)
{
    SoftbusCache::GetInstance().DeleteLocalDeviceInfo();
    SetLocalNodeDeviceInfoMockRet(DM_OK);
    SetLocalNodeDeviceInfoMock(BuildLocalNodeInfo(deviceTypeId));
}
} // namespace

void SoftbusPublishTest::SetUp()
{
    ResetSoftbusBusCenterMock();
    SetPublishLnnMockRet(DM_OK);
    SetStopPublishLnnMockRet(DM_OK);
    PrepareLocalDeviceInfoMock(static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PHONE));
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

HWTEST_F(SoftbusPublishTest, PublishSoftbusLNN_001, testing::ext::TestSize.Level1)
{
    SetPublishLnnMockRet(SOFTBUS_ERR);
    SoftbusPublish spftbusPublish;
    int32_t ret = spftbusPublish.PublishSoftbusLNN();
    int publishId = 1;
    SoftbusPublish::OnSoftbusPublishResult(publishId, PUBLISH_LNN_SUCCESS);
    EXPECT_EQ(ret, ERR_DM_PUBLISH_FAILED);
    EXPECT_EQ(GetPublishLnnMockCallCount(), 1);
    EXPECT_EQ(GetPublishLnnMockLastPublishId(), DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
}

HWTEST_F(SoftbusPublishTest, PublishSoftbusLNN_002, testing::ext::TestSize.Level1)
{
    SetPublishLnnMockRet(DM_OK);
    SoftbusPublish spftbusPublish;
    int32_t ret = spftbusPublish.PublishSoftbusLNN();
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(GetPublishLnnMockCallCount(), 1);
    EXPECT_EQ(GetPublishLnnMockLastPublishId(), DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
}

HWTEST_F(SoftbusPublishTest, StopPublishSoftbusLNN_001, testing::ext::TestSize.Level1)
{
    SetStopPublishLnnMockRet(SOFTBUS_ERR);
    SoftbusPublish spftbusPublish;
    int publishId = 2;
    int32_t ret = spftbusPublish.StopPublishSoftbusLNN(publishId);
    EXPECT_EQ(ret, ERR_DM_STOP_PUBLISH_LNN_FAILED);
    EXPECT_EQ(GetStopPublishLnnMockCallCount(), 1);
    EXPECT_EQ(GetStopPublishLnnMockLastPublishId(), publishId);
}

HWTEST_F(SoftbusPublishTest, StopPublishSoftbusLNN_002, testing::ext::TestSize.Level1)
{
    SetStopPublishLnnMockRet(DM_OK);
    SoftbusPublish spftbusPublish;
    int publishId = 2;
    int32_t ret = spftbusPublish.StopPublishSoftbusLNN(publishId);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(GetStopPublishLnnMockCallCount(), 1);
    EXPECT_EQ(GetStopPublishLnnMockLastPublishId(), publishId);
}

HWTEST_F(SoftbusPublishTest, PublishCommonEventCallback_001, testing::ext::TestSize.Level1)
{
    PrepareLocalDeviceInfoMock(static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_WATCH));
    PublishCommonEventCallback(0, 0, DM_SCREEN_ON);
    EXPECT_EQ(GetPublishLnnMockCallCount(), 0);
    EXPECT_EQ(GetStopPublishLnnMockCallCount(), 0);
}

HWTEST_F(SoftbusPublishTest, PublishCommonEventCallback_002, testing::ext::TestSize.Level1)
{
    PrepareLocalDeviceInfoMock(static_cast<uint16_t>(DmDeviceType::DEVICE_TYPE_PHONE));
    PublishCommonEventCallback(0, 0, DM_SCREEN_OFF);
    EXPECT_EQ(GetPublishLnnMockCallCount(), 0);
    EXPECT_EQ(GetStopPublishLnnMockCallCount(), 1);
    EXPECT_EQ(GetStopPublishLnnMockLastPublishId(), DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
}
} // namespace DistributedHardware
} // namespace OHOS
