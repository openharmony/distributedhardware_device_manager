/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "spatial_location_callback_impl.h"
#include "dm_auth_state.h"
#include "multiple_user_connector.h"
#include "dm_auth_state_machine.h"

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t pinCodeLength = 6;

SpatialLocationCallbackImpl::SpatialLocationCallbackImpl(std::shared_ptr<DmAuthContext> context)
{
    LOGI("SpatialLocationCallbackImpl Init.");
    context_ = context;
}

SpatialLocationCallbackImpl::~SpatialLocationCallbackImpl()
{
    LOGI("SpatialLocationCallbackImpl End.");
}

void SpatialLocationCallbackImpl::OnPinCodeChanged(const Msdp::PinCodeResponse &pinCodeResponse)
{
    LOGI("Start.");
    if (pinCodeResponse.pincode.length() != pinCodeLength) {
        LOGE("OnPinCodeChanged pincode length error.");
        return;
    }
    if (!IsValidPinCodeStr(pinCodeResponse.pincode)) {
        LOGE("OnPinCodeChanged pincode is invalid");
        return;
    }
    if (context_ == nullptr) {
        LOGE("OnPinCodeChanged context_ empty.");
        return;
    }
    context_->pinCode = pinCodeResponse.pincode;
    context_->authStateMachine->NotifyEventFinish(DmEventType::ON_ULTRASONIC_PIN_CHANGED);
}

bool SpatialLocationCallbackImpl::IsValidPinCodeStr(const std::string &pinCodeStr)
{
    for (auto &digit : pinCodeStr) {
        if (digit < '0' || digit > '9') {
            return false;
        }
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS