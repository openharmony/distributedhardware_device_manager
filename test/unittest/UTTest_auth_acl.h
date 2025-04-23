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

#ifndef OHOS_DM_AUTH_REQUEST_STATE_TEST_H
#define OHOS_DM_AUTH_REQUEST_STATE_TEST_H

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <sstream>

#include "auth_request_state.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
class AuthAclTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
    static inline std::shared_ptr<HiChainAuthConnectorMock> dmHiChainAuthConnectorMock =
        std::make_shared<HiChainAuthConnectorMock>();
    static inline std::shared_ptr<SoftbusSessionMock> dmSoftbusSessionMock =
        std::make_shared<SoftbusSessionMock>();
    static inline std::shared_ptr<DmAuthStateMachineMock> dmAuthStateMachineMock =
        std::make_shared<DmAuthStateMachineMock>();
    static inline std::shared_ptr<SoftbusConnectorMock> dmSoftbusConnectorMock =
        std::make_shared<SoftbusConnectorMock>();
    std::shared_ptr<IDeviceManagerServiceListener> listener;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector;
    std::shared_ptr<HiChainConnector> hiChainConnector;
    std::shared_ptr<AuthManager> authManager;
    std::shared_ptr<DmAuthContext> context;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
