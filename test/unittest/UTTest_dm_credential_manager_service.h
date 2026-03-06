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

#ifndef OHOS_UTTEST_DM_CREDENTIAL_MANAGER_SERVICE_H
#define OHOS_UTTEST_DM_CREDENTIAL_MANAGER_SERVICE_H

#include <type_traits>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "device_manager_service_listener.h"
#include "hichain/dm_credential_manager.h"
#include "hichain/dm_service_hichain_connector.h"
#include "multiple_user_connector_mock.h"

namespace OHOS {
namespace DistributedHardware {

static_assert(std::is_base_of_v<IDeviceManagerServiceListener, DeviceManagerServiceListener>,
    "DeviceManagerServiceListener must derive from IDeviceManagerServiceListener.");
static_assert(!std::is_abstract_v<DeviceManagerServiceListener>,
    "DeviceManagerServiceListener became abstract. Update service listener implementation first.");

class DmCredentialManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<DmServiceHiChainConnector> hiChainConnector_;
    std::shared_ptr<DeviceManagerServiceListener> listener_;
    std::shared_ptr<DmCredentialManager> dmCreMgr_;
    static inline std::shared_ptr<MultipleUserConnectorMock> multipleUserConnectorMock_ =
        std::make_shared<MultipleUserConnectorMock>();
};

class MockDeviceManagerServiceListenerForCred : public DeviceManagerServiceListener {
public:
    MockDeviceManagerServiceListenerForCred() = default;
    ~MockDeviceManagerServiceListenerForCred() = default;
    void OnCredentialResult(const ProcessInfo &processInfo, int32_t action, const std::string &resultInfo) override {}
};

} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_UTTEST_DM_CREDENTIAL_MANAGER_SERVICE_H
