/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "UTTest_ipc_client_stub.h"

#include <memory>
#include <unistd.h>

#include "device_manager.h"
#include "device_manager_impl.h"
#include "device_manager_ipc_interface_code.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "mock/mock_ipc_client_stub.h"
#include "ipc_remote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ipc_client_manager.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_cmd_register.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "ipc_rsp.h"
#include "ipc_def.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void IpcClientStubTest::SetUp()
{
}

void IpcClientStubTest::TearDown()
{
}

void IpcClientStubTest::SetUpTestCase()
{
    ipcCmdRegisterMock_ = std::make_shared<IpcCmdRegisterMock>();
    DmIpcCmdRegister::dmIpcCmdRegister = ipcCmdRegisterMock_;
}

void IpcClientStubTest::TearDownTestCase()
{
    DmIpcCmdRegister::dmIpcCmdRegister = nullptr;
    ipcCmdRegisterMock_ = nullptr;
}

namespace {
/**
 * @tc.name: OnRemoteRequest_001
 * @tc.desc: 1. set MessageOption not null
 *              set MessageParcel not null
 *              set MessageParcel not null
 *           2. set set code is 999
 *           3. call IpcClientStub OnRemoteRequest with parameter
 *           4. check result is ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcClientStubTest, OnRemoteRequest_001, testing::ext::TestSize.Level0)
{
    // 1. set MessageOption not null
    MessageOption option;
    // set MessageParcel not null
    MessageParcel data;
    // set MessageParcel not null
    MessageParcel reply;
    // 2. set set code is 999
    int code = 999;
    sptr<IpcClientStub> instance(new IpcClientStub());
    // 3. call IpcClientStub OnRemoteRequest with parameter
    int32_t result = instance->OnRemoteRequest(code, data, reply, option);
    // 4. check result is ERR_DM_FAILED
    ASSERT_NE(result, DM_OK);
}

/**
 * @tc.name: OnRemoteRequest_002
 * @tc.desc: 1. set MessageOption not null
 *              set MessageParcel not null
 *              set MessageParcel not null
 *           2. set set code is SERVER_DEVICE_FA_NOTIFY
 *           3. call IpcClientStub OnRemoteRequest with parameter
 *           4. check result is ERR_DM_IPC_READ_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcClientStubTest, OnRemoteRequest_002, testing::ext::TestSize.Level0)
{
    // 1. set MessageOption not null
    MessageOption option;
    // set MessageParcel not null
    MessageParcel data;
    // set MessageParcel not null
    MessageParcel reply;
    // 2. set set code is SERVER_DEVICE_FA_NOTIFY
    int code = SERVER_DEVICE_FA_NOTIFY;
    sptr<IpcClientStub> instance(new IpcClientStub());
    // 3. call IpcClientStub OnRemoteRequest with parameter
    int ret = instance->OnRemoteRequest(code, data, reply, option);
    // 4. check result is ERR_DM_IPC_READ_FAILED
    ASSERT_EQ(ret, ERR_DM_IPC_READ_FAILED);
}

/**
 * @tc.name: OnRemoteRequest_003
 * @tc.type: FUNC
 */
HWTEST_F(IpcClientStubTest, OnRemoteRequest_003, testing::ext::TestSize.Level0)
{
    MessageOption option;
    MessageParcel data;
    MessageParcel reply;
    int code = GET_TRUST_DEVICE_LIST;
    std::shared_ptr<MockIpcClientStub> mockInstance = std::make_shared<MockIpcClientStub>();
    std::shared_ptr<IpcClientStub> ipcClientStub= mockInstance;
    EXPECT_CALL(*mockInstance, OnRemoteRequest(testing::_, testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    std::shared_ptr<IpcClientStub> instance = std::shared_ptr<IpcClientStub>(ipcClientStub);
    int ret = instance->OnRemoteRequest(code, data, reply, option);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SendCmd_001
 * @tc.desc: 1. set set code is SERVER_DEVICE_FA_NOTIFY
 *              set req is nullptr
 *              set rsp is nullptr
 *           2. call IpcClientStub SendCmd with parameter
 *           3. check result is ERR_DM_IPC_SEND_REQUEST_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcClientStubTest, SendCmd_001, testing::ext::TestSize.Level0)
{
    // 1. set set code is SERVER_DEVICE_FA_NOTIFY
    int cmdCode = SERVER_DEVICE_FA_NOTIFY;
    // set req is nullptr
    std::shared_ptr<IpcReq> req = nullptr;
    // set rsp is nullptr
    std::shared_ptr<IpcRsp> rsp = nullptr;
    sptr<IpcClientStub> instance(new IpcClientStub());
    // 2. call IpcClientStub SendCmd with parameter
    int ret = instance->SendCmd(cmdCode, req, rsp);
    // 3. check result is ERR_DM_IPC_SEND_REQUEST_FAILED
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SendCmd_002
 * @tc.type: FUNC
 */
HWTEST_F(IpcClientStubTest, SendCmd_002, testing::ext::TestSize.Level0)
{
    int result = 305;
    int cmdCode = IPC_MSG_BUTT;
    std::shared_ptr<IpcReq> req = nullptr;
    std::shared_ptr<IpcRsp> rsp = nullptr;
    sptr<IpcClientStub> instance(new IpcClientStub());
    int ret = instance->SendCmd(cmdCode, req, rsp);
    ASSERT_EQ(ret, result);
}

/**
 * @tc.name: SendCmd_003
 * @tc.type: FUNC
 */
HWTEST_F(IpcClientStubTest, SendCmd_003, testing::ext::TestSize.Level0)
{
    int cmdCode = GET_TRUST_DEVICE_LIST;
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    std::shared_ptr<MockIpcClientStub> mockInstance = std::make_shared<MockIpcClientStub>();
    std::shared_ptr<IpcClientStub> ipcClientStub= mockInstance;
    EXPECT_CALL(*mockInstance, SendCmd(testing::_, testing::_, testing::_))
                .Times(1).WillOnce(testing::Return(DM_OK));
    std::shared_ptr<IpcClientStub> instance = std::shared_ptr<IpcClientStub>(ipcClientStub);
    int ret = instance->SendCmd(cmdCode, req, rsp);
    ASSERT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SendCmd_004
 * @tc.type: FUNC
 */
HWTEST_F(IpcClientStubTest, SendCmd_004, testing::ext::TestSize.Level0)
{
    int result = 305;
    int cmdCode = -1;
    std::shared_ptr<IpcReq> req = nullptr;
    std::shared_ptr<IpcRsp> rsp = nullptr;
    sptr<IpcClientStub> instance = sptr<IpcClientStub>(new IpcClientStub());
    int ret = instance->SendCmd(cmdCode, req, rsp);
    ASSERT_EQ(ret, result);
}

HWTEST_F(IpcClientStubTest, SendCmd_005, testing::ext::TestSize.Level0)
{
    int cmdCode = 1;
    std::shared_ptr<IpcReq> req = nullptr;
    std::shared_ptr<IpcRsp> rsp = nullptr;
    sptr<IpcClientStub> instance = sptr<IpcClientStub>(new IpcClientStub());
    EXPECT_CALL(*ipcCmdRegisterMock_, SetRequest(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*ipcCmdRegisterMock_, OnIpcCmd(_, _, _)).WillOnce(Return(DM_OK));
    int ret = instance->SendCmd(cmdCode, req, rsp);
    ASSERT_EQ(ret, DM_OK);

    req = std::make_shared<IpcReq>();
    rsp = std::make_shared<IpcRsp>();
    EXPECT_CALL(*ipcCmdRegisterMock_, SetRequest(_, _, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*ipcCmdRegisterMock_, OnIpcCmd(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    ret = instance->SendCmd(cmdCode, req, rsp);
    ASSERT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
