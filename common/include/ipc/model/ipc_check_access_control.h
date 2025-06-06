/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_CHECK_ACCESS_CONTROL_H
#define OHOS_DM_IPC_CHECK_ACCESS_CONTROL_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcCheckAcl : public IpcReq {
    DECLARE_IPC_MODEL(IpcCheckAcl);

public:
    const DmAccessCaller &GetAccessCaller() const
    {
        return caller_;
    }

    void SetAccessCaller(const DmAccessCaller &caller)
    {
        caller_ = caller;
    }

    const DmAccessCallee &GetAccessCallee() const
    {
        return callee_;
    }

    void SetAccessCallee(const DmAccessCallee &callee)
    {
        callee_ = callee;
    }
private:
    DmAccessCaller caller_;
    DmAccessCallee callee_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_CHECK_ACCESS_CONTROL_H
