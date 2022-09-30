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

#include "dm_constants.h"  // for DM_OK
#include "dm_log.h"        // for LOGE
#include "hisysevent.h"    // for HiSysEvent, HiSysEvent::Domain, HiSysEvent...
#include "unistd.h"        // for getpid, getuid

namespace OHOS {
namespace DistributedHardware {
void SysEventWrite(const std::string &status, int32_t eventType, const std::string &msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        status.c_str(),
        (OHOS::HiviewDFX::HiSysEvent::EventType)eventType,
        "PID", getpid(),
        "UID", getuid(),
        "MSG", msg.c_str());
    if (res != DM_OK) {
        LOGE("%s Write HiSysEvent error, res:%d", status.c_str(), res);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
