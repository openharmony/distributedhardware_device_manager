/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_IPC_SERVER_LISTENER_MOCK_H
#define OHOS_IPC_SERVER_LISTENER_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "ipc_server_listener.h"

namespace OHOS {
namespace DistributedHardware {
class DmIpcServerListener {
public:
    virtual ~DmIpcServerListener() = default;
public:
    virtual std::vector<ProcessInfo> GetAllProcessInfo() = 0;
    virtual std::set<std::string> GetSystemSA() = 0;
public:
    static inline std::shared_ptr<DmIpcServerListener> dmIpcServerListener = nullptr;
};

class IpcServerListenerMock : public DmIpcServerListener {
public:
    MOCK_METHOD((std::vector<ProcessInfo>), GetAllProcessInfo, ());
    MOCK_METHOD((std::set<std::string>), GetSystemSA, ());
};
}
}
#endif
