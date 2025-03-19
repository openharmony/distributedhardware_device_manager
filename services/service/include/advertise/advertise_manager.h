/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ADVERTISE_MANAGER_H
#define OHOS_ADVERTISE_MANAGER_H

#include "dm_timer.h"
#include "softbus_listener.h"

namespace OHOS {
namespace DistributedHardware {
class AdvertiseManager : public std::enable_shared_from_this<AdvertiseManager> {
public:
    AdvertiseManager(std::shared_ptr<SoftbusListener> softbusListener);
    ~AdvertiseManager();

    int32_t StartAdvertising(const std::string &pkgName, const std::map<std::string, std::string> &advertiseParam);
    int32_t StopAdvertising(const std::string &pkgName, int32_t publishId);
    void ClearPublishIdCache(const std::string &pkgName);

private:
    void HandleAutoStopAdvertise(const std::string &timerName, const std::string &pkgName, int32_t publishId);
    void ConfigAdvParam(const std::map<std::string, std::string> &advertiseParam, DmPublishInfo *dmPubInfo,
                        const std::string &pkgName);
    int32_t GenInnerPublishId(const std::string &pkgName, int32_t publishId);
    int32_t GetAndRemoveInnerPublishId(const std::string &pkgName, int32_t publishId);

private:
    std::shared_ptr<DmTimer> timer_;
    std::shared_ptr<SoftbusListener> softbusListener_;

    std::mutex pubMapLock_;
    // Caller publishId to inner publishId. The key is the caller pkgName, the value is
    // a list of externals publishId to inner publishId.
    std::map<std::string, std::map<int32_t, int32_t>> pkgName2PubIdMap_;
    std::set<int32_t> publishIdSet_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_ADVERTISE_MANAGER_H
