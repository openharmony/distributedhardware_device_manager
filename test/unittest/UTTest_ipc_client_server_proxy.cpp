/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "UTTest_ipc_client_server_proxy.h"

#include <unistd.h>

#include "mock/mock_ipc_client_proxy.h"
#include "device_manager.h"
#include "single_instance.h"
#include "device_manager_impl.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_cmd_register.h"
#include "ipc_def.h"
#include "ipc_types.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_stop_discovery_req.h"
#include "ipc_start_discovery_req.h"

namespace OHOS {
namespace DistributedHardware {
void IpcClientServerProxyTest::SetUp()
{
}

void IpcClientServerProxyTest::TearDown()
{
}

void IpcClientServerProxyTest::SetUpTestCase()
{
}

void IpcClientServerProxyTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: SendCmd_001
 * @tc.desc: 1. set cmdCode not null
 *           2. set remoteObject nullptr
 *           3. call IpcClientServerProxy SendCmd
 *           4. check ret is DEVICEMANAGER_NULLPTR
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcClientServerProxyTest, SendCmd_001, testing::ext::TestSize.Level0)
{
    // 1. set cmdCode not null
    int32_t cmdCode = 1;
    // 2. set remoteObject nullptr
    sptr<IRemoteObject> remoteObject = nullptr;
    // 3. call IpcClientServerProxy SendCmd
    auto instance = new IpcClientServerProxy(remoteObject);
    int ret = instance->SendCmd(cmdCode, nullptr, nullptr);
    // 4. check ret is DEVICEMANAGER_NULLPTR
    ASSERT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: SendCmd_002
 * @tc.type: FUNC
 */
HWTEST_F(IpcClientServerProxyTest, SendCmd_002, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = IPC_MSG_BUTT;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto instance = new IpcClientServerProxy(remoteObject);
    int ret = instance->SendCmd(cmdCode, nullptr, nullptr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: SendCmd_003
 * @tc.type: FUNC
 */
HWTEST_F(IpcClientServerProxyTest, SendCmd_003, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = 1;
    sptr<IRemoteObject> remoteObject = sptr<IpcClientStub>(new IpcClientStub());
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    auto instance = new IpcClientServerProxy(remoteObject);
    int ret = instance->SendCmd(cmdCode, req, rsp);
    ASSERT_EQ(ret, ERR_DM_IPC_SEND_REQUEST_FAILED);
}

/**
 * @tc.name: SendCmd_004
 * @tc.type: FUNC
 */
HWTEST_F(IpcClientServerProxyTest, SendCmd_004, testing::ext::TestSize.Level0)
{
    int32_t cmdCode = -1;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto instance = new IpcClientServerProxy(remoteObject);
    int ret = instance->SendCmd(cmdCode, nullptr, nullptr);
    ASSERT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
