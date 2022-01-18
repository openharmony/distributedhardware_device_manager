/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "dm_device_state_manager.h"

#include "dm_adapter_manager.h"
#include "dm_constants.h"
#include "dm_log.h"
namespace OHOS {
namespace DistributedHardware {
DmDeviceStateManager::DmDeviceStateManager(std::shared_ptr<SoftbusConnector> softbusConnector,
                                           std::shared_ptr<DeviceManagerServiceListener> listener)
    : softbusConnector_(softbusConnector), listener_(listener)
{
    LOGI("DmDeviceStateManager constructor");
    profileSoName_ = "libdevicemanagerext_profile.z.so";
}

DmDeviceStateManager::~DmDeviceStateManager()
{
    LOGI("DmDeviceStateManager destructor");
    softbusConnector_->UnRegisterSoftbusStateCallback("DM_PKG_NAME");
}

void DmDeviceStateManager::OnDeviceOnline(const std::string &pkgName, const DmDeviceInfo &info)
{
    LOGI("DmDeviceStateManager::OnDeviceOnline in");
    DmAdapterManager &adapterMgrPtr = DmAdapterManager::GetInstance();
    std::shared_ptr<IProfileAdapter> profileAdapter = adapterMgrPtr.GetProfileAdapter(profileSoName_);
    if (profileAdapter == nullptr) {
        LOGE("OnDeviceOnline profile adapter is null");
    } else {
        uint8_t udid[UDID_BUF_LEN] = {0};
        int32_t ret = SoftbusConnector::GetNodeKeyInfoByNetworkId(info.deviceId, NodeDeivceInfoKey::NODE_KEY_UDID, udid,
                                                                  sizeof(udid));
        if (ret != DM_OK) {
            LOGE("DmDeviceStateManager::OnDeviceOnline GetNodeKeyInfo failed");
        } else {
            std::string deviceUdid = (char *)udid;
            DmDeviceInfo saveInfo = info;
            std::string uuid;
            SoftbusConnector::GetUuidByNetworkId(info.deviceId, uuid);
            remoteDeviceInfos_[uuid] = saveInfo;
            LOGI("RegisterProfileListener in, deviceId = %s, deviceUdid = %s, uuid = %s",
                 info.deviceId, deviceUdid.c_str(), uuid.c_str());
            profileAdapter->RegisterProfileListener(pkgName, deviceUdid, shared_from_this());
            LOGI("RegisterProfileListener out");
        }
    }
    DmDeviceState state = DEVICE_STATE_ONLINE;
    deviceStateMap_[info.deviceId] = DEVICE_STATE_ONLINE;
    listener_->OnDeviceStateChange(pkgName, state, info);
    LOGI("DmDeviceStateManager::OnDeviceOnline out");
}

void DmDeviceStateManager::OnDeviceOffline(const std::string &pkgName, const DmDeviceInfo &info)
{
    DmAdapterManager &adapterMgrPtr = DmAdapterManager::GetInstance();
    std::shared_ptr<IProfileAdapter> profileAdapter = adapterMgrPtr.GetProfileAdapter(profileSoName_);
    if (profileAdapter == nullptr) {
        LOGE("OnDeviceOffline profile adapter is null");
    } else {
        profileAdapter->UnRegisterProfileListener(pkgName);
        std::string uuid;
        SoftbusConnector::GetUuidByNetworkId(info.deviceId, uuid);
        auto iter = remoteDeviceInfos_.find(std::string(info.deviceId));
        if (iter == remoteDeviceInfos_.end()) {
        } else {
            remoteDeviceInfos_.erase(std::string(info.deviceId));
        }
    }
    DmDeviceState state = DEVICE_STATE_OFFLINE;
    deviceStateMap_[info.deviceId] = DEVICE_STATE_OFFLINE;
    listener_->OnDeviceStateChange(pkgName, state, info);
}

void DmDeviceStateManager::OnDeviceChanged(const std::string &pkgName, const DmDeviceInfo &info)
{
    deviceStateMap_[info.deviceId] = DEVICE_INFO_CHANGED;
}

void DmDeviceStateManager::OnDeviceReady(const std::string &pkgName, const DmDeviceInfo &info)
{
    deviceStateMap_[info.deviceId] = DEVICE_INFO_READY;
}

void DmDeviceStateManager::OnProfileReady(const std::string &pkgName, const std::string deviceId)
{
    DmDeviceInfo saveInfo;
    auto iter = remoteDeviceInfos_.find(deviceId);
    if (iter == remoteDeviceInfos_.end()) {
        LOGE("DmDeviceStateManager::OnProfileReady complete not find deviceID = %s", deviceId.c_str());
    } else {
        saveInfo = iter->second;
    }
    DmDeviceState state = DEVICE_INFO_READY;
    listener_->OnDeviceStateChange(pkgName, state, saveInfo);
}

int32_t DmDeviceStateManager::RegisterSoftbusStateCallback()
{
    softbusConnector_->RegisterSoftbusStateCallback(DM_PKG_NAME,
                                                    std::shared_ptr<DmDeviceStateManager>(shared_from_this()));
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
