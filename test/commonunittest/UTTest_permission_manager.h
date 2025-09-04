/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_PERMISSION_MANAGER_TEST_H
#define OHOS_DM_PERMISSION_MANAGER_TEST_H

#include <gtest/gtest.h>

#include <cstdint>
#include <string_view>

#include "dm_accesstoken_kit_mock.h"
#include "dm_constants.h"
#include "dm_ipc_skeleton_mock.h"
#include "permission_manager.h"

namespace OHOS {
namespace DistributedHardware {
class PermissionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DMIPCSkeletonMock> ipcSkeletonMock_ = std::make_shared<DMIPCSkeletonMock>();
    static inline std::shared_ptr<DmAccessTokenKitMock> accessTokenKitMock_ = std::make_shared<DmAccessTokenKitMock>();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
