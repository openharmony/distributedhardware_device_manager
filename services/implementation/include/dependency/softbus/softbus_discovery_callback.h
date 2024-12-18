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

#ifndef OHOS_DM_SOFTBUS_DISCOVERY_CALLBACK_H
#define OHOS_DM_SOFTBUS_DISCOVERY_CALLBACK_H

namespace OHOS {
namespace DistributedHardware {
class ISoftbusDiscoveryCallback {
public:
    virtual void OnDeviceFound(const std::string &pkgName, DmDeviceInfo &info, bool isOnline) = 0;
    virtual void OnDeviceFound(const std::string &pkgName, DmDeviceBasicInfo &info,
        const int32_t range, bool isOnline)
    {
        (void)pkgName;
        (void)info;
        (void)range;
        (void)isOnline;
    };
    virtual void OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId) = 0;
    virtual void OnDiscoveryFailed(const std::string &pkgName, int32_t subscribeId, int32_t failedReason) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_DISCOVERY_CALLBACK_H
