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
    hiChainConnector_ = std::make_shared<HiChainConnector>();
    listener_ = std::make_shared<MockDeviceManagerServiceListener>();
    dmCreMgr_ = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
}

void DmCredentialManagerTest::TearDown()
{
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    reqJsonStr = SafetyDump(jsonObject);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->RequestCredential(reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, ERR_DM_FAILED);
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
    credentialInfo = SafetyDump(jsonObject);
    ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    jsonObject["processType"] = -1;
    credentialInfo = SafetyDump(jsonObject);
    ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    jsonObject["processType"] = 2;
    credentialInfo = SafetyDump(jsonObject);
    ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    jsonObject["TType"] = "test";
    credentialInfo = SafetyDump(jsonObject);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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

    std::string credentialInfo = SafetyDump(jsonObject);
    dmCreMgr_->hiChainConnector_->deviceGroupManager_ = nullptr;
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    jsonObject[FIELD_CREDENTIAL_DATA] = 0;
    credentialInfo = SafetyDump(jsonObject);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->DeleteRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->GetCredentialData(credentialInfo, credentialData, jsonOutObj);
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
    jsonOutObj[FIELD_PKINFO] = SafetyDump(jsonPkInfo);
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->HandleCredentialAuthStatus(deviceList, deviceTypeId, errcode);
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
} // namespace DistributedHardware
} // namespace OHOS
