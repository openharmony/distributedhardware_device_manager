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

#ifndef UTTEST_AUTH_PIN_AUTH_STATE_H
#define UTTEST_AUTH_PIN_AUTH_STATE_H

#include <gtest/gtest.h>
#include "device_manager_service_listener.h"
#include "hichain_auth_connector_mock.h"
#include "auth_manager.h"
#include "dm_negotiate_process.h"
#include "distributed_device_profile_client_mock.h"

namespace OHOS {
namespace DistributedHardware {

class AuthPinAuthStateTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
    static inline std::shared_ptr<HiChainAuthConnectorMock> hiChainAuthConnectorMock =
        std::make_shared<HiChainAuthConnectorMock>();
    static inline std::shared_ptr<DistributedDeviceProfile::DistributedDeviceProfileClientMock>
        distributedDeviceProfileClientMock_ =
        std::make_shared<DistributedDeviceProfile::DistributedDeviceProfileClientMock>();
    std::shared_ptr<SoftbusConnector> softbusConnector;
    std::shared_ptr<IDeviceManagerServiceListener> listener;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector;
    std::shared_ptr<HiChainConnector> hiChainConnector;
    std::shared_ptr<AuthManager> authManager;
    std::shared_ptr<DmAuthContext> context;
};

}
}
#endif