/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_CLIENT_STUB_H
#define OHOS_DM_IPC_CLIENT_STUB_H

#include <cstdint>
#include <mutex>

#include "ipc_skeleton.h"
#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class IpcClientStub {
    DM_DECLARE_SINGLE_INSTANCE(IpcClientStub);

public:
    /**
     * @tc.name: IpcClientStub::Init
     * @tc.desc: Ipc client stub initialization
     * @tc.type: FUNC
     */
    int32_t Init();
    SvcIdentity GetSvcIdentity() const
    {
        return clientIdentity_;
    }

private:
    std::mutex lock_;
    bool bInit { false };
    SvcIdentity clientIdentity_;
    IpcObjectStub objectStub_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_CLIENT_STUB_H
