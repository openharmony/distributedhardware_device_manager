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
#include "dm_constants.h"
#include "dm_random.h"
#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {

namespace {
    constexpr uint32_t MAX_DATA_LEN = 65536;
    constexpr uint32_t DM_IDENTICAL_ACCOUNT = 1;
    constexpr uint32_t ACCOUNT_SHARED = 3;
    constexpr const char* DM_PKG_NAME_EXT = "com.huawei.devicemanager";
}

static DataChangeListener dataChangeListener_ = {
    .onDeviceUnBound = HichainListener::OnHichainDeviceUnBound,
};

static CredChangeListener credChangeListener_ = {
    .onCredDelete = HichainListener::OnCredentialDeleted,
};

void FromJson(const JsonItemObject &jsonObject, GroupInformation &groupInfo)
{
    if (jsonObject.Contains(FIELD_GROUP_TYPE) && jsonObject.At(FIELD_GROUP_TYPE).IsNumberInteger()) {
        groupInfo.groupType = jsonObject.At(FIELD_GROUP_TYPE).Get<int32_t>();
    }
    // FIELD_USER_ID is osAccountId
    if (jsonObject.Contains(FIELD_USER_ID) && jsonObject.At(FIELD_USER_ID).IsString()) {
        groupInfo.osAccountId = jsonObject.At(FIELD_USER_ID).Get<std::string>();
    }
    // FIELD_OS_ACCOUNT_ID is userId
    if (jsonObject.Contains(FIELD_OS_ACCOUNT_ID) &&
        jsonObject.At(FIELD_OS_ACCOUNT_ID).IsNumberInteger()) {
        groupInfo.userId = jsonObject.At(FIELD_OS_ACCOUNT_ID).Get<int32_t>();
    }
}

void FromJson(const JsonItemObject &jsonObject, GroupsInfo &groupInfo)
{
    if (jsonObject.Contains(FIELD_GROUP_NAME) && jsonObject.At(FIELD_GROUP_NAME).IsString()) {
        groupInfo.groupName = jsonObject.At(FIELD_GROUP_NAME).Get<std::string>();
    }

    if (jsonObject.Contains(FIELD_GROUP_ID) && jsonObject.At(FIELD_GROUP_ID).IsString()) {
        groupInfo.groupId = jsonObject.At(FIELD_GROUP_ID).Get<std::string>();
    }

    if (jsonObject.Contains(FIELD_GROUP_OWNER) && jsonObject.At(FIELD_GROUP_OWNER).IsString()) {
        groupInfo.groupOwner = jsonObject.At(FIELD_GROUP_OWNER).Get<std::string>();
    }

    if (jsonObject.Contains(FIELD_GROUP_TYPE) && jsonObject.At(FIELD_GROUP_TYPE).IsNumberInteger()) {
        groupInfo.groupType = jsonObject.At(FIELD_GROUP_TYPE).Get<int32_t>();
    }

    if (jsonObject.Contains(FIELD_GROUP_VISIBILITY) &&
        jsonObject.At(FIELD_GROUP_VISIBILITY).IsNumberInteger()) {
        groupInfo.groupVisibility = jsonObject.At(FIELD_GROUP_VISIBILITY).Get<int32_t>();
    }

    if (jsonObject.Contains(FIELD_USER_ID) && jsonObject.At(FIELD_USER_ID).IsString()) {
        groupInfo.userId = jsonObject.At(FIELD_USER_ID).Get<std::string>();
    }
}

HichainListener::HichainListener()
{
    LOGI("HichainListener constructor start.");
    InitDeviceAuthService();
    deviceGroupManager_ = GetGmInstance();
    credManager_ = GetCredMgrInstance();
    if (deviceGroupManager_ == nullptr) {
        LOGE("[HICHAIN]failed to init group manager.");
        return;
    }
    if (credManager_ == nullptr) {
        LOGE("[HICHAIN]failed to init cred manager.");
        return;
    }
    LOGI("HichainListener::constructor success.");
}

HichainListener::~HichainListener()
{
    LOGI("HichainListener::destructor.");
    DestroyDeviceAuthService();
}

//LCOV_EXCL_START
void HichainListener::RegisterDataChangeCb()
{
    LOGI("start");
    if (deviceGroupManager_ == nullptr) {
        LOGE("deviceGroupManager_ is null!");
        return;
    }
    int32_t ret = deviceGroupManager_->regDataChangeListener(DM_PKG_NAME, &dataChangeListener_);
    if (ret != DM_OK) {
        LOGE("[HICHAIN]regDataChangeListener failed with ret: %{public}d.", ret);
        return;
    }
    LOGI("success!");
}

void HichainListener::RegisterCredentialCb()
{
    LOGI("start");
    if (credManager_ == nullptr) {
        LOGE("credManager_ is null!");
        return;
    }
    int32_t ret = credManager_->registerChangeListener(DM_PKG_NAME, &credChangeListener_);
    if (ret != DM_OK) {
        LOGE("[HICHAIN]registerChangeListener failed with ret: %{public}d.", ret);
        return;
    }
    LOGI("success!");
}
//LCOV_EXCL_STOP

void HichainListener::OnHichainDeviceUnBound(const char *peerUdid, const char *groupInfo)
{
    LOGI("start");
    if (peerUdid == nullptr || groupInfo == nullptr) {
        LOGE("peerUdid or groupInfo is null!");
        return;
    }
    if (strlen(peerUdid) > MAX_DATA_LEN || strlen(groupInfo) > MAX_DATA_LEN) {
        LOGE("peerUdid or groupInfo is invalid");
        return;
    }
    std::string strJson(groupInfo);
    JsonObject groupInfoJsonObj(strJson);
    if (groupInfoJsonObj.IsDiscarded()) {
        LOGE("groupInfo parse error");
        return;
    }
    GroupInformation hichainGroupInfo;
    FromJson(groupInfoJsonObj, hichainGroupInfo);
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

void HichainListener::OnCredentialDeleted(const char *credId, const char *credInfo)
{
    if (credId == nullptr || credInfo == nullptr) {
        LOGE("credId or credInfo is null!");
        return;
    }
    if (strlen(credId) > MAX_DATA_LEN || strlen(credInfo) > MAX_DATA_LEN) {
        LOGE("credId or credInfo is invalid");
        return;
    }
    LOGI("start, credId: %{public}s.", GetAnonyString(credId).c_str());
    JsonObject jsonObject;
    jsonObject.Parse(std::string(credInfo));
    if (jsonObject.IsDiscarded()) {
        LOGE("credInfo prase error.");
        return;
    }
    uint32_t credType = 0;
    std::string credTypeTag = "credType";
    if (IsInt32(jsonObject, credTypeTag)) {
        credType = static_cast<uint32_t>(jsonObject[credTypeTag].Get<int32_t>());
    }
    if (credType != ACCOUNT_SHARED) {
        LOGE("credType %{public}d is invalid.", credType);
        return;
    }
    DeviceManagerService::GetInstance().HandleCredentialDeleted(credId, credInfo);
    return;
}

void HichainListener::DeleteAllGroup(const std::string &localUdid, const std::vector<int32_t> &backgroundUserIds)
{
    LOGI("start");
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
    if (userId < 0) {
        LOGE("user id failed");
        return ERR_DM_FAILED;
    }
    LOGI("Start.");
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
    JsonObject jsonObject(relatedGroups);
    if (jsonObject.IsDiscarded()) {
        LOGE("returnGroups parse error");
        return ERR_DM_FAILED;
    }
    if (!jsonObject.IsArray()) {
        LOGE("jsonObject is not an array.");
        return ERR_DM_FAILED;
    }
    std::vector<GroupsInfo> groupInfos;
    jsonObject.Get(groupInfos);
    if (groupInfos.empty()) {
        LOGE("HichainListener::GetRelatedGroups group failed, groupInfos is empty.");
        return ERR_DM_FAILED;
    }
    groupList = groupInfos;
    return DM_OK;
}

//LCOV_EXCL_START
int64_t HichainListener::GenRequestId()
{
    return GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
}
//LCOV_EXCL_STOP

int32_t HichainListener::DeleteGroup(const int32_t userId, std::string &groupId)
{
    if (userId < 0) {
        LOGE("user id failed");
        return ERR_DM_FAILED;
    }
    int64_t requestId = GenRequestId();
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = jsonObj.Dump();
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
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = jsonObj.Dump();
    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_PKG_NAME_EXT, disbandParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete group with ret:%{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HichainListener::DeleteCredential(int32_t osAccountId, const std::string &credId)
{
    LOGI("start. osAccountId=%{public}s, credId=%{public}s", GetAnonyInt32(osAccountId).c_str(),
        GetAnonyString(credId).c_str());
    if (credManager_ == nullptr) {
        LOGE("credManager_ is nullptr");
        return ERR_DM_POINT_NULL;
    }
    int32_t ret = credManager_->deleteCredential(osAccountId, credId.c_str());
    if (ret != DM_OK) {
        LOGE("Hichain deleteCredential failed ret %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS