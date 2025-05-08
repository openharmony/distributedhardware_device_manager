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

#ifndef OHOS_UTTEST_DM_SERVICE_IMPL_FIRST_H
#define OHOS_UTTEST_DM_SERVICE_IMPL_FIRST_H

#include <gtest/gtest.h>
#include <refbase.h>

#include <string>
#include <vector>

#include "device_manager_service_impl.h"
#include "device_manager_service_listener.h"
#include "deviceprofile_connector_mock.h"
#include "multiple_user_connector_mock.h"
#include "softbus_connector_mock.h"
#include "dm_device_state_manager_mock.h"
#include "device_manager_service_impl_mock.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerServiceImplFirstTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<IDeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DeviceManagerServiceImpl> deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    static inline std::shared_ptr<DeviceProfileConnectorMock> deviceProfileConnectorMock_ =
        std::make_shared<DeviceProfileConnectorMock>();
    static inline std::shared_ptr<MultipleUserConnectorMock> multipleUserConnectorMock_ =
        std::make_shared<MultipleUserConnectorMock>();
    static inline std::shared_ptr<SoftbusConnectorMock> softbusConnectorMock_ =
        std::make_shared<SoftbusConnectorMock>();
    static inline std::shared_ptr<DmDeviceStateManagerMock> dmDeviceStateManagerMock_ =
        std::make_shared<DmDeviceStateManagerMock>();
    static inline std::shared_ptr<DeviceManagerServiceImplMock> deviceManagerServiceImplMock_ =
        std::make_shared<DeviceManagerServiceImplMock>();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
