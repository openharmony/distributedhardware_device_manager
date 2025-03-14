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

#ifndef OHOS_DM_AUTH_MANAGER_FIRST_TEST_H
#define OHOS_DM_AUTH_MANAGER_FIRST_TEST_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <map>
#include <string>

#include "authentication.h"
#include "softbus_session.h"
#include "device_manager_service_listener.h"
#include "dm_adapter_manager.h"
#include "dm_constants.h"
#include "softbus_connector.h"
#include "hichain_connector.h"
#include "auth_request_state.h"
#include "auth_response_state.h"
#include "auth_message_processor.h"
#include "dm_timer.h"
#include "dm_auth_manager.h"
#include "softbus_session_mock.h"
#include "app_manager_mock.h"
#include "dm_crypto_mock.h"
#include "deviceprofile_connector_mock.h"
#include "hichain_auth_connector_mock.h"
#include "multiple_user_connector_mock.h"
#include "crypto_mgr_mock.h"

namespace OHOS {
namespace DistributedHardware {
class DmAuthManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();

    std::shared_ptr<DmAuthManager> authManager_ =
        std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector, listener, hiChainAuthConnector);
    static inline std::shared_ptr<SoftbusSessionMock> softbusSessionMock_ = nullptr;
    static inline std::shared_ptr<AppManagerMock> appManagerMock_ = nullptr;
    static inline std::shared_ptr<CryptoMock> cryptoMock_ = nullptr;
    static inline std::shared_ptr<DeviceProfileConnectorMock> deviceProfileConnectorMock_ = nullptr;
    static inline std::shared_ptr<HiChainAuthConnectorMock> hiChainAuthConnectorMock_ = nullptr;
    static inline std::shared_ptr<MultipleUserConnectorMock> multipleUserConnectorMock_ = nullptr;
    static inline std::shared_ptr<CryptoMgrMock> cryptoMgrMock_ = nullptr;
};

class AuthRequestFinishStateMock : public AuthRequestFinishState {
public:
    MOCK_METHOD(int32_t, GetStateType, (), (override));
};

class AuthRequestInitStateMock : public AuthRequestInitState {
public:
    MOCK_METHOD(int32_t, GetStateType, (), (override));
};

class AuthResponseInitStateMock : public AuthResponseInitState {
public:
    MOCK_METHOD(int32_t, GetStateType, (), (override));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
