#ifndef OHOS_DISTRIBUTED_DEVICE_MANAGER_H
#define OHOS_DISTRIBUTED_DEVICE_MANAGER_H

#include <string>
#include <functional>
#include "taihe/callback.hpp"
#include "dm_device_info.h"
#include "device_manager_callback.h"

namespace ANI::distributedDeviceManager {
using namespace taihe;
using namespace ohos::distributedDeviceManager;


class DeviceManagerImpl {
    public:
        DeviceManagerImpl() = default;
        explicit DeviceManagerImpl(std::shared_ptr<DeviceManagerImpl> impl);
        explicit DeviceManagerImpl(const std::string& bundleName) ;
        ~DeviceManagerImpl() = default;
        string getLocalDeviceId();
        void unbindTarget(string_view deviceId);
        double getDeviceType(string_view networkId);
        string getDeviceName(string_view networkId);
        string getLocalDeviceNetworkId();

        void ondeviceNameChange(callback_view<string(string_view)> f);
        void ondiscoverFailure(callback_view<void(int32_t)> f);
        void onreplyResult(callback_view<string(string_view)> f);
        void onserviceDie(callback_view<string(string_view)> f);
        void ondiscoverSuccess(callback_view<DeviceBasicInfo(DeviceBasicInfo const&)> f);
        void ondeviceStateChange(callback_view<DeviceStatusStructer(DeviceStatusStructer const&)> f);

        void offdeviceNameChange(callback_view<string(string_view)> f);
        void offdiscoverFailure(callback_view<void(int32_t)> f);
        void offreplyResult(callback_view<string(string_view)> f);
        void offserviceDie(callback_view<string(string_view)> f);
        void offdiscoverSuccess(callback_view<DeviceBasicInfo(DeviceBasicInfo const&)> f);
        void offdeviceStateChange(callback_view<DeviceStatusStructer(DeviceStatusStructer const&)> f);

        friend DeviceManager createDeviceManager(string_view bundleName);

    private:
        std::shared_ptr<ohos::distributedDeviceManager::DeviceManager> deviceManager_;
        std::string bundleName_;
};

DeviceManager createDeviceManager(string_view bundleName);

class DmAniInitCallback : public OHOS::DistributedHardware::DmInitCallback {
    public:
        explicit DmAniInitCallback(string_view bundleName) : bundleName_(std::string(bundleName))
        {
        }
        ~DmAniInitCallback() override {}
        void OnRemoteDied() override;

    private:
        std::string bundleName_;
    };
}

#endif