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

#include "UTTest_dm_pin_holder.h"
#include "dm_device_info.h"

#include <unistd.h>
#include "accesstoken_kit.h"
#include "device_manager_notify.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_authenticate_device_req.h"
#include "ipc_get_dmfaparam_rsp.h"
#include "ipc_get_info_by_network_req.h"
#include "ipc_get_info_by_network_rsp.h"
#include "ipc_get_local_device_info_rsp.h"
#include "ipc_get_trustdevice_req.h"
#include "ipc_get_trustdevice_rsp.h"
#include "ipc_req.h"
#include "ipc_rsp.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_skeleton.h"
#include "ipc_start_discovery_req.h"
#include "ipc_stop_discovery_req.h"
#include "ipc_publish_req.h"
#include "ipc_unpublish_req.h"
#include "ipc_unauthenticate_device_req.h"
#include "ipc_verify_authenticate_req.h"
#include "nativetoken_kit.h"
#include "securec.h"
#include "token_setproc.h"

namespace OHOS {
namespace DistributedHardware {
void DmPinHolderTest::SetUp()
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
        .processName = "CollaborationFwk",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void DmPinHolderTest::TearDown()
{
}

void DmPinHolderTest::SetUpTestCase()
{
}

void DmPinHolderTest::TearDownTestCase()
{
}

void DmPinHolderCallbackTest::OnCreateResult(int32_t result)
{
    std::cout << "OnCreateResult " << result << std::endl;
}

void DmPinHolderCallbackTest::OnDestroyResult(int32_t result)
{
    std::cout << "OnDestroyResult " << result << std::endl;
}

void DmPinHolderCallbackTest::OnPinHolderDestroy(DmPinType pinType)
{
    std::cout << "OnPinHolderDestroy" << std::endl;
    std::cout << "pinType: " << pinType << std::endl;
}

void DmPinHolderCallbackTest::OnPinHolderCreate(const std::string &deviceId, DmPinType pinType,
    const std::string &payload)
{
    std::cout << "OnPinHolderCreate" << std::endl;
    std::cout << "pinType: " << pinType << std::endl;
    std::cout << "payload: " << payload << std::endl;
}

void DeviceDiscoveryCallbackTest::OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo)
{
    std::cout << "pin holder test ondeviceFound start." << std::endl;
}

namespace {
/**
 * @tc.name: InitDeviceManager_001
 * @tc.desc: 1. set packName not null
 *              set dmInitCallback not null
 *           2. call DeviceManagerImpl::InitDeviceManager with parameter
 *           4. check ret is DM_OK
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmPinHolderTest, InitDeviceManager_101, testing::ext::TestSize.Level0)
{
    // 1. set packName not null
    std::string packName = "com.ohos.dmtest";
    // set dmInitCallback not null
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);

    std::shared_ptr<DmPinHolderCallbackTest> pinHolderCallback = std::make_shared<DmPinHolderCallbackTest>();
    ret = DeviceManager::GetInstance().RegisterPinHolderCallback(packName, pinHolderCallback);

    std::shared_ptr<DeviceDiscoveryCallbackTest> discCallback = std::make_shared<DeviceDiscoveryCallbackTest>();
    DmSubscribeInfo subscribeInfo;
    subscribeInfo.subscribeId = static_cast<uint16_t>(123456);
    subscribeInfo.mode = DmDiscoverMode::DM_DISCOVER_MODE_ACTIVE;
    subscribeInfo.medium = DmExchangeMedium::DM_AUTO;
    subscribeInfo.freq = DmExchangeFreq::DM_HIGH;
    subscribeInfo.isSameAccount = false;
    subscribeInfo.isWakeRemote = false;
    (void)strncpy_s(subscribeInfo.capability, sizeof(subscribeInfo.capability),
        DM_CAPABILITY_OSD, strlen(DM_CAPABILITY_OSD));
    std::string extra = "";
    ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeInfo, extra, discCallback);
    sleep(60);
    // 3. check ret is DM_OK
    ASSERT_EQ(ret, DM_OK);
    DeviceManager::GetInstance().UnInitDeviceManager(packName);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS