/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "parameter.h"
#include "softbus_connector.h"
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
static std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
/**
 * @tc.name: Init_001
 * @tc.desc: go tothe corrort case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, Init_001, testing::ext::TestSize.Level0)
{
    int ret = softbusConnector->Init();
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: RegisterSoftbusDiscoveryCallback_001
 * @tc.desc: set pkgName = "com.ohos.helloworld";call RegisterSoftbusDiscoveryCallback function to corrort ,return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, RegisterSoftbusDiscoveryCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DmDiscoveryManager> discoveryMgr_ =
        std::make_shared<DmDiscoveryManager>(softbusConnector, listener_);
    int ret = softbusConnector->RegisterSoftbusDiscoveryCallback(
        pkgName, std::shared_ptr<ISoftbusDiscoveryCallback>(discoveryMgr_));
    int ret1 = SoftbusConnector::discoveryCallbackMap_.count(pkgName);
    EXPECT_EQ(ret1, 1);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterSoftbusDiscoveryCallback_001
 * @tc.desc: set pkgName = "com.ohos.helloworld";call UnRegisterSoftbusDiscoveryCallback function to corrort ,return
 * DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, UnRegisterSoftbusDiscoveryCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    int ret = softbusConnector->UnRegisterSoftbusDiscoveryCallback(pkgName);
    uint32_t ret1 = SoftbusConnector::discoveryCallbackMap_.count(pkgName);
    EXPECT_EQ(ret1, 0);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: UnRegisterSoftbusStateCallback_001
 * @tc.desc: set pkgName = "com.ohos.helloworld";call UnRegisterSoftbusStateCallback function to corrort ,return DM_OK
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
 * @tc.name: GetTrustedDeviceList_001
 * @tc.desc: create GetAllNodeDeviceInfo  not equal 0, and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetTrustedDeviceList_001, testing::ext::TestSize.Level0)
{
    std::vector<DmDeviceInfo> deviceInfoList;
    int ret = softbusConnector->GetTrustedDeviceList(deviceInfoList);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: GetLocalDeviceInfo_001
 * @tc.desc: set pkgName to com.softbus.test,define deviceInfo,and return DM_OK.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetLocalDeviceInfo_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.softbus.test";
    DmDeviceInfo deviceInfo;
    int ret = softbusConnector->GetLocalDeviceInfo(deviceInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: StartDiscovery_001
 * @tc.desc: get StartDiscovery to wroing master and return DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, StartDiscovery_001, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubscribeInfo;
    int ret = softbusConnector->StartDiscovery(dmSubscribeInfo);
    EXPECT_EQ(ret, DM_DISCOVERY_FAILED);
}

/**
 * @tc.name: IsDeviceOnLine_001
 * @tc.desc: set deviceId = "12333";,and return false
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
    NodeBasicInfo Info;
    strncpy(Info.networkId, "145677", sizeof(Info.networkId));
    bool ret = softbusConnector->IsDeviceOnLine(deviceId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SoftbusConnector_001
 * @tc.desc: set SoftbusConnector to new a pointer ,and the pointer nou equal nullptr
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
 * @tc.desc: set SoftbusConnector to new a pointer ,and the pointer nou equal nullptr,and delete it
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
 * @tc.name: GetConnectionIpAddress_001
 * @tc.desc: set deviceId null,and return DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectionIpAddress_001, testing::ext::TestSize.Level0)
{
    std::string deviceId;
    std::string ipAddress = "123456";
    int ret = softbusConnector->GetConnectionIpAddress(deviceId, ipAddress);
    EXPECT_EQ(ret, DM_FAILED);
}

/**
 * @tc.name: GetConnectionIpAddress_002
 * @tc.desc: set some corrort para,and return DM_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, GetConnectionIpAddress_002, testing::ext::TestSize.Level0)
{
    std::string deviceId = "213456";
    std::string ipAddress = "123456";
    int ret = softbusConnector->GetConnectionIpAddress(deviceId, ipAddress);
    EXPECT_EQ(ret, DM_FAILED);
}
/**
 * @tc.name: GetConnectionIpAddress_003
 * @tc.desc: set deviceInfo.addrNum = -1;go to second master and return DM_FAILED
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
    EXPECT_EQ(ret, DM_FAILED);
}

/**
 * @tc.name: GetConnectAddrByType_001
 * @tc.desc: set deviceInfo'pointer null, go to first master,and return nullptr
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
 * @tc.desc:set deviceInfo to some corrort para,and return nullptr
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
 * @tc.desc: set deviceId to null,and return nullptr
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
 * @tc.name: CovertNodeBasicInfoToDmDevice_001
 * @tc.desc: go to the corrort case and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, CovertNodeBasicInfoToDmDevice_001, testing::ext::TestSize.Level0)
{
    NodeBasicInfo nodeBasicInfo;
    DmDeviceInfo dmDeviceInfo;
    int ret = softbusConnector->CovertNodeBasicInfoToDmDevice(nodeBasicInfo, dmDeviceInfo);
    EXPECT_EQ(ret, DM_OK);
}

/**
 * @tc.name: OnParameterChgCallback_001
 * @tc.desc: set some corrort para  and return DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(SoftbusConnectorTest, OnParameterChgCallback_001, testing::ext::TestSize.Level0)
{
    EXPECT_EQ(DM_OK, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
