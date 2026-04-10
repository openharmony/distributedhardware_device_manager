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

#ifndef OHOS_SOFTBUS_SESSION_3RD_H
#define OHOS_SOFTBUS_SESSION_3RD_H

#include <map>
#include "ffrt.h"

#include "inner_session.h"
#include "session.h"

#include "device_manager_data_struct_3rd.h"
#include "isession_callback_3rd.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusSession3rd {
public:
    SoftbusSession3rd();
    ~SoftbusSession3rd();

    int32_t OpenSessionServer(const PeerTargetId3rd &targetId);
    int32_t SendData(int32_t sessionId, const std::string &message);
    int32_t CloseAuthSession(int32_t sessionId);

private:
    ConnectionAddr GetAddrByTargetId(const PeerTargetId3rd &targetId);
    ConnectionAddr CreateWifiAddr(const PeerTargetId3rd &targetId);
    ConnectionAddr CreateBleAddr(const PeerTargetId3rd &targetId);
    ConnectionAddr CreateBrAddr(const PeerTargetId3rd &targetId);
};
}
}
#endif // OHOS_SOFTBUS_SESSION_3RD_H