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

#ifndef OHOS_DM_SERVICE_H
#define OHOS_DM_SERVICE_H

#include <string>
#include <memory>
#if defined(__LITEOS_M__)
#include "dm_mutex.h"
#else
#include <mutex>
#endif

#include "device_manager_service_listener.h"
#include "idevice_manager_service_impl.h"
#include "single_instance.h"
#include "softbus_listener.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerService {
DECLARE_SINGLE_INSTANCE_BASE(DeviceManagerService);
public:
    DeviceManagerService() {}

    ~DeviceManagerService();

    int32_t Init();

    int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                 std::vector<DmDeviceInfo> &deviceList);

    int32_t GetLocalDeviceInfo(DmDeviceInfo &info);

    int32_t GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &udid);

    int32_t GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &uuid);

    int32_t StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                 const std::string &extra);

    int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId);

    int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
                               const std::string &extra);

    int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId);

    int32_t VerifyAuthentication(const std::string &authParam);

    int32_t GetFaParam(std::string &pkgName, DmAuthParam &authParam);

    int32_t SetUserOperation(std::string &pkgName, int32_t action);

    int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra);

    int32_t UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra);

    void HandleDeviceOnline(const DmDeviceInfo &info);

    void HandleDeviceOffline(const DmDeviceInfo &info);

    int OnSessionOpened(int sessionId, int result);

    void OnSessionClosed(int sessionId);

    void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    bool IsDMServiceImplReady();

private:
    bool isImplsoLoaded_ = false;
    std::shared_ptr<SoftbusListener> softbusListener_;
    std::shared_ptr<DeviceManagerServiceListener> listener_;
    std::shared_ptr<IDeviceManagerServiceImpl> dmServiceImpl_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_H