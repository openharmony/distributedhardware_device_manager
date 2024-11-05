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
#ifndef OHOS_UTTEST_DM_SERVICE_H
#define OHOS_UTTEST_DM_SERVICE_H

#include <gtest/gtest.h>
#include <memory>
#include <refbase.h>

#include <string>
#include <vector>

#include "device_manager_service.h"
#include "device_manager_service_listener.h"
#include "device_manager_service_impl.h"
#include "dm_single_instance.h"
#include "common_event_support.h"
#include "permission_manager_mock.h"
#include "softbus_listener_mock.h"
#include "app_manager_mock.h"
#include "kv_adapter_manager_mock.h"
#include "device_manager_service_impl_mock.h"
namespace OHOS {
namespace DistributedHardware {
class DeviceManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline std::shared_ptr<PermissionManagerMock> permissionManagerMock_ =
        std::make_shared<PermissionManagerMock>();
    static inline  std::shared_ptr<SoftbusListenerMock> softbusListenerMock_ =
        std::make_shared<SoftbusListenerMock>();
        std::make_shared<DeviceManagerServiceMock>();
    static inline  std::shared_ptr<AppManagerMock> appManagerMock_ =
        std::make_shared<AppManagerMock>();
    static inline  std::shared_ptr<KVAdapterManagerMock> kVAdapterManagerMock_ =
        std::make_shared<KVAdapterManagerMock>();
    static inline  std::shared_ptr<DeviceManagerServiceImplMock> deviceManagerServiceImplMock_ =
        std::make_shared<DeviceManagerServiceImplMock>();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
