/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_AUTH_MANAGER_THIRD_TEST_H
#define OHOS_DM_AUTH_MANAGER_THIRD_TEST_H

#include <map>
#include <string>
#include <gtest/gtest.h>

#include "authentication.h"
#include "softbus_session.h"
#include "device_manager_service_listener.h"
#include "dm_constants.h"
#include "softbus_connector.h"
#include "hichain_connector.h"
#include "auth_request_state.h"
#include "auth_response_state.h"
#include "auth_message_processor.h"
#include "dm_timer.h"
#include "dm_auth_manager.h"

namespace OHOS {
namespace DistributedHardware {
class DmAuthManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<DmAuthManager> authManager_;
};

class SoftbusStateCallbackMock : public ISoftbusStateCallback {
public:
    SoftbusStateCallbackMock() {}
    virtual ~SoftbusStateCallbackMock() {}
    void OnDeviceOnline(std::string deviceId, int32_t authForm) {}
    void OnDeviceOffline(std::string deviceId) {}
    void DeleteOffLineTimer(std::string udidHash) {}
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
