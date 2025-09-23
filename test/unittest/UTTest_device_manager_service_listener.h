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
#ifndef OHOS_UTTEST_DM_SERVICE_LISTENER_H
#define OHOS_UTTEST_DM_SERVICE_LISTENER_H

#include <gtest/gtest.h>
#include <refbase.h>
#include <string>

#include "device_manager_service_listener.h"
#include "dm_device_info.h"
#include "ipc_server_listener.h"
#include "dm_softbus_cache_mock.h"
#include "dm_crypto_mock.h"
#include "ipc_server_listener_mock.h"
#include "kv_adapter_manager_mock.h"
#include "app_manager_mock.h"
#include "device_manager_service_mock.h"
namespace OHOS {
namespace DistributedHardware {
class DeviceManagerServiceListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline  std::shared_ptr<SoftbusCacheMock> softbusCacheMock_ =
        std::make_shared<SoftbusCacheMock>();
    static inline  std::shared_ptr<CryptoMock> cryptoMock_ = std::make_shared<CryptoMock>();
    static inline  std::shared_ptr<IpcServerListenerMock> ipcServerListenerMock_ =
        std::make_shared<IpcServerListenerMock>();
    static inline  std::shared_ptr<KVAdapterManagerMock> kVAdapterManagerMock_ =
        std::make_shared<KVAdapterManagerMock>();
    static inline  std::shared_ptr<AppManagerMock> appManagerMock_ = std::make_shared<AppManagerMock>();
    static inline  std::shared_ptr<DeviceManagerServiceMock> deviceManagerServiceMock_ =
        std::make_shared<DeviceManagerServiceMock>();
};
} // namespace DistributedHardware
} // namespace OHOS
#undef private
#undef protected
#endif
