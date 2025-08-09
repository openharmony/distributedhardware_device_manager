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

#ifndef OHOS_NDK_DM_CLIENT_H
#define OHOS_NDK_DM_CLIENT_H

#include <mutex>
#include <string>
#include "device_manager_callback.h"

namespace OHOS {
namespace DistributedHardware {
class DmClient {
public:
    static DmClient &GetInstance();
public:
    int32_t Init();
    int32_t UnInit();
    int32_t ReInit();
    int32_t GetLocalDeviceName(std::string &deviceName);
private:
    DmClient() = default;
    ~DmClient() = default;
    DmClient(const DmClient &) = delete;
    DmClient &operator=(const DmClient &) = delete;
    DmClient(DmClient &&) = delete;
    DmClient &operator=(DmClient &&) = delete;
private:
    std::mutex initMtx_;
    std::string pkgName_ = "";
    std::shared_ptr<DmInitCallback> dmInitCallback_;

class InitCallback : public DmInitCallback {
public:
    void OnRemoteDied() override;
};
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_NDK_DM_CLIENT_H
