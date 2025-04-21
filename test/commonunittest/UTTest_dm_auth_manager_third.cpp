/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "UTTest_dm_auth_manager_third.h"

#include "auth_message_processor.h"
#include "device_manager_service_listener.h"
#include "deviceprofile_connector.h"
#include "dm_auth_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_dialog_manager.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "multiple_user_connector.h"
#include "json_object.h"
#include "softbus_error_code.h"

static bool g_checkIsOnlineReturnBoolValue = false;
static bool g_checkSrcDevIdInAclForDevBindReturnBoolValue = false;
static bool g_isDevicesInP2PGroupReturnBoolValue = false;
static bool g_isIdenticalAccountReturnBoolValue = false;
static bool g_isLocked = false;
static bool g_reportAuthConfirmBoxReturnBoolValue = false;
static bool g_reportAuthInputPinBoxReturnBoolValue = false;
static int32_t g_bindType = INVALIED_TYPE;
static int32_t g_leftAclNumber = 0;
static int32_t g_trustNumber = 0;
static std::string g_accountId = "";
static std::string g_createSimpleMessageReturnDataStr = "{}";
static std::string g_peerUdidHash = "";

namespace OHOS {
namespace DistributedHardware {
std::string AuthMessageProcessor::CreateSimpleMessage(int32_t msgType)
{
    return g_createSimpleMessageReturnDataStr;
}

bool DmAuthManager::IsIdenticalAccount()
{
    return g_isIdenticalAccountReturnBoolValue;
}

void DmAuthManager::GetPeerUdidHash(int32_t sessionId, std::string &peerUdidHash)
{
    peerUdidHash = g_peerUdidHash;
}

bool DmAuthManager::IsScreenLocked()
{
    return g_isLocked;
}

bool SoftbusConnector::CheckIsOnline(const std::string &targetDeviceId)
{
    return g_checkIsOnlineReturnBoolValue;
}

bool DmRadarHelper::ReportAuthConfirmBox(struct RadarInfo &info)
{
    return g_reportAuthConfirmBoxReturnBoolValue;
}

bool DmRadarHelper::ReportAuthInputPinBox(struct RadarInfo &info)
{
    return g_reportAuthInputPinBoxReturnBoolValue;
}

int32_t DeviceProfileConnector::GetTrustNumber(const std::string &deviceId)
{
    return g_trustNumber;
}

std::string MultipleUserConnector::GetOhosAccountId(void)
{
    return g_accountId;
}

bool HiChainConnector::IsDevicesInP2PGroup(const std::string &hostDevice, const std::string &peerDevice)
{
    return g_isDevicesInP2PGroupReturnBoolValue;
}

bool DeviceProfileConnector::CheckSrcDevIdInAclForDevBind(const std::string &pkgName, const std::string &deviceId)
{
    return g_checkSrcDevIdInAclForDevBindReturnBoolValue;
}

void DmAuthManagerTest::SetUp()
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
    authManager_ = std::make_shared<DmAuthManager>(softbusConnector, hiChainConnector, listener, hiChainAuthConnector);

    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->softbusConnector_ = std::make_shared<SoftbusConnector>();
    authManager_->softbusConnector_->GetSoftbusSession()->RegisterSessionCallback(
        std::shared_ptr<ISoftbusSessionCallback>(authManager_));
    authManager_->timer_ = std::make_shared<DmTimer>();
}
void DmAuthManagerTest::TearDown()
{
    g_bindType = INVALIED_TYPE;
    g_createSimpleMessageReturnDataStr = "{}";
    g_isIdenticalAccountReturnBoolValue = false;
    g_leftAclNumber = 0;
    g_peerUdidHash = "";
}
void DmAuthManagerTest::SetUpTestCase() {}
void DmAuthManagerTest::TearDownTestCase() {}

HWTEST_F(DmAuthManagerTest, ProcRespNegotiate001, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    g_createSimpleMessageReturnDataStr = SafetyDump(jsonObject);
    int32_t sessionId = 0;
    authManager_->remoteDeviceId_ = "ProcRespNegotiate001";
    authManager_->ProcRespNegotiate(sessionId);

    jsonObject[TAG_CRYPTO_SUPPORT] = false;
    g_createSimpleMessageReturnDataStr = SafetyDump(jsonObject);
    g_isIdenticalAccountReturnBoolValue = true;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->authType = AUTH_TYPE_CRE;
    authManager_->ProcRespNegotiate(sessionId);

    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    jsonObject[TAG_CRYPTO_SUPPORT] = true;
    g_createSimpleMessageReturnDataStr = SafetyDump(jsonObject);
    g_isIdenticalAccountReturnBoolValue = true;
    authManager_->importAuthCode_ = "";
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = false;
    authManager_->ProcRespNegotiate(sessionId);

    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    g_createSimpleMessageReturnDataStr = SafetyDump(jsonObject);
    g_isIdenticalAccountReturnBoolValue = true;
    authManager_->importAuthCode_ = "test";
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = true;
    authManager_->ProcRespNegotiate(sessionId);

    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    jsonObject[TAG_CRYPTO_NAME] = "test";
    g_createSimpleMessageReturnDataStr = SafetyDump(jsonObject);
    g_isIdenticalAccountReturnBoolValue = true;
    authManager_->importAuthCode_ = "test";
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = true;
    authManager_->ProcRespNegotiate(sessionId);
    EXPECT_FALSE(authManager_->authResponseContext_->isOnline);
}

HWTEST_F(DmAuthManagerTest, ProcRespNegotiate002, testing::ext::TestSize.Level1)
{
    JsonObject jsonObject;
    g_createSimpleMessageReturnDataStr = SafetyDump(jsonObject);
    int32_t sessionId = 0;
    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    jsonObject[TAG_CRYPTO_SUPPORT] = true;
    jsonObject[TAG_CRYPTO_NAME] = "test";
    jsonObject[TAG_CRYPTO_VERSION] = "test";
    g_createSimpleMessageReturnDataStr = SafetyDump(jsonObject);
    g_isIdenticalAccountReturnBoolValue = false;
    authManager_->remoteDeviceId_ = "ProcRespNegotiate002";
    authManager_->importAuthCode_ = "test";
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = true;
    authManager_->authResponseState_->context_->cryptoName = jsonObject[TAG_CRYPTO_NAME].Get<std::string>();
    authManager_->ProcRespNegotiate(sessionId);
    EXPECT_FALSE(authManager_->authResponseContext_->isOnline);

    authManager_->authResponseState_->context_ = std::make_shared<DmAuthResponseContext>();
    jsonObject[TAG_CRYPTO_SUPPORT] = true;
    jsonObject[TAG_CRYPTO_NAME] = "test";
    jsonObject[TAG_CRYPTO_VERSION] = "test";
    g_createSimpleMessageReturnDataStr = SafetyDump(jsonObject);
    g_isIdenticalAccountReturnBoolValue = false;
    authManager_->importAuthCode_ = "test";
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseState_->context_->cryptoSupport = true;
    authManager_->authResponseState_->context_->cryptoName = jsonObject[TAG_CRYPTO_NAME].Get<std::string>();
    authManager_->authResponseState_->context_->cryptoVer = jsonObject[TAG_CRYPTO_VERSION].Get<std::string>();
    authManager_->ProcRespNegotiate(sessionId);
    EXPECT_FALSE(authManager_->authResponseContext_->isOnline);
}

HWTEST_F(DmAuthManagerTest, AuthenticateFinish001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authResponseState_ = nullptr;
    authManager_->remoteVersion_ = "4.1.5.2";
    authManager_->authResponseContext_->bindLevel = INVALIED_TYPE;
    authManager_->authResponseContext_->isFinish = true;
    authManager_->authRequestState_ = std::make_shared<AuthRequestJoinState>();
    g_checkIsOnlineReturnBoolValue = true;
    g_trustNumber = 1;
    authManager_->softbusConnector_->deviceStateManagerCallback_ = std::make_shared<SoftbusStateCallbackMock>();
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authMessageProcessor_, nullptr);

    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->isFinish = false;
    g_trustNumber = 1;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authMessageProcessor_, nullptr);

    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    g_checkIsOnlineReturnBoolValue = false;
    g_trustNumber = 1;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authMessageProcessor_, nullptr);

    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseContext_->bindLevel = APP_PEER_TO_PEER_TYPE;
    g_trustNumber = 1;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authMessageProcessor_, nullptr);

    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->remoteVersion_ = "4.1.5.0";
    g_trustNumber = 2;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authMessageProcessor_, nullptr);

    g_trustNumber = 0;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authMessageProcessor_, nullptr);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->authType = AUTH_TYPE_UNKNOW;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    g_isLocked = true;
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_->state, STATUS_DM_AUTH_DEFAULT);

    g_isLocked = false;
    authManager_->authResponseContext_->targetDeviceName = "test";
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_->targetDeviceName, DmDialogManager::GetInstance().targetDeviceName_);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->authType = AUTH_TYPE_NFC;
    authManager_->importAuthCode_ = "123456";
    authManager_->importPkgName_ = "hostPkgName";
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->targetDeviceName = "ShowStartAuthDialog_004";
    authManager_->ShowStartAuthDialog();
    EXPECT_NE(authManager_->authResponseContext_->targetDeviceName, DmDialogManager::GetInstance().targetDeviceName_);
}

HWTEST_F(DmAuthManagerTest, DeleteAcl001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::string localUdid = "localUdid";
    std::string remoteUdid = "remoteUdid";
    std::string extra = "extraTest";
    int32_t sessionId = 0;
    int32_t bindLevel = APP;
    g_bindType = INVALIED_TYPE;
    int32_t ret = authManager_->DeleteAcl(pkgName, localUdid, remoteUdid, bindLevel, extra);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, DeleteAccessControlList001, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    offlineParam.leftAclNumber = 1;
    std::string pkgName = "pkgName";
    std::string localDeviceId = "localDeviceId";
    std::string remoteDeviceId = "remoteDeviceId";
    int32_t bindLevel = APP;
    std::string extra = "";
    auto ret = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(0, ret.leftAclNumber);
}

HWTEST_F(DmAuthManagerTest, DeleteAccessControlList002, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    offlineParam.leftAclNumber = 1;
    std::string pkgName = "pkgName";
    std::string localDeviceId = "localDeviceId";
    std::string remoteDeviceId = "remoteDeviceId";
    int32_t bindLevel = APP;
    std::string extra = "extratest";
    auto ret = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(0, ret.leftAclNumber);
}

HWTEST_F(DmAuthManagerTest, DeleteAccessControlList003, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    offlineParam.leftAclNumber = 1;
    std::string pkgName = "pkgName";
    std::string localDeviceId = "localDeviceId";
    std::string remoteDeviceId = "remoteDeviceId";
    int32_t bindLevel = SERVICE;
    std::string extra = "extratest";
    auto ret = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(0, ret.leftAclNumber);
}

HWTEST_F(DmAuthManagerTest, DeleteAccessControlList004, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    offlineParam.leftAclNumber = 1;
    std::string pkgName = "pkgName";
    std::string localDeviceId = "localDeviceId";
    std::string remoteDeviceId = "remoteDeviceId";
    int32_t bindLevel = USER;
    std::string extra = "";
    auto ret = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(0, ret.leftAclNumber);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation001, testing::ext::TestSize.Level1)
{
    int32_t action = USER_OPERATION_TYPE_AUTH_CONFIRM_TIMEOUT;
    std::string params = "12345";
    g_reportAuthConfirmBoxReturnBoolValue = false;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    g_reportAuthInputPinBoxReturnBoolValue = true;
    int32_t ret = authManager_->OnUserOperation(action, params);
    g_reportAuthInputPinBoxReturnBoolValue = false;
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation002, testing::ext::TestSize.Level1)
{
    int32_t action = USER_OPERATION_TYPE_CANCEL_PINCODE_DISPLAY;
    std::string params = "12345";
    g_reportAuthConfirmBoxReturnBoolValue = false;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    g_reportAuthInputPinBoxReturnBoolValue = true;
    int32_t ret = authManager_->OnUserOperation(action, params);
    g_reportAuthInputPinBoxReturnBoolValue = false;
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation003, testing::ext::TestSize.Level1)
{
    int32_t action = USER_OPERATION_TYPE_CANCEL_PINCODE_INPUT;
    std::string params = "12345";
    g_reportAuthConfirmBoxReturnBoolValue = false;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    g_reportAuthInputPinBoxReturnBoolValue = true;
    int32_t ret = authManager_->OnUserOperation(action, params);
    g_reportAuthInputPinBoxReturnBoolValue = false;
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, OnUserOperation004, testing::ext::TestSize.Level1)
{
    int32_t action = USER_OPERATION_TYPE_DONE_PINCODE_INPUT;
    std::string params = "12345";
    g_reportAuthConfirmBoxReturnBoolValue = false;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    g_reportAuthInputPinBoxReturnBoolValue = true;
    int32_t ret = authManager_->OnUserOperation(action, params);
    g_reportAuthInputPinBoxReturnBoolValue = false;
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ProcRespNegotiateExt001, testing::ext::TestSize.Level1)
{
    int32_t sessionId = 0;
    g_accountId = "test";
    authManager_->authResponseContext_->localAccountId = "test";
    authManager_->authenticationMap_.clear();
    authManager_->authResponseContext_->hostPkgName = "test";
    authManager_->importPkgName_ = "test";
    authManager_->importAuthCode_ = "12345";
    authManager_->ProcRespNegotiateExt(sessionId);
    ASSERT_EQ(authManager_->authResponseContext_->reply, ERR_DM_UNSUPPORTED_AUTH_TYPE);
    g_accountId = "";
}

HWTEST_F(DmAuthManagerTest, ProcRespNegotiateExt002, testing::ext::TestSize.Level1)
{
    authManager_->authenticationMap_.clear();
    authManager_->authResponseContext_->hostPkgName = "test";
    authManager_->authResponseContext_->localAccountId = "test";
    authManager_->importAuthCode_ = "12345";
    authManager_->importPkgName_ = "test";
    g_accountId = "ohosAnonymousUid";
    int32_t sessionId = 0;
    authManager_->ProcRespNegotiateExt(sessionId);
    ASSERT_EQ(authManager_->authResponseContext_->reply, ERR_DM_UNSUPPORTED_AUTH_TYPE);
    g_accountId = "";
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_001, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseContext_->importAuthCode = "importAuthCode";
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->authResponseContext_->isOnline = true;
    authManager_->importAuthCode_ = "";
    g_isIdenticalAccountReturnBoolValue = true;
    int32_t ret = authManager_->CheckTrustState();
    g_isIdenticalAccountReturnBoolValue = false;
    ASSERT_EQ(ret, 1);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_002, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseContext_->importAuthCode = "";
    authManager_->authResponseContext_->isAuthCodeReady = false;
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->reply = ERR_DM_AUTH_PEER_REJECT;
    g_checkSrcDevIdInAclForDevBindReturnBoolValue = false;
    g_isDevicesInP2PGroupReturnBoolValue = true;
    g_isIdenticalAccountReturnBoolValue = false;
    int32_t ret = authManager_->CheckTrustState();
    g_isDevicesInP2PGroupReturnBoolValue = false;
    ASSERT_EQ(ret, 1);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_003, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->authType = AUTH_TYPE_UNKNOW;
    authManager_->authResponseContext_->importAuthCode = "";
    authManager_->authResponseContext_->isAuthCodeReady = false;
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->reply = ERR_DM_AUTH_PEER_REJECT;
    g_checkSrcDevIdInAclForDevBindReturnBoolValue = true;
    g_isDevicesInP2PGroupReturnBoolValue = true;
    g_isIdenticalAccountReturnBoolValue = false;
    int32_t ret = authManager_->CheckTrustState();
    g_isDevicesInP2PGroupReturnBoolValue = false;
    ASSERT_EQ(ret, 1);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_004, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->authType = AUTH_TYPE_UNKNOW;
    authManager_->authResponseContext_->importAuthCode = "";
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->reply = ERR_DM_AUTH_PEER_REJECT;
    g_isDevicesInP2PGroupReturnBoolValue = false;
    g_isIdenticalAccountReturnBoolValue = false;
    int32_t ret = authManager_->CheckTrustState();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_005, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->authType = AUTH_TYPE_UNKNOW;
    authManager_->authResponseContext_->importAuthCode = "";
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->reply = ERR_DM_UNSUPPORTED_AUTH_TYPE;
    g_isDevicesInP2PGroupReturnBoolValue = false;
    g_isIdenticalAccountReturnBoolValue = false;
    int32_t ret = authManager_->CheckTrustState();
    ASSERT_EQ(ret, ERR_DM_BIND_PEER_UNSUPPORTED);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_006, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->authType = AUTH_TYPE_IMPORT_AUTH_CODE;
    authManager_->authResponseContext_->importAuthCode = "";
    authManager_->authResponseContext_->isAuthCodeReady = false;
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->reply = AUTH_TYPE_IMPORT_AUTH_CODE;
    g_isDevicesInP2PGroupReturnBoolValue = false;
    g_isIdenticalAccountReturnBoolValue = false;
    int32_t ret = authManager_->CheckTrustState();
    ASSERT_EQ(ret, ERR_DM_BIND_PEER_UNSUPPORTED);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_007, testing::ext::TestSize.Level1)
{
    authManager_->authResponseContext_->authType = AUTH_TYPE_UNKNOW;
    authManager_->authResponseContext_->importAuthCode = "";
    authManager_->authResponseContext_->isAuthCodeReady = false;
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->reply = AUTH_TYPE_IMPORT_AUTH_CODE;
    g_isDevicesInP2PGroupReturnBoolValue = false;
    g_isIdenticalAccountReturnBoolValue = false;
    int32_t ret = authManager_->CheckTrustState();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, GetCloseSessionDelaySeconds_001, testing::ext::TestSize.Level1)
{
    std::string delaySecondsStr("123jlk");
    int32_t ret = authManager_->GetCloseSessionDelaySeconds(delaySecondsStr);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(DmAuthManagerTest, GetCloseSessionDelaySeconds_002, testing::ext::TestSize.Level1)
{
    std::string delaySecondsStr("123");
    int32_t ret = authManager_->GetCloseSessionDelaySeconds(delaySecondsStr);
    ASSERT_EQ(ret, 0);
}

HWTEST_F(DmAuthManagerTest, GetCloseSessionDelaySeconds_003, testing::ext::TestSize.Level1)
{
    std::string delaySecondsStr("5");
    int32_t ret = authManager_->GetCloseSessionDelaySeconds(delaySecondsStr);
    ASSERT_EQ(ret, 5);
}

HWTEST_F(DmAuthManagerTest, GetCloseSessionDelaySeconds_004, testing::ext::TestSize.Level1)
{
    std::string delaySecondsStr("10");
    int32_t ret = authManager_->GetCloseSessionDelaySeconds(delaySecondsStr);
    ASSERT_EQ(ret, 10);
}

HWTEST_F(DmAuthManagerTest, GetCloseSessionDelaySeconds_005, testing::ext::TestSize.Level1)
{
    std::string delaySecondsStr("0");
    int32_t ret = authManager_->GetCloseSessionDelaySeconds(delaySecondsStr);
    ASSERT_EQ(ret, 0);
}
} // namespace DistributedHardware
} // namespace OHOS
