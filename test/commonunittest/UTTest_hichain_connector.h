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
#ifndef OHOS_UTTEST_HICHAIN_CONNECTOR_H
#define OHOS_UTTEST_HICHAIN_CONNECTOR_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <refbase.h>
#include <string>
#include <memory>
#include <cstdint>
#include <map>
#include <vector>

#include "json_object.h"
#include "device_auth.h"
#include "dm_single_instance.h"
#include "hichain_connector_callback.h"
#include "device_manager_service_listener.h"
#include "dm_auth_manager.h"
#include "dm_device_state_manager.h"
#include "hichain_connector.h"

namespace OHOS {
namespace DistributedHardware {
class HichainConnectorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class MockIHiChainConnectorCallback : public IHiChainConnectorCallback {
public:
    MockIHiChainConnectorCallback() = default;
    virtual ~MockIHiChainConnectorCallback() = default;
    MOCK_METHOD(void, OnGroupCreated, (int64_t requestId, const std::string &groupId), (override));
    MOCK_METHOD(void, OnMemberJoin, (int64_t requestId, int32_t status), (override));
    MOCK_METHOD(std::string, GetConnectAddr, (std::string deviceId), (override));
    MOCK_METHOD(int32_t, GetPinCode, (std::string &code), (override));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HICHAIN_CONNECTOR_H
