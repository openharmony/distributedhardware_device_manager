/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_AUTH_ACL_TEST_H
#define OHOS_DM_AUTH_ACL_TEST_H


#include <gtest/gtest.h>
#include "distributed_device_profile_client_mock.h"
#include "hichain_auth_connector_mock.h"
#include "softbus_connector_mock.h"
#include "softbus_session_mock.h"
#include "auth_manager.h"

namespace OHOS {
namespace DistributedHardware {
class AuthAclTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
    static inline std::shared_ptr<DistributedDeviceProfile::DistributedDeviceProfileClientMock>
        distributedDeviceProfileClientMock_ =
        std::make_shared<DistributedDeviceProfile::DistributedDeviceProfileClientMock>();
    static inline std::shared_ptr<SoftbusConnectorMock> dmSoftbusConnectorMock =
        std::make_shared<SoftbusConnectorMock>();
    static inline std::shared_ptr<SoftbusSessionMock> dmSoftbusSessionMock =
        std::make_shared<SoftbusSessionMock>();
    std::shared_ptr<SoftbusConnector> softbusConnector;
    std::shared_ptr<IDeviceManagerServiceListener> listener;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector;
    std::shared_ptr<HiChainConnector> hiChainConnector;
    std::shared_ptr<AuthManager> authManager;
    std::shared_ptr<DmAuthContext> context;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
