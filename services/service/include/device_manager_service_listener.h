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

#ifndef OHOS_DM_SERVICE_LISTENER_H
#define OHOS_DM_SERVICE_LISTENER_H

#include <map>
#include <string>
#include <unordered_set>
#include <mutex>

#include "dm_device_info.h"
#include "idevice_manager_service_listener.h"
#if !defined(__LITEOS_M__)
#include "ipc_notify_dmfa_result_req.h"
#include "ipc_server_listener.h"
#include "ipc_notify_device_state_req.h"
#endif

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerServiceListener : public IDeviceManagerServiceListener {
public:
    DeviceManagerServiceListener() {};
    virtual ~DeviceManagerServiceListener() {};

    void OnDeviceStateChange(const std::string &pkgName, const DmDeviceState &state, const DmDeviceInfo &info) override;

    void OnDeviceFound(const std::string &pkgName, uint16_t subscribeId, const DmDeviceInfo &info) override;

    void OnDeviceFound(const std::string &pkgName, uint16_t subscribeId, DmDeviceBasicInfo &info) override;

    void OnDiscoveryFailed(const std::string &pkgName, uint16_t subscribeId, int32_t failedReason) override;

    void OnDiscoverySuccess(const std::string &pkgName, int32_t subscribeId) override;

    void OnPublishResult(const std::string &pkgName, int32_t publishId, int32_t publishResult) override;

    void OnAuthResult(const std::string &pkgName, const std::string &deviceId, const std::string &token, int32_t status,
                      int32_t reason) override;

    void OnUiCall(std::string &pkgName, std::string &paramJson) override;

    void OnCredentialResult(const std::string &pkgName, int32_t action, const std::string &resultInfo) override;

    void OnBindResult(const std::string &pkgName, const PeerTargetId &targetId, int32_t result,
        int32_t status, std::string content) override;

    void OnUnbindResult(const std::string &pkgName, const PeerTargetId &targetId, int32_t result,
        std::string content) override;

    void DeleteDeviceIdFromMap(const std::string &deviceId, const std::string &pkgName);
    void RegisterDmListener(const std::string &pkgName, const std::string &appId);
    void UnRegisterDmListener(const std::string &pkgName);
    std::string GetAppId(const std::string &pkgName);
    std::string CalcDeviceId(const std::string &udidHash, const std::string &appId);
    void SetUdidHashMap(const std::string &udidHash, const std::string &deviceId, const std::string &pkgName);
    std::string GetDeviceId(const std::string &udidHash, const std::string &pkgName);
    std::string GetUdidHash(const std::string &deviceId, const std::string &pkgName);
    void OnPinHolderCreate(const std::string &pkgName, const std::string &deviceId, DmPinType pinType,
        const std::string &payload) override;
    void OnPinHolderDestroy(const std::string &pkgName, DmPinType pinType, const std::string &payload) override;
    void OnCreateResult(const std::string &pkgName, int32_t result) override;
    void OnDestroyResult(const std::string &pkgName, int32_t result) override;
    void OnPinHolderEvent(const std::string &pkgName, DmPinHolderEvent event, int32_t result,
        const std::string &content) override;

private:
    void ConvertDeviceInfoToDeviceBasicInfo(const std::string &pkgName,
        const DmDeviceInfo &info, DmDeviceBasicInfo &deviceBasicInfo);
    void SetDeviceInfo(std::shared_ptr<IpcNotifyDeviceStateReq> pReq, const std::string &pkgName,
        const DmDeviceState &state, const DmDeviceInfo &deviceInfo, const DmDeviceBasicInfo &deviceBasicInfo);
private:
#if !defined(__LITEOS_M__)
    IpcServerListener ipcServerListener_;
    static std::mutex dmListenerMapLock_;
    static std::map<std::string, std::string> dmListenerMap_;
    static std::mutex udidHashMapLock_;
    static std::map<std::string, std::map<std::string, std::string>> udidHashMap_;
    static std::mutex alreadyOnlineSetLock_;
    static std::unordered_set<std::string> alreadyOnlineSet_;
#endif
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_LISTENER_H