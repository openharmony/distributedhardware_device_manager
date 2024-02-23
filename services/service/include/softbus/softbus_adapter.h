/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_SOFTBUS_ADAPTER
#define OHOS_SOFTBUS_ADAPTER

#include <mutex>
#include <map>
#include <set>

#include "session.h"
#include "socket.h"
#include "single_instance.h"
namespace OHOS {
namespace DistributedHardware {
class SoftbusAdapter {
    DECLARE_SINGLE_INSTANCE_BASE(SoftbusAdapter);
public:
    SoftbusAdapter();
    ~SoftbusAdapter();
    int32_t CreateSoftbusSessionServer(const std::string &pkgname, const std::string &sessionName);
    int32_t RemoveSoftbusSessionServer(const std::string &pkgname, const std::string &sessionName);

    void OnSoftbusSessionOpened(int32_t socket, PeerSocketInfo info);
    void OnSoftbusSessionClosed(int32_t socket, ShutdownReason reason);
    void OnBytesReceived(int32_t socket, const void *data, uint32_t dataLen);
    void OnStreamReceived(int32_t socket, const StreamData *data, const StreamData *ext,
        const StreamFrameInfo *frameInfo);
    void OnMessageReceived(int32_t socket, const void *data, unsigned int dataLen) const;
    void OnQosEvent(int32_t socket, QoSEvent eventId, const QosTV *qos, uint32_t qosCount) const;

private:
    ISocketListener iSocketListener_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif