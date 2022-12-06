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

#include "UTTest_device_manager_service_impl.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceImplTest::SetUp()
{
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
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();

/**
 * @tc.name: Initialize_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, Initialize_001, testing::ext::TestSize.Level0)
{
    auto ideviceManagerServiceListener = std::shared_ptr<IDeviceManagerServiceListener>();
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>
    ();
    deviceManagerServiceImpl->commonEventManager_ = std::make_shared<DmCommonEventManager>();
    int ret = deviceManagerServiceImpl->Initialize(ideviceManagerServiceListener);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: Initialize_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, Initialize_002, testing::ext::TestSize.Level0)
{
    auto ideviceManagerServiceListener = std::shared_ptr<IDeviceManagerServiceListener>();
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>
    ();
    deviceManagerServiceImpl->commonEventManager_ = nullptr;
    int ret = deviceManagerServiceImpl->Initialize(ideviceManagerServiceListener);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterDevStateCallback_001
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterDevStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string extra;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int32_t ret = deviceManagerServiceImpl->RegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterDevStateCallback_002
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterDevStateCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string extra;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int32_t ret = deviceManagerServiceImpl->RegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterDevStateCallback_003
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterDevStateCallback_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string extra = "extraTest";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->deviceStateMgr_ =
        std::make_shared<DmDeviceStateManager>(softbusConnector, listener, hiChainConnector_);
    int32_t ret = deviceManagerServiceImpl->RegisterDevStateCallback(pkgName, extra);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->PraseNotifyEventJson(event, jsonObject);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->NotifyEvent(pkgName, eventId, event);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->NotifyEvent(pkgName, eventId, event);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int ret = deviceManagerServiceImpl->NotifyEvent(pkgName, eventId, event);
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
    std::string returnJsonStr = "returnTest";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int32_t ret = deviceManagerServiceImpl->RequestCredential(reqJsonStr, returnJsonStr);
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
    std::string returnJsonStr = "returnTest";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->credentialMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl->RequestCredential(reqJsonStr, returnJsonStr);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int32_t ret = deviceManagerServiceImpl->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: ImportCredential_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, ImportCredential_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNameTest";
    const std::string credentialInfo = "credentialInfoTest";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->credentialMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl->ImportCredential(pkgName, credentialInfo);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int32_t ret = deviceManagerServiceImpl->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: DeleteCredential_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, DeleteCredential_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNameTest";
    const std::string deleteInfo = "deleteInfoTest";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->credentialMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl->DeleteCredential(pkgName, deleteInfo);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: RegisterCredentialCallback_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterCredentialCallback_001, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int32_t ret = deviceManagerServiceImpl->RegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: RegisterCredentialCallback_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, RegisterCredentialCallback_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNameTest";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->credentialMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl->RegisterCredentialCallback(pkgName);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int32_t ret = deviceManagerServiceImpl->UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterCredentialCallback_002
 * @tc.desc: return ERR_DM_POINT_NULL
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnRegisterCredentialCallback_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "pkgNameTest";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->credentialMgr_ = nullptr;
    int32_t ret = deviceManagerServiceImpl->UnRegisterCredentialCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: GetFaParam_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, GetFaParam_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "";
    DmAuthParam authParam;
    DmDeviceInfo info;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->softbusConnector_ = std::make_shared<SoftbusConnector>();
    deviceManagerServiceImpl->HandleDeviceOnline(info);
    int32_t ret = deviceManagerServiceImpl->GetFaParam(pkgName, authParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetFaParam_002
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, GetFaParam_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    DmAuthParam authParam;
    DmDeviceInfo info;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->softbusConnector_ = softbusConnector;
    deviceManagerServiceImpl->authMgr_ =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    deviceManagerServiceImpl->HandleDeviceOffline(info);
    int32_t ret = deviceManagerServiceImpl->GetFaParam(pkgName, authParam);
    EXPECT_EQ(ret, DM_OK);
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
    const std::string params = "";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int32_t ret = deviceManagerServiceImpl->SetUserOperation(pkgName, action, params);
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
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->authMgr_ =
        std::make_shared<DmAuthManager>(softbusConnector, listener, hiChainConnector_);
    int32_t ret = deviceManagerServiceImpl->SetUserOperation(pkgName, action, params);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterDevStateCallback_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnRegisterDevStateCallback_001, testing::ext::TestSize.Level0)
{
    const std::string pkgName;
    const std::string extra;
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    int32_t ret = deviceManagerServiceImpl->UnRegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: UnRegisterDevStateCallback_002
 * @tc.desc: return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerServiceImplTest, UnRegisterDevStateCallback_002, testing::ext::TestSize.Level0)
{
    const std::string pkgName = "com.ohos.test";
    const std::string extra = "extraTest";
    auto deviceManagerServiceImpl = std::make_shared<DeviceManagerServiceImpl>();
    deviceManagerServiceImpl->deviceStateMgr_ =
        std::make_shared<DmDeviceStateManager>(softbusConnector, listener, hiChainConnector_);
    int32_t ret = deviceManagerServiceImpl->UnRegisterDevStateCallback(pkgName, extra);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
