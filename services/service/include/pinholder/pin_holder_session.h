/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_PIN_HOLDER_SESSION_H
#define OHOS_PIN_HOLDER_SESSION_H

#include "dm_device_info.h"
#include "pinholder_session_callback.h"

#include "inner_session.h"
#include "session.h"

namespace OHOS {
namespace DistributedHardware {
class PinHolderSession {
public:
    static int OnSessionOpened(int sessionId, int result);
    static void OnSessionClosed(int sessionId);
    static void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen);

public:
    PinHolderSession();
    ~PinHolderSession();

    /**
     * @tc.name: PinHolderSession::RegisterSessionCallback
     * @tc.desc: RegisterSessionCallback of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t RegisterSessionCallback(std::shared_ptr<IPinholderSessionCallback> callback);

    /**
     * @tc.name: PinHolderSession::UnRegisterSessionCallback
     * @tc.desc: UnRegister SessionCallback of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t UnRegisterSessionCallback();

    /**
     * @tc.name: PinHolderSession::OpenAuthSession
     * @tc.desc: Open AuthSession of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t OpenSessionServer(const PeerTargetId &targetId);

    /**
     * @tc.name: PinHolderSession::CloseAuthSession
     * @tc.desc: Close AuthSession of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t CloseSessionServer(int32_t sessionId);

    /**
     * @tc.name: PinHolderSession::SendData
     * @tc.desc: Send Data of the Softbus Session
     * @tc.type: FUNC
     */
    int32_t SendData(int32_t sessionId, const std::string &message);

private:
    int32_t GetAddrByTargetId(const PeerTargetId &targetId, ConnectionAddr &addrInfo);

private:
    static std::shared_ptr<IPinholderSessionCallback> pinholderSessionCallback_;
};
}
}
#endif // OHOS_PIN_HOLDER_SESSION_H