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
 
#ifndef OHOS_DM_IPC_SYNC_SERVICE_INFO_RESULT_REQ_H
#define OHOS_DM_IPC_SYNC_SERVICE_INFO_RESULT_REQ_H
 
#include "ipc_req.h"
#include "dm_device_info.h"
 
namespace OHOS {
namespace DistributedHardware {
class IpcSyncServiceInfoResultReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcSyncServiceInfoResultReq);
public:
    /**
     * @tc.name: IpcSyncServiceInfoResultReq::SetServiceSyncInfo
     * @tc.desc: Ipc Set ServiceSyncInfo Request Set ServiceSyncInfo
     * @tc.type: FUNC
     */
    void SetServiceSyncInfo(const ServiceSyncInfo &serviceSyncInfo)
    {
        serviceSyncInfo_ = serviceSyncInfo;
    }
 
    /**
     * @tc.name: IpcSyncServiceInfoResultReq::SetServiceSyncInfo
     * @tc.desc: Ipc Get ServiceSyncInfo Request Get ServiceSyncInfo
     * @tc.type: FUNC
     */
    const ServiceSyncInfo &GetServiceSyncInfo() const
    {
        return serviceSyncInfo_;
    }
 
    /**
     * @tc.name: IpcSyncServiceInfoResultReq::SetResult
     * @tc.desc: Ipc Set Result Request Set Result
     * @tc.type: FUNC
     */
    void SetResult(int32_t result)
    {
        result_ = result;
    }
 
    /**
     * @tc.name: IpcSyncServiceInfoResultReq::GetResult
     * @tc.desc: Ipc Get Result Request Get Result
     * @tc.type: FUNC
     */
    int32_t &GetResult()
    {
        return result_;
    }
 
    /**
     * @tc.name: IpcSyncServiceInfoResultReq::SetContent
     * @tc.desc: Ipc Set Content Request Set Content
     * @tc.type: FUNC
     */
    void SetContent(const std::string &content)
    {
        content_ = content;
    }
 
    /**
     * @tc.name: IpcSyncServiceInfoResultReq::GetContent
     * @tc.desc: Ipc Get Content Request Get Content
     * @tc.type: FUNC
     */
    const std::string &GetContent() const
    {
        return content_;
    }
 
    /**
     * @tc.name: IpcSyncServiceInfoResultReq::SetServiceId
     * @tc.desc: Ipc Set ServiceId Request Set ServiceId
     * @tc.type: FUNC
     */
    void SetServiceId(int64_t serviceId)
    {
        serviceId_ = serviceId;
    }
 
    /**
     * @tc.name: IpcSyncServiceInfoResultReq::GetServiceId
     * @tc.desc: Ipc Get ServiceId Request Get ServiceId
     * @tc.type: FUNC
     */
    int32_t GetServiceId()
    {
        return serviceId_;
    }
 
private:
    ServiceSyncInfo serviceSyncInfo_;
    int32_t result_ = 0;
    std::string content_;
    int64_t serviceId_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_SYNC_SERVICE_INFO_REQ_H