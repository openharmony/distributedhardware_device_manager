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

#ifndef OHOS_DM_IPC_SERVER_LISTENER_MGR_H
#define OHOS_DM_IPC_SERVER_LISTENER_MGR_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>

#include "ipc_skeleton.h"
#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
typedef struct CommonSvcId {
    uint32_t handle;
    uint32_t token;
    uint32_t cookie;
    uint32_t cbId;
} CommonSvcId;

class IpcServerListenermgr {
    DM_DECLARE_SINGLE_INSTANCE(IpcServerListenermgr);

public:
    /**
     * @tc.name: IpcServerListener::RegisterListener
     * @tc.desc: Register Listener of the Ipc Server Listener
     * @tc.type: FUNC
     */
    int32_t RegisterListener(std::string &pkgName, const CommonSvcId *svcId);

    /**
     * @tc.name: IpcServerListener::GetListenerByPkgName
     * @tc.desc: Get Listener By package Name of the Ipc Server Listener
     * @tc.type: FUNC
     */
    int32_t GetListenerByPkgName(std::string &pkgName, CommonSvcId *svcId);

    /**
     * @tc.name: IpcServerListener::UnregisterListener
     * @tc.desc: Unregister Listener of the Ipc Server Listener
     * @tc.type: FUNC
     */
    int32_t UnregisterListener(std::string &pkgName);

    /**
     * @tc.name: IpcServerListener::GetAllListeners
     * @tc.desc: Get All Listeners of the Ipc Server Listener
     * @tc.type: FUNC
     */
    const std::map<std::string, CommonSvcId> &GetAllListeners();

private:
    std::map<std::string, CommonSvcId> dmListenerMap_;
    std::mutex lock_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_SERVER_LISTENER_MGR_H
