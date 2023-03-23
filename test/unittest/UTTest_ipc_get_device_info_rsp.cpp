/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "UTTest_ipc_get_device_info_rsp.h"

namespace OHOS {
namespace DistributedHardware {
void IpcGetDeviceInfoRspTest::SetUp()
{
}

void IpcGetDeviceInfoRspTest::TearDown()
{
}

void IpcGetDeviceInfoRspTest::SetUpTestCase()
{
}

void IpcGetDeviceInfoRspTest::TearDownTestCase()
{
}

namespace {

/**
 * @tc.name: GetDeviceInfo_001
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcGetDeviceInfoRspTest, GetDeviceInfo_001, testing::ext::TestSize.Level0)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
    };
    std::shared_ptr<IpcGetDeviceInfoRsp> rsp = std::make_shared<IpcGetDeviceInfoRsp>();
    rsp->SetDeviceInfo(deviceInfo);
    DmDeviceInfo info = rsp->GetDeviceInfo();
    int32_t ret = strcmp(info.deviceId, deviceInfo.deviceId);
    EXPECT_EQ(ret, 0);
    strcmp(info.deviceName, deviceInfo.deviceName);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(info.deviceTypeId, deviceInfo.deviceTypeId);
}

/**
 * @tc.name: GetDeviceInfo_002
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcGetDeviceInfoRspTest, GetDeviceInfo_002, testing::ext::TestSize.Level0)
{
    DmDeviceInfo deviceInfo;
    std::shared_ptr<IpcGetDeviceInfoRsp> rsp = std::make_shared<IpcGetDeviceInfoRsp>();
    rsp->SetDeviceInfo(deviceInfo);
    DmDeviceInfo info = rsp->GetDeviceInfo();
    std::string deviceName = std::string(info.deviceName);
    ASSERT_TRUE(deviceName.empty());
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS