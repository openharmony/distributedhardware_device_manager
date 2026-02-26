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

#include "UTTest_ipc_model_codec_test.h"
#include "dm_constants.h"
#include "dm_subscribe_info.h"

namespace OHOS {
namespace DistributedHardware {

void IpcModelCodecTest::SetUpTestCase()
{
}

void IpcModelCodecTest::TearDownTestCase()
{
}

void IpcModelCodecTest::SetUp()
{
}

void IpcModelCodecTest::TearDown()
{
}

/**
 * @tc.name: EncodeDmRegServiceInfo_001
 * @tc.desc: EncodeDmRegServiceInfo with normal input
 *           Step 1: Prepare DmRegisterServiceInfo with normal values
 *           Step 2: Call EncodeDmRegServiceInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmRegServiceInfo_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.userId = 100;
    regServiceInfo.displayId = 1000;
    regServiceInfo.serviceOwnerTokenId = 12345;
    regServiceInfo.serviceOwnerPkgName = "com.test.owner";
    regServiceInfo.serviceRegisterTokenId = 54321;
    regServiceInfo.serviceType = "testType";
    regServiceInfo.serviceName = "testServiceName";
    regServiceInfo.serviceDisplayName = "Test Display Name";
    regServiceInfo.serviceCode = "testCode";
    regServiceInfo.customData = "custom data";
    regServiceInfo.dataLen = 100;
    regServiceInfo.timeStamp = 1234567890;
    regServiceInfo.description = "test description";

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmRegServiceInfo(regServiceInfo, parcel);
    EXPECT_EQ(ret, true);

    DmRegisterServiceInfo decodedInfo;
    bool decodeRet = IpcModelCodec::DecodeDmRegServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedInfo.userId, 100);
    EXPECT_EQ(decodedInfo.displayId, 1000);
    EXPECT_EQ(decodedInfo.serviceOwnerTokenId, 12345);
    EXPECT_EQ(decodedInfo.serviceOwnerPkgName, "com.test.owner");
    EXPECT_EQ(decodedInfo.serviceRegisterTokenId, 54321);
    EXPECT_EQ(decodedInfo.serviceType, "testType");
    EXPECT_EQ(decodedInfo.serviceName, "testServiceName");
    EXPECT_EQ(decodedInfo.serviceDisplayName, "Test Display Name");
    EXPECT_EQ(decodedInfo.serviceCode, "testCode");
    EXPECT_EQ(decodedInfo.customData, "custom data");
    EXPECT_EQ(decodedInfo.dataLen, 100);
    EXPECT_EQ(decodedInfo.timeStamp, 1234567890);
    EXPECT_EQ(decodedInfo.description, "test description");
}

/**
 * @tc.name: EncodeDmRegServiceInfo_002
 * @tc.desc: EncodeDmRegServiceInfo with empty strings
 *           Step 1: Prepare DmRegisterServiceInfo with empty string values
 *           Step 2: Call EncodeDmRegServiceInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmRegServiceInfo_002, testing::ext::TestSize.Level1)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.userId = 0;
    regServiceInfo.displayId = 0;
    regServiceInfo.serviceOwnerTokenId = 0;
    regServiceInfo.serviceOwnerPkgName = "";
    regServiceInfo.serviceRegisterTokenId = 0;
    regServiceInfo.serviceType = "";
    regServiceInfo.serviceName = "";
    regServiceInfo.serviceDisplayName = "";
    regServiceInfo.serviceCode = "";
    regServiceInfo.customData = "";
    regServiceInfo.dataLen = 0;
    regServiceInfo.timeStamp = 0;
    regServiceInfo.description = "";

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmRegServiceInfo(regServiceInfo, parcel);
    EXPECT_EQ(ret, true);

    DmRegisterServiceInfo decodedInfo;
    bool decodeRet = IpcModelCodec::DecodeDmRegServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedInfo.userId, 0);
    EXPECT_EQ(decodedInfo.serviceOwnerPkgName, "");
    EXPECT_EQ(decodedInfo.serviceType, "");
}

/**
 * @tc.name: DecodeDmRegServiceInfo_001
 * @tc.desc: DecodeDmRegServiceInfo with normal parcel data
 *           Step 1: Prepare MessageParcel with encoded data
 *           Step 2: Call DecodeDmRegServiceInfo
 *           Step 3: Verify decoded values match original data
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, DecodeDmRegServiceInfo_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceInfo originalInfo;
    originalInfo.userId = 200;
    originalInfo.displayId = 2000;
    originalInfo.serviceOwnerTokenId = 22345;
    originalInfo.serviceOwnerPkgName = "com.test.owner2";
    originalInfo.serviceRegisterTokenId = 254321;
    originalInfo.serviceType = "testType2";
    originalInfo.serviceName = "testServiceName2";
    originalInfo.serviceDisplayName = "Test Display Name 2";
    originalInfo.serviceCode = "testCode2";
    originalInfo.customData = "custom data 2";
    originalInfo.dataLen = 200;
    originalInfo.timeStamp = 2234567890;
    originalInfo.description = "test description 2";

    MessageParcel parcel;
    IpcModelCodec::EncodeDmRegServiceInfo(originalInfo, parcel);

    DmRegisterServiceInfo decodedInfo;
    bool ret = IpcModelCodec::DecodeDmRegServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(decodedInfo.userId, 200);
    EXPECT_EQ(decodedInfo.displayId, 2000);
    EXPECT_EQ(decodedInfo.serviceOwnerTokenId, 22345);
    EXPECT_EQ(decodedInfo.serviceOwnerPkgName, "com.test.owner2");
    EXPECT_EQ(decodedInfo.serviceRegisterTokenId, 254321);
}

/**
 * @tc.name: EncodeDmSrvDiscParam_001
 * @tc.desc: EncodeDmSrvDiscParam with normal input
 *           Step 1: Prepare DmDiscoveryServiceParam with normal values
 *           Step 2: Call EncodeDmSrvDiscParam
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmSrvDiscParam_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    param.serviceName = "testService";
    param.serviceType = "testType";
    param.serviceDisplayName = "Test Service Display";
    param.freq = DmExchangeFreq::DM_HIGH;
    param.medium = DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE;
    param.mode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmSrvDiscParam(param, parcel);
    EXPECT_EQ(ret, true);

    DmDiscoveryServiceParam decodedParam;
    bool decodeRet = IpcModelCodec::DecodeDmSrvDiscParam(parcel, decodedParam);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedParam.serviceName, "testService");
    EXPECT_EQ(decodedParam.serviceType, "testType");
    EXPECT_EQ(decodedParam.serviceDisplayName, "Test Service Display");
    EXPECT_EQ(decodedParam.freq, DmExchangeFreq::DM_HIGH);
    EXPECT_EQ(decodedParam.medium, DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE);
    EXPECT_EQ(decodedParam.mode, DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE);
}

/**
 * @tc.name: EncodeDmSrvDiscParam_002
 * @tc.desc: EncodeDmSrvDiscParam with different enum values
 *           Step 1: Prepare DmDiscoveryServiceParam with PASSIVE mode and AUTO medium
 *           Step 2: Call EncodeDmSrvDiscParam
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmSrvDiscParam_002, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    param.serviceName = "testService2";
    param.serviceType = "testType2";
    param.serviceDisplayName = "Test Service Display 2";
    param.freq = DmExchangeFreq::DM_LOW;
    param.medium = DMSrvMediumType::SERVICE_MEDIUM_TYPE_AUTO;
    param.mode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_PASSIVE;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmSrvDiscParam(param, parcel);
    EXPECT_EQ(ret, true);

    DmDiscoveryServiceParam decodedParam;
    bool decodeRet = IpcModelCodec::DecodeDmSrvDiscParam(parcel, decodedParam);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedParam.freq, DmExchangeFreq::DM_LOW);
    EXPECT_EQ(decodedParam.medium, DMSrvMediumType::SERVICE_MEDIUM_TYPE_AUTO);
    EXPECT_EQ(decodedParam.mode, DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_PASSIVE);
}

/**
 * @tc.name: DecodeDmSrvDiscParam_001
 * @tc.desc: DecodeDmSrvDiscParam with normal parcel data
 *           Step 1: Prepare MessageParcel with encoded data
 *           Step 2: Call DecodeDmSrvDiscParam
 *           Step 3: Verify decoded values match original data
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, DecodeDmSrvDiscParam_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam originalParam;
    originalParam.serviceName = "discoverService";
    originalParam.serviceType = "discoverType";
    originalParam.serviceDisplayName = "Discover Service";
    originalParam.freq = DmExchangeFreq::DM_MID;
    originalParam.medium = DMSrvMediumType::SERVICE_MEDIUM_TYPE_MDNS;
    originalParam.mode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE;

    MessageParcel parcel;
    IpcModelCodec::EncodeDmSrvDiscParam(originalParam, parcel);

    DmDiscoveryServiceParam decodedParam;
    bool ret = IpcModelCodec::DecodeDmSrvDiscParam(parcel, decodedParam);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(decodedParam.serviceName, "discoverService");
    EXPECT_EQ(decodedParam.serviceType, "discoverType");
    EXPECT_EQ(decodedParam.freq, DmExchangeFreq::DM_MID);
    EXPECT_EQ(decodedParam.medium, DMSrvMediumType::SERVICE_MEDIUM_TYPE_MDNS);
}

/**
 * @tc.name: EncodeDmPublishServiceParam_001
 * @tc.desc: EncodeDmPublishServiceParam with ACTIVE mode
 *           Step 1: Prepare DmPublishServiceParam with ACTIVE mode and BLE medium
 *           Step 2: Call EncodeDmPublishServiceParam
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmPublishServiceParam_001, testing::ext::TestSize.Level1)
{
    DmPublishServiceParam publishServiceParam;
    publishServiceParam.discoverMode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE;
    publishServiceParam.media = DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE;
    publishServiceParam.freq = DmExchangeFreq::DM_HIGH;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmPublishServiceParam(publishServiceParam, parcel);
    EXPECT_EQ(ret, true);

    DmPublishServiceParam decodedParam;
    bool decodeRet = IpcModelCodec::DecodeDmPublishServiceParam(parcel, decodedParam);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedParam.discoverMode, DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE);
    EXPECT_EQ(decodedParam.media, DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE);
    EXPECT_EQ(decodedParam.freq, DmExchangeFreq::DM_HIGH);
}

/**
 * @tc.name: EncodeDmPublishServiceParam_002
 * @tc.desc: EncodeDmPublishServiceParam with PASSIVE mode and AUTO medium
 *           Step 1: Prepare DmPublishServiceParam with PASSIVE mode and AUTO medium
 *           Step 2: Call EncodeDmPublishServiceParam
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmPublishServiceParam_002, testing::ext::TestSize.Level1)
{
    DmPublishServiceParam publishServiceParam;
    publishServiceParam.discoverMode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_PASSIVE;
    publishServiceParam.media = DMSrvMediumType::SERVICE_MEDIUM_TYPE_AUTO;
    publishServiceParam.freq = DmExchangeFreq::DM_LOW;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmPublishServiceParam(publishServiceParam, parcel);
    EXPECT_EQ(ret, true);

    DmPublishServiceParam decodedParam;
    bool decodeRet = IpcModelCodec::DecodeDmPublishServiceParam(parcel, decodedParam);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedParam.discoverMode, DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_PASSIVE);
    EXPECT_EQ(decodedParam.media, DMSrvMediumType::SERVICE_MEDIUM_TYPE_AUTO);
    EXPECT_EQ(decodedParam.freq, DmExchangeFreq::DM_LOW);
}

/**
 * @tc.name: DecodeDmPublishServiceParam_001
 * @tc.desc: DecodeDmPublishServiceParam with BLE_TRIGGER medium
 *           Step 1: Prepare MessageParcel with encoded data
 *           Step 2: Call DecodeDmPublishServiceParam
 *           Step 3: Verify decoded values match original data
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, DecodeDmPublishServiceParam_001, testing::ext::TestSize.Level1)
{
    DmPublishServiceParam originalParam;
    originalParam.discoverMode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE;
    originalParam.media = DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE_TRIGGER;
    originalParam.freq = DmExchangeFreq::DM_MID;

    MessageParcel parcel;
    IpcModelCodec::EncodeDmPublishServiceParam(originalParam, parcel);

    DmPublishServiceParam decodedParam;
    bool ret = IpcModelCodec::DecodeDmPublishServiceParam(parcel, decodedParam);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(decodedParam.discoverMode, DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE);
    EXPECT_EQ(decodedParam.media, DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE_TRIGGER);
    EXPECT_EQ(decodedParam.freq, DmExchangeFreq::DM_MID);
}

/**
 * @tc.name: EncodeDmServiceInfo_001
 * @tc.desc: EncodeDmServiceInfo with normal input
 *           Step 1: Prepare DmServiceInfo with normal values
 *           Step 2: Call EncodeDmServiceInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmServiceInfo_001, testing::ext::TestSize.Level1)
{
    DmServiceInfo serviceInfo;
    serviceInfo.userId = 100;
    serviceInfo.serviceId = 1000;
    serviceInfo.displayId = 5000;
    serviceInfo.deviceId = "device123";
    serviceInfo.networkId = "network123";
    serviceInfo.serviceOwnerTokenId = 11111;
    serviceInfo.serviceOwnerPkgName = "com.owner.pkg";
    serviceInfo.serviceRegisterTokenId = 22222;
    serviceInfo.serviceType = "serviceType";
    serviceInfo.serviceName = "serviceName";
    serviceInfo.serviceDisplayName = "Service Display Name";
    serviceInfo.publishState = 1;
    serviceInfo.serviceCode = "serviceCode";
    serviceInfo.customData = "custom data";
    serviceInfo.dataLen = 50;
    serviceInfo.timeStamp = 9876543210;
    serviceInfo.description = "service description";
    serviceInfo.authform = DmAuthForm::PEER_TO_PEER;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmServiceInfo(serviceInfo, parcel);
    EXPECT_EQ(ret, true);

    DmServiceInfo decodedInfo;
    bool decodeRet = IpcModelCodec::DecodeDmServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedInfo.userId, 100);
    EXPECT_EQ(decodedInfo.serviceId, 1000);
    EXPECT_EQ(decodedInfo.displayId, 5000);
    EXPECT_EQ(decodedInfo.deviceId, "device123");
    EXPECT_EQ(decodedInfo.networkId, "network123");
    EXPECT_EQ(decodedInfo.serviceOwnerTokenId, 11111);
    EXPECT_EQ(decodedInfo.serviceOwnerPkgName, "com.owner.pkg");
    EXPECT_EQ(decodedInfo.serviceRegisterTokenId, 22222);
    EXPECT_EQ(decodedInfo.serviceType, "serviceType");
    EXPECT_EQ(decodedInfo.serviceName, "serviceName");
    EXPECT_EQ(decodedInfo.serviceDisplayName, "Service Display Name");
    EXPECT_EQ(decodedInfo.publishState, 1);
    EXPECT_EQ(decodedInfo.serviceCode, "serviceCode");
    EXPECT_EQ(decodedInfo.customData, "custom data");
    EXPECT_EQ(decodedInfo.dataLen, 50);
    EXPECT_EQ(decodedInfo.timeStamp, 9876543210);
    EXPECT_EQ(decodedInfo.description, "service description");
    EXPECT_EQ(decodedInfo.authform, DmAuthForm::PEER_TO_PEER);
}

/**
 * @tc.name: EncodeDmServiceInfo_002
 * @tc.desc: EncodeDmServiceInfo with IDENTICAL_ACCOUNT authform
 *           Step 1: Prepare DmServiceInfo with IDENTICAL_ACCOUNT authform
 *           Step 2: Call EncodeDmServiceInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmServiceInfo_002, testing::ext::TestSize.Level1)
{
    DmServiceInfo serviceInfo;
    serviceInfo.userId = 200;
    serviceInfo.serviceId = 2000;
    serviceInfo.displayId = 6000;
    serviceInfo.deviceId = "device456";
    serviceInfo.networkId = "network456";
    serviceInfo.serviceOwnerTokenId = 33333;
    serviceInfo.serviceOwnerPkgName = "com.owner.pkg2";
    serviceInfo.serviceRegisterTokenId = 44444;
    serviceInfo.serviceType = "serviceType2";
    serviceInfo.serviceName = "serviceName2";
    serviceInfo.serviceDisplayName = "Service Display Name 2";
    serviceInfo.publishState = 0;
    serviceInfo.serviceCode = "serviceCode2";
    serviceInfo.customData = "custom data 2";
    serviceInfo.dataLen = 60;
    serviceInfo.timeStamp = 9876543211;
    serviceInfo.description = "service description 2";
    serviceInfo.authform = DmAuthForm::IDENTICAL_ACCOUNT;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmServiceInfo(serviceInfo, parcel);
    EXPECT_EQ(ret, true);

    DmServiceInfo decodedInfo;
    bool decodeRet = IpcModelCodec::DecodeDmServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedInfo.userId, 200);
    EXPECT_EQ(decodedInfo.serviceId, 2000);
    EXPECT_EQ(decodedInfo.publishState, 0);
    EXPECT_EQ(decodedInfo.authform, DmAuthForm::IDENTICAL_ACCOUNT);
}

/**
 * @tc.name: DecodeDmServiceInfo_001
 * @tc.desc: DecodeDmServiceInfo with normal parcel data
 *           Step 1: Prepare MessageParcel with encoded data
 *           Step 2: Call DecodeDmServiceInfo
 *           Step 3: Verify decoded values match original data
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, DecodeDmServiceInfo_001, testing::ext::TestSize.Level1)
{
    DmServiceInfo originalInfo;
    originalInfo.userId = 300;
    originalInfo.serviceId = 3000;
    originalInfo.displayId = 7000;
    originalInfo.deviceId = "device789";
    originalInfo.networkId = "network789";
    originalInfo.serviceOwnerTokenId = 55555;
    originalInfo.serviceOwnerPkgName = "com.owner.pkg3";
    originalInfo.serviceRegisterTokenId = 66666;
    originalInfo.serviceType = "serviceType3";
    originalInfo.serviceName = "serviceName3";
    originalInfo.serviceDisplayName = "Service Display Name 3";
    originalInfo.publishState = 2;
    originalInfo.serviceCode = "serviceCode3";
    originalInfo.customData = "custom data 3";
    originalInfo.dataLen = 70;
    originalInfo.timeStamp = 9876543212;
    originalInfo.description = "service description 3";
    originalInfo.authform = DmAuthForm::ACROSS_ACCOUNT;

    MessageParcel parcel;
    IpcModelCodec::EncodeDmServiceInfo(originalInfo, parcel);

    DmServiceInfo decodedInfo;
    bool ret = IpcModelCodec::DecodeDmServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(decodedInfo.userId, 300);
    EXPECT_EQ(decodedInfo.serviceId, 3000);
    EXPECT_EQ(decodedInfo.publishState, 2);
    EXPECT_EQ(decodedInfo.authform, DmAuthForm::ACROSS_ACCOUNT);
}

/**
 * @tc.name: DecodeDmServiceInfo_002
 * @tc.desc: DecodeDmServiceInfo with SHARE authform
 *           Step 1: Prepare MessageParcel with encoded data with SHARE authform
 *           Step 2: Call DecodeDmServiceInfo
 *           Step 3: Verify decoded authform is SHARE
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, DecodeDmServiceInfo_002, testing::ext::TestSize.Level1)
{
    DmServiceInfo originalInfo;
    originalInfo.userId = 400;
    originalInfo.serviceId = 4000;
    originalInfo.deviceId = "device999";
    originalInfo.networkId = "network999";
    originalInfo.serviceOwnerTokenId = 77777;
    originalInfo.serviceOwnerPkgName = "com.owner.pkg4";
    originalInfo.serviceRegisterTokenId = 88888;
    originalInfo.serviceType = "serviceType4";
    originalInfo.serviceName = "serviceName4";
    originalInfo.serviceDisplayName = "Service Display Name 4";
    originalInfo.publishState = 1;
    originalInfo.serviceCode = "serviceCode4";
    originalInfo.customData = "custom data 4";
    originalInfo.dataLen = 80;
    originalInfo.timeStamp = 9876543213;
    originalInfo.description = "service description 4";
    originalInfo.authform = DmAuthForm::SHARE;

    MessageParcel parcel;
    IpcModelCodec::EncodeDmServiceInfo(originalInfo, parcel);

    DmServiceInfo decodedInfo;
    bool ret = IpcModelCodec::DecodeDmServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(decodedInfo.authform, DmAuthForm::SHARE);
}

/**
 * @tc.name: EncodeDmRegisterServiceState_001
 * @tc.desc: EncodeDmRegisterServiceState with normal input
 *           Step 1: Prepare DmRegisterServiceState with normal values
 *           Step 2: Call EncodeDmRegisterServiceState
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmRegisterServiceState_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState serviceState;
    serviceState.userId = 100;
    serviceState.tokenId = 12345;
    serviceState.pkgName = "com.test.package";
    serviceState.serviceId = 9999;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmRegisterServiceState(serviceState, parcel);
    EXPECT_EQ(ret, true);

    DmRegisterServiceState decodedState;
    bool decodeRet = IpcModelCodec::DecodeDmRegisterServiceState(parcel, decodedState);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedState.userId, 100);
    EXPECT_EQ(decodedState.tokenId, 12345);
    EXPECT_EQ(decodedState.pkgName, "com.test.package");
    EXPECT_EQ(decodedState.serviceId, 9999);
}

/**
 * @tc.name: EncodeDmRegisterServiceState_002
 * @tc.desc: EncodeDmRegisterServiceState with zero values
 *           Step 1: Prepare DmRegisterServiceState with zero values
 *           Step 2: Call EncodeDmRegisterServiceState
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmRegisterServiceState_002, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState serviceState;
    serviceState.userId = 0;
    serviceState.tokenId = 0;
    serviceState.pkgName = "";
    serviceState.serviceId = 0;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmRegisterServiceState(serviceState, parcel);
    EXPECT_EQ(ret, true);

    DmRegisterServiceState decodedState;
    bool decodeRet = IpcModelCodec::DecodeDmRegisterServiceState(parcel, decodedState);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedState.userId, 0);
    EXPECT_EQ(decodedState.tokenId, 0);
    EXPECT_EQ(decodedState.pkgName, "");
    EXPECT_EQ(decodedState.serviceId, 0);
}

/**
 * @tc.name: DecodeDmRegisterServiceState_001
 * @tc.desc: DecodeDmRegisterServiceState with normal parcel data
 *           Step 1: Prepare MessageParcel with encoded data
 *           Step 2: Call DecodeDmRegisterServiceState
 *           Step 3: Verify decoded values match original data
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, DecodeDmRegisterServiceState_001, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState originalState;
    originalState.userId = 200;
    originalState.tokenId = 54321;
    originalState.pkgName = "com.test.package2";
    originalState.serviceId = 8888;

    MessageParcel parcel;
    IpcModelCodec::EncodeDmRegisterServiceState(originalState, parcel);

    DmRegisterServiceState decodedState;
    bool ret = IpcModelCodec::DecodeDmRegisterServiceState(parcel, decodedState);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(decodedState.userId, 200);
    EXPECT_EQ(decodedState.tokenId, 54321);
    EXPECT_EQ(decodedState.pkgName, "com.test.package2");
    EXPECT_EQ(decodedState.serviceId, 8888);
}

/**
 * @tc.name: EncodeServiceSyncInfo_001
 * @tc.desc: EncodeServiceSyncInfo with normal input
 *           Step 1: Prepare ServiceSyncInfo with normal values
 *           Step 2: Call EncodeServiceSyncInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeServiceSyncInfo_001, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo serviceSyncInfo;
    serviceSyncInfo.pkgName = "com.test.sync";
    serviceSyncInfo.localUserId = 100;
    serviceSyncInfo.networkId = "network123";
    serviceSyncInfo.serviceId = 7777;
    serviceSyncInfo.callerUserId = 200;
    serviceSyncInfo.callerTokenId = 300;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeServiceSyncInfo(serviceSyncInfo, parcel);
    EXPECT_EQ(ret, true);

    ServiceSyncInfo decodedInfo;
    IpcModelCodec::DecodeServiceSyncInfo(parcel, decodedInfo);
    EXPECT_EQ(decodedInfo.pkgName, "com.test.sync");
    EXPECT_EQ(decodedInfo.localUserId, 100);
    EXPECT_EQ(decodedInfo.networkId, "network123");
    EXPECT_EQ(decodedInfo.serviceId, 7777);
    EXPECT_EQ(decodedInfo.callerUserId, 200);
    EXPECT_EQ(decodedInfo.callerTokenId, 300);
}

/**
 * @tc.name: EncodeServiceSyncInfo_002
 * @tc.desc: EncodeServiceSyncInfo with zero values
 *           Step 1: Prepare ServiceSyncInfo with zero values
 *           Step 2: Call EncodeServiceSyncInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeServiceSyncInfo_002, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo serviceSyncInfo;
    serviceSyncInfo.pkgName = "";
    serviceSyncInfo.localUserId = 0;
    serviceSyncInfo.networkId = "";
    serviceSyncInfo.serviceId = 0;
    serviceSyncInfo.callerUserId = 0;
    serviceSyncInfo.callerTokenId = 0;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeServiceSyncInfo(serviceSyncInfo, parcel);
    EXPECT_EQ(ret, true);

    ServiceSyncInfo decodedInfo;
    IpcModelCodec::DecodeServiceSyncInfo(parcel, decodedInfo);
    EXPECT_EQ(decodedInfo.pkgName, "");
    EXPECT_EQ(decodedInfo.localUserId, 0);
    EXPECT_EQ(decodedInfo.networkId, "");
    EXPECT_EQ(decodedInfo.serviceId, 0);
    EXPECT_EQ(decodedInfo.callerUserId, 0);
    EXPECT_EQ(decodedInfo.callerTokenId, 0);
}

/**
 * @tc.name: DecodeServiceSyncInfo_001
 * @tc.desc: DecodeServiceSyncInfo with normal parcel data
 *           Step 1: Prepare MessageParcel with encoded data
 *           Step 2: Call DecodeServiceSyncInfo
 *           Step 3: Verify decoded values match original data
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, DecodeServiceSyncInfo_001, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo originalInfo;
    originalInfo.pkgName = "com.test.sync2";
    originalInfo.localUserId = 300;
    originalInfo.networkId = "network456";
    originalInfo.serviceId = 6666;
    originalInfo.callerUserId = 400;
    originalInfo.callerTokenId = 500;

    MessageParcel parcel;
    IpcModelCodec::EncodeServiceSyncInfo(originalInfo, parcel);

    ServiceSyncInfo decodedInfo;
    IpcModelCodec::DecodeServiceSyncInfo(parcel, decodedInfo);
    EXPECT_EQ(decodedInfo.pkgName, "com.test.sync2");
    EXPECT_EQ(decodedInfo.localUserId, 300);
    EXPECT_EQ(decodedInfo.networkId, "network456");
    EXPECT_EQ(decodedInfo.serviceId, 6666);
    EXPECT_EQ(decodedInfo.callerUserId, 400);
    EXPECT_EQ(decodedInfo.callerTokenId, 500);
}

/**
 * @tc.name: EncodeDmSrvDiscParam_003
 * @tc.desc: EncodeDmSrvDiscParam with SUPER_HIGH and BUTT enum values
 *           Step 1: Prepare DmDiscoveryServiceParam with DM_SUPER_HIGH freq and BUTT medium
 *           Step 2: Call EncodeDmSrvDiscParam
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmSrvDiscParam_003, testing::ext::TestSize.Level1)
{
    DmDiscoveryServiceParam param;
    param.serviceName = "testService3";
    param.serviceType = "testType3";
    param.serviceDisplayName = "Test Service Display 3";
    param.freq = DmExchangeFreq::DM_SUPER_HIGH;
    param.medium = DMSrvMediumType::SERVICE_MEDIUM_TYPE_BUTT;
    param.mode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmSrvDiscParam(param, parcel);
    EXPECT_EQ(ret, true);

    DmDiscoveryServiceParam decodedParam;
    bool decodeRet = IpcModelCodec::DecodeDmSrvDiscParam(parcel, decodedParam);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedParam.freq, DmExchangeFreq::DM_SUPER_HIGH);
    EXPECT_EQ(decodedParam.medium, DMSrvMediumType::SERVICE_MEDIUM_TYPE_BUTT);
}

/**
 * @tc.name: EncodeDmPublishServiceParam_003
 * @tc.desc: EncodeDmPublishServiceParam with BLE_TRIGGER and DM_SUPER_HIGH
 *           Step 1: Prepare DmPublishServiceParam with BLE_TRIGGER medium and DM_SUPER_HIGH freq
 *           Step 2: Call EncodeDmPublishServiceParam
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmPublishServiceParam_003, testing::ext::TestSize.Level1)
{
    DmPublishServiceParam publishServiceParam;
    publishServiceParam.discoverMode = DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE;
    publishServiceParam.media = DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE_TRIGGER;
    publishServiceParam.freq = DmExchangeFreq::DM_SUPER_HIGH;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmPublishServiceParam(publishServiceParam, parcel);
    EXPECT_EQ(ret, true);

    DmPublishServiceParam decodedParam;
    bool decodeRet = IpcModelCodec::DecodeDmPublishServiceParam(parcel, decodedParam);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedParam.discoverMode, DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE);
    EXPECT_EQ(decodedParam.media, DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE_TRIGGER);
    EXPECT_EQ(decodedParam.freq, DmExchangeFreq::DM_SUPER_HIGH);
}


/**
 * @tc.name: EncodeDmServiceInfo_003
 * @tc.desc: EncodeDmServiceInfo with INVALID_TYPE authform
 *           Step 1: Prepare DmServiceInfo with INVALID_TYPE authform
 *           Step 2: Call EncodeDmServiceInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmServiceInfo_003, testing::ext::TestSize.Level1)
{
    DmServiceInfo serviceInfo;
    serviceInfo.userId = 500;
    serviceInfo.serviceId = 5000;
    serviceInfo.displayId = 9000;
    serviceInfo.deviceId = "deviceInvalid";
    serviceInfo.networkId = "networkInvalid";
    serviceInfo.serviceOwnerTokenId = 99999;
    serviceInfo.serviceOwnerPkgName = "com.owner.invalid";
    serviceInfo.serviceRegisterTokenId = 88888;
    serviceInfo.serviceType = "invalidType";
    serviceInfo.serviceName = "invalidName";
    serviceInfo.serviceDisplayName = "Invalid Display";
    serviceInfo.publishState = 0;
    serviceInfo.serviceCode = "invalidCode";
    serviceInfo.customData = "invalid data";
    serviceInfo.dataLen = 90;
    serviceInfo.timeStamp = 1111111111;
    serviceInfo.description = "invalid description";
    serviceInfo.authform = DmAuthForm::INVALID_TYPE;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmServiceInfo(serviceInfo, parcel);
    EXPECT_EQ(ret, true);

    DmServiceInfo decodedInfo;
    bool decodeRet = IpcModelCodec::DecodeDmServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedInfo.authform, DmAuthForm::INVALID_TYPE);
    EXPECT_EQ(decodedInfo.publishState, 0);
}

/**
 * @tc.name: DecodeDmServiceInfo_003
 * @tc.desc: DecodeDmServiceInfo with publishState boundary values
 *           Step 1: Prepare MessageParcel with publishState = 2
 *           Step 2: Call DecodeDmServiceInfo
 *           Step 3: Verify decoded publishState is correct
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, DecodeDmServiceInfo_003, testing::ext::TestSize.Level1)
{
    DmServiceInfo originalInfo;
    originalInfo.userId = 600;
    originalInfo.serviceId = 6000;
    originalInfo.deviceId = "deviceBoundary";
    originalInfo.networkId = "networkBoundary";
    originalInfo.serviceOwnerTokenId = 101010;
    originalInfo.serviceOwnerPkgName = "com.owner.boundary";
    originalInfo.serviceRegisterTokenId = 202020;
    originalInfo.serviceType = "boundaryType";
    originalInfo.serviceName = "boundaryName";
    originalInfo.serviceDisplayName = "Boundary Display";
    originalInfo.publishState = 2;
    originalInfo.serviceCode = "boundaryCode";
    originalInfo.customData = "boundary data";
    originalInfo.dataLen = 100;
    originalInfo.timeStamp = 2222222222;
    originalInfo.description = "boundary description";
    originalInfo.authform = DmAuthForm::PEER_TO_PEER;

    MessageParcel parcel;
    IpcModelCodec::EncodeDmServiceInfo(originalInfo, parcel);

    DmServiceInfo decodedInfo;
    bool ret = IpcModelCodec::DecodeDmServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(decodedInfo.publishState, 2);
    EXPECT_EQ(decodedInfo.authform, DmAuthForm::PEER_TO_PEER);
}

/**
 * @tc.name: EncodeDmRegServiceInfo_003
 * @tc.desc: EncodeDmRegServiceInfo with maximum boundary values
 *           Step 1: Prepare DmRegisterServiceInfo with max int32 and uint64 values
 *           Step 2: Call EncodeDmRegServiceInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmRegServiceInfo_003, testing::ext::TestSize.Level1)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.userId = 2147483647;
    regServiceInfo.displayId = 9223372036854775807;
    regServiceInfo.serviceOwnerTokenId = 18446744073709551615ULL;
    regServiceInfo.serviceOwnerPkgName = "com.test.max";
    regServiceInfo.serviceRegisterTokenId = 18446744073709551615ULL;
    regServiceInfo.serviceType = "maxType";
    regServiceInfo.serviceName = "maxServiceName";
    regServiceInfo.serviceDisplayName = "Max Display Name";
    regServiceInfo.serviceCode = "maxCode";
    regServiceInfo.customData = "max data";
    regServiceInfo.dataLen = 4294967295U;
    regServiceInfo.timeStamp = 9223372036854775807;
    regServiceInfo.description = "max description";

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmRegServiceInfo(regServiceInfo, parcel);
    EXPECT_EQ(ret, true);

    DmRegisterServiceInfo decodedInfo;
    bool decodeRet = IpcModelCodec::DecodeDmRegServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedInfo.userId, 2147483647);
    EXPECT_EQ(decodedInfo.dataLen, 4294967295U);
}

/**
 * @tc.name: EncodeDmRegServiceInfo_004
 * @tc.desc: EncodeDmRegServiceInfo with minimum negative values
 *           Step 1: Prepare DmRegisterServiceInfo with negative int values
 *           Step 2: Call EncodeDmRegServiceInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmRegServiceInfo_004, testing::ext::TestSize.Level1)
{
    DmRegisterServiceInfo regServiceInfo;
    regServiceInfo.userId = -2147483648;
    regServiceInfo.displayId = -9223372036854775807LL;
    regServiceInfo.serviceOwnerTokenId = 0;
    regServiceInfo.serviceOwnerPkgName = "com.test.min";
    regServiceInfo.serviceRegisterTokenId = 0;
    regServiceInfo.serviceType = "minType";
    regServiceInfo.serviceName = "minServiceName";
    regServiceInfo.serviceDisplayName = "Min Display Name";
    regServiceInfo.serviceCode = "minCode";
    regServiceInfo.customData = "min data";
    regServiceInfo.dataLen = 0;
    regServiceInfo.timeStamp = -9223372036854775807LL;
    regServiceInfo.description = "min description";

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmRegServiceInfo(regServiceInfo, parcel);
    EXPECT_EQ(ret, true);

    DmRegisterServiceInfo decodedInfo;
    bool decodeRet = IpcModelCodec::DecodeDmRegServiceInfo(parcel, decodedInfo);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedInfo.userId, -2147483648);
    EXPECT_EQ(decodedInfo.timeStamp, -9223372036854775807LL);
}

/**
 * @tc.name: EncodeDmServiceInfo_004
 * @tc.desc: EncodeDmServiceInfo with all authform enum values
 *           Step 1: Test all remaining authform values systematically
 *           Step 2: Call EncodeDmServiceInfo for each
 *           Step 3: Verify all encode and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmServiceInfo_004, testing::ext::TestSize.Level1)
{
    std::vector<DmAuthForm> authForms = {
        DmAuthForm::INVALID_TYPE,
        DmAuthForm::PEER_TO_PEER,
        DmAuthForm::IDENTICAL_ACCOUNT,
        DmAuthForm::ACROSS_ACCOUNT,
        DmAuthForm::SHARE
    };

    for (size_t i = 0; i < authForms.size(); ++i) {
        DmServiceInfo serviceInfo;
        serviceInfo.userId = 700 + static_cast<int32_t>(i);
        serviceInfo.serviceId = 7000 + static_cast<int64_t>(i);
        serviceInfo.deviceId = "device" + std::to_string(i);
        serviceInfo.networkId = "network" + std::to_string(i);
        serviceInfo.serviceOwnerTokenId = 30303 + i;
        serviceInfo.serviceOwnerPkgName = "com.owner.all" + std::to_string(i);
        serviceInfo.serviceRegisterTokenId = 40404 + i;
        serviceInfo.serviceType = "allType";
        serviceInfo.serviceName = "allName";
        serviceInfo.serviceDisplayName = "All Display";
        serviceInfo.publishState = static_cast<int8_t>(i % 3);
        serviceInfo.serviceCode = "allCode";
        serviceInfo.customData = "all data";
        serviceInfo.dataLen = 110;
        serviceInfo.timeStamp = 3333333333;
        serviceInfo.description = "all description";
        serviceInfo.authform = authForms[i];

        MessageParcel parcel;
        bool ret = IpcModelCodec::EncodeDmServiceInfo(serviceInfo, parcel);
        EXPECT_EQ(ret, true);

        DmServiceInfo decodedInfo;
        bool decodeRet = IpcModelCodec::DecodeDmServiceInfo(parcel, decodedInfo);
        EXPECT_EQ(decodeRet, true);
        EXPECT_EQ(decodedInfo.authform, authForms[i]);
        EXPECT_EQ(decodedInfo.publishState, static_cast<int8_t>(i % 3));
    }
}

/**
 * @tc.name: DecodeDmPublishServiceParam_002
 * @tc.desc: DecodeDmPublishServiceParam with all freq enum values
 *           Step 1: Test all DmExchangeFreq enum values
 *           Step 2: Call DecodeDmPublishServiceParam for each
 *           Step 3: Verify all decode correctly
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, DecodeDmPublishServiceParam_002, testing::ext::TestSize.Level1)
{
    std::vector<DmExchangeFreq> freqs = {
        DmExchangeFreq::DM_LOW,
        DmExchangeFreq::DM_MID,
        DmExchangeFreq::DM_HIGH,
        DmExchangeFreq::DM_SUPER_HIGH
    };

    for (size_t i = 0; i < freqs.size(); ++i) {
        DmPublishServiceParam originalParam;
        originalParam.discoverMode = (i % 2 == 0) ?
            DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_ACTIVE :
            DMSrvDiscoveryMode::SERVICE_PUBLISH_MODE_PASSIVE;
        originalParam.media = DMSrvMediumType::SERVICE_MEDIUM_TYPE_BLE;
        originalParam.freq = freqs[i];

        MessageParcel parcel;
        IpcModelCodec::EncodeDmPublishServiceParam(originalParam, parcel);

        DmPublishServiceParam decodedParam;
        bool ret = IpcModelCodec::DecodeDmPublishServiceParam(parcel, decodedParam);
        EXPECT_EQ(ret, true);
        EXPECT_EQ(decodedParam.freq, freqs[i]);
    }
}

/**
 * @tc.name: EncodeDmRegisterServiceState_003
 * @tc.desc: EncodeDmRegisterServiceState with maximum values
 *           Step 1: Prepare DmRegisterServiceState with max values
 *           Step 2: Call EncodeDmRegisterServiceState
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeDmRegisterServiceState_003, testing::ext::TestSize.Level1)
{
    DmRegisterServiceState serviceState;
    serviceState.userId = 2147483647;
    serviceState.tokenId = 18446744073709551615ULL;
    serviceState.pkgName = "com.test.max.state";
    serviceState.serviceId = 9223372036854775807;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeDmRegisterServiceState(serviceState, parcel);
    EXPECT_EQ(ret, true);

    DmRegisterServiceState decodedState;
    bool decodeRet = IpcModelCodec::DecodeDmRegisterServiceState(parcel, decodedState);
    EXPECT_EQ(decodeRet, true);
    EXPECT_EQ(decodedState.userId, 2147483647);
    EXPECT_EQ(decodedState.tokenId, 18446744073709551615ULL);
    EXPECT_EQ(decodedState.serviceId, 9223372036854775807);
}

/**
 * @tc.name: EncodeServiceSyncInfo_003
 * @tc.desc: EncodeServiceSyncInfo with negative userId values
 *           Step 1: Prepare ServiceSyncInfo with negative userId
 *           Step 2: Call EncodeServiceSyncInfo
 *           Step 3: Verify return value is true and decode successfully
 * @tc.type: FUNC
 */
HWTEST_F(IpcModelCodecTest, EncodeServiceSyncInfo_003, testing::ext::TestSize.Level1)
{
    ServiceSyncInfo serviceSyncInfo;
    serviceSyncInfo.pkgName = "com.test.negative";
    serviceSyncInfo.localUserId = -1;
    serviceSyncInfo.networkId = "networkNegative";
    serviceSyncInfo.serviceId = -9999;
    serviceSyncInfo.callerUserId = -100;
    serviceSyncInfo.callerTokenId = 0;

    MessageParcel parcel;
    bool ret = IpcModelCodec::EncodeServiceSyncInfo(serviceSyncInfo, parcel);
    EXPECT_EQ(ret, true);

    ServiceSyncInfo decodedInfo;
    IpcModelCodec::DecodeServiceSyncInfo(parcel, decodedInfo);
    EXPECT_EQ(decodedInfo.pkgName, "com.test.negative");
    EXPECT_EQ(decodedInfo.localUserId, -1);
    EXPECT_EQ(decodedInfo.serviceId, -9999);
    EXPECT_EQ(decodedInfo.callerUserId, -100);
}

} // namespace DistributedHardware
} // namespace OHOS
