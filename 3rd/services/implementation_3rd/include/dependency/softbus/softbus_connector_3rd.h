/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_SOFTBUS_CONNECTOR_3RD_H
#define OHOS_DM_SOFTBUS_CONNECTOR_3RD_H

#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <set>

#include "softbus_bus_center.h"
#include "device_manager_data_struct_3rd.h"
#include "deviceprofile_connector.h"
#include "softbus_session_3rd.h"
#include "hichain_auth_connector.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusConnector3rd {

public:
    SoftbusConnector3rd();
    ~SoftbusConnector3rd();
    std::shared_ptr<SoftbusSession3rd> GetSoftbusSession();

private:
    static std::string remoteUdidHash_;
    std::shared_ptr<SoftbusSession3rd> softbusSession_;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector_;
    static std::unordered_map<std::string, std::string> deviceUdidMap_;
    static std::mutex discoveryDeviceInfoMutex_;
    static std::mutex deviceUdidLocks_;
    static std::mutex processInfoVecMutex_;
    static std::mutex processChangeInfoVecMutex_;
    static std::mutex leaveLNNMutex_;
    static std::map<std::string, std::string> leaveLnnPkgMap_;
    static std::mutex dmDelInfoMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_CONNECTOR_3RD_H
