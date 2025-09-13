/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "dm_device_info.h"
#include "dm_single_instance.h"
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

    void OnDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
        const DmDeviceInfo &info) override
    {
        (void)processInfo;
        (void)state;
        (void)info;
    }

    void OnDeviceFound(const ProcessInfo &processInfo, uint16_t subscribeId, const DmDeviceInfo &info) override
    {
        (void)processInfo;
        (void)subscribeId;
        (void)info;
    }

    void OnDiscoveryFailed(const ProcessInfo &processInfo, uint16_t subscribeId, int32_t failedReason) override
    {
        (void)processInfo;
        (void)subscribeId;
        (void)failedReason;
    }

    void OnDiscoverySuccess(const ProcessInfo &processInfo, int32_t subscribeId) override
    {
        (void)processInfo;
        (void)subscribeId;
    }

    void OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult) override
    {
        (void)pkgName;
        (void)publishId;
        (void)publishResult;
    }

    void OnAuthResult(const ProcessInfo &processInfo, const std::string &deviceId, const std::string &token,
        int32_t status, int32_t reason) override
    {
        (void)processInfo;
        (void)deviceId;
        (void)token;
        (void)status;
        (void)reason;
    }

    void OnUiCall(const ProcessInfo &processInfo, std::string &paramJson) override
    {
        (void)processInfo;
        (void)paramJson;
    }

    void OnCredentialResult(const ProcessInfo &processInfo, int32_t action, const std::string &resultInfo) override
    {
        (void)processInfo;
        (void)action;
        (void)resultInfo;
    }

    void OnBindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId, int32_t result, int32_t status,
        std::string content) override
    {
        (void)processInfo;
        (void)targetId;
        (void)result;
        (void)status;
        (void)content;
    }

    void OnUnbindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId, int32_t result,
        std::string content) override
    {
        (void)processInfo;
        (void)targetId;
        (void)result;
        (void)content;
    }

    void OnPinHolderCreate(const ProcessInfo &processInfo, const std::string &deviceId, DmPinType pinType,
        const std::string &payload) override
    {
        (void)processInfo;
        (void)deviceId;
        (void)pinType;
        (void)payload;
    }

    void OnPinHolderDestroy(const ProcessInfo &processInfo, DmPinType pinType, const std::string &payload) override
    {
        (void)processInfo;
        (void)pinType;
        (void)payload;
    }

    void OnCreateResult(const ProcessInfo &processInfo, int32_t result) override
    {
        (void)processInfo;
        (void)result;
    }

    void OnDestroyResult(const ProcessInfo &processInfo, int32_t result) override
    {
        (void)processInfo;
        (void)result;
    }

    void OnPinHolderEvent(const ProcessInfo &processInfo, DmPinHolderEvent event, int32_t result,
        const std::string &content) override
    {
        (void)processInfo;
        (void)event;
        (void)result;
        (void)content;
    }

    void OnDeviceTrustChange(const std::string &udid, const std::string &uuid, DmAuthForm authForm) override
    {
        (void)udid;
        (void)uuid;
        (void)authForm;
    }

    void OnDeviceScreenStateChange(const ProcessInfo &processInfo, DmDeviceInfo &devInfo) override
    {
        (void)processInfo;
        (void)devInfo;
    }

    void OnCredentialAuthStatus(const ProcessInfo &processInfo, const std::string &deviceList, uint16_t deviceTypeId,
                                int32_t errcode) override
    {
        (void)processInfo;
        (void)deviceList;
        (void)deviceTypeId;
        (void)errcode;
    }

    void OnAppUnintall(const std::string &pkgName) override
    {
        (void)pkgName;
    }

    void OnSinkBindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId, int32_t result,
        int32_t status, std::string content) override
    {
        (void)processInfo;
        (void)targetId;
        (void)result;
        (void)status;
        (void)content;
    }
    void OnProcessRemove(const ProcessInfo &processInfo) override
    {
        (void)processInfo;
    }

    void OnDevStateCallbackAdd(const ProcessInfo &processInfo,
        const std::vector<DmDeviceInfo> &deviceList) override
    {
        (void)processInfo;
        (void)deviceList;
    }

    void OnGetDeviceProfileInfoListResult(const ProcessInfo &processInfo,
        const std::vector<DmDeviceProfileInfo> &deviceProfileInfos, int32_t code) override
    {
        (void)processInfo;
        (void)deviceProfileInfos;
        (void)code;
    }

    void OnGetDeviceIconInfoResult(const ProcessInfo &processInfo,
        const DmDeviceIconInfo &dmDeviceIconInfo, int32_t code) override
    {
        (void)processInfo;
        (void)dmDeviceIconInfo;
        (void)code;
    }

    void OnSetLocalDeviceNameResult(const ProcessInfo &processInfo,
        const std::string &deviceName, int32_t code) override
    {
        (void)processInfo;
        (void)deviceName;
        (void)code;
    }

    void OnSetRemoteDeviceNameResult(const ProcessInfo &processInfo, const std::string &deviceId,
        const std::string &deviceName, int32_t code) override
    {
        (void)processInfo;
        (void)deviceId;
        (void)deviceName;
        (void)code;
    }

    virtual std::string GetLocalDisplayDeviceName()
    {
        return "";
    }

    virtual void SetExistPkgName(const std::set<std::string> &pkgNameSet)
    {
        (void)pkgNameSet;
    }

    virtual int32_t OpenAuthSessionWithPara(const std::string &deviceId, int32_t actionId, bool isEnable160m)
    {
        (void)deviceId;
        (void)actionId;
        (void)isEnable160m;
        return 0;
    }
    virtual void OnServiceFound(const ProcessInfo &processInfo, int32_t discServiceId,
        const DiscoveryServiceInfo &discServiceInfo)
    {
        (void)processInfo;
        (void)discServiceId;
        (void)discServiceInfo;
    }

    virtual void OnServiceDiscoveryResult(const ProcessInfo &processInfo, int32_t discServiceId, int32_t reason)
    {
        (void)processInfo;
        (void)discServiceId;
        (void)reason;
    }

    virtual int32_t OpenAuthSessionWithPara(int64_t serviceId)
    {
        (void)serviceId;
        return 0;
    }

    virtual void OnDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
        const DmDeviceInfo &info, const std::vector<int64_t> &serviceIds)
    {
        (void)processInfo;
        (void)state;
        (void)info;
        (void)serviceIds;
    }

    virtual void OnServicePublishResult(const ProcessInfo &processInfo, int64_t serviceId, int32_t publishResult)
    {
        (void)processInfo;
        (void)serviceId;
        (void)publishResult;
    }
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_DM_PIN_HOLDER_TEST_H
