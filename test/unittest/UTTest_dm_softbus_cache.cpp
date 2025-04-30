/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_dm_softbus_cache.h"

#include "accesstoken_kit.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"
#include "softbus_error_code.h"
#include <utility>

using namespace OHOS::Security::AccessToken;
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void DMSoftbusCacheTest::SetUp()
{
    const int32_t permsNum = 4;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    const int32_t indexTwo = 2;
    const int32_t indexThree = 3;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[indexTwo] = "ohos.permission.ACCESS_SERVICE_DM";
    perms[indexThree] = "ohos.permission.MONITOR_DEVICE_NETWORK_STATE";
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

void DMSoftbusCacheTest::TearDown()
{
}

void DMSoftbusCacheTest::SetUpTestCase()
{
}

void DMSoftbusCacheTest::TearDownTestCase()
{
}

namespace {

bool CheckSoftbusRes(int32_t ret)
{
    return ret == SOFTBUS_INVALID_PARAM || ret == SOFTBUS_NETWORK_NOT_INIT || ret == SOFTBUS_NETWORK_LOOPER_ERR ||
        ret == SOFTBUS_IPC_ERR || ret == ERR_DM_FAILED || ret == SOFTBUS_NETWORK_GET_NODE_INFO_ERR;
}

HWTEST_F(DMSoftbusCacheTest, GetDeviceInfoFromCache_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1
    };

    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string networkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00";
    strncpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, networkId.c_str(), networkId.length());
    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid1"] = std::pair<std::string, DmDeviceInfo>("uuid1", deviceInfo);
    }
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().UninitSoftbusListener();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}

HWTEST_F(DMSoftbusCacheTest, CheckIsOnline_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
    };

    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    EXPECT_EQ(SoftbusCache::GetInstance().CheckIsOnline("deviceIdTest"), true);
    EXPECT_EQ(SoftbusCache::GetInstance().CheckIsOnline("deviceIdTest1"), false);
}

HWTEST_F(DMSoftbusCacheTest, GetUuidByUdid_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
        .networkId = "networkid"
    };

    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string uuid = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetUuidByUdid("udid", uuid), DM_OK);
    int32_t ret = SoftbusCache::GetInstance().GetUuidByUdid("test", uuid);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DMSoftbusCacheTest, GetNetworkIdFromCache_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
        .networkId = "networkid"
    };

    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string networkId = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetNetworkIdFromCache("udid", networkId), DM_OK);
    int32_t ret = SoftbusCache::GetInstance().GetNetworkIdFromCache("test", networkId);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DMSoftbusCacheTest, GetUdidByUdidHash_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
        .networkId = "networkid"
    };

    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string udid = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetUdidByUdidHash("deviceIdTest", udid), DM_OK);
    int32_t ret = SoftbusCache::GetInstance().GetUdidByUdidHash("test", udid);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DMSoftbusCacheTest, GetDevInfoByNetworkId_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
        .networkId = "networkid"
    };

    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    DmDeviceInfo nodeInfo;
    EXPECT_EQ(SoftbusCache::GetInstance().GetDevInfoByNetworkId("networkid", nodeInfo), DM_OK);
}

HWTEST_F(DMSoftbusCacheTest, GetSecurityDeviceLevel_001, testing::ext::TestSize.Level1)
{
    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceSecurityLevelMutex_);
        SoftbusCache::GetInstance().deviceSecurityLevel_["networkid"] = 1;
    }
    int32_t securityLevel = 0;
    EXPECT_EQ(SoftbusCache::GetInstance().GetSecurityDeviceLevel("networkid", securityLevel), DM_OK);
    EXPECT_EQ(securityLevel, 1);
    int32_t ret = SoftbusCache::GetInstance().GetSecurityDeviceLevel("test", securityLevel);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DMSoftbusCacheTest, GetUuidFromCache_001, testing::ext::TestSize.Level1)
{
    SoftbusCache::GetInstance().DeleteDeviceInfo();
    DmDeviceInfo nodeInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
        .networkId = ""
    };
    SoftbusCache::GetInstance().SaveDeviceInfo(nodeInfo);
    SoftbusCache::GetInstance().UpdateDeviceInfoCache();
    SoftbusCache::GetInstance().DeleteDeviceInfo();
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
        .networkId = "networkid"
    };

    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string uuid = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetUuidFromCache("networkid", uuid), DM_OK);
    EXPECT_EQ(uuid, "uuid");
}

HWTEST_F(DMSoftbusCacheTest, GetUdidFromCache_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
        .networkId = "networkid"
    };

    {
        std::lock_guard<std::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string udid = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetUdidFromCache("networkid", udid), DM_OK);
    EXPECT_EQ(udid, "udid");
    int32_t ret = SoftbusCache::GetInstance().GetUdidFromCache("test", udid);
    EXPECT_NE(CheckSoftbusRes(ret), DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
