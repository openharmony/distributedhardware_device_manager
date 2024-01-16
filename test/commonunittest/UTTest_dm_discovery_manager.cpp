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

#include "UTTest_dm_discovery_manager.h"

#include <iostream>
#include <string>
#include <unistd.h>

#include "dm_log.h"
#include "dm_constants.h"
#include "dm_anonymous.h"
#include "ipc_server_listener.h"
#include "device_manager_service_listener.h"
#include "softbus_bus_center.h"
#include "device_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
void DmDiscoveryManagerTest::SetUp()
{
}

void DmDiscoveryManagerTest::TearDown()
{
}

void DmDiscoveryManagerTest::SetUpTestCase()
{
}

void DmDiscoveryManagerTest::TearDownTestCase()
{
}

namespace {
std::shared_ptr<SoftbusConnector> softbusConnector_ = std::make_shared<SoftbusConnector>();
std::shared_ptr<DeviceManagerServiceListener> listener_ = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainConnector> hiChainConnector_ = std::make_shared<HiChainConnector>();
std::shared_ptr<DmDiscoveryManager> discoveryMgr_ =
    std::make_shared<DmDiscoveryManager>(softbusConnector_, listener_, hiChainConnector_);

/**
 * @tc.name: DmDiscoveryManager_001
 * @tc.desc: Test whether the DmDiscoveryManager function can generate a new pointer
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, DmDiscoveryManager_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmDiscoveryManager> Test =
        std::make_shared<DmDiscoveryManager>(softbusConnector_, listener_, hiChainConnector_);
    ASSERT_NE(Test, nullptr);
}

/**
 * @tc.name: DmDiscoveryManager_002
 * @tc.desc: Test whether the DmDiscoveryManager function can delete a new pointer
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, DmDiscoveryManager_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<DmDiscoveryManager> Test =
        std::make_shared<DmDiscoveryManager>(softbusConnector_, listener_, hiChainConnector_);
    Test.reset();
    EXPECT_EQ(Test, nullptr);
}

/**
 * @tc.name:StartDeviceDiscovery_001
 * @tc.desc: keeping pkgame unchanged, call StartDeviceDiscovery twice
 *           so that its discoveryQueue is not empty and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmSubscribeInfo subscribeInfo;
    const std::string extra;
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
    discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeInfo.subscribeId);
}

/**
 * @tc.name:StartDeviceDiscovery_002
 * @tc.desc: pkgame changed, call StartDeviceDiscovery twice
 *           so that its discoveryQueue is not empty and return ERR_DM_DISCOVERY_REPEATED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string extra;
    DmSubscribeInfo subscribeInfo;
    discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    pkgName = "com.ohos.helloworld.new";
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    ASSERT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
    discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeInfo.subscribeId);
}

/**
 * @tc.name: OnDeviceFound_001
 * @tc.desc: The OnDeviceFound function does the correct case and assigns pkgName
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDeviceFound_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string filterOptions = R"(
    {
        "filter_op": "OR",
        "filters":
        [
            {
                "type" : "credible",
                "value" : 0
            }
        ]
    }
    )";
    DmDeviceFilterOption dmFilter;
    dmFilter.TransformToFilter(filterOptions);
    uint16_t aaa = 11;
    DmDiscoveryContext context { pkgName, filterOptions, aaa, dmFilter.filterOp_, dmFilter.filters_ };
    discoveryMgr_->discoveryContextMap_[pkgName] = context;
    sleep(1);
    DmDeviceInfo info;
    info.deviceId[0] = '\0';
    info.deviceName[0] = '\0';
    bool isOnline = false;
    discoveryMgr_->OnDeviceFound(pkgName, info, isOnline);
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq =
        std::static_pointer_cast<IpcNotifyDeviceFoundReq>(listener_->ipcServerListener_.req_);
    int ret1 = discoveryMgr_->discoveryContextMap_.count(pkgName);
    EXPECT_EQ(ret1, 1);
    std::string ret = pReq->GetPkgName();
    EXPECT_EQ(ret, pkgName);
}

/**
 * @tc.name: OnDeviceFound_002
 * @tc.desc: set pkgName not null and discoveryContextMap_ null and return
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDeviceFound_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string filterOptions = R"(
    {
        "filter_op": "AND",
        "filters":
        [
            {
                "type" : "credible",
                "value" : 2
            }
        ]
    }
    )";
    DmDeviceFilterOption dmFilter;
    dmFilter.TransformToFilter(filterOptions);
    uint16_t aaa = 11;
    DmDiscoveryContext context { pkgName, filterOptions, aaa, dmFilter.filterOp_, dmFilter.filters_ };
    discoveryMgr_->discoveryContextMap_[pkgName] = context;
    DmDeviceInfo info;
    bool isOnline = false;
    discoveryMgr_->OnDeviceFound(pkgName, info, isOnline);
    int ret1 = discoveryMgr_->discoveryContextMap_.count(pkgName);
    EXPECT_EQ(ret1, 1);
    std::shared_ptr<IpcNotifyDeviceFoundReq> pReq =
        std::static_pointer_cast<IpcNotifyDeviceFoundReq>(listener_->ipcServerListener_.req_);
    std::string ret = pReq->GetPkgName();
    EXPECT_EQ(ret, pkgName);
}

/**
 * @tc.name: OnDiscoveryFailed_001
 * @tc.desc: The OnDeviceFound function takes the wrong case and emptying pkgName
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDiscoveryFailed_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    int32_t subscribeId = 1;
    int32_t failedReason = 3;
    discoveryMgr_->OnDiscoveryFailed(pkgName, subscribeId, failedReason);
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq =
        std::static_pointer_cast<IpcNotifyDiscoverResultReq>(listener_->ipcServerListener_.req_);
    std::string ret = pReq->GetPkgName();
    EXPECT_EQ(ret, pkgName);
}

/**
 * @tc.name: OnDiscoveryFailed_002
 * @tc.desc: The OnDeviceFound function takes the wrong case and emptying pkgName
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDiscoveryFailed_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t subscribeId = 1;
    int32_t failedReason = 3;
    discoveryMgr_->OnDiscoveryFailed(pkgName, subscribeId, failedReason);
    int ret1 = discoveryMgr_->discoveryContextMap_.count(pkgName);
    EXPECT_EQ(ret1, 0);
}

/**
 * @tc.name: OnDiscoverySuccess_001
 * @tc.desc: The OnDeviceFound function takes the wrong case and return pkgName
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDiscoverySuccess_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    int32_t subscribeId = 1;
    discoveryMgr_->OnDiscoverySuccess(pkgName, subscribeId);
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq =
        std::static_pointer_cast<IpcNotifyDiscoverResultReq>(listener_->ipcServerListener_.req_);
    std::string ret = pReq->GetPkgName();
    EXPECT_EQ(ret, pkgName);
}

/**
 * @tc.name: OnDiscoverySuccess_002
 * @tc.desc: set pkgName null and return discoveryContextMap_ null and return pkgName(null)
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDiscoverySuccess_002, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int32_t subscribeId = 1;
    discoveryMgr_->OnDiscoverySuccess(pkgName, subscribeId);
    int ret1 = discoveryMgr_->discoveryContextMap_.count(pkgName);
    EXPECT_EQ(ret1, 1);
    std::shared_ptr<IpcNotifyDiscoverResultReq> pReq =
        std::static_pointer_cast<IpcNotifyDiscoverResultReq>(listener_->ipcServerListener_.req_);
    std::string ret = pReq->GetPkgName();
    EXPECT_EQ(ret, pkgName);
}

/**
 * @tc.name: StartDeviceDiscovery_003
 * @tc.desc: return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    uint16_t subscribeId = 1;
    std::string filterOptions = "filterOptions";
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
}

/**
 * @tc.name: StartDeviceDiscovery_004
 * @tc.desc: return ERR_DM_DISCOVERY_REPEATED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    uint16_t subscribeId = 1;
    std::string filterOptions;
    discoveryMgr_->discoveryQueue_.push(pkgName);
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_REPEATED);
}

/**
 * @tc.name: StartDeviceDiscovery_005
 * @tc.desc: return ERR_DM_DISCOVERY_REPEATED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    uint16_t subscribeId = 1;
    std::string filterOptions;
    while (discoveryMgr_->discoveryQueue_.empty()) {
        discoveryMgr_->discoveryQueue_.pop();
    }
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_REPEATED);
}

/**
 * @tc.name: OnDeviceFound_003
 * @tc.desc: set pkgName not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDeviceFound_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmDeviceBasicInfo info;
    int32_t range = 0;
    bool isOnline = true;
    discoveryMgr_->OnDeviceFound(pkgName, info, range, isOnline);
    EXPECT_EQ(discoveryMgr_->discoveryContextMap_.empty(), false);
}

HWTEST_F(DmDiscoveryManagerTest, EnableDiscoveryListener_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmDiscoveryManagerTest, EnableDiscoveryListener_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    discoverParam.insert(std::pair<std::string, std::string>("META_TYPE", "ohos.test"));
    discoverParam.insert(std::pair<std::string, std::string>("SUBSCRIBE_ID", "ohos.test"));
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_ENABLE_DISCOVERY_LISTENER_FAILED);
}

HWTEST_F(DmDiscoveryManagerTest, EnableDiscoveryListener_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_ENABLE_DISCOVERY_LISTENER_FAILED);
}

HWTEST_F(DmDiscoveryManagerTest, DisableDiscoveryListener_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName;
    std::map<std::string, std::string> extraParam;
    int32_t ret = manager->DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmDiscoveryManagerTest, DisableDiscoveryListener_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    extraParam.insert(std::pair<std::string, std::string>("META_TYPE", "ohos.test"));
    extraParam.insert(std::pair<std::string, std::string>("SUBSCRIBE_ID", "ohos.test"));
    int32_t ret = manager->DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_STOP_REFRESH_LNN_FAILED);
}

HWTEST_F(DmDiscoveryManagerTest, DisableDiscoveryListener_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    int32_t ret = manager->DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_STOP_REFRESH_LNN_FAILED);
}

HWTEST_F(DmDiscoveryManagerTest, StartDiscovering_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmDiscoveryManagerTest, StartDiscovering_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    discoverParam.insert(std::pair<std::string, std::string>("SUBSCRIBE_ID", "ohos.test"));
    discoverParam.insert(std::pair<std::string, std::string>("DISC_MEDIUM", "ohos.test"));
    discoverParam.insert(std::pair<std::string, std::string>("META_TYPE", "ohos.test"));
    discoverParam.insert(std::pair<std::string, std::string>("FILTER_OPTIONS", "ohos.test"));
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_START_DISCOVERING_FAILED);
}

HWTEST_F(DmDiscoveryManagerTest, StartDiscovering_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = manager->StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_START_DISCOVERING_FAILED);
}

HWTEST_F(DmDiscoveryManagerTest, StartDiscovering4MineMetaNode_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName;
    DmSubscribeInfo dmSubInfo;
    std::string searchJson;
    int32_t ret = manager->StartDiscovering4MineMetaNode(pkgName, dmSubInfo, searchJson);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DmDiscoveryManagerTest, StartDiscoveringNoMetaType_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    DmSubscribeInfo dmSubInfo;
    std::map<std::string, std::string> param;
    param.insert(std::pair<std::string, std::string>("META_TYPE", std::to_string(MetaNodeType::PROXY_SHARE)));
    int32_t ret = manager->StartDiscoveringNoMetaType(dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DmDiscoveryManagerTest, StartDiscoveringNoMetaType_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    DmSubscribeInfo dmSubInfo;
    std::map<std::string, std::string> param;
    param.insert(std::pair<std::string, std::string>("META_TYPE", std::to_string(MetaNodeType::PROXY_WEAR)));
    int32_t ret = manager->StartDiscoveringNoMetaType(dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DmDiscoveryManagerTest, StartDiscoveringNoMetaType_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    DmSubscribeInfo dmSubInfo;
    std::map<std::string, std::string> param;
    param.insert(std::pair<std::string, std::string>("META_TYPE", std::to_string(MetaNodeType::PROXY_CASTPLUS)));
    int32_t ret = manager->StartDiscoveringNoMetaType(dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DmDiscoveryManagerTest, StartDiscoveringNoMetaType_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    DmSubscribeInfo dmSubInfo;
    std::map<std::string, std::string> param;
    int32_t ret = manager->StartDiscoveringNoMetaType(dmSubInfo, param);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DmDiscoveryManagerTest, StopDiscovering_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName;
    uint16_t subscribeId = 0;
    int32_t ret = manager->StopDiscovering(pkgName, subscribeId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

HWTEST_F(DmDiscoveryManagerTest, OnDeviceFound_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    DmDeviceInfo info;
    bool isOnline = true;
    manager->OnDeviceFound(pkgName, info, isOnline);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DmDiscoveryManagerTest, OnDiscoveringResult_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName;
    int32_t subscribeId = 1;
    int32_t result = 0;
    manager->OnDiscoveringResult(pkgName, subscribeId, result);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DmDiscoveryManagerTest, OnDiscoveringResult_002, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    int32_t subscribeId = 1;
    int32_t result = 0;
    manager->listener_ = nullptr;
    manager->OnDiscoveringResult(pkgName, subscribeId, result);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DmDiscoveryManagerTest, OnDiscoveringResult_003, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    int32_t subscribeId = 1;
    int32_t result = 0;
    manager->OnDiscoveringResult(pkgName, subscribeId, result);
    EXPECT_NE(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DmDiscoveryManagerTest, OnDiscoveringResult_004, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    int32_t subscribeId = 1;
    int32_t result = 1;
    manager->OnDiscoveringResult(pkgName, subscribeId, result);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DmDiscoveryManagerTest, StartDiscoveryTimer_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    manager->timer_ = nullptr;
    manager->StartDiscoveryTimer();
    EXPECT_EQ(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DmDiscoveryManagerTest, HandleDiscoveryQueue_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName = "pkgName";
    uint16_t subscribeId = 0;
    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>("FILTER_OPTIONS", pkgName));
    int32_t ret = manager->HandleDiscoveryQueue(pkgName, subscribeId, filterOps);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DmDiscoveryManagerTest, HandleDiscoveryTimeout_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string name = "name";
    manager->HandleDiscoveryTimeout(name);
    EXPECT_EQ(manager->discoveryContextMap_.empty(), true);
}

HWTEST_F(DmDiscoveryManagerTest, GetDeviceAclParam_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    std::string pkgName;
    std::string deviceId;
    bool isonline = true;
    int32_t authForm = 0;
    int32_t ret = manager->GetDeviceAclParam(pkgName, deviceId, isonline, authForm);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmDiscoveryManagerTest, GetCommonDependencyObj_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    auto ret = manager->GetCommonDependencyObj();
    EXPECT_EQ(ret, nullptr);
}

HWTEST_F(DmDiscoveryManagerTest, IsCommonDependencyReady_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    manager->isSoLoaded_ = false;
    bool ret = manager->IsCommonDependencyReady();
    EXPECT_EQ(ret, true);
}

HWTEST_F(DmDiscoveryManagerTest, CloseCommonDependencyObj_001, testing::ext::TestSize.Level0)
{
    std::shared_ptr<SoftbusListener> softbusListener = std::make_shared<SoftbusListener>();
    std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
    std::shared_ptr<DiscoveryManager> manager = std::make_shared<DiscoveryManager>(softbusListener, listener);
    bool ret = manager->CloseCommonDependencyObj();
    EXPECT_EQ(ret, true);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
