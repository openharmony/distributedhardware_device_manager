/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_DEVICE_MANAGER_IMPL_H
#define OHOS_DEVICE_MANAGER_IMPL_H

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>

#include "ffi_remote_data.h"

#include "device_manager_ffi.h"
#include "device_manager_utils.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerFfiImpl : public OHOS::FFI::FFIData {
public:
    explicit DeviceManagerFfiImpl(const std::string &bundleName, int32_t *errCode);
    ~DeviceManagerFfiImpl() override = default;

    static DeviceManagerFfiImpl *GetDeviceManagerFfi(std::string &bundleName);

    int32_t ReleaseDeviceManager();
    int32_t GetAvailableDeviceList(FfiDeviceBasicInfoArray &deviceInfoList);
    static void DeviceListFree(FfiDeviceBasicInfoArray &deviceInfoList, int64_t size = -1);
    int32_t GetLocalDeviceNetworkId(const char *&networkId);
    int32_t GetLocalDeviceName(const char *&deviceName);
    int32_t GetLocalDeviceType(int32_t &deviceType);
    int32_t GetLocalDeviceId(const char *&deviceId);
    int32_t GetDeviceName(const std::string &networkId, const char *&deviceName);
    int32_t GetDeviceType(const std::string &networkId, int32_t &deviceType);
    int32_t StartDiscovering(const std::string &extra);
    int32_t StopDiscovering();
    int32_t BindTarget(const std::string &deviceId, const std::string &bindParam, bool isMetaType);
    int32_t UnbindTarget(const std::string &deviceId);
    int32_t EventOn(const std::string &type, void *callback);
    int32_t EventOff(const std::string &type);

    void OnDeviceStatusChange(int32_t action, const DmDeviceBasicInfo &deviceBasicInfo);
    void OnDeviceNameChange(const std::string &deviceName);
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceBasicInfo &deviceBasicInfo);
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason);
    void OnPublishResult(int32_t publishId, int32_t publishResult);
    void OnAuthResult(const std::string &deviceId, const std::string &token, int32_t status, int32_t reason);
    void OnDmUiCall(const std::string &paramJson);

private:
    void ClearBundleCallbacks();
    static int32_t Transform2FfiDeviceBasicInfo(const DmDeviceBasicInfo &in, FfiDeviceBasicInfo &out);
    int32_t BindTargetWarpper(const std::string &deviceId,
        const std::string &bindParam, std::shared_ptr<DmFfiBindTargetCallback> callback);
    int32_t BindDevice(const std::string &deviceId, const std::string &bindParam);
    int32_t WaitForCallbackCv();

    int32_t RegisterDevStatusCallback();
    int32_t RegisterDiscoveryCallback();
    int32_t RegisterPublishCallback();
    int32_t RegisterReplyCallback();

    int32_t ReleaseDevStatusCallback();
    int32_t ReleaseDiscoveryCallback();
    int32_t ReleasePublishCallback();
    int32_t ReleaseReplyCallback();

    void RegisterCallbackByType(const std::string &type, void *callback);
    void Off(const std::string &type);

    inline static void FreeDeviceInfo(FfiDeviceBasicInfo &info)
    {
        free(info.deviceId);
        info.deviceId = nullptr;
        free(info.deviceName);
        info.deviceName = nullptr;
        free(info.networkId);
        info.networkId = nullptr;
    };

    std::string bundleName_;
    std::function<void(int32_t, FfiDeviceBasicInfo *)> deviceStateChangedCallback;
    std::function<void(FfiDeviceBasicInfo *)> discoverSuccessCallback;
    std::function<void(const char *)> deviceNameChangedCallback;
    std::function<void(int32_t)> deviceDiscoverFailedCallback;
    std::mutex callbackLock;

    std::condition_variable callbackFinishedCv;
    std::mutex callbackFinishedMutex;
    bool callbackFinished = false;
    std::atomic<int32_t> errCode_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IMPL_H
