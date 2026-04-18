/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_UTTEST_DEVICE_MANAGER_IMPL_3RD_H
#define OHOS_UTTEST_DEVICE_MANAGER_IMPL_3RD_H

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "device_manager_impl_3rd.h"
#include "ipc_client_stub_3rd.h"
#include "message_option.h"
#include "message_parcel.h"
#include "system_ability_manager_client_mock.h"

namespace OHOS {
namespace DistributedHardware {
struct TrustDeviceReplyItem {
    std::string trustDeviceId;
    int32_t sessionKeyId = -1;
    int64_t createTime = 0;
    int32_t userId = -1;
    std::string extra;
    int32_t bindLevel = -1;
    int32_t bindType = -1;
    std::string sessionKey;
};

class DmInit3rdCallbackTest : public DmInit3rdCallback {
public:
    explicit DmInit3rdCallbackTest(int &count);
    ~DmInit3rdCallbackTest() override = default;
    void OnRemoteDied() override;

private:
    int *count_ = nullptr;
};

class MockRemoteStub3rd : public IpcClientStub3rd {
public:
    MockRemoteStub3rd() = default;
    ~MockRemoteStub3rd() override = default;

    void Reset();
    void SetFailSend(bool failSend);
    void SetReplyCode(int32_t replyCode);
    void SetGeneratedPin(const std::string &generatedPin);
    void SetTrustDevices(const std::vector<TrustDeviceReplyItem> &trustDevices);
    void SetCustomTrustDeviceListSize(int32_t listSize);
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;

    uint32_t lastRequestCode_ = 0;
    std::string lastBusinessName_;
    std::string lastPinCode_;
    std::string lastAuthParamStr_;
    std::string lastUnbindParamStr_;
    std::string lastPeerDeviceId_;
    sptr<IRemoteObject> lastListener_ = nullptr;
    PeerTargetId3rd lastPeerTarget_;
    uint32_t lastPinLength_ = 0;

private:
    static void ReadPeerTarget(MessageParcel &data, PeerTargetId3rd &target);

    bool failSend_ = false;
    int32_t replyCode_ = 0;
    std::string generatedPin_;
    std::vector<TrustDeviceReplyItem> trustDevices_;
    bool useCustomTrustDeviceListSize_ = false;
    int32_t customTrustDeviceListSize_ = 0;
};

class DeviceManagerImpl3rdTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

protected:
    void ResetState();

    std::shared_ptr<SystemAbilityManagerClientMock> client_;
    sptr<MockRemoteStub3rd> remoteStub_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_UTTEST_DEVICE_MANAGER_IMPL_3RD_H
