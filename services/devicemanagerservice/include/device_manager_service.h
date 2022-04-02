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
#include <vector>

#include "dm_ability_manager.h"
#include "dm_auth_manager.h"
#include "dm_device_info.h"
#include "dm_device_info_manager.h"
#include "dm_device_state_manager.h"
#include "dm_discovery_manager.h"
#include "single_instance.h"
#include "softbus_connector.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerService {
DECLARE_SINGLE_INSTANCE_BASE(DeviceManagerService);
public:
    int32_t Init();
    ~DeviceManagerService();

    /**
     * @tc.name: DeviceManagerService::GetTrustedDeviceList
     * @tc.desc: Get Trusted DeviceList of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                 std::vector<DmDeviceInfo> &deviceList);

    /**
     * @tc.name: DeviceManagerService::GetLocalDeviceInfo
     * @tc.desc: Get Local DeviceInfo of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t GetLocalDeviceInfo(DmDeviceInfo &info);

    /**
     * @tc.name: DeviceManagerService::GetUdidByNetworkId
     * @tc.desc: Get Udid By NetworkId of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &udid);

    /**
     * @tc.name: DeviceManagerService::GetUuidByNetworkId
     * @tc.desc: Get Uuid By NetworkId of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId, std::string &uuid);

    /**
     * @tc.name: DeviceManagerService::StartDeviceDiscovery
     * @tc.desc: Start Device Discovery of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                 const std::string &extra);

    /**
     * @tc.name: DeviceManagerService::StopDeviceDiscovery
     * @tc.desc: Stop Device Discovery of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId);

    /**
     * @tc.name: DeviceManagerService::AuthenticateDevice
     * @tc.desc: Authenticate Device of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t AuthenticateDevice(const std::string &pkgName, int32_t authType, const std::string &deviceId,
                               const std::string &extra);

    /**
     * @tc.name: DeviceManagerService::UnAuthenticateDevice
     * @tc.desc: UnAuthenticate Device of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId);

    /**
     * @tc.name: DeviceManagerService::VerifyAuthentication
     * @tc.desc: Verify Authentication of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t VerifyAuthentication(const std::string &authParam);

    /**
     * @tc.name: DeviceManagerService::GetFaParam
     * @tc.desc: Get FaParam of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t GetFaParam(std::string &pkgName, DmAuthParam &authParam);

    /**
     * @tc.name: DeviceManagerService::SetUserOperation
     * @tc.desc: Se tUser Operation of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t SetUserOperation(std::string &pkgName, int32_t action);

    /**
     * @tc.name: DeviceManagerService::RegisterDevStateCallback
     * @tc.desc: Register DevState Callback of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t RegisterDevStateCallback(const std::string &pkgName, const std::string &extra);

    /**
     * @tc.name: DeviceManagerService::UnRegisterDevStateCallback
     * @tc.desc: UnRegister DevState Callback of the Device Manager Service
     * @tc.type: FUNC
     */
    int32_t UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra);

    /**
     * @tc.name: DeviceManagerService::IsServiceInitialized
     * @tc.desc: IsServiceInitialized of the Device Manager Service
     * @tc.type: FUNC
     */
    bool IsServiceInitialized();

private:
    DeviceManagerService() = default;
    bool intFlag_ = false;
    std::shared_ptr<DmAuthManager> authMgr_;
    std::shared_ptr<DmDeviceInfoManager> deviceInfoMgr_;
    std::shared_ptr<DmDeviceStateManager> deviceStateMgr_;
    std::shared_ptr<DmDiscoveryManager> discoveryMgr_;
    std::shared_ptr<SoftbusConnector> softbusConnector_;
    std::shared_ptr<DeviceManagerServiceListener> listener_;
    std::shared_ptr<DmAbilityManager> abilityMgr_;
    std::shared_ptr<HiChainConnector> hiChainConnector_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_H
