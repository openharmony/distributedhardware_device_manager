/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_UTTEST_HICHAIN_AUTH_CONNECTOR_H
#define OHOS_UTTEST_HICHAIN_AUTH_CONNECTOR_H

#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "hichain_auth_connector.h"
#include "hichain_connector_callback.h"

namespace OHOS {
namespace DistributedHardware {
class HiChainAuthConnectorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<HiChainAuthConnector> hiChain_ = std::make_shared<HiChainAuthConnector>();
};

class MockIDmDeviceAuthCallback : public IDmDeviceAuthCallback {
public:
    MockIDmDeviceAuthCallback() = default;
    virtual ~MockIDmDeviceAuthCallback() = default;

    MOCK_METHOD(bool, AuthDeviceTransmit, (int64_t requestId, const uint8_t *data, uint32_t dataLen), (override));
    MOCK_METHOD(void, AuthDeviceFinish, (int64_t requestId), (override));
    MOCK_METHOD(void, AuthDeviceError, (int64_t requestId, int32_t errorCode), (override));
    MOCK_METHOD(void, AuthDeviceSessionKey,
                (int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen), (override));
    MOCK_METHOD(char *, AuthDeviceRequest, (int64_t requestId, int operationCode, const char *reqParams), (override));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_UTTEST_HICHAIN_AUTH_CONNECTOR_H
