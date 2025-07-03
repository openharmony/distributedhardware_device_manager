/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <fuzzer/FuzzedDataProvider.h>

#include "dm_comm_tool_fuzzer.h"
#include "dm_comm_tool.h"


namespace OHOS {
namespace DistributedHardware {

namespace {
    constexpr uint32_t DATA_LEN = 10;
}

std::shared_ptr<DMCommTool> dmCommToolPtr_ = std::make_shared<DMCommTool>();

void DmCommToolFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t socketId = fdp.ConsumeIntegral<int32_t>();
    std::string rmtNetworkId(reinterpret_cast<const char*>(data), size);
    dmCommToolPtr_->Init();
    dmCommToolPtr_->GetInstance();
    dmCommToolPtr_->UnInit();
    std::vector<uint32_t> foregroundUserIds;
    foregroundUserIds.push_back(DATA_LEN);
    std::vector<uint32_t> backgroundUserIds;
    dmCommToolPtr_->SendUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds);
    dmCommToolPtr_->RspLocalFrontOrBackUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds, socketId);
    rmtNetworkId = "";
    dmCommToolPtr_->SendUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds);
    std::string remoteNetworkId(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    std::shared_ptr<InnerCommMsg> innerCommMsg = std::make_shared<InnerCommMsg>(remoteNetworkId, commMsg, socketId);
    dmCommToolPtr_->ProcessReceiveUserIdsEvent(innerCommMsg);
    dmCommToolPtr_->ProcessResponseUserIdsEvent(innerCommMsg);
    dmCommToolPtr_->ProcessReceiveCommonEvent(innerCommMsg);
    dmCommToolPtr_->ProcessResponseCommonEvent(innerCommMsg);
    dmCommToolPtr_->ProcessReceiveUninstAppEvent(innerCommMsg);
    dmCommToolPtr_->ProcessReceiveUnBindAppEvent(innerCommMsg);
    dmCommToolPtr_->ProcessReceiveRspAppUninstallEvent(innerCommMsg);
    dmCommToolPtr_->ProcessReceiveRspAppUnbindEvent(innerCommMsg);
    dmCommToolPtr_->GetDMTransportPtr();
    dmCommToolPtr_->GetEventHandler();
}

void DmCommToolFirstFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    DMCommTool::DMCommToolEventHandler dmCommToolEventHandler(
        AppExecFwk::EventRunner::Create(FUZZ_PROJECT_NAME), dmCommToolPtr_);
     
    int32_t socketId = fdp.ConsumeIntegral<int32_t>();
    std::string rmtNetworkId = fdp.ConsumeRandomLengthString();
    int32_t code = fdp.ConsumeIntegral<int32_t>();
    std::string msg = fdp.ConsumeRandomLengthString();
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>(code, msg);
    std::shared_ptr<InnerCommMsg> innrCommMsg = std::make_shared<InnerCommMsg>(rmtNetworkId, commMsg, socketId);
    UserIdsMsg userIdsMsg;
    userIdsMsg.foregroundUserIds.push_back(DATA_LEN);
    dmCommToolEventHandler.ParseUserIdsMsg(innrCommMsg, userIdsMsg);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(commMsg->code, innrCommMsg);
    dmCommToolEventHandler.ProcessEvent(event);
    dmCommToolPtr_->ProcessResponseUserStopEvent(innrCommMsg);
    std::string commonEventType;
    EventCallback eventCallback;
    dmCommToolPtr_->StartCommonEvent(commonEventType, eventCallback);
    dmCommToolPtr_->Init();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t tokenId = fdp.ConsumeIntegral<int32_t>();
    dmCommToolPtr_->SendUninstAppObj(userId, tokenId, rmtNetworkId);
    dmCommToolPtr_->RspAppUninstall(rmtNetworkId, socketId);
    dmCommToolPtr_->RspAppUnbind(rmtNetworkId, socketId);
    std::string udid = fdp.ConsumeRandomLengthString();
    dmCommToolPtr_->SendUnBindAppObj(userId, tokenId, msg, rmtNetworkId, udid);
    dmCommToolPtr_->StopSocket(rmtNetworkId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmCommToolFuzzTest(data, size);
    OHOS::DistributedHardware::DmCommToolFirstFuzzTest(data, size);
    return 0;
}
