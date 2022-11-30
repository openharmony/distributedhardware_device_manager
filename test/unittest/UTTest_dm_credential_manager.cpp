/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "dm_random.h"
#include "parameter.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"

using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace DistributedHardware {
void DmCredentialManagerTest::SetUp()
{
    uint64_t tokenId;
    const char *perms[2];
    perms[0] = OHOS_PERMISSION_DISTRIBUTED_SOFTBUS_CENTER;
    perms[1] = OHOS_PERMISSION_DISTRIBUTED_DATASYNC;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
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
}

void DmCredentialManagerTest::TearDown()
{
}

void DmCredentialManagerTest::SetUpTestCase()
{
}

void DmCredentialManagerTest::TearDownTestCase()
{
}

namespace {
std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();

/**
 * @tc.name: DmCredentialManager_001
 * @tc.desc: Test whether the DmCredentialManager function can generate a new pointer
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DmCredentialManager_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DmCredentialManagerTest, DmCredentialManager_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DmCredentialManagerTest, RegisterCredentialCallback_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DmCredentialManagerTest, RegisterCredentialCallback_002, testing::ext::TestSize.Level0)
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
HWTEST_F(DmCredentialManagerTest, UnRegisterCredentialCallback_001, testing::ext::TestSize.Level0)
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
HWTEST_F(DmCredentialManagerTest, UnRegisterCredentialCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name:RequestCredential_001
 * @tc.desc: get credential info and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, RequestCredential_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string reqJsonStr = R"(
    {
        "version" : "1.2.3",
        "userId" : "123"
    }
    )";
    std::string returnJsonStr;
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    int32_t ret = dmCreMgr->RequestCredential(reqJsonStr, returnJsonStr);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ImportCredential_001
 * @tc.desc: import local credential and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportCredential_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
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
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DeleteCredential_001
 * @tc.desc: delete local credential and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
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
    EXPECT_EQ(ret, DM_OK);
}


/**
 * @tc.name: ImportCredential_002
 * @tc.desc: import remote credential and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, ImportCredential_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string credentialInfo = R"(
    {
        "processType" : 2,
        "authType" : 1,
        "userId" : "123456785442435DlDFADFAsDFDsAFDjFsAjFDsFDAFDAFDAFDFAsDDFho",
        "credentialData" : 
        [
            {
                "credentialType" : 1,
                "credentialId" : "104",
                "authCode" : "456",
                "peerDeviceId" : "devD"
            },
            {
                "credentialType" : 1,
                "credentialId" : "105",
                "authCode" : "567",
                "peerDeviceId" : "devE"
            }
        ]
    }
    )";
    std::shared_ptr<DmCredentialManager> dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: DeleteCredential_002
 * @tc.desc: delete remote credential and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmCredentialManagerTest, DeleteCredential_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
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
    dmCreMgr->credentialVec_.push_back(pkgName);
    int32_t ret = dmCreMgr->DeleteCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
