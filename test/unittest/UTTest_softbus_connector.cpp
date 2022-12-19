/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "ipc_notify_verify_auth_result_req.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
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
std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
std::shared_ptr<DmDeviceStateManager> deviceStateMgr =
    std::make_shared<DmDeviceStateManager>(softbusConnector, listener, hiChainConnector);
std::shared_ptr<DmDiscoveryManager> discoveryMgr = std::make_shared<DmDiscoveryManager>(softbusConnector, listener);
std::shared_ptr<DmPublishManager> publishMgr = std::make_shared<DmPublishManager>(softbusConnector, listener);
/**
 * @tc.name: DeviceOnLine_001
 * @tc.desc: go to the corrort case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, DeviceOnLine_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "123";
    DmDeviceInfo info;
    strcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, "123");
    deviceStateMgr->RegisterSoftbusStateCallback();
    DeviceOnLine(info);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        ret = true;
        listener->ipcServerListener_.req_ = nullptr;
    }
    EXPECT_EQ(ret, false);
    SoftbusConnector::stateCallbackMap_.clear();
}

/**
 * @tc.name: DeviceOnLine_002
 * @tc.desc: go to the corrort case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, DeviceOnLine_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "123";
    DmDeviceInfo info;
    strcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, "123");
    DeviceOnLine(info);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        ret = true;
        listener->ipcServerListener_.req_ = nullptr;
    }
    EXPECT_EQ(ret, false);
    SoftbusConnector::stateCallbackMap_.clear();
}

/**
 * @tc.name: DeviceOffLine_001
 * @tc.desc: go to the corrort case and return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, DeviceOffLine_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "123";
    DmDeviceInfo info;
    strcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, "123");
    deviceStateMgr->RegisterSoftbusStateCallback();
    DeviceOffLine(info);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        ret = true;
        listener->ipcServerListener_.req_ = nullptr;
    }
    EXPECT_EQ(ret, true);
    SoftbusConnector::stateCallbackMap_.clear();
}

/**
 * @tc.name: DeviceOffLine_002
 * @tc.desc: go to the corrort case and return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, DeviceOffLine_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "123";
    DmDeviceInfo info;
    strcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, "123");
    DeviceOffLine(info);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        ret = true;
        listener->ipcServerListener_.req_ = nullptr;
    }
    EXPECT_EQ(ret, false);
    SoftbusConnector::stateCallbackMap_.clear();
}

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
    int ret1 = softbusConnector->RegisterSoftbusDiscoveryCallback(
        pkgName, std::shared_ptr<ISoftbusDiscoveryCallback>(discoveryMgr));
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
    int ret1 = softbusConnector->RegisterSoftbusPublishCallback(
        pkgName, std::shared_ptr<ISoftbusPublishCallback>(publishMgr));
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
    int ret = softbusConnector->UnRegisterSoftbusPublishCallback(pkgName);
    int ret1 = SoftbusConnector::publishCallbackMap_.count(pkgName);
    EXPECT_EQ(ret1, 0);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterSoftbusStateCallback_001
 * @tc.desc: set pkgName = "com.ohos.helloworld";call UnRegisterSoftbusStateCallback function to corrort, return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, RegisterSoftbusStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    int ret = softbusConnector->RegisterSoftbusStateCallback(
        pkgName, std::shared_ptr<ISoftbusStateCallback>(deviceStateMgr));
    int ret1 = SoftbusConnector::discoveryCallbackMap_.count(pkgName);
    EXPECT_EQ(ret1, DM_OK);
    EXPECT_EQ(ret, 0);
    softbusConnector->UnRegisterSoftbusStateCallback(pkgName);
}

/**
 * @tc.name: UnRegisterSoftbusStateCallback_001
 * @tc.desc: set pkgName = "com.ohos.helloworld";call UnRegisterSoftbusStateCallback function to corrort, return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, UnRegisterSoftbusStateCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    int ret = softbusConnector->UnRegisterSoftbusStateCallback(pkgName);
    int ret1 = SoftbusConnector::stateCallbackMap_.count(pkgName);
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
    int ret = softbusConnector->StartDiscovery(dmSubscribeInfo);
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
    std::string udid;
    int ret = softbusConnector->GetUdidByNetworkId(networkId, udid);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name: IsDeviceOnLine_001
 * @tc.desc: set deviceId = "12333", and return false
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, IsDeviceOnLine_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "12333";
    bool ret = softbusConnector->IsDeviceOnLine(deviceId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsDeviceOnLine_002
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, IsDeviceOnLine_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "145677";
    NodeBasicInfo info;
    strcpy_s(info.networkId, DM_MAX_DEVICE_ID_LEN, "145677");
    bool ret = softbusConnector->IsDeviceOnLine(deviceId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: GetSoftbusSession_001
 * @tc.desc: set SoftbusConnector to new a pointer, and the pointer nou equal nullptr, and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetSoftbusSession_001, testing::ext::TestSize.Level0)
{
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
    bool ret = softbusConnector->HaveDeviceInMap(deviceId);
    EXPECT_EQ(ret, false);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectionIpAddress_001
 * @tc.desc: set deviceId null, and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectionIpAddress_001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    std::string ipAddress = "123456";
    int ret = softbusConnector->GetConnectionIpAddress(deviceId, ipAddress);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetConnectionIpAddress_002
 * @tc.desc: set some corrort para, and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectionIpAddress_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "213456";
    std::string ipAddress = "123456";
    int ret = softbusConnector->GetConnectionIpAddress(deviceId, ipAddress);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetConnectionIpAddress_003
 * @tc.desc: set deviceInfo.addrNum = -1;go to second branch and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectionIpAddress_003, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    deviceInfo.addrNum = -1;
    std::string ipAddress;
    std::string deviceId = "3338848";
    int ret = softbusConnector->GetConnectionIpAddress(deviceId, ipAddress);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetConnectionIpAddress_004
 * @tc.desc: set deviceInfo.addrNum = 1;go to second branch and return ERR_DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectionIpAddress_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    constexpr char ETH_IP[] = "0.0.0.0";
    deviceInfo->addrNum = 1;
    deviceInfo->addr[0].type = CONNECTION_ADDR_WLAN;
    (void)strncpy_s(deviceInfo->addr[0].info.ip.ip, IP_STR_MAX_LEN, ETH_IP, strlen(ETH_IP));
    std::string ipAddress;
    std::string deviceId = "3338848";
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
    int ret = softbusConnector->GetConnectionIpAddress(deviceId, ipAddress);
    EXPECT_EQ(ret, DM_OK);
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
    constexpr char ETH_IP[] = "0.0.0.0";
    deviceInfo->addrNum = 1;
    deviceInfo->addr[0].type = CONNECTION_ADDR_ETH;
    (void)strncpy_s(deviceInfo->addr[0].info.ip.ip, IP_STR_MAX_LEN, ETH_IP, strlen(ETH_IP));
    deviceInfo->addr[0].info.ip.port = 0;
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
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
    constexpr char WLAN_IP[] = "1.1.1.1";
    deviceInfo->addrNum = 1;
    deviceInfo->addr[0].type = CONNECTION_ADDR_WLAN;
    (void)strncpy_s(deviceInfo->addr[0].info.ip.ip, IP_STR_MAX_LEN, WLAN_IP, strlen(WLAN_IP));
    deviceInfo->addr[0].info.ip.port = 0;
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
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
    constexpr char BR_MAC[] = "2:2:2:2";
    deviceInfo->addr[0].type = CONNECTION_ADDR_BR;
    (void)strncpy_s(deviceInfo->addr[0].info.br.brMac, IP_STR_MAX_LEN, BR_MAC, strlen(BR_MAC));
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
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
    constexpr char BLE_MAC[] = "3:3:3:3";
    deviceInfo->addrNum = 1;
    deviceInfo->addr[0].type = CONNECTION_ADDR_BLE;
    (void)strncpy_s(deviceInfo->addr[0].info.ble.bleMac, IP_STR_MAX_LEN, BLE_MAC, strlen(BLE_MAC));
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
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
HWTEST_F(SoftbusConnectorTest, ConvertNodeBasicInfoToDmDevice_001, testing::ext::TestSize.Level0)
{
    NodeBasicInfo nodeBasicInfo;
    DmDeviceInfo dmDeviceInfo;
    int ret = softbusListener->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, dmDeviceInfo);
    EXPECT_EQ(ret, DM_OK);
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
    softbusConnector->ConvertDeviceInfoToDmDevice(deviceInfo, dm);
    bool ret = false;
    if (strcmp(dm.deviceId, dm_1.deviceId) == 0) {
        ret = true;
    }
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: OnSoftBusDeviceOnline_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnSoftBusDeviceOnline_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123456";
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId];
    NodeBasicInfo *info = nullptr;
    softbusListener->OnSoftBusDeviceOnline(info);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: OnSoftBusDeviceOnline_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnSoftBusDeviceOnline_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123456";
    NodeBasicInfo info = {
            .networkId = "123456",
            .deviceName = "123456",
            .deviceTypeId = 1
        };
    std::string pkgName = "com.ohos.helloworld";
    softbusConnector->RegisterSoftbusStateCallback(
        pkgName, std::shared_ptr<ISoftbusStateCallback>(deviceStateMgr));
    softbusListener->OnSoftBusDeviceOnline(&info);
    sleep(1);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, true);
    softbusConnector->UnRegisterSoftbusStateCallback(pkgName);
}

/**
 * @tc.name: OnSoftBusDeviceOnline_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnSoftBusDeviceOnline_003, testing::ext::TestSize.Level0)
{
    std::string deviceId = "123456";
    NodeBasicInfo info = {
            .networkId = "123456",
            .deviceName = "123456",
            .deviceTypeId = 1
        };
    std::string oldName = std::string(DM_PKG_NAME);
    std::string pkgName = "com.ohos.test";
    softbusListener->OnSoftBusDeviceOnline(&info);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, false);
    pkgName = oldName;
}

/**
 * @tc.name: OnSoftbusDeviceOffline_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusDeviceOffline_001, testing::ext::TestSize.Level0)
{
    NodeBasicInfo *info = nullptr;
    softbusListener->OnSoftbusDeviceOffline(info);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        ret = true;
    }
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: OnSoftbusDeviceOffline_002
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnSoftbusDeviceOffline_002, testing::ext::TestSize.Level0)
{
    NodeBasicInfo info = {
            .networkId = "123456",
            .deviceName = "123456",
            .deviceTypeId = 1
        };
    std::string pkgName = "com.ohos.helloworld";
    softbusConnector->RegisterSoftbusStateCallback(
        pkgName, std::shared_ptr<ISoftbusStateCallback>(deviceStateMgr));
    softbusListener->OnSoftbusDeviceOffline(&info);
    sleep(1);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, true);
    softbusConnector->UnRegisterSoftbusStateCallback(pkgName);
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
        .devName = "11111"
    };
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
    int32_t subscribeId= 123456;
    RefreshResult result = (RefreshResult)1;
    std::string pkgName = "com.ohos.helloworld";
    softbusConnector->RegisterSoftbusDiscoveryCallback(
        pkgName, std::shared_ptr<ISoftbusDiscoveryCallback>(discoveryMgr));
    softbusConnector->OnSoftbusDiscoveryResult(subscribeId, result);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, true);
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
    int32_t subscribeId= 123456;
    RefreshResult result = (RefreshResult)0;
    std::string pkgName = "com.ohos.helloworld";
    softbusConnector->RegisterSoftbusDiscoveryCallback(
        pkgName, std::shared_ptr<ISoftbusDiscoveryCallback>(discoveryMgr));
    softbusConnector->OnSoftbusDiscoveryResult(subscribeId, result);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, true);
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
    softbusConnector->RegisterSoftbusPublishCallback(
        pkgName, std::shared_ptr<ISoftbusPublishCallback>(publishMgr));
    softbusConnector->OnSoftbusPublishResult(publishId, failReason);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, true);
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
    int32_t publishId= 123456;
    std::string pkgName = "com.ohos.helloworld";
    PublishResult failReason = (PublishResult)0;
    softbusConnector->RegisterSoftbusPublishCallback(
        pkgName, std::shared_ptr<ISoftbusPublishCallback>(publishMgr));
    softbusConnector->OnSoftbusPublishResult(publishId, failReason);
    bool ret = false;
    if (listener->ipcServerListener_.req_ != nullptr) {
        listener->ipcServerListener_.req_ = nullptr;
        ret = true;
    }
    EXPECT_EQ(ret, true);
    softbusConnector->UnRegisterSoftbusPublishCallback(pkgName);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS-