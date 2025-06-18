/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_SOFTBUS_SESSION_MOCK_H
#define OHOS_SOFTBUS_SESSION_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "softbus_session.h"

namespace OHOS {
namespace DistributedHardware {
class DmSoftbusSession {
public:
    virtual ~DmSoftbusSession() = default;
public:
    virtual int32_t GetPeerDeviceId(int32_t sessionId, std::string &peerDevId) = 0;
    virtual int32_t SendData(int32_t sessionId, std::string &message) = 0;
    virtual int32_t OpenAuthSessionWithPara(const std::string &deviceId, int32_t actionId, bool isEnable160m) = 0;
    virtual int32_t OpenAuthSession(const std::string &deviceId) = 0;

public:
    static inline std::shared_ptr<DmSoftbusSession> dmSoftbusSession = nullptr;
};

class SoftbusSessionMock : public DmSoftbusSession {
public:
    MOCK_METHOD(int32_t, GetPeerDeviceId, (int32_t, std::string &));
    MOCK_METHOD(int32_t, SendData, (int32_t, std::string &));
    MOCK_METHOD(int32_t, OpenAuthSessionWithPara, (const std::string &, int32_t, bool));
    MOCK_METHOD(int32_t, OpenAuthSession, (const std::string &));
};
}
}
#endif
