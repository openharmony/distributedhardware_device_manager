/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_UTTEST_DEVICE_MANAGER_SERVICE_IMPL_3RD_H
#define OHOS_UTTEST_DEVICE_MANAGER_SERVICE_IMPL_3RD_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <string>

#include "device_manager_service_impl_3rd.h"

namespace OHOS {
namespace DistributedHardware {

class DeviceManagerServiceImpl3rdTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();

protected:
    std::shared_ptr<DeviceManagerServiceImpl3rd> serviceImpl_;
};

} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_UTTEST_DEVICE_MANAGER_SERVICE_IMPL_3RD_H