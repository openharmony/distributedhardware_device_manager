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
#include "dm_constants.h"
#include "dm_device_info.h"
#include "hichain_connector_callback.h"
#include "nlohmann/json.hpp"
#include "single_instance.h"
#include "system_ability_status_change_stub.h"

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

static const std::unordered_map<int32_t, AuthFormPriority> g_authFormPriorityMap = {
    {GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP, AuthFormPriority::PRIORITY_IDENTICAL_ACCOUNT},
    {GROUP_TYPE_ACROSS_ACCOUNT_GROUP, AuthFormPriority::PRIORITY_ACROSS_ACCOUNT},
    {GROUP_TYPE_PEER_TO_PEER_GROUP, AuthFormPriority::PRIORITY_PEER_TO_PEER}
};

void from_json(const nlohmann::json &jsonObject, GroupInfo &groupInfo);

class HiChainConnector : public std::enable_shared_from_this<HiChainConnector> {
public:
    static bool onTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen);
    static void onFinish(int64_t requestId, int operationCode, const char *returnData);
    static void onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn);
    static char *onRequest(int64_t requestId, int operationCode, const char *reqParams);

public:
    HiChainConnector();
    ~HiChainConnector();

    int32_t RegisterHiChainCallback(std::shared_ptr<IHiChainConnectorCallback> callback);
    int32_t UnRegisterHiChainCallback();
    int32_t CreateGroup(int64_t requestId, const std::string &groupName);
    int32_t CreateGroup(int64_t requestId, int32_t authType, const std::string &userId,
        nlohmann::json &jsonOutObj);
    int32_t AddMember(const std::string &deviceId, const std::string &connectInfo);
    int32_t DelMemberFromGroup(const std::string &groupId, const std::string &deviceId);
    int32_t DeleteGroup(std::string &groupId);
    int32_t DeleteGroup(const int32_t userId, std::string &groupId);
    int32_t DeleteGroup(int64_t requestId_, const std::string &userId, const int32_t authType);
    bool IsDevicesInGroup(const std::string &hostDevice, const std::string &peerDevice);
    int32_t GetRelatedGroups(const std::string &deviceId, std::vector<GroupInfo> &groupList);
    bool GetGroupInfo(const std::string &queryParams, std::vector<GroupInfo> &groupList);
    int32_t GetGroupInfo(const int32_t userId, const std::string &queryParams, std::vector<GroupInfo> &groupList);
    DmAuthForm GetGroupType(const std::string &deviceId);
    int32_t DeleteTimeOutGroup(const char* deviceId);
    int32_t RegisterHiChainGroupCallback(const std::shared_ptr<IDmGroupResCallback> &callback);
    int32_t UnRegisterHiChainGroupCallback();
    int32_t getRegisterInfo(const std::string &queryParams, std::string &returnJsonStr);
    int32_t addMultiMembers(const int32_t groupType, const std::string &userId,
                            const nlohmann::json &jsonDeviceList);
    int32_t deleteMultiMembers(const int32_t groupType, const std::string &userId,
                            const nlohmann::json &jsonDeviceList);
    std::vector<std::string> GetTrustedDevices(const std::string &localDeviceUdid);
    void RegisterDevGroupMgrCallback();
    void UnregisterDevGroupMgrCallback();

public:
    class HiChainSystemAbilityListener : public SystemAbilityStatusChangeStub {
    public:
        explicit HiChainSystemAbilityListener(std::shared_ptr<HiChainConnector> subscriber)
            : hichainSubscriber_(subscriber) {}
        ~HiChainSystemAbilityListener() = default;
        void OnAddSystemAbility(int32_t saId, const std::string& deviceId) override;
        void OnRemoveSystemAbility(int32_t saId, const std::string& deviceId) override;

    private:
        std::shared_ptr<HiChainConnector> hichainSubscriber_;
    };

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
    const nlohmann::json &jsonDeviceList, std::string &params, int32_t &osAccountUserId);
    int32_t GetTrustedDevicesUdid(const char* jsonStr, std::vector<std::string> &udidList);
    void AddHiChainSAMonitor();
    void RemoveHiChainSAMonitor();

private:
    const DeviceGroupManager *deviceGroupManager_ = nullptr;
    DeviceAuthCallback deviceAuthCallback_;
    static std::shared_ptr<IHiChainConnectorCallback> hiChainConnectorCallback_;
    static std::shared_ptr<IDmGroupResCallback> hiChainResCallback_;
    static int32_t networkStyle_;
    sptr<HiChainSystemAbilityListener> hichainSAListener_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HICHAIN_CONNECTOR_H
