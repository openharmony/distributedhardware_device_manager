/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_CLIENT_FAIL_TO_SUPPORT_H
#define OHOS_DM_CLIENT_FAIL_TO_SUPPORT_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
class DmClientFailToSupport {
public:
    static DmClientFailToSupport &GetInstance();
public:
    int32_t Init();
    int32_t UnInit();
    int32_t ReInit();
    int32_t GetLocalDeviceName(std::string &deviceName);
private:
    DmClientFailToSupport() = default;
    ~DmClientFailToSupport() = default;
    DmClientFailToSupport(const DmClientFailToSupport &) = delete;
    DmClientFailToSupport &operator=(const DmClientFailToSupport &) = delete;
    DmClientFailToSupport(DmClientFailToSupport &&) = delete;
    DmClientFailToSupport &operator=(DmClientFailToSupport &&) = delete;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_CLIENT_FAIL_TO_SUPPORT_H
