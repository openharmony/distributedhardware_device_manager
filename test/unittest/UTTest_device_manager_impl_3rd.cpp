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

#include "UTTest_device_manager_impl_3rd.h"

#include "gmock/gmock.h"

#include <cstdlib>
#include <cstring>

#include "dm_error_type_3rd.h"
#include "dm_service_load_3rd.h"
#include "dm_system_ability_manager_mock.h"
#include "system_ability_definition.h"

using ::testing::_;
using ::testing::An;
using ::testing::Return;

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr const char *TEST_BUSINESS_NAME = "com.ohos.test.3rd";
constexpr const char *TEST_PIN_CODE = "12345678";
constexpr const char *TEST_GENERATED_PIN = "24681012";
constexpr const char *TEST_PEER_DEVICE_ID = "peer-device-id";
constexpr const char *TEST_TRUST_DEVICE_ID = "trust-device-id";
constexpr const char *TEST_SESSION_KEY = "trust-key";
constexpr int32_t INVALID_TRUST_DEVICE_LIST_SIZE = 501;
constexpr size_t INVALID_PIN_LENGTH_LOW = 5;
constexpr size_t INVALID_PIN_LENGTH_HIGH = 1025;
constexpr size_t VALID_PIN_LENGTH = 8;
constexpr size_t TOO_LONG_SESSION_KEY = 513;
constexpr uint16_t TEST_WIFI_PORT = 4567;
constexpr int32_t TEST_SESSION_KEY_ID = 7;
constexpr int32_t TEST_USER_ID = 100;
constexpr int32_t TEST_BIND_LEVEL = 3;
constexpr int32_t TEST_BIND_TYPE = 2;
constexpr int64_t TEST_CREATE_TIME = 123456789LL;

PeerTargetId3rd MakePeerTarget()
{
    PeerTargetId3rd target;
    target.deviceId = "device-id";
    target.brMac = "11:22:33:44:55:66";
    target.bleMac = "66:55:44:33:22:11";
    target.wifiIp = "192.168.1.100";
    target.wifiPort = TEST_WIFI_PORT;
    return target;
}

std::map<std::string, std::string> MakeAuthParam()
{
    return { { "authKey", "authValue" } };
}

TrustDeviceReplyItem MakeTrustReplyItem()
{
    TrustDeviceReplyItem item;
    item.trustDeviceId = TEST_TRUST_DEVICE_ID;
    item.sessionKeyId = TEST_SESSION_KEY_ID;
    item.createTime = TEST_CREATE_TIME;
    item.userId = TEST_USER_ID;
    item.extra = "extra-info";
    item.bindLevel = TEST_BIND_LEVEL;
    item.bindType = TEST_BIND_TYPE;
    item.sessionKey = TEST_SESSION_KEY;
    return item;
}
} // namespace

DmInit3rdCallbackTest::DmInit3rdCallbackTest(int &count) : count_(&count) {}

void DmInit3rdCallbackTest::OnRemoteDied()
{
    if (count_ != nullptr) {
        (*count_)++;
    }
}

DmAuthCallbackTest::DmAuthCallbackTest(int &count) : count_(&count) {}

void DmAuthCallbackTest::OnAuthResult(const ProcessInfo3rd &processInfo3rd, int32_t result, int32_t status,
    const std::string &authContent)
{
    (void)processInfo3rd;
    (void)result;
    (void)status;
    (void)authContent;
    if (count_ != nullptr) {
        (*count_)++;
    }
}

void DmAuthCallbackTest::OnAuthResult(const ProcessInfo3rd &processInfo3rd, int32_t result, int32_t status,
    std::vector<TrustDeviceInfo3rd> &deviceInfos, const std::string &authContent)
{
    (void)processInfo3rd;
    (void)result;
    (void)status;
    (void)deviceInfos;
    (void)authContent;
    if (count_ != nullptr) {
        (*count_)++;
    }
}

void MockRemoteStub3rd::Reset()
{
    failSend_ = false;
    replyCode_ = DM_OK;
    generatedPin_.assign(TEST_GENERATED_PIN);
    trustDevices_.clear();
    useCustomTrustDeviceListSize_ = false;
    customTrustDeviceListSize_ = 0;
    lastRequestCode_ = 0;
    lastBusinessName_.clear();
    lastPinCode_.clear();
    lastAuthParamStr_.clear();
    lastUnbindParamStr_.clear();
    lastPeerDeviceId_.clear();
    lastListener_ = nullptr;
    lastPeerTarget_ = {};
    lastPinLength_ = 0;
}

void MockRemoteStub3rd::SetFailSend(bool failSend)
{
    failSend_ = failSend;
}

void MockRemoteStub3rd::SetReplyCode(int32_t replyCode)
{
    replyCode_ = replyCode;
}

void MockRemoteStub3rd::SetGeneratedPin(const std::string &generatedPin)
{
    generatedPin_ = generatedPin;
}

void MockRemoteStub3rd::SetTrustDevices(const std::vector<TrustDeviceReplyItem> &trustDevices)
{
    trustDevices_ = trustDevices;
}

void MockRemoteStub3rd::SetCustomTrustDeviceListSize(int32_t listSize)
{
    useCustomTrustDeviceListSize_ = true;
    customTrustDeviceListSize_ = listSize;
}

int32_t MockRemoteStub3rd::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    (void)option;
    (void)data.ReadInterfaceToken();
    if (failSend_) {
        return ERR_DM_FAILED;
    }

    lastRequestCode_ = code;
    switch (code) {
        case INIT_DEVICE_MANAGER:
            return HandleInitDeviceManager(data, reply);
        case IMPORT_PINCODE_3RD:
            return HandleImportPinCode(data, reply);
        case GENERATE_PINCODE_3RD:
            return HandleGeneratePinCode(data, reply);
        case AUTH_PINCODE_3RD:
            return HandleAuthPincode(data, reply);
        case AUTH_DEVICE_3RD:
            return HandleAuthDevice(data, reply);
        case QUERY_TRUST_RELATION_3RD:
            return HandleQueryTrustRelation(data, reply);
        case DELETE_TRUST_RELATION_3RD:
            return HandleDeleteTrustRelation(data, reply);
        case AUTH_CREDENTIAL_3RD:
            return HandleAuthCredential(data, reply);
        default:
            return DM_OK;
    }
}

int32_t MockRemoteStub3rd::HandleInitDeviceManager(MessageParcel &data, MessageParcel &reply)
{
    lastBusinessName_ = data.ReadString();
    lastListener_ = data.ReadRemoteObject();
    reply.WriteInt32(replyCode_);
    return DM_OK;
}

int32_t MockRemoteStub3rd::HandleImportPinCode(MessageParcel &data, MessageParcel &reply)
{
    lastBusinessName_ = data.ReadString();
    lastPinCode_ = data.ReadString();
    reply.WriteInt32(replyCode_);
    return DM_OK;
}

int32_t MockRemoteStub3rd::HandleGeneratePinCode(MessageParcel &data, MessageParcel &reply)
{
    lastPinLength_ = data.ReadUint32();
    reply.WriteInt32(replyCode_);
    if (replyCode_ == DM_OK) {
        reply.WriteString(generatedPin_);
    }
    return DM_OK;
}

int32_t MockRemoteStub3rd::HandleAuthPincode(MessageParcel &data, MessageParcel &reply)
{
    ReadPeerTarget(data, lastPeerTarget_);
    lastAuthParamStr_ = data.ReadString();
    reply.WriteInt32(replyCode_);
    return DM_OK;
}

int32_t MockRemoteStub3rd::HandleAuthDevice(MessageParcel &data, MessageParcel &reply)
{
    ReadPeerTarget(data, lastPeerTarget_);
    lastAuthParamStr_ = data.ReadString();
    reply.WriteInt32(replyCode_);
    return DM_OK;
}

int32_t MockRemoteStub3rd::HandleQueryTrustRelation(MessageParcel &data, MessageParcel &reply)
{
    lastBusinessName_ = data.ReadString();
    reply.WriteInt32(replyCode_);
    if (replyCode_ != DM_OK) {
        return DM_OK;
    }
    reply.WriteInt32(useCustomTrustDeviceListSize_ ? customTrustDeviceListSize_ :
        static_cast<int32_t>(trustDevices_.size()));
    for (const auto &item : trustDevices_) {
        if (!reply.WriteString(item.trustDeviceId) ||
            !reply.WriteInt32(item.sessionKeyId) ||
            !reply.WriteInt64(item.createTime) ||
            !reply.WriteInt32(item.userId) ||
            !reply.WriteString(item.extra) ||
            !reply.WriteInt32(item.bindLevel) ||
            !reply.WriteInt32(item.bindType) ||
            !reply.WriteString(item.sessionKey)) {
            return ERR_DM_IPC_WRITE_FAILED;
        }
    }
    return DM_OK;
}

int32_t MockRemoteStub3rd::HandleDeleteTrustRelation(MessageParcel &data, MessageParcel &reply)
{
    lastBusinessName_ = data.ReadString();
    lastPeerDeviceId_ = data.ReadString();
    lastUnbindParamStr_ = data.ReadString();
    reply.WriteInt32(replyCode_);
    return DM_OK;
}

int32_t MockRemoteStub3rd::HandleAuthCredential(MessageParcel &data, MessageParcel &reply)
{
    ReadPeerTarget(data, lastPeerTarget_);
    lastAuthParamStr_ = data.ReadString();
    reply.WriteInt32(replyCode_);
    return DM_OK;
}

void MockRemoteStub3rd::ReadPeerTarget(MessageParcel &data, PeerTargetId3rd &target)
{
    target.deviceId = data.ReadString();
    target.brMac = data.ReadString();
    target.bleMac = data.ReadString();
    target.wifiIp = data.ReadString();
    target.wifiPort = data.ReadUint16();
}

void DeviceManagerImpl3rdTest::SetUp()
{
    client_ = std::static_pointer_cast<SystemAbilityManagerClientMock>(
        ISystemAbilityManagerClient::GetOrCreateSAMgrClient());
    remoteStub_ = sptr<MockRemoteStub3rd>(new (std::nothrow) MockRemoteStub3rd());
    ResetState();
}

void DeviceManagerImpl3rdTest::TearDown()
{
    ResetState();
    DmServiceLoad3rd::GetInstance().SetLoadFinish();
    ISystemAbilityManagerClient::ReleaseSAMgrClient();
    client_ = nullptr;
    remoteStub_ = nullptr;
}

void DeviceManagerImpl3rdTest::ResetState()
{
    auto &impl = DeviceManagerImpl3rd::GetInstance();
    impl.dm3rdremoteObject_ = nullptr;
    impl.saListenerCallback_ = nullptr;
    impl.isSubscribeDMSAChangeListener_.store(false);
    DeviceManagerNotify3rd::GetInstance().dmInit3rdCallbackMap_.clear();
    DeviceManagerNotify3rd::GetInstance().dmAuthCallbackMap_.clear();
    DmServiceLoad3rd::GetInstance().SetLoadFinish();
    if (remoteStub_ != nullptr) {
        remoteStub_->Reset();
    }
}

HWTEST_F(DeviceManagerImpl3rdTest, DmInit3rdCallback_OnRemoteDied_001, testing::ext::TestSize.Level0)
{
    int count = 0;
    auto callback = std::make_shared<DmInit3rdCallbackTest>(count);
    ASSERT_NE(callback, nullptr);
    callback->OnRemoteDied();
    ASSERT_EQ(count, 1);
}

HWTEST_F(DeviceManagerImpl3rdTest, DmAuthCallback_OnAuthResult_001, testing::ext::TestSize.Level0)
{
    int count = 0;
    auto callback = std::make_shared<DmAuthCallbackTest>(count);
    ASSERT_NE(callback, nullptr);

    ProcessInfo3rd processInfo;
    processInfo.tokenId = 123;
    processInfo.uid = 1000;
    processInfo.processName = "test_process";
    processInfo.businessName = "test_business";
    processInfo.userId = 100;

    int32_t result = 0;
    int32_t status = 1;
    std::string authContent = "test_auth_content";

    callback->OnAuthResult(processInfo, result, status, authContent);
    ASSERT_EQ(count, 1);
}

HWTEST_F(DeviceManagerImpl3rdTest, DmAuthCallback_OnAuthResult_002, testing::ext::TestSize.Level0)
{
    int count = 0;
    auto callback = std::make_shared<DmAuthCallbackTest>(count);
    ASSERT_NE(callback, nullptr);

    ProcessInfo3rd processInfo;
    processInfo.tokenId = 456;
    processInfo.uid = 2000;
    processInfo.processName = "test_process2";
    processInfo.businessName = "test_business2";
    processInfo.userId = 200;

    int32_t result = 0;
    int32_t status = 1;
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    TrustDeviceInfo3rd deviceInfo;
    deviceInfo.trustDeviceId = "device123";
    deviceInfo.sessionKeyId = 1;
    deviceInfo.createTime = 123456789LL;
    deviceInfo.userId = 100;
    deviceInfo.extra = "extra_info";
    deviceInfo.bindLevel = 2;
    deviceInfo.bindType = 1;
    deviceInfos.push_back(deviceInfo);
    std::string authContent = "test_auth_content2";

    callback->OnAuthResult(processInfo, result, status, deviceInfos, authContent);
    ASSERT_EQ(count, 1);
}

HWTEST_F(DeviceManagerImpl3rdTest, DmAuthCallback_OnAuthResult_003, testing::ext::TestSize.Level0)
{
    int count = 0;
    auto callback = std::make_shared<DmAuthCallbackTest>(count);
    ASSERT_NE(callback, nullptr);

    ProcessInfo3rd processInfo;
    int32_t result = -1;
    int32_t status = 0;
    std::string authContent;

    callback->OnAuthResult(processInfo, result, status, authContent);
    ASSERT_EQ(count, 1);

    std::vector<TrustDeviceInfo3rd> deviceInfos;
    callback->OnAuthResult(processInfo, result, status, deviceInfos, authContent);
    ASSERT_EQ(count, 2);
}

HWTEST_F(DeviceManagerImpl3rdTest, DmAuthCallback_OnAuthResult_004, testing::ext::TestSize.Level0)
{
    auto callback = std::make_shared<DmAuthCallback>();
    ASSERT_NE(callback, nullptr);

    ProcessInfo3rd processInfo;
    processInfo.tokenId = 123;
    processInfo.uid = 1000;
    processInfo.processName = "test_process";
    processInfo.businessName = "test_business";
    processInfo.userId = 100;

    int32_t result = 0;
    int32_t status = 1;
    std::string authContent = "test_auth_content";

    callback->OnAuthResult(processInfo, result, status, authContent);
    ASSERT_NE(callback, nullptr);
}

HWTEST_F(DeviceManagerImpl3rdTest, DmAuthCallback_OnAuthResult_005, testing::ext::TestSize.Level0)
{
    auto callback = std::make_shared<DmAuthCallback>();
    ASSERT_NE(callback, nullptr);

    ProcessInfo3rd processInfo;
    processInfo.tokenId = 456;
    processInfo.uid = 2000;
    processInfo.processName = "test_process2";
    processInfo.businessName = "test_business2";
    processInfo.userId = 200;

    int32_t result = 0;
    int32_t status = 1;
    std::vector<TrustDeviceInfo3rd> deviceInfos;
    TrustDeviceInfo3rd deviceInfo;
    deviceInfo.trustDeviceId = "device123";
    deviceInfo.sessionKeyId = 1;
    deviceInfo.createTime = 123456789LL;
    deviceInfo.userId = 100;
    deviceInfo.extra = "extra_info";
    deviceInfo.bindLevel = 2;
    deviceInfo.bindType = 1;
    deviceInfos.push_back(deviceInfo);
    std::string authContent = "test_auth_content2";

    callback->OnAuthResult(processInfo, result, status, deviceInfos, authContent);
    ASSERT_NE(callback, nullptr);
}
} // namespace DistributedHardware
} // namespace OHOS
