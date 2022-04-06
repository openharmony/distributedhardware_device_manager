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

#ifndef OHOS_DM_IPC_AUTHENTICATE_DEVICE_REQ_H
#define OHOS_DM_IPC_AUTHENTICATE_DEVICE_REQ_H

#include "dm_device_info.h"
#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcAuthenticateDeviceReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcAuthenticateDeviceReq);

public:
    /**
     * @tc.name: IpcAuthenticateDeviceReq::GetDeviceInfo
     * @tc.desc: Ipc Authenticate Device Req Get DeviceInfo
     * @tc.type: FUNC
     */
    const DmDeviceInfo &GetDeviceInfo() const
    {
        return deviceInfo_;
    }

    /**
     * @tc.name: IpcAuthenticateDeviceReq::SetDeviceInfo
     * @tc.desc: Ipc Authenticate Device Req Set DeviceInfo
     * @tc.type: FUNC
     */
    void SetDeviceInfo(const DmDeviceInfo &deviceInfo)
    {
        deviceInfo_ = deviceInfo;
    }

    /**
     * @tc.name: IpcAuthenticateDeviceReq::GetAuthType
     * @tc.desc: Ipc Authenticate Device Req Get AuthType
     * @tc.type: FUNC
     */
    int32_t GetAuthType()
    {
        return authType_;
    }

    /**
     * @tc.name: IpcAuthenticateDeviceReq::SetAuthType
     * @tc.desc: Ipc Authenticate Device Req Set AuthType
     * @tc.type: FUNC
     */
    void SetAuthType(int32_t authType)
    {
        authType_ = authType;
    }

    /**
     * @tc.name: IpcAuthenticateDeviceReq::GetExtra
     * @tc.desc: Ipc Authenticate Device Req Get Extra
     * @tc.type: FUNC
     */
    const std::string &GetExtra() const
    {
        return extra_;
    }

    /**
     * @tc.name: IpcAuthenticateDeviceReq::SetExtra
     * @tc.desc: Ipc Authenticate Device Req Set Extra
     * @tc.type: FUNC
     */
    void SetExtra(const std::string &extra)
    {
        extra_ = extra;
    }

private:
    DmDeviceInfo deviceInfo_;
    int32_t authType_;
    std::string extra_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_AUTHENTICATE_DEVICE_REQ_H
