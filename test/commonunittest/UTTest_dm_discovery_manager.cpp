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
 * @tc.desc: keeping pkgame unchanged, call StartDeviceDiscovery once,
 *           extra is not empty and return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmSubscribeInfo subscribeInfo;
    subscribeInfo.subscribeId = 1;
    const std::string extra = "com.ohos.test";
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeInfo.subscribeId);
}

/**
 * @tc.name:StartDeviceDiscovery_002
 * @tc.desc: keeping pkgame unchanged, call StartDeviceDiscovery once,
 *           extra is empty and return ERR_DM_DISCOVERY_REPEATED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmSubscribeInfo subscribeInfo;
    subscribeInfo.subscribeId = 1;
    std::string extra;
    std::queue<std::string> emptyQueue;
    discoveryMgr_->discoveryQueue_ = emptyQueue;
    discoveryMgr_->discoveryQueue_.push(pkgName);
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_REPEATED);
    discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeInfo.subscribeId);
}

/**
 * @tc.name:StartDeviceDiscovery_003
 * @tc.desc: keeping pkgame unchanged, call StartDeviceDiscovery once,
 *           extra is empty,  discoveryQueue_ is empty, and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    std::string extra;
    DmSubscribeInfo subscribeInfo;
    subscribeInfo.subscribeId = 1;
    std::queue<std::string> emptyQueue;
    discoveryMgr_->discoveryQueue_ = emptyQueue;
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
    ASSERT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
    discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeInfo.subscribeId);
}

/**
 * @tc.name:StartDeviceDiscovery_004
 * @tc.desc: pkgame changed, call StartDeviceDiscovery once,
 *           discoveryQueue is not empty and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    uint16_t subscribeId = 1;
    std::string filterOptions;
    std::queue<std::string> emptyQueue;
    discoveryMgr_->discoveryQueue_ = emptyQueue;
    discoveryMgr_->discoveryQueue_.push(pkgName);
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    ASSERT_EQ(ret, ERR_DM_DISCOVERY_REPEATED);
    discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeId);
}

/**
 * @tc.name:StartDeviceDiscovery_005
 * @tc.desc: pkgame changed, call StartDeviceDiscovery once,
 *           discoveryQueue is empty and return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StartDeviceDiscovery_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    uint16_t subscribeId = 1;
    std::string filterOptions;
    std::queue<std::string> emptyQueue;
    discoveryMgr_->discoveryQueue_ = emptyQueue;
    int32_t ret = discoveryMgr_->StartDeviceDiscovery(pkgName, subscribeId, filterOptions);
    ASSERT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
    discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeId);
}

/**
 * @tc.name: StopDeviceDiscovery_001
 * @tc.desc: return ERR_DM_INPUT_PARA_INVALID
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StopDeviceDiscovery_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    uint16_t subscribeId = 1;
    int32_t ret = discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: StopDeviceDiscovery_002
 * @tc.desc: return ERR_DM_DISCOVERY_FAILED
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, StopDeviceDiscovery_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    uint16_t subscribeId = 1;
    int32_t ret = discoveryMgr_->StopDeviceDiscovery(pkgName, subscribeId);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_FAILED);
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
 * @tc.name: OnDeviceFound_003
 * @tc.desc: set pkgName not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDeviceFound_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmDeviceInfo info;
    bool isOnline = true;
    discoveryMgr_->discoveryContextMap_.clear();
    discoveryMgr_->OnDeviceFound(pkgName, info, isOnline);
    EXPECT_EQ(discoveryMgr_->discoveryContextMap_.empty(), true);
}

/**
 * @tc.name: OnDeviceFound_004
 * @tc.desc: set pkgName not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDeviceFound_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmDeviceInfo info;
    bool isOnline = true;
    DmDiscoveryContext context;
    discoveryMgr_->discoveryContextMap_.emplace(pkgName, context);
    discoveryMgr_->OnDeviceFound(pkgName, info, isOnline);
    EXPECT_EQ(discoveryMgr_->discoveryContextMap_.empty(), false);
}

/**
 * @tc.name: OnDeviceFound_005
 * @tc.desc: set pkgName not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDeviceFound_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmDeviceBasicInfo info;
    int32_t range = 0;
    bool isOnline = true;
    discoveryMgr_->discoveryContextMap_.clear();
    discoveryMgr_->OnDeviceFound(pkgName, info, range, isOnline);
    EXPECT_EQ(discoveryMgr_->discoveryContextMap_.empty(), true);
}

/**
 * @tc.name: OnDeviceFound_006
 * @tc.desc: set pkgName not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDeviceFound_006, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    DmDeviceBasicInfo info;
    int32_t range = 0;
    bool isOnline = true;
    DmDiscoveryContext context;
    discoveryMgr_->discoveryContextMap_.emplace(pkgName, context);
    discoveryMgr_->OnDeviceFound(pkgName, info, range, isOnline);
    EXPECT_EQ(discoveryMgr_->discoveryContextMap_.empty(), false);
}

/**
 * @tc.name: GetAuthForm_001
 * @tc.desc: set localDeviceId null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, GetAuthForm_001, testing::ext::TestSize.Level0)
{
    std::string localDeviceId;
    std::string deviceId;
    bool isTrusted = true;
    DmAuthForm authForm;
    int32_t ret = discoveryMgr_->GetAuthForm(localDeviceId, deviceId, isTrusted, authForm);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetAuthForm_002
 * @tc.desc: set localDeviceId not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, GetAuthForm_002, testing::ext::TestSize.Level0)
{
    std::string localDeviceId = "125462";
    std::string deviceId;
    bool isTrusted = true;
    DmAuthForm authForm;
    int32_t ret = discoveryMgr_->GetAuthForm(localDeviceId, deviceId, isTrusted, authForm);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}

/**
 * @tc.name: GetAuthForm_003
 * @tc.desc: set localDeviceId not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, GetAuthForm_003, testing::ext::TestSize.Level0)
{
    std::string localDeviceId = "125462";
    std::string deviceId = "236541";
    bool isTrusted = true;
    DmAuthForm authForm;
    discoveryMgr_->hiChainConnector_ = nullptr;
    int32_t ret = discoveryMgr_->GetAuthForm(localDeviceId, deviceId, isTrusted, authForm);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: GetAuthForm_004
 * @tc.desc: set localDeviceId not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, GetAuthForm_004, testing::ext::TestSize.Level0)
{
    std::string localDeviceId = "125462";
    std::string deviceId = "236541";
    bool isTrusted = true;
    DmAuthForm authForm;
    discoveryMgr_->softbusConnector_ = nullptr;
    int32_t ret = discoveryMgr_->GetAuthForm(localDeviceId, deviceId, isTrusted, authForm);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}

/**
 * @tc.name: GetAuthForm_005
 * @tc.desc: set localDeviceId not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, GetAuthForm_005, testing::ext::TestSize.Level0)
{
    std::string localDeviceId = "125462";
    std::string deviceId = "236541";
    bool isTrusted = true;
    DmAuthForm authForm;
    discoveryMgr_->softbusConnector_ = std::make_shared<SoftbusConnector>();
    discoveryMgr_->hiChainConnector_ = std::make_shared<HiChainConnector>();
    int32_t ret = discoveryMgr_->GetAuthForm(localDeviceId, deviceId, isTrusted, authForm);
    EXPECT_EQ(ret, DM_OK);
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
 * @tc.name: OnDiscoveryFailed_003
 * @tc.desc: The OnDeviceFound function takes the wrong case and emptying pkgName
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DmDiscoveryManagerTest, OnDiscoveryFailed_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld";
    int32_t subscribeId = 1;
    int32_t failedReason = 3;
    discoveryMgr_->discoveryQueue_.push(pkgName);
    DmDiscoveryContext context;
    discoveryMgr_->discoveryContextMap_.emplace(pkgName, context);
    discoveryMgr_->OnDiscoveryFailed(pkgName, subscribeId, failedReason);
    EXPECT_EQ(discoveryMgr_->discoveryContextMap_.empty(), true);
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

HWTEST_F(DmDiscoveryManagerTest, HandleDiscoveryTimeout_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::queue<std::string> emptyQueue;
    discoveryMgr_->discoveryQueue_ = emptyQueue;
    discoveryMgr_->HandleDiscoveryTimeout(pkgName);
    EXPECT_EQ(discoveryMgr_->discoveryQueue_.empty(), true);
}

HWTEST_F(DmDiscoveryManagerTest, HandleDiscoveryTimeout_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld_new";
    discoveryMgr_->discoveryQueue_.push(pkgName);
    discoveryMgr_->HandleDiscoveryTimeout(pkgName);
    EXPECT_EQ(discoveryMgr_->discoveryQueue_.empty(), false);
}

HWTEST_F(DmDiscoveryManagerTest, HandleDiscoveryTimeout_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.helloworld_new";
    discoveryMgr_->discoveryQueue_.push(pkgName);
    DmDiscoveryContext context;
    discoveryMgr_->discoveryContextMap_.emplace(pkgName, context);
    discoveryMgr_->HandleDiscoveryTimeout(pkgName);
    EXPECT_EQ(discoveryMgr_->discoveryQueue_.empty(), false);
}

HWTEST_F(DmDiscoveryManagerTest, CheckDiscoveryQueue_001, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    std::queue<std::string> emptyQueue;
    discoveryMgr_->discoveryQueue_ = emptyQueue;
    int32_t ret = discoveryMgr_->CheckDiscoveryQueue(pkgName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DmDiscoveryManagerTest, CheckDiscoveryQueue_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "ohos_test";
    std::queue<std::string> emptyQueue;
    discoveryMgr_->discoveryQueue_ = emptyQueue;
    discoveryMgr_->discoveryQueue_.push(pkgName);
    int32_t ret = discoveryMgr_->CheckDiscoveryQueue(pkgName);
    EXPECT_EQ(ret, ERR_DM_DISCOVERY_REPEATED);
}

HWTEST_F(DmDiscoveryManagerTest, CheckDiscoveryQueue_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "pkgName";
    std::queue<std::string> emptyQueue;
    discoveryMgr_->discoveryQueue_ = emptyQueue;
    std::string frontPkgName = "ohos_test";
    discoveryMgr_->discoveryQueue_.push(frontPkgName);
    int32_t ret = discoveryMgr_->CheckDiscoveryQueue(pkgName);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
