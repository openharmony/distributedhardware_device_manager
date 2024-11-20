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
#ifndef OHOS_MULTIPLE_USER_CONNECTOR_MOCK_H
#define OHOS_MULTIPLE_USER_CONNECTOR_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {
class DmMultipleUserConnector {
public:
    virtual ~DmMultipleUserConnector() = default;
public:
    virtual int32_t GetCurrentAccountUserID(void) = 0;
public:
    static inline std::shared_ptr<DmMultipleUserConnector> dmMultipleUserConnector = nullptr;
};

class MultipleUserConnectorMock : public DmMultipleUserConnector {
public:
    MOCK_METHOD(int32_t, GetCurrentAccountUserID, ());
};
}
}
#endif
