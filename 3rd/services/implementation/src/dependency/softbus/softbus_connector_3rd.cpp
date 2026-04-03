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

#include "softbus_connector_3rd.h"

#include "dm_log_3rd.h"

namespace OHOS {
namespace DistributedHardware {

SoftbusConnector3rd::SoftbusConnector3rd()
{
    softbusSession_ = std::make_shared<SoftbusSession3rd>();
    LOGD("SoftbusConnector3rd constructor.");
}

SoftbusConnector3rd::~SoftbusConnector3rd()
{
    LOGD("SoftbusConnector3rd destructor.");
}

std::shared_ptr<SoftbusSession3rd> SoftbusConnector3rd::GetSoftbusSession()
{
    return softbusSession_;
}
} // namespace DistributedHardware
} // namespace OHOS
