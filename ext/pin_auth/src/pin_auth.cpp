/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "pin_auth.h"

#include <memory>
#include <string>

#include "dm_common_util.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
PinAuth::PinAuth()
{
    pinAuthUi_ = std::make_shared<PinAuthUi>();
    LOGI("PinAuth constructor");
}

PinAuth::~PinAuth()
{
}

int32_t PinAuth::ShowAuthInfo(std::string &authToken, std::shared_ptr<DmAuthManager> authManager)
{
    JsonObject jsonObject(authToken);
    if (jsonObject.IsDiscarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, PIN_CODE_KEY)) {
        LOGE("err json string, first time");
        return ERR_DM_FAILED;
    }
    return pinAuthUi_->ShowPinDialog(jsonObject[PIN_CODE_KEY].Get<int32_t>(), authManager);
}

int32_t PinAuth::StartAuth(std::string &authToken, std::shared_ptr<DmAuthManager> authManager)
{
    return pinAuthUi_->InputPinDialog(authManager);
}

extern "C" IAuthentication *CreatePinAuthObject(void)
{
    return new PinAuth;
}
} // namespace DistributedHardware
} // namespace OHOS
