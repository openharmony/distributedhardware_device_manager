/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dm_service_hichain_connector_Refactor.h"

#include <cstdlib>
#include <ctime>
#include <functional>
#include <securec.h>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_dfx_constants.h"
#include "dm_hisysevent.h"
#include "dm_log.h"
#include "dm_random.h"
#include "dm_radar_helper.h"
#include "json_object.h"
#include "multiple_user_connector.h"
#include "parameter.h"
#include "unistd.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t CREDENTIAL_NETWORK = 1;
const int32_t DELAY_TIME_MS = 10000;
const int32_t FIELD_EXPIRE_TIME_VALUE = 7;
const int32_t SAME_ACCOUNT = 1;
const int32_t MIN_USER_ID = 0;
const int32_t MAX_USER_ID = 9999;
const int32_t MIN_GROUP_TYPE = 0;
const int32_t MAX_GROUP_TYPE = 255;

constexpr const char* DM_CREATE_GROUP_SUCCESS = "DM_CREATE_GROUP_SUCCESS";
constexpr const char* DM_CREATE_GROUP_FAILED = "DM_CREATE_GROUP_FAILED";
constexpr const char* DM_CREATE_GROUP_SUCCESS_MSG = "dm create group success.";
constexpr const char* DM_CREATE_GROUP_FAILED_MSG = "dm create group failed.";

constexpr const char* FIELD_CREDENTIAL = "credential";
constexpr const char* FIELD_OPERATION_CODE = "operationCode";
constexpr const char* FIELD_META_NODE_TYPE = "metaNodeType";
constexpr const char* FIELD_TYPE = "TType";
constexpr const char* DM_SERVICE = "ohos.distributedhardware.devicemanagerservice";

void FromJson(const JsonItemObject &jsonObject, DmGroupInfo &groupInfo)
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

std::shared_ptr<IDmServiceGroupResCallback> DmServiceHiChainConnector::hiChainResCallback_ = nullptr;
int32_t DmServiceHiChainConnector::networkStyle_ = CREDENTIAL_NETWORK;
std::mutex DmServiceHiChainConnector::groupFlagMutex_;

DmServiceHiChainConnector::DmServiceHiChainConnector()
{
    LOGI("constructor");
    InitDeviceAuthService();
    deviceAuthCallback_ = {.onTransmit = nullptr,
                           .onSessionKeyReturned = nullptr,
                           .onFinish = DmServiceHiChainConnector::onFinish,
                           .onError = DmServiceHiChainConnector::onError,
                           .onRequest = nullptr};
    deviceGroupManager_ = GetGmInstance();
    if (deviceGroupManager_ == nullptr) {
        LOGE("[HICHAIN]failed to init group manager.");
        return;
    }
    int32_t ret = deviceGroupManager_->regCallback(DM_SERVICE, &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("[HICHAIN]fail to register callback to hachain with ret:%{public}d.", ret);
        return;
    }
    LOGI("success.");
}

DmServiceHiChainConnector::~DmServiceHiChainConnector()
{
    DestroyDeviceAuthService();
    LOGI("start");
}

bool DmServiceHiChainConnector::ValidateUserId(const std::string &userId)
{
    if (userId.empty()) {
        LOGE("userId is empty.");
        return false;
    }
    for (auto ch : userId) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            LOGE("userId contains non-digit character.");
            return false;
        }
    }
    return true;
}

bool DmServiceHiChainConnector::ValidateGroupType(int32_t groupType)
{
    if (groupType < MIN_GROUP_TYPE || groupType > MAX_GROUP_TYPE) {
        LOGE("Invalid groupType: %{public}d.", groupType);
        return false;
    }
    return true;
}

void DmServiceHiChainConnector::ClearSensitiveString(std::string &sensitiveData)
{
    if (!sensitiveData.empty()) {
        (void)memset_s(const_cast<char*>(sensitiveData.data()), sensitiveData.size(), 0, sensitiveData.size());
        sensitiveData.clear();
    }
}

std::string DmServiceHiChainConnector::GetJsonStr(const JsonObject &jsonObj, const std::string &key)
{
    if (!IsString(jsonObj, key)) {
        LOGE("User string key not exist!");
        return "";
    }
    return jsonObj[key].Get<std::string>();
}

int32_t DmServiceHiChainConnector::GetJsonInt(const JsonObject &jsonObj, const std::string &key)
{
    if (!IsInt32(jsonObj, key)) {
        LOGE("User string key not exist!");
        return ERR_DM_FAILED;
    }
    return jsonObj[key].Get<int32_t>();
}

bool DmServiceHiChainConnector::GetGroupInfoCommon(const int32_t userId, const std::string &queryParams,
    const char* pkgName, std::vector<DmGroupInfo> &groupList)
{
    char *groupVec = nullptr;
    uint32_t num = 0;
    if (deviceGroupManager_ == nullptr) {
        LOGE("deviceGroupManager_ is null");
        return false;
    }
    int32_t ret = deviceGroupManager_->getGroupInfo(userId, pkgName, queryParams.c_str(), &groupVec, &num);
    if (ret != 0) {
        LOGE("[HICHAIN]fail to get group info with ret:%{public}d.", ret);
        deviceGroupManager_->destroyInfo(&groupVec);
        return false;
    }
    if (groupVec == nullptr) {
        LOGE("[HICHAIN]return groups info point is nullptr");
        return false;
    }
    if (num == 0) {
        LOGE("[HICHAIN]return groups info number is zero.");
        deviceGroupManager_->destroyInfo(&groupVec);
        return false;
    }
    LOGI("groupNum(%{public}u)", num);
    std::string relatedGroups = std::string(groupVec);
    deviceGroupManager_->destroyInfo(&groupVec);
    JsonObject jsonObject(relatedGroups);
    ClearSensitiveString(relatedGroups);
    if (jsonObject.IsDiscarded()) {
        LOGE("returnGroups parse error");
        return false;
    }
    if (!jsonObject.IsArray()) {
        LOGE("json string is not array.");
        return false;
    }
    std::vector<DmGroupInfo> groupInfos;
    jsonObject.Get(groupInfos);
    if (groupInfos.empty()) {
        LOGE("group failed, groupInfos is empty.");
        return false;
    }
    groupList = groupInfos;
    return true;
}

bool DmServiceHiChainConnector::GetGroupInfo(const int32_t userId, const std::string &queryParams,
    std::vector<DmGroupInfo> &groupList)
{
    return GetGroupInfoCommon(userId, queryParams, DM_SERVICE, groupList);
}

bool DmServiceHiChainConnector::GetGroupInfo(const std::string &queryParams, std::vector<DmGroupInfo> &groupList)
{
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return false;
    }
    return GetGroupInfo(userId, queryParams, groupList);
}

int32_t DmServiceHiChainConnector::GetGroupIdExt(const std::string &userId, const int32_t groupType,
    std::string &groupId, std::string &groupOwner)
{
    if (!ValidateUserId(userId)) {
        LOGE("Invalid userId.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!ValidateGroupType(groupType)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    JsonObject jsonObjGroup;
    jsonObjGroup[FIELD_GROUP_TYPE] = groupType;
    std::string queryParams = jsonObjGroup.Dump();
    std::vector<DmGroupInfo> groupList;

    if (!GetGroupInfo(queryParams.c_str(), groupList)) {
        LOGE("failed to get device join groups");
        ClearSensitiveString(queryParams);
        return ERR_DM_FAILED;
    }
    ClearSensitiveString(queryParams);
    for (auto &groupinfo : groupList) {
        LOGI("groupinfo.groupId:%{public}s", GetAnonyString(groupinfo.groupId).c_str());
        if (groupinfo.userId == userId) {
            groupId = groupinfo.groupId;
            groupOwner = groupinfo.groupOwner;
            return DM_OK;
        }
    }
    return ERR_DM_FAILED;
}

int32_t DmServiceHiChainConnector::ParseRemoteCredentialExt(const std::string &credentialInfo, std::string &params,
    std::string &groupOwner)
{
    LOGI("start.");
    JsonObject jsonObject(credentialInfo);
    if (jsonObject.IsDiscarded()) {
        LOGE("CredentialInfo string not a json type.");
        return ERR_DM_FAILED;
    }
    JsonObject jsonObj;
    int32_t groupType = 0;
    std::string userId = "";
    int32_t authType = GetJsonInt(jsonObject, AUTH_TYPE);
    if (authType == SAME_ACCOUNT) {
        groupType = IDENTICAL_ACCOUNT_GROUP;
        userId = GetJsonStr(jsonObject, FIELD_USER_ID);
    } else {
        LOGE("Failed to get userId.");
        return ERR_DM_FAILED;
    }
    if (!ValidateUserId(userId)) {
        LOGE("Invalid userId from credential.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string groupId = "";
    if (GetGroupIdExt(userId, groupType, groupId, groupOwner) != DM_OK) {
        LOGE("Failed to get groupid");
        return ERR_DM_FAILED;
    }
    jsonObj[FIELD_GROUP_TYPE] = groupType;
    jsonObj[FIELD_GROUP_ID] = groupId;
    jsonObj[FIELD_USER_ID] = userId;
    jsonObj[FIELD_CREDENTIAL_TYPE] = GetJsonInt(jsonObject, FIELD_CREDENTIAL_TYPE);
    jsonObj[FIELD_OPERATION_CODE] = GetJsonInt(jsonObject, FIELD_OPERATION_CODE);
    jsonObj[FIELD_META_NODE_TYPE] = GetJsonStr(jsonObject, FIELD_TYPE);
    if (!jsonObject.Contains(FIELD_DEVICE_LIST)) {
        LOGE("Credentialdata or authType string key not exist!");
        return ERR_DM_FAILED;
    }
    std::string jsonStr = jsonObject[FIELD_DEVICE_LIST].Dump();
    JsonObject jsonArray(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    jsonArray.Parse(jsonStr);
    jsonObj.Insert(FIELD_DEVICE_LIST, jsonArray);
    params = jsonObj.Dump();
    ClearSensitiveString(jsonStr);
    return DM_OK;
}

bool DmServiceHiChainConnector::IsRedundanceGroup(const std::string &userId,
    int32_t authType, std::vector<DmGroupInfo> &groupList)
{
    if (!ValidateUserId(userId)) {
        LOGE("Invalid userId.");
        return false;
    }
    if (!ValidateGroupType(authType)) {
        return false;
    }
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = authType;
    std::string queryParams = jsonObj.Dump();

    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        ClearSensitiveString(queryParams);
        return false;
    }
    if (!GetGroupInfo(osAccountUserId, queryParams, groupList)) {
        ClearSensitiveString(queryParams);
        return false;
    }
    ClearSensitiveString(queryParams);
    for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
        if (iter->userId != userId) {
            return true;
        }
    }
    return false;
}

void DmServiceHiChainConnector::DeleteRedundanceGroup(std::string &userId)
{
    int32_t nTickTimes = 0;
    g_deleteGroupFlag.store(false);
    DeleteGroup(userId);
    while (!g_deleteGroupFlag.load()) {
        usleep(DELAY_TIME_MS);
        if (++nTickTimes > SERVICE_INIT_TRY_MAX_NUM) {
            LOGE("failed to delete group because timeout!");
            return;
        }
    }
}

void DmServiceHiChainConnector::DealRedundanceGroup(const std::string &userId, int32_t authType)
{
    g_groupIsRedundance.store(false);
    std::vector<DmGroupInfo> groupList;
    if (IsRedundanceGroup(userId, authType, groupList)) {
        LOGI("IsRedundanceGroup");
        g_groupIsRedundance.store(true);
        for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
            if (iter->userId != userId) {
                DeleteRedundanceGroup(iter->userId);
            }
        }
        g_groupIsRedundance.store(false);
    }
}

int32_t DmServiceHiChainConnector::RegisterHiChainGroupCallback(
    const std::shared_ptr<IDmServiceGroupResCallback> &callback)
{
    std::lock_guard<std::mutex> lock(groupFlagMutex_);
    hiChainResCallback_ = callback;
    return DM_OK;
}

int32_t DmServiceHiChainConnector::UnRegisterHiChainGroupCallback()
{
    std::lock_guard<std::mutex> lock(groupFlagMutex_);
    hiChainResCallback_ = nullptr;
    return DM_OK;
}

int32_t DmServiceHiChainConnector::GetGroupId(const std::string &userId,
    const int32_t groupType, std::string &groupId)
{
    if (!ValidateUserId(userId)) {
        LOGE("Invalid userId.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!ValidateGroupType(groupType)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    JsonObject jsonObjGroup;
    jsonObjGroup[FIELD_GROUP_TYPE] = groupType;
    std::string queryParams = jsonObjGroup.Dump();
    std::vector<DmGroupInfo> groupList;

    if (!GetGroupInfo(queryParams.c_str(), groupList)) {
        LOGE("failed to get device join groups");
        ClearSensitiveString(queryParams);
        return ERR_DM_FAILED;
    }
    ClearSensitiveString(queryParams);
    for (auto &groupinfo : groupList) {
        LOGI("groupinfo.groupId:%{public}s", GetAnonyString(groupinfo.groupId).c_str());
        if (groupinfo.userId == userId) {
            groupId = groupinfo.groupId;
            return DM_OK;
        }
    }
    return ERR_DM_FAILED;
}

int32_t DmServiceHiChainConnector::ParseRemoteCredential(const int32_t groupType, const std::string &userId,
    const JsonObject &jsonDeviceList, std::string &params, int32_t &osAccountUserId)
{
    if (!ValidateUserId(userId)) {
        LOGE("Invalid userId.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!ValidateGroupType(groupType)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (userId.empty() || !jsonDeviceList.Contains(FIELD_DEVICE_LIST)) {
        LOGE("userId or deviceList is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string groupId;
    if (GetGroupId(userId, groupType, groupId) != DM_OK) {
        LOGE("failed to get groupid");
        return ERR_DM_FAILED;
    }
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    jsonObj[FIELD_GROUP_TYPE] = groupType;
    std::string jsonStr = jsonDeviceList[FIELD_DEVICE_LIST].Dump();
    JsonObject jsonArray(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    jsonArray.Parse(jsonStr);
    jsonObj.Insert(FIELD_DEVICE_LIST, jsonArray);
    params = jsonObj.Dump();
    ClearSensitiveString(jsonStr);
    osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DmServiceHiChainConnector::addMultiMembers(const int32_t groupType, const std::string &userId,
    const JsonObject &jsonDeviceList)
{
    if (deviceGroupManager_ == nullptr) {
        LOGE("deviceGroupManager_ is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string addParams;
    int32_t osAccountUserId = 0;
    if (ParseRemoteCredential(groupType, userId, jsonDeviceList, addParams, osAccountUserId) != DM_OK) {
        LOGE("ParseRemoteCredential failed!");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->addMultiMembersToGroup(osAccountUserId, DM_SERVICE, addParams.c_str());
    ClearSensitiveString(addParams);
    if (ret != DM_OK) {
        LOGE("[HICHAIN]fail to add member to hichain group with ret:%{public}d.", ret);
        return ERR_DM_ADD_GROUP_FAILED;
    }
    return DM_OK;
}

int32_t DmServiceHiChainConnector::deleteMultiMembers(const int32_t groupType, const std::string &userId,
    const JsonObject &jsonDeviceList)
{
    if (deviceGroupManager_ == nullptr) {
        LOGE("deviceGroupManager_ is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::string deleteParams;
    int32_t osAccountUserId = 0;
    if (ParseRemoteCredential(groupType, userId, jsonDeviceList, deleteParams, osAccountUserId) != DM_OK) {
        LOGE("ParseRemoteCredential failed!");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->delMultiMembersFromGroup(osAccountUserId, DM_SERVICE, deleteParams.c_str());
    ClearSensitiveString(deleteParams);
    if (ret != DM_OK) {
        LOGE("[HICHAIN]fail to delete member from hichain group with ret:%{public}d.", ret);
        return ret;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
