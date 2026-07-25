/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <functional>
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
    if (localDeviceName != nullptr) {
        delete[] localDeviceName;
    }
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceName_002, testing::ext::TestSize.Level0)
{
    unsigned int len = 0;
    int32_t result = OH_DeviceManager_GetLocalDeviceName(nullptr, len);

    EXPECT_EQ(result, ERR_INVALID_PARAMETER);
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceName_003, testing::ext::TestSize.Level0)
{
    char tmp[] = "nullptr";
    char *localDeviceName = tmp;
    unsigned int len = 0;
    int32_t result = OH_DeviceManager_GetLocalDeviceName(&localDeviceName, len);

    EXPECT_EQ(result, ERR_INVALID_PARAMETER);
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceName_004_Auto, testing::ext::TestSize.Level1)
{
    auto fn = OH_DeviceManager_GetLocalDeviceName;
    char *localDeviceName = nullptr;
    unsigned int len = 256;
    int32_t result = fn(&localDeviceName, len);

    EXPECT_EQ(result, DM_ERR_OBTAIN_BUNDLE_NAME);
    if (localDeviceName != nullptr) {
        delete[] localDeviceName;
    }
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceName_005_AddrOf, testing::ext::TestSize.Level1)
{
    using FnType = int32_t (*)(char **, unsigned int &);
    FnType fnPtr = &OH_DeviceManager_GetLocalDeviceName;
    char *localDeviceName = nullptr;
    unsigned int len = 256;
    int32_t result = fnPtr(&localDeviceName, len);

    EXPECT_EQ(result, DM_ERR_OBTAIN_BUNDLE_NAME);
    if (localDeviceName != nullptr) {
        delete[] localDeviceName;
    }
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceName_006_Decltype, testing::ext::TestSize.Level1)
{
    using FnType = decltype(&OH_DeviceManager_GetLocalDeviceName);
    FnType fnPtr = &OH_DeviceManager_GetLocalDeviceName;
    char *localDeviceName = nullptr;
    unsigned int len = 256;
    int32_t result = fnPtr(&localDeviceName, len);

    EXPECT_EQ(result, DM_ERR_OBTAIN_BUNDLE_NAME);
    if (localDeviceName != nullptr) {
        delete[] localDeviceName;
    }
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceName_007_StdFunction, testing::ext::TestSize.Level1)
{
    std::function<int32_t(char **, unsigned int &)> wrapper = OH_DeviceManager_GetLocalDeviceName;
    char *localDeviceName = nullptr;
    unsigned int len = 256;
    int32_t result = wrapper(&localDeviceName, len);

    EXPECT_EQ(result, DM_ERR_OBTAIN_BUNDLE_NAME);
    if (localDeviceName != nullptr) {
        delete[] localDeviceName;
    }
}

/* ---- New C ABI API: OH_DeviceManager_GetLocalDeviceNameC ---- */

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceNameC_001, testing::ext::TestSize.Level1)
{
    char *localDeviceName = nullptr;
    unsigned int len = 256;
    int32_t result = OH_DeviceManager_GetLocalDeviceNameC(&localDeviceName, &len);

    EXPECT_EQ(result, DM_ERR_OBTAIN_BUNDLE_NAME);
    if (localDeviceName != nullptr) {
        delete[] localDeviceName;
    }
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceNameC_002, testing::ext::TestSize.Level1)
{
    unsigned int len = 0;
    int32_t result = OH_DeviceManager_GetLocalDeviceNameC(nullptr, &len);

    EXPECT_EQ(result, ERR_INVALID_PARAMETER);
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceNameC_003, testing::ext::TestSize.Level1)
{
    char tmp[] = "nullptr";
    char *localDeviceName = tmp;
    unsigned int len = 0;
    int32_t result = OH_DeviceManager_GetLocalDeviceNameC(&localDeviceName, &len);

    EXPECT_EQ(result, ERR_INVALID_PARAMETER);
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceNameC_004_Auto, testing::ext::TestSize.Level1)
{
    auto cFn = &OH_DeviceManager_GetLocalDeviceNameC;
    char *localDeviceName = nullptr;
    unsigned int len = 256;
    int32_t result = cFn(&localDeviceName, &len);

    EXPECT_EQ(result, DM_ERR_OBTAIN_BUNDLE_NAME);
    if (localDeviceName != nullptr) {
        delete[] localDeviceName;
    }
}

HWTEST_F(OhDeviceManagerTest, OH_DeviceManager_GetLocalDeviceNameC_005_StdFunction, testing::ext::TestSize.Level1)
{
    std::function<int32_t(char **, unsigned int *)> cWrapper = OH_DeviceManager_GetLocalDeviceNameC;
    char *localDeviceName = nullptr;
    unsigned int len = 256;
    int32_t result = cWrapper(&localDeviceName, &len);

    EXPECT_EQ(result, DM_ERR_OBTAIN_BUNDLE_NAME);
    if (localDeviceName != nullptr) {
        delete[] localDeviceName;
    }
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
