/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
 * You may obtain a copy of License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See License for the specific language governing permissions and
 * limitations under License.
 */

#include <algorithm>
#include <memory>
#include <vector>

#include "auth_manager_3rd.h"
#include "deviceprofile_connector_3rd.h"
#include "dm_auth_context_3rd.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_message_processor_3rd.h"
#include "dm_auth_state_3rd.h"
#include "dm_auth_state_machine_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_crypto_3rd.h"
#include "multiple_user_connector_3rd.h"

namespace OHOS {
namespace DistributedHardware {

int32_t AuthSinkDataSyncState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkDataSyncState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    // Query the ACL of the sink end. Compare the ACLs at both ends.
    // Synchronize the local SP information, the format is uncertain, not done for now
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    SaveAcl(context);
    context->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_RESP_DATA_SYNC, context);
    LOGI("AuthSinkDataSyncState::Action ok");
    return DM_OK;
}

DmAuthStateType AuthSinkDataSyncState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SINK_DATA_SYNC_STATE;
}

int32_t AuthSrcDataSyncState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcDataSyncState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->softbusConnector, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    SaveAcl(context);
    context->reason = DM_OK;
    context->reply = DM_OK;
    context->state = static_cast<int32_t>(GetStateType());
    context->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_REQ_FINISH, context);
    LOGI("AuthSrcDataSyncState::Action ok");
    return DM_OK;
}

DmAuthStateType AuthSrcDataSyncState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SRC_DATA_SYNC_STATE;
}

int32_t AuthSinkFinishState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkFinishState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    LOGI("reason: %{public}d", context->reason);

    SinkFinish(context);
    LOGI("AuthSinkFinishState::Action ok");
    if (context->cleanNotifyCallback != nullptr) {
        context->cleanNotifyCallback(context->logicalSessionId, context->connDelayCloseTime);
    }

    return DM_OK;
}

DmAuthStateType AuthSinkFinishState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SINK_FINISH_STATE;
}

int32_t AuthSrcFinishState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcFinishState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (context->reason != DM_OK) {
        context->authMessageProcessor->CreateAndSendMsg(DmMessageType::ACL_REQ_FINISH, context);
    } else {
        context->state = static_cast<int32_t>(GetStateType());
    }
    SourceFinish(context);
    LOGI("AuthSrcFinishState::Action ok");
    if (context->reason != DM_OK) {
        context->connDelayCloseTime = 0;
    }
    if (context->cleanNotifyCallback != nullptr) {
        context->cleanNotifyCallback(context->logicalSessionId, context->connDelayCloseTime);
    }
    return DM_OK;
}

DmAuthStateType AuthSrcFinishState::GetStateType()
{
    return DmAuthStateType::ACL_AUTH_SRC_FINISH_STATE;
}

} // namespace DistributedHardware
} // namespace OHOS
