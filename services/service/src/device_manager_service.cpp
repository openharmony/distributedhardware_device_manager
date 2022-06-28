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

#include "device_manager_service.h"

#include <dlfcn.h>
#include <functional>

#include "dm_constants.h"
#include "dm_log.h"
#include "permission_manager.h"
#include "dm_distributed_hardware_load.h"

const std::string LIB_IMPL_NAME = "libdevicemanagerserviceimpl.z.so";

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);

DeviceManagerService::~DeviceManagerService()
{
    LOGI("DeviceManagerService destructor");
    if (dmServiceImpl_ != nullptr) {
        dmServiceImpl_->Release();
    }
    std::string soPathName = LIB_LOAD_PATH + LIB_IMPL_NAME;
    void *so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
    if (so_handle != nullptr) {
        dlclose(so_handle);
    }
}

int32_t DeviceManagerService::Init()
{
    if (softbusListener_ == nullptr) {
        softbusListener_ = std::make_shared<SoftbusListener>();
    }

    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
    }

    LOGI("Init success, dm service single instance initialized.");
    return DM_OK;
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                   std::vector<DmDeviceInfo> &deviceList)
{
    if (pkgName.empty()) {
        LOGE("GetTrustedDeviceList failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    int32_t ret = softbusListener_->GetTrustedDeviceList(deviceList);
    if (ret != DM_OK) {
        LOGE("GetTrustedDeviceList failed");
    }
    return ret;
}

int32_t DeviceManagerService::GetLocalDeviceInfo(DmDeviceInfo &info)
{
    if (!PermissionManager::GetInstance().CheckPermission()) {
        LOGI("The caller does not have permission to call");
        return ERR_DM_NO_PERMISSION;
    }
    int32_t ret = softbusListener_->GetLocalDeviceInfo(info);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceInfo failed");
    }
    return ret;
}

int32_t DeviceManagerService::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &udid)
{
    if (pkgName.empty()) {
        LOGE("GetUdidByNetworkId failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    SoftbusListener::GetUdidByNetworkId(netWorkId.c_str(), udid);
    return DM_OK;
}

int32_t DeviceManagerService::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &uuid)
{
    if (pkgName.empty()) {
        LOGE("GetUuidByNetworkId failed, pkgName is empty");
        return ERR_DM_INPUT_PARAMETER_EMPTY;
    }
    SoftbusListener::GetUuidByNetworkId(netWorkId.c_str(), uuid);
    return DM_OK;
}

int32_t DeviceManagerService::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                                   const std::string &extra)
{
    if (!IsDMServiceImplReady()) {
        LOGE("StartDeviceDiscovery failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
}

int32_t DeviceManagerService::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    if (!IsDMServiceImplReady()) {
        LOGE("StopDeviceDiscovery failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->StopDeviceDiscovery(pkgName, subscribeId);
}

int32_t DeviceManagerService::AuthenticateDevice(const std::string &pkgName, int32_t authType,
                                                 const std::string &deviceId, const std::string &extra)
{
    if (!IsDMServiceImplReady()) {
        LOGE("AuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->AuthenticateDevice(pkgName, authType, deviceId, extra);
}

int32_t DeviceManagerService::UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId)
{
    if (!IsDMServiceImplReady()) {
        LOGE("UnAuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnAuthenticateDevice(pkgName, deviceId);
}

int32_t DeviceManagerService::VerifyAuthentication(const std::string &authParam)
{
    if (!IsDMServiceImplReady()) {
        LOGE("VerifyAuthentication failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->VerifyAuthentication(authParam);
}

int32_t DeviceManagerService::GetFaParam(std::string &pkgName, DmAuthParam &authParam)
{
    if (!IsDMServiceImplReady()) {
        LOGE("GetFaParam failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->GetFaParam(pkgName, authParam);
}

int32_t DeviceManagerService::SetUserOperation(std::string &pkgName, int32_t action)
{
    if (!IsDMServiceImplReady()) {
        LOGE("SetUserOperation failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->SetUserOperation(pkgName, action);
}

int32_t DeviceManagerService::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (!IsDMServiceImplReady()) {
        LOGE("RegisterDevStateCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RegisterDevStateCallback(pkgName, extra);
}

int32_t DeviceManagerService::UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (!IsDMServiceImplReady()) {
        LOGE("UnRegisterDevStateCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnRegisterDevStateCallback(pkgName, extra);
}

void DeviceManagerService::HandleDeviceOnline(const DmDeviceInfo &info)
{
    if (!IsDMServiceImplReady()) {
        LOGE("HandleDeviceOnline failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->HandleDeviceOnline(info);
}

void DeviceManagerService::HandleDeviceOffline(const DmDeviceInfo &info)
{
    if (!IsDMServiceImplReady()) {
        LOGE("HandleDeviceOffline failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->HandleDeviceOffline(info);
}

int DeviceManagerService::OnSessionOpened(int sessionId, int result)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnSessionOpened failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->OnSessionOpened(sessionId, result);
}

void DeviceManagerService::OnSessionClosed(int sessionId)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnSessionClosed failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnSessionClosed(sessionId);
}

void DeviceManagerService::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (!IsDMServiceImplReady()) {
        LOGE("OnBytesReceived failed, instance not init or init failed.");
        return;
    }
    dmServiceImpl_->OnBytesReceived(sessionId, data, dataLen);
}

bool DeviceManagerService::IsDMServiceImplReady()
{
    if (isImplsoLoaded_ && (dmServiceImpl_ != nullptr)) {
        return true;
    }

    std::string soName = LIB_LOAD_PATH + LIB_IMPL_NAME;
    void *so_handle = dlopen(soName.c_str(), RTLD_NOW | RTLD_NOLOAD);
    if (so_handle == nullptr) {
        so_handle = dlopen(soName.c_str(), RTLD_NOW);
        if (so_handle == nullptr) {
            LOGE("load profile so %s failed.", soName.c_str());
            return false;
        }
    }
    dlerror();
    auto func = (CreateDMServiceFuncPtr)dlsym(so_handle, "CreateDMServiceObject");
    if (dlerror() != nullptr || func == nullptr) {
        LOGE("Create object function is not exist.");
        return false;
    }

    dmServiceImpl_ = std::shared_ptr<IDeviceManagerServiceImpl>(func());
    if (dmServiceImpl_->Initialize(listener_) != DM_OK) {
        dlclose(so_handle);
        dmServiceImpl_ = nullptr;
        isImplsoLoaded_ = false;
        return false;
    }
    isImplsoLoaded_ = true;
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
