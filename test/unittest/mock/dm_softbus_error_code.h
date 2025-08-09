/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_SOFTBUS_ERROR_CODE_H
#define OHOS_DM_SOFTBUS_ERROR_CODE_H

#include <memory>
#include <iostream>
#include "ipc_req.h"
#include "ipc_rsp.h"

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t SOFTBUS_IPC_ERR = -426442720;
constexpr int32_t SOFTBUS_INVALID_PARAM = -426442749;
constexpr int32_t SOFTBUS_PERMISSION_DENIED = -426442743;
constexpr int32_t SOFTBUS_NETWORK_NOT_INIT = -426049518;
constexpr int32_t SOFTBUS_WIFI_DIRECT_INIT_FAILED = -426442707;
constexpr int32_t SOFTBUS_ERR = -426442706;
constexpr int32_t SOFTBUS_DISCOVER_MANAGER_DUPLICATE_PARAM = -425799671;
constexpr int32_t SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL = -425799673;
constexpr int32_t SOFTBUS_TRANS_SESSION_SERVER_NOINIT = -426115031;
constexpr int32_t SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED = -426115035;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_ERROR_CODE_H