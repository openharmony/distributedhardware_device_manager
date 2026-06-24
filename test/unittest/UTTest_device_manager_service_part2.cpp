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
HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> extraParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
HWTEST_F(DeviceManagerServiceTest, DisableDiscoveryListener_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> extraParam;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DisableDiscoveryListener(pkgName, extraParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
HWTEST_F(DeviceManagerServiceTest, StartAdvertising_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, StartAdvertising_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, StartAdvertising_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::map<std::string, std::string> advertiseParam;
    advertiseParam[PARAM_KEY_META_TYPE] = "metaType";
    advertiseParam[PARAM_KEY_PUBLISH_ID] = "123456";
    advertiseParam[PARAM_KEY_DISC_MODE] =
        std::to_string(static_cast<int32_t>(DmDiscoverMode::DM_DISCOVER_MODE_PASSIVE));
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().StartAdvertising(pkgName, advertiseParam);
    EXPECT_NE(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
HWTEST_F(DeviceManagerServiceTest, StopAdvertising_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, StopAdvertising_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> advertiseParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, StopAdvertising_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> advertiseParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
HWTEST_F(DeviceManagerServiceTest, StopAdvertising_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "StopAdvertising_004";
    std::map<std::string, std::string> advertiseParam;
    DeviceManagerService::GetInstance().InitDMServiceListener();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_TRUE(ret == SOFTBUS_DISCOVER_COAP_STOP_PUBLISH_FAIL || ret == SOFTBUS_ERR ||
                ret == ERR_DM_INPUT_PARA_INVALID);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
HWTEST_F(DeviceManagerServiceTest, StopAdvertising_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "StopAdvertising_005";
    std::map<std::string, std::string> advertiseParam;
    std::string key = PARAM_KEY_META_TYPE;
    std::string value = "125";
    advertiseParam.insert(std::make_pair(key, value));
    key = PARAM_KEY_PUBLISH_ID;
    value = "128";
    advertiseParam.insert(std::make_pair(key, value));
    DeviceManagerService::GetInstance().InitDMServiceListener();
    int32_t ret = DeviceManagerService::GetInstance().StopAdvertising(pkgName, advertiseParam);
    EXPECT_NE(ret, DM_OK);
    DeviceManagerService::GetInstance().UninitDMServiceListener();
}
HWTEST_F(DeviceManagerServiceTest, BindTarget_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, BindTarget_005, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, BindTarget_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> bindParam;
    targetId.wifiIp = "";
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().dmServiceImpl_ = nullptr;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().BindTarget(pkgName, targetId, bindParam);
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, UnbindTarget_004, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, UnbindTarget_005, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, UnbindTarget_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    std::map<std::string, std::string> unbindParam;
    int32_t ret = DeviceManagerService::GetInstance().UnbindTarget(pkgName, targetId, unbindParam);
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string pkgName;
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, DpAclAdd_001, testing::ext::TestSize.Level1)
{
    DeletePermission();
    std::string udid;
    int64_t accessControlId = 0;
    int32_t ret = DeviceManagerService::GetInstance().DpAclAdd(udid, accessControlId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, DpAclAdd_002, testing::ext::TestSize.Level1)
{
    std::string udid = "udid";
    int64_t accessControlId = 0;
    int32_t ret = DeviceManagerService::GetInstance().DpAclAdd(udid, accessControlId);
    EXPECT_EQ(ret, ERR_DM_UNSUPPORTED_METHOD);
}
HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId = "12345";
    int32_t securityLevel = -1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId = "12345";
    int32_t securityLevel = -1;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId;
    int32_t securityLevel = 0;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, IsSameAccount_001, testing::ext::TestSize.Level1)
{
    std::string udid = "";
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(ERR_DM_INPUT_PARA_INVALID));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, IsSameAccount_002, testing::ext::TestSize.Level1)
{
    std::string udid = "udidTest";
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(ERR_DM_INPUT_PARA_INVALID));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_NE(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, IsSameAccount_003, testing::ext::TestSize.Level1)
{
    std::string udid = "udidTest";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, CheckIsSameAccount_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    DeletePermission();
    bool ret = DeviceManagerService::GetInstance().CheckIsSameAccount(caller, callee);
    EXPECT_FALSE(ret);
}
HWTEST_F(DeviceManagerServiceTest, CheckAccessControl_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    DmAccessCallee callee;
    DeletePermission();
    bool ret = DeviceManagerService::GetInstance().CheckAccessControl(caller, callee);
    EXPECT_FALSE(ret);
}
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::map<std::string, std::string> policy;
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().SetDnPolicy(pkgName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "openharmony123";
    std::map<std::string, std::string> policy;
    DeletePermission();
    int ret = DeviceManagerService::GetInstance().SetDnPolicy(pkgName, policy);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, SetDnPolicy_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "openharmony123";
    std::map<std::string, std::string> policy;
    std::vector<std::string> peerUdids;
    int32_t userId = 1;
    uint64_t tokenId = 87;
    DeviceManagerService::GetInstance().SendUnBindBroadCast(peerUdids, userId, tokenId, USER);
    DeviceManagerService::GetInstance().SendUnBindBroadCast(peerUdids, userId, tokenId, APP);
    DeviceManagerService::GetInstance().SendUnBindBroadCast(peerUdids, userId, tokenId, 2);
    DeviceManagerService::GetInstance().SendDeviceUnBindBroadCast(peerUdids, userId, tokenId);
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    DeviceManagerService::GetInstance().SendDeviceUnBindBroadCast(peerUdids, userId, tokenId);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    std::string processName = "collaboration_service";
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_))
        .WillOnce(DoAll(SetArgReferee<0>(processName), Return(DM_OK)));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnSetDnPolicy(_)).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().SetDnPolicy(pkgName, policy);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(false));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string networkId;
    int32_t screenStatus = 1;
    DeletePermission();
    int ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgname";
    std::string networkId = "networkId_003";
    int32_t screenStatus = 1;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetDeviceScreenStatus(_, _)).WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    EXPECT_EQ(ret, ERR_DM_FAILED);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
}
HWTEST_F(DeviceManagerServiceTest, GetDeviceScreenStatus_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgname";
    std::string networkId = "networkId_003";
    int32_t screenStatus = 1;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int ret = DeviceManagerService::GetInstance().GetDeviceScreenStatus(pkgName, networkId, screenStatus);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
HWTEST_F(DeviceManagerServiceTest, HandleDeviceScreenStatusChange_001, testing::ext::TestSize.Level1)
{
    DmDeviceInfo deviceInfo;
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().IsDMServiceImplReady();
    EXPECT_CALL(*softbusCacheMock_, GetUdidFromCache(_, _)).WillOnce(Return(DM_OK));
    DeviceManagerService::GetInstance().HandleDeviceScreenStatusChange(deviceInfo);
    EXPECT_NE(DeviceManagerService::GetInstance().dmServiceImpl_, nullptr);
}
HWTEST_F(DeviceManagerServiceTest, EnableDiscoveryListener_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::map<std::string, std::string> discoverParam;
    std::map<std::string, std::string> filterOptions;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().EnableDiscoveryListener(pkgName, discoverParam, filterOptions);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
HWTEST_F(DeviceManagerServiceTest, GetLocalDeviceInfo_002, testing::ext::TestSize.Level1)
{
    DmDeviceInfo info;
    DeletePermission();
    std::string msg = "";
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    bool result = DeviceManagerService::GetInstance().IsDMServiceImplReady();
    EXPECT_TRUE(result);
    msg =  R"(
    {
        "type" : 0,
        "userId" : 123,
        "accountId" : "28776",
        "tokenId" : 16,
        "peerUdid" : "104",
        "accountName" : "account_xiao"
    })";
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
    msg =  R"(
    {
        "type" : 1,
        "userId" : 128,
        "accountId" : "28778",
        "tokenId" : 17,
        "peerUdid" : "108",
        "accountName" : "account_li"
    })";
    DeviceManagerService::GetInstance().HandleDeviceTrustedChange(msg);
    msg =  R"(
    {
        "type" : 2,
        "userId" : 124,
        "accountId" : "28779",
        "tokenId" : 18,
        "peerUdid" : "110",
        "accountName" : "account_wang"
    })";
    int32_t ret = DeviceManagerService::GetInstance().GetLocalDeviceInfo(info);
    EXPECT_EQ(ret, ERR_DM_POINT_NULL);
}
HWTEST_F(DeviceManagerServiceTest, GetNetworkIdByUdid_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string udid;
    std::string networkId = "networkIdTest_001";
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, GetNetworkIdByUdid_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string udid;
    std::string networkId = "networkIdTest_002";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, GetNetworkIdByUdid_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName_003";
    std::string udid = "sewdwed98897";
    std::string networkId = "networkIdTest_003";
    EXPECT_CALL(*softbusCacheMock_, GetNetworkIdFromCache(_, _))
        .WillOnce(Return(ERR_DM_FAILED));
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().GetNetworkIdByUdid(pkgName, udid, networkId);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
HWTEST_F(DeviceManagerServiceTest, StopAuthenticateDevice_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    DeletePermission();
    int32_t ret = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    EXPECT_EQ(ret, ERR_DM_NO_PERMISSION);
}
HWTEST_F(DeviceManagerServiceTest, StopAuthenticateDevice_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, StopAuthenticateDevice_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName_003";
    DeviceManagerService::GetInstance().isImplsoLoaded_ = false;
    DeviceManagerService::GetInstance().IsDMServiceImplReady();
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*deviceManagerServiceImplMock_, StopAuthenticateDevice(_)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().StopAuthenticateDevice(pkgName);
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, DestroyPinHolder_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName_005";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    if (DeviceManagerService::GetInstance().pinHolder_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    int32_t ret = DeviceManagerService::GetInstance().DestroyPinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, CreatePinHolder_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    PeerTargetId targetId;
    DmPinType pinType = DmPinType::QR_CODE;
    std::string payload;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    if (DeviceManagerService::GetInstance().pinHolder_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    int32_t ret = DeviceManagerService::GetInstance().CreatePinHolder(pkgName, targetId, pinType, payload);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_003, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, RegisterPinHolderCallback_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnPinHolder(_)).WillOnce(Return(true));
    if (DeviceManagerService::GetInstance().pinHolder_ == nullptr) {
        DeviceManagerService::GetInstance().InitDMServiceListener();
    }
    int32_t ret = DeviceManagerService::GetInstance().RegisterPinHolderCallback(pkgName);
    EXPECT_EQ(ret, DM_OK);
}
/**
 * @tc.name: ShiftLNNGear_008
 * @tc.desc:Set the callerId and pkgNamenot not to null, and isRefresh to true; Return NOT DM_OK
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(DeviceManagerServiceTest, ShiftLNNGear_008, testing::ext::TestSize.Level1)
{
    std::string pkgName = "ShiftLNNGear_008";
    std::string callerId = "ShiftLNNGear_008";
    bool isRefresh = true;
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, ShiftLNNGear(_, _)).WillOnce(Return(ERR_DM_FAILED));
    int ret = DeviceManagerService::GetInstance().ShiftLNNGear(pkgName, callerId, isRefresh, false);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, ERR_DM_FAILED);
}
HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string authCode;
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, ImportAuthCode_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ImportAuthCode(pkgName, authCode);
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, ExportAuthCode_003, testing::ext::TestSize.Level1)
{
    std::string authCode = "authCode";
    EXPECT_CALL(*permissionManagerMock_, CheckDataSyncPermission()).WillOnce(Return(true));
    EXPECT_CALL(*permissionManagerMock_, GetCallerProcessName(_)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*permissionManagerMock_, CheckProcessNameValidOnAuthCode(_)).WillOnce(Return(true));
    int32_t ret = DeviceManagerService::GetInstance().ExportAuthCode(authCode);
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, IsSameAccount_004, testing::ext::TestSize.Level1)
{
    std::string udid = "udidTest";
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    EXPECT_CALL(*softbusListenerMock_, GetUdidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_INPUT_PARA_INVALID);
}
HWTEST_F(DeviceManagerServiceTest, GetDeviceSecurityLevel_006, testing::ext::TestSize.Level1)
{
    std::string pkgName = "com.ohos.test";
    std::string invalidNetworkId = "networkId";
    int32_t securityLevel = 0;
    EXPECT_CALL(*permissionManagerMock_, CheckAccessServicePermission()).WillOnce(Return(true));
    DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    EXPECT_CALL(*softbusListenerMock_, GetDeviceSecurityLevel(_, _)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceManagerService::GetInstance().GetDeviceSecurityLevel(pkgName, invalidNetworkId, securityLevel);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, GetEncryptedUuidByNetworkId_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string networkId = "network_id";
    std::string uuid = "13345689";
    if (DeviceManagerService::GetInstance().softbusListener_ == nullptr) {
        DeviceManagerService::GetInstance().softbusListener_ = std::make_shared<SoftbusListener>();
    }
    EXPECT_CALL(*softbusListenerMock_, GetUuidByNetworkId(_, _)).WillOnce(Return(DM_OK));
    EXPECT_CALL(*appManagerMock_, GetAppId()).WillOnce(Return("appId"));
    int32_t ret = DeviceManagerService::GetInstance().GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
    DeviceManagerService::GetInstance().softbusListener_ = nullptr;
    EXPECT_EQ(ret, DM_OK);
}
HWTEST_F(DeviceManagerServiceTest, OpenAuthSessionWithPara_001, testing::ext::TestSize.Level1)
{
    const std::string deviceId = "OpenAuthSessionWithPara";
    int32_t actionId = 0;
    bool isEnable160m = false;
    int32_t ret = DeviceManagerService::GetInstance().OpenAuthSessionWithPara(deviceId, actionId, isEnable160m);
    EXPECT_NE(ret, DM_OK);
}

} // namespace DistributedHardware
} // namespace OHOS
