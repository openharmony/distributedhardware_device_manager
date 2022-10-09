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

#ifndef OHOS_I_DM_SERVICE_LISTENER_H
#define OHOS_I_DM_SERVICE_LISTENER_H

#include <map>
#include <string>

#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
class IDeviceManagerServiceListener {
public:
    virtual ~IDeviceManagerServiceListener() {}

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDeviceStateChange
     * @tc.desc: Device State Change of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDeviceStateChange(const std::string &pkgName, const DmDeviceState &state, const DmDeviceInfo &info);

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDeviceFound
     * @tc.desc: Device Found of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDeviceFound(const std::string &pkgName, uint16_t subscribeId, const DmDeviceInfo &info);

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDiscoveryFailed
     * @tc.desc: Discovery Failed of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDiscoveryFailed(const std::string &pkgName, uint16_t subscribeId, int32_t failedReason);

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDiscoverySuccess
     * @tc.desc: Discovery Success of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId);

    /**
     * @tc.name: IDeviceManagerServiceListener::OnPublishResult
     * @tc.desc: OnPublish Result of the Dm Publish Manager
     * @tc.type: FUNC
     */
    virtual void OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult);

    /**
     * @tc.name: IDeviceManagerServiceListener::OnAuthResult
     * @tc.desc: Auth Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnAuthResult(const std::string &pkgName, const std::string &deviceId, const std::string &token,
                              int32_t status, int32_t reason);

    /**
     * @tc.name: IDeviceManagerServiceListener::OnVerifyAuthResult
     * @tc.desc: Verify Auth Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnVerifyAuthResult(const std::string &pkgName, const std::string &deviceId, int32_t resultCode,
                            const std::string &flag);

    /**
     * @tc.name: IDeviceManagerServiceListener::OnUiCall
     * @tc.desc: Fa Call of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnUiCall(std::string &pkgName, std::string &paramJson);

    /**
     * @tc.name: IDeviceManagerServiceListener::OnCredentialResult
     * @tc.desc: Credential Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnCredentialResult(const std::string &pkgName, int32_t action, const std::string &resultInfo);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_LISTENER_H
