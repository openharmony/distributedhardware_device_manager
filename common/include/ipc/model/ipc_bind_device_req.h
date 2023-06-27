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

#ifndef OHOS_DM_IPC_BIND_DEVICE_REQ_H
#define OHOS_DM_IPC_BIND_DEVICE_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcBindDeviceReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcBindDeviceReq);

public:
    /**
     * @tc.name: IpcBindDeviceReq::GetDeviceInfo
     * @tc.desc: Ipc Bind Device Req Get DeviceInfo
     * @tc.type: FUNC
     */
    const std::string &GetDeviceId() const
    {
        return deviceId_;
    }

    /**
     * @tc.name: IpcBindDeviceReq::SetDeviceInfo
     * @tc.desc: Ipc Bind Device Req Set DeviceId
     * @tc.type: FUNC
     */
    void SetDeviceId(const std::string &deviceId)
    {
        deviceId_ = deviceId;
    }

    /**
     * @tc.name: IpcBindDeviceReq::GetAuthType
     * @tc.desc: Ipc Bind Device Req Get AuthType
     * @tc.type: FUNC
     */
    int32_t GetBindType()
    {
        return bindType_;
    }

    /**
     * @tc.name: IpcBindDeviceReq::SetAuthType
     * @tc.desc: Ipc Bind Device Req Set AuthType
     * @tc.type: FUNC
     */
    void SetBindType(int32_t bindType)
    {
        bindType_ = bindType;
    }

    /**
     * @tc.name: IpcBindDeviceReq::GetBindParam
     * @tc.desc: Ipc Bind Device Req Get bindParam
     * @tc.type: FUNC
     */
    const std::string &GetBindParam() const
    {
        return bindParam_;
    }

    /**
     * @tc.name: IpcBindDeviceReq::SetExtra
     * @tc.desc: Ipc Bind Device Req Set bindParam
     * @tc.type: FUNC
     */
    void SetBindParam(const std::string &bindParam)
    {
        bindParam_ = bindParam;
    }

private:
    std::string deviceId_;
    int32_t bindType_;
    std::string bindParam_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_BIND_DEVICE_REQ_H
