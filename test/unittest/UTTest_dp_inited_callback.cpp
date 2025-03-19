/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "UTTest_dp_inited_callback.h"
#include "deviceprofile_connector_mock.h"
#include "dm_softbus_cache_mock.h"
#include "dm_constants.h"
#include "dm_anonymous.h"
#include "nlohmann/json.hpp"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;

namespace OHOS {
namespace DistributedHardware {
bool CreateDmDeviceInfo(const char* networkId, const std::string& extraData, DmDeviceInfo& deviceInfo)
{
    if (memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo)) != 0) {
        return false;
    }
    if (memcpy_s(deviceInfo.networkId, sizeof(deviceInfo.networkId), networkId, strlen(networkId)) != 0) {
        return false;
    }
    if (!extraData.empty()) {
        deviceInfo.extraData = extraData;
    }
    return true;
}

void DpInitedCallbackTest::SetUp()
{
    DmSoftbusCache::dmSoftbusCache = std::make_shared<SoftbusCacheMock>();
    DmDeviceProfileConnector::dmDeviceProfileConnector = std::make_shared<DeviceProfileConnectorMock>();
}

void DpInitedCallbackTest::TearDown()
{
    DmSoftbusCache::dmSoftbusCache = nullptr;
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
}

void DpInitedCallbackTest::SetUpTestCase()
{}

void DpInitedCallbackTest::TearDownTestCase()
{}

/**
 * @tc.name: PutAllTrustedDevices_001
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.0";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = SafetyDump(jsonObject);
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "mate-60";
    std::vector<DmDeviceInfo> deviceInfos = { deviceInfo };
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);

    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);

    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
                                    .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>("lmMock"), Return(DM_OK)));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _)).WillOnce(Return(authFormMap));

    // @tc.expect: put all trusted devices.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: PutAllTrustedDevices_002
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_002, testing::ext::TestSize.Level2)
{
    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_)).WillOnce(Return(ERR_DM_FAILED));

    // @tc.expect: GetDeviceInfoFromCache is error
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: PutAllTrustedDevices_003
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_003, testing::ext::TestSize.Level2)
{
    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_)).WillOnce(Return(DM_OK));
    
    // @tc.expect: dmDeviceInfos is empty.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: PutAllTrustedDevices_004
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_004, testing::ext::TestSize.Level2)
{
    DpInitedCallback dpInitedCallback;
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.0";
    std::string extraData;
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "mate-60";
    std::vector<DmDeviceInfo> deviceInfos = { deviceInfo };
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };

    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);
    
    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _)).WillOnce(Return(authFormMap));

    // @tc.expect: extraData is empty.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: PutAllTrustedDevices_005
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_005, testing::ext::TestSize.Level2)
{
    DpInitedCallback dpInitedCallback;
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.0";

    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = 1;
    std::string extraData = SafetyDump(jsonObject);
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "mate-60";
    std::vector<DmDeviceInfo> deviceInfos = { deviceInfo };
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };

    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);

    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _)).WillOnce(Return(authFormMap));

    // @tc.expect: PARAM_KEY_OS_VERSION value type is not string.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: PutAllTrustedDevices_006
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_006, testing::ext::TestSize.Level2)
{
    DpInitedCallback dpInitedCallback;
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.0";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = "1";
    std::string extraData = SafetyDump(jsonObject);
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "mate-60";
    std::vector<DmDeviceInfo> deviceInfos = { deviceInfo };

    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);

    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _))
                                    .WillOnce(Return(std::unordered_map<std::string, DmAuthForm>()));

    // @tc.expect: PARAM_KEY_OS_TYPE value type is not int.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: PutAllTrustedDevices_007
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_007, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.0";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = SafetyDump(jsonObject);
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "mate-60";
    std::vector<DmDeviceInfo> deviceInfos = { deviceInfo };
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);

    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);

    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
                                    .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _)).WillOnce(Return(authFormMap));

    // @tc.expect: GetUdidFromCache return error.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: PutAllTrustedDevices_008
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_008, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.0";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = SafetyDump(jsonObject);
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);
    std::vector<DmDeviceInfo> deviceInfos = { deviceInfo };

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);

    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);

    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
                                    .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _))
                                                .WillOnce(Return(std::unordered_map<std::string, DmAuthForm>()));

    // @tc.expect: GetUuidFromCache return error.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: PutAllTrustedDevices_009
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_009, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.0";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = SafetyDump(jsonObject);
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "mate-60";
    std::vector<DmDeviceInfo> deviceInfos = { deviceInfo };
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);

    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);

    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
                                    .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _)).WillOnce(Return(authFormMap));

    // @tc.expect: authFormMap find udid failed.
    dpInitedCallback.PutAllTrustedDevices();
}
} // DistributedHardware
} // OHOS