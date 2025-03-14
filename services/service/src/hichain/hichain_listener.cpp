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

#include "hichain_listener.h"

#include "device_manager_service.h"
#include "dm_random.h"
#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {

namespace {
    constexpr uint32_t MAX_DATA_LEN = 65536;
    constexpr uint32_t DM_IDENTICAL_ACCOUNT = 1;
    constexpr const char* DM_PKG_NAME_EXT = "com.huawei.devicemanager";
}

static DataChangeListener dataChangeListener_ = {
    .onDeviceUnBound = HichainListener::OnHichainDeviceUnBound,
};

void from_json(const nlohmann::json &jsonObject, GroupInformation &groupInfo)
{
    if (jsonObject.find(FIELD_GROUP_TYPE) != jsonObject.end() && jsonObject.at(FIELD_GROUP_TYPE).is_number_integer()) {
        groupInfo.groupType = jsonObject.at(FIELD_GROUP_TYPE).get<int32_t>();
    }
    // FIELD_USER_ID is osAccountId
    if (jsonObject.find(FIELD_USER_ID) != jsonObject.end() && jsonObject.at(FIELD_USER_ID).is_string()) {
        groupInfo.osAccountId = jsonObject.at(FIELD_USER_ID).get<std::string>();
    }
    // FIELD_OS_ACCOUNT_ID is userId
    if (jsonObject.find(FIELD_OS_ACCOUNT_ID) != jsonObject.end() &&
        jsonObject.at(FIELD_OS_ACCOUNT_ID).is_number_integer()) {
        groupInfo.userId = jsonObject.at(FIELD_OS_ACCOUNT_ID).get<int32_t>();
    }
}

void from_json(const nlohmann::json &jsonObject, GroupsInfo &groupInfo)
{
    if (jsonObject.find(FIELD_GROUP_NAME) != jsonObject.end() && jsonObject.at(FIELD_GROUP_NAME).is_string()) {
        groupInfo.groupName = jsonObject.at(FIELD_GROUP_NAME).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_ID) != jsonObject.end() && jsonObject.at(FIELD_GROUP_ID).is_string()) {
        groupInfo.groupId = jsonObject.at(FIELD_GROUP_ID).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_OWNER) != jsonObject.end() && jsonObject.at(FIELD_GROUP_OWNER).is_string()) {
        groupInfo.groupOwner = jsonObject.at(FIELD_GROUP_OWNER).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_TYPE) != jsonObject.end() && jsonObject.at(FIELD_GROUP_TYPE).is_number_integer()) {
        groupInfo.groupType = jsonObject.at(FIELD_GROUP_TYPE).get<int32_t>();
    }

    if (jsonObject.find(FIELD_GROUP_VISIBILITY) != jsonObject.end() &&
        jsonObject.at(FIELD_GROUP_VISIBILITY).is_number_integer()) {
        groupInfo.groupVisibility = jsonObject.at(FIELD_GROUP_VISIBILITY).get<int32_t>();
    }

    if (jsonObject.find(FIELD_USER_ID) != jsonObject.end() && jsonObject.at(FIELD_USER_ID).is_string()) {
        groupInfo.userId = jsonObject.at(FIELD_USER_ID).get<std::string>();
    }
}

HichainListener::HichainListener()
{
    LOGI("HichainListener constructor start.");
    InitDeviceAuthService();
    deviceGroupManager_ = GetGmInstance();
    if (deviceGroupManager_ == nullptr) {
        LOGE("[HICHAIN]failed to init group manager.");
        return;
    }
    LOGI("HichainListener::constructor success.");
}

HichainListener::~HichainListener()
{
    LOGI("HichainListener::destructor.");
    DestroyDeviceAuthService();
}

void HichainListener::RegisterDataChangeCb()
{
    LOGI("HichainListener::RegisterDataChangeCb start");
    if (deviceGroupManager_ == nullptr) {
        LOGE("deviceGroupManager_ is null!");
        return;
    }
    int32_t ret = deviceGroupManager_->regDataChangeListener(DM_PKG_NAME, &dataChangeListener_);
    if (ret != DM_OK) {
        LOGE("[HICHAIN]regDataChangeListener failed with ret: %{public}d.", ret);
        return;
    }
    LOGI("RegisterDataChangeCb success!");
}

void HichainListener::OnHichainDeviceUnBound(const char *peerUdid, const char *groupInfo)
{
    LOGI("HichainListener::onDeviceUnBound start");
    if (peerUdid == nullptr || groupInfo == nullptr) {
        LOGE("peerUdid or groupInfo is null!");
        return;
    }
    if (strlen(peerUdid) > MAX_DATA_LEN || strlen(groupInfo) > MAX_DATA_LEN) {
        LOGE("peerUdid or groupInfo is invalid");
        return;
    }
    nlohmann::json groupInfoJsonObj = nlohmann::json::parse(std::string(groupInfo), nullptr, false);
    if (groupInfoJsonObj.is_discarded()) {
        LOGE("groupInfo parse error");
        return ;
    }
    GroupInformation hichainGroupInfo;
    from_json(groupInfoJsonObj, hichainGroupInfo);
    if (hichainGroupInfo.groupType != DM_IDENTICAL_ACCOUNT) {
        LOGI("groupType is %{public}d, not idential account.", hichainGroupInfo.groupType);
        return;
    }
    string accountId = MultipleUserConnector::GetOhosAccountId();
    if (accountId == hichainGroupInfo.osAccountId && accountId != "ohosAnonymousUid") {
        LOGI("accountId = %{public}s.", GetAnonyString(accountId).c_str());
        DeviceManagerService::GetInstance().HandleDeviceUnBind(peerUdid, hichainGroupInfo);
        return;
    }
}

void HichainListener::DeleteAllGroup(const std::string &localUdid, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("OnStart HichainListener::DeleteAllGroup");
    for (auto &userId : backgroundUserIds) {
        std::vector<GroupsInfo> groupList;
        GetRelatedGroups(userId, localUdid, groupList);
        for (auto &iter : groupList) {
            if (iter.groupType == GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP) {
                continue;
            }
            if (DeleteGroup(userId, iter.groupId) != DM_OK) {
                LOGE("Delete groupId %{public}s failed.", GetAnonyString(iter.groupId).c_str());
            }
        }
        std::vector<GroupsInfo> groupListExt;
        GetRelatedGroupsExt(userId, localUdid, groupListExt);
        for (auto &iter : groupListExt) {
            if (iter.groupType == GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP) {
                continue;
            }
            if (DeleteGroupExt(userId, iter.groupId) != DM_OK) {
                LOGE("DeleteGroupExt groupId %{public}s failed.", GetAnonyString(iter.groupId).c_str());
            }
        }
    }
}

int32_t HichainListener::GetRelatedGroups(int32_t userId, const std::string &deviceId,
    std::vector<GroupsInfo> &groupList)
{
    return GetRelatedGroupsCommon(userId, deviceId, DM_PKG_NAME, groupList);
}

int32_t HichainListener::GetRelatedGroupsExt(int32_t userId, const std::string &deviceId,
    std::vector<GroupsInfo> &groupList)
{
    return GetRelatedGroupsCommon(userId, deviceId, DM_PKG_NAME_EXT, groupList);
}

int32_t HichainListener::GetRelatedGroupsCommon(int32_t userId, const std::string &deviceId, const char* pkgName,
    std::vector<GroupsInfo> &groupList)
{
    LOGI("Start to get related groups.");
    if (userId < 0) {
        LOGE("user id failed");
        return ERR_DM_FAILED;
    }
    uint32_t groupNum = 0;
    char *returnGroups = nullptr;
    int32_t ret =
        deviceGroupManager_->getRelatedGroups(userId, pkgName, deviceId.c_str(), &returnGroups, &groupNum);
    if (ret != 0) {
        LOGE("[HICHAIN] fail to get related groups with ret:%{public}d.", ret);
        deviceGroupManager_->destroyInfo(&returnGroups);
        return ERR_DM_FAILED;
    }
    if (returnGroups == nullptr) {
        LOGE("[HICHAIN] return related goups point is nullptr");
        return ERR_DM_FAILED;
    }
    if (groupNum == 0) {
        LOGE("[HICHAIN]return related goups number is zero.");
        deviceGroupManager_->destroyInfo(&returnGroups);
        return ERR_DM_FAILED;
    }
    std::string relatedGroups = std::string(returnGroups);
    deviceGroupManager_->destroyInfo(&returnGroups);
    nlohmann::json jsonObject = nlohmann::json::parse(relatedGroups, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("returnGroups parse error");
        return ERR_DM_FAILED;
    }
    if (!jsonObject.is_array()) {
        LOGE("jsonObject is not an array.");
        return ERR_DM_FAILED;
    }
    std::vector<GroupsInfo> groupInfos = jsonObject.get<std::vector<GroupsInfo>>();
    if (groupInfos.empty()) {
        LOGE("HichainListener::GetRelatedGroups group failed, groupInfos is empty.");
        return ERR_DM_FAILED;
    }
    groupList = groupInfos;
    return DM_OK;
}

int64_t HichainListener::GenRequestId()
{
    return GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
}

int32_t HichainListener::DeleteGroup(const int32_t userId, std::string &groupId)
{
    if (userId < 0) {
        LOGE("user id failed");
        return ERR_DM_FAILED;
    }
    int64_t requestId = GenRequestId();
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = SafetyDump(jsonObj);
    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_PKG_NAME, disbandParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete group with ret:%{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HichainListener::DeleteGroupExt(int32_t userId, std::string &groupId)
{
    int64_t requestId = GenRequestId();
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = SafetyDump(jsonObj);
    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_PKG_NAME_EXT, disbandParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete group with ret:%{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS