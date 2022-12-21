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

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
const int32_t MAX_VERIFY_TIMES = 3;
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
    nlohmann::json jsonObject = nlohmann::json::parse(authToken, nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(jsonObject, PIN_CODE_KEY)) {
        LOGE("err json string, first time");
        return ERR_DM_FAILED;
    }
    return pinAuthUi_->ShowPinDialog(jsonObject[PIN_CODE_KEY].get<int32_t>(), authManager);
}

int32_t PinAuth::StartAuth(std::string &authToken, std::shared_ptr<DmAuthManager> authManager)
{
    return pinAuthUi_->InputPinDialog(authManager);
}

int32_t PinAuth::VerifyAuthentication(std::string &authToken, const std::string &authParam)
{
    times_ += 1;
    if (authParam.length() == 1) {
        if (authParam == std::string(EVENT_CONFIRM_CODE)) {
            return DM_OK;
        }
        LOGE("Peer rejection");
        return ERR_DM_FAILED;
    }
    nlohmann::json authParamJson = nlohmann::json::parse(authParam, nullptr, false);
    if (authParamJson.is_discarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }
    nlohmann::json authTokenJson = nlohmann::json::parse(authToken, nullptr, false);
    if (authTokenJson.is_discarded()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }
    if (!IsInt32(authTokenJson, PIN_CODE_KEY) || !IsString(authTokenJson, PIN_TOKEN) ||
        !IsInt32(authParamJson, PIN_CODE_KEY) || !IsString(authParamJson, PIN_TOKEN)) {
        LOGE("PinAuth::VerifyAuthentication err json string.");
        return ERR_DM_FAILED;
    }
    int32_t code = authTokenJson[PIN_CODE_KEY].get<int32_t>();
    std::string pinToken = authTokenJson[PIN_TOKEN].get<std::string>();
    int32_t inputPinCode = authParamJson[PIN_CODE_KEY].get<int32_t>();
    std::string inputPinToken = authParamJson[PIN_TOKEN].get<std::string>();
    if (code == inputPinCode && pinToken == inputPinToken) {
        return DM_OK;
    } else if (code != inputPinCode && times_ < MAX_VERIFY_TIMES) {
        return ERR_DM_INPUT_PARA_INVALID;
    } else {
        return ERR_DM_FAILED;
    }
}

extern "C" IAuthentication *CreatePinAuthObject(void)
{
    return new PinAuth;
}
} // namespace DistributedHardware
} // namespace OHOS
