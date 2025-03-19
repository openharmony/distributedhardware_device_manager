/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_device_manager_service_notify.h"
#include "device_manager_service_notify.h"
#include "dm_device_info.h"
#include "dm_constants.h"

#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerServiceNotify);
void DeviceManagerServiceNotifyTest::SetUp() {}

void DeviceManagerServiceNotifyTest::TearDown() {}

void DeviceManagerServiceNotifyTest::SetUpTestCase() {}

void DeviceManagerServiceNotifyTest::TearDownTestCase() {}
namespace {

HWTEST_F(DeviceManagerServiceNotifyTest, RegisterCallBack_001, testing::ext::TestSize.Level1)
{
    int32_t dmCommonNotifyEvent = 1;
    ProcessInfo processInfo;
    int32_t ret = DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    processInfo.pkgName = "pkgName";
    processInfo.userId = 2;
    ret = DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo);
    ASSERT_EQ(ret, DM_OK);

    dmCommonNotifyEvent = 10;
    ret = DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);

    dmCommonNotifyEvent = 2;
    ret = DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo);
    ASSERT_EQ(ret, DM_OK);

    std::set<ProcessInfo> processInfoSet;
    processInfoSet.insert(processInfo);
    DeviceManagerServiceNotify::GetInstance().callbackMap_
        .insert(std::make_pair(static_cast<DmCommonNotifyEvent>(dmCommonNotifyEvent), processInfoSet));
    ret = DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo);
    ASSERT_EQ(ret, DM_OK);

    dmCommonNotifyEvent = 3;
    DmCommonNotifyEvent notifyEvent = static_cast<DmCommonNotifyEvent>(dmCommonNotifyEvent);
    processInfoSet.insert(processInfo);
    DeviceManagerServiceNotify::GetInstance().callbackMap_[notifyEvent] = processInfoSet;
    processInfo.pkgName = "proName";
    processInfo.userId = 10;
    ret = DeviceManagerServiceNotify::GetInstance().RegisterCallBack(dmCommonNotifyEvent, processInfo);
    ASSERT_EQ(ret, DM_OK);

    dmCommonNotifyEvent = 4;
    std::set<ProcessInfo> processInfos;
    DeviceManagerServiceNotify::GetInstance().GetCallBack(dmCommonNotifyEvent, processInfos);

    dmCommonNotifyEvent = 3;
    DeviceManagerServiceNotify::GetInstance().GetCallBack(dmCommonNotifyEvent, processInfos);

    ProcessInfo processInfo1;
    DeviceManagerServiceNotify::GetInstance().callbackMap_[notifyEvent].insert(processInfo1);
    DeviceManagerServiceNotify::GetInstance().ClearDiedProcessCallback(processInfo1);
}
}
} // namespace DistributedHardware
} // namespace OHOS
