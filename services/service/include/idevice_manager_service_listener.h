/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
    virtual void OnDeviceStateChange(const std::string &pkgName, const DmDeviceState &state,
        const DmDeviceInfo &info) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDeviceFound
     * @tc.desc: Device Found of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDeviceFound(const std::string &pkgName, uint16_t subscribeId, const DmDeviceInfo &info) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDeviceFound
     * @tc.desc: Device Found of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDeviceFound(const std::string &pkgName, uint16_t subscribeId, DmDeviceBasicInfo &info) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDiscoveryFailed
     * @tc.desc: Discovery Failed of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDiscoveryFailed(const std::string &pkgName, uint16_t subscribeId, int32_t failedReason) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDiscoverySuccess
     * @tc.desc: Discovery Success of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnPublishResult
     * @tc.desc: OnPublish Result of the Dm Publish Manager
     * @tc.type: FUNC
     */
    virtual void OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnAuthResult
     * @tc.desc: Auth Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnAuthResult(const std::string &pkgName, const std::string &deviceId, const std::string &token,
                              int32_t status, int32_t reason) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnUiCall
     * @tc.desc: Fa Call of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnUiCall(std::string &pkgName, std::string &paramJson) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnCredentialResult
     * @tc.desc: Credential Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnCredentialResult(const std::string &pkgName, int32_t action, const std::string &resultInfo) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnBindResult
     * @tc.desc: Bind target Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnBindResult(const std::string &pkgName, const PeerTargetId &targetId, int32_t result,
        int32_t status, std::string content) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnUnbindResult
     * @tc.desc: Unbind target Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnUnbindResult(const std::string &pkgName, const PeerTargetId &targetId, int32_t result,
        std::string content) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnPinHolderCreate
     * @tc.desc: Unbind target Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnPinHolderCreate(const std::string &pkgName, const std::string &deviceId, DmPinType pinType,
        const std::string &payload) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnPinHolderDestroy
     * @tc.desc: Unbind target Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnPinHolderDestroy(const std::string &pkgName, DmPinType pinType, const std::string &payload) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnCreateResult
     * @tc.desc: Create Pin Holder Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnCreateResult(const std::string &pkgName, int32_t result) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDestroyResult
     * @tc.desc: Destroy Pin Holder Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDestroyResult(const std::string &pkgName, int32_t result) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnPinHolderEvent
     * @tc.desc: Pin Holder Event of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnPinHolderEvent(const std::string &pkgName, DmPinHolderEvent event, int32_t result,
        const std::string &content) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_LISTENER_H
