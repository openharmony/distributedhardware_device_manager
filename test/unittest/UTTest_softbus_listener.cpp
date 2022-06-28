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

#include "UTTest_softbus_listener.h"

#include "dm_constants.h"
#include "dm_log.h"
#include "softbus_listener.h"

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
 * @tc.name: Init_001
 * @tc.desc: go to the corrort case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusListenerTest, Init_001, testing::ext::TestSize.Level0)
{
    int ret = softbusListener->Init();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetTrustedDeviceList_001
 * @tc.desc:  Returns a new pointer to the constructor
 * to construct an environment where the device has been discovered, and stop discovering
 * the device. Its return value is ERR_DM_INPUT_PARAMETER_EMPTY
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusListenerTest, GetTrustedDeviceList_001, testing::ext::TestSize.Level0)
{
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = softbusListener->GetTrustedDeviceList(deviceList);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetLocalDeviceInfo_001
 * @tc.desc:  Returns a new pointer to the constructor
 * to construct an environment where the device has been discovered, and stop discovering
 * the device. Its return value is ERR_DM_INPUT_PARAMETER_EMPTY
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusListenerTest, GetLocalDeviceInfo_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    int32_t ret = softbusListener->GetLocalDeviceInfo(deviceInfo);
    ASSERT_EQ(ret, DM_OK);
}
}
} // namespace DistributedHardware
} // namespace OHOS
