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
#include "dm_device_info.h"
#include <map>
#include "ipc_get_local_serviceinfo_ext_rsp.h"
#include "ipc_get_peerserviceinfo_by_serviceid_req.h"
#include "ipc_get_trust_service_info_rsp.h"
#include <memory>
#include <unistd.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "accesstoken_kit.h"
#include "device_manager_notify.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_req.h"
#include "ipc_rsp.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_skeleton.h"
#include "ipc_publish_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_sync_service_callback_req.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"
#include "softbus_error_code.h"
#include "ipc_publish_service_info_rsp.h"
#include "ipc_sync_service_callback_req.h"
#include "ipc_start_service_discovery_req.h"
#include "ipc_get_local_serviceinfo_ext_rsp.h"
#include "ipc_get_peerserviceinfo_by_serviceid_req.h"
#include "ipc_get_register_serviceinfo_rsp.h"
#include "ipc_notify_dmfa_result_req.h"
#include "ipc_get_trust_service_info_rsp.h"

using namespace testing;
namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DM_STRING_LENGTH_MAX = 1024;
void DeviceManagerImplTest::SetUp()
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

void DeviceManagerImplTest::TearDown()
{
    testing::Mock::VerifyAndClearExpectations(deviceManagerNotifyMock_.get());
    testing::Mock::VerifyAndClearExpectations(ipcClientProxyMock_.get());
}

void DeviceManagerImplTest::SetUpTestCase()
{
    DmDeviceManagerNotify::dmDeviceManagerNotify = deviceManagerNotifyMock_;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = ipcClientProxyMock_;
}

void DeviceManagerImplTest::TearDownTestCase()
{
    DmDeviceManagerNotify::dmDeviceManagerNotify = nullptr;
    deviceManagerNotifyMock_ = nullptr;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
    ipcClientProxyMock_ = nullptr;
}

namespace {
HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> unbindParam;
    std::string netWorkId = "test_network_id";
    int64_t serviceId = 1001;
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget(pkgName, unbindParam, netWorkId, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> unbindParam;
    std::string netWorkId = "test_network_id";
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget(pkgName, unbindParam, netWorkId, serviceId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> unbindParam;
    std::string netWorkId = "test_network_id";
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget(pkgName, unbindParam, netWorkId, serviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> unbindParam;
    std::string netWorkId = "test_network_id";
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget(pkgName, unbindParam, netWorkId, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnbindServiceTarget_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> unbindParam = {{"key", "value"}};
    std::string netWorkId = "test_network_id";
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnbindServiceTarget(pkgName, unbindParam, netWorkId, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int64_t serviceId = 1001;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = -1;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterServiceStateCallback(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(DM_OK));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterServiceStateCallback(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterServiceStateCallback(testing::_, testing::_, testing::_))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceStateCallback_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    std::shared_ptr<ServiceInfoStateCallback> callback = std::make_shared<ServiceInfoStateCallbackTest>();
    EXPECT_CALL(*deviceManagerNotifyMock_, RegisterServiceStateCallback(testing::_, testing::_, testing::_))
        .WillOnce(Return(DM_OK));
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceStateCallback(pkgName, serviceId, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int64_t serviceId = 1001;
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = -1;
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServiceStateCallback(testing::_, testing::_))
        .WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServiceStateCallback(testing::_, testing::_))
        .WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceStateCallback_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServiceStateCallback(testing::_, testing::_))
        .WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceStateCallback(pkgName, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.serviceType = "TestType";
    regServiceInfo.serviceName = "TestService";
    regServiceInfo.serviceDisplayName = "Test Service Display";
    int64_t serviceId = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(regServiceInfo, serviceId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_002, testing::ext::TestSize.Level1)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.serviceType = "TestType";
    regServiceInfo.serviceName = "TestService";
    regServiceInfo.serviceDisplayName = "Test Service Display";
    int64_t serviceId = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(regServiceInfo, serviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_003, testing::ext::TestSize.Level1)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.serviceType = "TestType";
    regServiceInfo.serviceName = "TestService";
    regServiceInfo.serviceDisplayName = "Test Service Display";
    int64_t serviceId = 0;
    int64_t expectedServiceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([expectedServiceId](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
                std::shared_ptr<IpcPublishServiceInfoRsp> publishRsp =
                    std::static_pointer_cast<IpcPublishServiceInfoRsp>(rsp);
                publishRsp->SetServiceId(expectedServiceId);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(regServiceInfo, serviceId);
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceId, expectedServiceId);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_004, testing::ext::TestSize.Level1)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.serviceType = "TestType";
    regServiceInfo.serviceName = "TestService";
    regServiceInfo.serviceDisplayName = "Test Service Display";
    int64_t serviceId = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
                std::shared_ptr<IpcPublishServiceInfoRsp> publishRsp =
                    std::static_pointer_cast<IpcPublishServiceInfoRsp>(rsp);
                publishRsp->SetServiceId(0);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(regServiceInfo, serviceId);
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceId, 0);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceInfo_001, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(serviceId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceInfo_002, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(serviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceInfo_003, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceInfo_004, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceInfo_005, testing::ext::TestSize.Level1)
{
    int64_t serviceId = -1001;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int64_t serviceId = 0;
    DmPublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId,
        publishServiceParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int64_t serviceId = 0;
    DmPublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId,
        publishServiceParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    DmPublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId,
        publishServiceParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    DmPublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServicePublishCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId,
        publishServiceParam, callback);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    DmPublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServicePublishCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId,
        publishServiceParam, callback);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    DmPublishServiceParam publishServiceParam;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId,
        publishServiceParam, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    DmPublishServiceParam publishServiceParam;
    publishServiceParam.discoverMode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_PASSIVE;
    publishServiceParam.media = DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE;
    publishServiceParam.freq = DmExchangeFreq::DM_LOW;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId,
        publishServiceParam, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 9999;
    DmPublishServiceParam publishServiceParam;
    publishServiceParam.discoverMode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_PASSIVE;
    publishServiceParam.media = DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE;
    publishServiceParam.freq = DmExchangeFreq::DM_LOW;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId,
        publishServiceParam, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StartPublishService_009, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    DmPublishServiceParam publishServiceParam;
    publishServiceParam.discoverMode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_PASSIVE;
    publishServiceParam.media = DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE;
    publishServiceParam.freq = DmExchangeFreq::DM_LOW;
    std::shared_ptr<ServicePublishCallback> callback = std::make_shared<ServicePublishCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartPublishService(pkgName, serviceId,
        publishServiceParam, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int64_t serviceId = 12345;
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServicePublishCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(pkgName, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 67890;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServicePublishCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(pkgName, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = -12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServicePublishCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(pkgName, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StopPublishService_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = INT64_MAX;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServicePublishCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StopPublishService(pkgName, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StartDiscoveryService_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StartDiscoveryService_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StartDiscoveryService_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    std::shared_ptr<ServiceDiscoveryCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StartDiscoveryService_004, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    DmDiscoveryServiceParam discParam;
    std::shared_ptr<ServiceDiscoveryCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StartDiscoveryService_005, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    std::shared_ptr<ServiceDiscoveryCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StartDiscoveryService_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServiceDiscoveryCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StartDiscoveryService_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServiceDiscoveryCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StartDiscoveryService_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StartDiscoveryService_009, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.serviceName = "testName";
    discParam.serviceDisplayName = "Test Display Name";
    std::shared_ptr<ServiceDiscoveryCallback> callback = std::make_shared<ServiceDiscoveryCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StartDiscoveryService(pkgName, discParam, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StopDiscoveryService_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    int32_t ret = DeviceManagerImpl::GetInstance().StopDiscoveryService(pkgName, discParam);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, StopDiscoveryService_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().StopDiscoveryService(pkgName, discParam);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StopDiscoveryService_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().StopDiscoveryService(pkgName, discParam);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, StopDiscoveryService_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServiceDiscoveryCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StopDiscoveryService(pkgName, discParam);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StopDiscoveryService_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    DmDiscoveryServiceParam discParam;
    discParam.serviceType = "testService";
    discParam.serviceName = "testName";
    discParam.serviceDisplayName = "Test Display Name";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterServiceDiscoveryCallback(testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().StopDiscoveryService(pkgName, discParam);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, SyncCallbackToServiceForServiceInfo_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int64_t serviceId = 1001;
    DmCommonNotifyEvent event = DmCommonNotifyEvent::REG_SERVICE_STATE;
    int32_t ret = DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(event, pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncCallbackToServiceForServiceInfo_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = -1;
    DmCommonNotifyEvent event = DmCommonNotifyEvent::REG_SERVICE_STATE;
    int32_t ret = DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(event, pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncCallbackToServiceForServiceInfo_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    DmCommonNotifyEvent event = DmCommonNotifyEvent::MIN;
    int32_t ret = DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(event, pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncCallbackToServiceForServiceInfo_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    DmCommonNotifyEvent event = DmCommonNotifyEvent::MAX;
    int32_t ret = DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(event, pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncCallbackToServiceForServiceInfo_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    DmCommonNotifyEvent event = DmCommonNotifyEvent::REG_SERVICE_STATE;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(event, pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, SyncCallbackToServiceForServiceInfo_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    DmCommonNotifyEvent event = DmCommonNotifyEvent::REG_SERVICE_STATE;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(event, pkgName, serviceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, SyncCallbackToServiceForServiceInfo_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    DmCommonNotifyEvent event = DmCommonNotifyEvent::REG_SERVICE_STATE;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(event, pkgName, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, SyncCallbackToServiceForServiceInfo_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 0;
    DmCommonNotifyEvent event = DmCommonNotifyEvent::UN_REG_SERVICE_STATE;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(event, pkgName, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, SyncCallbackToServiceForServiceInfo_009, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    DmCommonNotifyEvent event = DmCommonNotifyEvent::REG_DEVICE_STATE;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().SyncCallbackToServiceForServiceInfo(event, pkgName, serviceId);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = -1;
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId;
    int64_t serviceId = 1001;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    std::shared_ptr<SyncServiceInfoCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    int64_t serviceId = 0;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    int64_t serviceId = -1001;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterSyncServiceInfoCallback(testing::_, testing::_,
        testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterSyncServiceInfoCallback(testing::_, testing::_,
        testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_009, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceInfoByServiceId_010, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 0;
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().SyncServiceInfoByServiceId(pkgName, localUserId,
        networkId, serviceId, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, SyncAllServiceInfo_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().SyncAllServiceInfo(pkgName, localUserId,
        networkId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncAllServiceInfo_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = -1;
    std::string networkId = "test_network_id";
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().SyncAllServiceInfo(pkgName, localUserId,
        networkId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncAllServiceInfo_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId;
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    int32_t ret = DeviceManagerImpl::GetInstance().SyncAllServiceInfo(pkgName, localUserId,
        networkId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncAllServiceInfo_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    std::shared_ptr<SyncServiceInfoCallback> callback = nullptr;
    int32_t ret = DeviceManagerImpl::GetInstance().SyncAllServiceInfo(pkgName, localUserId,
        networkId, callback);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, SyncAllServiceInfo_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterSyncServiceInfoCallback(testing::_, testing::_,
        testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().SyncAllServiceInfo(pkgName, localUserId,
        networkId, callback);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, SyncAllServiceInfo_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    EXPECT_CALL(*deviceManagerNotifyMock_, UnRegisterSyncServiceInfoCallback(testing::_, testing::_,
        testing::_, testing::_))
        .Times(1);
    int32_t ret = DeviceManagerImpl::GetInstance().SyncAllServiceInfo(pkgName, localUserId,
        networkId, callback);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, SyncAllServiceInfo_007, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 100;
    std::string networkId = "test_network_id";
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().SyncAllServiceInfo(pkgName, localUserId,
        networkId, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, SyncAllServiceInfo_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t localUserId = 0;
    std::string networkId = "test_network_id";
    std::shared_ptr<SyncServiceInfoCallback> callback = std::make_shared<SyncServiceInfoCallbackTest>();
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().SyncAllServiceInfo(pkgName, localUserId,
        networkId, callback);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetLocalServiceInfoByServiceId_001, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 0;
    DmRegisterServiceInfo serviceInfo;
    int32_t ret = DeviceManagerImpl::GetInstance().GetLocalServiceInfoByServiceId(serviceId, serviceInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetLocalServiceInfoByServiceId_002, testing::ext::TestSize.Level1)
{
    int64_t serviceId = -1001;
    DmRegisterServiceInfo serviceInfo;
    int32_t ret = DeviceManagerImpl::GetInstance().GetLocalServiceInfoByServiceId(serviceId, serviceInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetLocalServiceInfoByServiceId_003, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 1001;
    DmRegisterServiceInfo serviceInfo;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().GetLocalServiceInfoByServiceId(serviceId, serviceInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetLocalServiceInfoByServiceId_004, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 1001;
    DmRegisterServiceInfo serviceInfo;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetLocalServiceInfoByServiceId(serviceId, serviceInfo);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetLocalServiceInfoByServiceId_005, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 1001;
    DmRegisterServiceInfo serviceInfo;
    DmRegisterServiceInfo expectedServiceInfo;
    expectedServiceInfo.serviceType = "testType";
    expectedServiceInfo.serviceName = "testName";
    expectedServiceInfo.serviceDisplayName = "Test Display Name";
    expectedServiceInfo.serviceOwnerPkgName = "com.ohos.test";

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([expectedServiceInfo](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
                std::shared_ptr<IpcGetLocalServiceInfoExtRsp> extRsp =
                    std::static_pointer_cast<IpcGetLocalServiceInfoExtRsp>(rsp);
                extRsp->SetServiceInfo(expectedServiceInfo);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetLocalServiceInfoByServiceId(serviceId, serviceInfo);
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceInfo.serviceType, expectedServiceInfo.serviceType);
    ASSERT_EQ(serviceInfo.serviceName, expectedServiceInfo.serviceName);
    ASSERT_EQ(serviceInfo.serviceDisplayName, expectedServiceInfo.serviceDisplayName);
    ASSERT_EQ(serviceInfo.serviceOwnerPkgName, expectedServiceInfo.serviceOwnerPkgName);
}

HWTEST_F(DeviceManagerImplTest, GetLocalServiceInfoByServiceId_006, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 9999;
    DmRegisterServiceInfo serviceInfo;
    DmRegisterServiceInfo expectedServiceInfo;

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([expectedServiceInfo](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
                std::shared_ptr<IpcGetLocalServiceInfoExtRsp> extRsp =
                    std::static_pointer_cast<IpcGetLocalServiceInfoExtRsp>(rsp);
                extRsp->SetServiceInfo(expectedServiceInfo);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetLocalServiceInfoByServiceId(serviceId, serviceInfo);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetTrustServiceInfo_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> param;
    std::vector<DmServiceInfo> serviceInfoList;
    int32_t ret = DeviceManagerImpl::GetInstance().GetTrustServiceInfo(pkgName, param, serviceInfoList);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetTrustServiceInfo_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> param = {{"key", "value"}};
    std::vector<DmServiceInfo> serviceInfoList;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().GetTrustServiceInfo(pkgName, param, serviceInfoList);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetTrustServiceInfo_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> param;
    std::vector<DmServiceInfo> serviceInfoList;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetTrustServiceInfo(pkgName, param, serviceInfoList);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetTrustServiceInfo_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> param;
    std::vector<DmServiceInfo> serviceInfoList;

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
                std::shared_ptr<IpcGetTrustServiceInfoRsp> trustRsp =
                    std::static_pointer_cast<IpcGetTrustServiceInfoRsp>(rsp);
                std::vector<DmServiceInfo> serviceInfoVec;
                DmServiceInfo serviceInfo;
                serviceInfo.serviceId = 1001;
                serviceInfo.serviceName = "testService";
                serviceInfoVec.push_back(serviceInfo);
                trustRsp->SetServiceInfoVec(serviceInfoVec);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetTrustServiceInfo(pkgName, param, serviceInfoList);
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceInfoList.size(), 1);
    if (serviceInfoList.size() > 0) {
        ASSERT_EQ(serviceInfoList[0].serviceId, 1001);
        ASSERT_EQ(serviceInfoList[0].serviceName, "testService");
    }
}

HWTEST_F(DeviceManagerImplTest, GetTrustServiceInfo_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> param = {{"extra", "data"}};
    std::vector<DmServiceInfo> serviceInfoList;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
                std::shared_ptr<IpcGetTrustServiceInfoRsp> trustRsp =
                    std::static_pointer_cast<IpcGetTrustServiceInfoRsp>(rsp);
                std::vector<DmServiceInfo> emptyVec;
                trustRsp->SetServiceInfoVec(emptyVec);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetTrustServiceInfo(pkgName, param, serviceInfoList);
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceInfoList.size(), 0);
}

HWTEST_F(DeviceManagerImplTest, GetTrustServiceInfo_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> param;
    std::vector<DmServiceInfo> serviceInfoList;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
                std::shared_ptr<IpcGetTrustServiceInfoRsp> trustRsp =
                    std::static_pointer_cast<IpcGetTrustServiceInfoRsp>(rsp);
                std::vector<DmServiceInfo> serviceInfoVec;
                DmServiceInfo serviceInfo1;
                serviceInfo1.serviceId = 1001;
                DmServiceInfo serviceInfo2;
                serviceInfo2.serviceId = 1002;
                serviceInfoVec.push_back(serviceInfo1);
                serviceInfoVec.push_back(serviceInfo2);
                trustRsp->SetServiceInfoVec(serviceInfoVec);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetTrustServiceInfo(pkgName, param, serviceInfoList);
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceInfoList.size(), 2);
}

HWTEST_F(DeviceManagerImplTest, GetPeerServiceInfoByServiceId_001, testing::ext::TestSize.Level1)
{
    std::string networkId;
    int64_t serviceId = 1001;
    DmRegisterServiceInfo serviceInfo;
    int32_t ret = DeviceManagerImpl::GetInstance().GetPeerServiceInfoByServiceId(networkId, serviceId, serviceInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetPeerServiceInfoByServiceId_002, testing::ext::TestSize.Level1)
{
    std::string networkId = "test_network_id";
    int64_t serviceId = 0;
    DmRegisterServiceInfo serviceInfo;
    int32_t ret = DeviceManagerImpl::GetInstance().GetPeerServiceInfoByServiceId(networkId, serviceId, serviceInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetPeerServiceInfoByServiceId_003, testing::ext::TestSize.Level1)
{
    std::string networkId = "test_network_id";
    int64_t serviceId = -1001;
    DmRegisterServiceInfo serviceInfo;
    int32_t ret = DeviceManagerImpl::GetInstance().GetPeerServiceInfoByServiceId(networkId, serviceId, serviceInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetPeerServiceInfoByServiceId_004, testing::ext::TestSize.Level1)
{
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    DmRegisterServiceInfo serviceInfo;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().GetPeerServiceInfoByServiceId(networkId, serviceId, serviceInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetPeerServiceInfoByServiceId_005, testing::ext::TestSize.Level1)
{
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    DmRegisterServiceInfo serviceInfo;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetPeerServiceInfoByServiceId(networkId, serviceId, serviceInfo);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetPeerServiceInfoByServiceId_006, testing::ext::TestSize.Level1)
{
    std::string networkId = "test_network_id";
    int64_t serviceId = 1001;
    DmRegisterServiceInfo serviceInfo;
    DmRegisterServiceInfo expectedServiceInfo;
    expectedServiceInfo.serviceCode = "1001";
    expectedServiceInfo.serviceType = "testType";
    expectedServiceInfo.serviceName = "testName";

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([expectedServiceInfo](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
                std::shared_ptr<IpcGetLocalServiceInfoExtRsp> extRsp =
                    std::static_pointer_cast<IpcGetLocalServiceInfoExtRsp>(rsp);
                extRsp->SetServiceInfo(expectedServiceInfo);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetPeerServiceInfoByServiceId(networkId, serviceId, serviceInfo);
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(serviceInfo.serviceCode, expectedServiceInfo.serviceCode);
    ASSERT_EQ(serviceInfo.serviceType, expectedServiceInfo.serviceType);
    ASSERT_EQ(serviceInfo.serviceName, expectedServiceInfo.serviceName);
}

HWTEST_F(DeviceManagerImplTest, GetPeerServiceInfoByServiceId_007, testing::ext::TestSize.Level1)
{
    std::string networkId = "test_network_id";
    int64_t serviceId = 9999;
    DmRegisterServiceInfo serviceInfo;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
                std::shared_ptr<IpcGetLocalServiceInfoExtRsp> extRsp =
                    std::static_pointer_cast<IpcGetLocalServiceInfoExtRsp>(rsp);
                DmRegisterServiceInfo emptyServiceInfo;
                extRsp->SetServiceInfo(emptyServiceInfo);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().GetPeerServiceInfoByServiceId(networkId, serviceId, serviceInfo);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceCallbacksToService_001, testing::ext::TestSize.Level1)
{
    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> callbackMap;
    DeviceManagerImpl::GetInstance().SyncServiceCallbacksToService(callbackMap);
    // Should not crash, just verify it returns without error
    SUCCEED();
}

HWTEST_F(DeviceManagerImplTest, SyncServiceCallbacksToService_002, testing::ext::TestSize.Level1)
{
    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> callbackMap;
    callbackMap[DmCommonNotifyEvent::REG_SERVICE_STATE] = {};
    DeviceManagerImpl::GetInstance().SyncServiceCallbacksToService(callbackMap);
    // Should not crash, just verify it returns without error
    SUCCEED();
}

HWTEST_F(DeviceManagerImplTest, SyncServiceCallbacksToService_003, testing::ext::TestSize.Level1)
{
    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> callbackMap;
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    callbackMap[DmCommonNotifyEvent::REG_SERVICE_STATE].insert({pkgName, serviceId});

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    DeviceManagerImpl::GetInstance().SyncServiceCallbacksToService(callbackMap);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceCallbacksToService_004, testing::ext::TestSize.Level1)
{
    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> callbackMap;
    std::string pkgName = "com.ohos.test";
    int64_t serviceId = 1001;
    callbackMap[DmCommonNotifyEvent::REG_SERVICE_STATE].insert({pkgName, serviceId});
    callbackMap[DmCommonNotifyEvent::UN_REG_SERVICE_STATE].insert({pkgName, serviceId});

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    DeviceManagerImpl::GetInstance().SyncServiceCallbacksToService(callbackMap);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceCallbacksToService_005, testing::ext::TestSize.Level1)
{
    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> callbackMap;
    std::string pkgName1 = "com.ohos.test1";
    std::string pkgName2 = "com.ohos.test2";
    int64_t serviceId1 = 1001;
    int64_t serviceId2 = 1002;
    callbackMap[DmCommonNotifyEvent::REG_SERVICE_STATE].insert({pkgName1, serviceId1});
    callbackMap[DmCommonNotifyEvent::REG_SERVICE_STATE].insert({pkgName2, serviceId2});

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    DeviceManagerImpl::GetInstance().SyncServiceCallbacksToService(callbackMap);
}

HWTEST_F(DeviceManagerImplTest, SyncServiceCallbacksToService_006, testing::ext::TestSize.Level1)
{
    std::map<DmCommonNotifyEvent, std::set<std::pair<std::string, int64_t>>> callbackMap;
    std::string pkgName = "com.ohos.test";
    int64_t serviceId1 = 1001;
    int64_t serviceId2 = 1002;
    callbackMap[DmCommonNotifyEvent::REG_SERVICE_STATE].insert({pkgName, serviceId1});
    callbackMap[DmCommonNotifyEvent::UN_REG_SERVICE_STATE].insert({pkgName, serviceId2});

    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    DeviceManagerImpl::GetInstance().SyncServiceCallbacksToService(callbackMap);
}

HWTEST_F(DeviceManagerImplTest, UpdateServiceInfo_001, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 0;
    DmRegisterServiceInfo regServiceInfo;
    int32_t ret = DeviceManagerImpl::GetInstance().UpdateServiceInfo(serviceId, regServiceInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UpdateServiceInfo_002, testing::ext::TestSize.Level1)
{
    int64_t serviceId = -1001;
    DmRegisterServiceInfo regServiceInfo;
    int32_t ret = DeviceManagerImpl::GetInstance().UpdateServiceInfo(serviceId, regServiceInfo);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UpdateServiceInfo_003, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 1001;
    DmRegisterServiceInfo regServiceInfo;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().UpdateServiceInfo(serviceId, regServiceInfo);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UpdateServiceInfo_004, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 1001;
    DmRegisterServiceInfo regServiceInfo;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UpdateServiceInfo(serviceId, regServiceInfo);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UpdateServiceInfo_005, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 1001;
    DmRegisterServiceInfo regServiceInfo;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UpdateServiceInfo(serviceId, regServiceInfo);
    ASSERT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UpdateServiceInfo_006, testing::ext::TestSize.Level1)
{
    int64_t serviceId = 9999;
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.serviceType = "testType";
    regServiceInfo.serviceName = "testName";
    regServiceInfo.serviceOwnerPkgName = "com.ohos.test";
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UpdateServiceInfo(serviceId, regServiceInfo);
    ASSERT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS