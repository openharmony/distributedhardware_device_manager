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

#include "UTTest_device_manager_service_impl.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceImplTest::SetUp()
{
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->Initialize(listener_);
}

void DeviceManagerServiceImplTest::TearDown()
{
}

void DeviceManagerServiceImplTest::SetUpTestCase()
{
}

void DeviceManagerServiceImplTest::TearDownTestCase()
{
}

namespace {

bool CheckSoftbusRes(int32_t ret)
{
    return ret == SOFTBUS_INVALID_PARAM || ret == SOFTBUS_NETWORK_NOT_INIT || ret == SOFTBUS_NETWORK_LOOPER_ERR
        || ret == SOFTBUS_IPC_ERR;
}

/**
 * @tc.name: Initialize_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, Initialize_001, testing::ext::TestSize.Level0)
{
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->commonEventManager_ = std::make_shared<DmCommonEventManager>();
    int ret = deviceManagerServiceImpl_->Initialize(listener_);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: Initialize_002
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, Initialize_002, testing::ext::TestSize.Level0)
{
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->commonEventManager_ = nullptr;
    int ret = deviceManagerServiceImpl_->Initialize(listener_);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: Initialize_003
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, Initialize_003, testing::ext::TestSize.Level0)
{
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->softbusConnector_ = nullptr;
    deviceManagerServiceImpl_->hiChainConnector_ = nullptr;
    deviceManagerServiceImpl_->mineHiChainConnector_ = nullptr;
    deviceManagerServiceImpl_->discoveryMgr_ = nullptr;
    deviceManagerServiceImpl_->publishMgr_ = nullptr;
    deviceManagerServiceImpl_->hiChainAuthConnector_ = nullptr;
    deviceManagerServiceImpl_->deviceStateMgr_ = nullptr;
    deviceManagerServiceImpl_->authMgr_ = nullptr;
    deviceManagerServiceImpl_->credentialMgr_ = nullptr;
    int ret = deviceManagerServiceImpl_->Initialize(listener_);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: PraseNotifyEventJson_001
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_001, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra": {"deviceId": "123"})";
    nlohmann::json jsonObject;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_002
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_002, testing::ext::TestSize.Level0)
{
    std::string event = R"({"content": {"deviceid": "123"}})";
    nlohmann::json jsonObject;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_003
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_003, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra": {"deviceaId": "123"}})";
    nlohmann::json jsonObject;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_004
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_004, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra": {"deviceId": 123}})";
    nlohmann::json jsonObject;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_005
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_005, testing::ext::TestSize.Level0)
{
    std::string event = R"({"Extra": {"deviceId": "123"}})";
    nlohmann::json jsonObject;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_006
 * @tc.desc: return ERR_DM_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_006, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra":"123"}})";
    nlohmann::json jsonObject;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: PraseNotifyEventJson_007
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PraseNotifyEventJson_007, testing::ext::TestSize.Level0)
{
    std::string event = R"({"extra": {"deviceId": "123"}})";
    nlohmann::json jsonObject;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->PraseNotifyEventJson(event, jsonObject);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: NotifyEvent_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, NotifyEvent_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t eventId = DM_NOTIFY_EVENT_START;
    std::string event = R"({"extra": {"deviceId": "123"}})";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: NotifyEvent_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, NotifyEvent_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t eventId = DM_NOTIFY_EVENT_BUTT;
    std::string event = R"({"extra": {"deviceId": "123"}})";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: NotifyEvent_003
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, NotifyEvent_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t eventId = DM_NOTIFY_EVENT_ONDEVICEREADY;
    std::string event = R"({"extra": {"deviceId": "123"})";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RequestCredential_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RequestCredential_001, testing::ext::TestSize.Level0)
{
    const std::string reqJsonStr = "";
    std::string returnJsonStr = "returntest";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->RequestCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RequestCredential_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RequestCredential_002, testing::ext::TestSize.Level0)
{
    const std::string reqJsonStr = "test";
    std::string returnJsonStr = "returntest";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->credentialMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->RequestCredential(reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: ImportCredential_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportCredential_001, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "";
    const std::string credentialInfo = "";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ImportCredential_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportCredential_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    const std::string credentialInfo = "credentialInfotest";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->credentialMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: ImportCredential_003
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportCredential_003, testing::ext::TestSize.Level0)
{
    const std::string pkgName;
    const std::string credentialInfo = "credentialInfotest";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ImportCredential_004
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportCredential_004, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    const std::string credentialInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ImportCredential_005
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportCredential_005, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    std::string reqJsonStr;
    std::string returnJsonStr;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ImportCredential_006
 * @tc.desc: return ERR_DM_HICHAIN_CREDENTIAL_IMPORT_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportCredential_006, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    std::string reqJsonStr = "reqJsonStr";
    std::string returnJsonStr;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_HICHAIN_CREDENTIAL_IMPORT_FAILED);
}

/**
 * @tc.name: DeleteCredential_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, DeleteCredential_001, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "";
    const std::string deleteInfo = "";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DeleteCredential_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, DeleteCredential_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    const std::string deleteInfo = "deleteInfotest";
    deviceManagerServiceImpl_->credentialMgr_ = nullptr;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: DeleteCredential_003
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, DeleteCredential_003, testing::ext::TestSize.Level0)
{
    const std::string pkgName;
    const std::string deleteInfo = "deleteInfotest";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DeleteCredential_004
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, DeleteCredential_004, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    const std::string deleteInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DeleteCredential_005
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, DeleteCredential_005, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    std::string reqJsonStr;
    std::string returnJsonStr;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DeleteCredential_006
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, DeleteCredential_006, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    std::string reqJsonStr = "reqJsonStr";
    std::string returnJsonStr;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterCredentialCallback_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterCredentialCallback_001, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterCredentialCallback_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterCredentialCallback_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    deviceManagerServiceImpl_->credentialMgr_ = nullptr;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: UnRegisterCredentialCallback_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnRegisterCredentialCallback_001, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "";
    int32_t ret = deviceManagerServiceImpl_->UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterCredentialCallback_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnRegisterCredentialCallback_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->credentialMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: RegisterUiStateCallback_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterUiStateCallback_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterUiStateCallback_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->authMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: UnRegisterUiStateCallback_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnRegisterUiStateCallback_001, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterUiStateCallback_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnRegisterUiStateCallback_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNametest";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->authMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: SetUserOperation_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, SetUserOperation_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    int32_t action = 1;
    const std::string params = "params";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->SetUserOperation(pkgName, action, params);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetUserOperation_002
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, SetUserOperation_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t action = 1;
    const std::string params = "paramsTest";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->SetUserOperation(pkgName, action, params);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetUserOperation_003
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, SetUserOperation_003, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t action = 1;
    const std::string params;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->SetUserOperation(pkgName, action, params);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetUserOperation_004
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, SetUserOperation_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t action = 1;
    const std::string params;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->SetUserOperation(pkgName, action, params);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SetUserOperation_005
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, SetUserOperation_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t action = 1;
    const std::string params;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->authMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->SetUserOperation(pkgName, action, params);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: HandleOffline_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, HandleOffline_001, testing::ext::TestSize.Level0)
{
    DmDeviceState devState = DmDeviceState::DEVICE_INFO_READY;
    DmDeviceInfo devInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->HandleOffline(devState, devInfo);
    EXPECT_NE(deviceManagerServiceImpl_->authMgr_, nullptr);
}

/**
 * @tc.name: HandleOnline_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, HandleOnline_001, testing::ext::TestSize.Level0)
{
    DmDeviceState devState = DmDeviceState::DEVICE_INFO_READY;
    DmDeviceInfo devInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->HandleOffline(devState, devInfo);
    EXPECT_NE(deviceManagerServiceImpl_->authMgr_, nullptr);
}

/**
 * @tc.name: HandleDeviceStatusChange_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, HandleDeviceStatusChange_001, testing::ext::TestSize.Level0)
{
    DmDeviceState devState = DmDeviceState::DEVICE_INFO_READY;
    DmDeviceInfo devInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->deviceStateMgr_ = nullptr;
    deviceManagerServiceImpl_->HandleDeviceStatusChange(devState, devInfo);
    EXPECT_EQ(deviceManagerServiceImpl_->deviceStateMgr_, nullptr);
}

/**
 * @tc.name: HandleDeviceStatusChange_002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, HandleDeviceStatusChange_002, testing::ext::TestSize.Level0)
{
    DmDeviceState devState = DmDeviceState::DEVICE_STATE_ONLINE;
    DmDeviceInfo devInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->HandleDeviceStatusChange(devState, devInfo);
    EXPECT_NE(deviceManagerServiceImpl_->deviceStateMgr_, nullptr);
}

/**
 * @tc.name: HandleDeviceStatusChange_003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, HandleDeviceStatusChange_003, testing::ext::TestSize.Level0)
{
    DmDeviceState devState = DmDeviceState::DEVICE_STATE_OFFLINE;
    DmDeviceInfo devInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->HandleDeviceStatusChange(devState, devInfo);
    EXPECT_NE(deviceManagerServiceImpl_->deviceStateMgr_, nullptr);
}

/**
 * @tc.name: HandleDeviceStatusChange_004
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, HandleDeviceStatusChange_004, testing::ext::TestSize.Level0)
{
    DmDeviceState devState = DmDeviceState::DEVICE_INFO_CHANGED;
    DmDeviceInfo devInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->HandleDeviceStatusChange(devState, devInfo);
    EXPECT_NE(deviceManagerServiceImpl_->deviceStateMgr_, nullptr);
}

/**
 * @tc.name: StartDeviceDiscovery_001
 * @tc.desc: return SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, StartDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "com.ohos.test";
    uint16_t subscribeId = 0;
    std::string filterOptions;
    int32_t ret = deviceManagerServiceImpl_->StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_TRUE(CheckSoftbusRes(ret));
}

/**
 * @tc.name: StartDeviceDiscovery_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, StartDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName;
    uint16_t subscribeId = 0;
    std::string filterOptions;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StartDeviceDiscovery_003
 * @tc.desc: return SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, StartDeviceDiscovery_003, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "com.ohos.test";
    DmSubscribeInfo subscribeInfo;
    std::string extra;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_TRUE(CheckSoftbusRes(ret));
}

/**
 * @tc.name: StartDeviceDiscovery_004
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, StartDeviceDiscovery_004, testing::ext::TestSize.Level0)
{
    const std::string pkgName;
    DmSubscribeInfo subscribeInfo;
    std::string extra;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StopDeviceDiscovery_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, StopDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    uint16_t subscribeId = 1;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->StopDeviceDiscovery(pkgName, subscribeId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StopDeviceDiscovery_002
 * @tc.desc: return SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, StopDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    uint16_t subscribeId = 1;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->StopDeviceDiscovery(pkgName, subscribeId);
    EXPECT_TRUE(CheckSoftbusRes(ret));
}

/**
 * @tc.name: PublishDeviceDiscovery_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    DmPublishInfo publishInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: PublishDeviceDiscovery_002
 * @tc.desc: return SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PublishDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DmPublishInfo publishInfo;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->PublishDeviceDiscovery(pkgName, publishInfo);
    EXPECT_TRUE(CheckSoftbusRes(ret));
}

/**
 * @tc.name: UnPublishDeviceDiscovery_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnPublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t publishId = 1;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnPublishDeviceDiscovery(pkgName, publishId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnPublishDeviceDiscovery_002
 * @tc.desc: return SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnPublishDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t publishId = 1;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnPublishDeviceDiscovery(pkgName, publishId);
    EXPECT_TRUE(CheckSoftbusRes(ret));
}

/**
 * @tc.name: AuthenticateDevice_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, AuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t authType = 1;
    std::string deviceId = "deviceId";
    std::string extra;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: AuthenticateDevice_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, AuthenticateDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId;
    std::string extra;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: AuthenticateDevice_003
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, AuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t authType = 1;
    std::string deviceId;
    std::string extra;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: AuthenticateDevice_004
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, AuthenticateDevice_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "deviceId";
    std::string extra;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->AuthenticateDevice(pkgName, authType, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnAuthenticateDevice_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnAuthenticateDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string networkId = "networkId";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnAuthenticateDevice_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnAuthenticateDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnAuthenticateDevice_003
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnAuthenticateDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string networkId;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnAuthenticateDevice(pkgName, networkId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnAuthenticateDevice_004
 * @tc.desc: return SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnAuthenticateDevice_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "networkId";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnAuthenticateDevice(pkgName, networkId);
    EXPECT_TRUE(CheckSoftbusRes(ret));
}

/**
 * @tc.name: BindDevice_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, BindDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 0;
    std::string udidHash;
    std::string bindParam;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->BindDevice(pkgName, authType, udidHash, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: BindDevice_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, BindDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t authType = 0;
    std::string udidHash = "udidHash";
    std::string bindParam;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->BindDevice(pkgName, authType, udidHash, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: BindDevice_003
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, BindDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 0;
    std::string udidHash = "udidHash";
    std::string bindParam;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->BindDevice(pkgName, authType, udidHash, bindParam);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: UnBindDevice_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnBindDevice_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string udidHash;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnBindDevice(pkgName, udidHash);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnBindDevice_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnBindDevice_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string udidHash = "udidHash";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnBindDevice(pkgName, udidHash);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnBindDevice_003
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnBindDevice_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string udidHash = "udidHash";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->UnBindDevice(pkgName, udidHash);
    EXPECT_NE(ret, DM_OK);
}

/**
 * @tc.name: GetUdidHashByNetWorkId_001
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, GetUdidHashByNetWorkId_001, testing::ext::TestSize.Level0)
{
    char *networkId = nullptr;
    std::string deviceId;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->softbusConnector_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->GetUdidHashByNetWorkId(networkId, deviceId);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: GetUdidHashByNetWorkId_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, GetUdidHashByNetWorkId_002, testing::ext::TestSize.Level0)
{
    char *networkId = nullptr;
    std::string deviceId;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->hiChainConnector_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->GetUdidHashByNetWorkId(networkId, deviceId);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: GetUdidHashByNetWorkId_003
 * @tc.desc: return SOFTBUS_IPC_ERR
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, GetUdidHashByNetWorkId_003, testing::ext::TestSize.Level0)
{
    const char *networkId = "networkId";
    std::string deviceId;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->GetUdidHashByNetWorkId(networkId, deviceId);
    EXPECT_TRUE(CheckSoftbusRes(ret));
}

/**
 * @tc.name: Release_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, Release_001, testing::ext::TestSize.Level0)
{
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->Release();
    EXPECT_EQ(deviceManagerServiceImpl_->hiChainConnector_, nullptr);
}

/**
 * @tc.name: OnSessionOpened_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, OnSessionOpened_001, testing::ext::TestSize.Level0)
{
    int sessionId = 1;
    int result = 1;
    std::string data = "15631023";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int ret = deviceManagerServiceImpl_->OnSessionOpened(sessionId, result);
    deviceManagerServiceImpl_->OnBytesReceived(sessionId, data.c_str(), data.size());
    deviceManagerServiceImpl_->OnSessionClosed(sessionId);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: MineRequestCredential_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, MineRequestCredential_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string returnJsonStr;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->MineRequestCredential(pkgName, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: CheckCredential_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, CheckCredential_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    const std::string reqJsonStr;
    std::string returnJsonStr;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: CheckCredential_002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, CheckCredential_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    const std::string reqJsonStr = "reqJsonStr";
    std::string returnJsonStr;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetGroupType_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, GetGroupType_001, testing::ext::TestSize.Level0)
{
    std::vector<DmDeviceInfo> deviceList;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->softbusConnector_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->GetGroupType(deviceList);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: GetGroupType_002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, GetGroupType_002, testing::ext::TestSize.Level0)
{
    std::vector<DmDeviceInfo> deviceList;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->hiChainConnector_ = nullptr;
    int32_t ret = deviceManagerServiceImpl_->GetGroupType(deviceList);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: GetGroupType_003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, GetGroupType_003, testing::ext::TestSize.Level0)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "123456789101112131415",
        .deviceName = "deviceName",
        .deviceTypeId = 1
    };
    std::vector<DmDeviceInfo> deviceList;
    deviceList.push_back(deviceInfo);
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->GetGroupType(deviceList);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/**
 * @tc.name: ImportAuthCode_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string authCode = "authCode";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ImportAuthCode_002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportAuthCode_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string authCode;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ImportAuthCode_003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportAuthCode_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string authCode = "authCode";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: ExportAuthCode_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ExportAuthCode_001, testing::ext::TestSize.Level0)
{
    std::string authCode = "authCode";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->ExportAuthCode(authCode);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: BindTarget_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, BindTarget_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: PutIdenticalAccountToAcl_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, PutIdenticalAccountToAcl_001, testing::ext::TestSize.Level0)
{
    std::string requestDeviceId;
    std::string trustDeviceId;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->PutIdenticalAccountToAcl(requestDeviceId, trustDeviceId);
    EXPECT_NE(deviceManagerServiceImpl_->hiChainConnector_, nullptr);
}

/**
 * @tc.name: DpAclAdd_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, DpAclAdd_001, testing::ext::TestSize.Level0)
{
    std::string udid = "2342154";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->DpAclAdd(udid);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: IsSameAccount_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, IsSameAccount_001, testing::ext::TestSize.Level0)
{
    std::string udid;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: IsSameAccount_002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, IsSameAccount_002, testing::ext::TestSize.Level0)
{
    std::string udid = "2342154";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    int32_t ret = deviceManagerServiceImpl_->IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetAppTrustDeviceIdList_003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, GetAppTrustDeviceIdList_003, testing::ext::TestSize.Level0)
{
    std::string pkgname = "pkgname";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    auto ret = deviceManagerServiceImpl_->GetAppTrustDeviceIdList(pkgname);
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: OnUnbindSessionCloseed_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, OnUnbindSessionCloseed_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->OnUnbindSessionCloseed(socket);
    EXPECT_NE(deviceManagerServiceImpl_->hiChainConnector_, nullptr);
}

/**
 * @tc.name: OnUnbindBytesReceived_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, OnUnbindBytesReceived_001, testing::ext::TestSize.Level0)
{
    int32_t socket = 1;
    std::string data = "15135135";
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->OnUnbindBytesReceived(socket, data.c_str(), data.size());
    EXPECT_NE(deviceManagerServiceImpl_->hiChainConnector_, nullptr);
}

/**
 * @tc.name: LoadHardwareFwkService_001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, LoadHardwareFwkService_001, testing::ext::TestSize.Level0)
{
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->LoadHardwareFwkService();
    EXPECT_NE(deviceManagerServiceImpl_->hiChainConnector_, nullptr);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
