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

#ifndef OHOS_DM_IPC_GET_UDIDS_BY_DEVICEIDS_RSP_H
#define OHOS_DM_IPC_GET_UDIDS_BY_DEVICEIDS_RSP_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetUdidsByDeviceIdsRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetUdidsByDeviceIdsRsp);
public:
    /**
     * @tc.name: IpcGetTrustDeviceRsp::GetDeviceIdToUdidMap
     * @tc.desc: Get DeviceIdToUdidMap of the Ipc Get Trust Device Response
     * @tc.type: FUNC
     */
    std::map<std::string, std::string> GetDeviceIdToUdidMap() const
    {
        return deviceIdToUdidMap_;
    }

    /**
     * @tc.name: IpcGetTrustDeviceRsp::SetDeviceIdToUdidMap
     * @tc.desc: Set DeviceIdToUdidMap of the Ipc Get Trust Device Response
     * @tc.type: FUNC
     */
    void SetDeviceIdToUdidMap(const std::map<std::string, std::string> &deviceIdToUdidMap)
    {
        deviceIdToUdidMap_ = deviceIdToUdidMap;
    }
private:
    std::map<std::string, std::string> deviceIdToUdidMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_GET_UDIDS_BY_DEVICEIDS_RSP_H