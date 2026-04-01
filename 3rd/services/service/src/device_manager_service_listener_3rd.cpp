/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <set>
#include <sstream>

#include "device_manager_service_listener_3rd.h"
#include "ipc_service_stub_3rd.h"
#include "ipc_interface_code_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_log_3rd.h"
#include "dm_error_type_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "ipc_remote_broker_3rd.h"

namespace OHOS {
namespace DistributedHardware {

void DeviceManagerServiceListener3rd::OnAuthResult(const ProcessInfo3rd &processInfo, int32_t result, int32_t status,
    const std::string &authContent)
{
    sptr<IpcRemoteBroker3rd> listener = IpcServiceStub3rd::GetInstance().GetDmListener(processInfo);
    if (listener == nullptr) {
        LOGE("cannot get listener for package:%{public}s.", processInfo.businessName.c_str());
        return;
    }

    if (listener->SendCmd(ON_AUTH_RESULT_3RD, processInfo, result, status, authContent) != DM_OK) {
        LOGE("SendRequest fail");
        return;
    }
}
} // namespace DistributedHardware
} // namespace OHOS
