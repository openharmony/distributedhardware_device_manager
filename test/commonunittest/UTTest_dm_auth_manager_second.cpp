/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "UTTest_dm_auth_manager_second.h"

#include "nlohmann/json.hpp"

#include "dm_log.h"
#include "dm_constants.h"
#include "dm_auth_manager.h"
#include "auth_message_processor.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
void DmAuthManagerTest::SetUp()
{
    authManager_->authMessageProcessor_ = std::make_shared<AuthMessageProcessor>(authManager_);
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authRequestContext_ = std::make_shared<DmAuthRequestContext>();
    authManager_->authRequestState_ = std::make_shared<AuthRequestFinishState>();
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->hiChainAuthConnector_ = std::make_shared<HiChainAuthConnector>();
    authManager_->softbusConnector_ = std::make_shared<SoftbusConnector>();
    authManager_->softbusConnector_->GetSoftbusSession()->
        RegisterSessionCallback(std::shared_ptr<ISoftbusSessionCallback>(authManager_));
    authManager_->timer_ = std::make_shared<DmTimer>();
}
void DmAuthManagerTest::TearDown()
{
}
void DmAuthManagerTest::SetUpTestCase()
{
}
void DmAuthManagerTest::TearDownTestCase()
{
}

namespace {
HWTEST_F(DmAuthManagerTest, OnDataReceived_001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    std::string message;
    authManager_->authResponseContext_ = nullptr;
    authManager_->OnDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_002, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    std::string message;
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authMessageProcessor_ = nullptr;
    authManager_->OnDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_003, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    std::string message;
    authManager_->OnDataReceived(sessionId, message);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_004, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_MSG_TYPE] = 200;
    std::string message = jsonObject.dump();
    authManager_->OnDataReceived(sessionId, message);
    authManager_->authRequestState_ = std::make_shared<AuthRequestSyncDeleteAcl>();
    authManager_->authResponseState_ = nullptr;
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_005, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_MSG_TYPE] = 200;
    std::string message = jsonObject.dump();
    authManager_->OnDataReceived(sessionId, message);
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseSyncDeleteAclNone>();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnDataReceived_007, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 0;
    nlohmann::json jsonObject;
    jsonObject[TAG_MSG_TYPE] = 200;
    std::string message = jsonObject.dump();
    authManager_->OnDataReceived(sessionId, message);
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId;
    authManager_->authResponseContext_ = nullptr;
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId;
    authManager_->authResponseState_ = nullptr;
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_003, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId = "{}";
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_004, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId = "groupId";
    authManager_->authResponseContext_->isShowDialog = true;
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnGroupCreated_005, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    std::string groupId = "groupId";
    authManager_->authResponseContext_->isShowDialog = false;
    authManager_->OnGroupCreated(requestId, groupId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 11;
    authManager_->authResponseContext_ = nullptr;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 11;
    authManager_->authUiStateMgr_ = nullptr;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_003, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 11;
    authManager_->authRequestState_ = std::make_shared<AuthRequestJoinState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = 5;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_004, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 11;
    authManager_->authRequestState_ = std::make_shared<AuthRequestJoinState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->requestId = 1;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_006, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 0;
    authManager_->authRequestState_ = std::make_shared<AuthRequestJoinState>();
    authManager_->authResponseState_ = nullptr;
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_007, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 0;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = std::make_shared<AuthResponseShowState>();
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, OnMemberJoin_008, testing::ext::TestSize.Level0)
{
    int64_t requestId = 0;
    int32_t status = 0;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    authManager_->OnMemberJoin(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, HandleMemberJoinImportAuthCode_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    int32_t status = 0;
    authManager_->authResponseContext_->requestId = 1;
    authManager_->HandleMemberJoinImportAuthCode(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, HandleMemberJoinImportAuthCode_002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    int32_t status = 1;
    authManager_->authResponseContext_->requestId = 1;
    authManager_->HandleMemberJoinImportAuthCode(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, HandleMemberJoinImportAuthCode_003, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    int32_t status = 1;
    authManager_->authResponseContext_->requestId = 0;
    authManager_->HandleMemberJoinImportAuthCode(requestId, status);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, RespNegotiate_001, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    authManager_->authResponseContext_ = nullptr;
    authManager_->RespNegotiate(requestId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, RespNegotiate_002, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    authManager_->RespNegotiate(requestId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, RespNegotiate_003, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->RespNegotiate(requestId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, RespNegotiate_004, testing::ext::TestSize.Level0)
{
    int64_t requestId = 1;
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseContext_->dmVersion = "";
    authManager_->authResponseContext_->bindLevel = 0;
    authManager_->RespNegotiate(requestId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, SendAuthRequest_001, testing::ext::TestSize.Level0)
{
    int64_t sessionId = 1;
    authManager_->authResponseContext_ = nullptr;
    authManager_->SendAuthRequest(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, SendAuthRequest_002, testing::ext::TestSize.Level0)
{
    int64_t sessionId = 1;
    authManager_->authResponseContext_->cryptoSupport = true;
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->localDeviceId = "123456789";
    authManager_->authResponseContext_->deviceId = "deviceId";
    authManager_->authResponseContext_->reply = -20018;
    authManager_->SendAuthRequest(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, SendAuthRequest_003, testing::ext::TestSize.Level0)
{
    int64_t sessionId = 1;
    authManager_->authResponseContext_->cryptoSupport = true;
    authManager_->authResponseContext_->bindLevel = 0;
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->localDeviceId = "123456789";
    authManager_->authResponseContext_->deviceId = "deviceId";
    authManager_->authResponseContext_->reply = -20018;
    authManager_->SendAuthRequest(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthRequestContext_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->localDeviceId = "123456789";
    authManager_->authResponseContext_->localDeviceId = "123456789";
    authManager_->authResponseContext_->localAccountId = "123456789";
    authManager_->authRequestContext_->remoteAccountId = "123456789";
    authManager_->authRequestContext_->localAccountId = "123456789";
    authManager_->authResponseContext_->localUserId = 123456789;
    authManager_->GetAuthRequestContext();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessAuthRequestExt_001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    authManager_->authResponseContext_->reply = -20018;
    authManager_->ProcessAuthRequestExt(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessAuthRequestExt_002, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    authManager_->authResponseContext_->reply = -200;
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->authed = false;
    authManager_->authResponseContext_->isIdenticalAccount = true;
    authManager_->ProcessAuthRequestExt(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ProcessAuthRequestExt_004, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 1;
    authManager_->authResponseContext_->reply = -200;
    authManager_->authResponseContext_->authType = 5;
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->isAuthCodeReady = false;
    authManager_->ProcessAuthRequestExt(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcess_001, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 0;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    int32_t ret = authManager_->ConfirmProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcess_002, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 6;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    int32_t ret = authManager_->ConfirmProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcess_003, testing::ext::TestSize.Level0)
{
    int32_t action = 3;
    authManager_->action_ = 4;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    int32_t ret = authManager_->ConfirmProcess(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcessExt_001, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 0;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->authResponseContext_->isShowDialog = false;
    int32_t ret = authManager_->ConfirmProcessExt(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcessExt_002, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 6;
    authManager_->authResponseState_ = std::make_shared<AuthResponseConfirmState>();
    authManager_->authResponseContext_->isShowDialog = true;
    int32_t ret = authManager_->ConfirmProcessExt(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ConfirmProcessExt_003, testing::ext::TestSize.Level0)
{
    int32_t action = 1;
    authManager_->action_ = 4;
    int32_t ret = authManager_->ConfirmProcessExt(action);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->groupName = "111111";
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->groupName = "00000";
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->groupName = "00000";
    authManager_->authResponseContext_->reply = 0;
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, StartRespAuthProcess_005, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->groupName = "11111";
    authManager_->authResponseContext_->reply = 1;
    authManager_->StartRespAuthProcess();
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetConnectAddr_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    auto ret = authManager_->GetConnectAddr(deviceId);
    ASSERT_EQ(ret.empty(), true);
}

HWTEST_F(DmAuthManagerTest, GetConnectAddr_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    infoPtr->addrNum = 1;
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.insert(std::pair<std::string,
        std::shared_ptr<DeviceInfo>>(deviceId, infoPtr));
    auto ret = authManager_->GetConnectAddr(deviceId);
    ASSERT_EQ(ret.empty(), false);
}

HWTEST_F(DmAuthManagerTest, SinkAuthenticateFinish_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    authManager_->authPtr_ = authManager_->authenticationMap_[1];
    authManager_->isFinishOfLocal_ = true;
    authManager_->SinkAuthenticateFinish();
    ASSERT_EQ(authManager_->authResponseState_, nullptr);
}

HWTEST_F(DmAuthManagerTest, SrcAuthenticateFinish_001, testing::ext::TestSize.Level0)
{
    authManager_->isFinishOfLocal_ = true;
    authManager_->authResponseContext_->state = 5;
    authManager_->authPtr_ = authManager_->authenticationMap_[1];
    authManager_->SrcAuthenticateFinish();
    usleep(600);
    ASSERT_EQ(authManager_->authRequestState_, nullptr);
}

HWTEST_F(DmAuthManagerTest, SrcAuthenticateFinish_002, testing::ext::TestSize.Level0)
{
    authManager_->isFinishOfLocal_ = false;
    authManager_->authResponseContext_->state = 7;
    authManager_->authPtr_ = authManager_->authenticationMap_[1];
    authManager_->SrcAuthenticateFinish();
    usleep(600);
    ASSERT_EQ(authManager_->authRequestState_, nullptr);
}

HWTEST_F(DmAuthManagerTest, AuthenticateFinish_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, AuthenticateFinish_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->authUiStateMgr_ = nullptr;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authUiStateMgr_, nullptr);
}

HWTEST_F(DmAuthManagerTest, AuthenticateFinish_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authResponseState_ = std::make_shared<AuthResponseFinishState>();
    authManager_->authRequestState_ = nullptr;
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, AuthenticateFinish_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authResponseState_ = nullptr;
    authManager_->authRequestState_ = std::make_shared<AuthRequestJoinState>();
    authManager_->AuthenticateFinish();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, RegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    authManager_->authUiStateMgr_ = nullptr;
    int32_t ret = authManager_->RegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, RegisterUiStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    int32_t ret = authManager_->RegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, UnRegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    authManager_->authUiStateMgr_ = nullptr;
    int32_t ret = authManager_->UnRegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, UnRegisterUiStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    int32_t ret = authManager_->UnRegisterUiStateCallback(pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, SetAuthRequestState_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthRequestState> authRequestState = nullptr;
    int32_t ret = authManager_->SetAuthRequestState(authRequestState);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, SetAuthRequestState_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<AuthRequestState> authRequestState = std::make_shared<AuthRequestFinishState>();
    int32_t ret = authManager_->SetAuthRequestState(authRequestState);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ShowConfigDialog_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowConfigDialog();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, ShowConfigDialog_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isShowDialog = false;
    authManager_->ShowConfigDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, false);
}

HWTEST_F(DmAuthManagerTest, ShowConfigDialog_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isShowDialog = true;
    authManager_->ShowConfigDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, true);
}

HWTEST_F(DmAuthManagerTest, ShowAuthInfoDialog_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowAuthInfoDialog();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, ShowAuthInfoDialog_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isShowDialog = false;
    authManager_->ShowAuthInfoDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, false);
}

HWTEST_F(DmAuthManagerTest, ShowAuthInfoDialog_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isShowDialog = true;
    authManager_->ShowAuthInfoDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, true);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_ = nullptr;
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_, nullptr);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->authType = 5;
    authManager_->importAuthCode_ = "";
    authManager_->importPkgName_ = "";
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_->isShowDialog, false);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->authType = 5;
    authManager_->importAuthCode_ = "123456";
    authManager_->importPkgName_ = "hostPkgName";
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_->bindLevel, 1);
}

HWTEST_F(DmAuthManagerTest, ShowStartAuthDialog_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->authType = 5;
    authManager_->importAuthCode_ = "123456";
    authManager_->importPkgName_ = "hostPkgName";
    authManager_->authResponseContext_->hostPkgName = "hostPkgName";
    authManager_->authResponseContext_->dmVersion = "";
    authManager_->authResponseContext_->bindLevel = 0;
    authManager_->ShowStartAuthDialog();
    ASSERT_EQ(authManager_->authResponseContext_->bindLevel, 0);
}

HWTEST_F(DmAuthManagerTest, ProcessPincode_001, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->authResponseContext_ = nullptr;
    int32_t ret = authManager_->ProcessPincode(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, ProcessPincode_002, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->authResponseContext_->dmVersion = "dmVersion";
    authManager_->authResponseContext_->bindLevel = 1;
    authManager_->isAuthDevice_ = true;
    int32_t ret = authManager_->ProcessPincode(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, ProcessPincode_003, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->authResponseContext_->dmVersion = "";
    authManager_->authResponseContext_->bindLevel = 0;
    authManager_->authUiStateMgr_ = nullptr;
    int32_t ret = authManager_->ProcessPincode(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, AuthDevice_001, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->isAuthDevice_ = true;
    int32_t ret = authManager_->AuthDevice(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, AuthDevice_002, testing::ext::TestSize.Level0)
{
    int32_t pinCode = 123456;
    authManager_->isAuthDevice_ = false;
    authManager_->authResponseContext_->authType = 5;
    int32_t ret = authManager_->AuthDevice(pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, ImportAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string authCode = "";
    std::string pkgName = "pkgName";
    int32_t ret = authManager_->ImportAuthCode(authCode, pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ImportAuthCode_002, testing::ext::TestSize.Level0)
{
    std::string authCode = "123456";
    std::string pkgName = "";
    int32_t ret = authManager_->ImportAuthCode(authCode, pkgName);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ImportAuthCode_003, testing::ext::TestSize.Level0)
{
    std::string authCode = "123456";
    std::string pkgName = "pkgName";
    int32_t ret = authManager_->ImportAuthCode(authCode, pkgName);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, BindTarget_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = authManager_->BindTarget(pkgName, targetId, bindParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, BindTarget_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = authManager_->BindTarget(pkgName, targetId, bindParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, BindTarget_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    targetId.deviceId = "123456";
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "1"));
    bindParam.insert(std::pair<std::string, std::string>("CONN_ADDR_TYPE", "3"));
    int32_t ret = authManager_->BindTarget(pkgName, targetId, bindParam);
    ASSERT_EQ(ret, ERR_DM_AUTH_BUSINESS_BUSY);
}

HWTEST_F(DmAuthManagerTest, BindTarget_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    targetId.deviceId = "";
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "2"));
    bindParam.insert(std::pair<std::string, std::string>("CONN_ADDR_TYPE", "3"));
    int32_t ret = authManager_->BindTarget(pkgName, targetId, bindParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseConnectAddr_001, testing::ext::TestSize.Level0)
{
    PeerTargetId targetId;
    targetId.wifiIp = "12131415";
    targetId.brMac = "D4G4E5G2B4G";
    targetId.bleMac = "1463158131321";
    std::string deviceId;
    std::string addrType = "123456";
    int32_t ret = authManager_->ParseConnectAddr(targetId, deviceId, addrType);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ParseConnectAddr_002, testing::ext::TestSize.Level0)
{
    PeerTargetId targetId;
    std::string deviceId;
    std::string addrType;
    int32_t ret = authManager_->ParseConnectAddr(targetId, deviceId, addrType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_001, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_002, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", ""));
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_003, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "12"));
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_004, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "S"));
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, ParseAuthType_005, testing::ext::TestSize.Level0)
{
    int32_t authType = 0;
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("AUTH_TYPE", "1"));
    int32_t ret = authManager_->ParseAuthType(bindParam, authType);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, ParseExtraFromMap_001, testing::ext::TestSize.Level0)
{
    std::map<std::string, std::string> bindParam;
    bindParam.insert(std::pair<std::string, std::string>("BIND_EXTRA_DATA", "12"));
    std::string ret = authManager_->ParseExtraFromMap(bindParam);
    ASSERT_EQ(ret, "12");
}

HWTEST_F(DmAuthManagerTest, ParseExtraFromMap_002, testing::ext::TestSize.Level0)
{
    std::map<std::string, std::string> bindParam;
    std::string ret = authManager_->ParseExtraFromMap(bindParam);
    ASSERT_EQ(ret, "");
}

HWTEST_F(DmAuthManagerTest, IsAuthCodeReady_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    authManager_->importAuthCode_ = "";
    authManager_->importPkgName_ = "importPkgName_";
    bool ret = authManager_->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, IsAuthCodeReady_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "";
    bool ret = authManager_->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, IsAuthCodeReady_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "importPkgName_";
    bool ret = authManager_->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, IsAuthCodeReady_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "ohos_test";
    bool ret = authManager_->IsAuthCodeReady(pkgName);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DmAuthManagerTest, DeleteAuthCode_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->authType = 5;
    int32_t ret = authManager_->DeleteAuthCode();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, GetAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t pinCode = 1;
    authManager_->importAuthCode_ = "";
    authManager_->importPkgName_ = "importPkgName_";
    int32_t ret = authManager_->GetAuthCode(pkgName, pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, GetAuthCode_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t pinCode = 1;
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "";
    int32_t ret = authManager_->GetAuthCode(pkgName, pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, GetAuthCode_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t pinCode = 1;
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->importPkgName_ = "importPkgName_";
    int32_t ret = authManager_->GetAuthCode(pkgName, pinCode);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DmAuthManagerTest, GetAuthCode_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    int32_t pinCode = 1;
    authManager_->importAuthCode_ = "123456";
    authManager_->importPkgName_ = "ohos_test";
    int32_t ret = authManager_->GetAuthCode(pkgName, pinCode);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, IsAuthTypeSupported_001, testing::ext::TestSize.Level0)
{
    int32_t authType = 7;
    bool ret = authManager_->IsAuthTypeSupported(authType);
    ASSERT_EQ(ret, false);
}

HWTEST_F(DmAuthManagerTest, IsAuthTypeSupported_002, testing::ext::TestSize.Level0)
{
    int32_t authType = 7;
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    bool ret = authManager_->IsAuthTypeSupported(authType);
    ASSERT_EQ(ret, true);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_001, testing::ext::TestSize.Level0)
{
    int32_t authType = -1;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_FAILED);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_002, testing::ext::TestSize.Level0)
{
    int32_t authType = 6;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_FAILED);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_003, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName;
    std::string deviceId = "512156";
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_004, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId;
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_005, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->listener_ = nullptr;
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_006, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->authUiStateMgr_ = nullptr;
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_007, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.clear();
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_UNSUPPORTED_AUTH_TYPE);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_008, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    authManager_->authRequestState_ = nullptr;
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_BUSINESS_BUSY);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_009, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    authManager_->authRequestState_ = std::make_shared<AuthRequestInitState>();
    authManager_->authResponseState_ = nullptr;
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_BUSINESS_BUSY);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_010, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    authManager_->authRequestState_ = std::make_shared<AuthRequestInitState>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseInitState>();
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_BUSINESS_BUSY);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_0011, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->importPkgName_ = "ohos_test";
    authManager_->importAuthCode_ = "156161";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    authManager_->authRequestState_ = std::make_shared<AuthRequestInitState>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseInitState>();
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.clear();
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_BUSINESS_BUSY);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_0012, testing::ext::TestSize.Level0)
{
    int32_t authType = 5;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->importPkgName_ = "";
    authManager_->importAuthCode_ = "";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.emplace(deviceId, infoPtr);
    authManager_->authRequestState_ = std::make_shared<AuthRequestInitState>();
    authManager_->authResponseState_ = std::make_shared<AuthResponseInitState>();
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_AUTH_BUSINESS_BUSY);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_0013, testing::ext::TestSize.Level0)
{
    int32_t authType = 5;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->importPkgName_ = "ohos_test";
    authManager_->importAuthCode_ = "156161";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.clear();
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_0014, testing::ext::TestSize.Level0)
{
    int32_t authType = 4;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->importAuthCode_ = "";
    authManager_->importAuthCode_ = "";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.emplace(deviceId, infoPtr);
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, CheckAuthParamVaild_0015, testing::ext::TestSize.Level0)
{
    int32_t authType = 5;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->importAuthCode_ = "";
    authManager_->importAuthCode_ = "";
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    authManager_->authUiStateMgr_ = std::make_shared<AuthUiStateManager>(listener);
    authManager_->authenticationMap_.insert(std::pair<int32_t, std::shared_ptr<IAuthentication>>(authType, nullptr));
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    authManager_->softbusConnector_->discoveryDeviceInfoMap_.emplace(deviceId, infoPtr);
    authManager_->authRequestState_ = nullptr;
    authManager_->authResponseState_ = nullptr;
    int32_t ret = authManager_->CheckAuthParamVaild(pkgName, authType, deviceId, extra);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmAuthManagerTest, StartNegotiate_001, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 123;
    authManager_->authMessageProcessor_->authResponseContext_ = std::make_shared<DmAuthResponseContext>();
    authManager_->StartNegotiate(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, StartNegotiate_002, testing::ext::TestSize.Level0)
{
    int32_t sessionId = 123;
    authManager_->authResponseContext_ = nullptr;
    authManager_->StartNegotiate(sessionId);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthParam_001, testing::ext::TestSize.Level0)
{
    int32_t authType = 5;
    std::string extra;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->GetAuthParam(pkgName, authType, deviceId, extra);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthParam_002, testing::ext::TestSize.Level0)
{
    nlohmann::json jsonObject;
    jsonObject["targetPkgName"] = 1234;
    std::string extra = jsonObject.dump();
    int32_t authType = 5;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->GetAuthParam(pkgName, authType, deviceId, extra);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthParam_003, testing::ext::TestSize.Level0)
{
    nlohmann::json jsonObject;
    jsonObject["targetPkgName"] = "1234";
    jsonObject["appOperation"] = 1234;
    std::string extra = jsonObject.dump();
    int32_t authType = 5;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->GetAuthParam(pkgName, authType, deviceId, extra);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthParam_004, testing::ext::TestSize.Level0)
{
    nlohmann::json jsonObject;
    jsonObject["targetPkgName"] = "1234";
    jsonObject["appOperation"] = "1234";
    jsonObject["customDescription"] = 1234;
    std::string extra = jsonObject.dump();
    int32_t authType = 5;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->GetAuthParam(pkgName, authType, deviceId, extra);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthParam_005, testing::ext::TestSize.Level0)
{
    nlohmann::json jsonObject;
    jsonObject["targetPkgName"] = "1234";
    jsonObject["appOperation"] = "1234";
    jsonObject["customDescription"] = "1234";
    jsonObject["appThumbnail"] = 1234;
    std::string extra = jsonObject.dump();
    int32_t authType = 5;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->GetAuthParam(pkgName, authType, deviceId, extra);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthParam_006, testing::ext::TestSize.Level0)
{
    nlohmann::json jsonObject;
    jsonObject["targetPkgName"] = "1234";
    jsonObject["appOperation"] = "1234";
    jsonObject["customDescription"] = "1234";
    jsonObject["appThumbnail"] = "1234";
    jsonObject["tokenId"] = "1234";
    std::string extra = jsonObject.dump();
    int32_t authType = 5;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->GetAuthParam(pkgName, authType, deviceId, extra);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthParam_007, testing::ext::TestSize.Level0)
{
    nlohmann::json jsonObject;
    jsonObject["targetPkgName"] = "1234";
    jsonObject["appOperation"] = "1234";
    jsonObject["customDescription"] = "1234";
    jsonObject["appThumbnail"] = "1234";
    jsonObject["tokenId"] = 1234;
    jsonObject["bindLevel"] = "1234";
    std::string extra = jsonObject.dump();
    int32_t authType = 5;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->GetAuthParam(pkgName, authType, deviceId, extra);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, GetAuthParam_008, testing::ext::TestSize.Level0)
{
    nlohmann::json jsonObject;
    jsonObject["targetPkgName"] = "1234";
    jsonObject["appOperation"] = "1234";
    jsonObject["customDescription"] = "1234";
    jsonObject["appThumbnail"] = "1234";
    jsonObject["tokenId"] = 1234;
    jsonObject["bindLevel"] = 1234;
    std::string extra = jsonObject.dump();
    int32_t authType = 5;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->GetAuthParam(pkgName, authType, deviceId, extra);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, InitAuthState_001, testing::ext::TestSize.Level0)
{
    std::string extra;
    int32_t authType = 5;
    std::string pkgName = "ohos_test";
    std::string deviceId = "512156";
    authManager_->timer_ = nullptr;
    authManager_->InitAuthState(pkgName, authType, deviceId, extra);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, HandleSessionHeartbeat_001, testing::ext::TestSize.Level0)
{
    std::string name;
    authManager_->authRequestState_ = std::make_shared<AuthRequestInitState>();
    authManager_->HandleSessionHeartbeat(name);
    ASSERT_EQ(authManager_->isFinishOfLocal_, true);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_001, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isOnline = true;
    authManager_->authResponseContext_->authType = 5;
    authManager_->authResponseContext_->importAuthCode = "";
    authManager_->importAuthCode_ = "";
    int32_t ret = authManager_->CheckTrustState();
    ASSERT_NE(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_002, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->importAuthCode = "importAuthCode";
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->authResponseContext_->isIdenticalAccount = true;
    int32_t ret = authManager_->CheckTrustState();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_003, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->importAuthCode = "importAuthCode";
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->reply = ERR_DM_AUTH_PEER_REJECT;
    int32_t ret = authManager_->CheckTrustState();
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_004, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->importAuthCode = "importAuthCode";
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->reply = ERR_DM_UNSUPPORTED_AUTH_TYPE;
    authManager_->authResponseContext_->authType = 5;
    authManager_->authResponseContext_->isAuthCodeReady = false;
    int32_t ret = authManager_->CheckTrustState();
    ASSERT_EQ(ret, ERR_DM_BIND_PEER_UNSUPPORTED);
}

HWTEST_F(DmAuthManagerTest, CheckTrustState_005, testing::ext::TestSize.Level0)
{
    authManager_->authResponseContext_->isOnline = false;
    authManager_->authResponseContext_->authType = 6;
    authManager_->authResponseContext_->importAuthCode = "importAuthCode";
    authManager_->importAuthCode_ = "importAuthCode_";
    authManager_->authResponseContext_->isIdenticalAccount = false;
    authManager_->authResponseContext_->reply = 1;
    authManager_->authResponseContext_->authType = 1;
    authManager_->authResponseContext_->isAuthCodeReady = true;
    int32_t ret = authManager_->CheckTrustState();
    ASSERT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
