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
#ifndef OHOS_ANI_ERROR_UTILS_H
#define OHOS_ANI_ERROR_UTILS_H

#include <string>
#include <optional>
#include "taihe/runtime.hpp"
#include "dm_log.h"

namespace ani_errorutils {

enum DMBusinessErrorCode {
    ERR_OK = 0,
    ERR_NO_PERMISSION = 201,
    ERR_NOT_SYSTEM_APP = 202,
    ERR_INVALID_PARAMS = 401,
    DM_ERR_FAILED = 11600101,
    DM_ERR_OBTAIN_SERVICE = 11600102,
    DM_ERR_AUTHENTICATION_INVALID = 11600103,
    DM_ERR_DISCOVERY_INVALID = 11600104,
    DM_ERR_PUBLISH_INVALID = 11600105,
    DM_ERR_FROM_CLOUD_FAILED = 11600106,
    DM_ERR_NEED_LOGIN = 11600107,
    DM_ERR_SCAS_CHECK_FAILED = 11600108,
};

void ThrowError(const char *message);
void ThrowError(int32_t code, const char *message);

ani_object CreateBusinessError(int32_t errCode, bool isAsync = true);
ani_object ToBusinessError(ani_env *env, int32_t code, const std::string &message);

bool AniPromiseCallback(ani_env* env, ani_resolver deferred, int32_t errCode, ani_ref resolveResult = nullptr);

#define CHECK_AND_RETURN_RET_LOG(cond, ret, fmt, ...) \
    do {                                              \
        if (!(cond)) {                                \
            LOGE(fmt, ##__VA_ARGS__);           \
            return ret;                               \
        }                                             \
    } while (0)

}  // namespace ani_errorutils
#endif  // OHOS_ANI_ERROR_UTILS_H
