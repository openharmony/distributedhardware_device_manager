/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dm_comm_tool.h"
#include "device_manager_service.h"
#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_transport.h"
#include "dm_transport_msg.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "multiple_user_connector.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
// send local foreground userids msg
constexpr int32_t DM_COMM_SEND_LOCAL_USERIDS = 1;
// if receive remote device send foreground userids, response local foreground uerids
// This msg no need response
constexpr int32_t DM_COMM_RSP_LOCAL_USERIDS = 2;
constexpr int32_t DM_COMM_SEND_USER_STOP = 3;
constexpr int32_t DM_COMM_RSP_USER_STOP = 4;
constexpr int32_t DM_COMM_ACCOUNT_LOGOUT = 5;

const char* const USER_STOP_MSG_KEY = "stopUserId";

DMCommTool::DMCommTool() : dmTransportPtr_(nullptr)
{
    LOGI("Ctor DMCommTool");
}

void DMCommTool::Init()
{
    LOGI("Init DMCommTool");
    dmTransportPtr_ = std::make_shared<DMTransport>(shared_from_this());
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<DMCommTool::DMCommToolEventHandler>(runner, shared_from_this());
    dmTransportPtr_->Init();
}

void DMCommTool::UnInit()
{
    LOGI("UnInit DMCommTool");
    if (dmTransportPtr_ == nullptr) {
        LOGI("dmTransportPtr_ is null");
        return;
    }
    dmTransportPtr_->UnInit();
}

std::shared_ptr<DMCommTool> DMCommTool::GetInstance()
{
    static std::shared_ptr<DMCommTool> instance = std::make_shared<DMCommTool>();
    return instance;
}

int32_t DMCommTool::SendUserIds(const std::string rmtNetworkId,
    const std::vector<uint32_t> &foregroundUserIds, const std::vector<uint32_t> &backgroundUserIds)
{
    if (!IsIdLengthValid(rmtNetworkId) || foregroundUserIds.empty() || dmTransportPtr_ == nullptr) {
        LOGE("param invalid, networkId: %{public}s, foreground userids size: %{public}d",
            GetAnonyString(rmtNetworkId).c_str(), static_cast<int32_t>(foregroundUserIds.size()));
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t socketId;
    if (dmTransportPtr_->StartSocket(rmtNetworkId, socketId) != DM_OK || socketId <= 0) {
        LOGE("Start socket error");
        return ERR_DM_FAILED;
    }

    UserIdsMsg userIdsMsg(foregroundUserIds, backgroundUserIds);
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        LOGE("Create cJSON object failed.");
        return ERR_DM_FAILED;
    }
    ToJson(root, userIdsMsg);
    char *msg = cJSON_PrintUnformatted(root);
    if (msg == nullptr) {
        cJSON_Delete(root);
        return ERR_DM_FAILED;
    }
    std::string msgStr(msg);
    cJSON_Delete(root);
    cJSON_free(msg);
    CommMsg commMsg(DM_COMM_SEND_LOCAL_USERIDS, msgStr);
    std::string payload = GetCommMsgString(commMsg);

    int32_t ret = dmTransportPtr_->Send(rmtNetworkId, payload, socketId);
    if (ret != DM_OK) {
        LOGE("Send local foreground userids failed, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    LOGI("Send local foreground userids success");
    return DM_OK;
}

void DMCommTool::RspLocalFrontOrBackUserIds(const std::string rmtNetworkId,
    const std::vector<uint32_t> &foregroundUserIds, const std::vector<uint32_t> &backgroundUserIds, int32_t socketId)
{
    UserIdsMsg userIdsMsg(foregroundUserIds, backgroundUserIds);
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        LOGE("Create cJSON object failed.");
        return;
    }
    ToJson(root, userIdsMsg);
    char *msg = cJSON_PrintUnformatted(root);
    if (msg == nullptr) {
        cJSON_Delete(root);
        return;
    }
    std::string msgStr(msg);
    cJSON_Delete(root);
    cJSON_free(msg);
    CommMsg commMsg(DM_COMM_RSP_LOCAL_USERIDS, msgStr);
    std::string payload = GetCommMsgString(commMsg);

    int32_t ret = dmTransportPtr_->Send(rmtNetworkId, payload, socketId);
    if (ret != DM_OK) {
        LOGE("Response local foreground userids failed, ret: %{public}d", ret);
        return;
    }
    LOGI("Response local foreground userids success");
}

DMCommTool::DMCommToolEventHandler::DMCommToolEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner,
    std::shared_ptr<DMCommTool> dmCommToolPtr) : AppExecFwk::EventHandler(runner), dmCommToolWPtr_(dmCommToolPtr)
{
    LOGI("Ctor DMCommToolEventHandler");
}

void DMCommTool::DMCommToolEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    std::shared_ptr<InnerCommMsg> commMsg = event->GetSharedObject<InnerCommMsg>();
    if (commMsg == nullptr) {
        LOGE("ProcessEvent commMsg is null");
        return;
    }
    if (dmCommToolWPtr_.expired()) {
        LOGE("dmCommToolWPtr_ is expired");
        return;
    }
    std::shared_ptr<DMCommTool> dmCommToolPtr = dmCommToolWPtr_.lock();
    if (dmCommToolPtr == nullptr) {
        LOGE("dmCommToolPtr is null");
        return;
    }
    switch (eventId) {
        case DM_COMM_SEND_LOCAL_USERIDS: {
            // Process remote foreground userids and send back local user ids
            dmCommToolPtr->ProcessReceiveUserIdsEvent(commMsg);
            break;
        }
        case DM_COMM_RSP_LOCAL_USERIDS: {
            // Process remote foreground userids and close session
            dmCommToolPtr->ProcessResponseUserIdsEvent(commMsg);
            break;
        }
        case DM_COMM_SEND_USER_STOP: {
            dmCommToolPtr->ProcessReceiveUserStopEvent(commMsg);
            break;
        }
        case DM_COMM_RSP_USER_STOP: {
            dmCommToolPtr->ProcessResponseUserStopEvent(commMsg);
            break;
        }
        case DM_COMM_ACCOUNT_LOGOUT: {
            dmCommToolPtr->ProcessReceiveLogoutEvent(commMsg);
            break;
        }
        default:
            LOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}

void DMCommTool::ProcessReceiveUserIdsEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    LOGI("Receive remote userids, process and rsp local userid");
    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid: %{public}s", commMsg->remoteNetworkId.c_str());
        return;
    }

    std::string payload = commMsg->commMsg->msg;
    cJSON *root = cJSON_Parse(payload.c_str());
    if (root == NULL) {
        LOGE("the msg is not json format");
        return;
    }
    UserIdsMsg userIdsMsg;
    FromJson(root, userIdsMsg);
    cJSON_Delete(root);
    uint32_t totalUserNum = static_cast<uint32_t>(userIdsMsg.foregroundUserIds.size()) +
        static_cast<uint32_t>(userIdsMsg.backgroundUserIds.size());

    // step1: send back local userids
    std::vector<int32_t> foregroundUserIds;
    std::vector<int32_t> backgroundUserIds;
    MultipleUserConnector::GetForegroundUserIds(foregroundUserIds);
    MultipleUserConnector::GetBackgroundUserIds(backgroundUserIds);
    if (DeviceManagerService::GetInstance().IsPC()) {
        MultipleUserConnector::ClearLockedUser(foregroundUserIds, backgroundUserIds);
    }
    std::vector<uint32_t> foregroundUserIdsU32;
    std::vector<uint32_t> backgroundUserIdsU32;
    for (auto const &u : foregroundUserIds) {
        foregroundUserIdsU32.push_back(static_cast<uint32_t>(u));
    }
    for (auto const &u : backgroundUserIds) {
        backgroundUserIdsU32.push_back(static_cast<uint32_t>(u));
    }
    RspLocalFrontOrBackUserIds(commMsg->remoteNetworkId, foregroundUserIdsU32, backgroundUserIdsU32,
        commMsg->socketId);

    if (userIdsMsg.foregroundUserIds.empty()) {
        LOGE("Parse but get none remote foreground userids");
    } else {
        // step2: process remote foreground/background userids
        DeviceManagerService::GetInstance().ProcessSyncUserIds(userIdsMsg.foregroundUserIds,
            userIdsMsg.backgroundUserIds, rmtUdid);
    }
}

void DMCommTool::ProcessResponseUserIdsEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    LOGI("process receive remote userids response");
    // step1: close socket
    this->dmTransportPtr_->StopSocket(commMsg->remoteNetworkId);

    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid: %{public}s", commMsg->remoteNetworkId.c_str());
        return;
    }

    std::string payload = commMsg->commMsg->msg;
    cJSON *root = cJSON_Parse(payload.c_str());
    if (root == NULL) {
        LOGE("the msg is not json format");
        return;
    }
    UserIdsMsg userIdsMsg;
    FromJson(root, userIdsMsg);
    cJSON_Delete(root);
    if (userIdsMsg.foregroundUserIds.empty()) {
        LOGE("Parse but get none remote userids");
        return;
    }
    // step2: process remote foreground/background userids
    if (!userIdsMsg.foregroundUserIds.empty()) {
        DeviceManagerService::GetInstance().ProcessSyncUserIds(userIdsMsg.foregroundUserIds,
            userIdsMsg.backgroundUserIds, rmtUdid);
    } else {
        LOGE("Receive remote foreground userid empty");
    }
}

std::shared_ptr<DMCommTool::DMCommToolEventHandler> DMCommTool::GetEventHandler()
{
    return this->eventHandler_;
}

const std::shared_ptr<DMTransport> DMCommTool::GetDMTransportPtr()
{
    return this->dmTransportPtr_;
}

int32_t DMCommTool::CreateUserStopMessage(int32_t stopUserId, std::string &msgStr)
{
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        LOGE("Create cJSON object failed.");
        return ERR_DM_FAILED;
    }
    cJSON *numberObj = cJSON_CreateNumber(stopUserId);
    if (numberObj == nullptr) {
        cJSON_Delete(root);
        return ERR_DM_FAILED;
    }
    cJSON_AddItemToObject(root, USER_STOP_MSG_KEY, numberObj);
    char *msg = cJSON_PrintUnformatted(root);
    if (msg == nullptr) {
        cJSON_Delete(root);
        return ERR_DM_FAILED;
    }
    msgStr = std::string(msg);
    cJSON_free(msg);
    cJSON_Delete(root);
    return DM_OK;
}

int32_t DMCommTool::ParseUserStopMessage(const std::string &msgStr, int32_t &stopUserId)
{
    cJSON *root = cJSON_Parse(msgStr.c_str());
    if (root == NULL) {
        LOGE("the msg is not json format");
        return ERR_DM_FAILED;
    }
    cJSON *stopUserIdObj = cJSON_GetObjectItem(root, USER_STOP_MSG_KEY);
    if (stopUserIdObj == NULL || !cJSON_IsNumber(stopUserIdObj)) {
        LOGE("parse stopUserId id failed.");
        cJSON_Delete(root);
        return ERR_DM_FAILED;
    }
    stopUserId = static_cast<int32_t>(stopUserIdObj->valueint);
    cJSON_Delete(root);
    return DM_OK;
}

int32_t DMCommTool::SendUserStop(const std::string rmtNetworkId, int32_t stopUserId)
{
    std::string msgStr;
    int32_t ret = CreateUserStopMessage(stopUserId, msgStr);
    if (ret != DM_OK) {
        LOGE("error ret: %{public}d", ret);
        return ret;
    }
    return SendMsg(rmtNetworkId, DM_COMM_SEND_USER_STOP, msgStr);
}

int32_t DMCommTool::SendMsg(const std::string rmtNetworkId, int32_t msgType, const std::string &msg)
{
    if (!IsIdLengthValid(rmtNetworkId) || dmTransportPtr_ == nullptr) {
        LOGE("param invalid, networkId: %{public}s", GetAnonyString(rmtNetworkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t socketId;
    if (dmTransportPtr_->StartSocket(rmtNetworkId, socketId) != DM_OK || socketId <= 0) {
        LOGE("Start socket error");
        return ERR_DM_FAILED;
    }
    CommMsg commMsg(msgType, msg);
    std::string payload = GetCommMsgString(commMsg);
    int32_t ret = dmTransportPtr_->Send(rmtNetworkId, payload, socketId);
    if (ret != DM_OK) {
        LOGE("SendMsg failed, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    LOGI("SendMsg success");
    return DM_OK;
}

void DMCommTool::ProcessReceiveUserStopEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    LOGI("start");
    CHECK_NULL_VOID(commMsg);
    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid: %{public}s", GetAnonyString(commMsg->remoteNetworkId).c_str());
        return;
    }
    int32_t stopUserId = -1;
    int32_t ret = ParseUserStopMessage(commMsg->commMsg->msg, stopUserId);
    if (ret != DM_OK) {
        LOGE("ParseUserStopMessage error ret: %{public}d", ret);
        return;
    }
    RspUserStop(commMsg->remoteNetworkId, commMsg->socketId, stopUserId);
    DeviceManagerService::GetInstance().HandleUserStop(stopUserId, rmtUdid);
}

void DMCommTool::RspUserStop(const std::string rmtNetworkId, int32_t socketId, int32_t stopUserId)
{
    std::string msgStr = "";
    CHECK_NULL_VOID(dmTransportPtr_);
    int32_t ret = CreateUserStopMessage(stopUserId, msgStr);
    if (ret != DM_OK || msgStr.empty()) {
        LOGE("error ret: %{public}d", ret);
        return;
    }
    CommMsg commMsg(DM_COMM_RSP_USER_STOP, msgStr);
    std::string payload = GetCommMsgString(commMsg);
    ret = dmTransportPtr_->Send(rmtNetworkId, payload, socketId);
    if (ret != DM_OK) {
        LOGE("failed, ret: %{public}d", ret);
        return;
    }
    LOGI("success");
}

void DMCommTool::ProcessResponseUserStopEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    LOGI("start");
    CHECK_NULL_VOID(commMsg);
    this->dmTransportPtr_->StopSocket(commMsg->remoteNetworkId);
    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid: %{public}s", GetAnonyString(commMsg->remoteNetworkId).c_str());
        return;
    }
    int32_t stopUserId = -1;
    int32_t ret = ParseUserStopMessage(commMsg->commMsg->msg, stopUserId);
    if (ret != DM_OK) {
        LOGE("ParseUserStopMessage error ret: %{public}d", ret);
        return;
    }
    std::vector<std::string> acceptEventUdids;
    acceptEventUdids.push_back(rmtUdid);
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    DeviceManagerService::GetInstance().HandleUserStop(stopUserId, localUdid, acceptEventUdids);
}

int32_t DMCommTool::SendLogoutAccountInfo(const std::string &rmtNetworkId,
    const std::string &accountId, int32_t userId)
{
    if (!IsIdLengthValid(rmtNetworkId) || accountId.empty() || dmTransportPtr_ == nullptr) {
        LOGE("param invalid, networkId: %{public}s, userId: %{public}d",
            GetAnonyString(rmtNetworkId).c_str(), userId);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, send networkId: %{public}s", GetAnonyString(rmtNetworkId).c_str());
    int32_t socketId = 0;
    if (dmTransportPtr_->StartSocket(rmtNetworkId, socketId) != DM_OK || socketId <= 0) {
        LOGE("Start socket error");
        return ERR_DM_FAILED;
    }

    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        LOGE("Create cJSON object failed.");
        return ERR_DM_FAILED;
    }
    LogoutAccountMsg LogoutAccountMsg(accountId, userId);
    ToJson(root, LogoutAccountMsg);
    char *msg = cJSON_PrintUnformatted(root);
    if (msg == nullptr) {
        cJSON_Delete(root);
        return ERR_DM_FAILED;
    }
    std::string msgStr(msg);
    cJSON_Delete(root);
    cJSON_free(msg);
    CommMsg commMsg(DM_COMM_ACCOUNT_LOGOUT, msgStr);
    std::string payload = GetCommMsgString(commMsg);

    int32_t ret = dmTransportPtr_->Send(rmtNetworkId, payload, socketId);
    if (ret != DM_OK) {
        LOGE("Send account logout failed, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    LOGI("Send account logout success");
    return DM_OK;
}

void DMCommTool::ProcessReceiveLogoutEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    CHECK_NULL_VOID(commMsg);
    LOGI("Receive remote logout, networkId: %{public}s", GetAnonyString(commMsg->remoteNetworkId).c_str());
    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid: %{public}s", GetAnonyString(commMsg->remoteNetworkId).c_str());
        return;
    }

    CHECK_NULL_VOID(commMsg->commMsg);
    std::string payload = commMsg->commMsg->msg;
    cJSON *root = cJSON_Parse(payload.c_str());
    if (root == NULL) {
        LOGE("the msg is not json format");
        return;
    }
    LogoutAccountMsg logoutAccountMsg;
    FromJson(root, logoutAccountMsg);
    cJSON_Delete(root);

    if (logoutAccountMsg.accountId.empty() || logoutAccountMsg.userId == -1) {
        LOGE("param invalid, accountId: %{public}s, userId: %{public}d",
            GetAnonyString(logoutAccountMsg.accountId).c_str(), logoutAccountMsg.userId);
        return;
    }
    DeviceManagerService::GetInstance().ProcessSyncAccountLogout(logoutAccountMsg.accountId,
        rmtUdid, logoutAccountMsg.userId);
    LOGI("process remote logout success.");
}
} // DistributedHardware
} // OHOS