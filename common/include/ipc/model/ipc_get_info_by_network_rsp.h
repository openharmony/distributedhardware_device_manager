/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_IPC_GET_INFO_BY_NETWORK_RSP_H
#define OHOS_DEVICE_MANAGER_IPC_GET_INFO_BY_NETWORK_RSP_H

#include <string>

#include "ipc_rsp.h"

namespace OHOS {
namespace DistributedHardware {
class IpcGetInfoByNetWorkRsp : public IpcRsp {
    DECLARE_IPC_MODEL(IpcGetInfoByNetWorkRsp);

public:
    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::GetUdid
     * @tc.desc: Get Udid for Ipc to get information through NetWorkRsp
     * @tc.type: FUNC
     */
    const std::string GetUdid() const
    {
        return udid_;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::SetUdid
     * @tc.desc: Set Udid for Ipc to get information through NetWorkRsp
     * @tc.type: FUNC
     */
    void SetUdid(const std::string &udid)
    {
        udid_ = udid;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::GetUuid
     * @tc.desc: Get Uuid for Ipc to get information through NetWorkRsp
     * @tc.type: FUNC
     */
    const std::string GetUuid() const
    {
        return uuid_;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::SetExtra
     * @tc.desc: Set Uuid for Ipc to get information through NetWorkRsp
     * @tc.type: FUNC
     */
    void SetUuid(const std::string &uuid)
    {
        uuid_ = uuid;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::GetNetworkType
     * @tc.desc: Get NetworkType for Ipc to get information through NetWorkRsp
     * @tc.type: FUNC
     */
    int32_t GetNetworkType() const
    {
        return networkType_;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::SetNetworkType
     * @tc.desc: Set NetworkType for Ipc to get information through NetWorkRsp
     * @tc.type: FUNC
     */
    void SetNetworkType(const int32_t &networkType)
    {
        networkType_ = networkType;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::GetNetworkType
     * @tc.desc: Get NetworkType for Ipc to get information through NetWorkRsp
     * @tc.type: FUNC
     */
    int32_t GetSecurityLevel() const
    {
        return securityLevel_;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::SetNetworkType
     * @tc.desc: Set NetworkType for Ipc to get information through NetWorkRsp
     * @tc.type: FUNC
     */
    void SetSecurityLevel(const int32_t &securityLevel)
    {
        securityLevel_ = securityLevel;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::GetNetWorkId
     * @tc.desc: Ipc Get Info By NetWork request Get NetWorkId
     * @tc.type: FUNC
     */
    const std::string GetNetWorkId() const
    {
        return netWorkId_;
    }

    /**
     * @tc.name: IpcGetInfoByNetWorkRsp::SetNetworkId
     * @tc.desc: Ipc Get Info By NetWork request Set NetWorkId
     * @tc.type: FUNC
     */
    void SetNetworkId(const std::string &netWorkId)
    {
        netWorkId_ = netWorkId;
    }

private:
    std::string udid_;
    std::string uuid_;
    std::string netWorkId_;
    int32_t networkType_;
    int32_t securityLevel_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IPC_GET_INFO_BY_NETWORK_RSP_H
