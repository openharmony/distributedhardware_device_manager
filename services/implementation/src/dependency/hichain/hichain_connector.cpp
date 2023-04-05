/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "hichain_connector.h"

#include <cstdlib>
#include <ctime>
#include <functional>
#include <securec.h>

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_dfx_constants.h"
#include "dm_hisysevent.h"
#include "dm_log.h"
#include "dm_random.h"
#include "hichain_connector_callback.h"
#include "multiple_user_connector.h"
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "unistd.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t PIN_CODE_NETWORK = 0;
const int32_t CREDENTIAL_NETWORK = 1;
const int32_t DELAY_TIME_MS = 10000; // 10ms
const int32_t FIELD_EXPIRE_TIME_VALUE = 7;

constexpr const char* DEVICE_ID = "DEVICE_ID";
constexpr const char* FIELD_CREDENTIAL = "credential";

void from_json(const nlohmann::json &jsonObject, GroupInfo &groupInfo)
{
    if (jsonObject.find(FIELD_GROUP_NAME) != jsonObject.end()) {
        groupInfo.groupName = jsonObject.at(FIELD_GROUP_NAME).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_ID) != jsonObject.end()) {
        groupInfo.groupId = jsonObject.at(FIELD_GROUP_ID).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_OWNER) != jsonObject.end()) {
        groupInfo.groupOwner = jsonObject.at(FIELD_GROUP_OWNER).get<std::string>();
    }

    if (jsonObject.find(FIELD_GROUP_TYPE) != jsonObject.end()) {
        groupInfo.groupType = jsonObject.at(FIELD_GROUP_TYPE).get<int32_t>();
    }

    if (jsonObject.find(FIELD_GROUP_VISIBILITY) != jsonObject.end()) {
        groupInfo.groupVisibility = jsonObject.at(FIELD_GROUP_VISIBILITY).get<int32_t>();
    }

    if (jsonObject.find(FIELD_USER_ID) != jsonObject.end()) {
        groupInfo.userId = jsonObject.at(FIELD_USER_ID).get<std::string>();
    }
}

std::shared_ptr<IHiChainConnectorCallback> HiChainConnector::hiChainConnectorCallback_ = nullptr;
std::shared_ptr<IDmGroupResCallback> HiChainConnector::hiChainResCallback_ = nullptr;
int32_t HiChainConnector::networkStyle_ = PIN_CODE_NETWORK;
bool g_createGroupFlag = false;
bool g_deleteGroupFlag = false;
bool g_groupIsRedundance = false;

HiChainConnector::HiChainConnector()
{
    LOGI("HiChainConnector::constructor");
    deviceAuthCallback_ = {.onTransmit = nullptr,
                           .onSessionKeyReturned = nullptr,
                           .onFinish = HiChainConnector::onFinish,
                           .onError = HiChainConnector::onError,
                           .onRequest = HiChainConnector::onRequest};
    InitDeviceAuthService();
    deviceGroupManager_ = GetGmInstance();
    if (deviceGroupManager_ == nullptr) {
        LOGI("HiChainConnector::constructor, failed to init group manager!");
        return;
    }
    deviceGroupManager_->regCallback(DM_PKG_NAME, &deviceAuthCallback_);
    LOGI("HiChainConnector::constructor success.");
}

HiChainConnector::~HiChainConnector()
{
    LOGI("HiChainConnector::destructor.");
}

int32_t HiChainConnector::RegisterHiChainCallback(std::shared_ptr<IHiChainConnectorCallback> callback)
{
    hiChainConnectorCallback_ = callback;
    return DM_OK;
}

int32_t HiChainConnector::UnRegisterHiChainCallback()
{
    hiChainConnectorCallback_ = nullptr;
    return DM_OK;
}

int32_t HiChainConnector::CreateGroup(int64_t requestId, const std::string &groupName)
{
    if (deviceGroupManager_ == nullptr) {
        LOGE("HiChainConnector::CreateGroup group manager is null, requestId %lld.", requestId);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    networkStyle_ = PIN_CODE_NETWORK;
    GroupInfo groupInfo;
    if (IsGroupCreated(groupName, groupInfo)) {
        DeleteGroup(groupInfo.groupId);
    }
    LOGI("HiChainConnector::CreateGroup requestId %lld", requestId);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string sLocalDeviceId = localDeviceId;
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_PEER_TO_PEER_GROUP;
    jsonObj[FIELD_DEVICE_ID] = sLocalDeviceId;
    jsonObj[FIELD_GROUP_NAME] = groupName;
    jsonObj[FIELD_USER_TYPE] = 0;
    jsonObj[FIELD_GROUP_VISIBILITY] = GROUP_VISIBILITY_PUBLIC;
    jsonObj[FIELD_EXPIRE_TIME] = FIELD_EXPIRE_TIME_VALUE;
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->createGroup(userId, requestId, DM_PKG_NAME, jsonObj.dump().c_str());
    if (ret != 0) {
        LOGE("Failed to start CreateGroup task, ret: %d, requestId %lld.", ret, requestId);
        return ERR_DM_CREATE_GROUP_FAILED;
    }
    return DM_OK;
}

bool HiChainConnector::IsGroupCreated(std::string groupName, GroupInfo &groupInfo)
{
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_NAME] = groupName.c_str();
    std::string queryParams = jsonObj.dump();
    std::vector<GroupInfo> groupList;
    if (GetGroupInfo(queryParams, groupList)) {
        groupInfo = groupList[0];
        return true;
    }
    return false;
}

bool HiChainConnector::IsRedundanceGroup(const std::string &userId, int32_t authType, std::vector<GroupInfo> &groupList)
{
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = authType;
    std::string queryParams = jsonObj.dump();

    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
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

bool HiChainConnector::GetGroupInfo(const std::string &queryParams, std::vector<GroupInfo> &groupList)
{
    char *groupVec = nullptr;
    uint32_t num = 0;
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return false;
    }
    int32_t ret = deviceGroupManager_->getGroupInfo(userId, DM_PKG_NAME, queryParams.c_str(), &groupVec, &num);
    if (ret != 0) {
        LOGE("HiChainConnector::GetGroupInfo failed, ret: %d.", ret);
        return false;
    }
    if (groupVec == nullptr) {
        LOGE("HiChainConnector::GetGroupInfo failed, returnGroups is nullptr");
        return false;
    }
    if (num == 0) {
        LOGE("HiChainConnector::GetGroupInfo group failed, groupNum is 0.");
        return false;
    }
    LOGI("HiChainConnector::GetGroupInfo group(%s), groupNum(%u)", groupVec, num);
    std::string relatedGroups = std::string(groupVec);
    deviceGroupManager_->destroyInfo(&groupVec);
    nlohmann::json jsonObject = nlohmann::json::parse(relatedGroups);
    if (jsonObject.is_discarded()) {
        LOGE("returnGroups parse error");
        return false;
    }
    if (!jsonObject.is_array()) {
        LOGE("json string is not array.");
        return false;
    }
    std::vector<GroupInfo> groupInfos = jsonObject.get<std::vector<GroupInfo>>();
    if (groupInfos.size() == 0) {
        LOGE("HiChainConnector::GetGroupInfo group failed, groupInfos is empty.");
        return false;
    }
    groupList = groupInfos;
    return true;
}

int32_t HiChainConnector::GetGroupInfo(const int32_t userId, const std::string &queryParams,
    std::vector<GroupInfo> &groupList)
{
    char *groupVec = nullptr;
    uint32_t num = 0;
    int32_t ret = deviceGroupManager_->getGroupInfo(userId, DM_PKG_NAME, queryParams.c_str(), &groupVec, &num);
    if (ret != 0) {
        LOGE("HiChainConnector::GetGroupInfo failed, ret: %d.", ret);
        return false;
    }
    if (groupVec == nullptr) {
        LOGE("HiChainConnector::GetGroupInfo failed, returnGroups is nullptr.");
        return false;
    }
    if (num == 0) {
        LOGE("HiChainConnector::GetGroupInfo group failed, groupNum is 0.");
        return false;
    }
    LOGI("HiChainConnector::GetGroupInfo group(%s), groupNum(%ud)", groupVec, num);
    std::string relatedGroups = std::string(groupVec);
    deviceGroupManager_->destroyInfo(&groupVec);
    nlohmann::json jsonObject = nlohmann::json::parse(relatedGroups);
    if (jsonObject.is_discarded()) {
        LOGE("returnGroups parse error");
        return false;
    }
    if (!jsonObject.is_array()) {
        LOGE("json string is not array.");
        return false;
    }
    std::vector<GroupInfo> groupInfos = jsonObject.get<std::vector<GroupInfo>>();
    if (groupInfos.size() == 0) {
        LOGE("HiChainConnector::GetGroupInfo group failed, groupInfos is empty.");
        return false;
    }
    groupList = groupInfos;
    return true;
}

int32_t HiChainConnector::AddMember(const std::string &deviceId, const std::string &connectInfo)
{
    LOGI("HiChainConnector::AddMember");
    if (deviceGroupManager_ == nullptr) {
        LOGI("HiChainConnector::AddMember group manager is null.");
        return ERR_DM_POINT_NULL;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(connectInfo, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }
    if (!IsString(jsonObject, TAG_DEVICE_ID) || !IsInt32(jsonObject, PIN_CODE_KEY) ||
        !IsString(jsonObject, TAG_GROUP_ID) || !IsInt64(jsonObject, TAG_REQUEST_ID) ||
        !IsString(jsonObject, TAG_GROUP_NAME)) {
        LOGE("HiChainConnector::AddMember err json string.");
        return ERR_DM_FAILED;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string connectInfomation = GetConnectPara(deviceId, jsonObject[TAG_DEVICE_ID].get<std::string>());

    int32_t pinCode = jsonObject[PIN_CODE_KEY].get<int32_t>();
    std::string groupId = jsonObject[TAG_GROUP_ID].get<std::string>();
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_PEER_TO_PEER_GROUP;
    jsonObj[FIELD_PIN_CODE] = std::to_string(pinCode).c_str();
    jsonObj[FIELD_IS_ADMIN] = false;
    jsonObj[FIELD_DEVICE_ID] = localDeviceId;
    jsonObj[FIELD_GROUP_NAME] = jsonObject[TAG_GROUP_NAME].get<std::string>();
    jsonObj[FIELD_CONNECT_PARAMS] = connectInfomation.c_str();
    std::string tmpStr = jsonObj.dump();
    int64_t requestId = jsonObject[TAG_REQUEST_ID].get<int64_t>();
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = deviceGroupManager_->addMemberToGroup(userId, requestId, DM_PKG_NAME, tmpStr.c_str());
    if (ret != 0) {
        LOGE("HiChainConnector::AddMember failed, ret: %d", ret);
    }
    LOGI("HiChainConnector::AddMember completed");
    return ret;
}

void HiChainConnector::onFinish(int64_t requestId, int operationCode, const char *returnData)
{
    std::string data = "";
    if (returnData != nullptr) {
        data = std::string(returnData);
    }
    LOGI("HiChainConnector::onFinish reqId:%lld, operation:%d", requestId, operationCode);
    if (operationCode == GroupOperationCode::MEMBER_JOIN) {
        LOGI("Add Member To Group success");
        SysEventWrite(std::string(ADD_HICHAIN_GROUP_SUCCESS), DM_HISYEVENT_BEHAVIOR,
            std::string(ADD_HICHAIN_GROUP_SUCCESS_MSG));
        if (hiChainConnectorCallback_ != nullptr) {
            hiChainConnectorCallback_->OnMemberJoin(requestId, DM_OK);
        }
    }
    if (operationCode == GroupOperationCode::GROUP_CREATE) {
        LOGI("Create group success");
        SysEventWrite(std::string(DM_CREATE_GROUP_SUCCESS), DM_HISYEVENT_BEHAVIOR,
            std::string(DM_CREATE_GROUP_SUCCESS_MSG));
        if (networkStyle_ == CREDENTIAL_NETWORK) {
            if (hiChainResCallback_ != nullptr) {
                int32_t importAction = 0;
                hiChainResCallback_->OnGroupResult(requestId, importAction, data);
                g_createGroupFlag = true;
            }
        } else {
            if (hiChainConnectorCallback_ != nullptr) {
                hiChainConnectorCallback_->OnMemberJoin(requestId, DM_OK);
                hiChainConnectorCallback_->OnGroupCreated(requestId, data);
            }
        }
    }
    if (operationCode == GroupOperationCode::MEMBER_DELETE) {
        LOGI("Delete Member from group success");
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

void HiChainConnector::onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    std::string data = "";
    if (errorReturn != nullptr) {
        data = std::string(errorReturn);
    }
    LOGI("HichainAuthenCallBack::onError reqId:%lld, operation:%d, errorCode:%d.", requestId, operationCode, errorCode);
    if (operationCode == GroupOperationCode::MEMBER_JOIN) {
        LOGE("Add Member To Group failed");
        SysEventWrite(std::string(ADD_HICHAIN_GROUP_FAILED), DM_HISYEVENT_BEHAVIOR,
            std::string(ADD_HICHAIN_GROUP_FAILED_MSG));
        if (hiChainConnectorCallback_ != nullptr) {
            hiChainConnectorCallback_->OnMemberJoin(requestId, ERR_DM_FAILED);
        }
    }
    if (operationCode == GroupOperationCode::GROUP_CREATE) {
        LOGE("Create group failed");
        SysEventWrite(std::string(DM_CREATE_GROUP_FAILED), DM_HISYEVENT_BEHAVIOR,
            std::string(DM_CREATE_GROUP_FAILED_MSG));
        if (networkStyle_ == CREDENTIAL_NETWORK) {
            if (hiChainResCallback_ != nullptr) {
                int32_t importAction = 0;
                hiChainResCallback_->OnGroupResult(requestId, importAction, data);
                g_createGroupFlag = true;
            }
        } else {
            if (hiChainConnectorCallback_ != nullptr) {
                hiChainConnectorCallback_->OnGroupCreated(requestId, "{}");
            }
        }
    }
    if (operationCode == GroupOperationCode::MEMBER_DELETE) {
        LOGE("Delete Member from group failed");
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

char *HiChainConnector::onRequest(int64_t requestId, int operationCode, const char *reqParams)
{
    if (operationCode != GroupOperationCode::MEMBER_JOIN) {
        LOGE("HiChainConnector::onRequest operationCode %d", operationCode);
        return nullptr;
    }
    if (hiChainConnectorCallback_ == nullptr) {
        LOGE("HiChainConnector::onRequest hiChainConnectorCallback_ is nullptr.");
        return nullptr;
    }
    nlohmann::json jsonObj;
    int32_t pinCode = hiChainConnectorCallback_->GetPinCode();
    if (pinCode == ERR_DM_AUTH_NOT_START) {
        jsonObj[FIELD_CONFIRMATION] = REQUEST_REJECTED;
    } else {
        jsonObj[FIELD_CONFIRMATION] = REQUEST_ACCEPTED;
    }
    jsonObj[FIELD_PIN_CODE] = std::to_string(pinCode).c_str();
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    jsonObj[FIELD_DEVICE_ID] = localDeviceId;

    std::string jsonStr = jsonObj.dump();
    char *buffer = strdup(jsonStr.c_str());
    return buffer;
}

int64_t HiChainConnector::GenRequestId()
{
    return GenRandLongLong(MIN_REQUEST_ID, MAX_REQUEST_ID);
}

std::string HiChainConnector::GetConnectPara(std::string deviceId, std::string reqDeviceId)
{
    LOGI("HiChainConnector::GetConnectPara get addrInfo");
    if (hiChainConnectorCallback_ == nullptr) {
        LOGE("HiChainConnector::GetConnectPara hiChainConnectorCallback_ is nullptr.");
        return "";
    }
    std::string connectAddr = hiChainConnectorCallback_->GetConnectAddr(deviceId);
    nlohmann::json jsonObject = nlohmann::json::parse(connectAddr, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return connectAddr;
    }
    jsonObject[DEVICE_ID] = reqDeviceId;

    return jsonObject.dump();
}

int32_t HiChainConnector::GetRelatedGroups(const std::string &deviceId, std::vector<GroupInfo> &groupList)
{
    LOGI("HiChainConnector::GetRelatedGroups Start to get local related groups.");
    uint32_t groupNum = 0;
    char *returnGroups = nullptr;
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret =
        deviceGroupManager_->getRelatedGroups(userId, DM_PKG_NAME, deviceId.c_str(), &returnGroups, &groupNum);
    if (ret != 0) {
        LOGE("HiChainConnector::GetRelatedGroups failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    if (returnGroups == nullptr) {
        LOGE("HiChainConnector::GetRelatedGroups failed, returnGroups is nullptr");
        return ERR_DM_FAILED;
    }
    if (groupNum == 0) {
        LOGE("HiChainConnector::GetRelatedGroups group failed, groupNum is 0.");
        return ERR_DM_FAILED;
    }
    std::string relatedGroups = std::string(returnGroups);
    nlohmann::json jsonObject = nlohmann::json::parse(relatedGroups);
    if (jsonObject.is_discarded()) {
        LOGE("returnGroups parse error");
        return ERR_DM_FAILED;
    }
    if (!jsonObject.is_array()) {
        LOGE("jsonObject is not an array.");
        return ERR_DM_FAILED;
    }
    std::vector<GroupInfo> groupInfos = jsonObject.get<std::vector<GroupInfo>>();
    if (groupInfos.empty()) {
        LOGE("HiChainConnector::GetRelatedGroups group failed, groupInfos is empty.");
        return ERR_DM_FAILED;
    }
    groupList = groupInfos;
    return DM_OK;
}

int32_t HiChainConnector::GetSyncGroupList(std::vector<GroupInfo> &groupList, std::vector<std::string> &syncGroupList)
{
    if (groupList.empty()) {
        LOGE("groupList is empty.");
        return ERR_DM_FAILED;
    }
    for (auto group : groupList) {
        if (IsGroupInfoInvalid(group)) {
            continue;
        }
        syncGroupList.push_back(group.groupId);
    }
    return DM_OK;
}

bool HiChainConnector::IsDevicesInGroup(const std::string &hostDevice, const std::string &peerDevice)
{
    LOGI("HiChainConnector::IsDevicesInGroup");
    std::vector<GroupInfo> hostGroupInfoList;
    GetRelatedGroups(hostDevice, hostGroupInfoList);
    std::vector<GroupInfo> peerGroupInfoList;
    GetRelatedGroups(peerDevice, peerGroupInfoList);
    for (const auto &hostGroupInfo : hostGroupInfoList) {
        for (const auto &peerGroupInfo : peerGroupInfoList) {
            if (hostGroupInfo.groupId == peerGroupInfo.groupId && hostGroupInfo.groupName == peerGroupInfo.groupName) {
                LOGE("these are authenticated");
                return true;
            }
        }
    }
    return false;
}

bool HiChainConnector::IsGroupInfoInvalid(GroupInfo &group)
{
    if (group.groupType == GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP || group.groupVisibility == GROUP_VISIBILITY_PUBLIC ||
        group.groupOwner != std::string(DM_PKG_NAME)) {
        return true;
    }
    return false;
}

int32_t HiChainConnector::SyncGroups(std::string deviceId, std::vector<std::string> &remoteGroupIdList)
{
    std::vector<GroupInfo> groupInfoList;
    GetRelatedGroups(deviceId, groupInfoList);
    for (auto &groupInfo : groupInfoList) {
        if (IsGroupInfoInvalid(groupInfo)) {
            continue;
        }
        auto iter = std::find(remoteGroupIdList.begin(), remoteGroupIdList.end(), groupInfo.groupId);
        if (iter == remoteGroupIdList.end()) {
            (void)DelMemberFromGroup(groupInfo.groupId, deviceId);
        }
    }
    return DM_OK;
}

int32_t HiChainConnector::DelMemberFromGroup(const std::string &groupId, const std::string &deviceId)
{
    int64_t requestId = GenRequestId();
    LOGI("Start to delete member from group, requestId %lld, deviceId %s, groupId %s", requestId,
         GetAnonyString(deviceId).c_str(), GetAnonyString(groupId).c_str());
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    jsonObj[FIELD_DELETE_ID] = deviceId;
    std::string deleteParams = jsonObj.dump();
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = deviceGroupManager_->deleteMemberFromGroup(userId, requestId, DM_PKG_NAME, deleteParams.c_str());
    if (ret != 0) {
        LOGE("HiChainConnector::DelMemberFromGroup failed, ret: %d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t HiChainConnector::DeleteGroup(std::string &groupId)
{
    int64_t requestId = GenRequestId();
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = jsonObj.dump();
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_PKG_NAME, disbandParams.c_str());
    if (ret != 0) {
        LOGE("HiChainConnector::DeleteGroup failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainConnector::DeleteGroup(const int32_t userId, std::string &groupId)
{
    int64_t requestId = GenRequestId();
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = jsonObj.dump();
    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_PKG_NAME, disbandParams.c_str());
    if (ret != 0) {
        LOGE("HiChainConnector::DeleteGroup failed, ret: %d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainConnector::DeleteGroup(int64_t requestId_, const std::string &userId, const int32_t authType)
{
    networkStyle_ = CREDENTIAL_NETWORK;
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = authType;
    std::string queryParams = jsonObj.dump();
    std::vector<GroupInfo> groupList;
    if (!GetGroupInfo(queryParams, groupList)) {
        LOGE("failed to get device join groups");
        return ERR_DM_FAILED;
    }
    LOGI("HiChainConnector::DeleteGroup groupList count = %d", groupList.size());
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
    std::string disbandParams = jsonObj.dump();
    g_deleteGroupFlag = false;
    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = deviceGroupManager_->deleteGroup(osAccountUserId, requestId_, DM_PKG_NAME,
        disbandParams.c_str());
    if (ret != 0) {
        LOGE("HiChainConnector::DeleteGroup failed, ret: %d.", ret);
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

int32_t HiChainConnector::DeleteTimeOutGroup(const char* deviceId)
{
    LOGI("HiChainConnector::DeleteTimeOutGroup start");
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    std::vector<GroupInfo> peerGroupInfoList;
    GetRelatedGroups(deviceId, peerGroupInfoList);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    for (auto &group : peerGroupInfoList) {
        if (!(deviceGroupManager_->isDeviceInGroup(userId, DM_PKG_NAME, group.groupId.c_str(), localDeviceId))) {
            continue;
        }
        if (group.groupType == GROUP_TYPE_PEER_TO_PEER_GROUP) {
            DeleteGroup(group.groupId);
        }
    }
    return DM_OK;
}

void HiChainConnector::DeleteRedundanceGroup(std::string &userId)
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

void HiChainConnector::DealRedundanceGroup(const std::string &userId, int32_t authType)
{
    g_groupIsRedundance = false;
    std::vector<GroupInfo> groupList;
    if (IsRedundanceGroup(userId, authType, groupList)) {
        LOGI("HiChainConnector::CreateGroup IsRedundanceGroup");
        g_groupIsRedundance = true;
        for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
            if (iter->userId != userId) {
                DeleteRedundanceGroup(iter->userId);
            }
        }
        g_groupIsRedundance = false;
    }
}

int32_t HiChainConnector::CreateGroup(int64_t requestId, int32_t authType, const std::string &userId,
    nlohmann::json &jsonOutObj)
{
    if (deviceGroupManager_ == nullptr) {
        LOGE("HiChainConnector::CreateGroup group manager is null, requestId %lld.", requestId);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DealRedundanceGroup(userId, authType);
    networkStyle_ = CREDENTIAL_NETWORK;
    LOGI("HiChainConnector::CreateGroup requestId %lld", requestId);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string sLocalDeviceId = localDeviceId;
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = authType;
    jsonObj[FIELD_USER_ID] = userId;
    jsonObj[FIELD_CREDENTIAL] = jsonOutObj;
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
    LOGI("[DM] createParams:%s", jsonObj.dump().c_str());
    int32_t ret = deviceGroupManager_->createGroup(osAccountUserId, requestId, DM_PKG_NAME, jsonObj.dump().c_str());
    if (ret != DM_OK) {
        LOGE("Failed to start CreateGroup task, ret: %d, requestId %lld.", ret, requestId);
        return ERR_DM_CREATE_GROUP_FAILED;
    }
    int32_t nTickTimes = 0;
    while (!g_createGroupFlag) {
        usleep(DELAY_TIME_MS);
        if (++nTickTimes > SERVICE_INIT_TRY_MAX_NUM) {
            LOGE("failed to create group because timeout!");
            return ERR_DM_FAILED;
        }
    }
    return DM_OK;
}

int32_t HiChainConnector::RegisterHiChainGroupCallback(const std::shared_ptr<IDmGroupResCallback> &callback)
{
    hiChainResCallback_ = callback;
    return DM_OK;
}

int32_t HiChainConnector::UnRegisterHiChainGroupCallback()
{
    hiChainResCallback_ = nullptr;
    return DM_OK;
}

int32_t HiChainConnector::getRegisterInfo(const std::string &queryParams, std::string &returnJsonStr)
{
    if (deviceGroupManager_ == nullptr) {
        LOGE("HiChainConnector::deviceGroupManager_ is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    char *credentialInfo = nullptr;
    if (deviceGroupManager_->getRegisterInfo(queryParams.c_str(), &credentialInfo) != DM_OK) {
        LOGE("failed to request hichain registerinfo.");
        return ERR_DM_FAILED;
    }

    returnJsonStr = credentialInfo;
    deviceGroupManager_->destroyInfo(&credentialInfo);
    LOGI("request hichain device registerinfo successfully.");
    return DM_OK;
}

int32_t HiChainConnector::GetGroupId(const std::string &userId, const int32_t groupType, std::string &groupId)
{
    nlohmann::json jsonObjGroup;
    jsonObjGroup[FIELD_GROUP_TYPE] = groupType;
    std::string queryParams = jsonObjGroup.dump();
    std::vector<GroupInfo> groupList;

    if (!GetGroupInfo(queryParams.c_str(), groupList)) {
        LOGE("failed to get device join groups");
        return ERR_DM_FAILED;
    }
    for (auto &groupinfo : groupList) {
        LOGI("groupinfo.groupId:%s", groupinfo.groupId.c_str());
        if (groupinfo.userId == userId) {
            groupId = groupinfo.groupId;
            return DM_OK;
        }
    }
    return ERR_DM_FAILED;
}

int32_t HiChainConnector::ParseRemoteCredential(const int32_t groupType, const std::string &userId,
    const nlohmann::json &jsonDeviceList, std::string &params, int32_t &osAccountUserId)
{
    if (userId.empty() || !jsonDeviceList.contains(FIELD_DEVICE_LIST)) {
        LOGE("userId or deviceList is empty");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string groupId;
    if (GetGroupId(userId, groupType, groupId) != DM_OK) {
        LOGE("failed to get groupid");
        return ERR_DM_FAILED;
    }
    nlohmann::json jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    jsonObj[FIELD_GROUP_TYPE] = groupType;
    jsonObj[FIELD_DEVICE_LIST] = jsonDeviceList[FIELD_DEVICE_LIST];
    params = jsonObj.dump();
    osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainConnector::addMultiMembers(const int32_t groupType, const std::string &userId,
    const nlohmann::json &jsonDeviceList)
{
    if (deviceGroupManager_ == nullptr) {
        LOGE("HiChainConnector::deviceGroupManager_ is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string addParams;
    int32_t osAccountUserId = 0;
    if (ParseRemoteCredential(groupType, userId, jsonDeviceList, addParams, osAccountUserId) != DM_OK) {
        LOGE("addMultiMembers ParseRemoteCredential failed!");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->addMultiMembersToGroup(osAccountUserId, DM_PKG_NAME, addParams.c_str());
    if (ret!= DM_OK) {
        LOGE("HiChainConnector::addMultiMemberstoGroup failure! ret = %d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t HiChainConnector::deleteMultiMembers(const int32_t groupType, const std::string &userId,
    const nlohmann::json &jsonDeviceList)
{
    if (deviceGroupManager_ == nullptr) {
        LOGE("HiChainConnector::deviceGroupManager_ is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    std::string deleteParams;
    int32_t osAccountUserId = 0;
    if (ParseRemoteCredential(groupType, userId, jsonDeviceList, deleteParams, osAccountUserId) != DM_OK) {
        LOGE("deleteMultiMembers ParseRemoteCredential failed!");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->delMultiMembersFromGroup(osAccountUserId, DM_PKG_NAME, deleteParams.c_str());
    if (ret != DM_OK) {
        LOGE("HiChainConnector::deleteMultiMembers failure!, ret = %d", ret);
        return ret;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS