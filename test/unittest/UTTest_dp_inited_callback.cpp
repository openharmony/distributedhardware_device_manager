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
    std::string extraData = jsonObject.Dump();
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
    std::string extraData = jsonObject.Dump();
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
    std::string extraData = jsonObject.Dump();
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
    std::string extraData = jsonObject.Dump();
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
    std::string extraData = jsonObject.Dump();
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
    std::string extraData = jsonObject.Dump();
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

/**
 * @tc.name: ConvertToTrustedDeviceInfo_001
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.1";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);
    deviceInfo.deviceTypeId = 1001;

    std::string udid = "mate-60";
    std::string uuid = "uuid-mate-60";
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(uuid), Return(DM_OK)));

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_TRUE(convertRet);
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_002
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_002, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.2";
    std::string extraData;
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::unordered_map<std::string, DmAuthForm> authFormMap;
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;
    DpInitedCallback dpInitedCallback;

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_FALSE(convertRet);
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_003
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_003, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.3";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "mate-60";
    std::string uuid = "uuid-mate-60";
    std::unordered_map<std::string, DmAuthForm> authFormMap;
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(uuid), Return(DM_OK)));

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_FALSE(convertRet);
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_004
 * @tc.desc: extraData is not a valid json object, extraJson.IsDiscarded() returns true.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_004, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.4";
    // A malformed json string that fails to parse, so JsonObject::IsDiscarded() is true.
    std::string extraData = "{invalid_json";
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::unordered_map<std::string, DmAuthForm> authFormMap;
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_FALSE(convertRet);
}

/**
 * @tc.name: PutAllTrustedDevices_010
 * @tc.desc: All trusted devices fail to convert, so deviceInfos is empty and PutAllTrustedDevices returns early.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_010, testing::ext::TestSize.Level2)
{
    DpInitedCallback dpInitedCallback;
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.10";
    std::string extraData;
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::vector<DmDeviceInfo> deviceInfos = { deviceInfo };
    std::unordered_map<std::string, DmAuthForm> authFormMap;

    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);

    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _)).WillOnce(Return(authFormMap));

    // @tc.expect: extraData is empty so ConvertToTrustedDeviceInfo fails, deviceInfos empty, return early.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: PutAllTrustedDevices_011
 * @tc.desc: Multiple devices where only one matches authFormMap; the other fails convert, deviceInfos non-empty.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_011, testing::ext::TestSize.Level2)
{
    DpInitedCallback dpInitedCallback;
    // First device: valid extraData and will match authForm.
    DmDeviceInfo validDevice;
    char validNetworkId[] = "255.255.255.11";
    JsonObject validJson;
    validJson[PARAM_KEY_OS_VERSION] = "5.0.3";
    validJson[PARAM_KEY_OS_TYPE] = 1;
    std::string validExtra = validJson.Dump();
    auto validRet = CreateDmDeviceInfo(validNetworkId, validExtra, validDevice);
    ASSERT_TRUE(validRet);

    // Second device: empty extraData, will fail to convert.
    DmDeviceInfo invalidDevice;
    char invalidNetworkId[] = "255.255.255.12";
    auto invalidRet = CreateDmDeviceInfo(invalidNetworkId, std::string(), invalidDevice);
    ASSERT_TRUE(invalidRet);

    std::string udid = "mate-11";
    std::vector<DmDeviceInfo> deviceInfos = { validDevice, invalidDevice };
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };

    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);

    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>("uuid-11"), Return(DM_OK)));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _)).WillOnce(Return(authFormMap));

    // @tc.expect: only valid device converts; deviceInfos non-empty so PutAllTrustedDevices is called.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_005
 * @tc.desc: extraData json contains neither OS_VERSION nor OS_TYPE keys (IsString/IsInt32 false paths).
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_005, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.5";
    JsonObject jsonObject;
    jsonObject["unrelatedKey"] = "value";
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::unordered_map<std::string, DmAuthForm> authFormMap;
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    // @tc.expect: osVersion key absent so IsString fails, returns false.
    EXPECT_FALSE(convertRet);
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_006
 * @tc.desc: OS_VERSION present (string) but OS_TYPE key absent (IsInt32 false path).
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_006, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.6";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::unordered_map<std::string, DmAuthForm> authFormMap;
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    // @tc.expect: osType absent so IsInt32 fails, returns false.
    EXPECT_FALSE(convertRet);
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_007
 * @tc.desc: GetUdidFromCache returns DM_OK with empty udid; authForm lookup with empty key misses.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_007, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.7";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string emptyUdid = "";
    std::string uuid = "uuid-7";
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {"nonEmptyKey", PEER_TO_PEER} };
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(emptyUdid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(uuid), Return(DM_OK)));

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    // @tc.expect: empty udid not found in authFormMap, returns false.
    EXPECT_FALSE(convertRet);
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_008
 * @tc.desc: Full success path verifying networkId and deviceTypeId are assigned at top of function.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_008, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.8";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);
    deviceInfo.deviceTypeId = 0x0E;

    std::string udid = "mate-8";
    std::string uuid = "uuid-8";
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, ACROSS_ACCOUNT} };
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(uuid), Return(DM_OK)));

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_TRUE(convertRet);
    EXPECT_EQ(trustedDeviceInfo.GetDeviceTypeId(), static_cast<uint16_t>(0x0E));
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_009
 * @tc.desc: Full success path verifying osVersion and osType are correctly parsed from extraData.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_009, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.20";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "10.0.0";
    jsonObject[PARAM_KEY_OS_TYPE] = 2;
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "mate-20";
    std::string uuid = "uuid-20";
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(uuid), Return(DM_OK)));

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_TRUE(convertRet);
    EXPECT_EQ(trustedDeviceInfo.GetOsVersion(), "10.0.0");
    EXPECT_EQ(trustedDeviceInfo.GetOsType(), 2);
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_010
 * @tc.desc: Full success path verifying udid and uuid are set from the softbus cache.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_010, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.21";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "udid-21";
    std::string uuid = "uuid-21";
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, IDENTICAL_ACCOUNT} };
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(uuid), Return(DM_OK)));

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_TRUE(convertRet);
    EXPECT_EQ(trustedDeviceInfo.GetUdid(), udid);
    EXPECT_EQ(trustedDeviceInfo.GetUuid(), uuid);
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_011
 * @tc.desc: Full success path verifying networkId and authForm are set correctly.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_011, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.22";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);

    std::string udid = "udid-22";
    std::string uuid = "uuid-22";
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, SHARE} };
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(uuid), Return(DM_OK)));

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_TRUE(convertRet);
    EXPECT_EQ(trustedDeviceInfo.GetNetworkId(), std::string(networkId));
    EXPECT_EQ(trustedDeviceInfo.GetAuthForm(), static_cast<int32_t>(SHARE));
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_012
 * @tc.desc: Verify deviceTypeId 0 is correctly set (zero-type-id boundary).
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_012, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.23";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "5.0.3";
    jsonObject[PARAM_KEY_OS_TYPE] = 1;
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);
    deviceInfo.deviceTypeId = 0;

    std::string udid = "udid-23";
    std::string uuid = "uuid-23";
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(uuid), Return(DM_OK)));

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_TRUE(convertRet);
    EXPECT_EQ(trustedDeviceInfo.GetDeviceTypeId(), static_cast<uint16_t>(0));
}

/**
 * @tc.name: PutAllTrustedDevices_012
 * @tc.desc: Multiple valid devices that all convert successfully; deviceInfos is non-empty.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, PutAllTrustedDevices_012, testing::ext::TestSize.Level1)
{
    DpInitedCallback dpInitedCallback;
    // First valid device.
    DmDeviceInfo device1;
    char networkId1[] = "255.255.255.31";
    JsonObject json1;
    json1[PARAM_KEY_OS_VERSION] = "5.0.3";
    json1[PARAM_KEY_OS_TYPE] = 1;
    std::string extra1 = json1.Dump();
    auto ret1 = CreateDmDeviceInfo(networkId1, extra1, device1);
    ASSERT_TRUE(ret1);

    // Second valid device.
    DmDeviceInfo device2;
    char networkId2[] = "255.255.255.32";
    JsonObject json2;
    json2[PARAM_KEY_OS_VERSION] = "10.0.0";
    json2[PARAM_KEY_OS_TYPE] = 2;
    std::string extra2 = json2.Dump();
    auto ret2 = CreateDmDeviceInfo(networkId2, extra2, device2);
    ASSERT_TRUE(ret2);

    std::string udid = "mate-31";
    std::vector<DmDeviceInfo> deviceInfos = { device1, device2 };
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, PEER_TO_PEER} };

    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    auto dmDeviceProfileConnectorMock = std::static_pointer_cast<DeviceProfileConnectorMock>(
        DmDeviceProfileConnector::dmDeviceProfileConnector);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    ASSERT_TRUE(dmDeviceProfileConnectorMock != nullptr);

    EXPECT_CALL(*softbusCacheMock, GetDeviceInfoFromCache(_))
        .WillOnce(DoAll(SetArgReferee<0>(deviceInfos), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _))
        .Times(2).WillRepeatedly(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _))
        .Times(2).WillRepeatedly(DoAll(SetArgReferee<1>(std::string("uuid-mock")), Return(DM_OK)));
    EXPECT_CALL(*dmDeviceProfileConnectorMock, GetAppTrustDeviceList(_, _)).WillOnce(Return(authFormMap));

    // @tc.expect: both devices convert, deviceInfos non-empty, PutAllTrustedDevices invoked.
    dpInitedCallback.PutAllTrustedDevices();
}

/**
 * @tc.name: ConvertToTrustedDeviceInfo_013
 * @tc.desc: OS_TYPE with a larger int value is correctly parsed and set.
 * @tc.type: FUNC
 */
HWTEST_F(DpInitedCallbackTest, ConvertToTrustedDeviceInfo_013, testing::ext::TestSize.Level2)
{
    DmDeviceInfo deviceInfo;
    char networkId[] = "255.255.255.33";
    JsonObject jsonObject;
    jsonObject[PARAM_KEY_OS_VERSION] = "GenericOS";
    jsonObject[PARAM_KEY_OS_TYPE] = 0x100;
    std::string extraData = jsonObject.Dump();
    auto ret = CreateDmDeviceInfo(networkId, extraData, deviceInfo);
    ASSERT_TRUE(ret);
    deviceInfo.deviceTypeId = 0x0900;

    std::string udid = "udid-33";
    std::string uuid = "uuid-33";
    std::unordered_map<std::string, DmAuthForm> authFormMap = { {udid, ACROSS_ACCOUNT} };
    DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;

    DpInitedCallback dpInitedCallback;
    auto softbusCacheMock = std::static_pointer_cast<SoftbusCacheMock>(DmSoftbusCache::dmSoftbusCache);
    ASSERT_TRUE(softbusCacheMock != nullptr);
    EXPECT_CALL(*softbusCacheMock, GetUdidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(udid), Return(DM_OK)));
    EXPECT_CALL(*softbusCacheMock, GetUuidFromCache(_, _)).WillOnce(DoAll(SetArgReferee<1>(uuid), Return(DM_OK)));

    bool convertRet = dpInitedCallback.ConvertToTrustedDeviceInfo(authFormMap, deviceInfo, trustedDeviceInfo);
    EXPECT_TRUE(convertRet);
    EXPECT_EQ(trustedDeviceInfo.GetOsType(), 0x100);
    EXPECT_EQ(trustedDeviceInfo.GetOsVersion(), "GenericOS");
    EXPECT_EQ(trustedDeviceInfo.GetDeviceTypeId(), static_cast<uint16_t>(0x0900));
}

} // DistributedHardware
} // OHOS