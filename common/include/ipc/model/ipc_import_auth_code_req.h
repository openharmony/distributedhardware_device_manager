/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_IPC_IMPORT_AUTH_CODE_REQ_H
#define OHOS_DEVICE_MANAGER_IPC_IMPORT_AUTH_CODE_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcImportAuthCodeReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcImportAuthCodeReq);

public:
    /**
     * @tc.name: IpcImportAuthCodeReq::GetAuthCode
     * @tc.desc: Ipc Import Auth Code request Get Auth Code
     * @tc.type: FUNC
     */
    const std::string GetAuthCode() const
    {
        return authCode_;
    }

    /**
     * @tc.name: IpcImportAuthCodeReq::SetAuthCode
     * @tc.desc: Ipc Import Auth Code request Set Auth Code
     * @tc.type: FUNC
     */
    void SetAuthCode(const std::string &authCode)
    {
        authCode_ = authCode;
    }

private:
    std::string authCode_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_IMPORT_AUTH_CODE_REQ_H
