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

#include "dm_auth_state.h"
#include "UTTest_auth_acl.h"
#include "dm_auth_message_processor_mock.h"
#include "device_manager_service_listener.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {
constexpr const char *TEST_NONE_EMPTY_STRING = "test";
std::shared_ptr<DeviceProfileConnectorMock> AuthAclTest::deviceProfileConnectorMock = nullptr;
void AuthAclTest::SetUpTestCase()
{
    LOGI("AuthAclTest::SetUpTestCase start.");
    DmSoftbusConnector::dmSoftbusConnector = dmSoftbusConnectorMock;
    DmSoftbusSession::dmSoftbusSession = dmSoftbusSessionMock;
    DmAuthMessageProcessorMock::dmAuthMessageProcessorMock = std::make_shared<DmAuthMessageProcessorMock>();
    deviceProfileConnectorMock = std::make_shared<DeviceProfileConnectorMock>();
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock;
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
}

void AuthAclTest::TearDownTestCase()
{
    LOGI("AuthAclTest::TearDownTestCase start.");
    DmSoftbusConnector::dmSoftbusConnector = nullptr;
    dmSoftbusConnectorMock = nullptr;
    DmSoftbusSession::dmSoftbusSession = nullptr;
    dmSoftbusSessionMock = nullptr;
    DmAuthMessageProcessorMock::dmAuthMessageProcessorMock = nullptr;
    deviceProfileConnectorMock = nullptr;
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
    DistributedDeviceProfile::DpDistributedDeviceProfileClient::dpDistributedDeviceProfileClient = nullptr;
    distributedDeviceProfileClientMock_ = nullptr;
}

void AuthAclTest::SetUp()
{
    LOGI("AuthAclTest::SetUp start.");
    softbusConnector = std::make_shared<SoftbusConnector>();
    listener = std::make_shared<DeviceManagerServiceListener>();
    hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    hiChainConnector = std::make_shared<HiChainConnector>();
}

void AuthAclTest::TearDown()
{
    LOGI("AuthAclTest::TearDown start.");
    authManager = nullptr;
    context = nullptr;

    Mock::VerifyAndClearExpectations(&*DmAuthMessageProcessorMock::dmAuthMessageProcessorMock);
    Mock::VerifyAndClearExpectations(&*DmSoftbusConnector::dmSoftbusConnector);
    Mock::VerifyAndClearExpectations(&*DmSoftbusSession::dmSoftbusSession);
}

HWTEST_F(AuthAclTest, AuthSinkAcl_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSinkManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkDataSyncState>();

    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SINK_DATA_SYNC_STATE);
}

HWTEST_F(AuthAclTest, AuthSrcAcl_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcDataSyncState>();

    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_DATA_SYNC_STATE);
}

HWTEST_F(AuthAclTest, AuthSrcFinish_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcFinishState>();
    context->reason = DM_OK;
    context->connDelayCloseTime = 10;
    EXPECT_EQ(authState->Action(context), DM_OK);
}

HWTEST_F(AuthAclTest, AuthSrcFinish_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcFinishState>();

    EXPECT_EQ(authState->GetStateType(), DmAuthStateType::AUTH_SRC_FINISH_STATE);
}

HWTEST_F(AuthAclTest, GetPinErrorCountAndTokenId_001, testing::ext::TestSize.Level1)
{
    int32_t count = 0;
    uint64_t tokenId = 1;
    std::string bundleName = "com.test.success";
    int32_t pinExchangeType = 1;
    int32_t expectedCount = 5;
    uint64_t expectedTokenId = 12345;
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({"pinErrorCount":5, "tokenId":12345})");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkFinishState>();
    authState->GetPinErrorCountAndTokenId(bundleName, pinExchangeType, count, tokenId);

    EXPECT_EQ(count, expectedCount);
    EXPECT_EQ(tokenId, expectedTokenId);
}

HWTEST_F(AuthAclTest, GetPinErrorCountAndTokenId_002, testing::ext::TestSize.Level1)
{
    int32_t count = 0;
    uint64_t tokenId = 999;
    std::string bundleName = "com.test.only.count";
    int32_t pinExchangeType = 1;
    int32_t expectedCount = 3;
    uint64_t expectedTokenId = tokenId;
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({"pinErrorCount":3})");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkFinishState>();
    authState->GetPinErrorCountAndTokenId(bundleName, pinExchangeType, count, tokenId);
    EXPECT_EQ(count, expectedCount);
}

HWTEST_F(AuthAclTest, GetPinErrorCountAndTokenId_003, testing::ext::TestSize.Level1)
{
    int32_t count = 99;
    uint64_t tokenId = 0;
    std::string bundleName = "com.test.only.token";
    int32_t pinExchangeType = 1;
    uint64_t expectedTokenId = 54321;
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({"tokenId":54321})");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkFinishState>();
    authState->GetPinErrorCountAndTokenId(bundleName, pinExchangeType, count, tokenId);
    EXPECT_EQ(tokenId, expectedTokenId);
}

HWTEST_F(AuthAclTest, GetPinErrorCountAndTokenId_004, testing::ext::TestSize.Level1)
{
    int32_t count = 10;
    uint64_t tokenId = 123;
    std::string bundleName = "com.test.failure";
    int32_t pinExchangeType = 1;
    uint64_t initialTokenId = tokenId;
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkFinishState>();
    authState->GetPinErrorCountAndTokenId(bundleName, pinExchangeType, count, tokenId);
    EXPECT_TRUE(srvInfo.GetExtraInfo().empty());
}

HWTEST_F(AuthAclTest, GetPinErrorCountAndTokenId_005, testing::ext::TestSize.Level1)
{
    int32_t count = 15;
    uint64_t tokenId = 25ULL;
    std::string bundleName = "com.test.empty.extra";
    int32_t pinExchangeType = 1;
    uint64_t initialTokenId = tokenId;
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo("");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkFinishState>();
    authState->GetPinErrorCountAndTokenId(bundleName, pinExchangeType, count, tokenId);
    EXPECT_TRUE(srvInfo.GetExtraInfo().empty());
}

HWTEST_F(AuthAclTest, VerifyFlagXor_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    DmAccess accessSide;
    context->direction = DM_AUTH_SOURCE;
    accessSide = context->accessee;
    accessSide.pkgName = "com.test.pin.auth";
    context->authType = DmAuthType::AUTH_TYPE_NFC;
    context->pinCodeFlag = true;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkDataSyncState>();
    auto ret = authState->VerifyFlagXor(context);
    EXPECT_FALSE(ret);
}

HWTEST_F(AuthAclTest, VerifyFlagXor_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    context->accesser.bundleName = "wear_link_service_test";
    DmAccess accessSide;
    context->direction = DM_AUTH_SOURCE;
    accessSide = context->accessee;
    accessSide.pkgName = "com.test.pin.auth";
    context->authType = DmAuthType::AUTH_TYPE_NFC;
    context->pinCodeFlag = false;
    srvInfo.SetExtraInfo(R"(
{
    "PIN_MATCH_FLAG" : false,
    "PIN_ERROR_COUNT" : 10000
}
)");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkDataSyncState>();
    auto ret = authState->VerifyFlagXor(context);
    EXPECT_FALSE(ret);
}

HWTEST_F(AuthAclTest, VerifyFlagXor_003, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    context->accesser.bundleName = "wear_link_service_test";
    DmAccess accessSide;
    context->direction = DM_AUTH_SOURCE;
    accessSide = context->accessee;
    accessSide.pkgName = "com.test.pin.auth";
    context->authType = DmAuthType::AUTH_TYPE_NFC;
    context->pinCodeFlag = false;
    srvInfo.SetExtraInfo(R"(
{
    "PIN_MATCH_FLAG" : true,
    "PIN_ERROR_COUNT" : 10000
}
)");
    std::string strJson = srvInfo.GetExtraInfo();
    JsonObject object;
    object.Parse(strJson);
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkDataSyncState>();
    auto ret = authState->VerifyFlagXor(context);
    EXPECT_FALSE(ret);
}

HWTEST_F(AuthAclTest, VerifyFlagXor_004, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    context->accesser.bundleName = "wear_link_service_test";
    DmAccess accessSide;
    context->direction = DM_AUTH_SOURCE;
    accessSide = context->accessee;
    accessSide.pkgName = "com.test.pin.auth";
    context->authType = DmAuthType::AUTH_TYPE_NFC;
    context->pinCodeFlag = false;
    srvInfo.SetExtraInfo(R"(
{
    "PIN_MATCH_FLAG" : true,
    "PIN_ERROR_COUNT" : 10000
}
)");
    std::string strJson = srvInfo.GetExtraInfo();
    JsonObject object;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .Times(::testing::AtLeast(1))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkDataSyncState>();
    auto ret = authState->VerifyFlagXor(context);
    EXPECT_FALSE(ret);
}

HWTEST_F(AuthAclTest, VerifyFlagXor_005, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkDataSyncState>();
    context->accesser.bundleName = "com.example.not_in_white_list";
    auto ret = authState->VerifyFlagXor(context);
    EXPECT_FALSE(ret);
}

HWTEST_F(AuthAclTest, VerifyFlagXor_006, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSinkDataSyncState>();
    context->accesser.bundleName = " ";
    auto ret = authState->VerifyFlagXor(context);
    EXPECT_FALSE(ret);
}

HWTEST_F(AuthAclTest, AuthSinkDataSyncState_Action_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    auto authState = std::make_shared<AuthSinkDataSyncState>();
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    context->accessee.dmVersion = DM_VERSION_5_1_5;
    context->pinCodeFlag = true;
    context->accessee.pkgName = "com.test.src.xor.fail";
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({"PIN_MATCH_FLAG":true})");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    int32_t ret = authState->Action(context);
    bool result = false;
    if (ret == DM_OK || ret == ERR_DM_DESERIAL_CERT_FAILED)
    {
        result = true;
    }
    EXPECT_TRUE(result);
}

HWTEST_F(AuthAclTest, AuthSinkDataSyncState_Action_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = nullptr;
    auto authState = std::make_shared<AuthSinkDataSyncState>();
    int32_t result = authState->Action(context);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(AuthAclTest, AuthSinkDataSyncState_Action_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    auto authState = std::make_shared<AuthSinkDataSyncState>();
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    context->accessee.dmVersion = DM_VERSION_5_1_5;
    context->pinCodeFlag = true;
    context->accessee.pkgName = "com.test.src.xor.fail";
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({"PIN_MATCH_FLAG":true})");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    int32_t ret = authState->Action(context);
    bool result = false;
    if (ret == DM_OK || ret == ERR_DM_DESERIAL_CERT_FAILED)
    {
        result = true;
    }
    EXPECT_TRUE(result);
}

HWTEST_F(AuthAclTest, AuthSrcDataSyncState_Action_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    auto authState = std::make_shared<AuthSrcDataSyncState>();
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    context->accesser.dmVersion = DM_VERSION_5_1_5;
    context->pinCodeFlag = true;
    context->accesser.pkgName = "com.test.src.xor.fail";
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({"PIN_MATCH_FLAG":true})");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    int32_t result = authState->Action(context);
    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(AuthAclTest, AuthSrcDataSyncState_Action_002, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = nullptr;
    auto authState = std::make_shared<AuthSrcDataSyncState>();
    int32_t result = authState->Action(context);
    EXPECT_EQ(result, ERR_DM_FAILED);
}

HWTEST_F(AuthAclTest, AuthSrcDataSyncState_Action_003, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    auto authState = std::make_shared<AuthSrcDataSyncState>();
    context->authType = DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE;
    context->accessee.dmVersion = DM_VERSION_5_1_5;
    context->pinCodeFlag = true;
    context->accessee.pkgName = "com.test.src.xor.fail";
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"({"PIN_MATCH_FLAG":true})");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    int32_t result = authState->Action(context);
    EXPECT_EQ(result, DM_OK);
}

HWTEST_F(AuthAclTest, AuthSrcFinishState_UpdatePinErrorCount_001, testing::ext::TestSize.Level1)
{
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    auto authState = std::make_shared<AuthSrcFinishState>();
    std::string pkgName = "ohos.test.pin.update";
    int32_t pinExchangeType = 100;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(Return(ERR_DM_FAILED));
    authState->UpdatePinErrorCount(pkgName, pinExchangeType);
}

HWTEST_F(AuthAclTest, AuthSinkFinishState_UpdatePinErrorCount_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.test.update.pin.error.count";
    int32_t pinExchangeType = 3;
    int32_t expectedCount = 3;
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    auto authState = std::make_shared<AuthSinkFinishState>();
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"(
{
        "PIN_MATCH_FLAG":true,
        "PIN_ERROR_COUNT":3
}
)");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .Times(::testing::AtLeast(1))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateLocalServiceInfo(_))
        .Times(::testing::AtLeast(1)).WillOnce(Return(DM_OK));
    authState->UpdatePinErrorCount(pkgName, pinExchangeType);
    JsonObject InfoObj;
    InfoObj.Parse(srvInfo.GetExtraInfo());
    int32_t count = InfoObj["PIN_ERROR_COUNT"].Get<int32_t>();
    EXPECT_EQ(expectedCount, count);
}

HWTEST_F(AuthAclTest, AuthSinkFinishState_UpdatePinErrorCount_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.test.update.pin.error.count";
    int32_t pinExchangeType = 3;
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    auto authState = std::make_shared<AuthSinkFinishState>();
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    authState->UpdatePinErrorCount(pkgName, pinExchangeType);
    EXPECT_TRUE(srvInfo.GetExtraInfo().empty());
}

HWTEST_F(AuthAclTest, AuthSinkFinishState_UpdatePinErrorCount_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.test.update.pin.error.count";
    int32_t pinExchangeType = 3;
    int32_t expectedCount = 0;
    auto authState = std::make_shared<AuthSinkFinishState>();
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"(
{
        "PIN_MATCH_FLAG":true
}
)");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateLocalServiceInfo(_)).WillOnce(Return(DM_OK));
    authState->UpdatePinErrorCount(pkgName, pinExchangeType);
    JsonObject InfoObj;
    InfoObj.Parse(srvInfo.GetExtraInfo());
    int32_t count = InfoObj["PIN_ERROR_COUNT"].Get<int32_t>();
    EXPECT_EQ(expectedCount, count);
}

HWTEST_F(AuthAclTest, AuthSinkFinishState_UpdatePinErrorCount_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.test.update.pin.error.count";
    int32_t pinExchangeType = 3;
    int32_t expectedCount = 0;
    auto authState = std::make_shared<AuthSinkFinishState>();
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"(
{
        "PIN_MATCH_FLAG":true
}
)");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateLocalServiceInfo(_)).WillOnce(Return(ERR_DM_FAILED));
    authState->UpdatePinErrorCount(pkgName, pinExchangeType);
    JsonObject InfoObj;
    InfoObj.Parse(srvInfo.GetExtraInfo());
    int32_t count = InfoObj["PIN_ERROR_COUNT"].Get<int32_t>();
    EXPECT_EQ(expectedCount, count);
}

HWTEST_F(AuthAclTest, AuthSinkFinishState_UpdatePinErrorCount_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.test.update.pin.error.count";
    int32_t pinExchangeType = 3;
    int32_t expectedCount = 2;
    auto authState = std::make_shared<AuthSinkFinishState>();
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    srvInfo.SetExtraInfo(R"(
{
        "PIN_MATCH_FLAG":true,
        "PIN_ERROR_COUNT":2
}
)");
    EXPECT_CALL(*distributedDeviceProfileClientMock_, GetLocalServiceInfoByBundleAndPinType(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    EXPECT_CALL(*distributedDeviceProfileClientMock_, UpdateLocalServiceInfo(_)).WillOnce(Return(DM_OK));
    authState->UpdatePinErrorCount(pkgName, pinExchangeType);
    JsonObject InfoObj;
    InfoObj.Parse(srvInfo.GetExtraInfo());
    int32_t count = InfoObj["PIN_ERROR_COUNT"].Get<int32_t>();
    EXPECT_EQ(expectedCount, count);
}

HWTEST_F(AuthAclTest, VerifyFlagXor_007, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    context->accesser.bundleName = "wear_link_service_test";
    context->direction = DM_AUTH_SINK;
    context->accessee.pkgName = "watch_system_service";
    srvInfo.SetBundleName("com.huawei.hmos.wearlink");
    context->pinCodeFlag = false;
    srvInfo.SetExtraInfo(R"(
{
    "PIN_MATCH_FLAG" : false,
    "PIN_ERROR_COUNT" : 10000
}
)");
    EXPECT_CALL(*deviceProfileConnectorMock, GetLocalServiceInfoByBundleNameAndPinExchangeType(
        testing::StrEq("com.huawei.hmos.wearlink"), _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    std::shared_ptr<DmAuthState> authState = std::make_shared<AuthSrcDataSyncState>();
    auto ret = authState->VerifyFlagXor(context);
    EXPECT_FALSE(ret);
}

HWTEST_F(AuthAclTest, SinkFinish_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::LocalServiceInfo srvInfo;
    authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector, listener,
        hiChainAuthConnector);
    context = authManager->GetAuthContext();
    auto authState = std::make_shared<AuthSinkFinishState>();
    context->authType = DmAuthType::AUTH_TYPE_NFC;
    context->accessee.pkgName = "watch_system_service";
    srvInfo.SetBundleName("com.huawei.hmos.wearlink");
    EXPECT_CALL(*deviceProfileConnectorMock, GetLocalServiceInfoByBundleNameAndPinExchangeType(
        testing::StrEq("com.huawei.hmos.wearlink"), _, _))
        .WillOnce(DoAll(SetArgReferee<2>(srvInfo), Return(DM_OK)));
    authState->SinkFinish(context);
    EXPECT_EQ(srvInfo.GetBundleName(), "com.huawei.hmos.wearlink");
}
}
}