/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_COMM_TOOL_H
#define OHOS_DM_COMM_TOOL_H

#include "dm_transport.h"
#include "dm_transport_msg.h"

namespace OHOS {
namespace DistributedHardware {

class DMCommTool : public std::enable_shared_from_this<DMCommTool> {
public:
    DMCommTool();
    virtual ~DMCommTool() = default;
    static std::shared_ptr<DMCommTool> GetInstance();
    void Init();
    void UnInit();

    int32_t SendUserIds(const std::string rmtNetworkId, const std::vector<uint32_t> &foregroundUserIds,
        const std::vector<uint32_t> &backgroundUserIds);
    void RspLocalFrontOrBackUserIds(const std::string rmtNetworkId, const std::vector<uint32_t> &foregroundUserIds,
        const std::vector<uint32_t> &backgroundUserIds, int32_t socketId);
    int32_t CreateUserStopMessage(int32_t stopUserId, std::string &msgStr);
    int32_t SendMsg(const std::string rmtNetworkId, int32_t msgType, const std::string &msg);
    int32_t SendUserStop(const std::string rmtNetworkId, int32_t stopUserId);
    int32_t ParseUserStopMessage(const std::string &msgStr, int32_t &stopUserId);
    void ProcessReceiveUserStopEvent(const std::shared_ptr<InnerCommMsg> commMsg);
    void RspUserStop(const std::string rmtNetworkId, int32_t socketId, int32_t stopUserId);
    void ProcessResponseUserStopEvent(const std::shared_ptr<InnerCommMsg> commMsg);

    class DMCommToolEventHandler : public AppExecFwk::EventHandler {
    public:
        DMCommToolEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner,
            std::shared_ptr<DMCommTool> dmCommToolPtr);
        ~DMCommToolEventHandler() override = default;
        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    private:
        std::weak_ptr<DMCommTool> dmCommToolWPtr_;
    };
    std::shared_ptr<DMCommTool::DMCommToolEventHandler> GetEventHandler();
    const std::shared_ptr<DMTransport> GetDMTransportPtr();

    void ProcessReceiveUserIdsEvent(const std::shared_ptr<InnerCommMsg> commMsg);
    void ProcessResponseUserIdsEvent(const std::shared_ptr<InnerCommMsg> commMsg);
private:
    std::shared_ptr<DMTransport> dmTransportPtr_;
    std::shared_ptr<DMCommTool::DMCommToolEventHandler> eventHandler_;
};
} // DistributedHardware
} // OHOS
#endif