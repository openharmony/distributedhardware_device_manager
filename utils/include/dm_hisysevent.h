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

#ifndef OHOS_DISTRIBUTED_DM_HISYSEVENT_H
#define OHOS_DISTRIBUTED_DM_HISYSEVENT_H

#include <cstring>
#include <unistd.h>

#include "single_instance.h"
#include "hisysevent.h"
#include "dm_log.h"
#include "dm_constants.h"
#include "dm_anonymous.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedDM {
#define DM_INIT_DEVICE_MANAGER_SUCCESS "DM_INIT_DEVICE_MANAGER_SUCCESS"
#define DM_INIT_DEVICE_MANAGER_FAILED "DM_INIT_DEVICE_MANAGER_FAILED"
#define START_DEVICE_DISCOVERY_SUCCESS "START_DEVICE_DISCOVERY_SUCCESS"
#define START_DEVICE_DISCOVERY_FAILED "START_DEVICE_DISCOVERY_FAILED"
#define GET_LOCAL_DEVICE_INFO_SUCCESS "GET_LOCAL_DEVICE_INFO_SUCCESS"
#define GET_LOCAL_DEVICE_INFO_FAILED "GET_LOCAL_DEVICE_INFO_FAILED"
#define DM_SEND_REQUEST_SUCCESS "DM_SEND_REQUEST_SUCCESS"
#define DM_SEND_REQUEST_FAILED "DM_SEND_REQUEST_FAILED"
#define ADD_HICHAIN_GROUP_SUCCESS "ADD_HICHAIN_GROUP_SUCCESS"
#define ADD_HICHAIN_GROUP_FAILED "ADD_HICHAIN_GROUP_FAILED"
#define DM_CREATE_GROUP_SUCCESS "DM_CREATE_GROUP_SUCCESS"
#define DM_CREATE_GROUP_FAILED "DM_CREATE_GROUP_FAILED"
#define UNAUTHENTICATE_DEVICE_SUCCESS "UNAUTHENTICATE_DEVICE_SUCCESS"
#define UNAUTHENTICATE_DEVICE_FAILED "UNAUTHENTICATE_DEVICE_FAILED"

class HisyseventUtil {
DECLARE_SINGLE_INSTANCE_BASE(HisyseventUtil);
public:
    void SysEventWrite(std::string status, OHOS::HiviewDFX::HiSysEvent::EventType eventType, std::string msg);
    void SysEventWrite(std::string status, OHOS::HiviewDFX::HiSysEvent::EventType eventType,
        std::string devId, std::string msg);
    void SysEventWrite(std::string status, OHOS::HiviewDFX::HiSysEvent::EventType eventType,
        std::string devId, std::string dhId, std::string msg);
private:
    HisyseventUtil() = default;
    ~HisyseventUtil() = default;
};
}  // namespace DistributedDM
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DISTRIBUTED_DM_HISYSEVENT_H
