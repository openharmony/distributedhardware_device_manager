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

#include "UTTest_dm_library_manager_test.h"

#include <thread>

#include "deviceprofile_connector.h"
#include "dm_library_manager.h"
#include "dm_log.h"
#include "dm_radar_helper.h"
#include "idevice_manager_service_impl.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t WAITFOR_RESOURCE_RELEASE_SECONDS = 65;
}
void DMLibraryManagerTest::SetUp()
{
}

void DMLibraryManagerTest::TearDown()
{
}

void DMLibraryManagerTest::SetUpTestCase()
{
}

void DMLibraryManagerTest::TearDownTestCase()
{
    std::this_thread::sleep_for(std::chrono::seconds(WAITFOR_RESOURCE_RELEASE_SECONDS));
}

namespace {
HWTEST_F(DMLibraryManagerTest, LoadImplSo_001, testing::ext::TestSize.Level0)
{
    std::string soName = "libdevicemanagerserviceimpl.z.so";
    std::string funName = "CreateDMServiceObject";

    auto& libMgr = GetLibraryManager();
    auto createDMSvrObj = libMgr.GetFunction<IDeviceManagerServiceImpl*(*)()>(soName, funName);
    EXPECT_NE(createDMSvrObj, nullptr);
    IDeviceManagerServiceImpl* dmSvrImplPtr = createDMSvrObj();
    EXPECT_NE(dmSvrImplPtr, nullptr);
    libMgr.Release(soName);
}

HWTEST_F(DMLibraryManagerTest, LoadImplSo_002, testing::ext::TestSize.Level0)
{
    std::string soName1 = "libdevicemanagerserviceimpl.z.so";
    std::string funName1 = "CreateDMServiceObject";

    std::string soName2 = "libdevicemanagerradar.z.so";
    std::string funName2 = "CreateDmRadarInstance";

    std::string soName3 = "libdevicemanagerdependency.z.so";
    std::string funName3 = "CreateDpConnectorInstance";

    std::vector<std::thread> threads;
    for (int i = 0; i < 100; ++i) {
        threads.emplace_back([=]() {
            auto& libMgr = GetLibraryManager();
            std::string libPath = "";
            std::string funName = "";

            if (i % 3 == 0) {
                libPath = soName1;
                funName = funName1;
            } else if (i % 3 == 1) {
                libPath = soName2;
                funName = funName2;
            } else {
                libPath = soName3;
                funName = funName3;
            }

            if (i % 3 == 0) {
                auto createDMSvrObj = libMgr.GetFunction<IDeviceManagerServiceImpl*(*)()>(libPath, funName);
                EXPECT_NE(createDMSvrObj, nullptr);
                IDeviceManagerServiceImpl* dmSvrImplPtr = createDMSvrObj();
                EXPECT_NE(dmSvrImplPtr, nullptr);
                libMgr.Release(libPath);
            } else if (i % 3 == 1) {
                auto createRadarHelper = libMgr.GetFunction<IDmRadarHelper *(*)()>(libPath, funName);
                EXPECT_NE(createRadarHelper, nullptr);
                IDmRadarHelper* radarHelperPtr = createRadarHelper();
                EXPECT_NE(radarHelperPtr, nullptr);
                libMgr.Release(libPath);
            } else {
                auto createDpConn = libMgr.GetFunction<IDeviceProfileConnector *(*)()>(libPath, funName);
                EXPECT_NE(createDpConn, nullptr);
                IDeviceProfileConnector* dpConnectorPtr = createDpConn();
                EXPECT_NE(dpConnectorPtr, nullptr);
                libMgr.Release(libPath);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    for (auto& t : threads) {
        t.join();
    }
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS