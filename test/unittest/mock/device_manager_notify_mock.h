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
#ifndef OHOS_DEVICE_MANAGER_NOTIFY_MOCK_H
#define OHOS_DEVICE_MANAGER_NOTIFY_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "device_manager_notify.h"

namespace OHOS {
namespace DistributedHardware {
class DmDeviceManagerNotify {
public:
    virtual ~DmDeviceManagerNotify() = default;
public:
    virtual int32_t RegisterGetDeviceIconInfoCallback(const std::string &pkgName, const std::string &uk,
        std::shared_ptr<GetDeviceIconInfoCallback> callback) = 0;
    virtual int32_t RegisterGetDeviceProfileInfoListCallback(const std::string &pkgName,
        std::shared_ptr<GetDeviceProfileInfoListCallback> callback) = 0;
    virtual int32_t RegisterServiceStateCallback(const std::string &pkgName, int64_t serviceId,
        std::shared_ptr<ServiceInfoStateCallback> callback) = 0;
    virtual int32_t UnRegisterServiceStateCallback(const std::string &pkgName, int64_t serviceId) = 0;
    virtual void RegisterServicePublishCallback(const std::string &pkgName, int64_t serviceId,
        std::shared_ptr<ServicePublishCallback> callback) = 0;
    virtual void UnRegisterServicePublishCallback(const std::string &pkgName, int64_t serviceId) = 0;
    virtual void RegisterServiceDiscoveryCallback(const std::string &pkgName, const std::string &serviceType,
        std::shared_ptr<ServiceDiscoveryCallback> callback) = 0;
    virtual void UnRegisterServiceDiscoveryCallback(const std::string &pkgName, const std::string &serviceType) = 0;
    virtual void RegisterSyncServiceInfoCallback(const std::string &pkgName, int32_t localUserId,
        const std::string &networkId, std::shared_ptr<SyncServiceInfoCallback> callback, int64_t serviceId = 0) = 0;
    virtual void UnRegisterSyncServiceInfoCallback(const std::string &pkgName, int32_t localUserId,
        const std::string &networkId, int64_t serviceId = 0) = 0;
public:
    static inline std::shared_ptr<DmDeviceManagerNotify> dmDeviceManagerNotify = nullptr;
};

class DeviceManagerNotifyMock : public DmDeviceManagerNotify {
public:
    MOCK_METHOD(int32_t, RegisterGetDeviceIconInfoCallback, (const std::string &, const std::string &,
        std::shared_ptr<GetDeviceIconInfoCallback>));
    MOCK_METHOD(int32_t, RegisterGetDeviceProfileInfoListCallback, (const std::string &,
        std::shared_ptr<GetDeviceProfileInfoListCallback>));
    MOCK_METHOD(int32_t, RegisterServiceStateCallback, (const std::string &, int64_t,
        std::shared_ptr<ServiceInfoStateCallback>));
    MOCK_METHOD(int32_t, UnRegisterServiceStateCallback, (const std::string &, int64_t));
    MOCK_METHOD(void, RegisterServicePublishCallback, (const std::string &, int64_t,
        std::shared_ptr<ServicePublishCallback>));
    MOCK_METHOD(void, UnRegisterServicePublishCallback, (const std::string &, int64_t));
    MOCK_METHOD(void, RegisterServiceDiscoveryCallback, (const std::string &, const std::string &,
        std::shared_ptr<ServiceDiscoveryCallback>));
    MOCK_METHOD(void, UnRegisterServiceDiscoveryCallback, (const std::string &, const std::string &));
    MOCK_METHOD(void, RegisterSyncServiceInfoCallback, (const std::string &, int32_t,
        const std::string &, std::shared_ptr<SyncServiceInfoCallback>, int64_t));
    MOCK_METHOD(void, UnRegisterSyncServiceInfoCallback, (const std::string &, int32_t,
        const std::string &, int64_t));
};
}
}
#endif
