/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "device_manager_linux_test.h"

#include <unistd.h>

#include "dm_constants.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceManagerImplTest::SetUp()
{
}

void DeviceManagerImplTest::TearDown()
{
}

void DeviceManagerImplTest::SetUpTestCase()
{
}

void DeviceManagerImplTest::TearDownTestCase()
{
}

HWTEST_F(DeviceManagerImplTest, InitDeviceManager001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::shared_ptr<DmInitCallbackTest> callback = std::make_shared<DmInitCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(packName, callback);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RegisterDevStateCallback001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string extra = "";
    std::shared_ptr<DmDeviceStateCallbackTest> callback = std::make_shared<DmDeviceStateCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().RegisterDevStateCallback(packName, extra, callback);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StartDeviceDiscovery001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string extra = "{\"findDeviceMode\":1}";
    uint64_t subscribeId = 1000;
    std::shared_ptr<DiscoveryCallbackTest> callback = std::make_shared<DiscoveryCallbackTest>();
    int32_t ret = DeviceManager::GetInstance().StartDeviceDiscovery(packName, subscribeId, extra, callback);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, StopDeviceDiscovery001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    uint64_t subscribeId = 1000;
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(subscribeId, packName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetTrustDeviceList001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string extra = "";
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(packName, extra, deviceList);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, RequestCredential001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string returnJsonStr;
    int32_t ret = DeviceManager::GetInstance().RequestCredential(packName, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, CheckCredential001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string reqJsonStr = R"({})";
    std::string returnJsonStr;
    int32_t ret = DeviceManager::GetInstance().CheckCredential(packName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, ImportCredential001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string reqJsonStr = R"(
    {
        "processType": 1,
        "authType": 1,
        "userId": "123",
        "credentialData":
        [
            {
                "credentialType": 1,
                "credentialId": "104",
                "authCode": "10F9F0576E61730193D2052B7F771887124A68F1607EFCF7796C1491F834CD92",
                "serverPk": "",
                "pkInfoSignature": "",
                "pkInfo": "",
                "peerDeviceId": ""
            }
        ]
    }
    )";
    std::string returnJsonStr;
    int32_t ret = DeviceManager::GetInstance().ImportCredential(packName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, DeleteCredential001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string reqJsonStr = R"({"isDeleteAll:true"})";
    std::string returnJsonStr;
    int32_t ret = DeviceManager::GetInstance().DeleteCredential(packName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetAvailableDeviceList001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::vector<DmDeviceBasicInfo> deviceList;
    int32_t ret = DeviceManager::GetInstance().GetAvailableDeviceList(packName, deviceList);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetLocalDeviceNetWorkId001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string networkId;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceNetWorkId(packName, networkId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetLocalDeviceId001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string deviceId;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceId(packName, deviceId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetLocalDeviceName001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string deviceName;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceName(packName, deviceName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetLocalDeviceType001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    int32_t deviceType;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceType(packName, deviceType);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceName001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    std::string deviceName;
    std::string networkId = "109008080809";
    int32_t ret = DeviceManager::GetInstance().GetDeviceName(packName, networkId, deviceName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, GetDeviceType001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    int32_t deviceType;
    std::string networkId = "109008080809";
    int32_t ret = DeviceManager::GetInstance().GetDeviceType(packName, networkId, deviceType);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnRegisterDevStateCallback001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    int32_t ret = DeviceManager::GetInstance().UnRegisterDevStatusCallback(packName);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceManagerImplTest, UnInitDeviceManager001, testing::ext::TestSize.Level0)
{
    std::string packName = "com.ohos.helloworld";
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(packName);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
