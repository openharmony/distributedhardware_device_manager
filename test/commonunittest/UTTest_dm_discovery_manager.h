/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_UTTEST_DM_DISCOVERY_MANAGER_H
#define OHOS_UTTEST_DM_DISCOVERY_MANAGER_H

#include <gtest/gtest.h>
#include <refbase.h>
#include <queue>

#include "device_manager_service_listener.h"
#include "dm_discovery_manager.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_discover_result_req.h"
#include "softbus_connector.h"

namespace OHOS {
namespace DistributedHardware {
class DmDiscoveryManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_UTTEST_DM_DISCOVERY_MANAGER_H
