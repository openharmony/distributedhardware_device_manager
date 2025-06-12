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

#ifndef OHOS_DM_SOFTBUS_SESSION_H
#define OHOS_DM_SOFTBUS_SESSION_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "inner_session.h"
#include "session.h"
#include "socket.h"
#include "softbus_session_callback.h"

namespace OHOS {
namespace DistributedHardware {
class SoftbusSession {
public:
    static int OnSessionOpened(int sessionId, int result);
    static void OnSessionClosed(int sessionId);

public:
    SoftbusSession();
    ~SoftbusSession();

    /**
     * @tc.name: SoftbusSession::RegisterSessionCallback
     * @tc.desc: RegisterSessionCallback of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t RegisterSessionCallback(std::shared_ptr<ISoftbusSessionCallback> callback);

    /**
     * @tc.name: SoftbusSession::UnRegisterSessionCallback
     * @tc.desc: UnRegister SessionCallback of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t UnRegisterSessionCallback();

    /**
     * @tc.name: SoftbusSession::OpenAuthSession
     * @tc.desc: Open AuthSession of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t OpenAuthSession(const std::string &deviceId);

    /**
     * @tc.name: SoftbusSession::CloseAuthSession
     * @tc.desc: Close AuthSession of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t CloseAuthSession(int32_t sessionId);

    /**
     * @tc.name: SoftbusSession::SendData
     * @tc.desc: Send Data of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t SendData(int32_t sessionId, std::string &message);

    /**
     * @tc.name: SoftbusSession::GetPeerDeviceId
     * @tc.desc: Get Peer DeviceId of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t GetPeerDeviceId(int32_t sessionId, std::string &peerDevId);
    int32_t SendHeartbeatData(int32_t sessionId, std::string &message);

private:
    static std::shared_ptr<ISoftbusSessionCallback> sessionCallback_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_SESSION_H
