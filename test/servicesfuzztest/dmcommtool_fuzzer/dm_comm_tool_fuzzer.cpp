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
    dmCommToolPtr_->GetDMTransportPtr();
    dmCommToolPtr_->GetEventHandler();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmCommToolFuzzTest(data, size);
    return 0;
}
