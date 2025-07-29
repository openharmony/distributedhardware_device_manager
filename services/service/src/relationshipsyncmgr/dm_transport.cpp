/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <unistd.h>

#include "dm_transport.h"
#include "dm_anonymous.h"
#include "dm_comm_tool.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "dm_transport_msg.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
// Dsoftbus sendBytes max message length: 4MB
constexpr uint32_t MAX_SEND_MSG_LENGTH = 4 * 1024 * 1024;
constexpr uint32_t INTERCEPT_STRING_LENGTH = 20;
constexpr uint32_t MAX_ROUND_SIZE = 1000;
const int32_t USLEEP_TIME_US_200000 = 200000;           // 200ms
static QosTV g_qosInfo[] = {
    { .qos = QOS_TYPE_MIN_BW, .value = 256 * 1024},
    { .qos = QOS_TYPE_MAX_LATENCY, .value = 8000 },
    { .qos = QOS_TYPE_MIN_LATENCY, .value = 2000 }
};
static uint32_t g_qosTvParamIndex = static_cast<uint32_t>(sizeof(g_qosInfo) / sizeof(g_qosInfo[0]));
static std::weak_ptr<DMCommTool> g_dmCommToolWPtr_;
}

DMTransport::DMTransport(std::shared_ptr<DMCommTool> dmCommToolPtr) : remoteDevSocketIds_({}), localServerSocket_(-1),
    localSocketName_(""), isSocketSvrCreateFlag_(false), dmCommToolWPtr_(dmCommToolPtr)
{
    LOGI("Ctor DMTransport");
    g_dmCommToolWPtr_ = dmCommToolPtr;
}

int32_t DMTransport::OnSocketOpened(int32_t socketId, const PeerSocketInfo &info)
{
    LOGI("OnSocketOpened, socket: %{public}d, peerSocketName: %{public}s, peerNetworkId: %{public}s, "
        "peerPkgName: %{public}s", socketId, info.name, GetAnonyString(info.networkId).c_str(), info.pkgName);
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    if (remoteDevSocketIds_.find(info.networkId) == remoteDevSocketIds_.end()) {
        std::set<int32_t> socketSet;
        socketSet.insert(socketId);
        remoteDevSocketIds_[info.networkId] = socketSet;
        return DM_OK;
    }
    remoteDevSocketIds_.at(info.networkId).insert(socketId);
    return DM_OK;
}

void DMTransport::OnSocketClosed(int32_t socketId, ShutdownReason reason)
{
    LOGI("OnSocketClosed, socket: %{public}d, reason: %{public}d", socketId, (int32_t)reason);
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    for (auto iter = remoteDevSocketIds_.begin(); iter != remoteDevSocketIds_.end();) {
        iter->second.erase(socketId);
        if (iter->second.empty()) {
            iter = remoteDevSocketIds_.erase(iter);
        } else {
            ++iter;
        }
    }
    sourceSocketIds_.erase(socketId);
}

void DMTransport::OnBytesReceived(int32_t socketId, const void *data, uint32_t dataLen)
{
    if (socketId < 0 || data == nullptr || dataLen == 0 || dataLen > MAX_SEND_MSG_LENGTH) {
        LOGE("OnBytesReceived param check failed");
        return;
    }

    std::string remoteNeworkId = GetRemoteNetworkIdBySocketId(socketId);
    if (remoteNeworkId.empty()) {
        LOGE("Can not find the remote network id by socketId: %{public}d", socketId);
        return;
    }

    uint8_t *buf = reinterpret_cast<uint8_t *>(calloc(dataLen + 1, sizeof(uint8_t)));
    if (buf == nullptr) {
        LOGE("OnBytesReceived: malloc memory failed");
        return;
    }

    if (memcpy_s(buf, dataLen + 1,  reinterpret_cast<const uint8_t *>(data), dataLen) != EOK) {
        LOGE("OnBytesReceived: memcpy memory failed");
        free(buf);
        return;
    }

    std::string message(buf, buf + dataLen);
    LOGI("Receive message size: %{public}" PRIu32, dataLen);
    HandleReceiveMessage(socketId, message);
    free(buf);
    return;
}

void DMTransport::HandleReceiveMessage(const int32_t socketId, const std::string &payload)
{
    std::string rmtNetworkId = GetRemoteNetworkIdBySocketId(socketId);
    if (rmtNetworkId.empty()) {
        LOGE("Can not find networkId by socketId: %{public}d", socketId);
        return;
    }
    if (payload.empty() || payload.size() > MAX_SEND_MSG_LENGTH) {
        LOGE("payload invalid");
        return;
    }
    LOGI("Receive msg: %{public}s", GetAnonyString(payload).c_str());
    cJSON *root = cJSON_Parse(payload.c_str());
    if (root == NULL) {
        LOGE("the msg is not json format");
        return;
    }
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    FromJson(root, *commMsg);
    cJSON_Delete(root);

    std::shared_ptr<InnerCommMsg> innerMsg = std::make_shared<InnerCommMsg>(rmtNetworkId, commMsg, socketId);

    LOGI("Receive DM msg, code: %{public}d, msg: %{public}s", commMsg->code, GetAnonyString(commMsg->msg).c_str());
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(commMsg->code, innerMsg);
    std::shared_ptr<DMCommTool> dmCommToolSPtr = dmCommToolWPtr_.lock();
    if (dmCommToolSPtr == nullptr) {
        LOGE("Can not get DMCommTool ptr");
        return;
    }
    if (dmCommToolSPtr->GetEventHandler() == nullptr) {
        LOGE("Can not get eventHandler");
        return;
    }
    dmCommToolSPtr->GetEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
}

//LCOV_EXCL_START
std::shared_ptr<DMCommTool> GetDMCommToolPtr()
{
    if (g_dmCommToolWPtr_.expired()) {
        LOGE("DMCommTool Weak ptr expired");
        return nullptr;
    }

    std::shared_ptr<DMCommTool> dmCommToolSPtr = g_dmCommToolWPtr_.lock();
    if (dmCommToolSPtr == nullptr) {
        LOGE("Can not get DMCommTool ptr");
        return nullptr;
    }

    return dmCommToolSPtr;
}
//LCOV_EXCL_STOP

void OnBind(int32_t socket, PeerSocketInfo info)
{
    std::shared_ptr<DMCommTool> dmCommToolSPtr = GetDMCommToolPtr();
    if (dmCommToolSPtr == nullptr) {
        LOGE("Can not get DMCommTool ptr");
        return;
    }
    dmCommToolSPtr->GetDMTransportPtr()->OnSocketOpened(socket, info);
}

void OnShutdown(int32_t socket, ShutdownReason reason)
{
    std::shared_ptr<DMCommTool> dmCommToolSPtr = GetDMCommToolPtr();
    if (dmCommToolSPtr == nullptr) {
        LOGE("Can not get DMCommTool ptr");
        return;
    }
    dmCommToolSPtr->GetDMTransportPtr()->OnSocketClosed(socket, reason);
}

void OnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    std::shared_ptr<DMCommTool> dmCommToolSPtr = GetDMCommToolPtr();
    if (dmCommToolSPtr == nullptr) {
        LOGE("Can not get DMCommTool ptr");
        return;
    }
    dmCommToolSPtr->GetDMTransportPtr()->OnBytesReceived(socket, data, dataLen);
}

void OnMessage(int32_t socket, const void *data, uint32_t dataLen)
{
    (void)socket;
    (void)data;
    (void)dataLen;
    LOGI("socket: %{public}d, dataLen:%{public}" PRIu32, socket, dataLen);
}

void OnStream(int32_t socket, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    (void)socket;
    (void)data;
    (void)ext;
    (void)param;
    LOGI("socket: %{public}d", socket);
}

void OnFile(int32_t socket, FileEvent *event)
{
    (void)event;
    LOGI("socket: %{public}d", socket);
}

void OnQos(int32_t socket, QoSEvent eventId, const QosTV *qos, uint32_t qosCount)
{
    if (qosCount == 0 || qosCount > MAX_ROUND_SIZE) {
        LOGE("qosCount is invalid!");
        return;
    }
    LOGI("OnQos, socket: %{public}d, QoSEvent: %{public}d, qosCount: %{public}" PRIu32,
        socket, (int32_t)eventId, qosCount);
    for (uint32_t idx = 0; idx < qosCount; idx++) {
        LOGI("QosTV: type: %{public}d, value: %{public}d", (int32_t)qos[idx].qos, qos[idx].value);
    }
}

ISocketListener iSocketListener = {
    .OnBind = OnBind,
    .OnShutdown = OnShutdown,
    .OnBytes = OnBytes,
    .OnMessage = OnMessage,
    .OnStream = OnStream,
    .OnFile = OnFile,
    .OnQos = OnQos
};

int32_t DMTransport::CreateServerSocket()
{
    LOGI("CreateServerSocket start");
    localSocketName_ = DM_SYNC_USERID_SESSION_NAME;
    LOGI("CreateServerSocket , local socketName: %{public}s", localSocketName_.c_str());
    std::string dmPkgName(DM_PKG_NAME);
    SocketInfo info = {
        .name = const_cast<char*>(localSocketName_.c_str()),
        .pkgName = const_cast<char*>(dmPkgName.c_str()),
        .dataType = DATA_TYPE_BYTES
    };
    int32_t socket = Socket(info);
    LOGI("CreateServerSocket Finish, socket: %{public}d", socket);
    return socket;
}

int32_t DMTransport::CreateClientSocket(const std::string &rmtNetworkId)
{
    if (!IsIdLengthValid(rmtNetworkId)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("CreateClientSocket start, peerNetworkId: %{public}s", GetAnonyString(rmtNetworkId).c_str());
    std::string peerSocketName = DM_SYNC_USERID_SESSION_NAME;
    std::string dmPkgName(DM_PKG_NAME);
    SocketInfo info = {
        .name = const_cast<char*>(localSocketName_.c_str()),
        .peerName = const_cast<char*>(peerSocketName.c_str()),
        .peerNetworkId = const_cast<char*>(rmtNetworkId.c_str()),
        .pkgName = const_cast<char*>(dmPkgName.c_str()),
        .dataType = DATA_TYPE_BYTES
    };
    int32_t socket = Socket(info);
    LOGI("Bind Socket server, socket: %{public}d, localSocketName: %{public}s, peerSocketName: %{public}s",
        socket, localSocketName_.c_str(), peerSocketName.c_str());
    return socket;
}

int32_t DMTransport::Init()
{
    LOGI("Init DMTransport");
    if (isSocketSvrCreateFlag_.load()) {
        LOGI("SocketServer already create success.");
        return DM_OK;
    }
    int32_t socket = CreateServerSocket();
    if (socket < DM_OK) {
        LOGE("CreateSocketServer failed, ret: %{public}d", socket);
        return ERR_DM_FAILED;
    }

    int32_t ret = Listen(socket, g_qosInfo, g_qosTvParamIndex, &iSocketListener);
    if (ret != DM_OK) {
        LOGE("Socket Listen failed, error code %{public}d.", ret);
        return ERR_DM_FAILED;
    }
    isSocketSvrCreateFlag_.store(true);
    localServerSocket_ = socket;
    LOGI("Finish Init DSoftBus Server Socket, socket: %{public}d", socket);
    return DM_OK;
}

int32_t DMTransport::UnInit()
{
    {
        std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
        for (auto iter = remoteDevSocketIds_.begin(); iter != remoteDevSocketIds_.end(); ++iter) {
            for (auto iter1 = iter->second.begin(); iter1 != iter->second.end(); ++iter1) {
                LOGI("Shutdown client socket: %{public}d to remote dev: %{public}s", *iter1,
                    GetAnonyString(iter->first).c_str());
                Shutdown(*iter1);
            }
        }
        remoteDevSocketIds_.clear();
        sourceSocketIds_.clear();
    }

    if (!isSocketSvrCreateFlag_.load()) {
        LOGI("DSoftBus Server Socket already remove success.");
    } else {
        LOGI("Shutdown DSoftBus Server Socket, socket: %{public}d", localServerSocket_.load());
        Shutdown(localServerSocket_.load());
        localServerSocket_ = -1;
        isSocketSvrCreateFlag_.store(false);
    }
    return DM_OK;
}

bool DMTransport::IsDeviceSessionOpened(const std::string &rmtNetworkId, int32_t &socketId)
{
    if (!IsIdLengthValid(rmtNetworkId)) {
        return false;
    }
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    auto iter = remoteDevSocketIds_.find(rmtNetworkId);
    if (iter == remoteDevSocketIds_.end()) {
        return false;
    }
    for (auto iter1 = iter->second.begin(); iter1 != iter->second.end(); ++iter1) {
        if (sourceSocketIds_.find(*iter1) != sourceSocketIds_.end()) {
            socketId = *iter1;
            return true;
        }
    }
    return false;
}

std::string DMTransport::GetRemoteNetworkIdBySocketId(int32_t socketId)
{
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    std::string networkId = "";
    for (auto const &item : remoteDevSocketIds_) {
        if (item.second.find(socketId) != item.second.end()) {
            networkId = item.first;
            break;
        }
    }
    return networkId;
}

void DMTransport::ClearDeviceSocketOpened(const std::string &remoteDevId, int32_t socketId)
{
    if (!IsIdLengthValid(remoteDevId)) {
        return;
    }
    std::lock_guard<std::mutex> lock(rmtSocketIdMtx_);
    auto iter = remoteDevSocketIds_.find(remoteDevId);
    if (iter == remoteDevSocketIds_.end()) {
        return;
    }
    iter->second.erase(socketId);
    if (iter->second.empty()) {
        remoteDevSocketIds_.erase(iter);
    }
    sourceSocketIds_.erase(socketId);
}

int32_t DMTransport::StartSocket(const std::string &rmtNetworkId, int32_t &socketId)
{
    int32_t errCode = ERR_DM_FAILED;
    int32_t count = 0;
    const int32_t maxCount = 10;

    do {
        errCode = StartSocketInner(rmtNetworkId, socketId);
        if (errCode != ERR_DM_SOCKET_IN_USED) {
            break;
        }
        count++;
        usleep(USLEEP_TIME_US_200000);
    } while (count < maxCount);

    return errCode;
}

int32_t DMTransport::StartSocketInner(const std::string &rmtNetworkId, int32_t &socketId)
{
    if (!IsIdLengthValid(rmtNetworkId)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (IsDeviceSessionOpened(rmtNetworkId, socketId)) {
        LOGE("Softbus session has already opened, deviceId: %{public}s", GetAnonyString(rmtNetworkId).c_str());
        return ERR_DM_SOCKET_IN_USED;
    }

    int32_t socket = CreateClientSocket(rmtNetworkId);
    if (socket < DM_OK) {
        LOGE("StartSocket failed, ret: %{public}d", socket);
        return ERR_DM_FAILED;
    }

    int32_t ret = Bind(socket, g_qosInfo, g_qosTvParamIndex, &iSocketListener);
    if (ret < DM_OK) {
        if (ret == SOFTBUS_TRANS_SOCKET_IN_USE) {
            LOGI("Softbus trans socket in use.");
            return ERR_DM_SOCKET_IN_USED;
        }
        LOGE("OpenSession fail, rmtNetworkId: %{public}s, socket: %{public}d, ret: %{public}d",
            GetAnonyString(rmtNetworkId).c_str(), socket, ret);
        Shutdown(socket);
        return ERR_DM_FAILED;
    }

    LOGI("Bind Socket success, rmtNetworkId:%{public}s, socketId: %{public}d",
        GetAnonyString(rmtNetworkId).c_str(), socket);
    std::string peerSocketName = DM_SYNC_USERID_SESSION_NAME;
    std::string dmPkgName(DM_PKG_NAME);
    PeerSocketInfo peerSocketInfo = {
        .name = const_cast<char*>(peerSocketName.c_str()),
        .networkId = const_cast<char*>(rmtNetworkId.c_str()),
        .pkgName = const_cast<char*>(dmPkgName.c_str()),
        .dataType = DATA_TYPE_BYTES
    };
    OnSocketOpened(socket, peerSocketInfo);
    sourceSocketIds_.insert(socket);
    socketId = socket;
    return DM_OK;
}

int32_t DMTransport::StopSocket(const std::string &rmtNetworkId)
{
    if (!IsIdLengthValid(rmtNetworkId)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t socketId = -1;
    if (!IsDeviceSessionOpened(rmtNetworkId, socketId)) {
        LOGI("remote dev may be not opened, rmtNetworkId: %{public}s", GetAnonyString(rmtNetworkId).c_str());
        return ERR_DM_FAILED;
    }

    LOGI("StopSocket rmtNetworkId: %{public}s, socketId: %{public}d",
        GetAnonyString(rmtNetworkId).c_str(), socketId);
    Shutdown(socketId);
    ClearDeviceSocketOpened(rmtNetworkId, socketId);
    return DM_OK;
}

int32_t DMTransport::Send(const std::string &rmtNetworkId, const std::string &payload, int32_t socketId)
{
    if (!IsIdLengthValid(rmtNetworkId) || !IsMessageLengthValid(payload)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (socketId <= 0) {
        LOGI("The session is not open, target networkId: %{public}s", GetAnonyString(rmtNetworkId).c_str());
        return ERR_DM_FAILED;
    }
    uint32_t payLoadSize = payload.size();
    LOGI("Send payload size: %{public}" PRIu32 ", target networkId: %{public}s, socketId: %{public}d",
        static_cast<uint32_t>(payload.size()), GetAnonyString(rmtNetworkId).c_str(), socketId);

    if (payLoadSize > MAX_SEND_MSG_LENGTH) {
        LOGE("Send error: msg size: %{public}" PRIu32 " too long", payLoadSize);
        return ERR_DM_FAILED;
    }
    uint8_t *buf = reinterpret_cast<uint8_t *>(calloc((payLoadSize), sizeof(uint8_t)));
    if (buf == nullptr) {
        LOGE("Send: malloc memory failed");
        return ERR_DM_FAILED;
    }

    if (memcpy_s(buf, payLoadSize, reinterpret_cast<const uint8_t *>(payload.c_str()),
                 payLoadSize) != EOK) {
        LOGE("Send: memcpy memory failed");
        free(buf);
        return ERR_DM_FAILED;
    }

    int32_t ret = SendBytes(socketId, buf, payLoadSize);
    free(buf);
    if (ret != DM_OK) {
        LOGE("dsoftbus send error, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    LOGI("Send payload success");
    return DM_OK;
}
} // DistributedHardware
} // OHOS