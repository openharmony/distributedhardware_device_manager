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
namespace {
/*
 * Feature: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * Function: DeviceManagerNotifyTest
 * SubFunction: RegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * EnvConditions: RegisterDeviceManagerFaCallback success.
 * CaseDescription: 1. set pkgName not null
 *                     set dmUiCallback_ not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceManagerFaCallback_001, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int count = 0;
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = std::make_shared<DeviceManagerFaCallbackTest>(count);
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    ASSERT_NE(checkMap, nullptr);
}

/*
 * Feature: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * Function: DeviceManagerNotifyTest
 * SubFunction: RegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set dmUiCallback_ null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap null
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceManagerFaCallback_002, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = nullptr;
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    ASSERT_EQ(checkMap, nullptr);
}

/*
 * Feature: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * Function: DeviceManagerNotifyTest
 * SubFunction: RegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName com.ohos.test
 *                     set dmUiCallback_ not null
 *                  2. set checkMap null
 *                  3. set testpkcName com.ohos.test1
 *                  4. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  5. Get checkMap from DeviceManagerNotify with testpkcName
 *                  6. check checkMap null
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceManagerFaCallback_003, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = nullptr;
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    std::string testPkgName = "com.ohos.test1";
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[testPkgName];
    ASSERT_EQ(checkMap, nullptr);
}

/*
 * Feature: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * Function: DeviceManagerNotifyTest
 * SubFunction: RegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * EnvConditions: RegisterDeviceManagerFaCallback success.
 * CaseDescription: 1. set pkgName not null
 *                     set dmUiCallback_ not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceManagerFaCallback_004, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int count = 0;
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = std::make_shared<DeviceManagerFaCallbackTest>(count);
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    ASSERT_NE(checkMap, nullptr);
    std::string paramJson = "test";
    if (checkMap != nullptr) {
        checkMap->OnCall(paramJson);
    }
    ASSERT_EQ(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * Function: DeviceManagerNotifyTest
 * SubFunction: RegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest RegisterDeviceManagerFaCallback
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName com.ohos.test
 *                     set dmUiCallback_ not null
 *                  2. set checkMap null
 *                  3. set testpkcName com.ohos.test1
 *                  4. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  5. Get checkMap from DeviceManagerNotify with testpkcName
 *                  6. check checkMap null
 */
HWTEST_F(DeviceManagerNotifyTest, RegisterDeviceManagerFaCallback_005, testing::ext::TestSize.Level0)
{
    std::string pkgName = "com.ohos.test";
    int count = 0;
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = std::make_shared<DeviceManagerFaCallbackTest>(count);
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    std::string testPkgName = "com.ohos.test1";
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[testPkgName];
    ASSERT_EQ(checkMap, nullptr);
}

/*
 * Feature: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback
 * Function: DeviceManagerNotifyTest
 * SubFunction: UnRegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback
 * EnvConditions: UnRegisterDeviceManagerFaCallback success.
 * CaseDescription: 1. set pkgName not null
 *                     set dmUiCallback_ not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with parameter
 *                  7. Get checkMap from DeviceManagerNotify
 *                  8. check checkMap null
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceManagerFaCallback_001, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = std::make_shared<DeviceManagerFaCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with parameter
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 8 check checkMap null
    ASSERT_EQ(checkMap, nullptr);
}

/*
 * Feature: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback
 * Function: DeviceManagerNotifyTest
 * SubFunction: UnRegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName com.ohos.test
 *                     set dmInitCallback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with testpkcName
 *                  8. Get checkMap from DeviceManagerNotify
 *                  9. check checkMap not null
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceManagerFaCallback_002, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmUiCallback_ not null
    int count = 0;
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = std::make_shared<DeviceManagerFaCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 8 check checkMap not null
    ASSERT_NE(checkMap, nullptr);
}

/*
 * Feature: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback_003
 * Function: DeviceManagerNotifyTest
 * SubFunction: UnRegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName com.ohos.test
 *                     set dmInitCallback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with testpkcName
 *                  8. Get checkMap from DeviceManagerNotify
 *                  9. check checkMap not null
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceManagerFaCallback_003, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmUiCallback_ not null
    int count = 0;
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = std::make_shared<DeviceManagerFaCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 9. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 10. call checkMap OnCall
    std::string paramJson = "test";
    if (checkMap != nullptr) {
        checkMap->OnCall(paramJson);
    }
    ASSERT_EQ(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback_004
 * Function: DeviceManagerNotifyTest
 * SubFunction: UnRegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback
 * EnvConditions: UnRegisterDeviceManagerFaCallback success.
 * CaseDescription: 1. set pkgName not null
 *                     set dmUiCallback_ not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with parameter
 *                  7. Get checkMap from DeviceManagerNotify
 *                  8. check checkMap null
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceManagerFaCallback_004, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmInitCallback not null
    int count = 0;
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = std::make_shared<DeviceManagerFaCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with parameter
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(pkgName);
    // 7. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 8 check checkMap null
    ASSERT_EQ(checkMap, nullptr);
}

/*
 * Feature: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback_005
 * Function: DeviceManagerNotifyTest
 * SubFunction: UnRegisterDeviceManagerFaCallback
 * FunctionPoints: DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName com.ohos.test
 *                     set dmInitCallback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with testpkcName
 *                  8. Get checkMap from DeviceManagerNotify
 *                  9. check checkMap not null
 */
HWTEST_F(DeviceManagerNotifyTest, UnRegisterDeviceManagerFaCallback_005, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set dmUiCallback_ not null
    int count = 0;
    std::shared_ptr<DeviceManagerUiCallback> dmUiCallback_ = std::make_shared<DeviceManagerFaCallbackTest>(count);
    // 2. set checkMap null
    std::shared_ptr<DeviceManagerUiCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterDeviceManagerFaCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterDeviceManagerFaCallback(pkgName, dmUiCallback_);
    // 4. Get checkMap from DeviceManagerNotify RegisterDeathRecipientCallback
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "";
    // 7. call DeviceManagerNotifyTest UnRegisterDeviceManagerFaCallback with testpkcName
    DeviceManagerNotify::GetInstance().UnRegisterDeviceManagerFaCallback(testPkgName);
    // 8. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().dmUiCallback_[pkgName];
    // 9. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    // 10. call checkMap OnCall
    std::string paramJson = "test";
    if (checkMap != nullptr) {
        checkMap->OnCall(paramJson);
    }
    ASSERT_EQ(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest OnRemoteDied
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnRemoteDied
 * FunctionPoints: DeviceManagerNotifyTest OnRemoteDied
 * EnvConditions: OnRemoteDied success.
 * CaseDescription: 1. set pkgName not null
 *                     set dmInitCallback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeathRecipientCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnRemoteDied
 *                  7. check if dmInitCallback OnRemoteDied called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnRemoteDied1, testing::ext::TestSize.Level0)
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
    // 6. call DeviceManagerNotify OnRemoteDied
    DeviceManagerNotify::GetInstance().OnRemoteDied();
    // 7. check if dmInitCallback OnRemoteDied called
    ASSERT_EQ(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOnline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOnline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOnline
 * EnvConditions: OnDeviceOnline success.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDeviceOnline
 *                  7. check if callback OnDeviceOnline called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOnline_001, testing::ext::TestSize.Level0)
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
    // 6. call DeviceManagerNotify OnDeviceOnline
    DmDeviceInfo deviceInfo;
    DeviceManagerNotify::GetInstance().OnDeviceOnline(pkgName, deviceInfo);
    // 7. check if callback OnDeviceOnline called
    sleep(1);
    ASSERT_EQ(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOnline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOnline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOnline
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceOnline with testpkcName
 *                  8. check if callback OnDeviceOnline called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOnline_002, testing::ext::TestSize.Level0)
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
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotify OnDeviceOnline with testpkcName
    DmDeviceInfo deviceInfo;
    // 8. check if callback OnDeviceOnline called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceOnline(testPkgName, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOnline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOnline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOnline
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceOnline with testpkcName
 *                  8. check if callback OnDeviceOnline called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOnline_003, testing::ext::TestSize.Level0)
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
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "";
    // 7. call DeviceManagerNotify OnDeviceOnline with testpkcName
    DmDeviceInfo deviceInfo;
    // 8. check if callback OnDeviceOnline called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceOnline(testPkgName, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOnline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOnline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOnline
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceOnline with testpkcName
 *                  8. check if callback OnDeviceOnline called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOnline_004, testing::ext::TestSize.Level0)
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
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "111";
    // 7. call DeviceManagerNotify OnDeviceOnline with testpkcName
    DmDeviceInfo deviceInfo;
    DeviceManagerNotify::GetInstance().OnDeviceOnline(testPkgName, deviceInfo);
    // 8. check if callback OnDeviceOnline called
    ASSERT_NE(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOnline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOnline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOnline
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify UnRegisterDeviceStateCallback withtestPkgName
 *                  7. call DeviceManagerNotify OnDeviceOnline with testpkcName
 *                  8. check if callback OnDeviceOnline called
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOnline_005, testing::ext::TestSize.Level0)
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
    std::string testPkgName = "";
    // 6. call DeviceManagerNotify UnRegisterDeviceStateCallback withtestPkgName
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(testPkgName);
    // 7. call DeviceManagerNotify OnDeviceOnline with testpkcName
    DmDeviceInfo deviceInfo;
    // 8. check if callback OnDeviceOnline called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceOnline(testPkgName, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOffline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOffline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOffline
 * EnvConditions: OnDeviceOffline success.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDeviceOffline
 *                  7. check if callback OnDeviceOffline called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOffline_001, testing::ext::TestSize.Level0)
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
    // 6. call DeviceManagerNotify OnDeviceOffline
    DmDeviceInfo deviceInfo;
    DeviceManagerNotify::GetInstance().OnDeviceOffline(pkgName, deviceInfo);
    // 7. check if callback OnDeviceOffline called
    sleep(1);
    ASSERT_EQ(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOffline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOffline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOffline
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceOffline with testpkcName
 *                  8. check if callback OnDeviceOffline called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOffline_002, testing::ext::TestSize.Level0)
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
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotify OnDeviceOffline with testpkcName
    DmDeviceInfo deviceInfo;
    // 8. check if callback OnDeviceOffline called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceOffline(testPkgName, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOffline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOffline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOffline
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceOffline with testpkcName
 *                  8. check if callback OnDeviceOffline called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOffline_003, testing::ext::TestSize.Level0)
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
    // 6. set testpkcName null
    std::string testPkgName = "";
    // 7. call DeviceManagerNotify OnDeviceOffline with testpkcName
    DmDeviceInfo deviceInfo;
    // 8. check if callback OnDeviceOffline called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceOffline(testPkgName, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOffline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOffline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOffline
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceOffline with testpkcName
 *                  8. check if callback OnDeviceOffline called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOffline_004, testing::ext::TestSize.Level0)
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
    // 6. set testpkcName null
    std::string testPkgName = "";
    // 7. call DeviceManagerNotify OnDeviceOffline with testpkcName
    DmDeviceInfo deviceInfo;
    DeviceManagerNotify::GetInstance().OnDeviceOffline(testPkgName, deviceInfo);
    // 8. check if callback OnDeviceOffline called
    ASSERT_NE(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceOffline
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceOffline
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceOffline
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify UnRegisterDeviceStateCallback withtestPkgName
 *                  7. call DeviceManagerNotify OnDeviceOffline with testpkcName
 *                  8. check if callback OnDeviceOffline called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceOffline_005, testing::ext::TestSize.Level0)
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
    // 6. call DeviceManagerNotify UnRegisterDeviceStateCallback withtestPkgName
    std::string testPkgName = "";
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(testPkgName);
    // 7. call DeviceManagerNotify OnDeviceOffline with testpkcName
    DmDeviceInfo deviceInfo;
    // 8. check if callback OnDeviceOffline called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceOffline(testPkgName, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceChanged
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceChanged
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceChanged
 * EnvConditions: OnDeviceChanged success.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDeviceChanged
 *                  7. check if callback OnDeviceChanged called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceChanged_001, testing::ext::TestSize.Level0)
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
    // 6. call DeviceManagerNotify OnDeviceChanged
    DmDeviceInfo deviceInfo;
    DeviceManagerNotify::GetInstance().OnDeviceChanged(pkgName, deviceInfo);
    // 7. check if callback OnDeviceChanged called
    sleep(1);
    // 8. call OnDeviceChanged,count != 0
    ASSERT_NE(count, 0);
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceChanged
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceChanged
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceChanged
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceChanged with testpkcName
 *                  8. check if callback OnDeviceChanged called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceChanged_002, testing::ext::TestSize.Level0)
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
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "com.ohos.test1";
    // 7. call DeviceManagerNotify OnDeviceChanged with testpkcName
    DmDeviceInfo deviceInfo;
    // 8. check if callback OnDeviceChanged called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceChanged(testPkgName, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceChanged
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceChanged
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceChanged
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceChanged with testpkcName
 *                  8. check if callback OnDeviceChanged called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceChanged_003, testing::ext::TestSize.Level0)
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
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "";
    // 7. call DeviceManagerNotify OnDeviceChanged with testpkcName
    DmDeviceInfo deviceInfo;
    // 8. check if callback OnDeviceChanged called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceChanged(testPkgName, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceChanged
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceChanged
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceChanged
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceChanged with testpkcName
 *                  8. check if callback OnDeviceChanged called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceChanged_004, testing::ext::TestSize.Level0)
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
    // 6. set testpkcName com.ohos.test1
    std::string testPkgName = "";
    // 7. call DeviceManagerNotify OnDeviceChanged with testpkcName
    DmDeviceInfo deviceInfo;
    DeviceManagerNotify::GetInstance().OnDeviceChanged(testPkgName, deviceInfo);
    // 8. check if callback OnDeviceChanged called
    ASSERT_NE(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceChanged
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceChanged
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceChanged
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set Callback not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDeviceStateCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. set testpkcName com.ohos.test1
 *                  7. call DeviceManagerNotify OnDeviceChanged with testpkcName
 *                  8. check if callback OnDeviceChanged called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceChanged_005, testing::ext::TestSize.Level0)
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
    // 6. call DeviceManagerNotify UnRegisterDeviceStateCallback withtestPkgName
    std::string testPkgName = "";
    DeviceManagerNotify::GetInstance().UnRegisterDeviceStateCallback(testPkgName);
    // 7. call DeviceManagerNotify OnDeviceChanged with testpkcName
    DmDeviceInfo deviceInfo;
    // 8. check if callback OnDeviceChanged called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceChanged(testPkgName, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceFound
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceFound
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceFound
 * EnvConditions: OnDeviceFound success.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDeviceFound
 *                  7. check if callback OnDeviceFound called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceFound1, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    DmDeviceInfo deviceInfo;
    // 6. call DeviceManagerNotify OnDeviceFound
    DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, subscribeId, deviceInfo);
    // 7. check if callback OnDeviceFound called
    sleep(1);
    ASSERT_NE(count, 10);
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceFound
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceFound
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceFound
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDeviceFound With testPkgName
 *                  7. check if callback OnDeviceFound called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceFound2, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    DmDeviceInfo deviceInfo;
    std::string testPkgName = "com.ohos.test1";
    // 7. check if callback OnDeviceFound called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceFound(testPkgName, subscribeId, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceFound
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceFound
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceFound
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDeviceFound With testsubscribeId
 *                  7. check if callback OnDeviceFound called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceFound3, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    DmDeviceInfo deviceInfo;
    uint16_t testsubscribeId = 1;
    // 7. check if callback OnDeviceFound called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceFound(pkgName, testsubscribeId, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceFound
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceFound
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceFound
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDeviceFound With testPkgName
 *                  7. check if callback OnDeviceFound called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceFound4, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    DmDeviceInfo deviceInfo;
    std::string testPkgName = "";
    // 7. check if callback OnDeviceFound called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceFound(testPkgName, subscribeId, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDeviceFound
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDeviceFound
 * FunctionPoints: DeviceManagerNotifyTest OnDeviceFound
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDeviceFound With testPkgName
 *                  7. check if callback OnDeviceFound called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDeviceFound5, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    DmDeviceInfo deviceInfo;
    std::string testPkgName = "";
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(testPkgName, subscribeId);
    // 7. check if callback OnDeviceFound called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDeviceFound(testPkgName, subscribeId, deviceInfo));
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverFailed
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverFailed
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverFailed
 * EnvConditions: OnDiscoverFailed success.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverFailed
 *                  7. check if callback OnDiscoverFailed called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverFailed1, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    int32_t failedReason = 0;
    // 6. call DeviceManagerNotify OnDiscoverFailed
    DeviceManagerNotify::GetInstance().OnDiscoveryFailed(pkgName, subscribeId, failedReason);
    // 7. check if callback OnDiscoverFailed called
    ASSERT_NE(count, 10);
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverFailed
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverFailed
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverFailed
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverFailed With testPkgName
 *                  7. check if callback OnDiscoverFailed called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverFailed2, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    std::string testPkgName = "com.ohos.test1";
    int32_t failedReason = 0;
    // 7. check if callback OnDiscoverFailed called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDiscoveryFailed(testPkgName,
        subscribeId, failedReason));
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverFailed
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverFailed
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverFailed
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverFailed With testsubscribeId
 *                  7. check if callback OnDiscoverFailed called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverFailed3, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    int32_t failedReason = 0;
    uint16_t testsubscribeId = 1;
    // 7. check if callback OnDiscoverFailed called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDiscoveryFailed(pkgName,
        testsubscribeId, failedReason));
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverFailed
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverFailed
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverFailed
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverFailed With testPkgName
 *                  7. check if callback OnDiscoverFailed called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverFailed4, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    std::string testPkgName = "";
    int32_t failedReason = 0;
    // 7. check if callback OnDiscoverFailed called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDiscoveryFailed(testPkgName,
        subscribeId, failedReason));
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverFailed
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverFailed
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverFailed
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverFailed With testPkgName
 *                  7. check if callback OnDiscoverFailed called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverFailed5, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    std::string testPkgName = "";
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(testPkgName, subscribeId);
    int32_t failedReason = 0;
    // 7. check if callback OnDiscoverFailed called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDiscoveryFailed(testPkgName,
        subscribeId, failedReason));
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverySuccess
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverySuccess
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverySuccess
 * EnvConditions: OnDiscoverySuccess success.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverySuccess
 *                  7. check if callback OnDiscoverySuccess called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverySuccess1, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    // 6. call DeviceManagerNotify OnDiscoverySuccess
    DeviceManagerNotify::GetInstance().OnDiscoverySuccess(pkgName, subscribeId);
    // 7. check if callback OnDiscoverySuccess called
    ASSERT_NE(count, 10);
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverySuccess
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverySuccess
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverySuccess
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverySuccess With testPkgName
 *                  7. check if callback OnDiscoverySuccess called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverySuccess2, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    std::string testPkgName = "com.ohos.test1";
    // 7. check if callback OnDiscoverySuccess called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDiscoverySuccess(testPkgName, subscribeId));
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverySuccess
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverySuccess
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverySuccess
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverySuccess With testsubscribeId
 *                  7. check if callback OnDiscoverySuccess called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverySuccess3, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    uint16_t testsubscribeId = 1;
    // 7. check if callback OnDiscoverySuccess called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDiscoverySuccess(pkgName, testsubscribeId));
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverySuccess
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverySuccess
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverySuccess
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverySuccess With testPkgName
 *                  7. check if callback OnDiscoverySuccess called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverySuccess4, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    std::string testPkgName = "";
    // 7. check if callback OnDiscoverySuccess called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDiscoverySuccess(testPkgName, subscribeId));
}

/*
 * Feature: DeviceManagerNotifyTest OnDiscoverySuccess
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnDiscoverySuccess
 * FunctionPoints: DeviceManagerNotifyTest OnDiscoverySuccess
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set subscribeId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterDiscoverCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnDiscoverySuccess With testPkgName
 *                  7. check if callback OnDiscoverySuccess called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnDiscoverySuccess5, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
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
    std::string testPkgName = "";
    DeviceManagerNotify::GetInstance().UnRegisterDiscoveryCallback(testPkgName, subscribeId);
    // 7. check if callback OnDiscoverySuccess called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnDiscoverySuccess(testPkgName, subscribeId));
}

/*
 * Feature: DeviceManagerNotifyTest OnPublishResult
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnPublishResult Failed
 * FunctionPoints: DeviceManagerNotifyTest OnPublishResult
 * EnvConditions: OnPublishResult.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set publishId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnPublishResult
 *                  7. check if callback OnPublishResult called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnPublishResult1, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
    int count = 0;
    std::shared_ptr<PublishCallback> callback = std::make_shared<PublishCallbackTest>(count);
    // set publishId not null
    int32_t publishId = 0;
    // 2. set checkMap null
    std::shared_ptr<PublishCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    int32_t failedReason = 1;
    // 6. call DeviceManagerNotify OnPublishResult
    DeviceManagerNotify::GetInstance().OnPublishResult(pkgName, publishId, failedReason);
    // 7. check if callback OnPublishResult called
    ASSERT_EQ(count, 1);
}

/*
 * Feature: DeviceManagerNotifyTest OnPublishResult
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnPublishResult Failed
 * FunctionPoints: DeviceManagerNotifyTest OnPublishResult
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set publishId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnPublishResult With testPkgName
 *                  7. check if callback OnPublishResult called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnPublishResult2, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
    int count = 0;
    std::shared_ptr<PublishCallback> callback = std::make_shared<PublishCallbackTest>(count);
    // set publishId not null
    int32_t publishId = 0;
    // 2. set checkMap null
    std::shared_ptr<PublishCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    std::string testPkgName = "com.ohos.test1";
    int32_t failedReason = 1;
    // 7. check if callback OnPublishResult called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnPublishResult(testPkgName, publishId, failedReason));
}

/*
 * Feature: DeviceManagerNotifyTest OnPublishResult
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnPublishResult Failed
 * FunctionPoints: DeviceManagerNotifyTest OnPublishResult
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set publishId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnPublishFailed With testpublishId
 *                  7. check if callback OnPublishResult called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnPublishResult3, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
    int count = 0;
    std::shared_ptr<PublishCallback> callback = std::make_shared<PublishCallbackTest>(count);
    // set publishId not null
    int32_t publishId = 0;
    // 2. set checkMap null
    std::shared_ptr<PublishCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    int32_t failedReason = 1;
    int32_t testpublishId = 1;
    // 7. check if callback OnPublishResult called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnPublishResult(pkgName, testpublishId, failedReason));
}

/*
 * Feature: DeviceManagerNotifyTest OnPublishResult
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnPublishResult Failed
 * FunctionPoints: DeviceManagerNotifyTest OnPublishResult
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set publishId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnPublishResult With testPkgName
 *                  7. check if callback OnPublishResult called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnPublishFailed4, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
    int count = 0;
    std::shared_ptr<PublishCallback> callback = std::make_shared<PublishCallbackTest>(count);
    // set publishId not null
    int32_t publishId = 0;
    // 2. set checkMap null
    std::shared_ptr<PublishCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    std::string testPkgName = "";
    int32_t failedReason = 1;
    // 7. check if callback OnPublishResult called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnPublishResult(testPkgName, publishId, failedReason));
}

/*
 * Feature: DeviceManagerNotifyTest OnPublishResult
 * Function: DeviceManagerNotifyTest
 * SubFunction: OnPublishResult Failed
 * FunctionPoints: DeviceManagerNotifyTest OnPublishResult
 * EnvConditions: N/A.
 * CaseDescription: 1. set pkgName not null
 *                     set callback not null
 *                     set publishId not null
 *                  2. set checkMap null
 *                  3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
 *                  4. Get checkMap from DeviceManagerNotify
 *                  5. check checkMap not null
 *                  6. call DeviceManagerNotify OnPublishResult With testPkgName
 *                  7. check if callback OnPublishResult called.
 */
HWTEST_F(DeviceManagerNotifyTest, OnPublishFailed5, testing::ext::TestSize.Level0)
{
    // 1. set pkgName not null
    std::string pkgName = "com.ohos.test";
    // set callback not null
    int count = 0;
    std::shared_ptr<PublishCallback> callback = std::make_shared<PublishCallbackTest>(count);
    // set publishId not null
    int32_t publishId = 2;
    // 2. set checkMap null
    std::shared_ptr<PublishCallback> checkMap = nullptr;
    // 3. call DeviceManagerNotifyTest RegisterPublishCallback with parameter
    DeviceManagerNotify::GetInstance().RegisterPublishCallback(pkgName, publishId, callback);
    // 4. Get checkMap from DeviceManagerNotify
    checkMap = DeviceManagerNotify::GetInstance().devicePublishCallbacks_[pkgName][publishId];
    // 5. check checkMap not null
    ASSERT_NE(checkMap, nullptr);
    std::string testPkgName = "";
    DeviceManagerNotify::GetInstance().UnRegisterPublishCallback(testPkgName, publishId);
    int32_t failedReason = 1;
    // 7. check if callback OnPublishResult called
    ASSERT_NO_FATAL_FAILURE(DeviceManagerNotify::GetInstance().OnPublishResult(testPkgName, publishId, failedReason));
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
