/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "UTTest_mine_softbus_listener.h"

#include <securec.h>
#include <unistd.h>
#include <cstdlib>
#include <thread>

#include "dm_dialog_manager.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "parameter.h"
#include "json_object.h"
#include "system_ability_definition.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
void MineSoftbusListenerTest::SetUp()
{
}
void MineSoftbusListenerTest::TearDown()
{
}
void MineSoftbusListenerTest::SetUpTestCase()
{
}
void MineSoftbusListenerTest::TearDownTestCase()
{
}

namespace {

bool CheckSoftbusRes(int32_t ret)
{
    return ret == SOFTBUS_INVALID_PARAM || ret == SOFTBUS_NETWORK_NOT_INIT || ret == SOFTBUS_NETWORK_LOOPER_ERR ||
        ret == SOFTBUS_IPC_ERR;
}

HWTEST_F(MineSoftbusListenerTest, RefreshSoftbusLNN_001, testing::ext::TestSize.Level0)
{
    string pkgName;
    string searchJson;
    DmSubscribeInfo dmSubscribeInfo;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->RefreshSoftbusLNN(pkgName, searchJson, dmSubscribeInfo);
    EXPECT_EQ(ret, ERR_DM_JSON_PARSE_STRING);
}

HWTEST_F(MineSoftbusListenerTest, RefreshSoftbusLNN_002, testing::ext::TestSize.Level0)
{
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = 4;
    string pkgName;
    string searchJson = SafetyDump(jsonObj);
    DmSubscribeInfo dmSubscribeInfo;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->RefreshSoftbusLNN(pkgName, searchJson, dmSubscribeInfo);
    EXPECT_EQ(ret, ERR_DM_SOFTBUS_SEND_BROADCAST);
}

HWTEST_F(MineSoftbusListenerTest, StopRefreshSoftbusLNN_001, testing::ext::TestSize.Level0)
{
    uint16_t subscribeId = 1;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->StopRefreshSoftbusLNN(subscribeId);
    EXPECT_TRUE(CheckSoftbusRes(ret));
}

HWTEST_F(MineSoftbusListenerTest, OnPublishResult_001, testing::ext::TestSize.Level0)
{
    int publishId = 1;
    PublishResult reason = PublishResult::PUBLISH_LNN_SUCCESS;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->OnPublishResult(publishId, reason);
    EXPECT_NE(mineListener->PublishDeviceDiscovery(), DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, OnPublishResult_002, testing::ext::TestSize.Level0)
{
    int publishId = 1;
    PublishResult reason = static_cast<PublishResult>(12);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->OnPublishResult(publishId, reason);
    EXPECT_NE(mineListener->PublishDeviceDiscovery(), DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, OnPublishDeviceFound_001, testing::ext::TestSize.Level0)
{
    DeviceInfo *deviceInfo = nullptr;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->OnPublishDeviceFound(deviceInfo);
    EXPECT_NE(mineListener->PublishDeviceDiscovery(), DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, OnPublishDeviceFound_002, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "11111"
    };
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->OnPublishDeviceFound(&deviceInfo);
    EXPECT_NE(mineListener->PublishDeviceDiscovery(), DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, OnRePublish_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->OnRePublish();
    EXPECT_NE(mineListener->PublishDeviceDiscovery(), DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchJson_001, testing::ext::TestSize.Level0)
{
    string pkgName;
    string searchJson;
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchJson(pkgName, searchJson, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_INVALID_JSON_STRING);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchJson_002, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(1);
    string pkgName;
    string searchJson = SafetyDump(jsonObj);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchJson(pkgName, searchJson, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchJson_003, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    string pkgName;
    string searchJson = SafetyDump(jsonObj);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchJson(pkgName, searchJson, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchJson_004, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(3);
    string pkgName;
    string searchJson = SafetyDump(jsonObj);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchJson(pkgName, searchJson, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchJson_005, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(4);
    string pkgName;
    string searchJson = SafetyDump(jsonObj);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchJson(pkgName, searchJson, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchAllDevice_001, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    string pkgName;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchAllDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchAllDevice_002, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    string pkgName = "1234656";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchAllDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}
  
HWTEST_F(MineSoftbusListenerTest, ParseSearchScopeDevice_001, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    string pkgName;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchScopeDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchScopeDevice_002, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    string pkgName = "1231301230213";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchScopeDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchScopeDevice_003, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    jsonObj["filterOptions"] = 1;
    string pkgName = "1231301230213";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchScopeDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchScopeDevice_004, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::vector<int> info;
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    jsonObj["filterOptions"] = info;
    string pkgName = "1231301230213";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchScopeDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_INVALID_JSON_STRING);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchScopeDevice_005, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    int parameter = 6;
    std::vector<int> info;
    info.push_back(parameter);
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    jsonObj["filterOptions"] = info;
    string pkgName = "1231301230213";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchScopeDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchVertexDevice_001, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    string pkgName;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchVertexDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchVertexDevice_002, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    string pkgName = "1231301230213";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchVertexDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchVertexDevice_003, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    jsonObj["filterOptions"] = 1;
    string pkgName = "1231301230213";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchVertexDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchVertexDevice_004, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::vector<int> info;
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    jsonObj["filterOptions"] = info;
    string pkgName = "1231301230213";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchVertexDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseSearchVertexDevice_005, testing::ext::TestSize.Level0)
{
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    int parameter = 6;
    std::vector<int> info;
    info.push_back(parameter);
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    jsonObj["filterOptions"] = info;
    string pkgName = "1231301230213";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseSearchVertexDevice(jsonObj, pkgName, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, SetBroadcastHead_001, testing::ext::TestSize.Level0)
{
    JsonObject jsonObj;
    string pkgName;
    BroadcastHead broadcastHead;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SetBroadcastHead(jsonObj, pkgName, broadcastHead);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, SetBroadcastHead_002, testing::ext::TestSize.Level0)
{
    BroadcastHead broadcastHead;
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    string pkgName;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SetBroadcastHead(jsonObj, pkgName, broadcastHead);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, SetBroadcastHead_003, testing::ext::TestSize.Level0)
{
    BroadcastHead broadcastHead;
    JsonObject jsonObj;
    jsonObj["findDeviceMode"] = static_cast<char>(2);
    jsonObj["tructOptions"] = true;
    string pkgName = "1231301230213";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SetBroadcastHead(jsonObj, pkgName, broadcastHead);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, AddHeadToBroadcast_001, testing::ext::TestSize.Level0)
{
    BroadcastHead broadcastHead = {
        .version = '1',
        .headDataLen = '2',
        .tlvDataLen = '3',
        .pkgNameHash = "name",
        .findMode = '5',
        .trustFilter = '6',
    };
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->AddHeadToBroadcast(broadcastHead, output);
    EXPECT_NE(mineListener->PublishDeviceDiscovery(), DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseScopeDeviceJsonArray_001, testing::ext::TestSize.Level0)
{
    vector<ScopeOptionInfo> optionInfo;
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseScopeDeviceJsonArray(optionInfo, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseScopeDeviceJsonArray_002, testing::ext::TestSize.Level0)
{
    ScopeOptionInfo info = {
        .deviceAlias = "deviceAlias",
        .startNumber = 0,
        .endNumber = 1,
    };
    vector<ScopeOptionInfo> optionInfo;
    optionInfo.push_back(info);
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseScopeDeviceJsonArray(optionInfo, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseVertexDeviceJsonArray_001, testing::ext::TestSize.Level0)
{
    vector<VertexOptionInfo> optionInfo;
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseVertexDeviceJsonArray(optionInfo, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseVertexDeviceJsonArray_002, testing::ext::TestSize.Level0)
{
    VertexOptionInfo info = {
        .type = "",
        .value = "1",
    };
    vector<VertexOptionInfo> optionInfo;
    optionInfo.push_back(info);
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseVertexDeviceJsonArray(optionInfo, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseVertexDeviceJsonArray_003, testing::ext::TestSize.Level0)
{
    VertexOptionInfo info = {
        .type = "1",
        .value = "",
    };
    vector<VertexOptionInfo> optionInfo;
    optionInfo.push_back(info);
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseVertexDeviceJsonArray(optionInfo, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseVertexDeviceJsonArray_004, testing::ext::TestSize.Level0)
{
    VertexOptionInfo info = {
        .type = "1",
        .value = "1",
    };
    vector<VertexOptionInfo> optionInfo;
    optionInfo.push_back(info);
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseVertexDeviceJsonArray(optionInfo, output, &outLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseVertexDeviceJsonArray_005, testing::ext::TestSize.Level0)
{
    VertexOptionInfo info = {
        .type = "deviceUdid",
        .value = "112341234",
    };
    vector<VertexOptionInfo> optionInfo;
    optionInfo.push_back(info);
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseVertexDeviceJsonArray(optionInfo, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseVertexDeviceJsonArray_006, testing::ext::TestSize.Level0)
{
    VertexOptionInfo info = {
        .type = "deviceType",
        .value = "112341234",
    };
    vector<VertexOptionInfo> optionInfo;
    optionInfo.push_back(info);
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseVertexDeviceJsonArray(optionInfo, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseVertexDeviceJsonArray_007, testing::ext::TestSize.Level0)
{
    VertexOptionInfo info = {
        .type = "deviceSn",
        .value = "112341234",
    };
    vector<VertexOptionInfo> optionInfo;
    optionInfo.push_back(info);
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseVertexDeviceJsonArray(optionInfo, output, &outLen);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, GetSha256Hash_001, testing::ext::TestSize.Level0)
{
    char *data = nullptr;
    size_t outLen = 1024;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->GetSha256Hash(data, outLen, output);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, GetSha256Hash_002, testing::ext::TestSize.Level0)
{
    std::string data = "data";
    size_t outLen = 1024;
    char *output = nullptr;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->GetSha256Hash(data.c_str(), outLen, output);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, GetSha256Hash_003, testing::ext::TestSize.Level0)
{
    std::string data = "data";
    size_t outLen = 0;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->GetSha256Hash(data.c_str(), outLen, output);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, GetSha256Hash_004, testing::ext::TestSize.Level0)
{
    std::string data = "data";
    size_t outLen = 1024;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->GetSha256Hash(data.c_str(), outLen, output);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, SetBroadcastTrustOptions_001, testing::ext::TestSize.Level0)
{
    BroadcastHead broadcastHead;
    JsonObject jsonObj;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SetBroadcastTrustOptions(jsonObj, broadcastHead);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, SetBroadcastTrustOptions_002, testing::ext::TestSize.Level0)
{
    BroadcastHead broadcastHead;
    JsonObject jsonObj;
    jsonObj["tructOptions"] = "tructOptions";
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SetBroadcastTrustOptions(jsonObj, broadcastHead);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, SetBroadcastTrustOptions_003, testing::ext::TestSize.Level0)
{
    BroadcastHead broadcastHead;
    JsonObject jsonObj;
    jsonObj["tructOptions"] = true;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SetBroadcastTrustOptions(jsonObj, broadcastHead);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, SetBroadcastTrustOptions_004, testing::ext::TestSize.Level0)
{
    BroadcastHead broadcastHead;
    JsonObject jsonObj;
    jsonObj["tructOptions"] = false;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SetBroadcastTrustOptions(jsonObj, broadcastHead);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, SetBroadcastPkgname_001, testing::ext::TestSize.Level0)
{
    string pkgName;
    BroadcastHead broadcastHead;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SetBroadcastPkgname(pkgName, broadcastHead);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, SetBroadcastPkgname_002, testing::ext::TestSize.Level0)
{
    string pkgName = "2134165415";
    BroadcastHead broadcastHead;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SetBroadcastPkgname(pkgName, broadcastHead);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, SetSubscribeInfo_001, testing::ext::TestSize.Level0)
{
    SubscribeInfo subscribeInfo;
    DmSubscribeInfo dmSubscribeInfo;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->SetSubscribeInfo(dmSubscribeInfo, subscribeInfo);
    EXPECT_NE(mineListener->PublishDeviceDiscovery(), DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, SendBroadcastInfo_001, testing::ext::TestSize.Level0)
{
    string pkgName;
    SubscribeInfo subscribeInfo;
    char *output = nullptr;
    size_t outputLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->SendBroadcastInfo(pkgName, subscribeInfo, output, outputLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, DmBase64Encode_001, testing::ext::TestSize.Level0)
{
    char *output = nullptr;
    size_t outputLen = 1024;
    char input[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t inputLen = 1024;
    size_t base64OutLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->DmBase64Encode(output, outputLen, input, inputLen, base64OutLen);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, DmBase64Encode_002, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t outputLen = 1024;
    char *input = nullptr;
    size_t inputLen = 1024;
    size_t base64OutLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->DmBase64Encode(output, outputLen, input, inputLen, base64OutLen);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, DmBase64Encode_003, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t outputLen = 0;
    std::string input = "input";
    size_t inputLen = 1024;
    size_t base64OutLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->DmBase64Encode(output, outputLen, input.c_str(), inputLen, base64OutLen);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, DmBase64Encode_004, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t outputLen = 1024;
    std::string input = "input";
    size_t inputLen = 0;
    size_t base64OutLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->DmBase64Encode(output, outputLen, input.c_str(), inputLen, base64OutLen);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, DmBase64Decode_001, testing::ext::TestSize.Level0)
{
    char *output = nullptr;
    size_t outputLen = 1024;
    char input[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t inputLen = 1024;
    size_t base64OutLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->DmBase64Decode(output, outputLen, input, inputLen, base64OutLen);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, DmBase64Decode_002, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t outputLen = 1024;
    char *input = nullptr;
    size_t inputLen = 1024;
    size_t base64OutLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->DmBase64Decode(output, outputLen, input, inputLen, base64OutLen);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, DmBase64Decode_003, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t outputLen = 0;
    std::string input = "input";
    size_t inputLen = 1024;
    size_t base64OutLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->DmBase64Decode(output, outputLen, input.c_str(), inputLen, base64OutLen);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, DmBase64Decode_004, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t outputLen = 1024;
    std::string input = "input";
    size_t inputLen = 0;
    size_t base64OutLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->DmBase64Decode(output, outputLen, input.c_str(), inputLen, base64OutLen);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(MineSoftbusListenerTest, DmBase64Decode_005, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t outputLen = 1024;
    std::string input = "input";
    size_t inputLen = 1024;
    size_t base64OutLen = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->DmBase64Decode(output, outputLen, input.c_str(), inputLen, base64OutLen);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, PublishDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->PublishDeviceDiscovery();
    EXPECT_TRUE(CheckSoftbusRes(ret));
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchDealTask_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->MatchSearchDealTask();
    EXPECT_NE(mineListener->PublishDeviceDiscovery(), DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, ParseBroadcastInfo_001, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseBroadcastInfo(deviceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, ParseBroadcastInfo_002, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "com.ohos.helloworld"
    };
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    int32_t ret = mineListener->ParseBroadcastInfo(deviceInfo);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(MineSoftbusListenerTest, GetBroadcastData_001, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "com.ohos.helloworld"
    };
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    size_t outputLen = 1024;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    bool ret = mineListener->GetBroadcastData(deviceInfo, output, outputLen);
    EXPECT_EQ(ret, true);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchAllDevice_001, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "com.ohos.helloworld"
    };
    BroadcastHead broadcastHead;
    broadcastHead.trustFilter = 0;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchAllDevice(deviceInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchAllDevice_002, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "com.ohos.helloworld"
    };
    BroadcastHead broadcastHead;
    deviceInfo.isOnline = true;
    broadcastHead.trustFilter = static_cast<char>(3);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchAllDevice(deviceInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchAllDevice_003, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "com.ohos.helloworld"
    };
    BroadcastHead broadcastHead;
    deviceInfo.isOnline = true;
    broadcastHead.trustFilter = static_cast<char>(2);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchAllDevice(deviceInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_NOT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchAllDevice_004, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "com.ohos.helloworld"
    };
    BroadcastHead broadcastHead;
    broadcastHead.trustFilter = 1;
    deviceInfo.isOnline = false;
    broadcastHead.trustFilter = static_cast<char>(3);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchAllDevice(deviceInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_NOT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchAllDevice_005, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo = {
        .devId = "123456",
        .devType = (DeviceType)1,
        .devName = "com.ohos.helloworld"
    };
    BroadcastHead broadcastHead;
    broadcastHead.trustFilter = 1;
    deviceInfo.isOnline = false;
    broadcastHead.trustFilter = 7;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchAllDevice(deviceInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, GetScopeDevicePolicyInfo_001, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->GetScopeDevicePolicyInfo(devicePolicyInfo);
    EXPECT_NE(mineListener->PublishDeviceDiscovery(), DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchScopeDevice_001, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    deviceInfo.isOnline = true;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    DevicePolicyInfo devicePolicyInfo;
    BroadcastHead broadcastHead;
    broadcastHead.trustFilter = static_cast<char>(2);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchScopeDevice(deviceInfo, output, devicePolicyInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_NOT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchScopeDevice_002, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    deviceInfo.isOnline = false;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    DevicePolicyInfo devicePolicyInfo;
    BroadcastHead broadcastHead;
    broadcastHead.trustFilter = static_cast<char>(3);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchScopeDevice(deviceInfo, output, devicePolicyInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_NOT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchScopeDevice_003, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    deviceInfo.isOnline = false;
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    DevicePolicyInfo devicePolicyInfo;
    BroadcastHead broadcastHead;
    broadcastHead.trustFilter = static_cast<char>(7);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchScopeDevice(deviceInfo, output, devicePolicyInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_NOT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, GetVertexDevicePolicyInfo_001, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    mineListener->GetVertexDevicePolicyInfo(devicePolicyInfo);
    EXPECT_EQ(devicePolicyInfo.snHashValid, true);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchVertexDevice_001, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    deviceInfo.isOnline = true;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    DevicePolicyInfo devicePolicyInfo;
    BroadcastHead broadcastHead;
    broadcastHead.trustFilter = static_cast<char>(2);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchVertexDevice(deviceInfo, output, devicePolicyInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_NOT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchVertexDevice_002, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    deviceInfo.isOnline = false;
    char output[DISC_MAX_CUST_DATA_LEN] = {0};
    DevicePolicyInfo devicePolicyInfo;
    BroadcastHead broadcastHead;
    broadcastHead.trustFilter = static_cast<char>(3);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchVertexDevice(deviceInfo, output, devicePolicyInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_NOT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, MatchSearchVertexDevice_003, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    deviceInfo.isOnline = false;
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    DevicePolicyInfo devicePolicyInfo;
    BroadcastHead broadcastHead;
    broadcastHead.trustFilter = static_cast<char>(7);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->MatchSearchVertexDevice(deviceInfo, output, devicePolicyInfo, broadcastHead);
    EXPECT_EQ(ret, BUSINESS_EXACT_NOT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, SendReturnwave_001, testing::ext::TestSize.Level0)
{
    DeviceInfo deviceInfo;
    BroadcastHead broadcastHead;
    Action matchResult = static_cast<Action>(1);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->SendReturnwave(deviceInfo, broadcastHead, matchResult);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(MineSoftbusListenerTest, GetDeviceAliasHash_001, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->GetDeviceAliasHash(output);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, GetDeviceSnHash_001, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->GetDeviceSnHash(output);
    EXPECT_EQ(ret, true);
}

HWTEST_F(MineSoftbusListenerTest, GetDeviceUdidHash_001, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->GetDeviceUdidHash(output);
    EXPECT_EQ(ret, true);
}

HWTEST_F(MineSoftbusListenerTest, GetDeviceTypeHash_001, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->GetDeviceTypeHash(output);
    EXPECT_EQ(ret, true);
}

HWTEST_F(MineSoftbusListenerTest, GetDeviceNumber_001, testing::ext::TestSize.Level0)
{
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->GetDeviceNumber(output);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceAliasMatch_001, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    devicePolicyInfo.aliasHashValid = false;
    char data[DISC_MAX_CUST_DATA_LEN] = {1};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceAliasMatch(devicePolicyInfo, data);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceAliasMatch_002, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    devicePolicyInfo.aliasHashValid = true;
    char data[DISC_MAX_CUST_DATA_LEN] = {1};
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceAliasMatch(devicePolicyInfo, data);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceNumberMatch_001, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    devicePolicyInfo.aliasHashValid = false;
    int32_t startNumber = 1;
    int32_t endNumber = 1;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceNumberMatch(devicePolicyInfo, startNumber, endNumber);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceNumberMatch_002, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    devicePolicyInfo.aliasHashValid = true;
    int32_t startNumber = -1;
    int32_t endNumber = 2;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceNumberMatch(devicePolicyInfo, startNumber, endNumber);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceNumberMatch_003, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    devicePolicyInfo.aliasHashValid = true;
    int32_t startNumber = 2;
    int32_t endNumber = -1;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceNumberMatch(devicePolicyInfo, startNumber, endNumber);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceNumberMatchh_004, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo = {
        .number = "1",
    };
    devicePolicyInfo.aliasHashValid = true;
    int32_t startNumber = 2;
    int32_t endNumber = 2;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceNumberMatch(devicePolicyInfo, startNumber, endNumber);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceNumberMatch_005, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo = {
        .number = "3",
    };
    devicePolicyInfo.aliasHashValid = true;
    int32_t startNumber = 2;
    int32_t endNumber = 2;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceNumberMatch(devicePolicyInfo, startNumber, endNumber);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceNumberMatch_006, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo = {
        .number = "10",
    };
    devicePolicyInfo.aliasHashValid = true;
    int32_t startNumber = 2;
    int32_t endNumber = 20;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceNumberMatch(devicePolicyInfo, startNumber, endNumber);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceSnMatch_001, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    devicePolicyInfo.snHashValid = false;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceSnMatch(devicePolicyInfo, output);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceSnMatch_002, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    devicePolicyInfo.snHashValid = true;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceSnMatch(devicePolicyInfo, output);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceTypeMatch_001, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    devicePolicyInfo.typeHashValid = false;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceTypeMatch(devicePolicyInfo, output);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceTypeMatch_002, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    devicePolicyInfo.typeHashValid = true;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceTypeMatch(devicePolicyInfo, output);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceUdidMatch_001, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    devicePolicyInfo.udidHashValid = false;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceUdidMatch(devicePolicyInfo, output);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, CheckDeviceUdidMatch_002, testing::ext::TestSize.Level0)
{
    DevicePolicyInfo devicePolicyInfo;
    char output[DISC_MAX_CUST_DATA_LEN] = {1};
    devicePolicyInfo.udidHashValid = true;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->CheckDeviceUdidMatch(devicePolicyInfo, output);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MineSoftbusListenerTest, GetMatchResult_001, testing::ext::TestSize.Level0)
{
    std::vector<int> matchItemNum;
    std::vector<int> matchItemResult;
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->GetMatchResult(matchItemNum, matchItemResult);
    EXPECT_EQ(ret, BUSINESS_EXACT_NOT_MATCH);
}

HWTEST_F(MineSoftbusListenerTest, GetMatchResult_002, testing::ext::TestSize.Level0)
{
    std::vector<int> matchItemNum;
    matchItemNum.push_back(1);
    std::vector<int> matchItemResult;
    matchItemResult.push_back(2);
    std::shared_ptr<MineSoftbusListener> mineListener = std::make_shared<MineSoftbusListener>();
    auto ret = mineListener->GetMatchResult(matchItemNum, matchItemResult);
    EXPECT_EQ(ret, BUSINESS_PARTIAL_MATCH);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS