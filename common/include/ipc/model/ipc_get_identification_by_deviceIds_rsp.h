/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_IPC_GET_IDENTIFICATION_BY_DEVICEIDS_RSP_H
#define OHOS_DM_IPC_GET_IDENTIFICATION_BY_DEVICEIDS_RSP_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetIdentificationByDeviceIdsRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetIdentificationByDeviceIdsRsp);
public:
    /**
     * @tc.name: IpcGetIdentificationByDeviceIdsRsp::GetDeviceIdentificationMap
     * @tc.desc: Get DeviceIdentificationMap of the Ipc Get Identification By Devices Response
     * @tc.type: FUNC
     */
    std::map<std::string, std::string> GetDeviceIdentificationMap() const
    {
        return deviceIdentificationMap_;
    }

    /**
     * @tc.name: IpcGetIdentificationByDeviceIdsRsp::SetDeviceIdentificationMap
     * @tc.desc: Set DeviceIdentificationMap of the Ipc Get Identification By Devices Response
     * @tc.type: FUNC
     */
    void SetDeviceIdentificationMap(const std::map<std::string, std::string> &deviceIdentificationMap)
    {
        deviceIdentificationMap_ = deviceIdentificationMap;
    }
private:
    std::map<std::string, std::string> deviceIdentificationMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_GET_IDENTIFICATION_BY_DEVICEIDS_RSP_H