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

#include "dm_comm_tool_two_fuzzer.h"
#include "dm_comm_tool.h"


namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<DMCommTool> dmCommToolPtr_ = std::make_shared<DMCommTool>();

void GenerateUserIds(FuzzedDataProvider& fdp, std::vector<uint32_t>& outIds)
{
    outIds.clear();

    auto count = fdp.ConsumeIntegralInRange<size_t>(0, 10);

    while(count-- > 0 && fdp.remaining_bytes() >= sizeof(uint32_t)) {
        outIds.push_back(fdp.ConsumeIntegral<uint32_t>());
    }
}

void DmCommToolTwoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    int32_t commCode = fdp.ConsumeIntegral<int32_t>();
    std::string commMsgStr = fdp.ConsumeRandomLengthString();
    auto commMsgPtr = std::make_shared<CommMsg>(commCode, commMsgStr);

    std::string networkId = fdp.ConsumeRandomLengthString();
    int32_t socketId = fdp.ConsumeIntegral<int32_t>();
    auto innerCommMsg = std::make_shared<InnerCommMsg>(networkId, commMsgPtr, socketId);

    std::string accountId = fdp.ConsumeRandomLengthString();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    
    dmCommToolPtr_->ProcessReceiveUserIdsEvent(innerCommMsg);
    dmCommToolPtr_->SendLogoutAccountInfo(networkId, accountId, userId);
    dmCommToolPtr_->ProcessReceiveLogoutEvent(innerCommMsg);
    dmCommToolPtr_->ProcessReceiveCommonEvent(innerCommMsg);

    std::vector<uint32_t> foregroundIds;
    std::vector<uint32_t> backgroundIds;

    GenerateUserIds(fdp, foregroundIds);
    GenerateUserIds(fdp, backgroundIds);

    std::string msgStr = fdp.ConsumeRandomLengthString();
    int32_t stopUserId = fdp.ConsumeIntegral<int32_t>();

    dmCommToolPtr_->SendUserIds(networkId, foregroundIds, backgroundIds);
    dmCommToolPtr_->RspLocalFrontOrBackUserIds(networkId, foregroundIds, backgroundIds, socketId);

    dmCommToolPtr_->CreateUserStopMessage(stopUserId, msgStr);
    dmCommToolPtr_->SendMsg(networkId, fdp.ConsumeIntegral<int32_t>(), msgStr);
    dmCommToolPtr_->SendUserStop(networkId, stopUserId);

    int32_t parsedUserId = fdp.ConsumeIntegral<int32_t>();
    dmCommToolPtr_->ParseUserStopMessage(msgStr, parsedUserId);

    dmCommToolPtr_->ProcessReceiveUserStopEvent(innerCommMsg);
    dmCommToolPtr_->RspUserStop(networkId, socketId, stopUserId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmCommToolTwoFuzzTest(data, size);
    return 0;
}
