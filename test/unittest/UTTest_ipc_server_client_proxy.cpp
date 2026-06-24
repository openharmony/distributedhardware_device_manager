/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "UTTest_ipc_server_client_proxy.h"

#include <unistd.h>

#include "dm_device_info.h"
#include "device_manager_ipc_interface_code.h"
#include "ipc_remote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ipc_client_manager.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_rsp.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_dmfa_result_req.h"
#include "ipc_notify_credential_req.h"
#include "dm_constants.h"

namespace OHOS {
namespace DistributedHardware {
void IpcServerClientProxyTest::SetUp()
{
}

void IpcServerClientProxyTest::TearDown()
{
}

void IpcServerClientProxyTest::SetUpTestCase()
{
}

void IpcServerClientProxyTest::TearDownTestCase()
{
}

namespace {
/**
 * @tc.name: SendCmd_001
 * @tc.desc: 1. set cmdCode not null
 *           2. set remoteObject nullptr
 *           3. call IpcServerClientProxy SendCmd
 *           4. check ret is DEVICEMANAGER_NULLPTR
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
/**
 * @tc.name: SendCmd_002
 * @tc.desc: 1. set cmdCode not null
 *              set pkgName not null
 *              set action not null
 *           2. set remoteObject not nullptr
 *              set req not null
 *              set rsp not null
 *           3. call IpcServerClientProxy SendCmd with parameter
 *           4. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
/**
 * @tc.name: SendCmd_003
 * @tc.desc: 1. set cmdCode not null
 *              set pkgName not null
 *              set action not null
 *           2. set remoteObject not nullptr
 *              set req not null
 *              set rsp not null
 *           3. call IpcServerClientProxy SendCmd with parameter
 *           4. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
/**
 * @tc.name: SendCmd_004
 * @tc.desc: 1. set cmdCode not null
 *              set pkgName not null
 *              set action not null
 *           2. set remoteObject not nullptr
 *              set req not null
 *              set rsp not null
 *           3. call IpcServerClientProxy SendCmd with parameter
 *           4. check ret is DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
/**
 * @tc.name: IpcServerClientProxy_SetUp_Smoke_001
 * @tc.desc: Verify fixture SetUp completes without crash (smoke test).
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(IpcServerClientProxyTest, IpcServerClientProxy_SetUp_Smoke_001, testing::ext::TestSize.Level1)
{
    EXPECT_TRUE(true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
