/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_IPC_GET_DEVICE_SCREEN_STATUS_RSP_H
#define OHOS_DEVICE_MANAGER_IPC_GET_DEVICE_SCREEN_STATUS_RSP_H

#include "ipc_rsp.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetDeviceScreenStatusRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetDeviceScreenStatusRsp);

public:
    /**
     * @tc.name: IpcGetDeviceScreenStatusRsp::GetScreenStatus
     * @tc.desc: Get ScreenStatus for Ipc to get information through ScreenStatusRsp
     * @tc.type: FUNC
     */
    int32_t GetScreenStatus() const
    {
        return screenStatus_;
    }

    /**
     * @tc.name: IpcGetDeviceScreenStatusRsp::SetScreenStatus
     * @tc.desc: Set ScreenStatus for Ipc to get information through ScreenStatusRsp
     * @tc.type: FUNC
     */
    void SetScreenStatus(const int32_t &screenStatus)
    {
        screenStatus_ = screenStatus;
    }

private:
    int32_t screenStatus_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_GET_DEVICE_SCREEN_STATUS_RSP_H