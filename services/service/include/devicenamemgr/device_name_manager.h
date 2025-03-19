/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_NAME_MANAGER_H
#define OHOS_DEVICE_NAME_MANAGER_H

#include <memory>
#include <mutex>
#include <string>

#include "datashare_helper.h"
#include "device_name_change_monitor.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceNameManager {
    DECLARE_SINGLE_INSTANCE_BASE(DeviceNameManager);
public:
    int32_t UnInit();

    void DataShareReady();
    void AccountSysReady(int32_t userId);
    int32_t InitDeviceNameWhenSoftBusReady();

    int32_t GetLocalDisplayDeviceName(int32_t maxNamelength, std::string &displayName);
    int32_t InitDeviceNameWhenUserSwitch(int32_t curUserId, int32_t preUserId);
    int32_t InitDeviceNameWhenLogout();
    int32_t InitDeviceNameWhenLogin();
    int32_t InitDeviceNameWhenNickChange();
    int32_t InitDeviceNameWhenNameChange(int32_t userId);
    int32_t ModifyUserDefinedName(const std::string &deviceName);
    int32_t RestoreLocalDeviceName();

private:
    DeviceNameManager() = default;
    ~DeviceNameManager() = default;
    bool DependsIsReady();
    void RegisterDeviceNameChangeMonitor(int32_t curUserId, int32_t preUserId);
    void UnRegisterDeviceNameChangeMonitor(int32_t userId);
    void InitDeviceName(int32_t userId);
    void InitDeviceNameToSoftBus(const std::string &prefixName, const std::string &subffixName);
    std::string GetLocalDisplayDeviceName(const std::string &prefixName, const std::string &subffixName,
        int32_t maxNamelength);
    std::string SubstrByBytes(const std::string &str, int32_t maxNumBytes);

    std::string GetSystemLanguage();
    std::string GetSystemRegion();
    std::string GetLocalMarketName();

    int32_t InitDisplayDeviceNameToSettingsData(const std::string &nickName, const std::string &deviceName,
        int32_t userId);
    int32_t GetUserDefinedDeviceName(int32_t userId, std::string &deviceName);
    int32_t SetUserDefinedDeviceName(const std::string &deviceName, int32_t userId);
    int32_t GetDisplayDeviceName(int32_t userId, std::string &deviceName);
    int32_t SetDisplayDeviceName(const std::string &deviceName, int32_t userId);
    int32_t SetDisplayDeviceNameState(const std::string &state, int32_t userId);
    int32_t GetDeviceName(std::string &deviceName);
    int32_t SetDeviceName(const std::string &deviceName);
    int32_t GetValue(const std::string &tableName, int32_t userId, const std::string &key, std::string &value);
    int32_t SetValue(const std::string &tableName, int32_t userId, const std::string &key, const std::string &value);
    std::string GetProxyUriStr(const std::string &tableName, int32_t userId);
    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(const std::string &proxyUri);
    Uri MakeUri(const std::string &proxyUri, const std::string &key);
    bool ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> helper);

    sptr<IRemoteObject> GetRemoteObj();
private:
    std::mutex remoteObjMtx_;
    sptr<IRemoteObject> remoteObj_ = nullptr;
    std::mutex localMarketNameMtx_;
    std::string localMarketName_ = "";
    std::mutex monitorMapMtx_;
    std::map<int32_t, sptr<DeviceNameChangeMonitor>> monitorMap_;
    std::atomic<bool> isDataShareReady_ = false;
    std::atomic<bool> isAccountSysReady_ = false;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_DEVICE_NAME_MANAGER_H