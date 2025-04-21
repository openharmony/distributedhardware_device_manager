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

#include "softbus_session_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {

int32_t SoftbusSession::GetPeerDeviceId(int32_t sessionId, std::string &peerDevId)
{
    return DmSoftbusSession::dmSoftbusSession->GetPeerDeviceId(sessionId, peerDevId);
}

int32_t SoftbusSession::SendData(int32_t sessionId, std::string &message)
{
    return DmSoftbusSession::dmSoftbusSession->SendData(sessionId, message);
}

int32_t SoftbusSession::OpenAuthSessionWithPara(const std::string &deviceId, int32_t actionId, bool isEnable160m)
{
    return DmSoftbusSession::dmSoftbusSession->OpenAuthSessionWithPara(deviceId, actionId, isEnable160m);
}

int32_t SoftbusSession::OpenAuthSession(const std::string &deviceId)
{
    return DmSoftbusSession::dmSoftbusSession->OpenAuthSession(deviceId);
}

} // namespace DistributedHardware
} // namespace OHOS