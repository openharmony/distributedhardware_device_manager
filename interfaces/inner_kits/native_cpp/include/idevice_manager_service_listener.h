/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <set>

#include "dm_device_info.h"
#include "dm_device_profile_info.h"

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
    virtual void OnDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
        const DmDeviceInfo &info) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDeviceFound
     * @tc.desc: Device Found of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDeviceFound(const ProcessInfo &processInfo, uint16_t subscribeId, const DmDeviceInfo &info) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDiscoveryFailed
     * @tc.desc: Discovery Failed of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDiscoveryFailed(const ProcessInfo &processInfo, uint16_t subscribeId, int32_t failedReason) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDiscoverySuccess
     * @tc.desc: Discovery Success of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDiscoverySuccess(const ProcessInfo &processInfo, int32_t subscribeId) = 0;

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
    virtual void OnAuthResult(const ProcessInfo &processInfo, const std::string &deviceId, const std::string &token,
                              int32_t status, int32_t reason) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnUiCall
     * @tc.desc: Fa Call of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnUiCall(const ProcessInfo &processInfo, std::string &paramJson) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnCredentialResult
     * @tc.desc: Credential Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnCredentialResult(const ProcessInfo &processInfo, int32_t action, const std::string &resultInfo) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnBindResult
     * @tc.desc: Bind target Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnBindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId, int32_t result,
        int32_t status, std::string content) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnUnbindResult
     * @tc.desc: Unbind target Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnUnbindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId, int32_t result,
        std::string content) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnPinHolderCreate
     * @tc.desc: Unbind target Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnPinHolderCreate(const ProcessInfo &processInfo, const std::string &deviceId, DmPinType pinType,
        const std::string &payload) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnPinHolderDestroy
     * @tc.desc: Unbind target Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnPinHolderDestroy(const ProcessInfo &processInfo, DmPinType pinType, const std::string &payload) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnCreateResult
     * @tc.desc: Create Pin Holder Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnCreateResult(const ProcessInfo &processInfo, int32_t result) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDestroyResult
     * @tc.desc: Destroy Pin Holder Result of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDestroyResult(const ProcessInfo &processInfo, int32_t result) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnPinHolderEvent
     * @tc.desc: Pin Holder Event of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnPinHolderEvent(const ProcessInfo &processInfo, DmPinHolderEvent event, int32_t result,
        const std::string &content) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::SetNotOfflinePkgname
     * @tc.desc: Set the pkgname that not offline
     * @tc.type: FUNC
     */
    virtual void OnDeviceTrustChange(const std::string &udid, const std::string &uuid, DmAuthForm authForm) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnDeviceScreenStateChange
     * @tc.desc: Device Screen State Change of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnDeviceScreenStateChange(const ProcessInfo &processInfo, DmDeviceInfo &devInfo) = 0;

    /**
     * @tc.name: IDeviceManagerServiceListener::OnCredentialAuthStatus
     * @tc.desc: Candidate Restrict Status Change of the DeviceManager Service Listener
     * @tc.type: FUNC
     */
    virtual void OnCredentialAuthStatus(const ProcessInfo &processInfo, const std::string &deviceList,
                                        uint16_t deviceTypeId, int32_t errcode) = 0;
    virtual void OnAppUnintall(const std::string &pkgName) = 0;
    virtual void OnSinkBindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId, int32_t result,
        int32_t status, std::string content) = 0;
    virtual void OnProcessRemove(const ProcessInfo &processInfo) = 0;
    virtual void OnDevStateCallbackAdd(const ProcessInfo &processInfo,
        const std::vector<DmDeviceInfo> &deviceList) = 0;
    virtual void OnGetDeviceProfileInfoListResult(const ProcessInfo &processInfo,
        const std::vector<DmDeviceProfileInfo> &deviceProfileInfos, int32_t code) = 0;
    virtual void OnGetDeviceIconInfoResult(const ProcessInfo &processInfo,
        const DmDeviceIconInfo &dmDeviceIconInfo, int32_t code) = 0;
    virtual void OnSetLocalDeviceNameResult(const ProcessInfo &processInfo,
        const std::string &deviceName, int32_t code) = 0;
    virtual void OnSetRemoteDeviceNameResult(const ProcessInfo &processInfo, const std::string &deviceId,
        const std::string &deviceName, int32_t code) = 0;
    virtual void SetExistPkgName(const std::set<std::string> &pkgNameSet) = 0;
    virtual void OnServiceFound(const ProcessInfo &processInfo, int32_t discServiceId,
        const DiscoveryServiceInfo &discServiceInfo) = 0;
    virtual void OnServiceDiscoveryResult(const ProcessInfo &processInfo, int32_t discServiceId, int32_t reason) = 0;

    virtual std::string GetLocalDisplayDeviceName() = 0;
    virtual int32_t OpenAuthSessionWithPara(const std::string &deviceId, int32_t actionId, bool isEnable160m) = 0;
    virtual int32_t OpenAuthSessionWithPara(int64_t serviceId) = 0;
    virtual void OnDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
        const DmDeviceInfo &info, const std::vector<int64_t> &serviceIds) = 0;
    virtual void OnServicePublishResult(const ProcessInfo &processInfo, int64_t serviceId, int32_t publishResult) = 0;
    virtual void OnDevDbReadyCallbackAdd(const ProcessInfo &processInfo,
        const std::vector<DmDeviceInfo> &deviceList) = 0;
    virtual void OnLeaveLNNResult(const std::string &pkgName, const std::string &networkId,
        int32_t retCode) {}
    virtual void OnAuthCodeInvalid(const std::string &pkgName) {}
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_LISTENER_H
