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

#ifndef OHOS_UTTEST_DM_CREDENTIAL_MANAGER_H
#define OHOS_UTTEST_DM_CREDENTIAL_MANAGER_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "device_auth.h"
#include "dm_credential_manager.h"
#include "device_manager_service_listener.h"
#include "dm_timer.h"
#include "hichain_connector.h"

namespace OHOS {
namespace DistributedHardware {
class DmCredentialManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<DeviceManagerServiceListener> listener_;
    std::shared_ptr<HiChainConnector> hiChainConnector_;
    std::shared_ptr<DmCredentialManager> dmCreMgr_;
};

class MockDeviceManagerServiceListener : public DeviceManagerServiceListener {
public:
    MockDeviceManagerServiceListener() = default;
    ~MockDeviceManagerServiceListener() = default;
    void OnCredentialResult(const ProcessInfo &processInfo, int32_t action, const std::string &resultInfo) override {}
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_UTTEST_DM_CREDENTIAL_MANAGER_H
