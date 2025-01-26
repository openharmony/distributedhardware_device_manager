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

#ifndef OHOS_DEVICE_MANAGER_SERVICE_NOTIFY_TEST_H
#define OHOS_DEVICE_MANAGER_SERVICE_NOTIFY_TEST_H

#include <gtest/gtest.h>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <list>

#include "dm_device_info.h"
#include "dm_single_instance.h"
namespace OHOS {
namespace DistributedHardware {
class DeviceManagerServiceNotifyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DEVICE_MANAGER_SERVICE_NOTIFY_TEST_H
