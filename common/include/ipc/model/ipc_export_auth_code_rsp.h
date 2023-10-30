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

#ifndef OHOS_DM_IPC_EXPORT_AUTHCODE_RSP_H
#define OHOS_DM_IPC_EXPORT_AUTHCODE_RSP_H

#include "dm_device_info.h"
#include "ipc_rsp.h"

namespace OHOS {
namespace DistributedHardware {
class IpcExportAuthCodeRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcExportAuthCodeRsp);

public:
    /**
     * @tc.name: IpcExportAuthCodeRsp::GetAuthCode
     * @tc.desc: Get Auth Code of the Ipc Export Auth Code Response
     * @tc.type: FUNC
     */
    std::string GetAuthCode() const
    {
        return authCode_;
    }

    /**
     * @tc.name: IpcExportAuthCodeRsp::SetDeviceVec
     * @tc.desc: Set Auth Code of the Ipc Export Auth Code Response
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
#endif // OHOS_DM_IPC_EXPORT_AUTHCODE_RSP_H
