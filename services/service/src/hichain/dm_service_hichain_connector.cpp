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

#include "dm_service_hichain_connector.h"

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
const int32_t DELAY_TIME_MS = 10000; // 10ms
const int32_t FIELD_EXPIRE_TIME_VALUE = 7;
const int32_t SAME_ACCOUNT = 1;

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
bool g_createGroupFlag = false;
bool g_deleteGroupFlag = false;
bool g_groupIsRedundance = false;

DmServiceHiChainConnector::DmServiceHiChainConnector()
{
    LOGI("DmServiceHiChainConnector::constructor");
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
    LOGI("DmServiceHiChainConnector::constructor success.");
}

DmServiceHiChainConnector::~DmServiceHiChainConnector()
{
    DestroyDeviceAuthService();
    LOGI("DmServiceHiChainConnector::destructor.");
}

void DmServiceHiChainConnector::onFinish(int64_t requestId, int operationCode, const char *returnData)
{
    std::string data = (returnData != nullptr) ? std::string(returnData) : "";
    LOGI("DmServiceHiChainConnector::onFinish reqId:%{public}" PRId64 ", operation:%{public}d",
        requestId, operationCode);
    if (operationCode == GroupOperationCode::GROUP_CREATE) {
        LOGI("Create group success");
        if (!DmRadarHelper::GetInstance().ReportAuthCreateGroupCb(
            "onFinish", static_cast<int32_t>(StageRes::STAGE_SUCC))) {
            LOGE("ReportAuthCreateGroupCb failed");
        }
        SysEventWrite(std::string(DM_CREATE_GROUP_SUCCESS), DM_HISYEVENT_BEHAVIOR,
            std::string(DM_CREATE_GROUP_SUCCESS_MSG));
        if (networkStyle_ == CREDENTIAL_NETWORK) {
            if (hiChainResCallback_ != nullptr) {
                int32_t importAction = 0;
                hiChainResCallback_->OnGroupResult(requestId, importAction, data);
                g_createGroupFlag = true;
            }
        }
    }
    if (operationCode == GroupOperationCode::GROUP_DISBAND) {
        if (networkStyle_ == CREDENTIAL_NETWORK && hiChainResCallback_ != nullptr) {
            if (!g_groupIsRedundance) {
                int32_t deleteAction = 1;
                hiChainResCallback_->OnGroupResult(requestId, deleteAction, data);
            }
            g_deleteGroupFlag = true;
        }
        LOGI("Disband group success");
    }
}

void DmServiceHiChainConnector::onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    std::string data = (errorReturn != nullptr) ? std::string(errorReturn) : "";
    LOGI("HichainAuthenCallBack::onError reqId:%{public}" PRId64 ", operation:%{public}d, errorCode:%{public}d.",
        requestId, operationCode, errorCode);
    if (operationCode == GroupOperationCode::GROUP_CREATE) {
        LOGE("Create group failed");
        if (!DmRadarHelper::GetInstance().ReportAuthCreateGroupCb(
            "onError", static_cast<int32_t>(StageRes::STAGE_FAIL))) {
            LOGE("ReportAuthCreateGroupCb failed");
        }
        SysEventWrite(std::string(DM_CREATE_GROUP_FAILED), DM_HISYEVENT_BEHAVIOR,
            std::string(DM_CREATE_GROUP_FAILED_MSG));
        if (networkStyle_ == CREDENTIAL_NETWORK) {
            if (hiChainResCallback_ != nullptr) {
                int32_t importAction = 0;
                hiChainResCallback_->OnGroupResult(requestId, importAction, data);
                g_createGroupFlag = true;
            }
        }
    }
    if (operationCode == GroupOperationCode::GROUP_DISBAND) {
        if (networkStyle_ == CREDENTIAL_NETWORK && hiChainResCallback_ != nullptr) {
            if (!g_groupIsRedundance) {
                int32_t deleteAction = 1;
                hiChainResCallback_->OnGroupResult(requestId, deleteAction, data);
            }
            g_deleteGroupFlag = true;
        }
        LOGE("Disband group failed");
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
    LOGI("DmServiceHiChainConnector::GetGroupInfo groupNum(%{public}u)", num);
    std::string relatedGroups = std::string(groupVec);
    deviceGroupManager_->destroyInfo(&groupVec);
    JsonObject jsonObject(relatedGroups);
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
        LOGE("DmServiceHiChainConnector::GetGroupInfo group failed, groupInfos is empty.");
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
    JsonObject jsonObjGroup;
    jsonObjGroup[FIELD_GROUP_TYPE] = groupType;
    std::string queryParams = jsonObjGroup.Dump();
    std::vector<DmGroupInfo> groupList;

    if (!GetGroupInfo(queryParams.c_str(), groupList)) {
        LOGE("failed to get device join groups");
        return ERR_DM_FAILED;
    }
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
    LOGI("ParseRemoteCredentialExt start.");
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
    return DM_OK;
}

int32_t DmServiceHiChainConnector::addMultiMembersExt(const std::string &credentialInfo)
{
    if (deviceGroupManager_ == nullptr) {
        LOGE("DmServiceHiChainConnector::deviceGroupManager_ is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string addParams = "";
    std::string groupOwner = "";
    if (ParseRemoteCredentialExt(credentialInfo, addParams, groupOwner) != DM_OK) {
        LOGE("AddMultiMembers ParseRemoteCredentialExt failed!");
        return ERR_DM_FAILED;
    }
    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("Get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = deviceGroupManager_->addMultiMembersToGroup(osAccountUserId, groupOwner.c_str(), addParams.c_str());
    if (ret != DM_OK) {
        LOGE("[HICHAIN]fail to add member to hichain group with ret:%{public}d.", ret);
        return ret;
    }
    return DM_OK;
}

bool DmServiceHiChainConnector::IsRedundanceGroup(const std::string &userId,
    int32_t authType, std::vector<DmGroupInfo> &groupList)
{
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = authType;
    std::string queryParams = jsonObj.Dump();

    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return false;
    }
    if (!GetGroupInfo(osAccountUserId, queryParams, groupList)) {
        return false;
    }
    for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
        if (iter->userId != userId) {
            return true;
        }
    }
    return false;
}

int32_t DmServiceHiChainConnector::DeleteGroup(std::string &groupId)
{
    int64_t requestId = GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = jsonObj.Dump();
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_SERVICE, disbandParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete group with ret:%{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

void DmServiceHiChainConnector::DeleteRedundanceGroup(std::string &userId)
{
    int32_t nTickTimes = 0;
    g_deleteGroupFlag = false;
    DeleteGroup(userId);
    while (!g_deleteGroupFlag) {
        usleep(DELAY_TIME_MS);
        if (++nTickTimes > SERVICE_INIT_TRY_MAX_NUM) {
            LOGE("failed to delete group because timeout!");
            return;
        }
    }
}

void DmServiceHiChainConnector::DealRedundanceGroup(const std::string &userId, int32_t authType)
{
    g_groupIsRedundance = false;
    std::vector<DmGroupInfo> groupList;
    if (IsRedundanceGroup(userId, authType, groupList)) {
        LOGI("DmServiceHiChainConnector::CreateGroup IsRedundanceGroup");
        g_groupIsRedundance = true;
        for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
            if (iter->userId != userId) {
                DeleteRedundanceGroup(iter->userId);
            }
        }
        g_groupIsRedundance = false;
    }
}

int32_t DmServiceHiChainConnector::CreateGroup(int64_t requestId, int32_t authType, const std::string &userId,
    JsonObject &jsonOutObj)
{
    LOGI("DmServiceHiChainConnector::CreateGroup start.");
    if (deviceGroupManager_ == nullptr) {
        LOGE("DmServiceHiChainConnector::CreateGroup group manager is null, requestId %{public}" PRId64, requestId);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DealRedundanceGroup(userId, authType);
    networkStyle_ = CREDENTIAL_NETWORK;
    LOGI("DmServiceHiChainConnector::CreateGroup requestId %{public}" PRId64, requestId);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string sLocalDeviceId = localDeviceId;
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = authType;
    jsonObj[FIELD_USER_ID] = userId;
    jsonObj.Insert(FIELD_CREDENTIAL, jsonOutObj);
    jsonObj[FIELD_DEVICE_ID] = sLocalDeviceId;
    jsonObj[FIELD_USER_TYPE] = 0;
    jsonObj[FIELD_GROUP_VISIBILITY] = GROUP_VISIBILITY_PUBLIC;
    jsonObj[FIELD_EXPIRE_TIME] = FIELD_EXPIRE_TIME_VALUE;
    g_createGroupFlag = false;
    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = deviceGroupManager_->createGroup(osAccountUserId, requestId, DM_SERVICE,
        jsonObj.Dump().c_str());
    if (ret != DM_OK) {
        LOGE("[HICHAIN]fail to create group with ret:%{public}d, requestId:%{public}" PRId64, ret, requestId);
        return ERR_DM_CREATE_GROUP_FAILED;
    }
    int32_t nTickTimes = 0;
    while (!g_createGroupFlag) {
        usleep(DELAY_TIME_MS);
        if (++nTickTimes > SERVICE_INIT_TRY_MAX_NUM) {
            LOGE("failed to create group because timeout!");
            return ERR_DM_CREATE_GROUP_FAILED;
        }
    }
    return DM_OK;
}

int32_t DmServiceHiChainConnector::DeleteGroup(int64_t requestId_, const std::string &userId, const int32_t authType)
{
    networkStyle_ = CREDENTIAL_NETWORK;
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = authType;
    std::string queryParams = jsonObj.Dump();
    std::vector<DmGroupInfo> groupList;
    if (!GetGroupInfo(queryParams, groupList)) {
        LOGE("failed to get device join groups");
        return ERR_DM_FAILED;
    }
    LOGI("DmServiceHiChainConnector::DeleteGroup groupList count = %{public}zu", groupList.size());
    bool userIsExist = false;
    std::string groupId = "";
    for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
        if (iter->userId == userId) {
            userIsExist = true;
            groupId = iter->groupId;
            break;
        }
    }
    if (!userIsExist) {
        LOGE("input userId is exist in groupList!");
        return ERR_DM_FAILED;
    }
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = jsonObj.Dump();
    g_deleteGroupFlag = false;
    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = deviceGroupManager_->deleteGroup(osAccountUserId, requestId_, DM_SERVICE,
        disbandParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete hichain group with ret:%{public}d.", ret);
        return ERR_DM_FAILED;
    }
    int32_t nTickTimes = 0;
    while (!g_deleteGroupFlag) {
        usleep(DELAY_TIME_MS);
        if (++nTickTimes > SERVICE_INIT_TRY_MAX_NUM) {
            LOGE("failed to delete group because timeout!");
            return ERR_DM_FAILED;
        }
    }
    return DM_OK;
}

int32_t DmServiceHiChainConnector::RegisterHiChainGroupCallback(
    const std::shared_ptr<IDmServiceGroupResCallback> &callback)
{
    hiChainResCallback_ = callback;
    return DM_OK;
}

int32_t DmServiceHiChainConnector::UnRegisterHiChainGroupCallback()
{
    hiChainResCallback_ = nullptr;
    return DM_OK;
}

int32_t DmServiceHiChainConnector::GetGroupId(const std::string &userId,
    const int32_t groupType, std::string &groupId)
{
    JsonObject jsonObjGroup;
    jsonObjGroup[FIELD_GROUP_TYPE] = groupType;
    std::string queryParams = jsonObjGroup.Dump();
    std::vector<DmGroupInfo> groupList;

    if (!GetGroupInfo(queryParams.c_str(), groupList)) {
        LOGE("failed to get device join groups");
        return ERR_DM_FAILED;
    }
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
        LOGE("DmServiceHiChainConnector::deviceGroupManager_ is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string addParams;
    int32_t osAccountUserId = 0;
    if (ParseRemoteCredential(groupType, userId, jsonDeviceList, addParams, osAccountUserId) != DM_OK) {
        LOGE("addMultiMembers ParseRemoteCredential failed!");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->addMultiMembersToGroup(osAccountUserId, DM_SERVICE, addParams.c_str());
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
        LOGE("DmServiceHiChainConnector::deviceGroupManager_ is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::string deleteParams;
    int32_t osAccountUserId = 0;
    if (ParseRemoteCredential(groupType, userId, jsonDeviceList, deleteParams, osAccountUserId) != DM_OK) {
        LOGE("deleteMultiMembers ParseRemoteCredential failed!");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->delMultiMembersFromGroup(osAccountUserId, DM_SERVICE, deleteParams.c_str());
    if (ret != DM_OK) {
        LOGE("[HICHAIN]fail to delete member from hichain group with ret:%{public}d.", ret);
        return ret;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
