/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "UTTest_device_manager_notify.h"
#include "device_manager_notify.h"
#include "dm_device_info.h"
#include "ipc_remote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ipc_client_manager.h"
#include "ipc_set_useroperation_req.h"
#include "ipc_rsp.h"
#include "ipc_def.h"

#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerNotifyTest::SetUp() {}

void DeviceManagerNotifyTest::TearDown() {}

void DeviceManagerNotifyTest::SetUpTestCase() {}

void DeviceManagerNotifyTest::TearDownTestCase() {}
namespace {
/**
 * @tc.name: RegisterDeathRecipientCallback_001
 * @tc.desc: 1. set pkgName not null
 *              set dmInitCallback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeathRecipientCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDeathRecipientCallback_002
 * @tc.desc: 1. set pkgName not null
 *              set dmInitCallback null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeathRecipientCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    // 2. set checkMap not null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 5. check checkMap null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDeathRecipientCallback_003
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *           2. set checkMap null
 *           3. set testpkcName com.ohos.test1
 *           4. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           5. Get checkMap from DeviceManagerNotify with testpkcName
 *           6. check checkMap null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeathRecipientCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName com.ohos.test
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    std::shared_ptr<DmInitCallback> dmInitCallback = nullptr;
    // 2. set checkMap null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 4. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 5. Get checkMap from DeviceManagerNotify with testpkcName
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[testPkgName];
    // 6. check checkMap not null
    ASSERT_EQ(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDeathRecipientCallback_004
 * @tc.desc: 1. set pkgName not null
 *              set dmInitCallback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call checkMap OnRemoteDied
 *           7. check count is 1
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeathRecipientCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call checkMap OnRemoteDied
    checkMap->OnRemoteDied();
    // 7. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: RegisterDeathRecipientCallback_005
 * @tc.desc: 1. set pkgName not null
 *              set dmInitCallback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call checkMap OnRemoteDied
 *           7. check count is 1
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeathRecipientCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = nullptr;
    // 2. set checkMap null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 5. check checkMap not null
    if (checkMap == nullptr) {
        ASSERT_NE(count, 1);
        return;
    }
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDeathRecipientCallback_005
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeathRecipientCallback_006, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = nullptr;
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    if (checkMap == nullptr) {
        ASSERT_NE(count, 1);
        return;
    }
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeathRecipientCallback_001
 * @tc.desc: 1. set pkgName not null
 *              set dmInitCallback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with parameter
 *           7. Get checkMap from DeviceManagerNotify
 *           8. check checkMap null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeathRecipientCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().UnRegisterDeathRecipientCallback(pkgName);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 8 check checkMap null
    ASSERT_EQ(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeathRecipientCallback_002
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeathRecipientCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeathRecipientCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeathRecipientCallback_003
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. call checkMap OnRemoteDied
 *           10. check count is 1
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeathRecipientCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeathRecipientCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 9. call checkMap OnRemoteDied
    checkMap->OnRemoteDied();
    // 10. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: UnRegisterDeathRecipientCallback_004
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeathRecipientCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeathRecipientCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 9. check checkMap not null
    if (checkMap == nullptr) {
        ASSERT_NE(count, 1);
        return;
    }
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeathRecipientCallback_005
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeathRecipientCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName nullptr
    std::string testPkgName = "";
    // 7. call DeviceManagerNotifyTest UnRegisterDeathRecipientCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeathRecipientCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDeviceStateCallback_001
 * @tc.desc: 1. set pkgName not null
 *              set Callback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 * deviceTypeId
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceStateCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set Callback not null
    int count = 0;
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDeviceStateCallback_002
 * @tc.desc: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceStateCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 5. check checkMap null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDeviceStateCallback_003
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set Callback not null
 *           2. set checkMap null
 *           3. set testpkcName com.ohos.test1
 *           4. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *           5. Get checkMap from DeviceManagerNotify with testpkcName
 *           6. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceStateCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName com.ohos.test
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    std::shared_ptr<DeviceStateCallback> callback = nullptr;
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 4. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 5. Get checkMap from DeviceManagerNotify with testpkcName
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[testPkgName];
    // 6. check checkMap not null
    ASSERT_EQ(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDeviceStateCallback_004
 * @tc.desc: 1. set pkgName not null
 *              set Callback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call checkMap OnDeviceOnline
 *           7. check count is 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceStateCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set Callback not null
    int count = 0;
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call checkMap OnDeviceOnline
    DmDeviceInfo deviceInfo;
    checkMap->OnDeviceOnline(deviceInfo);
    // 7. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: RegisterDeviceStateCallback_005
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set Callback not null
 *           2. set checkMap null
 *           3. set testpkcName com.ohos.test1
 *           4. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *           5. Get checkMap from DeviceManagerNotify with testpkcName
 *           6. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceStateCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName com.ohos.test
    std::string pkgName = "com.ohos.test";
    int count = 0;
    // set dmInitCallback not null
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 4. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 5. Get checkMap from DeviceManagerNotify with testpkcName
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[testPkgName];
    // 6. check checkMap not null
    if (checkMap == nullptr) {
        ASSERT_NE(count, 1);
        return;
    }
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeviceStateCallback_001
 * @tc.desc: 1. set pkgName not null
 *              set Callback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with parameter
 *           7. Get checkMap from DeviceManagerNotify
 *           8. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceStateCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(pkgName);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 8 check checkMap null
    ASSERT_EQ(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeviceStateCallback_002
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set Callback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceStateCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeviceStateCallback_003
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set Callback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceStateCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 9. call checkMap OnDeviceOnline
    DmDeviceInfo deviceInfo;
    checkMap->OnDeviceOnline(deviceInfo);
    // 10. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: UnRegisterDeviceStateCallback_004
 * @tc.desc: 1. set pkgName not null
 *              set Callback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with parameter
 *           7. Get checkMap from DeviceManagerNotify
 *           8. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceStateCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(pkgName);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 8 check checkMap null
    if (checkMap == nullptr) {
        ASSERT_NE(count, 1);
        return;
    }
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDeviceStateCallback_005
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set Callback not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceStateCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DeviceStateCallback> callback = std::make_shared<DeviceStateCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceStateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, callback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "";
    // 7. call DeviceManagerNotifyTest UnRegisterDeviceStateCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 9. call checkMap OnDeviceOnline
    DmDeviceInfo deviceInfo;
    checkMap->OnDeviceOnline(deviceInfo);
    // 10. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: RegisterDiscoverCallback_001
 * @tc.desc: 1. set pkgName not null
 *              set Callback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDiscoveryCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    // set subscribeId not null
    uint16_t subscribeId = 0;
    // 2. set checkMap null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDiscoverCallback_002
 * @tc.desc: 1. set pkgName not null
 *              set Callback null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap mot null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDiscoveryCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    // set subscribeId not null
    uint16_t subscribeId = 0;
    // 2. set checkMap not null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 5. check checkMap null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDiscoverCallback_003
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set Callback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. set testpkcName com.ohos.test1
 *           4. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           5. Get checkMap from DeviceManagerNotify with testpkcName
 *           6. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDiscoveryCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName com.ohos.test
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    // set subscribeId not null
    uint16_t subscribeId = 0;
    // 2. set checkMap null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 4. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 5. Get checkMap from DeviceManagerNotify with testpkcName
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 6. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDiscoverCallback_004
 * @tc.desc: 1. set pkgName not null
 *              set Callback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call checkMap OnDiscoverySuccess
 *           7. check count is 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDiscoveryCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    // set subscribeId not null
    uint16_t subscribeId = 0;
    // 2. set checkMap null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call checkMap OnDiscoverySuccess
    checkMap->OnDiscoverySuccess(subscribeId);
    // 7. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: RegisterDiscoverCallback_005
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set Callback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. set testpkcName com.ohos.test1
 *           4. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           5. Get checkMap from DeviceManagerNotify with testpkcName
 *           6. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDiscoveryCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName com.ohos.test
    std::string pkgName = "com.ohos.test";
    int count = 0;
    // set dmInitCallback not null
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    uint16_t subscribeId = 0;
    // 2. set checkMap null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 4. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 5. Get checkMap from DeviceManagerNotify with testpkcName
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 6. check checkMap not null
    if (checkMap == nullptr) {
        ASSERT_NE(count, 1);
        return;
    }
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterDiscoverCallback_006
 * @tc.type: FUNC
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDiscoveryCallback_006, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    uint16_t subscribeId = 0;
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    EXPECT_EQ(DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_.count(pkgName), 0);
}

/**
 * @tc.name: UnRegisterDiscoverCallback_001
 * @tc.desc: 1. set pkgName not null
 *              set callback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with parameter
 *           7. Get checkMap from DeviceManagerNotify
 *           8. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDiscoveryCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    // set subscribeId not null
    uint16_t subscribeId = 0;
    // 2. set checkMap null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(pkgName, subscribeId);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 8 check checkMap null
    ASSERT_EQ(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDiscoverCallback_002
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDiscoveryCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    // set subscribeId not null
    uint16_t subscribeId = 0;
    // 2. set checkMap null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(testPkgName, subscribeId);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDiscoverCallback_003
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 *          10. call checkMap OnDiscoverySuccess
 *          11. check count is 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDiscoveryCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    // set subscribeId not null
    uint16_t subscribeId = 0;
    // 2. set checkMap null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(testPkgName, subscribeId);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 9 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 10. call checkMap OnDiscoverySuccess
    checkMap->OnDiscoverySuccess(subscribeId);
    // 11. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: UnRegisterDiscoverCallback_004
 * @tc.desc: 1. set pkgName not null
 *              set callback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with parameter
 *           7. Get checkMap from DeviceManagerNotify
 *           8. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDiscoveryCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    // set subscribeId not null
    uint16_t subscribeId = 0;
    // 2. set checkMap null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(pkgName, subscribeId);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 8 check checkMap null
    if (checkMap == nullptr) {
        ASSERT_NE(count, 1);
        return;
    }
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterDiscoverCallback_005
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 *          10. call checkMap OnDiscoverySuccess
 *          11. check count is 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDiscoveryCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DiscoveryCallback> callback = std::make_shared<DiscoveryCallbackTest>(count);
    // set subscribeId not null
    uint16_t subscribeId = 0;
    // 2. set checkMap null
    std::shared_ptr<DiscoveryCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "";
    // 7. call DeviceManagerNotifyTest UnRegisterDiscoverCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(testPkgName, subscribeId);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    // 9 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 10. call checkMap OnDiscoverySuccess
    checkMap->OnDiscoverySuccess(subscribeId);
    // 11. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: RegisterAuthenticateCallback_001
 * @tc.desc: 1. set pkgName not null
 *              set Callback not null
 *              set deviceId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterAuthenticateCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterAuthenticateCallback_002
 * @tc.desc: 1. set pkgName not null
 *              set Callback null
 *              set deviceId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterAuthenticateCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap not null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 5. check checkMap null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterAuthenticateCallback_003
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set Callback not null
 *              set deviceId not null
 *           2. set checkMap null
 *           3. set testpkcName com.ohos.test1
 *           4. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           5. Get checkMap from DeviceManagerNotify with testpkcName
 *           6. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterAuthenticateCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName com.ohos.test
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 4. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 5. Get checkMap from DeviceManagerNotify with testpkcName
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 6. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: RegisterAuthenticateCallback_004
 * @tc.desc: 1. set pkgName not null
 *              set Callback not null
 *              set deviceId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterAuthenticateCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call checkMap OnAuthResult
    std::string token = "1";
    int32_t status = 1;
    int32_t reason = 1;
    checkMap->OnAuthResult(deviceId, token, status, reason);
    // 7. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: RegisterAuthenticateCallback_005
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set Callback not null
 *              set deviceId not null
 *           2. set checkMap null
 *           3. set testpkcName com.ohos.test1
 *           4. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           5. Get checkMap from DeviceManagerNotify with testpkcName
 *           6. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterAuthenticateCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName com.ohos.test
    std::string pkgName = "com.ohos.test";
    int count = 0;
    // set dmInitCallback not null
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 4. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 5. Get checkMap from DeviceManagerNotify with testpkcName
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 6. check checkMap not null
    if (checkMap == nullptr) {
        ASSERT_NE(count, 1);
        return;
    }
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterAuthenticateCallback_001
 * @tc.desc: 1. set pkgName not null
 *              set callback not null
 *              set deviceId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with parameter
 *           7. Get checkMap from DeviceManagerNotify
 *           8. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterAuthenticateCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().UnRegisterAuthenticateCallback(pkgName, deviceId);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 8 check checkMap null
    ASSERT_EQ(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterAuthenticateCallback_002
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterAuthenticateCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterAuthenticateCallback(testPkgName, deviceId);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterAuthenticateCallback_003
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 *          10. call checkMap OnAuthResult
 *          11. check count is 1
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterAuthenticateCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterAuthenticateCallback(testPkgName, deviceId);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 9 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 10. call checkMap OnAuthResult
    std::string token = "1";
    int32_t status = 1;
    int32_t reason = 1;
    checkMap->OnAuthResult(deviceId, token, status, reason);
    // 11. check count is 1
    ASSERT_EQ(count, 1);
}

/**
 * @tc.name: UnRegisterAuthenticateCallback_004
 * @tc.desc: 1. set pkgName not null
 *              set callback not null
 *              set deviceId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with parameter
 *           7. Get checkMap from DeviceManagerNotify
 *           8. check checkMap null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterAuthenticateCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().UnRegisterAuthenticateCallback(pkgName, deviceId);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 8. check checkMap not null
    if (checkMap == nullptr) {
        ASSERT_NE(count, 1);
        return;
    }
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterAuthenticateCallback_005
 * @tc.desc: 1. set pkgName com.ohos.test
 *              set dmInitCallback not null
 *              set subscribeId not null
 *           2. set checkMap null
 *           3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
 *           4. Get checkMap from DeviceManagerNotify
 *           5. check checkMap not null
 *           6. set testpkcName com.ohos.test1
 *           7. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with testpkcName
 *           8. Get checkMap from DeviceManagerNotify
 *           9. check checkMap not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterAuthenticateCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<AuthenticateCallback> callback = std::make_shared<AuthenticateCallbackTest>(count);
    // set subscribeId not null
    std::string deviceId = "1";
    // 2. set checkMap null
    std::shared_ptr<AuthenticateCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterAuthenticateCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "";
    // 7. call DeviceManagerNotifyTest UnRegisterAuthenticateCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterAuthenticateCallback(testPkgName, deviceId);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/**
 * @tc.name: UnRegisterPackageCallback_001
 * @tc.desc: 1. RegisterDeathRecipientCallback with pkgName and dmInitCallback
 *           2. RegisterDeviceStateCallback with pkgName and deviceStateCallback
 *           3. RegisterDiscoverCallback with pkgName and deviceStateCallback and subscribeId
 *           4. RegisterAuthenticateCallback with pkgName and deviceId and authenticateCallback
 *           5. RegisterPublishCallback with pkgName and devicePublishCallback and publishId
 *           6. call DeviceManagerNotify UnRegisterPackageCallback with PkgName
 *           7. check if checkMap checkMap1 checkMap2 checkMap3 is null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterPackageCallback_001, testing::ext::TestSize.Level0)
{
    // 1. RegisterDeathRecipientCallback with pkgName and dmInitCallback
    std::string pkgName = "com.ohos.test";
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    ASSERT_NE(checkMap, nullptr);
    // 2. RegisterDeviceStateCallback with pkgName and deviceStateCallback
    int count1 = 0;
    std::shared_ptr<DeviceStateCallback> deviceStateCallback = std::make_shared<DeviceStateCallbackTest>(count1);
    std::shared_ptr<DeviceStateCallback> checkMap1 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, deviceStateCallback);
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    ASSERT_NE(checkMap1, nullptr);
    // 3. RegisterDiscoverCallback with pkgName and deviceStateCallback and subscribeId
    int count2 = 0;
    std::shared_ptr<DiscoveryCallback> deviceDiscoveryCallback = std::make_shared<DiscoveryCallbackTest>(count2);
    uint16_t subscribeId = 0;
    std::shared_ptr<DiscoveryCallback> checkMap2 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, deviceDiscoveryCallback);
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    ASSERT_NE(checkMap2, nullptr);
    // 4. RegisterAuthenticateCallback with pkgName and deviceId and authenticateCallback
    int count3 = 0;
    std::shared_ptr<AuthenticateCallback> authenticateCallback = std::make_shared<AuthenticateCallbackTest>(count3);
    std::string deviceId = "1";
    std::shared_ptr<AuthenticateCallback> checkMap3 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, authenticateCallback);
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    ASSERT_NE(checkMap3, nullptr);
    // 5. RegisterPublishCallback with pkgName and deviceStateCallback and subscribeId
    int count4 = 0;
    std::shared_ptr<PublishCallback> devicePublishCallback = std::make_shared<PublishCallbackTest>(count4);
    int32_t publishId = 0;
    std::shared_ptr<PublishCallback> checkMap4 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, devicePublishCallback);
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    ASSERT_NE(checkMap4, nullptr);
    // 6. call DeviceManagerNotify UnRegisterPackageCallback with PkgName
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(pkgName);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    // 7. check if checkMap checkMap1 checkMap2 checkMap3 is null
    ASSERT_EQ(checkMap, nullptr);
    ASSERT_EQ(checkMap1, nullptr);
    ASSERT_EQ(checkMap2, nullptr);
    ASSERT_EQ(checkMap3, nullptr);
    ASSERT_EQ(checkMap4, nullptr);
}

/**
 * @tc.name: UnRegisterPackageCallback_002
 * @tc.desc: 1. RegisterDeathRecipientCallback with pkgName and dmInitCallback
 *           2. RegisterDeviceStateCallback with pkgName and deviceStateCallback
 *           3. RegisterDiscoverCallback with pkgName and deviceStateCallback and subscribeId
 *           4. RegisterAuthenticateCallback with pkgName and deviceId and authenticateCallback
 *           5. RegisterPublishCallback with pkgName and devicePublishCallback and publishId
 *           6. call DeviceManagerNotify UnRegisterPackageCallback with testPkgName
 *           7. check if checkMap checkMap1 checkMap2 checkMap3 is not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterPackageCallback_002, testing::ext::TestSize.Level0)
{
    // 1. RegisterDeathRecipientCallback with pkgName and dmInitCallback
    std::string pkgName = "com.ohos.test";
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    ASSERT_NE(checkMap, nullptr);
    // 2. RegisterDeviceStateCallback with pkgName and deviceStateCallback
    int count1 = 0;
    std::shared_ptr<DeviceStateCallback> deviceStateCallback = std::make_shared<DeviceStateCallbackTest>(count1);
    std::shared_ptr<DeviceStateCallback> checkMap1 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, deviceStateCallback);
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    ASSERT_NE(checkMap1, nullptr);
    // 3. RegisterDiscoverCallback with pkgName and deviceStateCallback and subscribeId
    int count2 = 0;
    std::shared_ptr<DiscoveryCallback> deviceDiscoveryCallback = std::make_shared<DiscoveryCallbackTest>(count2);
    uint16_t subscribeId = 0;
    std::shared_ptr<DiscoveryCallback> checkMap2 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, deviceDiscoveryCallback);
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    ASSERT_NE(checkMap2, nullptr);
    // 4. RegisterAuthenticateCallback with pkgName and deviceId and authenticateCallback
    int count3 = 0;
    std::shared_ptr<AuthenticateCallback> authenticateCallback = std::make_shared<AuthenticateCallbackTest>(count3);
    std::string deviceId = "1";
    std::shared_ptr<AuthenticateCallback> checkMap3 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, authenticateCallback);
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    ASSERT_NE(checkMap3, nullptr);
    // 5. RegisterPublishCallback with pkgName and deviceStateCallback and publishId
    int count4 = 0;
    std::shared_ptr<PublishCallback> devicePublishCallback = std::make_shared<PublishCallbackTest>(count4);
    int32_t publishId = 0;
    std::shared_ptr<PublishCallback> checkMap4 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, devicePublishCallback);
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    ASSERT_NE(checkMap4, nullptr);
    // 6. call DeviceManagerNotify UnRegisterPackageCallback with testPkgName
    std::string testPkgName = "com.ohos.test1";
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(testPkgName);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    // 7. check if checkMap checkMap1 checkMap2 checkMap3 is null
    ASSERT_NE(checkMap, nullptr);
    ASSERT_NE(checkMap1, nullptr);
    ASSERT_NE(checkMap2, nullptr);
    ASSERT_NE(checkMap3, nullptr);
    ASSERT_NE(checkMap4, nullptr);
}

/**
 * @tc.name: UnRegisterPackageCallback_003
 * @tc.desc: 1. RegisterDeathRecipientCallback with pkgName and dmInitCallback
 *           2. RegisterDeviceStateCallback with pkgName and deviceStateCallback
 *           3. RegisterDiscoverCallback with pkgName and deviceStateCallback and subscribeId
 *           4. RegisterAuthenticateCallback with pkgName and deviceId and authenticateCallback
 *           5. RegisterPublishCallback with pkgName and devicePublishCallback and publishId
 *           6. call DeviceManagerNotify UnRegisterPackageCallback with testPkgName
 *           7. check if checkMap checkMap1 checkMap2 checkMap3 is not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterPackageCallback_003, testing::ext::TestSize.Level0)
{
    // 1. RegisterDeathRecipientCallback with pkgName and dmInitCallback
    std::string pkgName = "com.ohos.test";
    int count[5] = {0, 0, 0, 0, 0};
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count[0]);
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName], nullptr);
    // 2. RegisterDeviceStateCallback with pkgName and deviceStateCallback
    std::shared_ptr<DeviceStateCallback> deviceStateCallback = std::make_shared<DeviceStateCallbackTest>(count[1]);
    std::shared_ptr<DeviceStateCallback> checkMap1 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, deviceStateCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName], nullptr);
    // 3. RegisterDiscoverCallback with pkgName and deviceStateCallback and subscribeId
    std::shared_ptr<DiscoveryCallback> deviceDiscoveryCallback = std::make_shared<DiscoveryCallbackTest>(count[2]);
    uint16_t subscribeId = 0;
    std::shared_ptr<DiscoveryCallback> checkMap2 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, deviceDiscoveryCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId], nullptr);
    // 4. RegisterAuthenticateCallback with pkgName and deviceId and authenticateCallback
    std::shared_ptr<AuthenticateCallback> authenticateCallback = std::make_shared<AuthenticateCallbackTest>(count[3]);
    std::string deviceId = "1";
    std::shared_ptr<AuthenticateCallback> checkMap3 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, authenticateCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId], nullptr);
    // 5. RegisterPublishCallback with pkgName and devicePublishCallback and publishId
    std::shared_ptr<PublishCallback> devicePublishCallback = std::make_shared<PublishCallbackTest>(count[4]);
    int32_t publishId = 0;
    std::shared_ptr<PublishCallback> checkMap4 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, devicePublishCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId], nullptr);
    // 6. call DeviceManagerNotify UnRegisterPackageCallback with testPkgName
    std::string testPkgName = "com.ohos.test1";
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(testPkgName);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    // 6. check if checkMap checkMap1 checkMap2 checkMap3 is null
    ASSERT_NE(checkMap, nullptr);
    ASSERT_NE(checkMap1, nullptr);
    ASSERT_NE(checkMap2, nullptr);
    ASSERT_NE(checkMap3, nullptr);
    ASSERT_NE(checkMap4, nullptr);
    // 7. call checkMap OnRemoteDied()
    checkMap->OnRemoteDied();
    // call checkMap1 OnDeviceOnline
    DmDeviceInfo deviceInfo;
    checkMap1->OnDeviceOnline(deviceInfo);
    // call checkMap2 OnDeviceFound
    checkMap2->OnDeviceFound(subscribeId, deviceInfo);
    // call checkMap3 OnAuthResult
    checkMap3->OnAuthResult(deviceId, "1", 1, 1);
    // call checkMap4 OnPublishResult
    checkMap4->OnPublishResult(publishId, 0);
    // 8. check count count1 count2 count3 is 1
    for (uint32_t i = 0; i < 5; i++) {
        ASSERT_EQ(count[i], 1);
    }
}

/**
 * @tc.name: UnRegisterPackageCallback_004
 * @tc.desc: 1. RegisterDeathRecipientCallback with pkgName and dmInitCallback
 *           2. RegisterDeviceStateCallback with pkgName and deviceStateCallback
 *           3. RegisterDiscoverCallback with pkgName and deviceStateCallback and subscribeId
 *           4. RegisterAuthenticateCallback with pkgName and deviceId and authenticateCallback
 *           5. RegisterPublishCallback with pkgName and devicePublishCallback and publishId
 *           6. call DeviceManagerNotify UnRegisterPackageCallback with PkgName
 *           7. check if checkMap checkMap1 checkMap2 checkMap3 is null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterPackageCallback_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int count[5] = {0, 0, 0, 0, 0};
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count[0]);
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName], nullptr);
    std::shared_ptr<DeviceStateCallback> deviceStateCallback = std::make_shared<DeviceStateCallbackTest>(count[1]);
    std::shared_ptr<DeviceStateCallback> checkMap1 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, deviceStateCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName], nullptr);
    std::shared_ptr<DiscoveryCallback> deviceDiscoveryCallback = std::make_shared<DiscoveryCallbackTest>(count[2]);
    uint16_t subscribeId = 0;
    std::shared_ptr<DiscoveryCallback> checkMap2 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, deviceDiscoveryCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId], nullptr);
    std::shared_ptr<AuthenticateCallback> authenticateCallback = std::make_shared<AuthenticateCallbackTest>(count[3]);
    std::string deviceId = "1";
    std::shared_ptr<AuthenticateCallback> checkMap3 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, authenticateCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId], nullptr);
    std::shared_ptr<PublishCallback> devicePublishCallback = std::make_shared<PublishCallbackTest>(count[4]);
    int32_t publishId = 0;
    std::shared_ptr<PublishCallback> checkMap4 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, devicePublishCallback);
    ASSERT_NE(DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId], nullptr);
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(pkgName);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    if (checkMap == nullptr && checkMap1 == nullptr && checkMap2 == nullptr && checkMap3 == nullptr &&
        checkMap4 == nullptr) {
        for (uint32_t i = 0; i < 5; i++) {
            ASSERT_NE(count[i], 1);
        }
        return;
    }
    ASSERT_EQ(checkMap, nullptr);
    ASSERT_EQ(checkMap1, nullptr);
    ASSERT_EQ(checkMap2, nullptr);
    ASSERT_EQ(checkMap3, nullptr);
    ASSERT_EQ(checkMap4, nullptr);
}

/**
 * @tc.name: UnRegisterPackageCallback_005
 * @tc.desc: 1. RegisterDeathRecipientCallback with pkgName and dmInitCallback
 *           2. RegisterDeviceStateCallback with pkgName and deviceStateCallback
 *           3. RegisterDiscoverCallback with pkgName and deviceStateCallback and subscribeId
 *           4. RegisterAuthenticateCallback with pkgName and deviceId and authenticateCallback
 *           5. RegisterPublishCallback with pkgName and devicePublishCallback and publishId
 *           6. call DeviceManagerNotify UnRegisterPackageCallback with testPkgName
 *           7. check if checkMap checkMap1 checkMap2 checkMap3 is not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterPackageCallback_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    ASSERT_NE(checkMap, nullptr);
    int count1 = 0;
    std::shared_ptr<DeviceStateCallback> deviceStateCallback = std::make_shared<DeviceStateCallbackTest>(count1);
    std::shared_ptr<DeviceStateCallback> checkMap1 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, deviceStateCallback);
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    ASSERT_NE(checkMap1, nullptr);
    int count2 = 0;
    std::shared_ptr<DiscoveryCallback> deviceDiscoveryCallback = std::make_shared<DiscoveryCallbackTest>(count2);
    uint16_t subscribeId = 0;
    std::shared_ptr<DiscoveryCallback> checkMap2 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, deviceDiscoveryCallback);
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    ASSERT_NE(checkMap2, nullptr);
    int count3 = 0;
    std::shared_ptr<AuthenticateCallback> authenticateCallback = std::make_shared<AuthenticateCallbackTest>(count3);
    std::string deviceId = "1";
    std::shared_ptr<AuthenticateCallback> checkMap3 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, authenticateCallback);
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    ASSERT_NE(checkMap3, nullptr);
    int count4 = 0;
    std::shared_ptr<PublishCallback> devicePublishCallback = std::make_shared<PublishCallbackTest>(count4);
    int32_t publishId = 0;
    std::shared_ptr<PublishCallback> checkMap4 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, devicePublishCallback);
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][subscribeId];
    ASSERT_NE(checkMap4, nullptr);
    std::string testPkgName = "";
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(testPkgName);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    ASSERT_NE(checkMap, nullptr);
    ASSERT_NE(checkMap1, nullptr);
    ASSERT_NE(checkMap2, nullptr);
    ASSERT_NE(checkMap3, nullptr);
    ASSERT_NE(checkMap4, nullptr);
}

/**
 * @tc.name: UnRegisterPackageCallback_005
 * @tc.desc: 1. RegisterDeathRecipientCallback with pkgName and dmInitCallback
 *           2. RegisterDeviceStateCallback with pkgName and deviceStateCallback
 *           3. RegisterDiscoverCallback with pkgName and deviceStateCallback and subscribeId
 *           4. RegisterAuthenticateCallback with pkgName and deviceId and authenticateCallback
 *           5. RegisterPublishCallback with pkgName and devicePublishCallback and publishId
 *           6. call DeviceManagerNotify UnRegisterPackageCallback with testPkgName
 *           7. check if checkMap checkMap1 checkMap2 checkMap3 is not null
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterPackageCallback_006, testing::ext::TestSize.Level0)
{
    std::string pkgName;
    int count = 0;
    std::shared_ptr<DmInitCallback> dmInitCallback = std::make_shared<DmInitCallbackTest>(count);
    std::shared_ptr<DmInitCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeathRecipientCallback(pkgName, dmInitCallback);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    ASSERT_EQ(checkMap, nullptr);
    int count1 = 0;
    std::shared_ptr<DeviceStateCallback> deviceStateCallback = std::make_shared<DeviceStateCallbackTest>(count1);
    std::shared_ptr<DeviceStateCallback> checkMap1 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeviceStateCallback(pkgName, deviceStateCallback);
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    ASSERT_EQ(checkMap1, nullptr);
    int count2 = 0;
    std::shared_ptr<DiscoveryCallback> deviceDiscoveryCallback = std::make_shared<DiscoveryCallbackTest>(count2);
    uint16_t subscribeId = 0;
    std::shared_ptr<DiscoveryCallback> checkMap2 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDiscoveryCallback(pkgName, subscribeId, deviceDiscoveryCallback);
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    ASSERT_EQ(checkMap2, nullptr);
    int count3 = 0;
    std::shared_ptr<AuthenticateCallback> authenticateCallback = std::make_shared<AuthenticateCallbackTest>(count3);
    std::string deviceId = "1";
    std::shared_ptr<AuthenticateCallback> checkMap3 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterAuthenticateCallback(pkgName, deviceId, authenticateCallback);
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    ASSERT_EQ(checkMap3, nullptr);
    int count4 = 0;
    std::shared_ptr<PublishCallback> devicePublishCallback = std::make_shared<PublishCallbackTest>(count4);
    int32_t publishId = 0;
    std::shared_ptr<PublishCallback> checkMap4 = nullptr;
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, devicePublishCallback);
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][subscribeId];
    ASSERT_EQ(checkMap4, nullptr);
    std::string testPkgName = "";
    DeviceManagerNotify::GetInstance().UnRegisterPackageCallback(testPkgName);
    checkMap = DeviceManagerNotify::GetInstance().dmInitCallback_[pkgName];
    checkMap1 = DeviceManagerNotify::GetInstance().deviceStateCallback_[pkgName];
    checkMap2 = DeviceManagerNotify::GetInstance().deviceDiscoveryCallbacks_[pkgName][subscribeId];
    checkMap3 = DeviceManagerNotify::GetInstance().authenticateCallback_[pkgName][deviceId];
    checkMap4 = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    ASSERT_EQ(checkMap, nullptr);
    ASSERT_EQ(checkMap1, nullptr);
    ASSERT_EQ(checkMap2, nullptr);
    ASSERT_EQ(checkMap3, nullptr);
    ASSERT_EQ(checkMap4, nullptr);
}
}
} // namespace DistributedHardware
} // namespace OHOS
