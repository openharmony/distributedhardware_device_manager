/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include <cstdlib>
#include <set>
#include <sstream>

#include "device_manager_service_listener.h"
#include "app_manager.h"
#include "device_manager_ipc_interface_code.h"
#include "device_manager_service_notify.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"
#include "ipc_notify_auth_result_req.h"
#include "ipc_notify_bind_result_req.h"
#include "ipc_notify_device_found_req.h"
#include "ipc_notify_device_state_req.h"
#include "ipc_notify_discover_result_req.h"
#include "ipc_notify_publish_result_req.h"
#include "ipc_server_stub.h"

#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "datetime_ex.h"
#include "device_name_manager.h"
#include "device_manager_service.h"
#include "kv_adapter_manager.h"
#include "multiple_user_connector.h"
#endif

#include "parameter.h"
#include "permission_manager.h"

namespace OHOS {
namespace DistributedHardware {