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
#include <vector>

#include "device_auth.h"
#include "hichain_connector_callback.h"
#include "nlohmann/json.hpp"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
struct GroupInfo {
    std::string groupName;
    std::string groupId;
    std::string groupOwner;
    int32_t groupType;
    int32_t groupVisibility;

    GroupInfo() : groupName(""), groupId(""), groupOwner(""), groupType(0), groupVisibility(0)
    {
    }
};

void from_json(const nlohmann::json &jsonObject, GroupInfo &groupInfo);

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
     * @tc.name: HiChainConnector::IsDevicesInGroup
     * @tc.desc: IsDevicesInGroup of the HiChain Connector
     * @tc.type: FUNC
     */
    bool IsDevicesInGroup(const std::string &hostDevice, const std::string &peerDevice);

    /**
     * @tc.name: HiChainConnector::GetRelatedGroups
     * @tc.desc: Get Related Groups of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t GetRelatedGroups(const std::string &DeviceId, std::vector<GroupInfo> &groupList);

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
    int32_t GetGroupInfo(const int32_t userId, const std::string &queryParams, std::vector<GroupInfo> &groupList);

    /**
     * @tc.name: HiChainConnector::DeleteTimeOutGroup
     * @tc.desc: Delete TimeOut Group of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t DeleteTimeOutGroup(const char* deviceId);
private:
    int64_t GenRequestId();
    int32_t SyncGroups(std::string deviceId, std::vector<std::string> &remoteGroupIdList);
    int32_t GetSyncGroupList(std::vector<GroupInfo> &groupList, std::vector<std::string> &syncGroupList);
    std::string GetConnectPara(std::string deviceId, std::string reqDeviceId);
    bool IsGroupCreated(std::string groupName, GroupInfo &groupInfo);
    bool IsGroupInfoInvalid(GroupInfo &group);

private:
    const DeviceGroupManager *deviceGroupManager_ = nullptr;
    DeviceAuthCallback deviceAuthCallback_;
    static std::shared_ptr<IHiChainConnectorCallback> hiChainConnectorCallback_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HICHAIN_CONNECTOR_H
