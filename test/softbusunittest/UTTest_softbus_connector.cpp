/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "UTTest_softbus_connector.h"

#include <securec.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <thread>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_publish_result_req.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {

class SoftbusStateCallbackTest : public ISoftbusStateCallback {
public:
    SoftbusStateCallbackTest() {}
    virtual ~SoftbusStateCallbackTest() {}
    void OnDeviceOnline(std::string deviceId) {}
    void OnDeviceOffline(std::string deviceId) {}
};

class SoftbusDiscoveryCallbackTest : public ISoftbusDiscoveryCallback {
public:
    SoftbusDiscoveryCallbackTest() {}
    virtual ~SoftbusDiscoveryCallbackTest() {}
    void OnDeviceFound(const std::string &pkgName, DmDeviceInfo &info, bool isOnline) override
    {
        (void)pkgName;
        (void)info;
        (void)isOnline;
    }
    void OnDeviceFound(const std::string &pkgName, DmDeviceBasicInfo &info, const int32_t range, bool isOnline) override
    {
        (void)pkgName;
        (void)info;
        (void)range;
        (void)isOnline;
    }
    void OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId) override
    {
        (void)pkgName;
        (void)subscribeId;
    }
    void OnDiscoveryFailed(const std::string &pkgName, int32_t subscribeId, int32_t failedReason) override
    {
        (void)pkgName;
        (void)subscribeId;
        (void)failedReason;
    }
};

class SoftbusPublishCallbackTest : public ISoftbusPublishCallback {
public:
    SoftbusPublishCallbackTest() {}
    virtual ~SoftbusPublishCallbackTest() {}
    void OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult) override
    {
        (void)pkgName;
        (void)publishId;
        (void)publishResult;
    }
};

void SoftbusConnectorTest::SetUp()
{
}
void SoftbusConnectorTest::TearDown()
{
}
void SoftbusConnectorTest::SetUpTestCase()
{
}
void SoftbusConnectorTest::TearDownTestCase()
{
}

namespace {
std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();

/**
 * @tc.name: SoftbusConnector_001
 * @tc.desc: set SoftbusConnector to new a pointer, and the pointer nou equal nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, SoftbusConnector_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> m_SoftbusConnector = std::make_shared<SoftbusConnector>();
    ASSERT_NE(m_SoftbusConnector, nullptr);
}

/**
 * @tc.name: SoftbusConnector_002
 * @tc.desc: set SoftbusConnector to new a pointer, and the pointer nou equal nullptr, and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, SoftbusConnector_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> m_SoftbusConnector = std::make_shared<SoftbusConnector>();
    m_SoftbusConnector.reset();
    EXPECT_EQ(m_SoftbusConnector, nullptr);
}

/**
 * @tc.name: RegisterSoftbusDiscoveryCallback_001
 * @tc.desc: set pkgName = "com.ohos.helloworld";call RegisterSoftbusDiscoveryCallback function to corrort, return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, RegisterSoftbusDiscoveryCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret1 = softbusConnector->RegisterSoftbusDiscoveryCallback(
        pkgName, std::make_shared<SoftbusDiscoveryCallbackTest>());
    int ret = SoftbusConnector::discoveryCallbackMap_.count(pkgName);
    EXPECT_EQ(ret1, DM_OK);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: UnRegisterSoftbusDiscoveryCallback_001
 * @tc.desc: set pkgName = "com.ohos.helloworld";call UnRegisterSoftbusDiscoveryCallback function to corrort, return
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, UnRegisterSoftbusDiscoveryCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret = softbusConnector->UnRegisterSoftbusDiscoveryCallback(pkgName);
    int ret1 = SoftbusConnector::discoveryCallbackMap_.count(pkgName);
    EXPECT_EQ(ret1, 0);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterSoftbusPublishCallback_001
 * @tc.desc: set pkgName = "com.ohos.helloworld";call RegisterSoftbusPublishCallback function to corrort, return DM_OK
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(SoftbusConnectorTest, RegisterSoftbusPublishCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret1 = softbusConnector->RegisterSoftbusPublishCallback(
        pkgName, std::make_shared<SoftbusPublishCallbackTest>());
    int ret = SoftbusConnector::publishCallbackMap_.count(pkgName);
    EXPECT_EQ(ret1, DM_OK);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: UnRegisterSoftbusPublishCallback_001
 * @tc.desc: set pkgName = "com.ohos.helloworld";call UnRegisterSoftbusPublishyCallback function to corrort, return
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(SoftbusConnectorTest, UnRegisterSoftbusPublishCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret = softbusConnector->UnRegisterSoftbusPublishCallback(pkgName);
    int ret1 = SoftbusConnector::publishCallbackMap_.count(pkgName);
    EXPECT_EQ(ret1, 0);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: StartDiscovery_001
 * @tc.desc: get StartDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, StartDiscovery_001, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubscribeInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret = softbusConnector->StartDiscovery(dmSubscribeInfo);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
}

/**
 * @tc.name: StartDiscovery_002
 * @tc.desc: get StartDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, StartDiscovery_002, testing::ext::TestSize.Level0)
{
    uint16_t subscribeId = 0;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret = softbusConnector->StartDiscovery(subscribeId);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
}

/**
 * @tc.name: StopDiscovery_001
 * @tc.desc: get StartDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, StopDiscovery_001, testing::ext::TestSize.Level0)
{
    uint16_t subscribeId = static_cast<uint16_t>(123456);
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret = softbusConnector->StopDiscovery(subscribeId);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: PublishDiscovery_001
 * @tc.desc: get PublishDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(SoftbusConnectorTest, PublishDiscovery_001, testing::ext::TestSize.Level0)
{
    DmPublishInfo dmPublishInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret = softbusConnector->PublishDiscovery(dmPublishInfo);
    EXPECT_EQ(ret, ERR_DM_PUBLISH_FAILED);
}

/**
 * @tc.name: UnPublishDiscovery_001
 * @tc.desc: get UnPublishDiscovery to wrong branch and return ERR_DM_PUBLISH_FAILED
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(SoftbusConnectorTest, UnPublishDiscovery_001, testing::ext::TestSize.Level0)
{
    int32_t publishId = 123456;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret = softbusConnector->UnPublishDiscovery(publishId);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: GetUdidByNetworkId_001
 * @tc.desc: get StartDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetUdidByNetworkId_001, testing::ext::TestSize.Level0)
{
    const char *networkId = "123456";
    std::string udid;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret = softbusConnector->GetUdidByNetworkId(networkId, udid);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: GetUuidByNetworkId_001
 * @tc.desc: get StartDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetUuidByNetworkId_001, testing::ext::TestSize.Level0)
{
    const char *networkId = "123456";
    std::string uuid;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int ret = softbusConnector->GetUuidByNetworkId(networkId, uuid);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: GetSoftbusSession_001
 * @tc.desc: set SoftbusConnector to new a pointer, and the pointer nou equal nullptr, and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetSoftbusSession_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<SoftbusSession> softSession = softbusConnector->GetSoftbusSession();
    EXPECT_NE(softSession, nullptr);
}

/**
 * @tc.name: GetSoftbusSession_001
 * @tc.desc: set SoftbusConnector to new a pointer, and the pointer nou equal nullptr, and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, HaveDeviceInMap_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "12345678";
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId];
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    bool ret = softbusConnector->HaveDeviceInMap(deviceId);
    EXPECT_EQ(ret, true);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetSoftbusSession_001
 * @tc.desc: set SoftbusConnector to new a pointer, and the pointer nou equal nullptr, and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, HaveDeviceInMap_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "12345678";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    bool ret = softbusConnector->HaveDeviceInMap(deviceId);
    EXPECT_EQ(ret, false);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddrByType_001
 * @tc.desc: set deviceInfo'pointer null, go to first branch, and return nullptr
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddrByType_001, testing::ext::TestSize.Level0)
{
    ConnectionAddrType type;
    type = CONNECTION_ADDR_MAX;
    ConnectionAddr *p = nullptr;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    ConnectionAddr *ret = softbusConnector->GetConnectAddrByType(nullptr, type);
    EXPECT_EQ(p, ret);
}

/**
 * @tc.name: GetConnectAddrByType_002
 * @tc.desc:set deviceInfo to some corrort para, and return nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddrByType_002, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    deviceInfo.addrNum = 1;
    ConnectionAddrType type;
    type = CONNECTION_ADDR_BR;
    ConnectionAddr *p = nullptr;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    ConnectionAddr *ret = softbusConnector->GetConnectAddrByType(&deviceInfo, type);
    EXPECT_EQ(ret, p);
}

/**
 * @tc.name: GetConnectAddr_001
 * @tc.desc: set deviceId to null, and return nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    std::string connectAddr;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    ConnectionAddr *ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetConnectAddr_002
 * @tc.desc:set deviceId nit null set deviceInfo.addrNum = -1; and return nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123345";
    std::string connectAddr;
    DeviceInfo deviceInfo;
    deviceInfo.addrNum = -1;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    ConnectionAddr *ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetConnectAddr_003
 * @tc.desc:set deviceInfo.addrNum = 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_003, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123345";
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    std::string connectAddr;
    constexpr char ethIp[] = "0.0.0.0";
    deviceInfo->addrNum = 1;
    deviceInfo->addr[0].type = CONNECTION_ADDR_ETH;
    (void)strncpy_s(deviceInfo->addr[0].info.ip.ip, IP_STR_MAX_LEN, ethIp, strlen(ethIp));
    deviceInfo->addr[0].info.ip.port = 0;
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    ConnectionAddr *ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddr_004
 * @tc.desc:set deviceInfo.addrNum = 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_004, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123345";
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    std::string connectAddr;
    constexpr char wlanIp[] = "1.1.1.1";
    deviceInfo->addrNum = 1;
    deviceInfo->addr[0].type = CONNECTION_ADDR_WLAN;
    (void)strncpy_s(deviceInfo->addr[0].info.ip.ip, IP_STR_MAX_LEN, wlanIp, strlen(wlanIp));
    deviceInfo->addr[0].info.ip.port = 0;
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    ConnectionAddr *ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddr_005
 * @tc.desc:get brMac addr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_005, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123345";
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    std::string connectAddr;
    deviceInfo->addrNum = 1;
    constexpr char brMac[] = "2:2:2:2";
    deviceInfo->addr[0].type = CONNECTION_ADDR_BR;
    (void)strncpy_s(deviceInfo->addr[0].info.br.brMac, IP_STR_MAX_LEN, brMac, strlen(brMac));
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    ConnectionAddr *ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddr_006
 * @tc.desc:get bleMac addr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_006, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123345";
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    std::string connectAddr;
    constexpr char bleMac[] = "3:3:3:3";
    deviceInfo->addrNum = 1;
    deviceInfo->addr[0].type = CONNECTION_ADDR_BLE;
    (void)strncpy_s(deviceInfo->addr[0].info.ble.bleMac, IP_STR_MAX_LEN, bleMac, strlen(bleMac));
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    ConnectionAddr *ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: ConvertNodeBasicInfoToDmDevice_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, ConvertDeviceInfoToDmDevice_001, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "11111"
    };
    DmDeviceInfo dm;
    DmDeviceInfo dm_1 = {
        .deviceId = "123456",
        .deviceName = "11111",
        .deviceTypeId = 1
    };
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->ConvertDeviceInfoToDmDevice(deviceInfo, dm);
    bool ret = false;
    if (strcmp(dm.deviceId, dm_1.deviceId) == 0) {
        ret = true;
    }
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: OnSoftbusDeviceFound_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusDeviceFound_001, testing::ext::TestSize.Level0)
{
    DeviceInfo *device = nullptr;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->OnSoftbusDeviceFound(device);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: OnSoftbusDeviceFound_002
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusDeviceFound_002, testing::ext::TestSize.Level0)
{
    DeviceInfo device = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "com.ohos.helloworld"
    };
    std::string pkgName = "com.ohos.helloworld";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->RegisterSoftbusDiscoveryCallback(pkgName, std::make_shared<SoftbusDiscoveryCallbackTest>());
    softbusConnector->OnSoftbusDeviceFound(&device);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: OnSoftbusDiscoveryResult_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusDiscoveryResult_001, testing::ext::TestSize.Level0)
{
    int32_t subscribeId = 123456;
    RefreshResult result = (RefreshResult)1;
    std::string pkgName = "com.ohos.helloworld";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->RegisterSoftbusDiscoveryCallback(
        pkgName, std::make_shared<SoftbusDiscoveryCallbackTest>());
    softbusConnector->OnSoftbusDiscoveryResult(subscribeId, result);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, false);
    softbusConnector->UnRegisterSoftbusDiscoveryCallback(pkgName);
}

/**
 * @tc.name: OnSoftbusDiscoveryResult_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusDiscoveryResult_002, testing::ext::TestSize.Level0)
{
    int32_t subscribeId = 123456;
    RefreshResult result = (RefreshResult)0;
    std::string pkgName = "com.ohos.helloworld";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->RegisterSoftbusDiscoveryCallback(
        pkgName, std::make_shared<SoftbusDiscoveryCallbackTest>());
    softbusConnector->OnSoftbusDiscoveryResult(subscribeId, result);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, false);
    softbusConnector->UnRegisterSoftbusDiscoveryCallback(pkgName);
}

/**
 * @tc.name: OnSoftbusPublishResult_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusPublishResult_001, testing::ext::TestSize.Level0)
{
    int32_t publishId = 123456;
    PublishResult failReason = (PublishResult)1;
    std::string pkgName = "com.ohos.helloworld";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->RegisterSoftbusPublishCallback(
        pkgName, std::make_shared<SoftbusPublishCallbackTest>());
    softbusConnector->OnSoftbusPublishResult(publishId, failReason);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, false);
    softbusConnector->UnRegisterSoftbusPublishCallback(pkgName);
}

/**
 * @tc.name: OnSoftbusPublishResult_004
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: I5N1K3
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusPublishResult_002, testing::ext::TestSize.Level0)
{
    int32_t publishId = 123456;
    std::string pkgName = "com.ohos.helloworld";
    PublishResult failReason = (PublishResult)0;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->RegisterSoftbusPublishCallback(
        pkgName, std::make_shared<SoftbusPublishCallbackTest>());
    softbusConnector->OnSoftbusPublishResult(publishId, failReason);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, false);
    softbusConnector->UnRegisterSoftbusPublishCallback(pkgName);
}

/**
 * @tc.name: JoinLnn_001
 * @tc.desc: set deviceId null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, JoinLnn_001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->JoinLnn(deviceId);
    EXPECT_EQ(SoftbusConnector::discoveryDeviceInfoMap_.empty(), false);
}

/**
 * @tc.name: ConvertDeviceInfoToDmDevice_002
 * @tc.desc: set deviceInfo not null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, ConvertDeviceInfoToDmDevice_002, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "11111"
    };
    DmDeviceBasicInfo dmDeviceBasicInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->ConvertDeviceInfoToDmDevice(deviceInfo, dmDeviceBasicInfo);
    EXPECT_EQ(dmDeviceBasicInfo.deviceTypeId, deviceInfo.devType);
}

/**
 * @tc.name: OnSoftbusDeviceDiscovery_001
 * @tc.desc: set device null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    DeviceInfo *device = nullptr;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->OnSoftbusDeviceDiscovery(device);
    EXPECT_EQ(SoftbusConnector::discoveryDeviceInfoMap_.empty(), false);
}

/**
 * @tc.name: OnSoftbusDeviceDiscovery_002
 * @tc.desc: set device not null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    DeviceInfo device = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "11111"
    };
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->OnSoftbusDeviceDiscovery(&device);
    EXPECT_EQ(SoftbusConnector::discoveryDeviceInfoMap_.empty(), false);
}

/**
 * @tc.name: GetDeviceUdidByUdidHash_001
 * @tc.desc: set udidHash null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetDeviceUdidByUdidHash_001, testing::ext::TestSize.Level0)
{
    std::string udidHash;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::string str = softbusConnector->GetDeviceUdidByUdidHash(udidHash);
    EXPECT_EQ(str.empty(), true);
}

/**
 * @tc.name: RegisterSoftbusStateCallback_001
 * @tc.desc: set callback null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, RegisterSoftbusStateCallback_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ISoftbusStateCallback> callback = std::make_shared<SoftbusStateCallbackTest>();
    int32_t ret = softbusConnector->RegisterSoftbusStateCallback(callback);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterSoftbusStateCallback_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, UnRegisterSoftbusStateCallback_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int32_t ret = softbusConnector->UnRegisterSoftbusStateCallback();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnSoftbusJoinLNNResult_001
 * @tc.desc: set addr null
 * @tc.desc: set networkId null
 * @tc.desc: set result 0
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusJoinLNNResult_001, testing::ext::TestSize.Level0)
{
    ConnectionAddr *addr = nullptr;
    char *networkId = nullptr;
    int32_t result = 0;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->OnSoftbusJoinLNNResult(addr, networkId, result);
    EXPECT_EQ(SoftbusConnector::discoveryDeviceInfoMap_.empty(), false);
}

/**
 * @tc.name: GetLocalDeviceTypeId_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetLocalDeviceTypeId_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int32_t ret = softbusConnector->GetLocalDeviceTypeId();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetLocalDeviceNetworkId_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetLocalDeviceNetworkId_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    auto ret = softbusConnector->GetLocalDeviceNetworkId();
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: AddMemberToDiscoverMap_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, AddMemberToDiscoverMap_001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    std::shared_ptr<DeviceInfo> deviceInfo = nullptr;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int32_t ret = softbusConnector->AddMemberToDiscoverMap(deviceId, deviceInfo);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: AddMemberToDiscoverMap_002
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, AddMemberToDiscoverMap_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    int32_t ret = softbusConnector->AddMemberToDiscoverMap(deviceId, deviceInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: SetPkgName_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, SetPkgName_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::vector<std::string> pkgNameVec;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->SetPkgNameVec(pkgNameVec);
    softbusConnector->SetPkgName(pkgName);
    EXPECT_EQ(softbusConnector->pkgNameVec_.empty(), false);
}

/**
 * @tc.name: GetDeviceUdidHashByUdid_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetDeviceUdidHashByUdid_001, testing::ext::TestSize.Level0)
{
    std::string udid = "123456789";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::string ret = softbusConnector->GetDeviceUdidHashByUdid(udid);
    EXPECT_EQ(ret.empty(), false);
}

/**
 * @tc.name: EraseUdidFromMap_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, EraseUdidFromMap_001, testing::ext::TestSize.Level0)
{
    std::string udid = "123456789";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->EraseUdidFromMap(udid);
    EXPECT_EQ(softbusConnector->deviceUdidMap_.empty(), false);
}

/**
 * @tc.name: GetLocalDeviceName_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetLocalDeviceName_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::string ret = softbusConnector->GetLocalDeviceName();
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: GetNetworkIdByDeviceId_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetNetworkIdByDeviceId_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::string ret = softbusConnector->GetNetworkIdByDeviceId(deviceId);
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: SetPkgNameVec_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, SetPkgNameVec_001, testing::ext::TestSize.Level0)
{
    std::vector<std::string> pkgNameVec;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->SetPkgNameVec(pkgNameVec);
    EXPECT_EQ(pkgNameVec.empty(), true);
}

/**
 * @tc.name: GetPkgName_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetPkgName_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    auto ret = softbusConnector->GetPkgName();
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: ClearPkgName_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, ClearPkgName_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->ClearPkgName();
    EXPECT_EQ(softbusConnector->pkgNameVec_.empty(), true);
}

/**
 * @tc.name: HandleDeviceOnline_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, HandleDeviceOnline_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ISoftbusStateCallback> callback = std::make_shared<SoftbusStateCallbackTest>();
    softbusConnector->RegisterSoftbusStateCallback(callback);
    softbusConnector->HandleDeviceOnline(deviceId);
    EXPECT_EQ(softbusConnector->pkgNameVec_.empty(), true);
}

/**
 * @tc.name: HandleDeviceOffline_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, HandleDeviceOffline_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ISoftbusStateCallback> callback = std::make_shared<SoftbusStateCallbackTest>();
    softbusConnector->RegisterSoftbusStateCallback(callback);
    softbusConnector->HandleDeviceOffline(deviceId);
    EXPECT_EQ(softbusConnector->pkgNameVec_.empty(), true);
}

/**
 * @tc.name: CheckIsOnline_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, CheckIsOnline_001, testing::ext::TestSize.Level0)
{
    std::string targetDeviceId = "targetDeviceId";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->CheckIsOnline(targetDeviceId);
    EXPECT_EQ(softbusConnector->pkgNameVec_.empty(), true);
}

/**
 * @tc.name: GetDeviceInfoByDeviceId_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetDeviceInfoByDeviceId_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    auto ret = softbusConnector->GetDeviceInfoByDeviceId(deviceId);
    EXPECT_EQ(ret.deviceId == deviceId, false);
}

/**
 * @tc.name: ConvertNodeBasicInfoToDmDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, ConvertNodeBasicInfoToDmDevice_001, testing::ext::TestSize.Level0)
{
    NodeBasicInfo nodeBasicInfo = {
        .networkId = "123456",
        .deviceName = "name",
    };
    DmDeviceInfo dmDeviceInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, dmDeviceInfo);
    EXPECT_EQ(softbusConnector->pkgNameVec_.empty(), true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS-