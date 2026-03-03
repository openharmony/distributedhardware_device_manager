/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "UTTest_dm_credential_manager.h"
#include "accesstoken_kit.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_credential_manager.cpp"
#include "dm_random.h"
#include "nativetoken_kit.h"
#include "parameter.h"
#include "softbus_common.h"
#include "token_setproc.h"

using namespace OHOS::Security::AccessToken;
using ::testing::_;
using ::testing::Return;
namespace OHOS {
namespace DistributedHardware {
void DmCredentialManagerTest::SetUp()
{
    const int32_t permsNum = 2;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dsoftbus_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    hiChainConnectorMock_ = std::make_shared<testing::NiceMock<HiChainConnectorMock>>();
    DmHiChainConnector::dmHiChainConnector = hiChainConnectorMock_;
    ON_CALL(*hiChainConnectorMock_, GetGroupInfo(_, _, _)).WillByDefault(Return(false));
    ON_CALL(*hiChainConnectorMock_, IsDevicesInP2PGroup(_, _)).WillByDefault(Return(false));
    ON_CALL(*hiChainConnectorMock_, GetRelatedGroups(_, _)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, GetRelatedGroups(_, _, _)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, DeleteGroupByACL(_, _)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, DeleteTimeOutGroup(_, _)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, GetRelatedGroupsExt(_, _)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, DeleteGroupExt(_, _)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, GetRegisterInfo(_, _)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, CreateGroup(_, _, _, _)).WillByDefault(Return(ERR_DM_CREATE_GROUP_FAILED));
    ON_CALL(*hiChainConnectorMock_, AddMultiMembers(_, _, _)).WillByDefault(Return(ERR_DM_ADD_GROUP_FAILED));
    ON_CALL(*hiChainConnectorMock_, AddMultiMembersExt(_)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, DeleteMultiMembers(_, _, _)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, DeleteGroupByReqId(_, _, _)).WillByDefault(Return(ERR_DM_FAILED));
    ON_CALL(*hiChainConnectorMock_, RegisterHiChainGroupCallback(_)).WillByDefault(Return(DM_OK));
    ON_CALL(*hiChainConnectorMock_, UnRegisterHiChainGroupCallback()).WillByDefault(Return(DM_OK));
    hiChainConnector_ = std::make_shared<HiChainConnector>();
    listener_ = std::make_shared<MockDeviceManagerServiceListener>();
    dmCreMgr_ = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
}

void DmCredentialManagerTest::TearDown()
{
    DmHiChainConnector::dmHiChainConnector = nullptr;
    hiChainConnectorMock_ = nullptr;
    dmCreMgr_ = nullptr;
    hiChainConnector_ = nullptr;
    listener_ = nullptr;
}

void DmCredentialManagerTest::SetUpTestCase()
{
}

void DmCredentialManagerTest::TearDownTestCase()
{
}

class CaptureCredentialServiceListener : public DeviceManagerServiceListener {
public:
    int32_t credentialResultCallCount_ = 0;
    int32_t credentialAuthStatusCallCount_ = 0;
    int32_t lastAction_ = 0;
    std::string lastResultInfo_;
    std::string lastDeviceList_;
    uint16_t lastDeviceTypeId_ = 0;
    int32_t lastErrCode_ = 0;

    void OnCredentialResult(const ProcessInfo &processInfo, int32_t action, const std::string &resultInfo) override
    {
        (void)processInfo;
        credentialResultCallCount_++;
        lastAction_ = action;
        lastResultInfo_ = resultInfo;
    }

    void OnCredentialAuthStatus(const ProcessInfo &processInfo, const std::string &deviceList,
        uint16_t deviceTypeId, int32_t errcode) override
    {
        (void)processInfo;
        credentialAuthStatusCallCount_++;
        lastDeviceList_ = deviceList;
        lastDeviceTypeId_ = deviceTypeId;
        lastErrCode_ = errcode;
    }
};

/**
 * @tc.name: DmCredentialManager_001
 * @tc.desc: Test whether the DmCredentialManager function can generate a new pointer
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DmCredentialManager_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmCredentialManager> Test = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    ASSERT_NE(Test, nullptr);
}

/**
 * @tc.name: DmCredentialManager_002
 * @tc.desc: Test whether the DmCredentialManager function can delete a new pointer
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DmCredentialManager_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<DmCredentialManager> Test = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    Test.reset();
    EXPECT_EQ(Test, nullptr);
}

/**
 * @tc.name:RegisterCredentialCallback_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RegisterCredentialCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name:RegisterCredentialCallback_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RegisterCredentialCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name:UnRegisterCredentialCallback_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, UnRegisterCredentialCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name:UnRegisterCredentialCallback_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, UnRegisterCredentialCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name:UnRegisterCredentialCallback_003
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, UnRegisterCredentialCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    int32_t ret = dmCreMgr_->RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
    ret = dmCreMgr_->UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterCredentialCallback_004
 * @tc.desc: unregister callback with unregistered pkgName and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, UnRegisterCredentialCallback_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.unregistered";
    int32_t ret = dmCreMgr_->UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name:RequestCredential_001
 * @tc.desc: get credential info and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RequestCredential_001, testing::ext::TestSize.Level1)
{
    std::string reqJsonStr = "invalid json string";
    std::string returnJsonStr;
    int32_t ret = dmCreMgr_->RequestCredential(reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    JsonObject jsonObject;
    jsonObject["userId"] = "test";
    jsonObject["version"] = "test";
    reqJsonStr = jsonObject.Dump();
    ret = dmCreMgr_->RequestCredential(reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name:RequestCredential_002
 * @tc.desc: get credential info and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RequestCredential_002, testing::ext::TestSize.Level1)
{
    std::string reqJsonStr = R"(
    {
        "userId":"4269DC28B639681698809A67EDAD08E39F207900038F91EFF95DD042FE2874E4"
    }
    )";
    std::string returnJsonStr;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->RequestCredential(reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name:RequestCredential_003
 * @tc.desc: get credential info and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RequestCredential_003, testing::ext::TestSize.Level1)
{
    std::string reqJsonStr = R"(
    {
        "userId":"4269DC28B639681698809A67EDAD08E39F207900038F91EFF95DD042FE2874E4"
    }
    )";
    std::string returnJsonStr;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->RequestCredential(reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name:RequestCredential_004
 * @tc.desc: get credential info and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RequestCredential_004, testing::ext::TestSize.Level1)
{
    std::string reqJsonStr = R"(
    {
        "version" : "1.2.3"
    }
    )";
    std::string returnJsonStr;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->RequestCredential(reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: RequestCredential_005
 * @tc.desc: request credential with null hichain connector and return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RequestCredential_005, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[FIELD_USER_ID] = "123";
    jsonObject[FIELD_CREDENTIAL_VERSION] = "1.2.3";
    std::string reqJsonStr = jsonObject.Dump();
    std::string returnJsonStr;
    dmCreMgr_->hiChainConnector_ = nullptr;
    int32_t ret = dmCreMgr_->RequestCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: RequestCredential_006
 * @tc.desc: request credential with null device group manager and return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RequestCredential_006, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[FIELD_USER_ID] = "123";
    jsonObject[FIELD_CREDENTIAL_VERSION] = "1.2.3";
    std::string reqJsonStr = jsonObject.Dump();
    std::string returnJsonStr;
    dmCreMgr_->hiChainConnector_->deviceGroupManager_ = nullptr;
    EXPECT_CALL(*hiChainConnectorMock_, GetRegisterInfo(_, _)).WillOnce(Return(ERR_DM_POINT_NULL));
    int32_t ret = dmCreMgr_->RequestCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: RequestCredential_007
 * @tc.desc: request credential success path and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RequestCredential_007, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[FIELD_USER_ID] = "123";
    jsonObject[FIELD_CREDENTIAL_VERSION] = "1.2.3";
    std::string reqJsonStr = jsonObject.Dump();
    std::string returnJsonStr;
    std::string mockReturn = R"({"ret":"ok"})";
    EXPECT_CALL(*hiChainConnectorMock_, GetRegisterInfo(_, _))
        .WillOnce(testing::DoAll(testing::SetArgReferee<1>(mockReturn), Return(DM_OK)));
    int32_t ret = dmCreMgr_->RequestCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(returnJsonStr, mockReturn);
}

/**
 * @tc.name: ImportCredential_001
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportCredential_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : "test",
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    dmCreMgr_->hiChainConnector_->deviceGroupManager_ = nullptr;
    dmCreMgr_->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    JsonObject jsonObject(credentialInfo);
    jsonObject["TType"] = 1;
    jsonObject["processType"] = 1;
    credentialInfo = jsonObject.Dump();
    ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    jsonObject["processType"] = -1;
    credentialInfo = jsonObject.Dump();
    ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    jsonObject["processType"] = 2;
    credentialInfo = jsonObject.Dump();
    ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    jsonObject["TType"] = "test";
    credentialInfo = jsonObject.Dump();
    ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_002
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportCredential_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo;
    int32_t ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_003
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportCredential_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "userId" , "123"
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_004
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportCredential_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    dmCreMgr_->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_005
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportCredential_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType" : 0,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_006
 * @tc.desc: import remote credential ext success through ImportCredential
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportCredential_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    JsonObject jsonObject;
    jsonObject[FIELD_PROCESS_TYPE] = 2;
    jsonObject[FIELD_TYPE] = "meta";
    std::string credentialInfo = jsonObject.Dump();
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    EXPECT_CALL(*hiChainConnectorMock_, AddMultiMembersExt(_)).WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ImportCredential_007
 * @tc.desc: import remote credential success through ImportCredential
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportCredential_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    EXPECT_CALL(*hiChainConnectorMock_, AddMultiMembers(_, _, _))
        .WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ImportRemoteCredentialExt_001
 * @tc.desc: test ImportRemoteCredentialExt
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredentialExt_001, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = "";
    dmCreMgr_->hiChainConnector_->deviceGroupManager_ = nullptr;
    int32_t ret = dmCreMgr_->ImportRemoteCredentialExt(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredentialExt_002
 * @tc.desc: import remote credential ext success and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredentialExt_002, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = "{}";
    EXPECT_CALL(*hiChainConnectorMock_, AddMultiMembersExt(_)).WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr_->ImportRemoteCredentialExt(credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ImportLocalCredential_001
 * @tc.desc: test ImportLocalCredential
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_001, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 2,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 0,
                "credentialId" : "104",
                "authCode" : "12345",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            },
            {
                "credentialType" : 0,
                "credentialId" : "105",
                "authCode" : "12345",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    int32_t ret = dmCreMgr_->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_002
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType", 1,
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_003
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_003, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_004
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_004, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_005
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_005, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123"
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_006
 * @tc.desc: import local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_006, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_007
 * @tc.desc: test ImportLocalCredential
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_007, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 0,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    int32_t ret = dmCreMgr_->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_008
 * @tc.desc: test ImportLocalCredential
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_008, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 2,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    dmCreMgr_->hiChainConnector_->deviceGroupManager_ = nullptr;
    int32_t ret = dmCreMgr_->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_009
 * @tc.desc: test ImportLocalCredential
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_009, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 2,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    dmCreMgr_->hiChainConnector_->deviceGroupManager_ = nullptr;
    int32_t ret = dmCreMgr_->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_010
 * @tc.desc: import local credential success and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportLocalCredential_010, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : "peerA"
            }
        ]
    }
    )";
    EXPECT_CALL(*hiChainConnectorMock_, CreateGroup(_, _, _, _)).WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr_->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ImportRemoteCredential_001
 * @tc.desc: import remote symmetry credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_001, testing::ext::TestSize.Level1)
{
    CredentialDataInfo credentialDataInfo;
    credentialDataInfo.credentialType = SAME_ACCOUNT_TYPE;
    credentialDataInfo.credentailId = "credential_id";
    credentialDataInfo.serverPk = "server_pk";
    credentialDataInfo.pkInfoSignature = "pk_info_signature";
    credentialDataInfo.pkInfo = "pk_info";
    credentialDataInfo.authCode = "auth_code";
    credentialDataInfo.peerDeviceId = "peer_device_id";
    credentialDataInfo.userId = "user_id";
    credentialDataInfo.credentialType = SAME_ACCOUNT_TYPE;
    JsonObject jsonObject1;
    jsonObject1 = credentialDataInfo;
    JsonObject credentialJson(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    credentialJson.PushBack(jsonObject1);
    JsonObject jsonObject;
    jsonObject.Insert(FIELD_CREDENTIAL_DATA, credentialJson);
    jsonObject[FIELD_AUTH_TYPE] = CROSS_ACCOUNT_TYPE;
    jsonObject[FIELD_USER_ID] = 0;
    jsonObject[FIELD_PEER_USER_ID] = "peerUserId";

    std::string credentialInfo = jsonObject.Dump();
    dmCreMgr_->hiChainConnector_->deviceGroupManager_ = nullptr;
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    jsonObject[FIELD_CREDENTIAL_DATA] = 0;
    credentialInfo = jsonObject.Dump();
    ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_002
 * @tc.desc: import remote symmetry credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_002, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType", 2,
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_003
 * @tc.desc: import remote symmetry credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_003, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId" : "123456785442435DlDFADFAsDFDsAFDjFsAjFDsFDAFDAFDAFDFAsDDFho",
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_004
 * @tc.desc: import remote symmetry credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_004, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "userId" : "123456785442435DlDFADFAsDFDsAFDjFsAjFDsFDAFDAFDAFDFAsDDFho",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_005
 * @tc.desc: import remote symmetry credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_005, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_006
 * @tc.desc: import remote symmetry credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_006, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 2,
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_007
 * @tc.desc: import remote symmetry credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_007, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : "x",
        "userId" : "123456785442435DlDFADFAsDFDsAFDjFsAjFDsFDAFDAFDAFDFAsDDFho",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_008
 * @tc.desc: import remote symmetry credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_008, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId" : "",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_009
 * @tc.desc: import remote credential with unknown authType and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_009, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 0,
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_010
 * @tc.desc: import remote credential success and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_010, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    EXPECT_CALL(*hiChainConnectorMock_, AddMultiMembers(_, _, _))
        .WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ImportRemoteCredential_011
 * @tc.desc: import cross-account remote credential success and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportRemoteCredential_011, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 2,
        "peerUserId" : "cross_user",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    EXPECT_CALL(*hiChainConnectorMock_, AddMultiMembers(_, _, _))
        .WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DeleteRemoteCredential_001
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_001, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 2,
        "peerUserId": "123",
        "peerCredentialInfo" : 0
    }
    )";
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_002
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_002, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType", 2,
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_003
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_003, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "userId" : "123456785442435DlDFADFAsDFDsAFDjFsAjFDsFDAFDAFDAFDFAsDDFho",
        "peerCredentialInfo" :
        [
            {
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_004
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_004, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId" : "123456785442435DlDFADFAsDFDsAFDjFsAjFDsFDAFDAFDAFDFAsDDFho",
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_005
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_005, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId" : "123456785442435DlDFADFAsDFDsAFDjFsAjFDsFDAFDAFDAFDFAsDDFho",
        "peerCredentialInfo" :
        [
            {
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_006
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_006, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "peerCredentialInfo" :
        [
            {
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_007
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_007, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 2,
        "peerCredentialInfo" :
        [
            {
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_008
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_008, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_009
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_009, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 2,
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_0010
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_0010, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId": "123",
        "peerCredentialInfo" :
        [
            {
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_0011
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_0011, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 2,
        "peerUserId": "123",
        "peerCredentialInfo" :
        [
            {
                "peerDeviceId" : "devD"
            }
        ]
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_0012
 * @tc.desc: delete remote credential with unknown authType and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_0012, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 0,
        "peerCredentialInfo" :
        [
            {
                "peerUserId" : "devD"
            }
        ]
    }
    )";
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_0013
 * @tc.desc: delete remote credential success and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_0013, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 2,
        "peerUserId": "123",
        "peerCredentialInfo" :
        [
            {
                "peerUserId" : "devD"
            }
        ]
    }
    )";
    EXPECT_CALL(*hiChainConnectorMock_, DeleteMultiMembers(_, _, _))
        .WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DeleteRemoteCredential_0014
 * @tc.desc: delete same-account remote credential success and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteRemoteCredential_0014, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId": "123",
        "peerCredentialInfo" :
        [
            {
                "peerUserId" : "devD"
            }
        ]
    }
    )";
    EXPECT_CALL(*hiChainConnectorMock_, DeleteMultiMembers(_, _, _))
        .WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DeleteCredential_001
 * @tc.desc: test DeleteCredential
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string resultInfo;
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 2,
        "userId" : "123"
    }
    )";
    dmCreMgr_->requestId_ = 1;
    dmCreMgr_->OnGroupResult(0, 0, resultInfo);
    dmCreMgr_->requestId_ = 0;
    dmCreMgr_->OnGroupResult(0, 0, resultInfo);
    dmCreMgr_->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_002
 * @tc.desc: delete local credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123"
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_003
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "userId" , "123"
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_004
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "authType" : 1,
        "userId" : "123"
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_005
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "userId" : "123"
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_006
 * @tc.desc: delete remote credential and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType" : 0,
        "authType" : 1,
        "userId" : "123"
    }
    )";
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_007
 * @tc.desc: delete credential with unregistered package and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.unregistered";
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123"
    }
    )";
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_008
 * @tc.desc: delete credential with invalid processType and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType" : 3,
        "authType" : 1,
        "userId" : "123"
    }
    )";
    dmCreMgr_->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_009
 * @tc.desc: delete local credential success and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_009, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123"
    }
    )";
    dmCreMgr_->credentialVec_.push_back(pkgName);
    EXPECT_CALL(*hiChainConnectorMock_, DeleteGroupByReqId(_, _, _))
        .WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DeleteCredential_0010
 * @tc.desc: delete remote credential success through DeleteCredential
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_0010, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId" : "123",
        "peerCredentialInfo" :
        [
            {
                "peerUserId" : "devD"
            }
        ]
    }
    )";
    dmCreMgr_->credentialVec_.push_back(pkgName);
    EXPECT_CALL(*hiChainConnectorMock_, DeleteMultiMembers(_, _, _))
        .WillOnce(Return(DM_OK));
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetCredentialData_001
 * @tc.desc: get symmetry credential data and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetCredentialData_001, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    CredentialData credentialData;
    credentialData.credentialType = 1;
    credentialData.credentialId = "104";
    credentialData.serverPk = "";
    credentialData.pkInfoSignature = "";
    credentialData.pkInfo = "";
    credentialData.authCode = "1234567812345678123456781234567812345678123456781234567812345678";
    credentialData.peerDeviceId = "";
    JsonObject jsonOutObj;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->GetCredentialData(credentialInfo, credentialData, jsonOutObj);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetCredentialData_002
 * @tc.desc: get symmetry credential data and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetCredentialData_002, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "credentialData" :
        [
            {
                "credentialType" : 0,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    CredentialData credentialData;
    credentialData.credentialType = 0;
    credentialData.credentialId = "104";
    credentialData.serverPk = "";
    credentialData.pkInfoSignature = "";
    credentialData.pkInfo = "";
    credentialData.authCode = "1234567812345678123456781234567812345678123456781234567812345678";
    credentialData.peerDeviceId = "";
    JsonObject jsonOutObj;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->GetCredentialData(credentialInfo, credentialData, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetCredentialData_003
 * @tc.desc: get nonsymmetry credential data and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetCredentialData_003, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType", 1,
        "authType" : 1,
        "userId" : "123",
        "version" : "1.2.3",
        "deviceId" : "aaa",
        "devicePk" : "0000",
        "credentialData" :
        [
            {
                "credentialType" : 2,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    CredentialData credentialData;
    credentialData.credentialType = 2;
    credentialData.credentialId = "104";
    credentialData.serverPk = "";
    credentialData.pkInfoSignature = "";
    credentialData.pkInfo = "";
    credentialData.authCode = "1234567812345678123456781234567812345678123456781234567812345678";
    credentialData.peerDeviceId = "";
    JsonObject jsonOutObj;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->GetCredentialData(credentialInfo, credentialData, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetCredentialData_004
 * @tc.desc: get nonsymmetry credential data and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetCredentialData_004, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "version" : "1.2.3",
        "deviceId" : "aaa",
        "devicePk" : "0000",
        "credentialData" :
        [
            {
                "credentialType" : 2,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    CredentialData credentialData;
    credentialData.credentialType = 2;
    credentialData.credentialId = "104";
    credentialData.serverPk = "";
    credentialData.pkInfoSignature = "";
    credentialData.pkInfo = "";
    credentialData.authCode = "1234567812345678123456781234567812345678123456781234567812345678";
    credentialData.peerDeviceId = "";
    JsonObject jsonOutObj;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->GetCredentialData(credentialInfo, credentialData, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetCredentialData_005
 * @tc.desc: get nonsymmetry credential data and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetCredentialData_005, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "deviceId" : "aaa",
        "devicePk" : "0000",
        "credentialData" :
        [
            {
                "credentialType" : 2,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    CredentialData credentialData;
    credentialData.credentialType = 2;
    credentialData.credentialId = "104";
    credentialData.serverPk = "";
    credentialData.pkInfoSignature = "";
    credentialData.pkInfo = "";
    credentialData.authCode = "1234567812345678123456781234567812345678123456781234567812345678";
    credentialData.peerDeviceId = "";
    JsonObject jsonOutObj;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->GetCredentialData(credentialInfo, credentialData, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetCredentialData_006
 * @tc.desc: get nonsymmetry credential data and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetCredentialData_006, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "version" : "1.2.3",
        "devicePk" : "0000",
        "credentialData" :
        [
            {
                "credentialType" : 2,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    CredentialData credentialData;
    credentialData.credentialType = 2;
    credentialData.credentialId = "104";
    credentialData.serverPk = "";
    credentialData.pkInfoSignature = "";
    credentialData.pkInfo = "";
    credentialData.authCode = "1234567812345678123456781234567812345678123456781234567812345678";
    credentialData.peerDeviceId = "";
    JsonObject jsonOutObj;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->GetCredentialData(credentialInfo, credentialData, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetCredentialData_007
 * @tc.desc: get nonsymmetry credential data and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetCredentialData_007, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "version" : "1.2.3",
        "deviceId" : "aaa",
        "credentialData" :
        [
            {
                "credentialType" : 2,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    CredentialData credentialData;
    credentialData.credentialType = 2;
    credentialData.credentialId = "104";
    credentialData.serverPk = "";
    credentialData.pkInfoSignature = "";
    credentialData.pkInfo = "";
    credentialData.authCode = "1234567812345678123456781234567812345678123456781234567812345678";
    credentialData.peerDeviceId = "";
    JsonObject jsonOutObj;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->GetCredentialData(credentialInfo, credentialData, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetCredentialData_008
 * @tc.desc: get nonsymmetry credential data and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetCredentialData_008, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "processType" : 1,
        "authType" : 1,
        "userId" : "123",
        "version" : "1.2.3",
        "deviceId" : "aaa",
        "devicePk" : "0000",
        "credentialData" :
        [
            {
                "credentialType" : 2,
                "credentialId" : "104",
                "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
                "serverPk" : "",
                "pkInfoSignature" : "",
                "pkInfo" : "",
                "peerDeviceId" : ""
            }
        ]
    }
    )";
    CredentialData credentialData;
    credentialData.credentialType = 2;
    credentialData.credentialId = "104";
    credentialData.serverPk = "";
    credentialData.pkInfoSignature = "";
    credentialData.pkInfo = "";
    credentialData.authCode = "1234567812345678123456781234567812345678123456781234567812345678";
    credentialData.peerDeviceId = "";
    JsonObject jsonOutObj;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->GetCredentialData(credentialInfo, credentialData, jsonOutObj);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetAddDeviceList_001
 * @tc.desc: get add device list with invalid json and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetAddDeviceList_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    JsonObject jsonDeviceList;
    jsonObject[FIELD_AUTH_TYPE] = "test";
    int32_t ret = dmCreMgr_->GetAddDeviceList(jsonObject, jsonDeviceList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetAddDeviceList_002
 * @tc.desc: get add device list for same account and check userId injection
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetAddDeviceList_002, testing::ext::TestSize.Level1)
{
    JsonObject credentialData(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    JsonObject credentialItem;
    credentialItem[FIELD_CREDENTIAL_TYPE] = SYMMETRY_CREDENTIAL_TYPE;
    credentialItem[FIELD_CREDENTIAL_ID] = "104";
    credentialItem[FIELD_AUTH_CODE] = "authCode";
    credentialItem[FIELD_PEER_DEVICE_ID] = "devA";
    credentialData.PushBack(credentialItem);

    JsonObject jsonObject;
    jsonObject.Insert(FIELD_CREDENTIAL_DATA, credentialData);
    jsonObject[FIELD_AUTH_TYPE] = SAME_ACCOUNT_TYPE;
    jsonObject[FIELD_USER_ID] = "same_account_user";
    JsonObject jsonDeviceList;

    int32_t ret = dmCreMgr_->GetAddDeviceList(jsonObject, jsonDeviceList);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_NE(jsonDeviceList.Dump().find("\"userId\":\"same_account_user\""), std::string::npos);
}

/**
 * @tc.name: GetAddDeviceList_003
 * @tc.desc: get add device list for cross account and check peerUserId injection
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetAddDeviceList_003, testing::ext::TestSize.Level1)
{
    JsonObject credentialData(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    JsonObject credentialItem;
    credentialItem[FIELD_CREDENTIAL_TYPE] = SYMMETRY_CREDENTIAL_TYPE;
    credentialItem[FIELD_CREDENTIAL_ID] = "104";
    credentialItem[FIELD_AUTH_CODE] = "authCode";
    credentialItem[FIELD_PEER_DEVICE_ID] = "devA";
    credentialData.PushBack(credentialItem);

    JsonObject jsonObject;
    jsonObject.Insert(FIELD_CREDENTIAL_DATA, credentialData);
    jsonObject[FIELD_AUTH_TYPE] = CROSS_ACCOUNT_TYPE;
    jsonObject[FIELD_PEER_USER_ID] = "cross_account_user";
    JsonObject jsonDeviceList;

    int32_t ret = dmCreMgr_->GetAddDeviceList(jsonObject, jsonDeviceList);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_NE(jsonDeviceList.Dump().find("\"userId\":\"cross_account_user\""), std::string::npos);
}

/**
 * @tc.name: GetAddDeviceList_004
 * @tc.desc: get add device list with unknown authType and keep userId empty
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, GetAddDeviceList_004, testing::ext::TestSize.Level1)
{
    JsonObject credentialData(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    JsonObject credentialItem;
    credentialItem[FIELD_CREDENTIAL_TYPE] = SYMMETRY_CREDENTIAL_TYPE;
    credentialItem[FIELD_CREDENTIAL_ID] = "104";
    credentialItem[FIELD_AUTH_CODE] = "authCode";
    credentialItem[FIELD_PEER_DEVICE_ID] = "devA";
    credentialData.PushBack(credentialItem);

    JsonObject jsonObject;
    jsonObject.Insert(FIELD_CREDENTIAL_DATA, credentialData);
    jsonObject[FIELD_AUTH_TYPE] = UNKNOWN_CREDENTIAL_TYPE;
    JsonObject jsonDeviceList;

    int32_t ret = dmCreMgr_->GetAddDeviceList(jsonObject, jsonDeviceList);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: from_json_001
 * @tc.desc: test from_json
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, from_json_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonOutObj;
    CredentialDataInfo credentialDataInfo;
    jsonOutObj[FIELD_CREDENTIAL_TYPE] = "test";
    FromJson(jsonOutObj, credentialDataInfo);

    jsonOutObj[FIELD_CREDENTIAL_TYPE] = NONSYMMETRY_CREDENTIAL_TYPE;
    jsonOutObj[FIELD_SERVER_PK] = 0;
    jsonOutObj[FIELD_PKINFO_SIGNATURE] = 0;
    jsonOutObj[FIELD_PKINFO] = 0;
    jsonOutObj[FIELD_PEER_DEVICE_ID] = 0;
    credentialDataInfo.serverPk = "";
    FromJson(jsonOutObj, credentialDataInfo);
    EXPECT_TRUE(credentialDataInfo.serverPk.empty());

    JsonObject jsonPkInfo;
    jsonOutObj[FIELD_SERVER_PK] = "serverPk";
    jsonOutObj[FIELD_PKINFO_SIGNATURE] = "pkInfoSignature";
    jsonOutObj[FIELD_PKINFO] = jsonPkInfo.Dump();
    jsonOutObj[FIELD_PEER_DEVICE_ID] = "peerDeviceId";
    FromJson(jsonOutObj, credentialDataInfo);
    EXPECT_FALSE(credentialDataInfo.serverPk.empty());

    jsonOutObj[FIELD_CREDENTIAL_TYPE] = SYMMETRY_CREDENTIAL_TYPE;
    jsonOutObj[FIELD_AUTH_CODE] = 0;
    credentialDataInfo.authCode = "";
    FromJson(jsonOutObj, credentialDataInfo);
    EXPECT_TRUE(credentialDataInfo.authCode.empty());

    jsonOutObj[FIELD_AUTH_CODE] = "authCode";
    FromJson(jsonOutObj, credentialDataInfo);
    EXPECT_FALSE(credentialDataInfo.authCode.empty());

    jsonOutObj[FIELD_CREDENTIAL_TYPE] = 0;
    FromJson(jsonOutObj, credentialDataInfo);
    EXPECT_FALSE(credentialDataInfo.authCode.empty());
}

/**
 * @tc.name: from_json_002
 * @tc.desc: test from_json
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, from_json_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    PeerCredentialInfo peerCredentialInfo;
    jsonObject[FIELD_PEER_USER_ID] = "peerDeviceId";
    FromJson(jsonObject, peerCredentialInfo);
    EXPECT_EQ(peerCredentialInfo.peerDeviceId, "peerDeviceId");

    std::string deviceList = "deviceList";
    uint16_t deviceTypeId = 0;
    int32_t errcode = 0;
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->HandleCredentialAuthStatus(deviceList, deviceTypeId, errcode);
}

/**
 * @tc.name: from_json_003
 * @tc.desc: test from_json(CredentialData) with incomplete fields
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, from_json_003, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[FIELD_CREDENTIAL_TYPE] = SYMMETRY_CREDENTIAL_TYPE;
    CredentialData credentialData;
    credentialData.credentialType = UNKNOWN_CREDENTIAL_TYPE;
    credentialData.credentialId = "origin_id";
    credentialData.serverPk = "origin_server_pk";
    credentialData.pkInfoSignature = "origin_sig";
    credentialData.pkInfo = "origin_pk";
    credentialData.authCode = "origin_code";
    credentialData.peerDeviceId = "origin_peer";
    FromJson(jsonObject, credentialData);
    EXPECT_EQ(credentialData.credentialId, "origin_id");
}

/**
 * @tc.name: from_json_004
 * @tc.desc: test from_json(CredentialData) with complete fields
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, from_json_004, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    jsonObject[FIELD_CREDENTIAL_TYPE] = NONSYMMETRY_CREDENTIAL_TYPE;
    jsonObject[FIELD_CREDENTIAL_ID] = "credential_id";
    jsonObject[FIELD_SERVER_PK] = "server_pk";
    jsonObject[FIELD_PKINFO_SIGNATURE] = "pk_info_signature";
    jsonObject[FIELD_PKINFO] = "pk_info";
    jsonObject[FIELD_AUTH_CODE] = "auth_code";
    jsonObject[FIELD_PEER_DEVICE_ID] = "peer_device";
    CredentialData credentialData;
    FromJson(jsonObject, credentialData);
    EXPECT_EQ(credentialData.credentialType, NONSYMMETRY_CREDENTIAL_TYPE);
    EXPECT_EQ(credentialData.credentialId, "credential_id");
    EXPECT_EQ(credentialData.serverPk, "server_pk");
    EXPECT_EQ(credentialData.pkInfoSignature, "pk_info_signature");
    EXPECT_EQ(credentialData.pkInfo, "pk_info");
    EXPECT_EQ(credentialData.authCode, "auth_code");
    EXPECT_EQ(credentialData.peerDeviceId, "peer_device");
}

/**
 * @tc.name: to_json_001
 * @tc.desc: test to_json
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, to_json_001, testing::ext::TestSize.Level1)
{
    CredentialDataInfo credentialDataInfo;
    credentialDataInfo.peerDeviceId = "test";
    credentialDataInfo.userId = "test";
    credentialDataInfo.credentailId = "test";
    credentialDataInfo.credentialType = NONSYMMETRY_CREDENTIAL_TYPE;
    credentialDataInfo.serverPk = "test";
    credentialDataInfo.pkInfoSignature = "test";
    credentialDataInfo.pkInfo = "test";
    credentialDataInfo.authCode = "test";
    JsonObject jsonObject;
    ToJson(jsonObject, credentialDataInfo);
    EXPECT_EQ(jsonObject[FIELD_SERVER_PK].Get<std::string>(), "test");

    credentialDataInfo.credentialType = SYMMETRY_CREDENTIAL_TYPE;
    ToJson(jsonObject, credentialDataInfo);
    EXPECT_EQ(jsonObject[FIELD_AUTH_CODE].Get<std::string>(), "test");

    credentialDataInfo.credentialType = UNKNOWN_CREDENTIAL_TYPE;
    ToJson(jsonObject, credentialDataInfo);
    EXPECT_EQ(jsonObject[FIELD_AUTH_CODE].Get<std::string>(), "test");
}

/**
 * @tc.name: to_json_002
 * @tc.desc: test to_json(PeerCredentialInfo)
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, to_json_002, testing::ext::TestSize.Level1)
{
    PeerCredentialInfo peerCredentialInfo;
    peerCredentialInfo.peerDeviceId = "peer_user";
    JsonObject jsonObject;
    ToJson(jsonObject, peerCredentialInfo);
    EXPECT_EQ(jsonObject[FIELD_DEVICE_ID].Get<std::string>(), "peer_user");
}

/**
 * @tc.name: HandleCredentialAuthStatus_001
 * @tc.desc: test HandleCredentialAuthStatus when listener is null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, HandleCredentialAuthStatus_001, testing::ext::TestSize.Level1)
{
    dmCreMgr_->listener_ = nullptr;
    dmCreMgr_->HandleCredentialAuthStatus("device_list", 0, 0);
    EXPECT_EQ(dmCreMgr_->listener_, nullptr);
}

/**
 * @tc.name: HandleCredentialAuthStatus_002
 * @tc.desc: test HandleCredentialAuthStatus with valid listener callback
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, HandleCredentialAuthStatus_002, testing::ext::TestSize.Level1)
{
    auto captureListener = std::make_shared<CaptureCredentialServiceListener>();
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_,
        captureListener);
    dmCreMgr->HandleCredentialAuthStatus("device_list", 7, DM_OK);
    EXPECT_EQ(captureListener->credentialAuthStatusCallCount_, 1);
    EXPECT_EQ(captureListener->lastDeviceList_, "device_list");
    EXPECT_EQ(captureListener->lastDeviceTypeId_, 7);
    EXPECT_EQ(captureListener->lastErrCode_, DM_OK);
}

/**
 * @tc.name: OnGroupResultExt_001
 * @tc.desc: test OnGroupResultExt when listener is null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, OnGroupResultExt_001, testing::ext::TestSize.Level1)
{
    dmCreMgr_->listener_ = nullptr;
    dmCreMgr_->OnGroupResultExt(DM_OK, "success");
    EXPECT_EQ(dmCreMgr_->listener_, nullptr);
}

/**
 * @tc.name: OnGroupResultExt_002
 * @tc.desc: test OnGroupResultExt with valid listener callback
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, OnGroupResultExt_002, testing::ext::TestSize.Level1)
{
    auto captureListener = std::make_shared<CaptureCredentialServiceListener>();
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_,
        captureListener);
    dmCreMgr->OnGroupResultExt(DM_OK, "success");
    EXPECT_EQ(captureListener->credentialResultCallCount_, 1);
    EXPECT_EQ(captureListener->lastAction_, DM_OK);
    EXPECT_EQ(captureListener->lastResultInfo_, "success");
}

/**
 * @tc.name: OnGroupResult_004
 * @tc.desc: test OnGroupResult when requestId match and listener is null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, OnGroupResult_004, testing::ext::TestSize.Level1)
{
    dmCreMgr_->listener_ = nullptr;
    dmCreMgr_->requestId_ = 100;
    dmCreMgr_->OnGroupResult(100, DM_OK, "success");
    EXPECT_EQ(dmCreMgr_->listener_, nullptr);
    EXPECT_EQ(dmCreMgr_->requestId_, 100);
}

/**
 * @tc.name: OnGroupResult_005
 * @tc.desc: test OnGroupResult when requestId mismatch and listener should not be called
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, OnGroupResult_005, testing::ext::TestSize.Level1)
{
    auto captureListener = std::make_shared<CaptureCredentialServiceListener>();
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_,
        captureListener);
    dmCreMgr->requestId_ = 100;
    dmCreMgr->OnGroupResult(101, DM_OK, "success");
    EXPECT_EQ(captureListener->credentialResultCallCount_, 0);
}

/**
 * @tc.name: OnGroupResult_006
 * @tc.desc: test OnGroupResult when requestId match and listener callback is invoked
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, OnGroupResult_006, testing::ext::TestSize.Level1)
{
    auto captureListener = std::make_shared<CaptureCredentialServiceListener>();
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_,
        captureListener);
    dmCreMgr->requestId_ = 100;
    dmCreMgr->OnGroupResult(100, DM_OK, "success");
    EXPECT_EQ(captureListener->credentialResultCallCount_, 1);
    EXPECT_EQ(captureListener->lastAction_, DM_OK);
    EXPECT_EQ(captureListener->lastResultInfo_, "success");
}
} // namespace DistributedHardware
} // namespace OHOS