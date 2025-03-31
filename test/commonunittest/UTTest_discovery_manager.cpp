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

#include "UTTest_discovery_manager.h"

#include <iostream>
#include <string>
#include <unistd.h>

#include "device_manager_service_listener.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_server_listener.h"
#include "softbus_bus_center.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
void DiscoveryManagerTest::SetUp()
{
}

void DiscoveryManagerTest::TearDown()
{
}

void DiscoveryManagerTest::SetUpTestCase()
{
}

void DiscoveryManagerTest::TearDownTestCase()
{
}

namespace {
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);

bool CheckSoftbusRes(int32_t ret)
{
    return ret == SOFTBUS_INVALID_PARAM || ret == SOFTBUS_NETWORK_NOT_INIT || ret == SOFTBUS_NETWORK_LOOPER_ERR;
}

HWTEST_F(DiscoveryManagerTest, EnableDiscoveryListener_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryManagerTest, EnableDiscoveryListener_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    discoverParam.insert(std::pair<std::string, std::string>("META_TYPE", "ohos.test"));
    discoverParam.insert(std::pair<std::string, std::string>("SUBSCRIBE_ID", "ohos.test"));
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DiscoveryManagerTest, EnableDiscoveryListener_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DiscoveryManagerTest, DisableDiscoveryListener_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::map<std::string, std::string> extraParam;
    int32_t ret = manager->DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryManagerTest, DisableDiscoveryListener_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    extraParam.insert(std::pair<std::string, std::string>("META_TYPE", "ohos.test"));
    extraParam.insert(std::pair<std::string, std::string>("SUBSCRIBE_ID", "ohos.test"));
    int32_t ret = manager->DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DiscoveryManagerTest, DisableDiscoveryListener_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    int32_t ret = manager->DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DiscoveryManagerTest, StartDiscovering_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryManagerTest, StartDiscovering_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    discoverParam.insert(std::pair<std::string, std::string>("SUBSCRIBE_ID", "ohos.test"));
    discoverParam.insert(std::pair<std::string, std::string>("DISC_MEDIUM", "ohos.test"));
    discoverParam.insert(std::pair<std::string, std::string>("META_TYPE", "ohos.test"));
    discoverParam.insert(std::pair<std::string, std::string>("FILTER_OPTIONS", "ohos.test"));
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DiscoveryManagerTest, StartDiscovering_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_REPEATED);
}

HWTEST_F(DiscoveryManagerTest, StartDiscovering4MineLibary_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    DmSubscribeInfo dmSubInfo;
    std::string searchJson = "searchJson";
    int32_t ret = manager->StartDiscovering4MineLibary(pkgName, dmSubInfo, searchJson);
    EXPECT_EQ(ret, ERR_DM_JSON_PARSE_STRING);
}

HWTEST_F(DiscoveryManagerTest, StartDiscoveringNoMetaType_001, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubInfo;
    std::string pkgName = "pkgNameTest01";
    std::map<std::string, std::string> param;
    param.insert(std::pair<std::string, std::string>("META_TYPE", std::to_string(MetaNodeType::PROXY_SHARE)));
    int32_t ret = manager->StartDiscoveringNoMetaType(pkgName, dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DiscoveryManagerTest, StartDiscoveringNoMetaType_002, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubInfo;
    std::string pkgName = "pkgNameTest02";
    std::map<std::string, std::string> param;
    param.insert(std::pair<std::string, std::string>("META_TYPE", std::to_string(MetaNodeType::PROXY_WEAR)));
    int32_t ret = manager->StartDiscoveringNoMetaType(pkgName, dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DiscoveryManagerTest, StartDiscoveringNoMetaType_003, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubInfo;
    std::string pkgName = "pkgNameTest03";
    std::map<std::string, std::string> param;
    param.insert(std::pair<std::string, std::string>("META_TYPE", std::to_string(MetaNodeType::PROXY_CASTPLUS)));
    int32_t ret = manager->StartDiscoveringNoMetaType(pkgName, dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DiscoveryManagerTest, StartDiscoveringNoMetaType_004, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubInfo;
    std::string pkgName = "pkgNameTest04";
    std::map<std::string, std::string> param;
    int32_t ret = manager->StartDiscoveringNoMetaType(pkgName, dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DiscoveryManagerTest, StartDiscovering4MetaType_001, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubInfo;
    std::string pkgName = "pkgNameTest01";
    std::map<std::string, std::string> param;
    param.emplace("META_TYPE", "4");
    int32_t ret = manager->StartDiscovering4MetaType(pkgName, dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DiscoveryManagerTest, StartDiscovering4MetaType_002, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubInfo;
    std::string pkgName = "pkgNameTest02";
    std::map<std::string, std::string> param;
    param.emplace("META_TYPE", "7");
    int32_t ret = manager->StartDiscovering4MetaType(pkgName, dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DiscoveryManagerTest, StartDiscovering4MetaType_003, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubInfo;
    std::string pkgName = "pkgNameTest03";
    std::map<std::string, std::string> param;
    param.emplace("META_TYPE", "5");
    int32_t ret = manager->StartDiscovering4MetaType(pkgName, dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DiscoveryManagerTest, StartDiscovering4MetaType_004, testing::ext::TestSize.Level0)
{
    DmSubscribeInfo dmSubInfo;
    std::string pkgName = "pkgNameTest04";
    std::map<std::string, std::string> param;
    param.emplace("META_TYPE", "1234");
    int32_t ret = manager->StartDiscovering4MetaType(pkgName, dmSubInfo, param);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}

HWTEST_F(DiscoveryManagerTest, StopDiscovering_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    uint16_t subscribeId = 0;
    int32_t ret = manager->StopDiscovering(pkgName, subscribeId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DiscoveryManagerTest, StopDiscovering_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    uint16_t subscribeId = 0;
    manager->pkgNameSet_.insert(pkgName);
    DiscoveryContext context;
    manager->discoveryContextMap_.emplace(pkgName, context);
    int32_t ret = manager->StopDiscovering(pkgName, subscribeId);
    EXPECT_EQ(true, CheckSoftbusRes(ret));
}

HWTEST_F(DiscoveryManagerTest, OnDeviceFound_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    DmDeviceInfo info;
    bool isOnline = false;
    manager->OnDeviceFound(pkgName, info, isOnline);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), false);
}

HWTEST_F(DiscoveryManagerTest, OnDeviceFound_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    DmDeviceInfo info;
    bool isOnline = true;
    manager->OnDeviceFound(pkgName, info, isOnline);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), false);
}

HWTEST_F(DiscoveryManagerTest, OnDiscoveringResult_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t subscribeId = 1;
    int32_t result = 0;
    manager->OnDiscoveringResult(pkgName, subscribeId, result);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), false);
}

HWTEST_F(DiscoveryManagerTest, OnDiscoveringResult_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t subscribeId = 1;
    int32_t result = 0;
    manager->listener_ = nullptr;
    manager->OnDiscoveringResult(pkgName, subscribeId, result);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), false);
}

HWTEST_F(DiscoveryManagerTest, OnDiscoveringResult_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t subscribeId = 1;
    int32_t result = 0;
    manager->listener_ = std::make_shared<DeviceManagerServiceListener>();
    manager->OnDiscoveringResult(pkgName, subscribeId, result);
    EXPECT_NE(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DiscoveryManagerTest, OnDiscoveringResult_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    int32_t subscribeId = 1;
    int32_t result = 1;
    manager->pkgNameSet_.insert(pkgName);
    DiscoveryContext context;
    manager->discoveryContextMap_.emplace(pkgName, context);
    manager->listener_ = std::make_shared<DeviceManagerServiceListener>();
    manager->OnDiscoveringResult(pkgName, subscribeId, result);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DiscoveryManagerTest, StartDiscoveryTimer_001, testing::ext::TestSize.Level0)
{
    manager->timer_ = nullptr;
    std::string pkgName = "timeTest";
    manager->StartDiscoveryTimer(pkgName);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DiscoveryManagerTest, HandleDiscoveryQueue_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    uint16_t subscribeId = 0;
    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>("FILTER_OPTIONS", pkgName));
    int32_t ret = manager->HandleDiscoveryQueue(pkgName, subscribeId, filterOps);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DiscoveryManagerTest, HandleDiscoveryTimeout_001, testing::ext::TestSize.Level0)
{
    std::string name = "nameTest01";
    std::set<std::string> emptySet;
    manager->pkgNameSet_ = emptySet;
    manager->HandleDiscoveryTimeout(name);
    EXPECT_EQ(manager->pkgNameSet_.empty(), true);
}

HWTEST_F(DiscoveryManagerTest, HandleDiscoveryTimeout_002, testing::ext::TestSize.Level0)
{
    std::string name = "nameTest02";
    manager->pkgNameSet_.insert(name);
    manager->discoveryContextMap_.clear();
    manager->HandleDiscoveryTimeout(name);
    EXPECT_EQ(manager->pkgNameSet_.empty(), false);
}

HWTEST_F(DiscoveryManagerTest, HandleDiscoveryTimeout_003, testing::ext::TestSize.Level0)
{
    manager->pkgNameSet_.clear();
    std::string name = "nameTest03";
    manager->pkgNameSet_.insert(name);
    DiscoveryContext context;
    manager->discoveryContextMap_.emplace(name, context);
    manager->HandleDiscoveryTimeout(name);
    EXPECT_EQ(manager->pkgNameSet_.empty(), false);
}

HWTEST_F(DiscoveryManagerTest, GetDeviceAclParam_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::string deviceId;
    bool isonline = true;
    int32_t authForm = 0;
    int32_t ret = manager->GetDeviceAclParam(pkgName, 100, deviceId, isonline, authForm);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DiscoveryManagerTest, GetCommonDependencyObj_001, testing::ext::TestSize.Level0)
{
    auto ret = manager->GetCommonDependencyObj();
    EXPECT_NE(ret, nullptr);
}

HWTEST_F(DiscoveryManagerTest, IsCommonDependencyReady_001, testing::ext::TestSize.Level0)
{
    manager->isSoLoaded_ = false;
    bool ret = manager->IsCommonDependencyReady();
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryManagerTest, CloseCommonDependencyObj_001, testing::ext::TestSize.Level0)
{
    manager->isSoLoaded_ = false;
    manager->dpConnector_ = nullptr;
    manager->dpConnectorHandle_ = nullptr;
    bool ret = manager->CloseCommonDependencyObj();
    EXPECT_EQ(ret, true);
}

HWTEST_F(DiscoveryManagerTest, CloseCommonDependencyObj_002, testing::ext::TestSize.Level0)
{
    bool ret = manager->CloseCommonDependencyObj();
    EXPECT_EQ(ret, true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
