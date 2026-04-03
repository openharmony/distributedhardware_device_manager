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

#ifndef OHOS_DM_IPC_REMOTE_BROKER_3RD_H
#define OHOS_DM_IPC_REMOTE_BROKER_3RD_H

#include <cstdint>
#include <memory>

#include "iremote_broker.h"
#include "device_manager_data_struct_3rd.h"

namespace OHOS { class MessageOption; }
namespace OHOS { class MessageParcel; }
namespace OHOS {
namespace DistributedHardware {
class IpcRemoteBroker3rd : public OHOS::IRemoteBroker {
public:
    ~IpcRemoteBroker3rd() override {}
    virtual int32_t SendCmd(int32_t cmdCode, const ProcessInfo3rd &processInfo, int32_t result, int32_t status,
        const std::string &authContent) = 0;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.distributedhardware.devicemanager");
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_REMOTE_BROKER_3RD_H