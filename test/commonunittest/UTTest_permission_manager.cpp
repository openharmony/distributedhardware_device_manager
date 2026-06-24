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

#include "UTTest_permission_manager.h"
#include "access_token.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void PermissionManagerTest::SetUp()
{
    const int32_t permsNum = 2;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.ACCESS_SERVICE_DM";
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
}
constexpr int32_t PAKAGE_NAME_SIZE_MAX = 256;

#define SYSTEM_SA_WHITE_LIST_NUM (4)
constexpr const static char systemSaWhiteList[SYSTEM_SA_WHITE_LIST_NUM][PAKAGE_NAME_SIZE_MAX] = {
    "Samgr_Networking",
    "ohos.distributeddata.service",
    "ohos.dslm",
    "ohos.deviceprofile",
};

void PermissionManagerTest::TearDown()
{
}

void PermissionManagerTest::SetUpTestCase()
{
    DMIPCSkeleton::dmIpcSkeleton_ = ipcSkeletonMock_;
    DmAccessTokenKit::accessToken_ = accessTokenKitMock_;
}

void PermissionManagerTest::TearDownTestCase()
{
    DMIPCSkeleton::dmIpcSkeleton_ = nullptr;
    ipcSkeletonMock_ = nullptr;
    DmAccessTokenKit::accessToken_ = nullptr;
    accessTokenKitMock_ = nullptr;
}

namespace {

/**
 * @tc.name: PinAuthUi::CheckPermission_001
 * @tc.desc: the return value is true
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(PermissionManagerTest, CheckPermission_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    bool ret = PermissionManager::GetInstance().CheckAccessServicePermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_TYPE_BUTT));
    ret = PermissionManager::GetInstance().CheckAccessServicePermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_DENIED));
    ret = PermissionManager::GetInstance().CheckAccessServicePermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));
    ret = PermissionManager::GetInstance().CheckAccessServicePermission();
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: PinAuthUi::CheckWhiteListSystemSA_001
 * @tc.desc: the return value is false
 * @tc.type：FUNC
 * @tc.require: AR000GHSJK
*/
HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_EQ(ret, false);
    pkgName = "pkgName";
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: PinAuthUi::CheckWhiteListSystemSA_002
 * @tc.desc: the return value is true
 * @tc.type：FUNC
 * @tc.require: AR000GHSJK
*/
HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    std::string pkgName1(systemSaWhiteList[0]);
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName1);
    ASSERT_EQ(ret, true);
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    std::string pkgName2(systemSaWhiteList[1]);
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName2);
    ASSERT_EQ(ret, true);
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    std::string pkgName3(systemSaWhiteList[2]);
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName3);
    ASSERT_EQ(ret, true);
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    std::string pkgName4(systemSaWhiteList[3]);
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName4);
    ASSERT_EQ(ret, true);
}

HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_101, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_102, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ohos.dhardware";
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, CheckMonitorPermission_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    bool ret = PermissionManager::GetInstance().CheckMonitorPermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_TYPE_BUTT));
    ret = PermissionManager::GetInstance().CheckMonitorPermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_DENIED));
    ret = PermissionManager::GetInstance().CheckMonitorPermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));
    ret = PermissionManager::GetInstance().CheckMonitorPermission();
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, GetWhiteListSystemSA_001, testing::ext::TestSize.Level1)
{
    auto ret = PermissionManager::GetInstance().GetWhiteListSystemSA();
    ASSERT_FALSE(ret.empty());
}

HWTEST_F(PermissionManagerTest, CheckSystemSA_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    bool ret = PermissionManager::GetInstance().CheckSystemSA(pkgName);
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    ret = PermissionManager::GetInstance().CheckSystemSA(pkgName);
    ASSERT_TRUE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1002));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_TYPE_BUTT));
    ret = PermissionManager::GetInstance().CheckSystemSA(pkgName);
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, GetCallerProcessName_002, testing::ext::TestSize.Level1)
{
    std::string processName;
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    int32_t ret = PermissionManager::GetInstance().GetCallerProcessName(processName);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, GetHapTokenInfo(_, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = PermissionManager::GetInstance().GetCallerProcessName(processName);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, GetHapTokenInfo(_, _)).WillOnce(Return(DM_OK));
    ret = PermissionManager::GetInstance().GetCallerProcessName(processName);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, GetNativeTokenInfo(_, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = PermissionManager::GetInstance().GetCallerProcessName(processName);
    ASSERT_EQ(ret, ERR_DM_FAILED);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, GetNativeTokenInfo(_, _)).WillOnce(Return(DM_OK));
    ret = PermissionManager::GetInstance().GetCallerProcessName(processName);
    ASSERT_EQ(ret, DM_OK);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_TYPE_BUTT));
    ret = PermissionManager::GetInstance().GetCallerProcessName(processName);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(PermissionManagerTest, CheckProcessNameValidOnGetDeviceInfo_001, testing::ext::TestSize.Level1)
{
    std::string processName = "";
    auto ret = PermissionManager::GetInstance().CheckProcessNameValidOnGetDeviceInfo(processName);
    ASSERT_FALSE(ret);

    processName = "processName";
    ret = PermissionManager::GetInstance().CheckProcessNameValidOnGetDeviceInfo(processName);
    ASSERT_FALSE(ret);

    processName = "gameservice_server";
    ret = PermissionManager::GetInstance().CheckProcessNameValidOnGetDeviceInfo(processName);
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, CheckReadLocalDeviceName_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    bool ret = PermissionManager::GetInstance().CheckReadLocalDeviceName();
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, CheckDataSyncPermission_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    bool ret = PermissionManager::GetInstance().CheckDataSyncPermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_TYPE_BUTT));
    ret = PermissionManager::GetInstance().CheckDataSyncPermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_DENIED));
    ret = PermissionManager::GetInstance().CheckDataSyncPermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));
    ret = PermissionManager::GetInstance().CheckDataSyncPermission();
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, CheckAccessUdidPermission_001, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    bool ret = PermissionManager::GetInstance().CheckAccessUdidPermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(20));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_TYPE_BUTT));
    ret = PermissionManager::GetInstance().CheckAccessUdidPermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(20));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_DENIED));
    ret = PermissionManager::GetInstance().CheckAccessUdidPermission();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(20));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));
    ret = PermissionManager::GetInstance().CheckAccessUdidPermission();
    ASSERT_TRUE(ret);
}

/* CheckMonitorPermission TOKEN_HAP branch, missing from the suite above. */
HWTEST_F(PermissionManagerTest, CheckMonitorPermission_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));
    bool ret = PermissionManager::GetInstance().CheckMonitorPermission();
    ASSERT_TRUE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_DENIED));
    ret = PermissionManager::GetInstance().CheckMonitorPermission();
    ASSERT_FALSE(ret);
}

/* pkgName in white-list but caller is not TOKEN_NATIVE, and token id == 0. */
HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_201, testing::ext::TestSize.Level1)
{
    std::string pkgName("ohos.dhardware");
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA(pkgName);
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, CheckProcessNameValidOnAuthCode_001, testing::ext::TestSize.Level1)
{
    std::string emptyName;
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode(emptyName);
    ASSERT_FALSE(ret);

    std::string invalidName = "invalid_process";
    ret = PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode(invalidName);
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode("CollaborationFwk");
    ASSERT_TRUE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode("car_distributed_engine");
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, CheckProcessNameValidOnPinHolder_001, testing::ext::TestSize.Level1)
{
    std::string emptyName;
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(emptyName);
    ASSERT_FALSE(ret);

    std::string invalidName = "invalid_process";
    ret = PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder(invalidName);
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnPinHolder("CollaborationFwk");
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, CheckProcessNameValidOnSetDnPolicy_001, testing::ext::TestSize.Level1)
{
    std::string emptyName;
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnSetDnPolicy(emptyName);
    ASSERT_FALSE(ret);

    std::string invalidName = "invalid_process";
    ret = PermissionManager::GetInstance().CheckProcessNameValidOnSetDnPolicy(invalidName);
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnSetDnPolicy("collaboration_service");
    ASSERT_TRUE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnSetDnPolicy("glasses_collaboration_service");
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, CheckProcessNameValidOnGetDeviceInfo_002, testing::ext::TestSize.Level1)
{
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnGetDeviceInfo("token_sync_service");
    ASSERT_TRUE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnGetDeviceInfo("com.example.genericassistant");
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, CheckProcessNameValidModifyLocalDeviceName_001, testing::ext::TestSize.Level1)
{
    std::string emptyName;
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidModifyLocalDeviceName(emptyName);
    ASSERT_FALSE(ret);

    std::string invalidName = "invalid_process";
    ret = PermissionManager::GetInstance().CheckProcessNameValidModifyLocalDeviceName(invalidName);
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidModifyLocalDeviceName("com.ohos.settings");
    ASSERT_TRUE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidModifyLocalDeviceName("com.example.tvservice");
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, CheckProcessNameValidModifyRemoteDeviceName_001, testing::ext::TestSize.Level1)
{
    std::string emptyName;
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidModifyRemoteDeviceName(emptyName);
    ASSERT_FALSE(ret);

    std::string invalidName = "invalid_process";
    ret = PermissionManager::GetInstance().CheckProcessNameValidModifyRemoteDeviceName(invalidName);
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidModifyRemoteDeviceName("com.ohos.settings");
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, CheckProcessNameValidPutDeviceProfileInfoList_001, testing::ext::TestSize.Level1)
{
    std::string emptyName;
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidPutDeviceProfileInfoList(emptyName);
    ASSERT_FALSE(ret);

    std::string invalidName = "invalid_process";
    ret = PermissionManager::GetInstance().CheckProcessNameValidPutDeviceProfileInfoList(invalidName);
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidPutDeviceProfileInfoList("com.example.validsvc");
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidPutDeviceProfileInfoList("com.example.tvservice");
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, CheckProcessValidOnGetTrustedDeviceList_001, testing::ext::TestSize.Level1)
{
    /* GetCallerProcessName fails when token id == 0. */
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    bool ret = PermissionManager::GetInstance().CheckProcessValidOnGetTrustedDeviceList();
    ASSERT_FALSE(ret);

    /* Native token resolves a process name NOT in the white-list. */
    NativeTokenInfo invalidInfo;
    invalidInfo.processName = "invalid_process";
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, GetNativeTokenInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(invalidInfo), Return(DM_OK)));
    ret = PermissionManager::GetInstance().CheckProcessValidOnGetTrustedDeviceList();
    ASSERT_FALSE(ret);

    /* Native token resolves a process name that IS in the white-list. */
    NativeTokenInfo trustedInfo;
    trustedInfo.processName = "distributedsched";
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, GetNativeTokenInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(trustedInfo), Return(DM_OK)));
    ret = PermissionManager::GetInstance().CheckProcessValidOnGetTrustedDeviceList();
    ASSERT_TRUE(ret);
}

/* CheckReadLocalDeviceName TOKEN_HAP branch, missing from the suite above. */
HWTEST_F(PermissionManagerTest, CheckReadLocalDeviceName_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));
    bool ret = PermissionManager::GetInstance().CheckReadLocalDeviceName();
    ASSERT_TRUE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_DENIED));
    ret = PermissionManager::GetInstance().CheckReadLocalDeviceName();
    ASSERT_FALSE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    ret = PermissionManager::GetInstance().CheckReadLocalDeviceName();
    ASSERT_FALSE(ret);
}

HWTEST_F(PermissionManagerTest, CheckOnReadyRetrospectiveNotificationBlackList_001, testing::ext::TestSize.Level1)
{
    /* GetCallerProcessName fails when token id == 0. */
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(0));
    bool ret = PermissionManager::GetInstance().CheckOnReadyRetrospectiveNotificationBlackList();
    ASSERT_FALSE(ret);

    /* Native token resolves a process name NOT in the black-list. */
    NativeTokenInfo normalInfo;
    normalInfo.processName = "invalid_process";
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, GetNativeTokenInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(normalInfo), Return(DM_OK)));
    ret = PermissionManager::GetInstance().CheckOnReadyRetrospectiveNotificationBlackList();
    ASSERT_FALSE(ret);

    /* Native token resolves a process name that IS in the black-list. */
    NativeTokenInfo blackInfo;
    blackInfo.processName = "distributeddata";
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1001));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, GetNativeTokenInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(blackInfo), Return(DM_OK)));
    ret = PermissionManager::GetInstance().CheckOnReadyRetrospectiveNotificationBlackList();
    ASSERT_TRUE(ret);
}

HWTEST_F(PermissionManagerTest, CheckPkgNameInWhiteList_001, testing::ext::TestSize.Level1)
{
    std::string emptyName;
    bool ret = PermissionManager::GetInstance().CheckPkgNameInWhiteList(emptyName);
    ASSERT_FALSE(ret);

    std::string invalidName = "invalid_pkg";
    ret = PermissionManager::GetInstance().CheckPkgNameInWhiteList(invalidName);
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckPkgNameInWhiteList("valid_pkg1");
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckPkgNameInWhiteList("valid_pkg2");
    ASSERT_FALSE(ret);
}

/* VerifyAccessTokenByPermissionName TOKEN_NATIVE branch for CheckAccessServicePermission
 * (the || TOKEN_NATIVE operand on line 370 was only exercised via TOKEN_HAP). */
HWTEST_F(PermissionManagerTest, CheckAccessServicePermission_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));
    bool ret = PermissionManager::GetInstance().CheckAccessServicePermission();
    ASSERT_TRUE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_DENIED));
    ret = PermissionManager::GetInstance().CheckAccessServicePermission();
    ASSERT_FALSE(ret);
}

/* VerifyAccessTokenByPermissionName TOKEN_NATIVE branch for CheckDataSyncPermission. */
HWTEST_F(PermissionManagerTest, CheckDataSyncPermission_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));
    bool ret = PermissionManager::GetInstance().CheckDataSyncPermission();
    ASSERT_TRUE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_DENIED));
    ret = PermissionManager::GetInstance().CheckDataSyncPermission();
    ASSERT_FALSE(ret);
}

/* VerifyAccessTokenByPermissionName TOKEN_NATIVE branch for CheckAccessUdidPermission. */
HWTEST_F(PermissionManagerTest, CheckAccessUdidPermission_002, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(20));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_GRANTED));
    bool ret = PermissionManager::GetInstance().CheckAccessUdidPermission();
    ASSERT_TRUE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(20));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, VerifyAccessToken(_, _))
        .WillOnce(Return(Security::AccessToken::PermissionState::PERMISSION_DENIED));
    ret = PermissionManager::GetInstance().CheckAccessUdidPermission();
    ASSERT_FALSE(ret);
}

/* Additional SYSTEM_SA_WHITE_LIST members beyond the first four and ohos.dhardware,
 * exercising more loop iterations of CheckWhiteListSystemSA. */
HWTEST_F(PermissionManagerTest, CheckWhiteListSystemSA_202, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    bool ret = PermissionManager::GetInstance().CheckWhiteListSystemSA("distributed_bundle_framework");
    ASSERT_TRUE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA("audio_manager_service");
    ASSERT_TRUE(ret);

    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(10));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    ret = PermissionManager::GetInstance().CheckWhiteListSystemSA("hmos.collaborationfwk.deviceDetect");
    ASSERT_TRUE(ret);
}

/* GetCallerProcessName TOKEN_NATIVE success: processName out-param is filled via
 * std::move(tokenInfo.processName), verified by reading it back. */
HWTEST_F(PermissionManagerTest, GetCallerProcessName_003, testing::ext::TestSize.Level1)
{
    std::string processName;
    NativeTokenInfo nativeInfo;
    nativeInfo.processName = "native_proc_003";
    EXPECT_CALL(*ipcSkeletonMock_, GetCallingTokenID()).WillOnce(Return(1003));
    EXPECT_CALL(*accessTokenKitMock_, GetTokenTypeFlag(_)).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(*accessTokenKitMock_, GetNativeTokenInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(nativeInfo), Return(0)));
    int32_t ret = PermissionManager::GetInstance().GetCallerProcessName(processName);
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(processName, "native_proc_003");
}

/* CheckProcessNameValidOnAuthCode additional valid white-list members for loop coverage. */
HWTEST_F(PermissionManagerTest, CheckProcessNameValidOnAuthCode_002, testing::ext::TestSize.Level1)
{
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode("wear_link_service");
    ASSERT_TRUE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode("caas_service");
    ASSERT_TRUE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnAuthCode("xr_glass_app_service");
    ASSERT_TRUE(ret);
}

/* CheckProcessNameValidOnSetDnPolicy additional valid white-list members. */
HWTEST_F(PermissionManagerTest, CheckProcessNameValidOnSetDnPolicy_002, testing::ext::TestSize.Level1)
{
    bool ret = PermissionManager::GetInstance().CheckProcessNameValidOnSetDnPolicy("watch_system_service");
    ASSERT_TRUE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnSetDnPolicy("com.example.walletservice");
    ASSERT_FALSE(ret);

    ret = PermissionManager::GetInstance().CheckProcessNameValidOnSetDnPolicy("com.ohos.distributedjstest");
    ASSERT_TRUE(ret);
}
}
} // namespace DistributedHardware
} // namespace OHOS