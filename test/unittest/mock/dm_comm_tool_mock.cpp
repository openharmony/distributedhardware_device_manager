/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dm_comm_tool_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DMCommTool::SendUserIds(const std::string rmtNetworkId,
    const std::vector<uint32_t> &foregroundUserIds, const std::vector<uint32_t> &backgroundUserIds)
{
    return DmDMCommTool::dmDMCommTool->SendUserIds(rmtNetworkId, foregroundUserIds, backgroundUserIds);
}

int32_t DMCommTool::SendUserStop(const std::string rmtNetworkId, int32_t stopUserId)
{
    return DmDMCommTool::dmDMCommTool->SendUserStop(rmtNetworkId, stopUserId);
}

int32_t DMCommTool::CreateUserStopMessage(int32_t stopUserId, std::string &msgStr)
{
    return DmDMCommTool::dmDMCommTool->CreateUserStopMessage(stopUserId, msgStr);
}

int32_t DMCommTool::SendUninstAppObj(int32_t userId, int32_t tokenId, const std::string &networkId)
{
    return DmDMCommTool::dmDMCommTool->SendUninstAppObj(userId, tokenId, networkId);
}
} // namespace DistributedHardware
} // namespace OHOS