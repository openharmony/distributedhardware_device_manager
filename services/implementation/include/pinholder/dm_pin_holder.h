/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_PIN_HOLDER_H
#define OHOS_DM_PIN_HOLDER_H

#include "device_manager.h"
#include "dm_adapter_crypto.h"
#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "dm_timer.h"
#include "idevice_manager_service_listener.h"
#include "inner_session.h"
#include "pin_holder_session.h"
#include "session.h"

#include <map>
#include <mutex>

namespace OHOS {
namespace DistributedHardware {
typedef enum PinHolderState {
    SOURCE_INIT = 1,
    SOURCE_CREATE,
    SOURCE_DESTROY,
    SINK_INIT = 20,
    SINK_CREATE,
    SINK_DESTROY,
} PinHolderState;

class DmPinHolder final : public ISoftbusSessionCallback,
                          public std::enable_shared_from_this<DmPinHolder> {
public:
    DmPinHolder(std::shared_ptr<IDeviceManagerServiceListener> listener);
    ~DmPinHolder();
    int32_t RegisterPinHolderCallback(const std::string &pkgName);
    int32_t CreatePinHolder(const std::string &pkgName, const PeerTargetId &targetId,
        DmPinType pinType, const std::string &payload);
    int32_t DestroyPinHolder(const std::string &pkgName, const PeerTargetId &targetId,
        DmPinType pinType, const std::string &payload);
public:
    void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result);
    void OnSessionClosed(int32_t sessionId);
    void OnDataReceived(int32_t sessionId, std::string message);
    bool GetIsCryptoSupport()
    {
        return false;
    }
    AesGcmCipherKey GetSessionKeyAndLen()
    {
        AesGcmCipherKey cipherKey = { 0 };
        return cipherKey;
    }
private:
    int32_t CreateGeneratePinHolderMsg();
    int32_t ParseMsgType(const std::string &message);
    void ProcessCreateMsg(const std::string &message);
    void ProcessCreateRespMsg(const std::string &message);
    void ProcessDestroyMsg(const std::string &message);
    void ProcessDestroyResMsg(const std::string &message);
    void CloseSession(const std::string &name);
    void GetPeerDeviceId(int32_t sessionId, std::string &udidHash);
    int32_t CheckTargetIdVaild(const PeerTargetId &targetId);
private:
    std::shared_ptr<IDeviceManagerServiceListener> listener_ = nullptr;
    std::shared_ptr<PinHolderSession> session_ = nullptr;
    std::shared_ptr<DmTimer> timer_ = nullptr;

    std::string registerPkgName_ = "";
    std::string remoteDeviceId_ = "";
    std::string payload_ = "";
    DmPinType pinType_;
    PinHolderState sinkState_;
    PinHolderState sourceState_;
    int32_t sessionId_ = -1;
};
}
}
#endif // OHOS_DM_PIN_HOLDER_H