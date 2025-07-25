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
#include "dm_constants.h"
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
constexpr int32_t DM_COMM_SEND_LOCAL_APP_UNINSTALL = 6;
constexpr int32_t DM_COMM_SEND_LOCAL_APP_UNBIND = 7;
constexpr int32_t DM_COMM_RSP_APP_UNINSTALL = 8;
constexpr int32_t DM_COMM_RSP_APP_UNBIND = 9;
constexpr const char* EVENT_TASK = "EventTask";
const char* const USER_STOP_MSG_KEY = "stopUserId";

DMCommTool::DMCommTool() : dmTransportPtr_(nullptr)
{
    LOGI("Ctor DMCommTool");
}

void DMCommTool::Init()
{
    LOGI("Init DMCommTool");
    eventQueue_ = std::make_shared<ffrt::queue>(EVENT_TASK);
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
    if (!IsIdLengthValid(rmtNetworkId) || dmTransportPtr_ == nullptr) {
        LOGE("param invalid, networkId: %{public}s, foreground userids size: %{public}d",
            GetAnonyString(rmtNetworkId).c_str(), static_cast<int32_t>(foregroundUserIds.size()));
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t socketId;
    if (dmTransportPtr_->StartSocket(rmtNetworkId, socketId) != DM_OK || socketId <= 0) {
        LOGE("Start socket error");
        return ERR_DM_FAILED;
    }

    UserIdsMsg userIdsMsg(foregroundUserIds, backgroundUserIds, true);
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
    LOGI("success");
    return DM_OK;
}

int32_t DMCommTool::SendUninstAppObj(int32_t userId, int32_t tokenId, const std::string &networkId)
{
    LOGI("SendUninstAppObj, userId: %{public}s, tokenId: %{public}s", GetAnonyInt32(userId).c_str(),
        GetAnonyInt32(tokenId).c_str());
    if (!IsIdLengthValid(networkId)) {
        LOGE("param invalid, networkId: %{public}s", GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null");
        return ERR_DM_FAILED;
    }
    int32_t socketId = 0;
    if (dmTransportPtr_->StartSocket(networkId, socketId) != DM_OK || socketId <= 0) {
        LOGE("Start socket error");
        return ERR_DM_FAILED;
    }

    UninstAppMsg uninstAppMsg(userId, tokenId);
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        LOGE("Create cJSON object failed.");
        return ERR_DM_FAILED;
    }
    ToJson(root, uninstAppMsg);
    char *msg = cJSON_PrintUnformatted(root);
    if (msg == nullptr) {
        cJSON_Delete(root);
        return ERR_DM_FAILED;
    }
    std::string msgStr(msg);
    cJSON_Delete(root);
    cJSON_free(msg);
    CommMsg commMsg(DM_COMM_SEND_LOCAL_APP_UNINSTALL, msgStr);
    std::string payload = GetCommMsgString(commMsg);

    int32_t ret = dmTransportPtr_->Send(networkId, payload, socketId);
    if (ret != DM_OK) {
        LOGE("Send local foreground userids failed, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    LOGI("Send local foreground userids success");
    return DM_OK;
}

int32_t DMCommTool::RspAppUninstall(const std::string rmtNetworkId, int32_t socketId)
{
    LOGI("RspAppUninstall Start.");
    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null");
        return ERR_DM_FAILED;
    }
    std::string msgStr("");
    CommMsg commMsg(DM_COMM_RSP_APP_UNINSTALL, msgStr);
    std::string payload = GetCommMsgString(commMsg);

    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null");
        return ERR_DM_FAILED;
    }
    int32_t ret = dmTransportPtr_->Send(rmtNetworkId, payload, socketId);
    if (ret != DM_OK) {
        LOGE("RspAppUninstall failed, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    
    LOGI("RspAppUninstall success");
    return DM_OK;
}

int32_t DMCommTool::RspAppUnbind(const std::string rmtNetworkId, int32_t socketId)
{
    LOGI("RspAppUnbind Start.");
    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null");
        return ERR_DM_FAILED;
    }
    std::string msgStr("");
    CommMsg commMsg(DM_COMM_RSP_APP_UNBIND, msgStr);
    std::string payload = GetCommMsgString(commMsg);

    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null");
        return ERR_DM_FAILED;
    }
    int32_t ret = dmTransportPtr_->Send(rmtNetworkId, payload, socketId);
    if (ret != DM_OK) {
        LOGE("RspAppUnbind failed, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    
    LOGI("RspAppUnbind success");
    return DM_OK;
}

int32_t DMCommTool::SendUnBindAppObj(int32_t userId, int32_t tokenId, const std::string &extra,
    const std::string &networkId, const std::string &udid)
{
    LOGI("DMCommTool::SendUnBindAppObj, userId: %{public}s, tokenId: %{public}s, extra: %{public}s, udid: %{public}s",
        GetAnonyInt32(userId).c_str(), GetAnonyInt32(tokenId).c_str(), GetAnonyString(extra).c_str(),
        GetAnonyString(udid).c_str());
    if (!IsIdLengthValid(networkId)) {
        LOGE("param invalid, networkId: %{public}s", GetAnonyString(networkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    
    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null");
        return ERR_DM_FAILED;
    }
    int32_t socketId;
    if (dmTransportPtr_->StartSocket(networkId, socketId) != DM_OK || socketId <= 0) {
        LOGE("Start socket error");
        return ERR_DM_FAILED;
    }

    UnBindAppMsg unBindAppMsg(userId, tokenId, extra, udid);
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        LOGE("Create cJSON object failed.");
        return ERR_DM_FAILED;
    }
    ToJson(root, unBindAppMsg);
    char *msg = cJSON_PrintUnformatted(root);
    if (msg == nullptr) {
        cJSON_Delete(root);
        return ERR_DM_FAILED;
    }
    std::string msgStr(msg);
    cJSON_Delete(root);
    cJSON_free(msg);
    CommMsg commMsg(DM_COMM_SEND_LOCAL_APP_UNBIND, msgStr);
    std::string payload = GetCommMsgString(commMsg);

    int32_t ret = dmTransportPtr_->Send(networkId, payload, socketId);
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
    UserIdsMsg userIdsMsg(foregroundUserIds, backgroundUserIds, true);
    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null.");
        return;
    }
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        LOGE("Create cJSON object failed: root is null.");
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
    LOGI("success");
}

DMCommTool::DMCommToolEventHandler::DMCommToolEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner,
    std::shared_ptr<DMCommTool> dmCommToolPtr) : AppExecFwk::EventHandler(runner), dmCommToolWPtr_(dmCommToolPtr)
{
    LOGI("Ctor DMCommToolEventHandler");
}

void DMCommTool::DMCommToolEventHandler::ParseUserIdsMsg(std::shared_ptr<InnerCommMsg> commMsg, UserIdsMsg &userIdsMsg)
{
    CHECK_NULL_VOID(commMsg);
    std::string payload = commMsg->commMsg->msg;
    cJSON *root = cJSON_Parse(payload.c_str());
    if (root == NULL) {
        LOGE("the msg is not json format");
        return;
    }
    FromJson(root, userIdsMsg);
    cJSON_Delete(root);
}

void DMCommTool::DMCommToolEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    std::shared_ptr<InnerCommMsg> commMsg = event->GetSharedObject<InnerCommMsg>();
    CHECK_NULL_VOID(commMsg);
    UserIdsMsg userIdsMsg;
    ParseUserIdsMsg(commMsg, userIdsMsg);
    if (dmCommToolWPtr_.expired()) {
        LOGE("dmCommToolWPtr_ is expired");
        return;
    }
    std::shared_ptr<DMCommTool> dmCommToolPtr = dmCommToolWPtr_.lock();
    HandleEvent(dmCommToolPtr, eventId, commMsg, userIdsMsg);
}

void DMCommTool::DMCommToolEventHandler::HandleEvent(const std::shared_ptr<DMCommTool> &dmCommToolPtr, uint32_t eventId,
    const std::shared_ptr<InnerCommMsg> &commMsg, const UserIdsMsg &userIdsMsg)
{
    CHECK_NULL_VOID(dmCommToolPtr);
    HandleLocalUserIdEvent(dmCommToolPtr, eventId, commMsg, userIdsMsg);
    switch (eventId) {
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
        case DM_COMM_SEND_LOCAL_APP_UNINSTALL: {
            dmCommToolPtr->ProcessReceiveUninstAppEvent(commMsg);
            break;
        }
        case DM_COMM_SEND_LOCAL_APP_UNBIND: {
            dmCommToolPtr->ProcessReceiveUnBindAppEvent(commMsg);
            break;
        }
        case DM_COMM_RSP_APP_UNINSTALL: {
            dmCommToolPtr->ProcessReceiveRspAppUninstallEvent(commMsg);
            break;
        }
        case DM_COMM_RSP_APP_UNBIND: {
            dmCommToolPtr->ProcessReceiveRspAppUnbindEvent(commMsg);
            break;
        }
        default:
            LOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}

void DMCommTool::DMCommToolEventHandler::HandleLocalUserIdEvent(const std::shared_ptr<DMCommTool> &dmCommToolPtr,
    uint32_t eventId, const std::shared_ptr<InnerCommMsg> &commMsg, const UserIdsMsg &userIdsMsg)
{
    CHECK_NULL_VOID(dmCommToolPtr);
    switch (eventId) {
        case DM_COMM_SEND_LOCAL_USERIDS: {
            if (userIdsMsg.isNewEvent) {
                dmCommToolPtr->ProcessReceiveCommonEvent(commMsg);
            } else {
                dmCommToolPtr->ProcessReceiveUserIdsEvent(commMsg);
            }
            break;
        }
        case DM_COMM_RSP_LOCAL_USERIDS: {
            if (userIdsMsg.isNewEvent) {
                dmCommToolPtr->ProcessResponseCommonEvent(commMsg);
            } else {
                dmCommToolPtr->ProcessResponseUserIdsEvent(commMsg);
            }
            break;
        }
        default:
            break;
    }
}

void DMCommTool::ProcessReceiveCommonEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    LOGI("start");
    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid: %{public}s", GetAnonyString(commMsg->remoteNetworkId).c_str());
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
    MultipleUserConnector::GetForegroundUserIds(foregroundUserIds);
    std::vector<int32_t> backgroundUserIds;
    MultipleUserConnector::GetBackgroundUserIds(backgroundUserIds);
    MultipleUserConnector::ClearLockedUser(foregroundUserIds, backgroundUserIds);
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

    DeviceManagerService::GetInstance().ProcessCommonUserStatusEvent(userIdsMsg.foregroundUserIds,
        userIdsMsg.backgroundUserIds, rmtUdid);
}

void DMCommTool::ProcessResponseCommonEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    LOGI("start");
    // step1: close socket
    this->dmTransportPtr_->StopSocket(commMsg->remoteNetworkId);

    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid: %{public}s", GetAnonyString(commMsg->remoteNetworkId).c_str());
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
    DeviceManagerService::GetInstance().ProcessCommonUserStatusEvent(userIdsMsg.foregroundUserIds,
        userIdsMsg.backgroundUserIds, rmtUdid);
}

void DMCommTool::ProcessReceiveUserIdsEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    LOGI("start");
    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid: %{public}s", GetAnonyString(commMsg->remoteNetworkId).c_str());
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

void DMCommTool::ProcessReceiveUninstAppEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    if (commMsg == nullptr || commMsg->commMsg == nullptr) {
        LOGE("commMsg or commMsg->commMsg is null");
        return;
    }
    LOGI("DMCommTool::ProcessReceiveUninstAppEvent commMsg = %{public}s",
        GetAnonyString(commMsg->commMsg->msg).c_str());
    std::string payload = commMsg->commMsg->msg;
    cJSON *root = cJSON_Parse(payload.c_str());
    if (root == NULL) {
        LOGE("the msg is not json format");
        return;
    }
    UninstAppMsg uninstAppMsg;
    FromJson(root, uninstAppMsg);
    cJSON_Delete(root);
    RspAppUninstall(commMsg->remoteNetworkId, commMsg->socketId);

    if (uninstAppMsg.userId_ == -1 || uninstAppMsg.tokenId_ == -1) {
        LOGE("param invalid, userId: %{public}d, tokenId: %{public}d",
            uninstAppMsg.userId_, uninstAppMsg.tokenId_);
        return;
    } else {
        DeviceManagerService::GetInstance().ProcessUninstApp(uninstAppMsg.userId_,
            uninstAppMsg.tokenId_);
    }
}

void DMCommTool::ProcessReceiveUnBindAppEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    if (commMsg == nullptr || commMsg->commMsg == nullptr) {
        LOGE("commMsg or commMsg->commMsg is null");
        return;
    }
    LOGI("DMCommTool::ProcessReceiveUnBindAppEvent Receive remote uninstall app");
    std::string payload = commMsg->commMsg->msg;
    cJSON *root = cJSON_Parse(payload.c_str());
    if (root == NULL) {
        LOGE("the msg is not json format");
        return;
    }
    UnBindAppMsg unBindAppMsg;
    FromJson(root, unBindAppMsg);
    cJSON_Delete(root);
    RspAppUnbind(commMsg->remoteNetworkId, commMsg->socketId);

    if (unBindAppMsg.userId_ == -1 || unBindAppMsg.tokenId_ == -1) {
        LOGE("param invalid, userId: %{public}d, tokenId: %{public}d",
            unBindAppMsg.userId_, unBindAppMsg.tokenId_);
        return;
    } else {
        DeviceManagerService::GetInstance().ProcessUnBindApp(unBindAppMsg.userId_,
            unBindAppMsg.tokenId_, unBindAppMsg.extra_, unBindAppMsg.udid_);
    }
}

void DMCommTool::StopSocket(const std::string &networkId)
{
    LOGI("DMCommTool::StopSocket, networkId = %{public}s", GetAnonyString(networkId).c_str());
    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null");
        return;
    }
    dmTransportPtr_->StopSocket(networkId);
}

void DMCommTool::ProcessReceiveRspAppUninstallEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    if (commMsg == nullptr || commMsg->remoteNetworkId == "") {
        LOGE("commMsg or commMsg->remoteNetworkId is null");
        return;
    }
    LOGI("DMCommTool::ProcessReceiveRspAppUninstallEvent Start.");
    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null");
        return;
    }
    this->dmTransportPtr_->StopSocket(commMsg->remoteNetworkId);
    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid.");
        return;
    }
    DeviceManagerService::GetInstance().ProcessReceiveRspAppUninstall(rmtUdid);
}

void DMCommTool::ProcessReceiveRspAppUnbindEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    if (commMsg == nullptr || commMsg->remoteNetworkId == "") {
        LOGE("commMsg or commMsg->remoteNetworkId is null");
        return;
    }
    if (dmTransportPtr_ == nullptr) {
        LOGE("dmTransportPtr_ is null");
        return;
    }
    LOGI("DMCommTool::ProcessReceiveRspAppUnbindEvent Start.");
    this->dmTransportPtr_->StopSocket(commMsg->remoteNetworkId);
    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid.");
        return;
    }
    DeviceManagerService::GetInstance().ProcessReceiveRspAppUnbind(rmtUdid);
}

void DMCommTool::ProcessResponseUserIdsEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    LOGI("start");
    // step1: close socket
    this->dmTransportPtr_->StopSocket(commMsg->remoteNetworkId);

    std::string rmtUdid = "";
    SoftbusCache::GetInstance().GetUdidFromCache(commMsg->remoteNetworkId.c_str(), rmtUdid);
    if (rmtUdid.empty()) {
        LOGE("Can not find remote udid by networkid.");
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
    if (!cJSON_AddItemToObject(root, USER_STOP_MSG_KEY, numberObj)) {
        LOGE("add numberObj to root failed.");
        cJSON_Delete(numberObj);
        cJSON_Delete(root);
        return ERR_DM_FAILED;
    }
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
    LOGI("success");
    return DM_OK;
}

void DMCommTool::ProcessReceiveLogoutEvent(const std::shared_ptr<InnerCommMsg> commMsg)
{
    CHECK_NULL_VOID(commMsg);
    CHECK_NULL_VOID(dmTransportPtr_);
    this->dmTransportPtr_->StopSocket(commMsg->remoteNetworkId);
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
    LOGI("success.");
}

int32_t DMCommTool::StartCommonEvent(std::string commonEventType, EventCallback eventCallback)
{
    if (commonEventType.empty() || eventCallback == nullptr) {
        LOGE("StartCommonEvent input value invalid");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    CHECK_NULL_RETURN(eventQueue_, ERR_DM_POINT_NULL);
    LOGI("start eventType: %{public}s", commonEventType.c_str());
    std::lock_guard<std::mutex> locker(eventMutex_);
    auto taskFunc = [eventCallback] () { eventCallback(); };
    eventQueue_->submit(taskFunc);
    return DM_OK;
}
} // DistributedHardware
} // OHOS