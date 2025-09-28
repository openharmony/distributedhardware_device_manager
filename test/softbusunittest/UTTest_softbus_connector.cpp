/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "softbus_error_code.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {

class SoftbusStateCallbackTest : public ISoftbusStateCallback {
public:
    SoftbusStateCallbackTest() {}
    virtual ~SoftbusStateCallbackTest() {}
    void OnDeviceOnline(std::string deviceId, int32_t authForm) {}
    void OnDeviceOffline(std::string deviceId) {}
    void DeleteOffLineTimer(std::string udidHash) override {}
};

class ISoftbusLeaveLNNCallbackTest : public ISoftbusLeaveLNNCallback {
public:
    virtual ~ISoftbusLeaveLNNCallbackTest() {}
    void OnLeaveLNNResult(const std::string &pkgName, const std::string &networkId,
        int32_t retCode) {}
};

void SoftbusConnectorTest::SetUp()
{
}
void SoftbusConnectorTest::TearDown()
{
}
void SoftbusConnectorTest::SetUpTestCase()
{
    SoftbusCenterInterface::softbusCenterInterface_ = softbusCenterMock_;
    DmCrypto::dmCrypto = cryptoMock_;
}
void SoftbusConnectorTest::TearDownTestCase()
{
    SoftbusCenterInterface::softbusCenterInterface_ = nullptr;
    softbusCenterMock_ = nullptr;
    DmCrypto::dmCrypto = nullptr;
    cryptoMock_ = nullptr;
}

namespace {
std::shared_ptr<DeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();

bool CheckSoftbusRes(int32_t ret)
{
    return ret == SOFTBUS_INVALID_PARAM || ret == SOFTBUS_NETWORK_NOT_INIT || ret == SOFTBUS_NETWORK_LOOPER_ERR ||
        ret == SOFTBUS_IPC_ERR;
}

/**
 * @tc.name: SoftbusConnector_001
 * @tc.desc: set SoftbusConnector to new a pointer, and the pointer nou equal nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, SoftbusConnector_001, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, SoftbusConnector_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<SoftbusConnector> m_SoftbusConnector = std::make_shared<SoftbusConnector>();
    m_SoftbusConnector.reset();
    EXPECT_EQ(m_SoftbusConnector, nullptr);
}

/**
 * @tc.name: GetUdidByNetworkId_001
 * @tc.desc: get StartDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetUdidByNetworkId_001, testing::ext::TestSize.Level1)
{
    const char *networkId = "123456";
    std::string udid;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*softbusCenterMock_, GetNodeKeyInfo(_, _, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    int ret = softbusConnector->GetUdidByNetworkId(networkId, udid);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetUuidByNetworkId_001
 * @tc.desc: get StartDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetUuidByNetworkId_001, testing::ext::TestSize.Level1)
{
    const char *networkId = "123456";
    std::string uuid;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*softbusCenterMock_, GetNodeKeyInfo(_, _, _, _, _)).WillOnce(Return(ERR_DM_FAILED));
    int ret = softbusConnector->GetUuidByNetworkId(networkId, uuid);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

/**
 * @tc.name: GetSoftbusSession_001
 * @tc.desc: set SoftbusConnector to new a pointer, and the pointer nou equal nullptr, and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetSoftbusSession_001, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, HaveDeviceInMap_001, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, HaveDeviceInMap_002, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, GetConnectAddrByType_001, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, GetConnectAddrByType_002, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_001, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    std::string connectAddr;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ConnectionAddr> ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GetConnectAddr_002
 * @tc.desc:set deviceId nit null set deviceInfo.addrNum = -1; and return nullptr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_002, testing::ext::TestSize.Level1)
{
    std::string deviceId = "123345";
    std::string connectAddr;
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    deviceInfo->addrNum = -1;
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ConnectionAddr> ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_EQ(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddr_003
 * @tc.desc:set deviceInfo.addrNum = 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_003, testing::ext::TestSize.Level1)
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
    std::shared_ptr<ConnectionAddr> ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddr_004
 * @tc.desc:set deviceInfo.addrNum = 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_004, testing::ext::TestSize.Level1)
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
    std::shared_ptr<ConnectionAddr> ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddr_005
 * @tc.desc:get brMac addr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_005, testing::ext::TestSize.Level1)
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
    std::shared_ptr<ConnectionAddr> ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddr_006
 * @tc.desc:get bleMac addr
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_006, testing::ext::TestSize.Level1)
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
    std::shared_ptr<ConnectionAddr> ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddr_007
 * @tc.desc:set deviceInfo.addrNum = 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_007, testing::ext::TestSize.Level1)
{
    std::string deviceId = "123345";
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    std::string connectAddr;
    constexpr char ncmIp[] = "4.4.4.4";
    deviceInfo->addrNum = 1;
    deviceInfo->addr[0].type = CONNECTION_ADDR_NCM;
    (void)strncpy_s(deviceInfo->addr[0].info.ip.ip, IP_STR_MAX_LEN, ncmIp, strlen(ncmIp));
    deviceInfo->addr[0].info.ip.port = 0;
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ConnectionAddr> ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetConnectAddr_008
 * @tc.desc:set deviceInfo.addrNum = 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_008, testing::ext::TestSize.Level1)
{
    std::string deviceId = "123345";
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    std::string connectAddr;
    constexpr char addrIp[] = "5.5.5.5";
    deviceInfo->addrNum = 1;
    deviceInfo->addr[0].type = CONNECTION_ADDR_MAX;
    (void)strncpy_s(deviceInfo->addr[0].info.ip.ip, IP_STR_MAX_LEN, addrIp, strlen(addrIp));
    deviceInfo->addr[0].info.ip.port = 0;
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ConnectionAddr> ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
    EXPECT_EQ(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: ConvertNodeBasicInfoToDmDevice_001
 * @tc.desc: go to the correct case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, ConvertDeviceInfoToDmDevice_001, testing::ext::TestSize.Level1)
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
 * @tc.name: JoinLnn_001
 * @tc.desc: set deviceId null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, JoinLnn_001, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->JoinLnn(deviceId);
    EXPECT_EQ(SoftbusConnector::discoveryDeviceInfoMap_.empty(), true);
}

/**
 * @tc.name: ConvertDeviceInfoToDmDevice_002
 * @tc.desc: set deviceInfo not null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, ConvertDeviceInfoToDmDevice_002, testing::ext::TestSize.Level1)
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
 * @tc.name: GetDeviceUdidByUdidHash_001
 * @tc.desc: set udidHash null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetDeviceUdidByUdidHash_001, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, RegisterSoftbusStateCallback_001, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, UnRegisterSoftbusStateCallback_001, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, OnSoftbusJoinLNNResult_001, testing::ext::TestSize.Level1)
{
    ConnectionAddr *addr = nullptr;
    char *networkId = nullptr;
    int32_t result = 0;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->OnSoftbusJoinLNNResult(addr, networkId, result);
    EXPECT_EQ(SoftbusConnector::discoveryDeviceInfoMap_.empty(), true);
}

/**
 * @tc.name: AddMemberToDiscoverMap_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, AddMemberToDiscoverMap_001, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, AddMemberToDiscoverMap_002, testing::ext::TestSize.Level1)
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
HWTEST_F(SoftbusConnectorTest, SetProcessInfo_001, testing::ext::TestSize.Level1)
{
    ProcessInfo processInfo;
    std::vector<ProcessInfo> processInfoVec;
    std::string pkgName = "pkgName";
    std::vector<std::string> pkgNameVec;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->SetProcessInfoVec(processInfoVec);
    softbusConnector->SetProcessInfo(processInfo);
    EXPECT_EQ(softbusConnector->processInfoVec_.empty(), false);
}

/**
 * @tc.name: GetDeviceUdidHashByUdid_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetDeviceUdidHashByUdid_001, testing::ext::TestSize.Level1)
{
    std::string udid = "123456789";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(ERR_DM_FAILED));
    std::string ret = softbusConnector->GetDeviceUdidHashByUdid(udid);
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: EraseUdidFromMap_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, EraseUdidFromMap_001, testing::ext::TestSize.Level1)
{
    std::string udid = "123456789";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->EraseUdidFromMap(udid);
    EXPECT_EQ(softbusConnector->deviceUdidMap_.empty(), true);
}

/**
 * @tc.name: GetLocalDeviceName_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetLocalDeviceName_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    std::string ret = softbusConnector->GetLocalDeviceName();
    EXPECT_EQ(ret.empty(), true);

    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _)).WillOnce(Return(DM_OK));
    ret = softbusConnector->GetLocalDeviceName();
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: GetNetworkIdByDeviceId_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetNetworkIdByDeviceId_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::string deviceId = "deviceId";
    std::string strNetworkId = "net******12";
    int32_t deviceCount = 1;
    NodeBasicInfo nodeBasicInfo = {
        .networkId = "network*1",
        .deviceName = "deviceName",
        .deviceTypeId = 1
    };
    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    std::string ret = softbusConnector->GetNetworkIdByDeviceId(deviceId);
    EXPECT_EQ(ret.empty(), true);

    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _))
        .WillOnce(WithArgs<2>(Invoke([&deviceCount](int32_t *infoNum) {
            infoNum = &deviceCount;
            return DM_OK;
        })));
    EXPECT_CALL(*softbusCenterMock_, GetNodeKeyInfo(_, _, _, _, _))
        .WillOnce(WithArgs<3>(Invoke([deviceId](uint8_t *info) {
            memcpy_s(info, (deviceId.length() + 1), deviceId.c_str(), deviceId.length());
            return DM_OK;
        })));
    ret = softbusConnector->GetNetworkIdByDeviceId(deviceId);
    EXPECT_EQ(ret.empty(), true);

    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _)).WillOnce(Return(DM_OK));
    ret = softbusConnector->GetNetworkIdByDeviceId(deviceId);
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: SetProcessInfoVec_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, SetProcessInfoVec_001, testing::ext::TestSize.Level1)
{
    std::vector<ProcessInfo> processInfoVec;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->SetProcessInfoVec(processInfoVec);
    EXPECT_EQ(processInfoVec.empty(), true);
}

/**
 * @tc.name: GetPkgName_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetProcessInfo_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    auto ret = softbusConnector->GetProcessInfo();
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: ClearPkgName_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, ClearProcessInfo_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->ClearProcessInfo();
    EXPECT_EQ(softbusConnector->processInfoVec_.empty(), true);
}

/**
 * @tc.name: HandleDeviceOnline_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, HandleDeviceOnline_001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "deviceId";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ISoftbusStateCallback> callback = std::make_shared<SoftbusStateCallbackTest>();
    softbusConnector->RegisterSoftbusStateCallback(callback);
    softbusConnector->HandleDeviceOnline(deviceId, DmAuthForm::ACROSS_ACCOUNT);
    EXPECT_EQ(softbusConnector->processInfoVec_.empty(), true);
}

/**
 * @tc.name: HandleDeviceOffline_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, HandleDeviceOffline_001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "deviceId";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ISoftbusStateCallback> callback = std::make_shared<SoftbusStateCallbackTest>();
    softbusConnector->RegisterSoftbusStateCallback(callback);
    softbusConnector->HandleDeviceOffline(deviceId);
    EXPECT_EQ(softbusConnector->processInfoVec_.empty(), true);
}

/**
 * @tc.name: CheckIsOnline_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, CheckIsOnline_001, testing::ext::TestSize.Level1)
{
    std::string targetId = "targetDeviceId";
    int32_t deviceCount = 1;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    bool ret = softbusConnector->CheckIsOnline(targetId);
    EXPECT_FALSE(ret);

    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _))
        .WillOnce(WithArgs<2>(Invoke([&deviceCount](int32_t *infoNum) {
            infoNum = &deviceCount;
            return DM_OK;
        })));
    EXPECT_CALL(*softbusCenterMock_, GetNodeKeyInfo(_, _, _, _, _))
        .WillOnce(WithArgs<3>(Invoke([targetId](uint8_t *info) {
            memcpy_s(info, (targetId.length() + 1), targetId.c_str(), targetId.length());
            return DM_OK;
        })));
    ret = softbusConnector->CheckIsOnline(targetId);
    EXPECT_FALSE(ret);

    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _)).WillOnce(Return(DM_OK));
    ret = softbusConnector->CheckIsOnline(targetId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetDeviceInfoByDeviceId_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetDeviceInfoByDeviceId_001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "deviceId";
    int32_t deviceCount = 1;
    std::string strNetworkId = "networkId**1";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    auto ret = softbusConnector->GetDeviceInfoByDeviceId(deviceId);
    EXPECT_EQ(ret.deviceId == deviceId, false);

    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _))
        .WillOnce(WithArgs<2>(Invoke([&deviceCount](int32_t *infoNum) {
            infoNum = &deviceCount;
            return DM_OK;
        })));
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_EQ(ret.deviceId == deviceId, false);

    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _))
        .WillOnce(WithArgs<2>(Invoke([&deviceCount](int32_t *infoNum) {
            infoNum = &deviceCount;
            return DM_OK;
        })));
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusCenterMock_, GetNodeKeyInfo(_, _, _, _, _))
    .WillOnce(WithArgs<3>(Invoke([](uint8_t *info) {
        info[0] = 'd';
        info[1] = 'e';
        info[2] = 'v';
        info[3] = 'i';
        return DM_OK;
    })));
    EXPECT_EQ(ret.deviceId == deviceId, false);

    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _))
        .WillOnce(WithArgs<2>(Invoke([&deviceCount](int32_t *infoNum) {
            infoNum = &deviceCount;
            return DM_OK;
        })));
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusCenterMock_, GetNodeKeyInfo(_, _, _, _, _))
        .WillOnce(WithArgs<3>(Invoke([deviceId, strNetworkId](uint8_t *info) {
            memcpy_s(info, (strNetworkId.length() + 1), strNetworkId.c_str(), (strNetworkId.length()));
            return DM_OK;
        })));
    EXPECT_EQ(ret.deviceId == deviceId, false);
}

/**
 * @tc.name: ConvertNodeBasicInfoToDmDevice_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, ConvertNodeBasicInfoToDmDevice_001, testing::ext::TestSize.Level1)
{
    NodeBasicInfo nodeBasicInfo = {
        .networkId = "123456",
        .deviceName = "name",
    };
    DmDeviceInfo dmDeviceInfo;
    JsonObject extraJson;
    extraJson[PARAM_KEY_OS_TYPE] = 1;
    std::vector<int32_t> versions = {0};
    extraJson[PARAM_KEY_OS_VERSION] = versions;
    dmDeviceInfo.extraData = extraJson.Dump();
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, dmDeviceInfo);
    EXPECT_EQ(softbusConnector->processInfoVec_.empty(), true);
}

HWTEST_F(SoftbusConnectorTest, GetLocalDeviceTypeId_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*softbusCenterMock_, GetLocalNodeDeviceInfo(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = softbusConnector->GetLocalDeviceTypeId();
    EXPECT_EQ(ret, static_cast<int32_t>(DmDeviceType::DEVICE_TYPE_UNKNOWN));

    EXPECT_CALL(*softbusCenterMock_, GetLocalNodeDeviceInfo(_, _))
    .WillOnce(WithArgs<1>(Invoke([](NodeBasicInfo *info) {
        if (info != nullptr) {
            info->deviceTypeId = 1;
        }
        return DM_OK;
    })));
    ret = softbusConnector->GetLocalDeviceTypeId();
    EXPECT_EQ(ret, 1);
}

HWTEST_F(SoftbusConnectorTest, GetLocalDeviceNetworkId_001, testing::ext::TestSize.Level1)
{
    std::string networkId = "network*1";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*softbusCenterMock_, GetLocalNodeDeviceInfo(_, _)).WillOnce(Return(ERR_DM_FAILED));
    auto ret = softbusConnector->GetLocalDeviceNetworkId();
    EXPECT_EQ(ret.empty(), true);

    EXPECT_CALL(*softbusCenterMock_, GetLocalNodeDeviceInfo(_, _))
        .WillOnce(WithArgs<1>(Invoke([networkId](NodeBasicInfo *info) {
            if (info != nullptr) {
                memcpy_s(info->networkId, sizeof(info->networkId), networkId.c_str(), networkId.length());
            }
            return DM_OK;
        })));
    ret = softbusConnector->GetLocalDeviceNetworkId();
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(SoftbusConnectorTest, GetDeviceUdidHashByUdid_002, testing::ext::TestSize.Level1)
{
    std::string udid = "1********69";
    std::string udidHashTemp = "ajj*********47";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(Return(ERR_DM_FAILED));
    std::string ret = softbusConnector->GetDeviceUdidHashByUdid(udid);
    EXPECT_EQ(ret.empty(), true);

    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).WillOnce(WithArgs<1>(Invoke([udidHashTemp](unsigned char *udidHash) {
        memcpy_s(udidHash, (udidHashTemp.length() + 1), udidHashTemp.c_str(), (udidHashTemp.length()));
        return DM_OK;
    })));
    ret = softbusConnector->GetDeviceUdidHashByUdid(udid);
    EXPECT_EQ(ret.empty(), false);

    ret = softbusConnector->GetDeviceUdidHashByUdid(udid);
    EXPECT_EQ(ret.empty(), false);

    int32_t sessionId = 1;
    int32_t sessionKeyId = 1;
    int32_t remoteSessionKeyId = 1;
    softbusConnector->JoinLnnByHml(sessionId, sessionKeyId, remoteSessionKeyId);

    sessionId = 0;
    softbusConnector->JoinLnnByHml(sessionId, sessionKeyId, remoteSessionKeyId);
    std::string deviceId = "deviceId";
    bool isForceJoin = false;
    softbusConnector->JoinLnn(deviceId, isForceJoin);
}

HWTEST_F(SoftbusConnectorTest, OnLeaveLNNResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::string networkId = "12345";
    softbusConnector->leaveLnnPkgMap_[networkId] = "com.ohos.test";
    int32_t retCode = 123;
    softbusConnector->leaveLNNCallback_ = std::make_shared<ISoftbusLeaveLNNCallbackTest>();
    softbusConnector->OnLeaveLNNResult(networkId.c_str(), retCode);
    EXPECT_EQ(softbusConnector->leaveLnnPkgMap_.empty(), true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS-