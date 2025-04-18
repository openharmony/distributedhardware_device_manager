/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License") = 0;
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

#ifndef DM_AUTH_MESSAGE_PROCESSOR_MOCK_H
#define DM_AUTH_MESSAGE_PROCESSOR_MOCK_H

#include <gmock/gmock.h>
#include "dm_auth_message_processor.h"

namespace OHOS {
namespace DistributedHardware {

class DmAuthMessageProcessorMock {
public:
    MOCK_METHOD(int32_t, SaveSessionKeyToDP, (int32_t, int32_t &));
    MOCK_METHOD(std::string, CreateMessage, (DmMessageType, std::shared_ptr<DmAuthContext>));
    static inline std::shared_ptr<DmAuthMessageProcessorMock> dmAuthMessageProcessorMock = nullptr;
};

}
}
#endif