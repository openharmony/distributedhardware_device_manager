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

#ifndef OHOS_DM_SERVICE_LISTENER_H
#define OHOS_DM_SERVICE_LISTENER_H

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "dm_device_info.h"
#include "dm_device_profile_info.h"
#include "idevice_manager_service_listener.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "kv_adapter_manager.h"
#endif
#if !defined(__LITEOS_M__)
#include "ipc_notify_dmfa_result_req.h"
#include "ipc_server_listener.h"
#endif
#include "ipc_notify_device_state_req.h"

namespace OHOS {
namespace DistributedHardware {
class DM_EXPORT DeviceManagerServiceListener : public IDeviceManagerServiceListener {
public:
    DeviceManagerServiceListener() {};
    virtual ~DeviceManagerServiceListener() {};

    void OnDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
        const DmDeviceInfo &info, const bool isOnline) override;

    void OnDeviceFound(const ProcessInfo &processInfo, uint16_t subscribeId, const DmDeviceInfo &info) override;

    void OnDiscoveryFailed(const ProcessInfo &processInfo, uint16_t subscribeId, int32_t failedReason) override;

    void OnDiscoverySuccess(const ProcessInfo &processInfo, int32_t subscribeId) override;

    void OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult) override;

    void OnAuthResult(const ProcessInfo &processInfo, const std::string &deviceId, const std::string &token,
        int32_t status, int32_t reason) override;

    void OnUiCall(const ProcessInfo &processInfo, std::string &paramJson) override;

    void OnCredentialResult(const ProcessInfo &processInfo, int32_t action, const std::string &resultInfo) override;

    DM_EXPORT void OnBindResult(const ProcessInfo &processInfo,
        const PeerTargetId &targetId, int32_t result, int32_t status, std::string content) override;

    DM_EXPORT void OnUnbindResult(const ProcessInfo &processInfo,
        const PeerTargetId &targetId, int32_t result, std::string content) override;

    void OnPinHolderCreate(const ProcessInfo &processInfo, const std::string &deviceId, DmPinType pinType,
        const std::string &payload) override;
    void OnPinHolderDestroy(const ProcessInfo &processInfo, DmPinType pinType, const std::string &payload) override;
    void OnCreateResult(const ProcessInfo &processInfo, int32_t result) override;
    void OnDestroyResult(const ProcessInfo &processInfo, int32_t result) override;
    void OnPinHolderEvent(const ProcessInfo &processInfo, DmPinHolderEvent event, int32_t result,
        const std::string &content) override;
    void OnDeviceTrustChange(const std::string &udid, const std::string &uuid, DmAuthForm authForm) override;
    void OnDeviceScreenStateChange(const ProcessInfo &processInfo, DmDeviceInfo &devInfo) override;
    void OnCredentialAuthStatus(const ProcessInfo &processInfo, const std::string &deviceList, uint16_t deviceTypeId,
                                int32_t errcode) override;
    void OnAppUnintall(const std::string &pkgName) override;
    void OnSinkBindResult(const ProcessInfo &processInfo, const PeerTargetId &targetId, int32_t result,
        int32_t status, std::string content) override;
    void OnProcessRemove(const ProcessInfo &processInfo) override;
    void OnDevStateCallbackAdd(const ProcessInfo &processInfo, const std::vector<DmDeviceInfo> &deviceList) override;
    void OnGetDeviceProfileInfoListResult(const ProcessInfo &processInfo,
        const std::vector<DmDeviceProfileInfo> &deviceProfileInfos, int32_t code) override;
    void OnGetDeviceIconInfoResult(const ProcessInfo &processInfo,
        const DmDeviceIconInfo &dmDeviceIconInfo, int32_t code) override;
    void OnSetLocalDeviceNameResult(const ProcessInfo &processInfo,
        const std::string &deviceName, int32_t code) override;
    void OnSetRemoteDeviceNameResult(const ProcessInfo &processInfo, const std::string &deviceId,
        const std::string &deviceName, int32_t code) override;
    void SetExistPkgName(const std::set<std::string> &pkgNameSet) override;
    void OnServiceFound(const ProcessInfo &processInfo, const DmServiceInfo &service) override;
    void OnServiceDiscoveryResult(const ProcessInfo &processInfo, const std::string &serviceType,
        int32_t reason) override;
    void OnDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
        const DmDeviceInfo &info, const std::vector<int64_t> &serviceIds) override;

    std::string GetLocalDisplayDeviceName() override;
    int32_t OpenAuthSessionWithPara(const std::string &deviceId, int32_t actionId, bool isEnable160m) override;
    int32_t OpenAuthSessionWithPara(int64_t serviceId) override;
    void OnServicePublishResult(const ProcessInfo &processInfo, int64_t serviceId, int32_t publishResult) override;
    void OnDevDbReadyCallbackAdd(const ProcessInfo &processInfo, const std::vector<DmDeviceInfo> &deviceList) override;
    void OnLeaveLNNResult(const std::string &pkgName, const std::string &networkId, int32_t retCode) override;
    void OnAuthCodeInvalid(const std::string &pkgName) override;
private:
    void ConvertDeviceInfoToDeviceBasicInfo(const std::string &pkgName,
        const DmDeviceInfo &info, DmDeviceBasicInfo &deviceBasicInfo);
    void SetDeviceInfo(std::shared_ptr<IpcNotifyDeviceStateReq> pReq, const ProcessInfo &processInfo,
        const DmDeviceState &state, const DmDeviceInfo &deviceInfo, const DmDeviceBasicInfo &deviceBasicInfo);
    int32_t FillUdidAndUuidToDeviceInfo(const std::string &pkgName, DmDeviceInfo &dmDeviceInfo);
    void ProcessDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
        const DmDeviceBasicInfo &deviceBasicInfo, const bool isOnline);
    void ProcessAppStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
        const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo, const bool isOnline);
    void SetDeviceScreenInfo(std::shared_ptr<IpcNotifyDeviceStateReq> pReq, const ProcessInfo &processInfo,
        const DmDeviceInfo &deviceInfo);
    void RemoveOnlinePkgName(const DmDeviceInfo &info);
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    DM_EXPORT int32_t ConvertUdidHashToAnoyAndSave(const std::string &pkgName,
        DmDeviceInfo &deviceInfo, const int32_t userId);
    int32_t ConvertUdidHashToAnoyDeviceId(const std::string &pkgName, const std::string &udidHash,
        std::string &anoyDeviceId, const int32_t userId);
#endif
    std::vector<ProcessInfo> GetWhiteListSAProcessInfo(DmCommonNotifyEvent dmCommonNotifyEvent);
    std::vector<ProcessInfo> GetNotifyProcessInfoByUserId(int32_t userId, DmCommonNotifyEvent dmCommonNotifyEvent);
    ProcessInfo DealBindProcessInfo(const ProcessInfo &processInfo);
    void ProcessDeviceOnline(const std::vector<ProcessInfo> &procInfoVec, const ProcessInfo &processInfo,
        const DmDeviceState &state, const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo);
    void ProcessDeviceOffline(const std::vector<ProcessInfo> &procInfoVec, const ProcessInfo &processInfo,
        const DmDeviceState &state, const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo,
        const bool isOnline);
    void ProcessDeviceInfoChange(std::vector<ProcessInfo> &procInfoVec, const ProcessInfo &processInfo,
        const DmDeviceState &state, const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo);
    void ProcessAppOnline(std::vector<ProcessInfo> &procInfoVec, const ProcessInfo &processInfo,
        const DmDeviceState &state, const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo);
    void ProcessAppOffline(std::vector<ProcessInfo> &procInfoVec, const ProcessInfo &processInfo,
        const DmDeviceState &state, const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo,
        const bool isOnline);
    void RemoveNotExistProcess();
    void ProcessDeviceStateChange(const ProcessInfo &processInfo, const DmDeviceState &state, const DmDeviceInfo &info,
        const DmDeviceBasicInfo &deviceBasicInfo, const std::vector<int64_t> &serviceIds);
    void ProcessAppStateChange(const ProcessInfo &processInfo, const DmDeviceState &state,
        const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo,
        const std::vector<int64_t> &serviceIds);
    void ProcessDeviceOnline(const std::vector<ProcessInfo> &procinfoVec, const ProcessInfo &processInfo,
        const DmDeviceState &state, const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo,
        const std::vector<int64_t> &serviceIds);
    void ProcessAppOnline(std::vector<ProcessInfo> &procInfoVec, const ProcessInfo &processInfo,
        const DmDeviceState &state, const DmDeviceInfo &info, const DmDeviceBasicInfo &deviceBasicInfo,
        const std::vector<int64_t> &serviceIds);
    void ClearDbReadyMap(std::string &notifyPkgName);
    void SetNeedNotifyProcessInfos(const ProcessInfo &processInfo, std::vector<ProcessInfo> &procInfoVec);
private:
#if !defined(__LITEOS_M__)
    IpcServerListener ipcServerListener_;
    static std::mutex alreadyNotifyPkgNameLock_;
    static std::map<std::string, DmDeviceInfo> alreadyOnlinePkgName_;
    static std::mutex alreadyDbReadyPkgNameLock_;
    static std::map<std::string, DmDeviceInfo> alreadyDbReadyPkgName_;
    static std::unordered_set<std::string> highPriorityPkgNameSet_;
    static std::mutex actUnrelatedPkgNameLock_;
    static std::set<std::string> actUnrelatedPkgName_;
#endif
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_LISTENER_H