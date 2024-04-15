/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_PIN_HOLDER_TEST_H
#define OHOS_DM_PIN_HOLDER_TEST_H

#include <gtest/gtest.h>
#include <refbase.h>

#include <memory>
#include <cstdint>
#include "mock/mock_ipc_client_proxy.h"
#include "device_manager.h"
#include "single_instance.h"
#include "idevice_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
class DmPinHolderTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class DmInitCallbackTest : public DmInitCallback {
public:
    DmInitCallbackTest() : DmInitCallback() {}
    virtual ~DmInitCallbackTest() {}
    void OnRemoteDied() override {}
};

class DmPinHolderCallbackTest : public PinHolderCallback {
public:
    DmPinHolderCallbackTest() : PinHolderCallback() {}
    virtual ~DmPinHolderCallbackTest() {}
    void OnPinHolderCreate(const std::string &deviceId, DmPinType pinType, const std::string &payload) override;
    void OnPinHolderDestroy(DmPinType pinType, const std::string &payload) override;
    void OnCreateResult(int32_t result) override;
    void OnDestroyResult(int32_t result) override;
    void OnPinHolderEvent(DmPinHolderEvent event, int32_t result, const std::string &content) override;
};

class IDeviceManagerServiceListenerTest : public IDeviceManagerServiceListener {
public:
    virtual ~IDeviceManagerServiceListenerTest()
    {
    }

    void OnDeviceStateChange(const std::string &pkgName, const DmDeviceState &state, const DmDeviceInfo &info) override
    {
        (void)pkgName;
        (void)state;
        (void)info;
    }

    void OnDeviceFound(const std::string &pkgName, uint16_t subscribeId, const DmDeviceInfo &info) override
    {
        (void)pkgName;
        (void)subscribeId;
        (void)info;
    }

    void OnDeviceFound(const std::string &pkgName, uint16_t subscribeId, DmDeviceBasicInfo &info) override
    {
        (void)pkgName;
        (void)subscribeId;
        (void)info;
    }

    void OnDiscoveryFailed(const std::string &pkgName, uint16_t subscribeId, int32_t failedReason) override
    {
        (void)pkgName;
        (void)subscribeId;
        (void)failedReason;
    }

    void OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId) override
    {
        (void)pkgName;
        (void)subscribeId;
    }

    void OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult) override
    {
        (void)pkgName;
        (void)publishId;
        (void)publishResult;
    }

    void OnAuthResult(const std::string &pkgName, const std::string &deviceId, const std::string &token, int32_t status,
        int32_t reason) override
    {
        (void)pkgName;
        (void)deviceId;
        (void)token;
        (void)status;
        (void)reason;
    }

    void OnUiCall(std::string &pkgName, std::string &paramJson) override
    {
        (void)pkgName;
        (void)paramJson;
    }

    void OnCredentialResult(const std::string &pkgName, int32_t action, const std::string &resultInfo) override
    {
        (void)pkgName;
        (void)action;
        (void)resultInfo;
    }

    void OnBindResult(const std::string &pkgName, const PeerTargetId &targetId, int32_t result, int32_t status,
        std::string content) override
    {
        (void)pkgName;
        (void)targetId;
        (void)result;
        (void)status;
        (void)content;
    }

    void OnUnbindResult(const std::string &pkgName, const PeerTargetId &targetId, int32_t result,
        std::string content) override
    {
        (void)pkgName;
        (void)targetId;
        (void)result;
        (void)content;
    }

    void OnPinHolderCreate(const std::string &pkgName, const std::string &deviceId, DmPinType pinType,
        const std::string &payload) override
    {
        (void)pkgName;
        (void)deviceId;
        (void)pinType;
        (void)payload;
    }

    void OnPinHolderDestroy(const std::string &pkgName, DmPinType pinType, const std::string &payload) override
    {
        (void)pkgName;
        (void)pinType;
        (void)payload;
    }

    void OnCreateResult(const std::string &pkgName, int32_t result) override
    {
        (void)pkgName;
        (void)result;
    }

    void OnDestroyResult(const std::string &pkgName, int32_t result) override
    {
        (void)pkgName;
        (void)result;
    }

    void OnPinHolderEvent(const std::string &pkgName, DmPinHolderEvent event, int32_t result,
        const std::string &content) override
    {
        (void)pkgName;
        (void)event;
        (void)result;
        (void)content;
    }
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DM_PIN_HOLDER_TEST_H
