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

#include "UTTest_device_manager_impl.h"
#include "dm_device_info.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_get_os_type_by_network_req.h"
#include "ipc_get_os_type_by_network_rsp.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string networkId;
    int32_t osType = 0;
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId;
    int32_t osType = 0;
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_003, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string networkId = "networkId123";
    int32_t osType = 0;
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "networkId123";
    int32_t osType = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "networkId123";
    int32_t osType = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                rsp->SetErrCode(ERR_DM_NO_PERMISSION);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "networkId123";
    int32_t osType = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                auto osTypeRsp = std::static_pointer_cast<IpcGetOsTypeByNetworkIdRsp>(rsp);
                osTypeRsp->SetErrCode(DM_OK);
                osTypeRsp->SetOsType(1);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(osType, 1);
}

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_007, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "networkId123";
    int32_t osType = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                auto osTypeRsp = std::static_pointer_cast<IpcGetOsTypeByNetworkIdRsp>(rsp);
                osTypeRsp->SetErrCode(ERR_DM_GET_OSTYPE_FAILED);
                osTypeRsp->SetOsType(-1);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, ERR_DM_GET_OSTYPE_FAILED);
    EXPECT_EQ(osType, 0);
}

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_008, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "networkId123";
    int32_t osType = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(Return(ERR_DM_IPC_SEND_REQUEST_FAILED));
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_009, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "networkId123";
    int32_t osType = 0;
    EXPECT_CALL(*ipcClientProxyMock_, SendRequest(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(
            WithArg<2>([](std::shared_ptr<IpcRsp> rsp) {
                auto osTypeRsp = std::static_pointer_cast<IpcGetOsTypeByNetworkIdRsp>(rsp);
                osTypeRsp->SetErrCode(DM_OK);
                osTypeRsp->SetOsType(2);
            }),
            Return(DM_OK)
        ));
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(osType, 2);
}

HWTEST_F(DeviceManagerImplTest, GetOsTypeByNetworkId_010, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::string networkId = "networkId123";
    int32_t osType = 0;
    DeviceManagerImpl::GetInstance().ipcClientProxy_ = nullptr;
    int32_t ret = DeviceManager::GetInstance().GetOsTypeByNetworkId(pkgName, networkId, osType);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
} // namespace DistributedHardware
} // namespace OHOS