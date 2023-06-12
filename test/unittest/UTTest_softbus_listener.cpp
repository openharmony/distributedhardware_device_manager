/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "UTTest_softbus_listener.h"

#include <securec.h>
#include <unistd.h>
#include <cstdlib>
#include <thread>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusListenerTest::SetUp()
{
}
void SoftbusListenerTest::TearDown()
{
}
void SoftbusListenerTest::SetUpTestCase()
{
}
void SoftbusListenerTest::TearDownTestCase()
{
}

namespace {
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();

/**
 * @tc.name: ConvertNodeBasicInfoToDmDevice_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusListenerTest, ConvertNodeBasicInfoToDmDevice_001, testing::ext::TestSize.Level0)
{
    NodeBasicInfo nodeBasicInfo;
    DmDeviceInfo dmDeviceInfo;
    int32_t ret = softbusListener->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, dmDeviceInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnParameterChgCallback_001
 * @tc.desc: return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusListenerTest, OnParameterChgCallback_001, testing::ext::TestSize.Level0)
{
    const char *key = "123";
    const char *value = "1";
    void *context = nullptr;
    softbusListener->OnParameterChgCallback(key, value, context);
    NodeBasicInfo *info = nullptr;
    softbusListener->OnSoftBusDeviceOnline(info);
    softbusListener->OnSoftbusDeviceOffline(info);
    EXPECT_EQ(softbusListener->publishStatus, SoftbusListener::STATUS_UNKNOWN);
}

/**
 * @tc.name: ShiftLNNGear_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusListenerTest, ShiftLNNGear_001, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(softbusListener->ShiftLNNGear(), DM_OK);
}

/**
 * @tc.name: OnParameterChgCallback_002
 * @tc.desc: return true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusListenerTest, OnParameterChgCallback_002, testing::ext::TestSize.Level0)
{
    const char *key = "222";
    const char *value = "0";
    void *context = nullptr;
    softbusListener->OnParameterChgCallback(key, value, context);
    NodeBasicInfo info = {
            .networkId = "123456",
            .deviceName = "123456",
            .deviceTypeId = 1
        };
    softbusListener->OnSoftBusDeviceOnline(&info);
    softbusListener->OnSoftbusDeviceOffline(&info);
    sleep(1);
    EXPECT_EQ(softbusListener->publishStatus, SoftbusListener::STATUS_UNKNOWN);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS