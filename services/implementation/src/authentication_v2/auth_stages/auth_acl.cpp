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

#include <algorithm>
#include <vector>

#include "auth_manager.h"
#include "deviceprofile_connector.h"
#include "dm_auth_context.h"
#include "dm_auth_state.h"
#include "dm_auth_state_machine.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_freeze_process.h"
#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {

const int32_t USLEEP_TIME_US_500000 = 500000; // 500ms

// Received 180 synchronization message, send 190 message
int32_t AuthSinkDataSyncState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkDataSyncState::Action start");
    // Query the ACL of the sink end. Compare the ACLs at both ends.
    context->softbusConnector->SyncLocalAclListProcess({context->accessee.deviceId, context->accessee.userId},
        {context->accesser.deviceId, context->accesser.userId}, context->accesser.aclStrList);

    // Synchronize the local SP information, the format is uncertain, not done for now
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_DATA_SYNC, context);
    context->accessee.deviceName = context->softbusConnector->GetLocalDeviceName();
    LOGI("AuthSinkDataSyncState::Action ok");
    return DM_OK;
}

DmAuthStateType AuthSinkDataSyncState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_DATA_SYNC_STATE;
}

// Received 190 message, sent 200 message
int32_t AuthSrcDataSyncState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcDataSyncState::Action start");

    if (NeedAgreeAcl(context)) {
        // Query the ACL of the sink end. Compare the ACLs at both ends.
        context->softbusConnector->SyncLocalAclListProcess({context->accesser.deviceId, context->accesser.userId},
            {context->accessee.deviceId, context->accessee.userId}, context->accessee.aclStrList);
        context->accesser.deviceName = context->softbusConnector->GetLocalDeviceName();
        // Save this acl
        SetAclInfo(context);
        context->authMessageProcessor->PutAccessControlList(context, context->accesser, context->accessee.deviceId);
        // Synchronize the local SP information, the format is uncertain, not done for now
    }

    std::string peerDeviceId = "";
    peerDeviceId = context->accesser.aclProfiles[DM_IDENTICAL_ACCOUNT].GetAccessee().GetAccesseeDeviceId();
    if (peerDeviceId.empty()) {
        peerDeviceId = context->accesser.aclProfiles[DM_SHARE].GetAccessee().GetAccesseeDeviceId();
    }
    if (peerDeviceId.empty()) {
        peerDeviceId = context->accesser.aclProfiles[DM_POINT_TO_POINT].GetAccessee().GetAccesseeDeviceId();
    }
    bool isNeedJoinLnn = context->softbusConnector->CheckIsNeedJoinLnn(peerDeviceId, context->accessee.addr);
    // Trigger networking
    if (!context->accesser.isOnline || isNeedJoinLnn) {
        if (context->connSessionType == CONN_SESSION_TYPE_HML) {
            context->softbusConnector->JoinLnnByHml(context->sessionId, context->accesser.transmitSessionKeyId,
                context->accessee.transmitSessionKeyId);
        } else {
            char udidHashTmp[DM_MAX_DEVICE_ID_LEN] = {0};
            if (Crypto::GetUdidHash(context->accessee.deviceId, reinterpret_cast<uint8_t*>(udidHashTmp)) != DM_OK) {
                LOGE("AuthSrcDataSyncState joinLnn get udidhash by udid: %{public}s failed",
                    GetAnonyString(context->accessee.deviceId).c_str());
                return ERR_DM_FAILED;
            }
            std::string peerUdidHash = std::string(udidHashTmp);
            context->softbusConnector->JoinLNNBySkId(context->sessionId, context->accesser.transmitSessionKeyId,
                context->accessee.transmitSessionKeyId, context->accessee.addr, peerUdidHash);
        }
    }
    context->reason = DM_OK;
    context->reply = DM_OK;
    context->state = static_cast<int32_t>(GetStateType());
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_AUTH_REQ_FINISH, context);
    LOGI("AuthSrcDataSyncState::Action ok");
    return DM_OK;
}

DmAuthStateType AuthSrcDataSyncState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_DATA_SYNC_STATE;
}

// Received 200 end message, send 201
int32_t AuthSinkFinishState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkFinishState::Action start");
    int32_t ret = DM_OK;
    LOGI("reason: %{public}d", context->reason);
    if (context->reason == DM_OK) {
        context->state = static_cast<int32_t>(GetStateType());
        ret = FreezeProcess::GetInstance().DeleteFreezeRecord();
        LOGI("DeleteFreezeRecord ret: %{public}d", ret);
    }
    if (context->reason == ERR_DM_BIND_PIN_CODE_ERROR) {
        ret = FreezeProcess::GetInstance().UpdateFreezeRecord();
        LOGI("UpdateFreezeData ret: %{public}d", ret);
    }
    SinkFinish(context);
    LOGI("AuthSinkFinishState::Action ok");
    if (context->cleanNotifyCallback != nullptr) {
        context->cleanNotifyCallback(context->logicalSessionId);
    }

    return DM_OK;
}

DmAuthStateType AuthSinkFinishState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_FINISH_STATE;
}

// Received 201 end message
int32_t AuthSrcFinishState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcFinishState::Action start");
    if (context->reason != DM_OK) {
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_AUTH_REQ_FINISH, context);
    } else {
        context->state = static_cast<int32_t>(GetStateType());
    }
    SourceFinish(context);
    LOGI("AuthSrcFinishState::Action ok");
    std::shared_ptr<DmAuthContext> tempContext = context;
    auto taskFunc = [this, tempContext]() {
        if (tempContext->cleanNotifyCallback != nullptr) {
            tempContext->cleanNotifyCallback(tempContext->logicalSessionId);
        }
    };
    const int64_t MICROSECOND_PER_SECOND = 1000000L;
    int32_t delaySeconds = context->connDelayCloseTime;
    if (context->reason != DM_OK && context->reason != DM_ALREADY_AUTHED) {
        delaySeconds = 0;
    }
    ffrt::submit(taskFunc, ffrt::task_attr().delay(delaySeconds * MICROSECOND_PER_SECOND));
    return DM_OK;
}

DmAuthStateType AuthSrcFinishState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_FINISH_STATE;
}
} // namespace DistributedHardware
} // namespace OHOS