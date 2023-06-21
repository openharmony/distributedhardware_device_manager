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

#ifndef OHOS_DM_IPC_GET_LOCAL_DEVICE_TYPE_RSP_H
#define OHOS_DM_IPC_GET_LOCAL_DEVICE_TYPE_RSP_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetLocalDeviceTypeRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetLocalDeviceTypeRsp);

public:
    /**
     * @tc.name: IpcGetLocalDeviceInfoRsp::GetLocalDeviceType
     * @tc.desc: Ipc get local device information reply Get Local Device Type
     * @tc.type: FUNC
     */
    const int32_t &GetLocalDeviceType() const
    {
        return deviceType_;
    }

    /**
     * @tc.name: IpcGetLocalDeviceInfoRsp::SetLocalDeviceType
     * @tc.desc: Ipc get local device information reply Set Local Device Type
     * @tc.type: FUNC
     */
    void SetLocalDeviceType(const int32_t &deviceType)
    {
        deviceType_ = deviceType;
    }

private:
    int32_t deviceType_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_GET_LOCAL_DEVICE_TYPE_RSP_H
