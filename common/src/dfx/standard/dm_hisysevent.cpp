/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dm_hisysevent.h"

#include "dm_log.h"        // for LOGE
#include "hisysevent.h"    // for HiSysEvent, HiSysEvent::Domain, HiSysEvent...
#include "unistd.h"        // for getpid, getuid

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DM_OK = 0;
void SysEventWrite(const std::string &status, int32_t eventType, const std::string &msg)
{
    HiSysEventParam params[] = {
        {.name = "PID", .t = HISYSEVENT_INT32, .v = { .i32 = getpid() }, .arraySize = 0, },
        {.name = "UID", .t = HISYSEVENT_INT32, .v = { .i32 = getuid() }, .arraySize = 0, },
        {.name = "MSG", .t = HISYSEVENT_STRING, .v = { .s = (char *)msg.c_str() }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        status.c_str(),
        (HiSysEventEventType)eventType, params, len);
    if (res != DM_OK) {
        LOGE("%{public}s Write HiSysEvent error, res:%{public}d", status.c_str(), res);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
