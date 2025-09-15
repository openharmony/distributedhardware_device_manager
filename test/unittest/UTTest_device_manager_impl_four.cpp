/*
* Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <memory>
#include <unistd.h>
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
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"
#include "softbus_error_code.h"
#include "ipc_publish_service_info_rsp.h"
using namespace testing;
namespace OHOS {
namespace DistributedHardware {
namespace {
HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_001, testing::ext::TestSize.Level0)
{
    ServiceRegInfo serviceInfo;
    serviceInfo.serviceInfo.serviceType = "";
    serviceInfo.serviceInfo.serviceName = "TestService";
    serviceInfo.serviceInfo.serviceDisplayName = "Test Service Display";
    int32_t regServiceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(serviceInfo, regServiceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_002, testing::ext::TestSize.Level0)
{
    ServiceRegInfo serviceInfo;
    serviceInfo.serviceInfo.serviceType = "TestType";
    serviceInfo.serviceInfo.serviceName = "";
    serviceInfo.serviceInfo.serviceDisplayName = "Test Service Display";
    int32_t regServiceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(serviceInfo, regServiceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_003, testing::ext::TestSize.Level0)
{
    ServiceRegInfo serviceInfo;
    serviceInfo.serviceInfo.serviceType = "TestType";
    serviceInfo.serviceInfo.serviceName = "TestService";
    serviceInfo.serviceInfo.serviceDisplayName = "";
    int32_t regServiceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(serviceInfo, regServiceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_004, testing::ext::TestSize.Level0)
{
    ServiceRegInfo serviceInfo;
    serviceInfo.serviceInfo.serviceType = "";
    serviceInfo.serviceInfo.serviceName = "";
    serviceInfo.serviceInfo.serviceDisplayName = "";
    int32_t regServiceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(serviceInfo, regServiceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_005, testing::ext::TestSize.Level0)
{
    ServiceRegInfo serviceInfo;
    serviceInfo.serviceInfo.serviceType = "TestType";
    serviceInfo.serviceInfo.serviceName = "TestService";
    serviceInfo.serviceInfo.serviceDisplayName = "Test Service Display";
    int32_t regServiceId = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));

    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(serviceInfo, regServiceId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_006, testing::ext::TestSize.Level0)
{
    ServiceRegInfo serviceInfo;
    serviceInfo.serviceInfo.serviceType = "TestType";
    serviceInfo.serviceInfo.serviceName = "TestService";
    serviceInfo.serviceInfo.serviceDisplayName = "Test Service Display";
    int32_t regServiceId = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                std::shared_ptr<IpcPublishServiceInfoRsp> publishRsp =
                    std::static_pointer_cast<IpcPublishServiceInfoRsp>(rsp);
                publishRsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(serviceInfo, regServiceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, RegisterServiceInfo_007, testing::ext::TestSize.Level0)
{
    ServiceRegInfo serviceInfo;
    serviceInfo.serviceInfo.serviceType = "TestType";
    serviceInfo.serviceInfo.serviceName = "TestService";
    serviceInfo.serviceInfo.serviceDisplayName = "Test Service Display";
    int32_t regServiceId = 0;
    int32_t expectedRegServiceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                std::shared_ptr<IpcPublishServiceInfoRsp> publishRsp =
                    std::static_pointer_cast<IpcPublishServiceInfoRsp>(rsp);
                publishRsp->SetErrCode(DM_OK);
                publishRsp->SetRegServiceId(12345);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().RegisterServiceInfo(serviceInfo, regServiceId);
    ASSERT_EQ(ret, DM_OK);
    ASSERT_EQ(regServiceId, expectedRegServiceId);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceInfo_001, testing::ext::TestSize.Level0)
{
    int32_t regServiceId = 0;
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(regServiceId);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceInfo_002, testing::ext::TestSize.Level0)
{
    int32_t regServiceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(regServiceId);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceInfo_003, testing::ext::TestSize.Level0)
{
    int32_t regServiceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_FAILED);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(regServiceId);
    ASSERT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterServiceInfo_004, testing::ext::TestSize.Level0)
{
    int32_t regServiceId = 12345;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(DM_OK);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManagerImpl::GetInstance().UnRegisterServiceInfo(regServiceId);
    ASSERT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS