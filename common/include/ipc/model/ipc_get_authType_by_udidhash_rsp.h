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

#ifndef OHOS_DEVICE_MANAGER_IPC_GET_AUTHTYPE_BY_UDIDHASH_RSP_H
#define OHOS_DEVICE_MANAGER_IPC_GET_AUTHTYPE_BY_UDIDHASH_RSP_H

#include <string>

#include "ipc_rsp.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetAuthTypeByUdidHashRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetAuthTypeByUdidHashRsp);

public:
    int32_t GetAuthType() const
    {
        return authType_;
    }

    void SetAuthType(const int32_t &authType)
    {
        authType_ = authType;
    }

private:
    int32_t authType_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_GET_AUTHTYPE_BY_UDIDHASH_RSP_H
