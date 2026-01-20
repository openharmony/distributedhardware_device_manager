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
    SoftbusCenterInterface::softbusCenterInterface_ = softbusCenterMock_;
}

void DMSoftbusCacheTest::TearDownTestCase()
{
    SoftbusCenterInterface::softbusCenterInterface_ = nullptr;
    softbusCenterMock_ = nullptr;
}

namespace {

HWTEST_F(DMSoftbusCacheTest, GetDeviceInfoFromCache_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1
    };
    {
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_.clear();
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().UninitSoftbusListener();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
    std::vector<DmDeviceInfo> deviceInfoList;
    int32_t ret = SoftbusCache::GetInstance().GetDeviceInfoFromCache(deviceInfoList);
    EXPECT_EQ(ret, DM_OK);
    SoftbusCache::GetInstance().DeleteDeviceInfo();
}

HWTEST_F(DMSoftbusCacheTest, CheckIsOnline_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
    };
    {
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_.clear();
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    EXPECT_TRUE(SoftbusCache::GetInstance().CheckIsOnline("deviceIdTest"));
    EXPECT_FALSE(SoftbusCache::GetInstance().CheckIsOnline("deviceIdTest1"));
    SoftbusCache::GetInstance().DeleteDeviceInfo();
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
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_.clear();
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string uuid = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetUuidByUdid("udid", uuid), DM_OK);
    EXPECT_EQ(uuid, "uuid");
    uuid = "";
    EXPECT_NE(SoftbusCache::GetInstance().GetUuidByUdid("test", uuid), DM_OK);
    EXPECT_TRUE(uuid.empty());
    SoftbusCache::GetInstance().DeleteDeviceInfo();
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
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_.clear();
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string networkId = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetNetworkIdFromCache("udid", networkId), DM_OK);
    EXPECT_EQ(networkId, deviceInfo.networkId);
    networkId = "";
    EXPECT_NE(SoftbusCache::GetInstance().GetNetworkIdFromCache("test", networkId), DM_OK);
    EXPECT_TRUE(networkId.empty());
    SoftbusCache::GetInstance().DeleteDeviceInfo();
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
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_.clear();
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string udid = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetUdidByUdidHash("deviceIdTest", udid), DM_OK);
    EXPECT_EQ(udid, "udid");
    udid = "";
    EXPECT_NE(SoftbusCache::GetInstance().GetUdidByUdidHash("test", udid), DM_OK);
    EXPECT_TRUE(udid.empty());
    SoftbusCache::GetInstance().DeleteDeviceInfo();
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
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_.clear();
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    DmDeviceInfo nodeInfo;
    EXPECT_EQ(SoftbusCache::GetInstance().GetDevInfoByNetworkId("networkid", nodeInfo), DM_OK);
    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _)).Times(::testing::AtLeast(1))
        .WillOnce(Return(ERR_DM_FAILED));
    EXPECT_NE(SoftbusCache::GetInstance().GetDevInfoByNetworkId("test", nodeInfo), DM_OK);
    SoftbusCache::GetInstance().DeleteDeviceInfo();
}

HWTEST_F(DMSoftbusCacheTest, GetSecurityDeviceLevel_001, testing::ext::TestSize.Level1)
{
    {
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceSecurityLevelMutex_);
        SoftbusCache::GetInstance().deviceSecurityLevel_.clear();
        SoftbusCache::GetInstance().deviceSecurityLevel_["networkid"] = 1;
    }
    int32_t securityLevel = 0;
    EXPECT_EQ(SoftbusCache::GetInstance().GetSecurityDeviceLevel("networkid", securityLevel), DM_OK);
    EXPECT_EQ(securityLevel, 1);
    EXPECT_CALL(*softbusCenterMock_, GetNodeKeyInfo(_, _, _, _, _)).Times(::testing::AtLeast(1))
        .WillOnce(Return(ERR_DM_FAILED));
    EXPECT_NE(SoftbusCache::GetInstance().GetSecurityDeviceLevel("test", securityLevel), DM_OK);
    {
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceSecurityLevelMutex_);
        SoftbusCache::GetInstance().deviceSecurityLevel_.clear();
    }
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
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string uuid = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetUuidFromCache("networkid", uuid), DM_OK);
    EXPECT_EQ(uuid, "uuid");
    SoftbusCache::GetInstance().DeleteDeviceInfo();
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
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_.clear();
        SoftbusCache::GetInstance().deviceInfo_["udid"] = std::pair<std::string, DmDeviceInfo>("uuid", deviceInfo);
    }
    std::string udid = "";
    EXPECT_EQ(SoftbusCache::GetInstance().GetUdidFromCache("networkid", udid), DM_OK);
    EXPECT_EQ(udid, "udid");
    EXPECT_CALL(*softbusCenterMock_, GetNodeKeyInfo(_, _, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_NE(SoftbusCache::GetInstance().GetUdidFromCache("test", udid), DM_OK);
    SoftbusCache::GetInstance().DeleteDeviceInfo();
}

HWTEST_F(DMSoftbusCacheTest, GetDeviceInfoByDeviceId_001, testing::ext::TestSize.Level1)
{
    SoftbusCache::GetInstance().DeleteDeviceInfo();
    DmDeviceInfo deviceInfo = {
        .deviceId = "deviceIdTest",
        .deviceName = "deviceNameTest",
        .deviceTypeId = 1,
        .networkId = "networkidTest"
    };

    {
        std::lock_guard<ffrt::mutex> mutexLock(SoftbusCache::GetInstance().deviceInfosMutex_);
        SoftbusCache::GetInstance().deviceInfo_["udidTest"] =
            std::pair<std::string, DmDeviceInfo>("uuidTest", deviceInfo);
    }
    std::string deviceId = "udidTest";
    std::string uuid = "";
    DmDeviceInfo devInfo;
    EXPECT_EQ(SoftbusCache::GetInstance().GetDeviceInfoByDeviceId(deviceId, uuid, devInfo), true);
    EXPECT_EQ(uuid, "uuidTest");
    SoftbusCache::GetInstance().DeleteDeviceInfo();
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
