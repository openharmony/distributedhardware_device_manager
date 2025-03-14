/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_HICHAIN_LISTENER_H
#define OHOS_DM_HICHAIN_LISTENER_H


#include "device_auth.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {

struct GroupInformation {
    std::string groupName;
    std::string groupId;
    std::string groupOwner;
    int32_t groupType;
    int32_t groupVisibility;
    int32_t userId;
    std::string osAccountId;

    GroupInformation() : groupName(""), groupId(""), groupOwner(""), groupType(0),
        groupVisibility(0), userId(0), osAccountId("") {
    }
};

struct GroupsInfo {
    std::string groupName;
    std::string groupId;
    std::string groupOwner;
    int32_t groupType;
    int32_t groupVisibility;
    std::string userId;

    GroupsInfo() : groupName(""), groupId(""), groupOwner(""), groupType(0), groupVisibility(0), userId("")
    {
    }
};

void from_json(const nlohmann::json &jsonObject, GroupInformation &groupInfo);
void from_json(const nlohmann::json &jsonObject, GroupsInfo &groupInfo);

class HichainListener {
public:
    HichainListener();
    ~HichainListener();
    void RegisterDataChangeCb();
    void DeleteAllGroup(const std::string &localUdid, const std::vector<int32_t> &backgroundUserIds);
    int32_t GetRelatedGroups(int32_t userId, const std::string &deviceId,
        std::vector<GroupsInfo> &groupList);
    int32_t GetRelatedGroupsCommon(int32_t userId, const std::string &deviceId, const char* pkgName,
        std::vector<GroupsInfo> &groupList);
    int32_t DeleteGroup(const int32_t userId, std::string &groupId);
    int32_t GetRelatedGroupsExt(int32_t userId, const std::string &deviceId, std::vector<GroupsInfo> &groupList);
    int32_t DeleteGroupExt(int32_t userId, std::string &groupId);
    int64_t GenRequestId();

    static void OnHichainDeviceUnBound(const char *peerUdid, const char *groupInfo);

private:
    const DeviceGroupManager *deviceGroupManager_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_HICHAIN_LISTENER_H