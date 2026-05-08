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

#include "auth_pincode_manager_3rd.h"
#include "dm_auth_manager_base_3rd.h"
#include "dm_auth_pincode_context_3rd.h"
#include "dm_auth_pincode_message_processor_3rd.h"
#include "dm_auth_pincode_state_3rd.h"
#include "dm_auth_pincode_state_machine_3rd.h"
#include "dm_constants_3rd.h"

namespace OHOS {
namespace DistributedHardware {

int32_t AuthPincodeSinkFinishState::Action(std::shared_ptr<DmAuthPincodeContext> context)
{
    LOGI("AuthPincodeSinkFinishState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    LOGI("reason: %{public}d", context->reason);

    SinkFinish(context);
    LOGI("AuthPincodeSinkFinishState::Action ok");
    if (context->cleanNotifyCallback != nullptr) {
        context->cleanNotifyCallback(context->logicalSessionId, context->connDelayCloseTime);
    }

    return DM_OK;
}

DmAuthPincodeStateType AuthPincodeSinkFinishState::GetStateType()
{
    return DmAuthPincodeStateType::ACL_AUTH_PINCODE_SINK_FINISH_STATE;  // 55
}

int32_t AuthPincodeSrcFinishState::Action(std::shared_ptr<DmAuthPincodeContext> context)
{
    LOGI("AuthPincodeSrcFinishState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (context->reason != DM_OK) {
        context->authPinMsgProc3rd->CreateAndSendMsg(DmPincodeMessageType::AUTH_PINCODE_REQ_FINISH, context);
    } else {
        context->state = static_cast<int32_t>(GetStateType());
    }
    SourceFinish(context);
    LOGI("AuthPincodeSrcFinishState::Action ok");
    if (context->reason != DM_OK) {
        context->connDelayCloseTime = 0;
    }
    if (context->cleanNotifyCallback != nullptr) {
        context->cleanNotifyCallback(context->logicalSessionId, context->connDelayCloseTime);
    }
    return DM_OK;
}

DmAuthPincodeStateType AuthPincodeSrcFinishState::GetStateType()
{
    return DmAuthPincodeStateType::ACL_AUTH_PINCODE_SRC_FINISH_STATE;  // 6
}
} // namespace DistributedHardware
} // namespace OHOS
