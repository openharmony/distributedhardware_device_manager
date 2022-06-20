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

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(HisyseventUtil);

void HisyseventUtil::SysEventWrite(std::string &status, DM_HISYEVENT_EventType eventType, std::string &msg)
{
    int32_t res = OHOS::HiviewDFX::HiSysEvent::Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        status.c_str(),
        (OHOS::HiviewDFX::HiSysEvent::EventType)eventType,
        "MSG", msg.c_str());
    if (res != DM_OK) {
        LOGE("%s  Write HiSysEvent error, res:%d", status.c_str(), res);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
