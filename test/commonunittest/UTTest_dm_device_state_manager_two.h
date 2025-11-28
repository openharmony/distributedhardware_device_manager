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
#ifndef OHOS_UTTEST_DM_DEVICE_STATE_MANAGER_TWO_H
#define OHOS_UTTEST_DM_DEVICE_STATE_MANAGER_TWO_H

#include <gtest/gtest.h>
#include <refbase.h>

#include "softbus_connector_mock.h"
#include "device_manager_service_listener.h"
#include "softbus_session_callback.h"
#include "dm_crypto_mock.h"
#include "multiple_user_connector_mock.h"

namespace OHOS {
namespace DistributedHardware {
class DmDeviceStateManagerTestTwo : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static inline std::shared_ptr<SoftbusConnectorMock> softbusConnectorMock_ =
        std::make_shared<SoftbusConnectorMock>();
    static inline std::shared_ptr<CryptoMock> cryptoMock_ =
        std::make_shared<CryptoMock>();
    static inline std::shared_ptr<MultipleUserConnectorMock> multipleUserConnectorMock_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
