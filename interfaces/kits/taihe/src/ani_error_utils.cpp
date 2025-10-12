/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <algorithm>

#include "taihe/runtime.hpp"
#include "ani_error_utils.h"
#include "dm_error_type.h"

namespace ani_errorutils {
using namespace OHOS::DistributedHardware;

constexpr char CLASS_NAME_BUSINESSERROR[] = "@ohos.base.BusinessError";

const std::string ERR_MESSAGE_NO_PERMISSION = "Permission verify failed.";
const std::string ERR_MESSAGE_NOT_SYSTEM_APP = "The caller is not a system application.";
const std::string ERR_MESSAGE_INVALID_PARAMS = "Input parameter error.";
const std::string ERR_MESSAGE_FAILED = "Failed to execute the function.";
const std::string ERR_MESSAGE_OBTAIN_SERVICE = "Failed to obtain the service.";
const std::string ERR_MESSAGE_AUTHENTICALTION_INVALID = "Authentication invalid.";
const std::string ERR_MESSAGE_DISCOVERY_INVALID = "Discovery invalid.";
const std::string ERR_MESSAGE_PUBLISH_INVALID = "Publish invalid.";
const std::string ERR_MESSAGE_FROM_CLOUD_FAILED = "Get data from cloud failed.";
const std::string ERR_MESSAGE_NEED_LOGIN = "A login account is required.";
const std::string ERR_MESSAGE_SCAS_CHECK_FAILED = "The device name contains non-compliant content.";

void ThrowError(const char* message)
{
    if (message == nullptr) {
        return;
    }
    std::string errMsg(message);
    taihe::set_error(errMsg);
}

void ThrowError(int32_t code, const char* message)
{
    if (message == nullptr) {
        return;
    }
    std::string errMsg(message);
    taihe::set_business_error(code, errMsg);
}

ani_object ToBusinessError(ani_env *env, int32_t code, const std::string &message)
{
    if (env == nullptr) {
        return {};
    }
    ani_class cls {};
    CHECK_AND_RETURN_RET_LOG(env->FindClass(CLASS_NAME_BUSINESSERROR, &cls) == ANI_OK, nullptr,
        "find class %{public}s failed", CLASS_NAME_BUSINESSERROR);
    ani_method ctor {};
    CHECK_AND_RETURN_RET_LOG(env->Class_FindMethod(cls, "<ctor>", ":", &ctor) == ANI_OK, nullptr,
        "find method BusinessError constructor failed");
    ani_object error {};
    CHECK_AND_RETURN_RET_LOG(env->Object_New(cls, ctor, &error) == ANI_OK, nullptr,
        "new object %{public}s failed", CLASS_NAME_BUSINESSERROR);
    CHECK_AND_RETURN_RET_LOG(
        env->Object_SetPropertyByName_Int(error, "code", static_cast<ani_int>(code)) == ANI_OK, nullptr,
        "set property BusinessError.code failed");
    if (message.size() > 0) {
        ani_string messageRef {};
        CHECK_AND_RETURN_RET_LOG(env->String_NewUTF8(message.c_str(), message.size(), &messageRef) == ANI_OK, nullptr,
            "new message string failed");
        CHECK_AND_RETURN_RET_LOG(
            env->Object_SetPropertyByName_Ref(error, "message", static_cast<ani_ref>(messageRef)) == ANI_OK, nullptr,
            "set property BusinessError.message failed");
    }
    return error;
}

ani_object CreateErrorForCall(ani_env* env, int32_t code, const std::string &errMsg, bool isAsync)
{
    LOGI("CreateErrorForCall code:%{public}d, message:%{public}s", code, errMsg.c_str());
    ani_object error = nullptr;
    if (isAsync) {
        ThrowError(code, errMsg.c_str());
    } else {
        error = ToBusinessError(env, code, errMsg);
    }
    return error;
}

ani_object CreateBusinessError(int32_t errCode, bool isAsync)
{
    ani_env *env = taihe::get_env();
    ani_object error = nullptr;
    switch (errCode) {
        case ERR_DM_NO_PERMISSION:
            error = CreateErrorForCall(env, ERR_NO_PERMISSION, ERR_MESSAGE_NO_PERMISSION, isAsync);
            break;
        case ERR_DM_DISCOVERY_REPEATED:
            error = CreateErrorForCall(env, DM_ERR_DISCOVERY_INVALID, ERR_MESSAGE_DISCOVERY_INVALID, isAsync);
            break;
        case ERR_DM_PUBLISH_REPEATED:
            error = CreateErrorForCall(env, DM_ERR_PUBLISH_INVALID, ERR_MESSAGE_PUBLISH_INVALID, isAsync);
            break;
        case ERR_DM_AUTH_BUSINESS_BUSY:
            error = CreateErrorForCall(env, DM_ERR_AUTHENTICALTION_INVALID,
                ERR_MESSAGE_AUTHENTICALTION_INVALID, isAsync);
            break;
        case ERR_DM_INPUT_PARA_INVALID:
        case ERR_DM_UNSUPPORTED_AUTH_TYPE:
        case ERR_DM_CALLBACK_REGISTER_FAILED:
            error = CreateErrorForCall(env, ERR_INVALID_PARAMS, ERR_MESSAGE_INVALID_PARAMS, isAsync);
            break;
        case ERR_DM_INIT_FAILED:
            error = CreateErrorForCall(env, DM_ERR_OBTAIN_SERVICE, ERR_MESSAGE_OBTAIN_SERVICE, isAsync);
            break;
        case ERR_NOT_SYSTEM_APP:
            error = CreateErrorForCall(env, ERR_NOT_SYSTEM_APP, ERR_MESSAGE_NOT_SYSTEM_APP, isAsync);
            break;
        case ERR_DM_HILINKSVC_RSP_PARSE_FAILD:
        case ERR_DM_HILINKSVC_REPLY_FAILED:
        case ERR_DM_HILINKSVC_ICON_URL_EMPTY:
        case ERR_DM_HILINKSVC_DISCONNECT:
            error = CreateErrorForCall(env, DM_ERR_FROM_CLOUD_FAILED, ERR_MESSAGE_FROM_CLOUD_FAILED, isAsync);
            break;
        case ERR_DM_WISE_NEED_LOGIN:
            error = CreateErrorForCall(env, DM_ERR_NEED_LOGIN, ERR_MESSAGE_NEED_LOGIN, isAsync);
            break;
        case ERR_DM_HILINKSVC_SCAS_CHECK_FAILED:
            error = CreateErrorForCall(env, DM_ERR_SCAS_CHECK_FAILED, ERR_MESSAGE_SCAS_CHECK_FAILED, isAsync);
            break;
        default:
            error = CreateErrorForCall(env, DM_ERR_FAILED, ERR_MESSAGE_FAILED, isAsync);
            break;
    }
    return error;
}

bool AniPromiseCallback(ani_env* env, ani_resolver deferred, int32_t result, ani_ref resolveResult)
{
    ani_status status = ANI_OK;
    if (result != DM_OK) {
        ani_ref errobj = static_cast<ani_error>(CreateBusinessError(result, false));
        if (errobj == nullptr) {
            LOGE("ToBusinessError return null");
            env->GetUndefined(&errobj);
        }
        if ((status = env->PromiseResolver_Reject(deferred, static_cast<ani_error>(errobj))) != ANI_OK) {
            LOGE("PromiseResolver_Reject failed, status = %{public}d", status);
            return false;
        }
        return true;
    }
    ani_ref promiseResult = resolveResult;
    if (promiseResult == nullptr) {
        if ((status = env->GetUndefined(&promiseResult)) != ANI_OK) {
            LOGE("get undefined value failed, status = %{public}d", status);
            return false;
        }
    }
    if ((status = env->PromiseResolver_Resolve(deferred, promiseResult)) != ANI_OK) {
        LOGE("PromiseResolver_Resolve failed, status = %{public}d", status);
        return false;
    }
    return true;
}

} // namespace ani_errorutils