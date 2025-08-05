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
#ifndef OHOS_DM_SOFTBUS_CACHE_TEST_H
#define OHOS_DM_SOFTBUS_CACHE_TEST_H

#include <gtest/gtest.h>
#include <memory>
#include <refbase.h>

#include <string>
#include <vector>

#include "common_event_support.h"
#include "device_manager_service.h"
#include "device_manager_service_impl.h"
#include "device_manager_service_listener.h"
#include "dm_softbus_error_code.h"
#include "softbus_center_mock.h"

namespace OHOS {
namespace DistributedHardware {
class DMSoftbusCacheTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<SoftbusCenterMock> softbusCenterMock_ = std::make_shared<SoftbusCenterMock>();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
