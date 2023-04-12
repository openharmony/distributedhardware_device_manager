/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_hidumper.h"
#include "dm_log.h"

constexpr const char* LIB_IMPL_NAME = "libdevicemanagerserviceimpl.z.so";

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService);

DeviceManagerService::~DeviceManagerService()
{
    LOGI("DeviceManagerService destructor");
    if (dmServiceImpl_ != nullptr) {
        dmServiceImpl_->Release();
    }
    std::string soPathName = std::string(LIB_LOAD_PATH) + std::string(LIB_IMPL_NAME);
    void *so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
    if (so_handle != nullptr) {
        dlclose(so_handle);
    }
}

int32_t DeviceManagerService::Init()
{
    InitSoftbusListener();
    InitDMServiceListener();

    LOGI("Init success, dm service single instance initialized.");
    return DM_OK;
}

int32_t DeviceManagerService::InitSoftbusListener()
{
    if (softbusListener_ == nullptr) {
        softbusListener_ = std::make_shared<SoftbusListener>();
    }
    LOGI("SoftbusListener init success.");

    return DM_OK;
}

void DeviceManagerService::UninitSoftbusListener()
{
    softbusListener_ = nullptr;
    LOGI("SoftbusListener uninit.");
}

int32_t DeviceManagerService::InitDMServiceListener()
{
    if (listener_ == nullptr) {
        listener_ = std::make_shared<DeviceManagerServiceListener>();
    }

    LOGI("DeviceManagerServiceListener init success.");
    return DM_OK;
}

void DeviceManagerService::UninitDMServiceListener()
{
    listener_ = nullptr;
    LOGI("DeviceManagerServiceListener uninit.");
}

int32_t DeviceManagerService::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                   std::vector<DmDeviceInfo> &deviceList)
{
    LOGI("DeviceManagerService::GetTrustedDeviceList begin for pkgName = %s, extra = %s", pkgName.c_str(),
        extra.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    int32_t ret = softbusListener_->GetTrustedDeviceList(deviceList);
    if (ret != DM_OK) {
        LOGE("GetTrustedDeviceList failed");
    }
    return ret;
}

int32_t DeviceManagerService::GetLocalDeviceInfo(DmDeviceInfo &info)
{
    LOGI("DeviceManagerService::GetLocalDeviceInfo begin.");
    int32_t ret = softbusListener_->GetLocalDeviceInfo(info);
    if (ret != DM_OK) {
        LOGE("GetLocalDeviceInfo failed");
    }
    return ret;
}

int32_t DeviceManagerService::GetUdidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &udid)
{
    LOGI("DeviceManagerService::GetUdidByNetworkId begin for pkgName = %s", pkgName.c_str());
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("Invalid parameter, pkgName: %s, netWorkId: %s", pkgName.c_str(), GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SoftbusListener::GetUdidByNetworkId(netWorkId.c_str(), udid);
    return DM_OK;
}

int32_t DeviceManagerService::GetUuidByNetworkId(const std::string &pkgName, const std::string &netWorkId,
                                                 std::string &uuid)
{
    LOGI("DeviceManagerService::GetUuidByNetworkId begin for pkgName = %s", pkgName.c_str());
    if (pkgName.empty() || netWorkId.empty()) {
        LOGE("Invalid parameter, pkgName: %s, netWorkId: %s", pkgName.c_str(), GetAnonyString(netWorkId).c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    SoftbusListener::GetUuidByNetworkId(netWorkId.c_str(), uuid);
    return DM_OK;
}

int32_t DeviceManagerService::StartDeviceDiscovery(const std::string &pkgName, const DmSubscribeInfo &subscribeInfo,
                                                   const std::string &extra)
{
    LOGI("DeviceManagerService::StartDeviceDiscovery begin for pkgName = %s, extra = %s", pkgName.c_str(),
        extra.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("StartDeviceDiscovery failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->StartDeviceDiscovery(pkgName, subscribeInfo, extra);
}

int32_t DeviceManagerService::StopDeviceDiscovery(const std::string &pkgName, uint16_t subscribeId)
{
    LOGI("DeviceManagerService::StopDeviceDiscovery begin for pkgName = %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("StopDeviceDiscovery failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->StopDeviceDiscovery(pkgName, subscribeId);
}

int32_t DeviceManagerService::PublishDeviceDiscovery(const std::string &pkgName, const DmPublishInfo &publishInfo)
{
    LOGI("DeviceManagerService::PublishDeviceDiscovery begin for pkgName = %s", pkgName.c_str());
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("PublishDeviceDiscovery failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->PublishDeviceDiscovery(pkgName, publishInfo);
}

int32_t DeviceManagerService::UnPublishDeviceDiscovery(const std::string &pkgName, int32_t publishId)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnPublishDeviceDiscovery failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnPublishDeviceDiscovery(pkgName, publishId);
}

int32_t DeviceManagerService::AuthenticateDevice(const std::string &pkgName, int32_t authType,
                                                 const std::string &deviceId, const std::string &extra)
{
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerService::AuthenticateDevice error: Invalid parameter, pkgName: %s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("AuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->AuthenticateDevice(pkgName, authType, deviceId, extra);
}

int32_t DeviceManagerService::UnAuthenticateDevice(const std::string &pkgName, const std::string &deviceId)
{
    LOGI("DeviceManagerService::UnAuthenticateDevice begin for pkgName = %s, deviceId = %s",
        pkgName.c_str(), GetAnonyString(deviceId).c_str());
    if (pkgName.empty() || deviceId.empty()) {
        LOGE("DeviceManagerService::UnAuthenticateDevice error: Invalid parameter, pkgName: %s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("UnAuthenticateDevice failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnAuthenticateDevice(pkgName, deviceId);
}

int32_t DeviceManagerService::VerifyAuthentication(const std::string &authParam)
{
    if (authParam.empty()) {
        LOGE("DeviceManagerService::VerifyAuthentication error: Invalid parameter, authParam: %s", authParam.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("DeviceManagerService::VerifyAuthentication failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->VerifyAuthentication(authParam);
}

int32_t DeviceManagerService::GetFaParam(std::string &pkgName, DmAuthParam &authParam)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("GetFaParam failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->GetFaParam(pkgName, authParam);
}

int32_t DeviceManagerService::SetUserOperation(std::string &pkgName, int32_t action, const std::string &params)
{
    if (pkgName.empty() || params.empty()) {
        LOGE("DeviceManagerService::SetUserOperation error: Invalid parameter, pkgName: %s", pkgName.c_str(),
            params.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (!IsDMServiceImplReady()) {
        LOGE("SetUserOperation failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->SetUserOperation(pkgName, action, params);
}

int32_t DeviceManagerService::RegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerService::RegisterDevStateCallback error: Invalid parameter, pkgName: %s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    {
        std::lock_guard<std::mutex> lock(registerDevStateLock_);
        if (registerDevStateMap_.count(pkgName) == 0) {
            registerDevStateMap_.insert(std::map<std::string, std::string>::value_type (pkgName, extra));
        }
    }
    return DM_OK;
}

int32_t DeviceManagerService::UnRegisterDevStateCallback(const std::string &pkgName, const std::string &extra)
{
    if (pkgName.empty()) {
        LOGE("DeviceManagerService::UnRegisterDevStateCallback error: Invalid parameter, pkgName: %s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    {
        std::lock_guard<std::mutex> lock(registerDevStateLock_);
        if (registerDevStateMap_.count(pkgName) > 0) {
            registerDevStateMap_.erase(pkgName);
        }
    }
    if (IsDMServiceImplSoLoaded()) {
        return dmServiceImpl_->UnRegisterDevStateCallback(pkgName, extra);
    }
    return DM_OK;
}

void DeviceManagerService::HandleDeviceOnline(const DmDeviceInfo &info)
{
    if (!IsDMServiceImplReady()) {
        LOGE("HandleDeviceOnline failed, instance not init or init failed.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(registerDevStateLock_);
        for (auto iter : registerDevStateMap_) {
            dmServiceImpl_->RegisterDevStateCallback(iter.first, iter.second);
        }
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

int32_t DeviceManagerService::RequestCredential(const std::string &reqJsonStr, std::string &returnJsonStr)
{
    if (!IsDMServiceImplReady()) {
        LOGE("RequestCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RequestCredential(reqJsonStr, returnJsonStr);
}

int32_t DeviceManagerService::ImportCredential(const std::string &pkgName, const std::string &credentialInfo)
{
    if (!IsDMServiceImplReady()) {
        LOGE("ImportCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->ImportCredential(pkgName, credentialInfo);
}

int32_t DeviceManagerService::DeleteCredential(const std::string &pkgName, const std::string &deleteInfo)
{
    if (!IsDMServiceImplReady()) {
        LOGE("DeleteCredential failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->DeleteCredential(pkgName, deleteInfo);
}

int32_t DeviceManagerService::RegisterCredentialCallback(const std::string &pkgName)
{
    if (!IsDMServiceImplReady()) {
        LOGE("RegisterCredentialCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->RegisterCredentialCallback(pkgName);
}

int32_t DeviceManagerService::UnRegisterCredentialCallback(const std::string &pkgName)
{
    if (!IsDMServiceImplReady()) {
        LOGE("UnRegisterCredentialCallback failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->UnRegisterCredentialCallback(pkgName);
}

bool DeviceManagerService::IsDMServiceImplReady()
{
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
    if (isImplsoLoaded_ && (dmServiceImpl_ != nullptr)) {
        return true;
    }

    std::string soName = std::string(LIB_LOAD_PATH) + std::string(LIB_IMPL_NAME);
    void *so_handle = dlopen(soName.c_str(), RTLD_NOW | RTLD_NOLOAD);
    if (so_handle == nullptr) {
        so_handle = dlopen(soName.c_str(), RTLD_NOW);
        if (so_handle == nullptr) {
            LOGE("load libdevicemanagerserviceimpl so %s failed.", soName.c_str());
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

bool DeviceManagerService::IsDMServiceImplSoLoaded()
{
    std::lock_guard<std::mutex> lock(isImplLoadLock_);
    return isImplsoLoaded_;
}

int32_t DeviceManagerService::DmHiDumper(const std::vector<std::string>& args, std::string &result)
{
    LOGI("HiDump GetTrustedDeviceList");
    std::vector<HidumperFlag> dumpflag;
    HiDumpHelper::GetInstance().GetArgsType(args, dumpflag);

    for (unsigned int i = 0; i < dumpflag.size(); i++) {
        if (dumpflag[i] == HidumperFlag::HIDUMPER_GET_TRUSTED_LIST) {
            std::vector<DmDeviceInfo> deviceList;

            int32_t ret = softbusListener_->GetTrustedDeviceList(deviceList);
            if (ret != DM_OK) {
                result.append("HiDumpHelper GetTrustedDeviceList failed");
                LOGE("HiDumpHelper GetTrustedDeviceList failed");
                return ERR_DM_FAILED;
            }

            for (unsigned int j = 0; j < deviceList.size(); j++) {
                HiDumpHelper::GetInstance().SetNodeInfo(deviceList[j]);
                LOGI("DeviceManagerService::DmHiDumper SetNodeInfo.");
            }
        }
    }
    HiDumpHelper::GetInstance().HiDump(args, result);
    return DM_OK;
}

int32_t DeviceManagerService::NotifyEvent(const std::string &pkgName, const int32_t eventId, const std::string &event)
{
    if (!IsDMServiceImplReady()) {
        LOGE("NotifyEvent failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->NotifyEvent(pkgName, eventId, event);
}

void DeviceManagerService::LoadHardwareFwkService()
{
    std::string extra;
    std::vector<DmDeviceInfo> deviceList;
    int32_t ret = GetTrustedDeviceList(DM_PKG_NAME, extra, deviceList);
    if (ret != DM_OK) {
        LOGE("LoadHardwareFwkService failed, get trusted devicelist failed.");
        return;
    }
    if (deviceList.size() > 0) {
        dmServiceImpl_->LoadHardwareFwkService();
    }
}

int32_t DeviceManagerService::GetEncryptedUuidByNetworkId(const std::string &pkgName, const std::string &networkId,
    std::string &uuid)
{
    if (!IsDMServiceImplReady()) {
        LOGE("GetEncryptedUuidByNetworkId failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->GetEncryptedUuidByNetworkId(pkgName, networkId, uuid);
}

int32_t DeviceManagerService::GenerateEncryptedUuid(const std::string &pkgName, const std::string &uuid,
    const std::string &appId, std::string &encryptedUuid)
{
    if (!IsDMServiceImplReady()) {
        LOGE("GenerateEncryptedUuid failed, instance not init or init failed.");
        return ERR_DM_NOT_INIT;
    }
    return dmServiceImpl_->GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
}
} // namespace DistributedHardware
} // namespace OHOS
