/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_HICHAIN_CONNECTOR_H
#define OHOS_HICHAIN_CONNECTOR_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "device_auth.h"
#include "device_auth_defines.h"
#include "dm_device_info.h"
#include "hichain_connector_callback.h"
#include "json_object.h"
#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
struct GroupInfo {
    std::string groupName;
    std::string groupId;
    std::string groupOwner;
    int32_t groupType;
    int32_t groupVisibility;
    std::string userId;

    GroupInfo() : groupName(""), groupId(""), groupOwner(""), groupType(0), groupVisibility(0), userId("")
    {
    }
};

enum class AuthFormPriority {
    PRIORITY_PEER_TO_PEER = 0,
    PRIORITY_ACROSS_ACCOUNT = 1,
    PRIORITY_IDENTICAL_ACCOUNT = 2,
};

void FromJson(const JsonItemObject &jsonObject, GroupInfo &groupInfo);

class HiChainConnector {
public:
    static bool onTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen);
    static void onFinish(int64_t requestId, int operationCode, const char *returnData);
    static void onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn);
    static char *onRequest(int64_t requestId, int operationCode, const char *reqParams);

public:
    HiChainConnector();
    ~HiChainConnector();

    /**
     * @tc.name: HiChainConnector::RegisterHiChainCallback
     * @tc.desc: Register HiChain Callback of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t RegisterHiChainCallback(std::shared_ptr<IHiChainConnectorCallback> callback);

    /**
     * @tc.name: HiChainConnector::UnRegisterHiChainCallback
     * @tc.desc: Un Register HiChain Callback of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t UnRegisterHiChainCallback();

    /**
     * @tc.name: HiChainConnector::CreateGroup
     * @tc.desc: Create Group of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t CreateGroup(int64_t requestId, const std::string &groupName);

    /**
     * @tc.name: HiChainConnector::CreateGroup
     * @tc.desc: Create Group of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t CreateGroup(int64_t requestId, int32_t authType, const std::string &userId,
        JsonObject &jsonOutObj);

    /**
     * @tc.name: HiChainConnector::AddMember
     * @tc.desc: Add Member of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t AddMember(const std::string &deviceId, const std::string &connectInfo);

    /**
     * @tc.name: HiChainConnector::DelMemberFromGroup
     * @tc.desc: Delete Member From Group of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t DelMemberFromGroup(const std::string &groupId, const std::string &deviceId);

    /**
     * @tc.name: HiChainConnector::DeleteGroup
     * @tc.desc: Delete Group of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t DeleteGroup(std::string &groupId);

    /**
     * @tc.name: HiChainConnector::DeleteGroup
     * @tc.desc: DeleteGroup of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t DeleteGroup(const int32_t userId, std::string &groupId);

    /**
     * @tc.name: HiChainConnector::DeleteGroup
     * @tc.desc: DeleteGroup of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t DeleteGroup(int64_t requestId_, const std::string &userId, const int32_t authType);

    /**
     * @tc.name: HiChainConnector::IsDevicesInP2PGroup
     * @tc.desc: IsDevicesInP2PGroup of the HiChain Connector
     * @tc.type: FUNC
     */
    bool IsDevicesInP2PGroup(const std::string &hostDevice, const std::string &peerDevice);

    /**
     * @tc.name: HiChainConnector::GetRelatedGroups
     * @tc.desc: Get Related Groups of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t GetRelatedGroups(const std::string &deviceId, std::vector<GroupInfo> &groupList);

    /**
     * @tc.name: HiChainConnector::GetGroupInfo
     * @tc.desc: Get GroupInfo of the HiChain Connector
     * @tc.type: FUNC
     */
    bool GetGroupInfo(const std::string &queryParams, std::vector<GroupInfo> &groupList);

    /**
     * @tc.name: HiChainConnector::GetGroupInfo
     * @tc.desc: Get GroupInfo of the HiChain Connector
     * @tc.type: FUNC
     */
    bool GetGroupInfo(const int32_t userId, const std::string &queryParams, std::vector<GroupInfo> &groupList);
    
    bool GetGroupInfoExt(const int32_t userId, const std::string &queryParams, std::vector<GroupInfo> &groupList);

    bool GetGroupInfoCommon(const int32_t userId, const std::string &queryParams, const char* pkgName,
        std::vector<GroupInfo> &groupList);
    /**
     * @tc.name: HiChainConnector::GetGroupType
     * @tc.desc: Get GroupType of the HiChain Connector
     * @tc.type: FUNC
     */
    DmAuthForm GetGroupType(const std::string &deviceId);

    /**
     * @tc.name: HiChainConnector::DeleteTimeOutGroup
     * @tc.desc: Delete TimeOut Group of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t DeleteTimeOutGroup(const std::string &peerUdid, int32_t userId);

    /**
     * @tc.name: HiChainConnector::RegisterHiChainCallback
     * @tc.desc: Register HiChain Callback of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t RegisterHiChainGroupCallback(const std::shared_ptr<IDmGroupResCallback> &callback);

    /**
     * @tc.name: HiChainConnector::UnRegisterHiChainCallback
     * @tc.desc: Un Register HiChain Callback of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t UnRegisterHiChainGroupCallback();

    /**
     * @tc.name: HiChainConnector::getRegisterInfo
     * @tc.desc: Get RegisterInfo Info of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t getRegisterInfo(const std::string &queryParams, std::string &returnJsonStr);

    /**
     * @tc.name: HiChainConnector::addMultiMembers
     * @tc.desc: Get RegisterInfo Info of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t addMultiMembers(const int32_t groupType, const std::string &userId,
                            const JsonObject &jsonDeviceList);
    /**
     * @tc.name: HiChainConnector::deleteMultiMembers
     * @tc.desc: Get RegisterInfo Info of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t deleteMultiMembers(const int32_t groupType, const std::string &userId,
                            const JsonObject &jsonDeviceList);

    /**
     * @tc.name: HiChainConnector::GetTrustedDevices
     * @tc.desc: Get TrustDevicesUdid Info of the HiChain Connector
     * @tc.type: FUNC
     */
    std::vector<std::string> GetTrustedDevices(const std::string &localDeviceUdid);

    int32_t addMultiMembersExt(const std::string &credentialInfo);

    void DeleteAllGroup(int32_t userId);
    void DeleteAllGroup(int32_t userId, const std::string &remoteUdid);
    int32_t GetRelatedGroups(int32_t userId, const std::string &deviceId, std::vector<GroupInfo> &groupList);
    int32_t GetRelatedGroupsExt(const std::string &deviceId, std::vector<GroupInfo> &groupList);
    int32_t GetRelatedGroupsExt(int32_t userId, const std::string &deviceId, std::vector<GroupInfo> &groupList);

    int32_t DeleteGroupExt(const std::string &groupId);
    int32_t DeleteGroupExt(int32_t userId, const std::string &groupId);

    int32_t GetRelatedGroupsCommon(const std::string &deviceId, const char* pkgName,
        std::vector<GroupInfo> &groupList);
    int32_t GetRelatedGroupsCommon(int32_t userId, const std::string &deviceId, const char* pkgName,
        std::vector<GroupInfo> &groupList);
    void DeleteAllGroupByUdid(const std::string &udid);
    int32_t DeleteGroupByACL(std::vector<std::pair<int32_t, std::string>> &delACLInfoVec,
        std::vector<int32_t> &userIdVec);
    bool IsNeedDelete(std::string &groupName, int32_t userId,
        std::vector<std::pair<int32_t, std::string>> &delACLInfoVec);
    void DeleteHoDevice(const std::string &peerUdid, const std::vector<int32_t> &foreGroundUserIds,
        const std::vector<int32_t> &backGroundUserIds);
private:
    int64_t GenRequestId();
    int32_t SyncGroups(std::string deviceId, std::vector<std::string> &remoteGroupIdList);
    int32_t GetSyncGroupList(std::vector<GroupInfo> &groupList, std::vector<std::string> &syncGroupList);
    std::string GetConnectPara(std::string deviceId, std::string reqDeviceId);
    bool IsGroupCreated(std::string groupName, GroupInfo &groupInfo);
    bool IsRedundanceGroup(const std::string &userId, int32_t authType, std::vector<GroupInfo> &groupList);
    void DealRedundanceGroup(const std::string &userId, int32_t authType);
    void DeleteRedundanceGroup(std::string &userId);
    bool IsGroupInfoInvalid(GroupInfo &group);
    int32_t GetStrFieldByType(const std::string &reqJsonStr, const std::string &outField, int32_t type);
    int32_t GetNumsFieldByType(const std::string &reqJsonStr, int32_t &outField, int32_t type);
    int32_t GetGroupId(const std::string &userId, const int32_t groupType, std::string &groupId);
    int32_t ParseRemoteCredential(const int32_t groupType, const std::string &userId,
    const JsonObject &jsonDeviceList, std::string &params, int32_t &osAccountUserId);
    int32_t GetTrustedDevicesUdid(const char* jsonStr, std::vector<std::string> &udidList);
    int32_t GetGroupIdExt(const std::string &userId, const int32_t groupType,
        std::string &groupId, std::string &groupOwner);
    int32_t ParseRemoteCredentialExt(const std::string &credentialInfo, std::string &params, std::string &groupOwner);
    int32_t GetJsonInt(const JsonObject &jsonObj, const std::string &key);
    std::string GetJsonStr(const JsonObject &jsonObj, const std::string &key);

private:
    const DeviceGroupManager *deviceGroupManager_ = nullptr;
    DeviceAuthCallback deviceAuthCallback_;
    static std::shared_ptr<IHiChainConnectorCallback> hiChainConnectorCallback_;
    static std::shared_ptr<IDmGroupResCallback> hiChainResCallback_;
    static int32_t networkStyle_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HICHAIN_CONNECTOR_H
