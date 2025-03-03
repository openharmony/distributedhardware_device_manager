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

#ifndef OHOS_DM_IPCSKELETON_MOCK_H
#define OHOS_DM_IPCSKELETON_MOCK_H

#include <gmock/gmock.h>

#include "ipc_skeleton.h"

namespace OHOS {
namespace DistributedHardware {
class DMIPCSkeleton {
public:
    virtual ~DMIPCSkeleton() = default;

    virtual pid_t GetCallingUid() = 0;
    virtual uint32_t GetCallingTokenID() = 0;
public:
    static inline std::shared_ptr<DMIPCSkeleton> dmIpcSkeleton_ = nullptr;
};

class DMIPCSkeletonMock : public DMIPCSkeleton {
public:
    MOCK_METHOD(pid_t, GetCallingUid, ());
    MOCK_METHOD(uint32_t, GetCallingTokenID, ());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPCSKELETON_MOCK_H
