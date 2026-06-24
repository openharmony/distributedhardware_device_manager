/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "UTTest_device_manager_service.h"
#include "accesstoken_kit.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"
#include "softbus_error_code.h"

using namespace OHOS::Security::AccessToken;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
HWTEST_F(DeviceManagerServiceTest, BindDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t authType = 1;
    std::string deviceId;
    std::string bindParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, BindDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId;
    std::string bindParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, BindDevice_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*softbusListenerMock_,
        GetTargetInfoFromCache(_, _, _)).WillOnce(Return(ERR_DM_BIND_INPUT_PARA_INVALID));
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_BIND_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, BindDevice_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    int32_t authType = 1;
    std::string deviceId = "1234";
    std::string bindParam;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().BindDevice(pkgName, authType, deviceId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_NE(ret, DM_OK);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_003, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string deviceId = "1234";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    std::string extra = "extra";
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*kVAdapterManagerMock_, Get(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*softbusListenerMock_, GetUdidFromDp(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId;
    std::string extra = "extra";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_007, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string deviceId = "1234";
    std::string extra = "extra";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, UnBindDevice_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string deviceId = "1234";
    std::string extra = "extra";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().UnBindDevice(pkgName, deviceId, extra);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, OnSessionOpened_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    int sessionId = 0;
    int result = 0;
    void *data = nullptr;
    unsigned int dataLen = 0;
    int ret = DeviceManagerService::GetInstance().OnSessionOpened(sessionId, result);
    DeviceManagerService::GetInstance().OnBytesReceived(sessionId, data, dataLen);
    DeviceManagerService::GetInstance().OnSessionClosed(sessionId);
    EXPECT_NE(ret, ERR_DM_NOT_INIT);
}
HWTEST_F(DeviceManagerServiceTest, MineRequestCredential_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().MineRequestCredential(pkgName, returnJsonStr);
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, MineRequestCredential_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string returnJsonStr;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().MineRequestCredential(pkgName, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, CheckCredential_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, CheckCredential_002, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().CheckCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, ImportCredential_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, ImportCredential_005, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().ImportCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, DeleteCredential_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, DeleteCredential_005, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    std::string pkgName;
    std::string returnJsonStr;
    std::string reqJsonStr;
    int32_t ret = DeviceManagerService::GetInstance().DeleteCredential(pkgName, reqJsonStr, returnJsonStr);
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*appManagerMock_, IsSystemApp()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    DeletePermission();
    EXPECT_CALL(*appManagerMock_, IsSystemApp()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, RegisterUiStateCallback_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*appManagerMock_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().RegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NOT_SYSTEM_APP);
}
HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*appManagerMock_, IsSystemApp()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    DeletePermission();
    EXPECT_CALL(*appManagerMock_, IsSystemApp()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, UnRegisterUiStateCallback_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*appManagerMock_, IsSystemApp()).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().UnRegisterUiStateCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NOT_SYSTEM_APP);
}
HWTEST_F(DeviceManagerServiceTest, NotifyEvent_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t eventId = 0;
    std::string event;
    std::string msg = "";
    DeviceManagerService::GetInstance().HandleDeviceNotTrust(msg);
    msg =  R"({"authType" : 1, "userId" : "123", "credentialData" : "cryptosupportData",
        "CRYPTOSUPPORT" : "cryptosupportTest", "credentialType" : 1, "credentialId" : "104",
        "NETWORK_ID" : "108", "authCode" : "1234567812345678123456781234567812345678123456781234567812345678",
        "serverPk" : "hello", "pkInfoSignature" : "world", "pkInfo" : "pkginfo", "peerDeviceId" : "3515656546"})";
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(Return(DM_OK));
    DeviceManagerService::GetInstance().HandleDeviceNotTrust(msg);
    std::string commonEventType = "helloworld";
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().ScreenCommonEventCallback(commonEventType);
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, NotifyEvent_002, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    int32_t eventId = 0;
    std::string event;
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, NotifyEvent_003, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    int32_t eventId = DM_NOTIFY_EVENT_ON_PINHOLDER_EVENT;
    std::string event;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().NotifyEvent(pkgName, eventId, event);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string networkId;
    std::string uuid;
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string networkId;
    std::string uuid;
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string networkId = "network_id";
    std::string uuid = "13345689";
    if (DeviceManagerService::GetInstance().softbusListener_ == nullptr) {
        DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    }
    EXPECT_CALL(*softbusListenerMock_, GetUuidByNetworkId(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_002, testing::ext::TestSize.Level1)
{
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(0);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_003, testing::ext::TestSize.Level1)
{
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(1);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(2);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_005, testing::ext::TestSize.Level1)
{
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(1);
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_006, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(2);
    EXPECT_NE(ret, ERR_DM_FAILED);
}
HWTEST_F(DeviceManagerServiceTest, CheckApiPermission_007, testing::ext::TestSize.Level1)
{
    int32_t ret = DeviceManagerService::GetInstance().CheckApiPermission(3);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::string netWorkId;
    int32_t networkType = 0;
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string netWorkId = "netWorkId";
    int32_t networkType = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
HWTEST_F(DeviceManagerServiceTest, GetNetworkTypeByNetworkId_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string netWorkId = "netWorkId";
    int32_t networkType = 0;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetNetworkTypeByNetworkId(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkTypeByNetworkId(pkgName, netWorkId, networkType);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::string authCode;
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, ExportAuthCode_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string authCode;
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, ExportAuthCode_002, testing::ext::TestSize.Level1)
{
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(false));
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, UnloadDMServiceImplSo_001, testing::ext::TestSize.Level1)
{
    DeviceManagerService::GetInstance().UnloadDMServiceImplSo();
    EXPECT_EQ(DeviceManagerService::GetInstance().softbusListener_, nullptr);
}
HWTEST_F(DeviceManagerServiceTest, StartDiscovering_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, StartDiscovering_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, StartDiscovering_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_TRUE(ret == SOFTBUS_IPC_ERR || ret == DM_OK || ret == SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL);
    ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
HWTEST_F(DeviceManagerServiceTest, StartDiscovering_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    discoverParam[PARAM_KEY_META_TYPE] = "metaType";
    discoverParam[PARAM_KEY_SUBSCRIBE_ID] = "123456";
    discoverParam[PARAM_KEY_DISC_MEDIUM] =
        std::to_string(static_cast<int32_t>(DmExchangeMedium::DM_AUTO));
    discoverParam[PARAM_KEY_DISC_FREQ] =
        std::to_string(static_cast<int32_t>(DmExchangeFreq::DM_LOW));
    std::map<std::string, std::string> filterOptions;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillRepeatedly(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartDiscovering(pkgName, discoverParam, filterOptions);
    EXPECT_TRUE(ret == SOFTBUS_IPC_ERR || ret == DM_OK || ret == SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL);
    DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
HWTEST_F(DeviceManagerServiceTest, StopDiscovering_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, StopDiscovering_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, StopDiscovering_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopDiscovering(pkgName, discoverParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> extraParam;
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}

} // namespace DistributedHardware
} // namespace OHOS
