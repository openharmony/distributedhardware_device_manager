/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_device_manager_impl.h"

#include <memory>
#include <unistd.h>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "softbus_error_code.h"
#include "token_setproc.h"

#include "device_manager_notify.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_publish_req.h"
#include "ipc_req.h"
#include "ipc_rsp.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_skeleton.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_unpublish_req.h"

using namespace testing;
namespace OHOS {
namespace DistributedHardware {
namespace {
HWTEST_F(DeviceManagerImplTest, ImportAuthInfo_001, testing::ext::TestSize.Level1)
{
    DmAuthInfo authInfo;
    authInfo.authType = DMLocalServiceInfoAuthType::CANCEL;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().ImportAuthInfo(authInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, ImportAuthInfo_002, testing::ext::TestSize.Level1)
{
    DmAuthInfo authInfo;
    authInfo.authType = DMLocalServiceInfoAuthType::CANCEL;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().ImportAuthInfo(authInfo);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, ImportAuthInfo_003, testing::ext::TestSize.Level1)
{
    DmAuthInfo authInfo;
    authInfo.authType = DMLocalServiceInfoAuthType::CANCEL;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().ImportAuthInfo(authInfo);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, ExportAuthInfo_001, testing::ext::TestSize.Level1)
{
    DmAuthInfo authInfo;
    authInfo.authType = DMLocalServiceInfoAuthType::CANCEL;
    uint32_t pinLength = 6;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().ExportAuthInfo(authInfo, pinLength);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, ExportAuthInfo_002, testing::ext::TestSize.Level1)
{
    DmAuthInfo authInfo;
    authInfo.authType = DMLocalServiceInfoAuthType::CANCEL;
    uint32_t pinLength = 6;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().ExportAuthInfo(authInfo, pinLength);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, ExportAuthInfo_003, testing::ext::TestSize.Level1)
{
    DmAuthInfo authInfo;
    authInfo.authType = DMLocalServiceInfoAuthType::CANCEL;
    uint32_t pinLength = 6;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().ExportAuthInfo(authInfo, pinLength);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, ConvertLocalServiceInfoToAuthInfo_001, testing::ext::TestSize.Level1)
{
    DMLocalServiceInfo localInfo;
    DmAuthInfo authInfo;
    localInfo.description = "test_description";
    localInfo.pinCode = "123456";
    localInfo.bundleName = "test_bundle";
    localInfo.extraInfo = R"({"userId" : "123", "bizSrcPkgName" : "bizSrcPkgNameTest",
                                "bizSinkPkgName" : "bizSinkPkgNameTest", "metaToken" : "123456"})";
    DeviceManagerImpl::GetInstance().ConvertLocalServiceInfoToAuthInfo(localInfo, authInfo);
    EXPECT_EQ(authInfo.description, localInfo.description);
    EXPECT_EQ(std::string(authInfo.pinCode), localInfo.pinCode);
    EXPECT_EQ(authInfo.pinConsumerPkgName, localInfo.bundleName);
    EXPECT_EQ(authInfo.extraInfo, localInfo.extraInfo);
}

HWTEST_F(DeviceManagerImplTest, ConvertLocalServiceInfoToAuthInfo_002, testing::ext::TestSize.Level1)
{
    DMLocalServiceInfo localInfo;
    DmAuthInfo authInfo;
    localInfo.pinCode = std::string(DM_MAX_PIN_CODE_LEN, 'a');
    DeviceManagerImpl::GetInstance().ConvertLocalServiceInfoToAuthInfo(localInfo, authInfo);
    EXPECT_TRUE(std::string(authInfo.pinCode).empty());
}

HWTEST_F(DeviceManagerImplTest, ConvertLocalServiceInfoToAuthInfo_003, testing::ext::TestSize.Level1)
{
    DMLocalServiceInfo localInfo;
    localInfo.pinCode = "123456";
    localInfo.extraInfo = R"({})";
    DmAuthInfo authInfo;
    DeviceManagerImpl::GetInstance().ConvertLocalServiceInfoToAuthInfo(localInfo, authInfo);
    EXPECT_EQ(std::string(authInfo.pinCode), "123456");
}

HWTEST_F(DeviceManagerImplTest, ConvertLocalServiceInfoToAuthInfo_004, testing::ext::TestSize.Level1)
{
    DMLocalServiceInfo localInfo;
    localInfo.pinCode = "123456";
    localInfo.extraInfo = R"({"pinErrorCount" : "1"})";
    DmAuthInfo authInfo;
    DeviceManagerImpl::GetInstance().ConvertLocalServiceInfoToAuthInfo(localInfo, authInfo);
    EXPECT_EQ(std::string(authInfo.pinCode), "123456");
}

HWTEST_F(DeviceManagerImplTest, ConvertLocalServiceInfoToAuthInfo_005, testing::ext::TestSize.Level1)
{
    DMLocalServiceInfo localInfo;
    DmAuthInfo authInfo;
    localInfo.description = "test_description";
    localInfo.pinCode = "123456";
    localInfo.bundleName = "test_bundle";
    std::string longMetaToken(2025, 'a');
    localInfo.extraInfo = R"({"META_TOKEN": ")" + longMetaToken + R"("})";
    DeviceManagerImpl::GetInstance().ConvertLocalServiceInfoToAuthInfo(localInfo, authInfo);
    EXPECT_EQ(std::string(authInfo.pinCode), "123456");
}

HWTEST_F(DeviceManagerImplTest, RegisterAuthCodeInvalidCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::shared_ptr<AuthCodeInvalidCallback> cb = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterAuthCodeInvalidCallback(pkgName, cb);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterAuthCodeInvalidCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::shared_ptr<AuthCodeInvalidCallback> cb = std::make_shared<AuthCodeInvalidCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterAuthCodeInvalidCallback(pkgName, cb);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterAuthCodeInvalidCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::shared_ptr<AuthCodeInvalidCallback> cb = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterAuthCodeInvalidCallback(pkgName, cb);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterAuthCodeInvalidCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterAuthCodeInvalidCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterAuthCodeInvalidCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterAuthCodeInvalidCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
