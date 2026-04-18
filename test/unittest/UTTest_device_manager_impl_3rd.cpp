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

PeerTargetId3rd MakePeerTarget()
{
    PeerTargetId3rd target;
    target.deviceId = "device-id";
    target.brMac = "11:22:33:44:55:66";
    target.bleMac = "66:55:44:33:22:11";
    target.wifiIp = "192.168.1.100";
    target.wifiPort = 4567;
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
    item.sessionKeyId = 7;
    item.createTime = 123456789LL;
    item.userId = 1001;
    item.extra = "extra-info";
    item.bindLevel = 3;
    item.bindType = 2;
    item.sessionKey = TEST_SESSION_KEY;
    return item;
}
} // namespace

DmInit3rdCallbackTest::DmInit3rdCallbackTest(int &count) : count_(&count) {}

void DmInit3rdCallbackTest::OnRemoteDied()
{
    if (count_ != nullptr) {
        ++(*count_);
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
            lastBusinessName_ = data.ReadString();
            lastListener_ = data.ReadRemoteObject();
            reply.WriteInt32(replyCode_);
            return DM_OK;
        case IMPORT_PINCODE_3RD:
            lastBusinessName_ = data.ReadString();
            lastPinCode_ = data.ReadString();
            reply.WriteInt32(replyCode_);
            return DM_OK;
        case GENERATE_PINCODE_3RD:
            lastPinLength_ = data.ReadUint32();
            reply.WriteInt32(replyCode_);
            if (replyCode_ == DM_OK) {
                reply.WriteString(generatedPin_);
            }
            return DM_OK;
        case AUTH_PINCODE_3RD:
            ReadPeerTarget(data, lastPeerTarget_);
            lastAuthParamStr_ = data.ReadString();
            reply.WriteInt32(replyCode_);
            return DM_OK;
        case AUTH_DEVICE_3RD:
            ReadPeerTarget(data, lastPeerTarget_);
            lastAuthParamStr_ = data.ReadString();
            reply.WriteInt32(replyCode_);
            return DM_OK;
        case QUERY_TRUST_RELATION_3RD:
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
        case DELETE_TRUST_RELATION_3RD:
            lastBusinessName_ = data.ReadString();
            lastPeerDeviceId_ = data.ReadString();
            lastUnbindParamStr_ = data.ReadString();
            reply.WriteInt32(replyCode_);
            return DM_OK;
        default:
            return DM_OK;
    }
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

HWTEST_F(DeviceManagerImpl3rdTest, OnRemoveSystemAbility_001, testing::ext::TestSize.Level0)
{
    int count = 0;
    auto callback = std::make_shared<DmInit3rdCallbackTest>(count);
    DeviceManagerNotify3rd::GetInstance().RegisterDeathRecipientCallback(TEST_BUSINESS_NAME, callback);

    DeviceManagerImpl3rd::SystemAbilityListener3rd listener;
    listener.OnAddSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, "device");
    listener.OnRemoveSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, "device");
    ASSERT_EQ(count, 1);

    listener.OnRemoveSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID + 1, "device");
    ASSERT_EQ(count, 1);
}

HWTEST_F(DeviceManagerImpl3rdTest, SubscribeDMSAChangeListener_001, testing::ext::TestSize.Level0)
{
    int count = 0;
    auto callback = std::make_shared<DmInit3rdCallbackTest>(count);
    DeviceManagerNotify3rd::GetInstance().RegisterDeathRecipientCallback(TEST_BUSINESS_NAME, callback);

    auto samgr = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    EXPECT_CALL(*client_, GetSystemAbilityManager()).Times(1).WillOnce(Return(samgr));
    EXPECT_CALL(*samgr, SubscribeSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, _))
        .Times(1).WillOnce(Return(DM_OK));

    auto &impl = DeviceManagerImpl3rd::GetInstance();
    impl.SubscribeDMSAChangeListener();

    ASSERT_NE(impl.saListenerCallback_, nullptr);
    ASSERT_TRUE(impl.isSubscribeDMSAChangeListener_);

    impl.saListenerCallback_->OnAddSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, "device");
    impl.saListenerCallback_->OnRemoveSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, "device");
    ASSERT_EQ(count, 1);
}

HWTEST_F(DeviceManagerImpl3rdTest, RegisterAuthCallback_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmAuthCallback> callback = nullptr;
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().RegisterAuthCallback("", callback), ERR_DM_INPUT_PARA_INVALID);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().RegisterAuthCallback(TEST_BUSINESS_NAME, callback),
        ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImpl3rdTest, UnRegisterAuthCallback_001, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().UnRegisterAuthCallback(""), ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImpl3rdTest, InitDeviceManager_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmInit3rdCallback> callback = nullptr;
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().InitDeviceManager("", callback), ERR_DM_INPUT_PARA_INVALID);

    int callbackCount = 0;
    callback = std::make_shared<DmInit3rdCallbackTest>(callbackCount);
    auto samgr = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(DM_OK);
    EXPECT_CALL(*client_, GetSystemAbilityManager()).Times(1).WillOnce(Return(samgr));
    EXPECT_CALL(*samgr, SubscribeSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, _))
        .Times(1).WillOnce(Return(DM_OK));

    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().InitDeviceManager(TEST_BUSINESS_NAME, callback), DM_OK);
    ASSERT_TRUE(DeviceManagerImpl3rd::GetInstance().isSubscribeDMSAChangeListener_);
    ASSERT_EQ(remoteStub_->lastRequestCode_, INIT_DEVICE_MANAGER);
    ASSERT_EQ(remoteStub_->lastBusinessName_, TEST_BUSINESS_NAME);
    ASSERT_NE(remoteStub_->lastListener_, nullptr);
    auto callbacks = DeviceManagerNotify3rd::GetInstance().GetDmInit3rdCallback();
    auto iter = callbacks.find(TEST_BUSINESS_NAME);
    ASSERT_NE(iter, callbacks.end());
    ASSERT_EQ(iter->second, callback);
}

HWTEST_F(DeviceManagerImpl3rdTest, InitDeviceManager_002, testing::ext::TestSize.Level0)
{
    int callbackCount = 0;
    auto callback = std::make_shared<DmInit3rdCallbackTest>(callbackCount);
    auto samgr = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetFailSend(true);

    EXPECT_CALL(*client_, GetSystemAbilityManager()).Times(1).WillOnce(Return(samgr));
    EXPECT_CALL(*samgr, SubscribeSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, _))
        .Times(1).WillOnce(Return(DM_OK));

    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().InitDeviceManager(TEST_BUSINESS_NAME, callback),
        ERR_DM_IPC_SEND_REQUEST_FAILED);
    ASSERT_TRUE(DeviceManagerImpl3rd::GetInstance().isSubscribeDMSAChangeListener_);
    ASSERT_TRUE(DeviceManagerNotify3rd::GetInstance().GetDmInit3rdCallback().empty());
}

HWTEST_F(DeviceManagerImpl3rdTest, InitDeviceManager_003, testing::ext::TestSize.Level0)
{
    int callbackCount = 0;
    auto callback = std::make_shared<DmInit3rdCallbackTest>(callbackCount);
    auto samgr = sptr<SystemAbilityManagerMock>(new (std::nothrow) SystemAbilityManagerMock());

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(ERR_DM_FAILED);

    EXPECT_CALL(*client_, GetSystemAbilityManager()).Times(1).WillOnce(Return(samgr));
    EXPECT_CALL(*samgr, SubscribeSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, _))
        .Times(1).WillOnce(Return(DM_OK));

    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().InitDeviceManager(TEST_BUSINESS_NAME, callback), ERR_DM_FAILED);
    ASSERT_TRUE(DeviceManagerNotify3rd::GetInstance().GetDmInit3rdCallback().empty());
}

HWTEST_F(DeviceManagerImpl3rdTest, ImportPinCode3rd_001, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().ImportPinCode3rd("", ""), ERR_DM_INPUT_PARA_INVALID);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().ImportPinCode3rd(TEST_BUSINESS_NAME, ""), ERR_DM_INPUT_PARA_INVALID);

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(DM_OK);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().ImportPinCode3rd(TEST_BUSINESS_NAME, TEST_PIN_CODE), DM_OK);
    ASSERT_EQ(remoteStub_->lastRequestCode_, IMPORT_PINCODE_3RD);
    ASSERT_EQ(remoteStub_->lastBusinessName_, TEST_BUSINESS_NAME);
    ASSERT_EQ(remoteStub_->lastPinCode_, TEST_PIN_CODE);
}

HWTEST_F(DeviceManagerImpl3rdTest, ImportPinCode3rd_002, testing::ext::TestSize.Level0)
{
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetFailSend(true);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().ImportPinCode3rd(TEST_BUSINESS_NAME, TEST_PIN_CODE),
        ERR_DM_IPC_SEND_REQUEST_FAILED);

    remoteStub_->Reset();
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(ERR_DM_FAILED);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().ImportPinCode3rd(TEST_BUSINESS_NAME, TEST_PIN_CODE),
        ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImpl3rdTest, GeneratePinCode_001, testing::ext::TestSize.Level0)
{
    std::string pincode;
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().GeneratePinCode(INVALID_PIN_LENGTH_LOW, pincode),
        ERR_DM_INPUT_PARA_INVALID);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().GeneratePinCode(INVALID_PIN_LENGTH_HIGH, pincode),
        ERR_DM_INPUT_PARA_INVALID);

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(DM_OK);
    remoteStub_->SetGeneratedPin(TEST_GENERATED_PIN);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().GeneratePinCode(VALID_PIN_LENGTH, pincode), DM_OK);
    ASSERT_EQ(pincode, TEST_GENERATED_PIN);
    ASSERT_EQ(remoteStub_->lastRequestCode_, GENERATE_PINCODE_3RD);
    ASSERT_EQ(remoteStub_->lastPinLength_, VALID_PIN_LENGTH);
}

HWTEST_F(DeviceManagerImpl3rdTest, GeneratePinCode_002, testing::ext::TestSize.Level0)
{
    std::string pincode;

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetFailSend(true);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().GeneratePinCode(VALID_PIN_LENGTH, pincode),
        ERR_DM_IPC_SEND_REQUEST_FAILED);

    remoteStub_->Reset();
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(ERR_DM_FAILED);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().GeneratePinCode(VALID_PIN_LENGTH, pincode), ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImpl3rdTest, AuthPincode_001, testing::ext::TestSize.Level0)
{
    PeerTargetId3rd targetId;
    std::map<std::string, std::string> authParam;
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().AuthPincode(targetId, authParam), ERR_DM_INPUT_PARA_INVALID);

    targetId = MakePeerTarget();
    authParam = MakeAuthParam();
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(DM_OK);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().AuthPincode(targetId, authParam), DM_OK);
    ASSERT_EQ(remoteStub_->lastRequestCode_, AUTH_PINCODE_3RD);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.deviceId, targetId.deviceId);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.brMac, targetId.brMac);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.bleMac, targetId.bleMac);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.wifiIp, targetId.wifiIp);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.wifiPort, targetId.wifiPort);
    ASSERT_FALSE(remoteStub_->lastAuthParamStr_.empty());
}

HWTEST_F(DeviceManagerImpl3rdTest, AuthPincode_002, testing::ext::TestSize.Level0)
{
    auto targetId = MakePeerTarget();
    auto authParam = MakeAuthParam();

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetFailSend(true);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().AuthPincode(targetId, authParam),
        ERR_DM_IPC_SEND_REQUEST_FAILED);

    remoteStub_->Reset();
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(ERR_DM_FAILED);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().AuthPincode(targetId, authParam), ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImpl3rdTest, AuthDevice3rd_001, testing::ext::TestSize.Level0)
{
    PeerTargetId3rd targetId;
    std::map<std::string, std::string> authParam;
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().AuthDevice3rd(targetId, authParam), ERR_DM_INPUT_PARA_INVALID);

    targetId = MakePeerTarget();
    authParam = MakeAuthParam();
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(DM_OK);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().AuthDevice3rd(targetId, authParam), DM_OK);
    ASSERT_EQ(remoteStub_->lastRequestCode_, AUTH_DEVICE_3RD);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.deviceId, targetId.deviceId);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.brMac, targetId.brMac);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.bleMac, targetId.bleMac);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.wifiIp, targetId.wifiIp);
    ASSERT_EQ(remoteStub_->lastPeerTarget_.wifiPort, targetId.wifiPort);
    ASSERT_FALSE(remoteStub_->lastAuthParamStr_.empty());
}

HWTEST_F(DeviceManagerImpl3rdTest, AuthDevice3rd_002, testing::ext::TestSize.Level0)
{
    auto targetId = MakePeerTarget();
    auto authParam = MakeAuthParam();

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetFailSend(true);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().AuthDevice3rd(targetId, authParam),
        ERR_DM_IPC_SEND_REQUEST_FAILED);

    remoteStub_->Reset();
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(ERR_DM_FAILED);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().AuthDevice3rd(targetId, authParam), ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImpl3rdTest, QueryTrustRelation_001, testing::ext::TestSize.Level0)
{
    std::vector<TrustDeviceInfo3rd> trustedDeviceList;
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().QueryTrustRelation("", trustedDeviceList),
        ERR_DM_INPUT_PARA_INVALID);

    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().QueryTrustRelation(TEST_BUSINESS_NAME, trustedDeviceList),
        ERR_DM_POINT_NULL);

    auto trustItem = MakeTrustReplyItem();
    remoteStub_->SetReplyCode(DM_OK);
    remoteStub_->SetTrustDevices({ trustItem });
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;

    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().QueryTrustRelation(TEST_BUSINESS_NAME, trustedDeviceList), DM_OK);
    ASSERT_EQ(remoteStub_->lastRequestCode_, QUERY_TRUST_RELATION_3RD);
    ASSERT_EQ(remoteStub_->lastBusinessName_, TEST_BUSINESS_NAME);
    ASSERT_EQ(trustedDeviceList.size(), 1u);
    ASSERT_EQ(trustedDeviceList[0].trustDeviceId, trustItem.trustDeviceId);
    ASSERT_EQ(trustedDeviceList[0].sessionKeyId, trustItem.sessionKeyId);
    ASSERT_EQ(trustedDeviceList[0].createTime, trustItem.createTime);
    ASSERT_EQ(trustedDeviceList[0].userId, trustItem.userId);
    ASSERT_EQ(trustedDeviceList[0].extra, trustItem.extra);
    ASSERT_EQ(trustedDeviceList[0].bindLevel, trustItem.bindLevel);
    ASSERT_EQ(trustedDeviceList[0].bindType, trustItem.bindType);
    ASSERT_EQ(trustedDeviceList[0].sessionKey.keyLen, trustItem.sessionKey.length());
    ASSERT_EQ(std::string(reinterpret_cast<char *>(trustedDeviceList[0].sessionKey.key),
        trustedDeviceList[0].sessionKey.keyLen), trustItem.sessionKey);
    free(trustedDeviceList[0].sessionKey.key);
    trustedDeviceList[0].sessionKey.key = nullptr;
    trustedDeviceList[0].sessionKey.keyLen = 0;
}

HWTEST_F(DeviceManagerImpl3rdTest, QueryTrustRelation_002, testing::ext::TestSize.Level0)
{
    std::vector<TrustDeviceInfo3rd> trustedDeviceList;

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetFailSend(true);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().QueryTrustRelation(TEST_BUSINESS_NAME, trustedDeviceList),
        ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImpl3rdTest, QueryTrustRelation_003, testing::ext::TestSize.Level0)
{
    std::vector<TrustDeviceInfo3rd> trustedDeviceList;

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(ERR_DM_FAILED);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().QueryTrustRelation(TEST_BUSINESS_NAME, trustedDeviceList),
        ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImpl3rdTest, QueryTrustRelation_004, testing::ext::TestSize.Level0)
{
    std::vector<TrustDeviceInfo3rd> trustedDeviceList;

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(DM_OK);
    remoteStub_->SetCustomTrustDeviceListSize(INVALID_TRUST_DEVICE_LIST_SIZE);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().QueryTrustRelation(TEST_BUSINESS_NAME, trustedDeviceList),
        ERR_DM_IPC_READ_FAILED);
}

HWTEST_F(DeviceManagerImpl3rdTest, QueryTrustRelation_005, testing::ext::TestSize.Level0)
{
    std::vector<TrustDeviceInfo3rd> trustedDeviceList;
    auto trustItem = MakeTrustReplyItem();
    trustItem.sessionKey.assign(TOO_LONG_SESSION_KEY, 'a');

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(DM_OK);
    remoteStub_->SetTrustDevices({ trustItem });
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().QueryTrustRelation(TEST_BUSINESS_NAME, trustedDeviceList), DM_OK);
    ASSERT_EQ(trustedDeviceList.size(), 1u);
    ASSERT_EQ(trustedDeviceList[0].sessionKey.key, nullptr);
    ASSERT_EQ(trustedDeviceList[0].sessionKey.keyLen, 0u);
}

HWTEST_F(DeviceManagerImpl3rdTest, DeleteTrustRelation_001, testing::ext::TestSize.Level0)
{
    std::map<std::string, std::string> unbindParam;
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().DeleteTrustRelation("", "", unbindParam),
        ERR_DM_INPUT_PARA_INVALID);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().DeleteTrustRelation(TEST_BUSINESS_NAME, "", unbindParam),
        ERR_DM_INPUT_PARA_INVALID);

    unbindParam = MakeAuthParam();
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(DM_OK);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().DeleteTrustRelation(TEST_BUSINESS_NAME, TEST_PEER_DEVICE_ID,
        unbindParam), DM_OK);
    ASSERT_EQ(remoteStub_->lastRequestCode_, DELETE_TRUST_RELATION_3RD);
    ASSERT_EQ(remoteStub_->lastBusinessName_, TEST_BUSINESS_NAME);
    ASSERT_EQ(remoteStub_->lastPeerDeviceId_, TEST_PEER_DEVICE_ID);
    ASSERT_FALSE(remoteStub_->lastUnbindParamStr_.empty());
}

HWTEST_F(DeviceManagerImpl3rdTest, DeleteTrustRelation_002, testing::ext::TestSize.Level0)
{
    auto unbindParam = MakeAuthParam();

    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetFailSend(true);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().DeleteTrustRelation(TEST_BUSINESS_NAME, TEST_PEER_DEVICE_ID,
        unbindParam), ERR_DM_IPC_SEND_REQUEST_FAILED);

    remoteStub_->Reset();
    DeviceManagerImpl3rd::GetInstance().dm3rdremoteObject_ = remoteStub_;
    remoteStub_->SetReplyCode(ERR_DM_FAILED);
    ASSERT_EQ(DeviceManagerImpl3rd::GetInstance().DeleteTrustRelation(TEST_BUSINESS_NAME, TEST_PEER_DEVICE_ID,
        unbindParam), ERR_DM_FAILED);
}
} // namespace DistributedHardware
} // namespace OHOS
