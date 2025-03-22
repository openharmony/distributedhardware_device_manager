/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ipc_client_manager_mini.h"

#include <set>
#include <unistd.h>

#include "device_manager_ipc_interface_code.h"
#include "device_manager_impl_mini.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "dm_service_load.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {

void DmDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    LOGW("DmDeathRecipient : OnRemoteDied");
    (void)remote;
}

int32_t IpcClientManagerMini::ClientInit()
{
    LOGI("Start");
    if (dmInterface_ != nullptr) {
        LOGI("DeviceManagerService Already Init");
        return DM_OK;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Get SystemAbilityManager Failed");
        return ERR_DM_INIT_FAILED;
    }

    auto object = samgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (object == nullptr) {
        LOGE("Get DeviceManager SystemAbility Failed");
        DmServiceLoad::GetInstance().LoadDMService();
        return ERR_DM_INIT_FAILED;
    }

    if (dmRecipient_ == nullptr) {
        dmRecipient_ = sptr<DmDeathRecipient>(new DmDeathRecipient());
    }
    if (!object->AddDeathRecipient(dmRecipient_)) {
        LOGE("InitDeviceManagerService: AddDeathRecipient Failed");
    }
    dmInterface_ = iface_cast<IpcRemoteBroker>(object);
    LOGI("Completed");
    return DM_OK;
}

int32_t IpcClientManagerMini::Init(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    SubscribeDMSAChangeListener(pkgName);
    int32_t ret = ClientInit();
    if (ret != DM_OK) {
        LOGE("InitDeviceManager Failed with ret %{public}d", ret);
        return ret;
    }
    return DM_OK;
}

int32_t IpcClientManagerMini::UnInit(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("UnInit in, pkgName %{public}s", pkgName.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    if (dmInterface_ == nullptr) {
        LOGE("DeviceManager not Init");
        return ERR_DM_INIT_FAILED;
    }
    LOGI("completed, pkgName: %{public}s", pkgName.c_str());
    return DM_OK;
}

int32_t IpcClientManagerMini::SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    if (cmdCode < 0 || cmdCode >= IPC_MSG_BUTT) {
        LOGE("IpcClientManagerMini::SendRequest cmdCode param invalid!");
        return ERR_DM_UNSUPPORTED_IPC_COMMAND;
    }
    if (req == nullptr || rsp == nullptr) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::lock_guard<std::mutex> autoLock(lock_);
    if (dmInterface_ != nullptr) {
        LOGD("IpcClientManagerMini::SendRequest cmdCode: %{public}d", cmdCode);
        return dmInterface_->SendCmd(cmdCode, req, rsp);
    } else {
        LOGE("dmInterface_ is not init.");
        return ERR_DM_INIT_FAILED;
    }
}

int32_t IpcClientManagerMini::OnDmServiceDied()
{
    LOGI("IpcClientManagerMini::OnDmServiceDied begin");
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (dmInterface_ == nullptr) {
            LOGE("IpcClientManagerMini::OnDmServiceDied, dmInterface_ null");
            return ERR_DM_POINT_NULL;
        }
        if (dmRecipient_ != nullptr) {
            if (dmInterface_->AsObject() == nullptr) {
                LOGE("IpcClientManagerMini::OnDmServiceDied, dmInterface_->AsObject() null");
                return ERR_DM_POINT_NULL;
            }
            dmInterface_->AsObject()->RemoveDeathRecipient(dmRecipient_);
            dmRecipient_ = nullptr;
        }
        dmInterface_ = nullptr;
    }
    LOGI("IpcClientManagerMini::OnDmServiceDied complete");
    return DM_OK;
}

void IpcClientManagerMini::SubscribeDMSAChangeListener(const std::string &pkgName)
{
    saListenerCallback = new (std::nothrow) SystemAbilityListener();
    if (saListenerCallback == nullptr) {
        LOGE("saListenerCallback is nullptr.");
        return;
    }
    saListenerCallback->InitPkgName(pkgName);
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();

    if (systemAbilityManager == nullptr) {
        LOGE("get system ability manager failed.");
        return;
    }

    if (!isSubscribeDMSAChangeListener.load()) {
        LOGI("try subscribe source sa change listener, sa id: %{public}d", DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        int32_t ret = systemAbilityManager->SubscribeSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID,
            saListenerCallback);
        if (ret != DM_OK) {
            LOGE("subscribe source sa change failed: %{public}d", ret);
            return;
        }
        isSubscribeDMSAChangeListener.store(true);
    }
}

void IpcClientManagerMini::SystemAbilityListener::OnRemoveSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    if (systemAbilityId == DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID) {
        DeviceManagerImplMini::GetInstance().OnDmServiceDied();
    }
    LOGI("sa %{public}d is removed.", systemAbilityId);
}

void IpcClientManagerMini::SystemAbilityListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    LOGI("sa %{public}d is added.", systemAbilityId);
    DeviceManagerImplMini::GetInstance().InitDeviceManager(pkgName_);
}
} // namespace DistributedHardware
} // namespace OHOS
