/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_DEVICE_MANAGER_SERVICE_MOCK_H
#define OHOS_DEVICE_MANAGER_SERVICE_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "device_manager_service.h"

namespace OHOS {
namespace DistributedHardware {
class DmDeviceManagerService {
public:
    virtual ~DmDeviceManagerService() = default;
public:
    virtual bool IsDMServiceImplReady() = 0;
    virtual int32_t GetTrustedDeviceList(const std::string &pkgName, std::vector<DmDeviceInfo> &deviceList) = 0;
    virtual bool IsDMServiceAdapterSoLoaded() = 0;
    virtual bool IsDMServiceAdapterResidentLoad() = 0;
    virtual int32_t OpenAuthSessionWithPara(int64_t serviceId) = 0;
    virtual int32_t GenerateServiceId(int64_t &serviceId) = 0;
    virtual int32_t CheckServiceHasRegistered(const ServiceRegInfo &serviceRegInfo, int64_t tokenId,
        int32_t &regServiceId) = 0;
    virtual int32_t GenerateRegServiceId(int32_t &regServiceId) = 0;
    virtual bool IsImportAuthInfoValid(const DmAuthInfo &dmAuthInfo) = 0;
    virtual bool IsExportAuthInfoValid(const DmAuthInfo &dmAuthInfo) = 0;
public:
    static inline std::shared_ptr<DmDeviceManagerService> dmDeviceManagerService = nullptr;
};

class DeviceManagerServiceMock : public DmDeviceManagerService {
public:
    MOCK_METHOD(bool, IsDMServiceImplReady, ());
    MOCK_METHOD(int32_t, GetTrustedDeviceList, (const std::string &, std::vector<DmDeviceInfo> &));
    MOCK_METHOD(bool, IsDMServiceAdapterSoLoaded, ());
    MOCK_METHOD(bool, IsDMServiceAdapterResidentLoad, ());
    MOCK_METHOD(int32_t, OpenAuthSessionWithPara, (int64_t));
    MOCK_METHOD(int32_t, GenerateServiceId, (int64_t &));
    MOCK_METHOD(int32_t, CheckServiceHasRegistered, (const ServiceRegInfo &, int64_t, int32_t &));
    MOCK_METHOD(int32_t, GenerateRegServiceId, (int32_t &));
    MOCK_METHOD(bool, IsImportAuthInfoValid, (const DmAuthInfo &));
    MOCK_METHOD(bool, IsExportAuthInfoValid, (const DmAuthInfo &));
};
}
}
#endif
