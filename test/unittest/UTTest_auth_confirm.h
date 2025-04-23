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

#ifndef UTTEST_AUTH_CONFIRM_H
#define UTTEST_AUTH_CONFIRM_H

#include <gtest/gtest.h>

#include "auth_manager.h"
#include "deviceprofile_connector_mock.h"

namespace OHOS {
namespace DistributedHardware {

class AuthConfirmTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
    std::shared_ptr<SoftbusConnector> softbusConnector;
    std::shared_ptr<IDeviceManagerServiceListener> listener;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector;
    std::shared_ptr<HiChainConnector> hiChainConnector;
    std::shared_ptr<AuthManager> authManager;
    std::shared_ptr<DmAuthContext> context;
    static std::shared_ptr<DeviceProfileConnectorMock> deviceProfileConnectorMock;
};

}  // end namespace DistributedHardware
}  // end namespace OHOS
#endif  // end UTTEST_AUTH_CONFIRM_H