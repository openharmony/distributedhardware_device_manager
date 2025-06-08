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
#include "UTTest_oh_device_manager.h"

#include <iostream>

#include "dm_client.h"
#include "dm_log.h"
#include "oh_device_manager_err_code.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void OhDeviceManagerTest::SetUp() {}

void OhDeviceManagerTest::TearDown() {}

void OhDeviceManagerTest::SetUpTestCase() {}

void OhDeviceManagerTest::TearDownTestCase() {}
namespace {
HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceName_001, testing::ext::TestSize.Level0)
{
    char *localDeviceName = nullptr;
    unsigned int len = 256;
    int32_t result = OH_DeviceManager_GetLocalDeviceName(&localDeviceName, len);

    EXPECT_EQ(result, DM_ERR_OBTAIN_BUNDLE_NAME);

    delete[] localDeviceName;
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
