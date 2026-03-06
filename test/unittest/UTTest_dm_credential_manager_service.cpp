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

#include "UTTest_dm_credential_manager_service.h"

#include "dm_anonymous.h"
#include "dm_constants.h"

using ::testing::_;
using ::testing::Return;

namespace OHOS {
namespace DistributedHardware {

constexpr int32_t NONSYMMETRY_CREDENTIAL_TYPE = 2;
constexpr int32_t SYMMETRY_CREDENTIAL_TYPE = 1;
constexpr int32_t UNKNOWN_CREDENTIAL_TYPE = 0;

constexpr const char *FIELD_SERVER_PK = "serverPk";
constexpr const char *FIELD_CREDENTIAL_ID = "credentialId";

struct CredentialDataInfo {
    int32_t credentialType;
    std::string credentialId;
    std::string serverPk;
    std::string pkInfoSignature;
    std::string pkInfo;
    std::string authCode;
    std::string peerDeviceId;
    std::string userId;
    CredentialDataInfo() : credentialType(UNKNOWN_CREDENTIAL_TYPE)
    {
    }
};

struct PeerCredentialInfo {
    std::string peerDeviceId;
    std::string peerCredentialId;
};

void FromJson(const JsonItemObject &jsonObject, CredentialData &credentialData);
void FromJson(const JsonItemObject &jsonObject, CredentialDataInfo &credentialDataInfo);
void ToJson(JsonItemObject &jsonObject, const CredentialDataInfo &credentialDataInfo);
void FromJson(const JsonItemObject &jsonObject, PeerCredentialInfo &peerCredentialInfo);
void ToJson(JsonItemObject &jsonObject, const PeerCredentialInfo &peerCredentialInfo);
int32_t GetDeleteDeviceList(const JsonObject &jsonObject, JsonObject &deviceList);

constexpr int32_t TEST_CALLER_USER_ID = 100;
constexpr int64_t TEST_MISMATCH_REQUEST_ID = 100;
constexpr int64_t TEST_DEFAULT_REQUEST_ID = 0;
constexpr int32_t TEST_GROUP_ACTION_IGNORE = 1;
constexpr int32_t TEST_GROUP_ACTION_NOTIFY = 7;
constexpr int32_t TEST_UNSUPPORTED_CREDENTIAL_TYPE = 99;
constexpr int32_t TEST_INVALID_CREDENTIAL_TYPE = -1;
constexpr int32_t TEST_CREDENTIAL_ID_202 = 202;
constexpr int32_t TEST_CREDENTIAL_ID_203 = 203;

class CaptureCredentialServiceListenerForCred final : public DeviceManagerServiceListener {
public:
    int32_t credentialResultCallCount_ = 0;
    int32_t lastAction_ = 0;
    std::string lastResultInfo_;

    void OnCredentialResult(const ProcessInfo &processInfo, int32_t action, const std::string &resultInfo) override
    {
        (void)processInfo;
        credentialResultCallCount_++;
        lastAction_ = action;
        lastResultInfo_ = resultInfo;
    }
};

void DmCredentialManagerServiceTest::SetUp()
{
    hiChainConnector_ = std::make_shared<DmServiceHiChainConnector>();
    listener_ = std::make_shared<MockDeviceManagerServiceListenerForCred>();
    dmCreMgr_ = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
}

void DmCredentialManagerServiceTest::TearDown()
{
    dmCreMgr_ = nullptr;
    listener_ = nullptr;
    hiChainConnector_ = nullptr;
}

void DmCredentialManagerServiceTest::SetUpTestCase()
{
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
    ON_CALL(*multipleUserConnectorMock_, GetCurrentAccountUserID()).WillByDefault(Return(TEST_CALLER_USER_ID));
    ON_CALL(*multipleUserConnectorMock_, GetCallerUserId(_))
        .WillByDefault([](int32_t &userId) { userId = TEST_CALLER_USER_ID; });
}

void DmCredentialManagerServiceTest::TearDownTestCase()
{
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
}

/**
 * @tc.name: DmCredentialManagerService_001
 * @tc.desc: Test constructor and destructor
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DmCredentialManagerService_001, testing::ext::TestSize.Level1)
{
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener_);
    ASSERT_NE(dmCreMgr, nullptr);
    dmCreMgr.reset();
    EXPECT_EQ(dmCreMgr, nullptr);
}

/**
 * @tc.name: RegisterCredentialCallback_001
 * @tc.desc: Register callback with valid package
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, RegisterCredentialCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    int32_t ret = dmCreMgr_->RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterCredentialCallback_002
 * @tc.desc: Register callback with empty package
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, RegisterCredentialCallback_002, testing::ext::TestSize.Level1)
{
    int32_t ret = dmCreMgr_->RegisterCredentialCallback("");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: UnRegisterCredentialCallback_001
 * @tc.desc: Unregister callback with empty package
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, UnRegisterCredentialCallback_001, testing::ext::TestSize.Level1)
{
    int32_t ret = dmCreMgr_->UnRegisterCredentialCallback("");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: UnRegisterCredentialCallback_002
 * @tc.desc: Unregister callback with registered package
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, UnRegisterCredentialCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    int32_t ret = dmCreMgr_->RegisterCredentialCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
    ret = dmCreMgr_->UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterCredentialCallback_003
 * @tc.desc: Unregister callback for package that was never registered
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, UnRegisterCredentialCallback_003, testing::ext::TestSize.Level1)
{
    int32_t ret = dmCreMgr_->UnRegisterCredentialCallback("com.ohos.unregistered");
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ImportCredential_001
 * @tc.desc: Import credential without registering callback first
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportCredential_001, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"({"processType":1})";
    int32_t ret = dmCreMgr_->ImportCredential("com.ohos.pkg", credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_002
 * @tc.desc: Import credential with invalid json
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportCredential_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    int32_t ret = dmCreMgr_->ImportCredential(pkgName, "invalid json");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_003
 * @tc.desc: Import credential with unknown process type
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportCredential_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    std::string credentialInfo = R"({"processType":3})";
    int32_t ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_004
 * @tc.desc: Import remote credential ext path returns failure and reports callback
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportCredential_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    auto listener = std::make_shared<CaptureCredentialServiceListenerForCred>();
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener);
    ASSERT_EQ(dmCreMgr->RegisterCredentialCallback(pkgName), DM_OK);

    std::string credentialInfo = R"({"processType":2,"TType":"1"})";
    int32_t ret = dmCreMgr->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    EXPECT_EQ(listener->credentialResultCallCount_, 1);
    EXPECT_EQ(listener->lastAction_, ERR_DM_FAILED);
    EXPECT_EQ(listener->lastResultInfo_, "failed");
}

/**
 * @tc.name: ImportCredential_005
 * @tc.desc: Import credential with missing processType
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportCredential_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    std::string credentialInfo = R"({"authType":1})";
    int32_t ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_006
 * @tc.desc: Import credential routes to ImportRemoteCredential branch
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportCredential_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    std::string credentialInfo = R"({"processType":2,"userId":"userA"})";
    int32_t ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportCredential_007
 * @tc.desc: Import credential routes to ImportLocalCredential branch
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportCredential_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    std::string credentialInfo = R"({"processType":1,"authType":"invalid","userId":"userA","credentialData":[]})";
    int32_t ret = dmCreMgr_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_001
 * @tc.desc: Import local credential with invalid json
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportLocalCredential_001, testing::ext::TestSize.Level1)
{
    int32_t ret = dmCreMgr_->ImportLocalCredential("invalid json");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_002
 * @tc.desc: Import local credential with non-array credentialData
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportLocalCredential_002, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"({"authType":1,"userId":"userA","credentialData":{}})";
    int32_t ret = dmCreMgr_->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_003
 * @tc.desc: Import local credential with valid payload and hit CreateGroup path
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportLocalCredential_003, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "authType":1,
        "userId":"userA",
        "version":"1.0",
        "deviceId":"deviceA",
        "devicePk":"devicePkA",
        "credentialData":[
            {
                "credentialType":1,
                "credentialId":"1001",
                "authCode":"authCodeA",
                "serverPk":"",
                "pkInfoSignature":"",
                "pkInfo":"",
                "peerDeviceId":"peerA"
            }
        ]
    })";
    int32_t ret = dmCreMgr_->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportLocalCredential_004
 * @tc.desc: Import local credential with credentialData size not equal to one
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportLocalCredential_004, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "authType":2,
        "userId":"userA",
        "credentialData":[]
    })";
    int32_t ret = dmCreMgr_->ImportLocalCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_001
 * @tc.desc: Import remote credential without authType
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportRemoteCredential_001, testing::ext::TestSize.Level1)
{
    int32_t ret = dmCreMgr_->ImportRemoteCredential(R"({"userId":"userA"})");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_002
 * @tc.desc: Import remote credential without credentialData
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportRemoteCredential_002, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"({"authType":1,"userId":"userA"})";
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_003
 * @tc.desc: Import remote credential with SAME_ACCOUNT but missing userId
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportRemoteCredential_003, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"({"authType":1,"credentialData":[]})";
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_004
 * @tc.desc: Import remote credential with CROSS_ACCOUNT but missing peerUserId
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportRemoteCredential_004, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"({"authType":2,"credentialData":[]})";
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_005
 * @tc.desc: Import remote credential with unknown authType
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportRemoteCredential_005, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "authType":3,
        "credentialData":[
            {
                "credentialType":1,
                "credentialId":"1001",
                "authCode":"authCode"
            }
        ]
    })";
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_006
 * @tc.desc: Import remote credential SAME_ACCOUNT valid json
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportRemoteCredential_006, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "authType":1,
        "userId":"userA",
        "credentialData":[
            {
                "credentialType":1,
                "credentialId":"1002",
                "authCode":"authCodeA",
                "peerDeviceId":"peerA"
            }
        ]
    })";
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: ImportRemoteCredential_007
 * @tc.desc: Import remote credential CROSS_ACCOUNT valid json
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, ImportRemoteCredential_007, testing::ext::TestSize.Level1)
{
    std::string credentialInfo = R"(
    {
        "authType":2,
        "peerUserId":"peerUserA",
        "credentialData":[
            {
                "credentialType":1,
                "credentialId":"1003",
                "authCode":"authCodeB",
                "peerDeviceId":"peerB"
            }
        ]
    })";
    int32_t ret = dmCreMgr_->ImportRemoteCredential(credentialInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_001
 * @tc.desc: Delete credential without registering callback first
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteCredential_001, testing::ext::TestSize.Level1)
{
    std::string deleteInfo = R"({"processType":1,"authType":1,"userId":"userA"})";
    int32_t ret = dmCreMgr_->DeleteCredential("com.ohos.pkg", deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_002
 * @tc.desc: Delete credential with invalid json
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteCredential_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, "invalid json");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_003
 * @tc.desc: Delete credential with unknown process type
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteCredential_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    std::string deleteInfo = R"({"processType":3,"authType":1,"userId":"userA"})";
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_004
 * @tc.desc: Delete credential with missing userId field
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteCredential_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    std::string deleteInfo = R"({"processType":1,"authType":1})";
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_005
 * @tc.desc: Delete credential routes to DeleteRemoteCredential branch
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteCredential_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    std::string deleteInfo = R"({"processType":2,"authType":1,"userId":"userA"})";
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteCredential_006
 * @tc.desc: Delete credential routes to local DeleteGroup branch
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteCredential_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.credential.service";
    ASSERT_EQ(dmCreMgr_->RegisterCredentialCallback(pkgName), DM_OK);
    std::string deleteInfo = R"({"processType":1,"authType":1,"userId":"userA"})";
    int32_t ret = dmCreMgr_->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_001
 * @tc.desc: Delete remote credential without authType
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteRemoteCredential_001, testing::ext::TestSize.Level1)
{
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(R"({"userId":"userA"})");
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_002
 * @tc.desc: Delete remote credential without peerCredentialInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteRemoteCredential_002, testing::ext::TestSize.Level1)
{
    std::string deleteInfo = R"({"authType":1,"userId":"userA"})";
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_003
 * @tc.desc: Delete remote credential with SAME_ACCOUNT but missing userId
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteRemoteCredential_003, testing::ext::TestSize.Level1)
{
    std::string deleteInfo = R"({"authType":1,"peerCredentialInfo":[]})";
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_004
 * @tc.desc: Delete remote credential with CROSS_ACCOUNT but missing peerUserId
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteRemoteCredential_004, testing::ext::TestSize.Level1)
{
    std::string deleteInfo = R"({"authType":2,"peerCredentialInfo":[]})";
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_005
 * @tc.desc: Delete remote credential with unknown authType
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteRemoteCredential_005, testing::ext::TestSize.Level1)
{
    std::string deleteInfo = R"({"authType":3,"peerCredentialInfo":[]})";
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_006
 * @tc.desc: Delete remote credential SAME_ACCOUNT valid json
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteRemoteCredential_006, testing::ext::TestSize.Level1)
{
    std::string deleteInfo = R"(
    {
        "authType":1,
        "userId":"userA",
        "peerCredentialInfo":[
            {
                "peerUserId":"peerUserA"
            }
        ]
    })";
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: DeleteRemoteCredential_007
 * @tc.desc: Delete remote credential CROSS_ACCOUNT valid json
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, DeleteRemoteCredential_007, testing::ext::TestSize.Level1)
{
    std::string deleteInfo = R"(
    {
        "authType":2,
        "peerUserId":"peerUserA",
        "peerCredentialInfo":[
            {
                "peerUserId":"peerUserB"
            }
        ]
    })";
    int32_t ret = dmCreMgr_->DeleteRemoteCredential(deleteInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: OnGroupResult_001
 * @tc.desc: Ignore callback when request id does not match
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, OnGroupResult_001, testing::ext::TestSize.Level1)
{
    auto listener = std::make_shared<CaptureCredentialServiceListenerForCred>();
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener);

    dmCreMgr->OnGroupResult(TEST_MISMATCH_REQUEST_ID, TEST_GROUP_ACTION_IGNORE, "ignore");
    EXPECT_EQ(listener->credentialResultCallCount_, 0);
}

/**
 * @tc.name: OnGroupResult_002
 * @tc.desc: Dispatch callback when request id matches
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, OnGroupResult_002, testing::ext::TestSize.Level1)
{
    auto listener = std::make_shared<CaptureCredentialServiceListenerForCred>();
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener);

    dmCreMgr->OnGroupResult(TEST_DEFAULT_REQUEST_ID, TEST_GROUP_ACTION_NOTIFY, "ok");
    EXPECT_EQ(listener->credentialResultCallCount_, 1);
    EXPECT_EQ(listener->lastAction_, TEST_GROUP_ACTION_NOTIFY);
    EXPECT_EQ(listener->lastResultInfo_, "ok");
}

/**
 * @tc.name: OnGroupResult_003
 * @tc.desc: Handle null listener safely when request id matches
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, OnGroupResult_003, testing::ext::TestSize.Level1)
{
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, nullptr);
    EXPECT_NO_FATAL_FAILURE(dmCreMgr->OnGroupResult(TEST_DEFAULT_REQUEST_ID, TEST_GROUP_ACTION_NOTIFY, "ok"));
}

/**
 * @tc.name: OnGroupResultExt_001
 * @tc.desc: Dispatch extended callback result
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, OnGroupResultExt_001, testing::ext::TestSize.Level1)
{
    auto listener = std::make_shared<CaptureCredentialServiceListenerForCred>();
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, listener);

    dmCreMgr->OnGroupResultExt(DM_OK, "success");
    EXPECT_EQ(listener->credentialResultCallCount_, 1);
    EXPECT_EQ(listener->lastAction_, DM_OK);
    EXPECT_EQ(listener->lastResultInfo_, "success");
}

/**
 * @tc.name: OnGroupResultExt_002
 * @tc.desc: Handle null listener safely
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, OnGroupResultExt_002, testing::ext::TestSize.Level1)
{
    auto dmCreMgr = std::make_shared<DmCredentialManager>(hiChainConnector_, nullptr);
    EXPECT_NO_FATAL_FAILURE(dmCreMgr->OnGroupResultExt(ERR_DM_FAILED, "failed"));
}

/**
 * @tc.name: GetCredentialData_001
 * @tc.desc: GetCredentialData fails when credentialType is unknown
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetCredentialData_001, testing::ext::TestSize.Level1)
{
    CredentialData creData {};
    creData.credentialType = TEST_UNSUPPORTED_CREDENTIAL_TYPE;
    JsonObject jsonOutObj;
    int32_t ret = dmCreMgr_->GetCredentialData("{}", creData, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetCredentialData_002
 * @tc.desc: GetCredentialData with symmetry credential
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetCredentialData_002, testing::ext::TestSize.Level1)
{
    CredentialData creData {};
    creData.credentialType = SYMMETRY_CREDENTIAL_TYPE;
    creData.authCode = "authCode";
    JsonObject jsonOutObj;
    int32_t ret = dmCreMgr_->GetCredentialData("{}", creData, jsonOutObj);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_TRUE(IsString(jsonOutObj, FIELD_AUTH_CODE));
    EXPECT_EQ(jsonOutObj[FIELD_AUTH_CODE].Get<std::string>(), "authCode");
}

/**
 * @tc.name: GetCredentialData_003
 * @tc.desc: GetCredentialData with nonsymmetry credential but missing fields
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetCredentialData_003, testing::ext::TestSize.Level1)
{
    CredentialData creData {};
    creData.credentialType = NONSYMMETRY_CREDENTIAL_TYPE;
    JsonObject jsonOutObj;
    std::string credentialInfo = R"({"userId":"userA","version":"1.0"})";
    int32_t ret = dmCreMgr_->GetCredentialData(credentialInfo, creData, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetCredentialData_004
 * @tc.desc: GetCredentialData with nonsymmetry credential success
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetCredentialData_004, testing::ext::TestSize.Level1)
{
    CredentialData creData {};
    creData.credentialType = NONSYMMETRY_CREDENTIAL_TYPE;
    creData.serverPk = "serverPk";
    creData.pkInfoSignature = "pkSig";
    JsonObject jsonOutObj;
    std::string credentialInfo = R"(
    {
        "userId":"userA",
        "version":"1.0",
        "deviceId":"deviceA",
        "devicePk":"devicePkA"
    })";
    int32_t ret = dmCreMgr_->GetCredentialData(credentialInfo, creData, jsonOutObj);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_TRUE(IsString(jsonOutObj, FIELD_SERVER_PK));
    EXPECT_EQ(jsonOutObj[FIELD_SERVER_PK].Get<std::string>(), "serverPk");
}

/**
 * @tc.name: GetCredentialData_005
 * @tc.desc: GetCredentialData with nonsymmetry credential and invalid json
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetCredentialData_005, testing::ext::TestSize.Level1)
{
    CredentialData creData {};
    creData.credentialType = NONSYMMETRY_CREDENTIAL_TYPE;
    JsonObject jsonOutObj;
    int32_t ret = dmCreMgr_->GetCredentialData("invalid json", creData, jsonOutObj);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetAddDeviceList_001
 * @tc.desc: GetAddDeviceList fails when credentialData is missing
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetAddDeviceList_001, testing::ext::TestSize.Level1)
{
    JsonObject inputObj(R"({"authType":1})");
    JsonObject deviceList;
    int32_t ret = dmCreMgr_->GetAddDeviceList(inputObj, deviceList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetAddDeviceList_002
 * @tc.desc: GetAddDeviceList for SAME_ACCOUNT fills userId
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetAddDeviceList_002, testing::ext::TestSize.Level1)
{
    JsonObject inputObj(R"(
    {
        "authType":1,
        "userId":"userA",
        "credentialData":[
            {
                "credentialType":1,
                "credentialId":"101",
                "authCode":"authA",
                "peerDeviceId":"peerA"
            }
        ]
    })");
    JsonObject deviceList;
    int32_t ret = dmCreMgr_->GetAddDeviceList(inputObj, deviceList);
    EXPECT_EQ(ret, DM_OK);
    std::string deviceListStr = deviceList.Dump();
    EXPECT_NE(deviceListStr.find("\"deviceList\""), std::string::npos);
    EXPECT_NE(deviceListStr.find("\"userId\":\"userA\""), std::string::npos);
}

/**
 * @tc.name: GetAddDeviceList_003
 * @tc.desc: GetAddDeviceList for CROSS_ACCOUNT fills peerUserId
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetAddDeviceList_003, testing::ext::TestSize.Level1)
{
    JsonObject inputObj(R"(
    {
        "authType":2,
        "peerUserId":"peerUserA",
        "credentialData":[
            {
                "credentialType":1,
                "credentialId":"102",
                "authCode":"authB",
                "peerDeviceId":"peerB"
            }
        ]
    })");
    JsonObject deviceList;
    int32_t ret = dmCreMgr_->GetAddDeviceList(inputObj, deviceList);
    EXPECT_EQ(ret, DM_OK);
    std::string deviceListStr = deviceList.Dump();
    EXPECT_NE(deviceListStr.find("\"deviceList\""), std::string::npos);
    EXPECT_NE(deviceListStr.find("\"userId\":\"peerUserA\""), std::string::npos);
}

/**
 * @tc.name: CredentialDataJson_001
 * @tc.desc: FromJson parses complete CredentialData
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, CredentialDataJson_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject(R"(
    {
        "credentialType":1,
        "credentialId":"200",
        "serverPk":"serverPk",
        "pkInfoSignature":"pkSig",
        "pkInfo":"pkInfo",
        "authCode":"authCode",
        "peerDeviceId":"peerDevice"
    })");
    CredentialData credentialData {};
    FromJson(jsonObject, credentialData);
    EXPECT_EQ(credentialData.credentialType, SYMMETRY_CREDENTIAL_TYPE);
    EXPECT_EQ(credentialData.credentialId, "200");
    EXPECT_EQ(credentialData.authCode, "authCode");
    EXPECT_EQ(credentialData.peerDeviceId, "peerDevice");
}

/**
 * @tc.name: CredentialDataJson_002
 * @tc.desc: FromJson handles incomplete CredentialData fields
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, CredentialDataJson_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject(R"({"credentialType":1})");
    CredentialData credentialData {};
    credentialData.credentialType = TEST_INVALID_CREDENTIAL_TYPE;
    FromJson(jsonObject, credentialData);
    EXPECT_EQ(credentialData.credentialType, TEST_INVALID_CREDENTIAL_TYPE);
}

/**
 * @tc.name: CredentialDataInfoJson_001
 * @tc.desc: FromJson parses symmetry CredentialDataInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, CredentialDataInfoJson_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject(R"(
    {
        "credentialType":1,
        "credentialId":"201",
        "authCode":"authCode",
        "peerDeviceId":"peerDevice"
    })");
    CredentialDataInfo credentialDataInfo;
    FromJson(jsonObject, credentialDataInfo);
    EXPECT_EQ(credentialDataInfo.credentialType, SYMMETRY_CREDENTIAL_TYPE);
    EXPECT_EQ(credentialDataInfo.credentialId, "201");
    EXPECT_EQ(credentialDataInfo.authCode, "authCode");
    EXPECT_EQ(credentialDataInfo.peerDeviceId, "peerDevice");
}

/**
 * @tc.name: CredentialDataInfoJson_002
 * @tc.desc: ToJson serializes nonsymmetry CredentialDataInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, CredentialDataInfoJson_002, testing::ext::TestSize.Level1)
{
    CredentialDataInfo credentialDataInfo;
    credentialDataInfo.credentialType = NONSYMMETRY_CREDENTIAL_TYPE;
    credentialDataInfo.credentialId = "202";
    credentialDataInfo.serverPk = "serverPk";
    credentialDataInfo.pkInfoSignature = "pkSig";
    credentialDataInfo.pkInfo = "pkInfo";
    credentialDataInfo.peerDeviceId = "peerDevice";
    credentialDataInfo.userId = "userA";

    JsonObject jsonObject;
    ToJson(jsonObject, credentialDataInfo);
    EXPECT_TRUE(IsString(jsonObject, FIELD_SERVER_PK));
    EXPECT_EQ(jsonObject[FIELD_SERVER_PK].Get<std::string>(), "serverPk");
    EXPECT_TRUE(IsInt32(jsonObject, FIELD_CREDENTIAL_ID));
    EXPECT_EQ(jsonObject[FIELD_CREDENTIAL_ID].Get<int32_t>(), TEST_CREDENTIAL_ID_202);
}

/**
 * @tc.name: CredentialDataInfoJson_003
 * @tc.desc: ToJson serializes symmetry CredentialDataInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, CredentialDataInfoJson_003, testing::ext::TestSize.Level1)
{
    CredentialDataInfo credentialDataInfo;
    credentialDataInfo.credentialType = SYMMETRY_CREDENTIAL_TYPE;
    credentialDataInfo.credentialId = "203";
    credentialDataInfo.authCode = "authCode203";
    credentialDataInfo.peerDeviceId = "peerDevice203";
    credentialDataInfo.userId = "user203";

    JsonObject jsonObject;
    ToJson(jsonObject, credentialDataInfo);
    EXPECT_TRUE(IsString(jsonObject, FIELD_AUTH_CODE));
    EXPECT_EQ(jsonObject[FIELD_AUTH_CODE].Get<std::string>(), "authCode203");
    EXPECT_TRUE(IsInt32(jsonObject, FIELD_CREDENTIAL_ID));
    EXPECT_EQ(jsonObject[FIELD_CREDENTIAL_ID].Get<int32_t>(), TEST_CREDENTIAL_ID_203);
}

/**
 * @tc.name: CredentialDataInfoJson_004
 * @tc.desc: FromJson with missing credentialType
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, CredentialDataInfoJson_004, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject(R"({"credentialId":"204"})");
    CredentialDataInfo credentialDataInfo;
    FromJson(jsonObject, credentialDataInfo);
    EXPECT_EQ(credentialDataInfo.credentialType, UNKNOWN_CREDENTIAL_TYPE);
}

/**
 * @tc.name: PeerCredentialInfoJson_001
 * @tc.desc: FromJson and ToJson for PeerCredentialInfo
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, PeerCredentialInfoJson_001, testing::ext::TestSize.Level1)
{
    JsonObject inJson(R"({"peerUserId":"peerUserA"})");
    PeerCredentialInfo peerCredentialInfo;
    FromJson(inJson, peerCredentialInfo);
    EXPECT_EQ(peerCredentialInfo.peerDeviceId, "peerUserA");

    JsonObject outJson;
    ToJson(outJson, peerCredentialInfo);
    EXPECT_TRUE(IsString(outJson, FIELD_DEVICE_ID));
    EXPECT_EQ(outJson[FIELD_DEVICE_ID].Get<std::string>(), "peerUserA");
}

/**
 * @tc.name: GetDeleteDeviceList_001
 * @tc.desc: GetDeleteDeviceList fails when peerCredentialInfo is missing
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetDeleteDeviceList_001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject(R"({"authType":1})");
    JsonObject deviceList;
    int32_t ret = GetDeleteDeviceList(jsonObject, deviceList);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetDeleteDeviceList_002
 * @tc.desc: GetDeleteDeviceList success
 * @tc.type: FUNC
 */
HWTEST_F(DmCredentialManagerServiceTest, GetDeleteDeviceList_002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject(R"({"peerCredentialInfo":[{"peerUserId":"peerUserA"}]})");
    JsonObject deviceList;
    int32_t ret = GetDeleteDeviceList(jsonObject, deviceList);
    EXPECT_EQ(ret, DM_OK);
    std::string deviceListStr = deviceList.Dump();
    EXPECT_NE(deviceListStr.find("\"deviceList\""), std::string::npos);
    EXPECT_NE(deviceListStr.find("\"deviceId\":\"peerUserA\""), std::string::npos);
}

} // namespace DistributedHardware
} // namespace OHOS
