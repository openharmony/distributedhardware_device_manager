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
    void OnDeviceOffline(std::string deviceId, const bool isOnline) {}
    void DeleteOffLineTimer(const std::string &peerUdid) override {}
};

class ISoftbusLeaveLNNCallbackTest : public ISoftbusLeaveLNNCallback {
public:
    virtual ~ISoftbusLeaveLNNCallbackTest() {}
    void OnLeaveLNNResult(const std::string &pkgName, const std::string &networkId,
        int32_t retCode) {}
};

class ISoftbusConnectorCallbackTest : public ISoftbusConnectorCallback {
public:
    ISoftbusConnectorCallbackTest() {}
    virtual ~ISoftbusConnectorCallbackTest() {}
    void OnSoftbusJoinLNNResult(const int32_t sessionId, const char *networkId, int32_t result) {}
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
/**
 * @tc.name: SoftbusConnector_002
 * @tc.desc: set SoftbusConnector to new a pointer, and the pointer nou equal nullptr, and delete it
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
/**
 * @tc.name: GetUdidByNetworkId_001
 * @tc.desc: get StartDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
/**
 * @tc.name: GetUuidByNetworkId_001
 * @tc.desc: get StartDiscovery to wrong branch and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
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
    std::shared_ptr<ConnectionAddr> ret = softbusConnector->GetConnectAddr(deviceId, connectAddr);
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
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_007, testing::ext::TestSize.Level0)
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
HWTEST_F(SoftbusConnectorTest, GetConnectAddr_008, testing::ext::TestSize.Level0)
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
 * @tc.name: JoinLnn_001
 * @tc.desc: set deviceId null
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, JoinLnn_001, testing::ext::TestSize.Level0)
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
    EXPECT_EQ(SoftbusConnector::discoveryDeviceInfoMap_.empty(), true);
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
HWTEST_F(SoftbusConnectorTest, SetProcessInfo_001, testing::ext::TestSize.Level0)
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
HWTEST_F(SoftbusConnectorTest, GetDeviceUdidHashByUdid_001, testing::ext::TestSize.Level0)
{
    std::string udid = "123456789";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::string ret = softbusConnector->GetDeviceUdidHashByUdid(udid);
    EXPECT_EQ(ret.empty(), true);
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
HWTEST_F(SoftbusConnectorTest, SetProcessInfoVec_001, testing::ext::TestSize.Level0)
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
HWTEST_F(SoftbusConnectorTest, GetProcessInfo_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    auto ret = softbusConnector->GetProcessInfo();
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: ClearPkgName_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, ClearProcessInfo_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->ClearProcessInfo();
    EXPECT_EQ(softbusConnector->processInfoVec_.empty(), true);
}

/**
 * @tc.name: CheckIsOnline_001
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, CheckIsOnline_001, testing::ext::TestSize.Level0)
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
HWTEST_F(SoftbusConnectorTest, GetDeviceInfoByDeviceId_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceId";
    std::string uuid = "";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    EXPECT_CALL(*softbusCenterMock_, GetAllNodeDeviceInfo(_, _, _)).WillOnce(Return(ERR_DM_FAILED));
    auto ret = softbusConnector->GetDeviceInfoByDeviceId(deviceId, uuid);
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
    JsonObject extraJson;
    extraJson[PARAM_KEY_OS_TYPE] = 1;
    std::vector<int32_t> versions = {0};
    extraJson[PARAM_KEY_OS_VERSION] = versions;
    dmDeviceInfo.extraData = extraJson.Dump();
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, dmDeviceInfo);
    EXPECT_EQ(softbusConnector->processInfoVec_.empty(), true);
}

HWTEST_F(SoftbusConnectorTest, GetLocalDeviceTypeId_001, testing::ext::TestSize.Level0)
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

HWTEST_F(SoftbusConnectorTest, GetLocalDeviceNetworkId_001, testing::ext::TestSize.Level0)
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

HWTEST_F(SoftbusConnectorTest, GetDeviceUdidHashByUdid_002, testing::ext::TestSize.Level0)
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

/**
 * @tc.name: RegisterConnectorCallback_001
 * @tc.desc: register then unregister connector callback, both return DM_OK
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, RegisterConnectorCallback_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ISoftbusConnectorCallback> callback = std::make_shared<ISoftbusConnectorCallbackTest>();
    int32_t ret = softbusConnector->RegisterConnectorCallback(callback);
    EXPECT_EQ(ret, DM_OK);
    ret = softbusConnector->UnRegisterConnectorCallback();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetDeviceInfoFromMap_001
 * @tc.desc: deviceId not present in map -> return nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetDeviceInfoFromMap_001, testing::ext::TestSize.Level0)
{
    std::string deviceId = "noSuchDevice";
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    auto ret = SoftbusConnector::GetDeviceInfoFromMap(deviceId);
    EXPECT_EQ(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: GetDeviceInfoFromMap_002
 * @tc.desc: deviceId present in map -> return the stored deviceInfo
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, GetDeviceInfoFromMap_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "deviceInMap";
    std::shared_ptr<DeviceInfo> deviceInfo = std::make_shared<DeviceInfo>();
    SoftbusConnector::discoveryDeviceInfoMap_[deviceId] = deviceInfo;
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    auto ret = SoftbusConnector::GetDeviceInfoFromMap(deviceId);
    EXPECT_NE(ret, nullptr);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: CheckIsNeedJoinLnn_001
 * @tc.desc: empty udid -> return false (early-return branch)
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, CheckIsNeedJoinLnn_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    bool ret = softbusConnector->CheckIsNeedJoinLnn("", "deviceId");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CheckIsNeedJoinLnn_002
 * @tc.desc: empty deviceId -> return false (early-return branch)
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, CheckIsNeedJoinLnn_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    bool ret = softbusConnector->CheckIsNeedJoinLnn("udid", "");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CheckIsNeedJoinLnn_003
 * @tc.desc: deviceId not in discovery map -> GetConnectAddr returns nullptr -> return false
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, CheckIsNeedJoinLnn_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    bool ret = softbusConnector->CheckIsNeedJoinLnn("udid", "missingDeviceId");
    EXPECT_FALSE(ret);
    SoftbusConnector::discoveryDeviceInfoMap_.clear();
}

/**
 * @tc.name: SetChangeProcessInfo_001
 * @tc.desc: push, get and clear change-process-info, exercising the change-info vector trio
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, SetChangeProcessInfo_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    ProcessInfo processInfo;
    processInfo.userId = 1;
    processInfo.pkgName = "pkgName";
    softbusConnector->SetChangeProcessInfo(processInfo);
    auto ret = softbusConnector->GetChangeProcessInfo();
    EXPECT_EQ(ret.empty(), false);
    softbusConnector->ClearChangeProcessInfo();
    EXPECT_EQ(softbusConnector->processChangeInfoVec_.empty(), true);
}

/**
 * @tc.name: LeaveLNN_001
 * @tc.desc: LeaveLNN records pkg into leaveLnnPkgMap_ for the given networkId
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, LeaveLNN_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::string pkgName = "com.ohos.leave";
    std::string networkId = "leaveNet123";
    softbusConnector->LeaveLNN(pkgName, networkId);
    auto it = softbusConnector->leaveLnnPkgMap_.find(networkId);
    EXPECT_NE(it, softbusConnector->leaveLnnPkgMap_.end());
    softbusConnector->leaveLnnPkgMap_.clear();
}

/**
 * @tc.name: RegisterLeaveLNNCallback_001
 * @tc.desc: register then unregister the LeaveLNN callback
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, RegisterLeaveLNNCallback_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::shared_ptr<ISoftbusLeaveLNNCallback> callback = std::make_shared<ISoftbusLeaveLNNCallbackTest>();
    softbusConnector->RegisterLeaveLNNCallback(callback);
    EXPECT_NE(softbusConnector->leaveLNNCallback_, nullptr);
    softbusConnector->UnRegisterLeaveLNNCallback();
    EXPECT_EQ(softbusConnector->leaveLNNCallback_, nullptr);
}

/**
 * @tc.name: OnLeaveLNNResult_002
 * @tc.desc: retCode == SOFTBUS_OK, early-return success branch (map left untouched)
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, OnLeaveLNNResult_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    std::string networkId = "okNet";
    softbusConnector->leaveLnnPkgMap_[networkId] = "com.ohos.ok";
    softbusConnector->leaveLNNCallback_ = std::make_shared<ISoftbusLeaveLNNCallbackTest>();
    softbusConnector->OnLeaveLNNResult(networkId.c_str(), SOFTBUS_OK);
    // success branch returns before erasing, so the entry must remain
    EXPECT_EQ(softbusConnector->leaveLnnPkgMap_.empty(), false);
    softbusConnector->leaveLnnPkgMap_.clear();
}

/**
 * @tc.name: OnLeaveLNNResult_003
 * @tc.desc: retCode != OK but networkId is null/empty -> early-return (empty-networkId branch)
 * @tc.type: FUNC
 */
HWTEST_F(SoftbusConnectorTest, OnLeaveLNNResult_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
    softbusConnector->leaveLNNCallback_ = std::make_shared<ISoftbusLeaveLNNCallbackTest>();
    softbusConnector->OnLeaveLNNResult(nullptr, 999);
    EXPECT_EQ(softbusConnector->leaveLnnPkgMap_.empty(), true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS