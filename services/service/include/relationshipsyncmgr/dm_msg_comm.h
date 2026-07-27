/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_MSG_COMM_H
#define OHOS_DM_MSG_COMM_H

#include <string>
#include "foreground_account_info.h"

namespace OHOS {
namespace DistributedHardware {
enum class AccountEventType : uint32_t {
    SEND_ACCOUNT = 0,
    RESP_ACCOUNT = 1,
    ACCOUNT_DELETED = 2,
    ACCOUNT_UNBOUND = 3,
    ACCOUNT_LOGOUT = 4,
    TYPE_MAX = 5,
};
} // DistributedHardware
} // OHOS
#endif