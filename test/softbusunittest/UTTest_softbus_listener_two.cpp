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
#include "UTTest_softbus_listener_two.h"

#include <securec.h>
#include <unistd.h>
#include <cstdlib>
#include <thread>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "softbus_listener.cpp"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusListenerTwoTest::SetUp()
{
}
void SoftbusListenerTwoTest::TearDown()
{
}
void SoftbusListenerTwoTest::SetUpTestCase()
{
}
void SoftbusListenerTwoTest::TearDownTestCase()
{
}

namespace {
std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();

bool CheckSoftbusRes(int32_t ret)
{
    return ret == SOFTBUS_INVALID_PARAM || ret == SOFTBUS_NETWORK_NOT_INIT || ret == SOFTBUS_NETWORK_LOOPER_ERR ||
        ret == SOFTBUS_IPC_ERR;
}

HWTEST_F(SoftbusListenerTwoTest, OnCredentialAuthStatus_001, testing::ext::TestSize.Level1)
{
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    softbusListener->DeviceTrustedChange("");
    softbusListener->DeviceUserIdCheckSumChange("");
    uint16_t subscribeId = 1345;
    int32_t ret = softbusListener->StopRefreshSoftbusLNN(subscribeId);
    softbusListener->OnLocalDevInfoChange();
    std::string msg = "123";
    softbusListener->DeviceNotTrust(msg);
    NodeBasicInfo *info = nullptr;
    softbusListener->OnSoftbusDeviceOffline(info);
    TrustChangeType type = TrustChangeType::DEVICE_NOT_TRUSTED;
    const uint32_t msgLen = 100;
    char msg1[msgLen] = {0};
    softbusListener->OnDeviceTrustedChange(type, msg1, msgLen);
    softbusListener->SendAclChangedBroadcast(msg);
    char devicelist[msgLen] = {0};
    uint16_t deviceTypeId = 0;
    int32_t errcode = -1;
    softbusListener->OnCredentialAuthStatus(nullptr, msgLen, deviceTypeId, errcode);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(SoftbusListenerTwoTest, OnSoftbusDeviceOnline_001, testing::ext::TestSize.Level1)
{
    NodeBasicInfo info = {
        .networkId = "123456",
        .deviceName = "123456",
        .deviceTypeId = 1
    };
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    softbusListener->OnSoftbusDeviceOnline(&info);
    EXPECT_EQ(softbusListener->isRadarSoLoad_, true);
    NodeBasicInfo info1 = {
        .networkId = "1234567",
        .deviceName = "1234567",
        .deviceTypeId = 2
    };
    softbusListener->OnSoftbusDeviceOnline(&info);
    EXPECT_EQ(softbusListener->isRadarSoLoad_, true);
    softbusListener->OnSoftbusDeviceOffline(&info);
}

HWTEST_F(SoftbusListenerTwoTest, OnDeviceTrustedChange_001, testing::ext::TestSize.Level1)
{
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    TrustChangeType type = TrustChangeType::DEVICE_NOT_TRUSTED;
    char msg[10] = "testtest";
    uint32_t msgLen = strlen(msg);
    softbusListener->OnDeviceTrustedChange(type, msg, msgLen);
    type = static_cast<TrustChangeType>(10);
    softbusListener->OnDeviceTrustedChange(type, msg, msgLen);
    EXPECT_GE(strlen(msg), 0);
}

HWTEST_F(SoftbusListenerTwoTest, OnSoftbusDeviceFound_001, testing::ext::TestSize.Level1)
{
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    std::string pkgName;
    std::shared_ptr<ISoftbusDiscoveringCallback> callback = std::make_shared<ISoftbusDiscoveringCallbackTest>();
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    int32_t ret = softbusListener->RegisterSoftbusLnnOpsCbk(pkgName, callback);
    EXPECT_EQ(ret, DM_OK);
    DeviceInfo deviceInfo = {
        .devId = "deviceId",
        .devType = (DeviceType)1,
        .devName = "11111",
        .addrNum = 1,
    };
    DeviceInfo *device = &deviceInfo;
    softbusListener->OnSoftbusDeviceFound(device);
    ret = softbusListener->StopRefreshSoftbusLNN(123);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(SoftbusListenerTwoTest, OnSoftbusDeviceInfoChanged_001, testing::ext::TestSize.Level1)
{
    NodeBasicInfoType type = static_cast<NodeBasicInfoType>(100);
    NodeBasicInfo nodeBasic = {
        .networkId = "123456",
        .deviceName = "123456",
        .deviceTypeId = 1
    };
    NodeBasicInfo *info = &nodeBasic;
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    softbusListener->OnSoftbusDeviceInfoChanged(type, info);
    EXPECT_GE(strlen(info->networkId), 0);
}

HWTEST_F(SoftbusListenerTwoTest, ConvertDeviceInfoToDmDevice_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo dmDevice;
    DeviceInfo deviceInfo = {
        .devId = "deviceId",
        .devType = (DeviceType)1,
        .devName = "11111",
        .addrNum = 1,
        .addr[0] = {
            .type = static_cast<ConnectionAddrType>(5),
            .info {
                .ip {
                    .ip = "172.0.0.1",
                    .port = 0,
                }
            }
        }
    };
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    softbusListener->ConvertDeviceInfoToDmDevice(deviceInfo, dmDevice);
    EXPECT_NE(dmDevice.deviceId, deviceInfo.devId);
}

HWTEST_F(SoftbusListenerTwoTest, CacheDiscoveredDevice_001, testing::ext::TestSize.Level1)
{
    DeviceInfo deviceInfo = {
        .devId = "0",
        .devType = (DeviceType)1,
        .devName = "11111",
        .addrNum = 1,
        .addr[0] = {
            .type = ConnectionAddrType::CONNECTION_ADDR_ETH,
            .info {
                .ip {
                    .ip = "172.0.0.1",
                    .port = 0,
                }
            }
        }
    };
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    for (int i = 0; i < 101; i++) {
        std::string id = std::to_string(i);
        strcpy_s(deviceInfo.devId, id.size() + 1, id.c_str());
        softbusListener->CacheDiscoveredDevice(&deviceInfo);
    }
    deviceInfo.addr[0].type = ConnectionAddrType::CONNECTION_ADDR_WLAN;
    softbusListener->CacheDiscoveredDevice(&deviceInfo);
    EXPECT_GE(strlen(deviceInfo.devId), 0);
    int32_t ret = softbusListener->GetIPAddrTypeFromCache("test", deviceInfo.addr[0].info.ip.ip,
        deviceInfo.addr[0].type);
    EXPECT_EQ(ret, ERR_DM_BIND_INPUT_PARA_INVALID);
    ret = softbusListener->GetIPAddrTypeFromCache("55", deviceInfo.addr[0].info.ip.ip,
        deviceInfo.addr[0].type);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(SoftbusListenerTwoTest, GetIPAddrTypeFromCache_001, testing::ext::TestSize.Level1)
{
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    discoveredDeviceMap.clear();
    std::string deviceId = "deviceId";
    std::string ip = "10.11.12.13.14";
    ConnectionAddrType addrType;
    std::shared_ptr<DeviceInfo> infoPtr = std::make_shared<DeviceInfo>();
    std::vector<std::pair<ConnectionAddrType, std::shared_ptr<DeviceInfo>>> deviceVec;
    discoveredDeviceMap.insert(std::pair<std::string,
        std::vector<std::pair<ConnectionAddrType, std::shared_ptr<DeviceInfo>>>>(deviceId, deviceVec));
    if (softbusListener == nullptr) {
        softbusListener = std::make_shared<SoftbusListener>();
    }
    int32_t ret = softbusListener->GetIPAddrTypeFromCache(deviceId, ip, addrType);
    EXPECT_EQ(ret, ERR_DM_BIND_INPUT_PARA_INVALID);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS