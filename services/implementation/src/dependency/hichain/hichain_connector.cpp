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
#include "dm_radar_helper.h"
#include "hichain_connector_callback.h"
#include "multiple_user_connector.h"
#include "json_object.h"
#include "parameter.h"
#include "unistd.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t PIN_CODE_NETWORK = 0;
const int32_t CREDENTIAL_NETWORK = 1;
const int32_t DELAY_TIME_MS = 10000; // 10ms
const int32_t FIELD_EXPIRE_TIME_VALUE = 7;
const int32_t SAME_ACCOUNT = 1;
const int32_t DEVICE_ID_HALF = 2;

constexpr const char* DEVICE_ID = "DEVICE_ID";
constexpr const char* FIELD_CREDENTIAL = "credential";
constexpr const char* ADD_HICHAIN_GROUP_SUCCESS = "ADD_HICHAIN_GROUP_SUCCESS";
constexpr const char* ADD_HICHAIN_GROUP_FAILED = "ADD_HICHAIN_GROUP_FAILED";
constexpr const char* DM_CREATE_GROUP_SUCCESS = "DM_CREATE_GROUP_SUCCESS";
constexpr const char* DM_CREATE_GROUP_FAILED = "DM_CREATE_GROUP_FAILED";
constexpr const char* ADD_HICHAIN_GROUP_SUCCESS_MSG = "dm add member to group success.";
constexpr const char* ADD_HICHAIN_GROUP_FAILED_MSG = "dm add member to group failed.";
constexpr const char* DM_CREATE_GROUP_SUCCESS_MSG = "dm create group success.";
constexpr const char* DM_CREATE_GROUP_FAILED_MSG = "dm create group failed.";
constexpr const char* DM_PKG_NAME_EXT = "com.huawei.devicemanager";
constexpr const char* FIELD_OPERATION_CODE = "operationCode";
constexpr const char* FIELD_META_NODE_TYPE = "metaNodeType";
constexpr const char* FIELD_TYPE = "TType";

static const std::unordered_map<int32_t, AuthFormPriority> g_authFormPriorityMap = {
    {GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP, AuthFormPriority::PRIORITY_IDENTICAL_ACCOUNT},
    {GROUP_TYPE_ACROSS_ACCOUNT_GROUP, AuthFormPriority::PRIORITY_ACROSS_ACCOUNT},
    {GROUP_TYPE_PEER_TO_PEER_GROUP, AuthFormPriority::PRIORITY_PEER_TO_PEER}
};

void FromJson(const JsonItemObject &jsonObject, GroupInfo &groupInfo)
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
    deviceGroupManager_ = GetGmInstance();
    if (deviceGroupManager_ == nullptr) {
        LOGE("[HICHAIN]failed to init group manager.");
        return;
    }
    int32_t ret = deviceGroupManager_->regCallback(DM_PKG_NAME, &deviceAuthCallback_);
    if (ret != HC_SUCCESS) {
        LOGE("[HICHAIN]fail to register callback to hachain with ret:%{public}d.", ret);
        return;
    }
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
        LOGE("HiChainConnector::CreateGroup group manager is null, requestId %{public}" PRId64, requestId);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    networkStyle_ = PIN_CODE_NETWORK;
    GroupInfo groupInfo;
    if (IsGroupCreated(groupName, groupInfo)) {
        DeleteGroup(groupInfo.groupId);
    }
    LOGI("HiChainConnector::CreateGroup requestId %{public}" PRId64, requestId);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string sLocalDeviceId = localDeviceId;
    JsonObject jsonObj;
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
    int32_t ret = deviceGroupManager_->createGroup(userId, requestId, DM_PKG_NAME, SafetyDump(jsonObj).c_str());
    struct RadarInfo info = {
        .funcName = "CreateGroup",
        .toCallPkg = HICHAINNAME,
        .stageRes = (ret != 0) ?
            static_cast<int32_t>(StageRes::STAGE_FAIL) : static_cast<int32_t>(StageRes::STAGE_IDLE),
        .bizState = (ret != 0) ?
            static_cast<int32_t>(BizState::BIZ_STATE_END) : static_cast<int32_t>(BizState::BIZ_STATE_START),
        .errCode = DmRadarHelper::GetInstance().GetErrCode(ERR_DM_CREATE_GROUP_FAILED),
    };
    if (!DmRadarHelper::GetInstance().ReportAuthCreateGroup(info)) {
        LOGE("ReportAuthCreateGroup failed");
    }
    if (ret != 0) {
        LOGE("[HICHAIN]fail to create group with ret:%{public}d, requestId:%{public}" PRId64, ret, requestId);
        return ERR_DM_CREATE_GROUP_FAILED;
    }
    return DM_OK;
}

bool HiChainConnector::IsGroupCreated(std::string groupName, GroupInfo &groupInfo)
{
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_NAME] = groupName.c_str();
    std::string queryParams = SafetyDump(jsonObj);
    std::vector<GroupInfo> groupList;
    if (GetGroupInfo(queryParams, groupList)) {
        groupInfo = groupList[0];
        return true;
    }
    return false;
}

bool HiChainConnector::IsRedundanceGroup(const std::string &userId, int32_t authType, std::vector<GroupInfo> &groupList)
{
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = authType;
    std::string queryParams = SafetyDump(jsonObj);

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
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return false;
    }
    return GetGroupInfo(userId, queryParams, groupList);
}

bool HiChainConnector::GetGroupInfo(const int32_t userId, const std::string &queryParams,
    std::vector<GroupInfo> &groupList)
{
    return GetGroupInfoCommon(userId, queryParams, DM_PKG_NAME, groupList);
}

bool HiChainConnector::GetGroupInfoExt(const int32_t userId, const std::string &queryParams,
    std::vector<GroupInfo> &groupList)
{
    return GetGroupInfoCommon(userId, queryParams, DM_PKG_NAME_EXT, groupList);
}

bool HiChainConnector::GetGroupInfoCommon(const int32_t userId, const std::string &queryParams, const char* pkgName,
    std::vector<GroupInfo> &groupList)
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
    LOGI("HiChainConnector::GetGroupInfo groupNum(%{public}u)", num);
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
    std::vector<GroupInfo> groupInfos;
    jsonObject.Get(groupInfos);
    if (groupInfos.empty()) {
        LOGE("HiChainConnector::GetGroupInfo group failed, groupInfos is empty.");
        return false;
    }
    groupList = groupInfos;
    return true;
}

DmAuthForm HiChainConnector::GetGroupType(const std::string &deviceId)
{
    std::vector<OHOS::DistributedHardware::GroupInfo> groupList;
    int32_t ret = GetRelatedGroups(deviceId, groupList);
    if (ret != DM_OK) {
        LOGE("HiChainConnector::GetGroupType get related groups failed");
        return DmAuthForm::INVALID_TYPE;
    }

    if (groupList.size() == 0) {
        LOGE("HiChainConnector::GetGroupType group list is empty");
        return DmAuthForm::INVALID_TYPE;
    }

    AuthFormPriority highestPriority = AuthFormPriority::PRIORITY_PEER_TO_PEER;
    for (auto it = groupList.begin(); it != groupList.end(); ++it) {
        if (g_authFormPriorityMap.count(it->groupType) == 0) {
            LOGE("HiChainConnector::GetGroupType unsupported auth form");
            return DmAuthForm::INVALID_TYPE;
        }
        AuthFormPriority priority = g_authFormPriorityMap.at(it->groupType);
        if (priority > highestPriority) {
            highestPriority = priority;
        }
    }

    if (highestPriority == AuthFormPriority::PRIORITY_IDENTICAL_ACCOUNT) {
        return DmAuthForm::IDENTICAL_ACCOUNT;
    } else if (highestPriority == AuthFormPriority::PRIORITY_ACROSS_ACCOUNT) {
        return DmAuthForm::ACROSS_ACCOUNT;
    } else if (highestPriority == AuthFormPriority::PRIORITY_PEER_TO_PEER) {
        return DmAuthForm::PEER_TO_PEER;
    }

    return DmAuthForm::INVALID_TYPE;
}

int32_t HiChainConnector::AddMember(const std::string &deviceId, const std::string &connectInfo)
{
    LOGI("HiChainConnector::AddMember");
    if (deviceGroupManager_ == nullptr) {
        LOGI("HiChainConnector::AddMember group manager is null.");
        return ERR_DM_POINT_NULL;
    }
    JsonObject jsonObject(connectInfo);
    if (jsonObject.IsDiscarded()) {
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
    std::string connectInfomation = GetConnectPara(deviceId, jsonObject[TAG_DEVICE_ID].Get<std::string>());

    std::string pinCode = jsonObject[PIN_CODE_KEY].Get<std::string>();
    std::string groupId = jsonObject[TAG_GROUP_ID].Get<std::string>();
    LOGI("pinCode: %{public}s", GetAnonyString(pinCode).c_str());
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_PEER_TO_PEER_GROUP;
    jsonObj[FIELD_PIN_CODE] = pinCode;
    jsonObj[FIELD_IS_ADMIN] = false;
    jsonObj[FIELD_DEVICE_ID] = localDeviceId;
    jsonObj[FIELD_GROUP_NAME] = jsonObject[TAG_GROUP_NAME].Get<std::string>();
    jsonObj[FIELD_CONNECT_PARAMS] = connectInfomation.c_str();
    std::string tmpStr = SafetyDump(jsonObj);
    int64_t requestId = jsonObject[TAG_REQUEST_ID].Get<int64_t>();
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = deviceGroupManager_->addMemberToGroup(userId, requestId, DM_PKG_NAME, tmpStr.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to add number to hichain group with ret:%{public}d.", ret);
    }
    LOGI("HiChainConnector::AddMember completed");
    return ret;
}

void HiChainConnector::onFinish(int64_t requestId, int operationCode, const char *returnData)
{
    std::string data = (returnData != nullptr) ? std::string(returnData) : "";
    LOGI("HiChainConnector::onFinish reqId:%{public}" PRId64 ", operation:%{public}d", requestId, operationCode);
    if (operationCode == GroupOperationCode::MEMBER_JOIN) {
        LOGI("Add Member To Group success");
        if (!DmRadarHelper::GetInstance().ReportAuthAddGroupCb(
            "onFinish", static_cast<int32_t>(StageRes::STAGE_SUCC))) {
            LOGE("ReportAuthAddGroupCb failed");
        }
        SysEventWrite(std::string(ADD_HICHAIN_GROUP_SUCCESS), DM_HISYEVENT_BEHAVIOR,
            std::string(ADD_HICHAIN_GROUP_SUCCESS_MSG));
        if (hiChainConnectorCallback_ != nullptr) {
            hiChainConnectorCallback_->OnMemberJoin(requestId, DM_OK);
        }
    }
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
    std::string data = (errorReturn != nullptr) ? std::string(errorReturn) : "";
    LOGI("HichainAuthenCallBack::onError reqId:%{public}" PRId64 ", operation:%{public}d, errorCode:%{public}d.",
        requestId, operationCode, errorCode);
    if (operationCode == GroupOperationCode::MEMBER_JOIN) {
        LOGE("Add Member To Group failed");
        if (!DmRadarHelper::GetInstance().ReportAuthAddGroupCb(
            "onError", static_cast<int32_t>(StageRes::STAGE_FAIL))) {
            LOGE("ReportAuthAddGroupCb failed");
        }
        SysEventWrite(std::string(ADD_HICHAIN_GROUP_FAILED), DM_HISYEVENT_BEHAVIOR,
            std::string(ADD_HICHAIN_GROUP_FAILED_MSG));
        if (hiChainConnectorCallback_ != nullptr) {
            hiChainConnectorCallback_->OnMemberJoin(requestId, ERR_DM_ADD_GROUP_FAILED);
        }
    }
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
    (void)requestId;
    (void)reqParams;
    if (operationCode != GroupOperationCode::MEMBER_JOIN) {
        LOGE("HiChainConnector::onRequest operationCode %{public}d", operationCode);
        return nullptr;
    }
    if (hiChainConnectorCallback_ == nullptr) {
        LOGE("HiChainConnector::onRequest hiChainConnectorCallback_ is nullptr.");
        return nullptr;
    }
    JsonObject jsonObj;
    std::string pinCode = "";
    if (hiChainConnectorCallback_->GetPinCode(pinCode) == ERR_DM_FAILED || pinCode == "") {
        jsonObj[FIELD_CONFIRMATION] = REQUEST_REJECTED;
    } else {
        jsonObj[FIELD_CONFIRMATION] = REQUEST_ACCEPTED;
        jsonObj[FIELD_PIN_CODE] = pinCode;
    }
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    jsonObj[FIELD_DEVICE_ID] = localDeviceId;

    std::string jsonStr = SafetyDump(jsonObj);
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
    JsonObject jsonObject(connectAddr);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return connectAddr;
    }
    jsonObject[DEVICE_ID] = reqDeviceId;

    return SafetyDump(jsonObject);
}

int32_t HiChainConnector::GetRelatedGroups(const std::string &deviceId, std::vector<GroupInfo> &groupList)
{
    return GetRelatedGroupsCommon(deviceId, DM_PKG_NAME, groupList);
}

int32_t HiChainConnector::GetRelatedGroups(int32_t userId, const std::string &deviceId,
    std::vector<GroupInfo> &groupList)
{
    return GetRelatedGroupsCommon(userId, deviceId, DM_PKG_NAME, groupList);
}

int32_t HiChainConnector::GetRelatedGroupsExt(const std::string &deviceId, std::vector<GroupInfo> &groupList)
{
    return GetRelatedGroupsCommon(deviceId, DM_PKG_NAME_EXT, groupList);
}

int32_t HiChainConnector::GetRelatedGroupsExt(int32_t userId, const std::string &deviceId,
    std::vector<GroupInfo> &groupList)
{
    return GetRelatedGroupsCommon(userId, deviceId, DM_PKG_NAME_EXT, groupList);
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

bool HiChainConnector::IsDevicesInP2PGroup(const std::string &hostDevice, const std::string &peerDevice)
{
    LOGI("HiChainConnector::IsDevicesInP2PGroup");
    if (hostDevice == peerDevice || peerDevice == "" || hostDevice == "") {
        LOGE("invalid param");
        return false;
    }
    std::vector<GroupInfo> hostGroupInfoList;
    GetRelatedGroups(hostDevice, hostGroupInfoList);
    std::vector<GroupInfo> peerGroupInfoList;
    GetRelatedGroups(peerDevice, peerGroupInfoList);
    for (const auto &hostGroupInfo : hostGroupInfoList) {
        if (hostGroupInfo.groupType != GROUP_TYPE_PEER_TO_PEER_GROUP) {
            continue;
        }
        for (const auto &peerGroupInfo : peerGroupInfoList) {
            if (peerGroupInfo.groupType != GROUP_TYPE_PEER_TO_PEER_GROUP) {
                continue;
            }
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
    LOGI("Start to delete member from group, requestId %{public}" PRId64", deviceId %{public}s, groupId %{public}s",
        requestId, GetAnonyString(deviceId).c_str(), GetAnonyString(groupId).c_str());
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    jsonObj[FIELD_DELETE_ID] = deviceId;
    std::string deleteParams = SafetyDump(jsonObj);
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = deviceGroupManager_->deleteMemberFromGroup(userId, requestId, DM_PKG_NAME, deleteParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete member from group with ret:%{public}d.", ret);
        return ret;
    }
    return DM_OK;
}

int32_t HiChainConnector::DeleteGroup(std::string &groupId)
{
    int64_t requestId = GenRequestId();
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = SafetyDump(jsonObj);
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_PKG_NAME, disbandParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete group with ret:%{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainConnector::DeleteGroupExt(std::string &groupId)
{
    int64_t requestId = GenRequestId();
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = SafetyDump(jsonObj);
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }

    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_PKG_NAME_EXT, disbandParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete group with ret:%{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainConnector::DeleteGroupExt(int32_t userId, std::string &groupId)
{
    int64_t requestId = GenRequestId();
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = SafetyDump(jsonObj);
    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_PKG_NAME_EXT, disbandParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete group with ret:%{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainConnector::DeleteGroup(int64_t requestId_, const std::string &userId, const int32_t authType)
{
    networkStyle_ = CREDENTIAL_NETWORK;
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = authType;
    std::string queryParams = SafetyDump(jsonObj);
    std::vector<GroupInfo> groupList;
    if (!GetGroupInfo(queryParams, groupList)) {
        LOGE("failed to get device join groups");
        return ERR_DM_FAILED;
    }
    LOGI("HiChainConnector::DeleteGroup groupList count = %{public}zu", groupList.size());
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
    std::string disbandParams = SafetyDump(jsonObj);
    g_deleteGroupFlag = false;
    int32_t osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = deviceGroupManager_->deleteGroup(osAccountUserId, requestId_, DM_PKG_NAME,
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
        if ((!group.groupName.empty()) && (group.groupName[CHECK_AUTH_ALWAYS_POS] == AUTH_ALWAYS)) {
            LOGI("HiChainConnector::DeleteTimeOutGroup always trusted group");
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
    JsonObject &jsonOutObj)
{
    LOGI("HiChainConnector::CreateGroup start.");
    if (deviceGroupManager_ == nullptr) {
        LOGE("HiChainConnector::CreateGroup group manager is null, requestId %{public}" PRId64, requestId);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    DealRedundanceGroup(userId, authType);
    networkStyle_ = CREDENTIAL_NETWORK;
    LOGI("HiChainConnector::CreateGroup requestId %{public}" PRId64, requestId);
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
    int32_t ret = deviceGroupManager_->createGroup(osAccountUserId, requestId, DM_PKG_NAME,
        SafetyDump(jsonObj).c_str());
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
        LOGE("[HICHAIN]fail to request hichain registerinfo.");
        deviceGroupManager_->destroyInfo(&credentialInfo);
        return ERR_DM_FAILED;
    }

    returnJsonStr = credentialInfo;
    deviceGroupManager_->destroyInfo(&credentialInfo);
    LOGI("request hichain device registerinfo successfully.");
    return DM_OK;
}

int32_t HiChainConnector::GetGroupId(const std::string &userId, const int32_t groupType, std::string &groupId)
{
    JsonObject jsonObjGroup;
    jsonObjGroup[FIELD_GROUP_TYPE] = groupType;
    std::string queryParams = SafetyDump(jsonObjGroup);
    std::vector<GroupInfo> groupList;

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

int32_t HiChainConnector::ParseRemoteCredential(const int32_t groupType, const std::string &userId,
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
    params = SafetyDump(jsonObj);
    osAccountUserId = MultipleUserConnector::GetCurrentAccountUserID();
    if (osAccountUserId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainConnector::addMultiMembers(const int32_t groupType, const std::string &userId,
    const JsonObject &jsonDeviceList)
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
    if (ret != DM_OK) {
        LOGE("[HICHAIN]fail to add member to hichain group with ret:%{public}d.", ret);
        return ERR_DM_ADD_GROUP_FAILED;
    }
    return DM_OK;
}

std::string HiChainConnector::GetJsonStr(const JsonObject &jsonObj, const std::string &key)
{
    if (!IsString(jsonObj, key)) {
        LOGE("User string key not exist!");
        return "";
    }
    return jsonObj[key].Get<std::string>();
}

int32_t HiChainConnector::GetJsonInt(const JsonObject &jsonObj, const std::string &key)
{
    if (!IsInt32(jsonObj, key)) {
        LOGE("User string key not exist!");
        return ERR_DM_FAILED;
    }
    return jsonObj[key].Get<int32_t>();
}

int32_t HiChainConnector::GetGroupIdExt(const std::string &userId, const int32_t groupType,
    std::string &groupId, std::string &groupOwner)
{
    JsonObject jsonObjGroup;
    jsonObjGroup[FIELD_GROUP_TYPE] = groupType;
    std::string queryParams = SafetyDump(jsonObjGroup);
    std::vector<GroupInfo> groupList;

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

int32_t HiChainConnector::ParseRemoteCredentialExt(const std::string &credentialInfo, std::string &params,
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
        LOGE("Credentaildata or authType string key not exist!");
        return ERR_DM_FAILED;
    }
    std::string jsonStr = jsonObject[FIELD_DEVICE_LIST].Dump();
    JsonObject jsonArray(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    jsonArray.Parse(jsonStr);
    jsonObj.Insert(FIELD_DEVICE_LIST, jsonArray);
    params = SafetyDump(jsonObj);
    return DM_OK;
}

int32_t HiChainConnector::addMultiMembersExt(const std::string &credentialInfo)
{
    if (deviceGroupManager_ == nullptr) {
        LOGE("HiChainConnector::deviceGroupManager_ is nullptr.");
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

int32_t HiChainConnector::deleteMultiMembers(const int32_t groupType, const std::string &userId,
    const JsonObject &jsonDeviceList)
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
        LOGE("[HICHAIN]fail to delete member from hichain group with ret:%{public}d.", ret);
        return ret;
    }
    return DM_OK;
}

std::vector<std::string> HiChainConnector::GetTrustedDevices(const std::string &localDeviceUdid)
{
    LOGI("get localDeviceUdid: %{public}s trusted devices.", GetAnonyString(localDeviceUdid).c_str());
    std::vector<GroupInfo> groups;
    int32_t ret = GetRelatedGroups(localDeviceUdid, groups);
    if (ret != DM_OK) {
        LOGE("failed to get groupInfo, ret: %{public}d", ret);
        return {};
    }

    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return {};
    }
    std::vector<std::string> trustedDevices;
    for (const auto &group : groups) {
        char *devicesJson = nullptr;
        uint32_t devNum = 0;
        ret = deviceGroupManager_->getTrustedDevices(userId, DM_PKG_NAME, group.groupId.c_str(),
        &devicesJson, &devNum);
        if (ret != 0 || devicesJson == nullptr) {
            LOGE("[HICHAIN]failed to get trusted devicesJson, ret: %{public}d", ret);
            deviceGroupManager_->destroyInfo(&devicesJson);
            return {};
        }
        GetTrustedDevicesUdid(devicesJson, trustedDevices);
        deviceGroupManager_->destroyInfo(&devicesJson);
    }
    return trustedDevices;
}

int32_t HiChainConnector::GetTrustedDevicesUdid(const char* jsonStr, std::vector<std::string> &udidList)
{
    JsonObject jsonObject(jsonStr);
    if (jsonObject.IsDiscarded()) {
        LOGE("credentialInfo string not a json type.");
        return ERR_DM_FAILED;
    }
    std::vector<JsonItemObject> children = jsonObject.Items();
    for (auto it1 = children.begin(); it1 != children.end(); it1++) {
        if (!IsString((*it1), FIELD_AUTH_ID)) {
            continue;
        }
        std::string udid = (*it1)[FIELD_AUTH_ID].Get<std::string>();
        udidList.push_back(udid);
    }
    return DM_OK;
}

void HiChainConnector::DeleteAllGroup(int32_t userId)
{
    LOGI("HiChainConnector::DeleteAllGroup");
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    std::vector<GroupInfo> groupList;
    GetRelatedGroups(userId, localUdid, groupList);
    for (auto &iter : groupList) {
        if (iter.groupType == GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP) {
            continue;
        }
        if (DeleteGroup(userId, iter.groupId) != DM_OK) {
            LOGE("Delete groupId %{public}s failed.", GetAnonyString(iter.groupId).c_str());
        }
    }
    std::vector<GroupInfo> groupListExt;
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

int32_t HiChainConnector::GetRelatedGroupsCommon(const std::string &deviceId, const char* pkgName,
    std::vector<GroupInfo> &groupList)
{
    LOGI("HiChainConnector::GetRelatedGroupsCommon Start to get local related groups.");
    uint32_t groupNum = 0;
    char *returnGroups = nullptr;
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (userId < 0) {
        LOGE("get current process account user id failed");
        return ERR_DM_FAILED;
    }
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
    std::vector<GroupInfo> groupInfos;
    jsonObject.Get(groupInfos);
    if (groupInfos.empty()) {
        LOGE("HiChainConnector::GetRelatedGroups group failed, groupInfos is empty.");
        return ERR_DM_FAILED;
    }
    groupList = groupInfos;
    return DM_OK;
}

int32_t HiChainConnector::DeleteGroup(const int32_t userId, std::string &groupId)
{
    if (userId < 0) {
        LOGE("user id failed");
        return ERR_DM_FAILED;
    }
    int64_t requestId = GenRequestId();
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_ID] = groupId;
    std::string disbandParams = SafetyDump(jsonObj);
    int32_t ret = deviceGroupManager_->deleteGroup(userId, requestId, DM_PKG_NAME, disbandParams.c_str());
    if (ret != 0) {
        LOGE("[HICHAIN]fail to delete group with ret:%{public}d.", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t HiChainConnector::GetRelatedGroupsCommon(int32_t userId, const std::string &deviceId, const char* pkgName,
    std::vector<GroupInfo> &groupList)
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
    JsonObject jsonObject(relatedGroups);
    if (jsonObject.IsDiscarded()) {
        LOGE("returnGroups parse error");
        return ERR_DM_FAILED;
    }
    if (!jsonObject.IsArray()) {
        LOGE("jsonObject is not an array.");
        return ERR_DM_FAILED;
    }
    std::vector<GroupInfo> groupInfos;
    jsonObject.Get(groupInfos);
    if (groupInfos.empty()) {
        LOGE("HiChainConnector::GetRelatedGroups group failed, groupInfos is empty.");
        return ERR_DM_FAILED;
    }
    groupList = groupInfos;
    return DM_OK;
}

void HiChainConnector::DeleteAllGroupByUdid(const std::string &udid)
{
    LOGI("HiChainConnector::DeleteAllGroupByUdid %{public}s.", GetAnonyString(udid).c_str());
    std::vector<GroupInfo> groupList;
    GetRelatedGroups(udid, groupList);
    for (auto &iter : groupList) {
        if (DeleteGroup(iter.groupId) != DM_OK) {
            LOGE("Delete groupId %{public}s failed.", GetAnonyString(iter.groupId).c_str());
        }
    }
    std::vector<GroupInfo> groupListExt;
    GetRelatedGroupsExt(udid, groupListExt);
    for (auto &iter : groupListExt) {
        if (DeleteGroupExt(iter.groupId) != DM_OK) {
            LOGE("DeleteGroupExt groupId %{public}s failed.", GetAnonyString(iter.groupId).c_str());
        }
    }
}

int32_t HiChainConnector::DeleteGroupByACL(std::vector<std::pair<int32_t, std::string>> &delACLInfoVec,
    std::vector<int32_t> &userIdVec)
{
    if (delACLInfoVec.size() == 0) {
        LOGI("delACLInfoVec is empty");
        return DM_OK;
    }
    if (userIdVec.size() == 0) {
        LOGI("userIdVec is empty");
        return DM_OK;
    }
    JsonObject jsonObj;
    jsonObj[FIELD_GROUP_TYPE] = GROUP_TYPE_PEER_TO_PEER_GROUP;
    std::string queryParams = SafetyDump(jsonObj);
    for (int32_t userId : userIdVec) {
        std::vector<GroupInfo> groupList;
        if (!GetGroupInfo(userId, queryParams, groupList)) {
            continue;
        }
        for (auto iter = groupList.begin(); iter != groupList.end(); iter++) {
            if (!IsNeedDelete(iter->groupName, userId, delACLInfoVec)) {
                continue;
            }
            if (DeleteGroup(userId, iter->groupId) != DM_OK) {
                LOGE("failed to delete group %{public}s", GetAnonyString(iter->groupId).c_str());
            }
        }
    }
    return DM_OK;
}

bool HiChainConnector::IsNeedDelete(std::string &groupName, int32_t userId,
    std::vector<std::pair<int32_t, std::string>> &delACLInfoVec)
{
    if (delACLInfoVec.size() == 0 || groupName.empty()) {
        LOGI("delACLInfoVec or groupName is empty");
        return false;
    }
    for (auto item : delACLInfoVec) {
        uint32_t interceptLength = item.second.size() / DEVICE_ID_HALF;
        std::string interceptUdid = item.second.substr(0, interceptLength);
        if (groupName.find(interceptUdid) != std::string::npos && userId == item.first) {
            return true;
        }
    }
    return false;
}

void HiChainConnector::DeleteHoDevice(const std::string &peerUdid, const std::vector<int32_t> &foreGroundUserIds,
    const std::vector<int32_t> &backGroundUserIds)
{
    LOGI("peerudid %{public}s, foreGroundUserIds %{public}s, backGroundUserIds %{public}s.",
        GetAnonyString(peerUdid).c_str(), GetIntegerList(foreGroundUserIds).c_str(),
        GetIntegerList(backGroundUserIds).c_str());
    std::vector<int32_t> localUserIds(foreGroundUserIds.begin(), foreGroundUserIds.end());
    std::copy(backGroundUserIds.begin(), backGroundUserIds.end(), std::back_inserter(localUserIds));
    for (const auto &item : localUserIds) {
        std::vector<GroupInfo> groupList;
        GetRelatedGroupsCommon(item, peerUdid, DM_PKG_NAME_EXT, groupList);
        for (auto &iter : groupList) {
            if (iter.groupType == GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP) {
                continue;
            }
            if (DeleteGroupExt(item, iter.groupId) != DM_OK) {
                LOGE("DeleteGroupExt groupId %{public}s failed.", GetAnonyString(iter.groupId).c_str());
            }
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS