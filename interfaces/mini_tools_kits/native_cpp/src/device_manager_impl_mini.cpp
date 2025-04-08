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

#include "device_manager_impl_mini.h"

#include "device_manager_ipc_interface_code.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "ipc_get_local_display_device_name_req.h"
#include "ipc_get_local_display_device_name_rsp.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t SERVICE_INIT_MAX_NUM = 20;
const int32_t USLEEP_TIME_US_100000 = 100000; // 100ms
}
DeviceManagerImplMini &DeviceManagerImplMini::GetInstance()
{
    static DeviceManagerImplMini instance;
    return instance;
}

int32_t DeviceManagerImplMini::InitDeviceManager(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("InitDeviceManager error: Invalid parameter, pkgName: %{public}s", pkgName.c_str());
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    int32_t ret = DM_OK;
    int32_t retryNum = 0;
    while (retryNum < SERVICE_INIT_MAX_NUM) {
        ret = ipcClientProxy_->Init(pkgName);
        if (ret == DM_OK) {
            break;
        }
        usleep(USLEEP_TIME_US_100000);
        retryNum++;
        if (retryNum == SERVICE_INIT_MAX_NUM) {
            LOGE("InitDeviceManager error, wait for device manager service starting timeout.");
            return ERR_DM_NOT_INIT;
        }
    }
    if (ret != DM_OK) {
        LOGE("InitDeviceManager error, proxy init failed ret: %{public}d", ret);
        return ERR_DM_INIT_FAILED;
    }
    LOGI("Success");
    return DM_OK;
}

int32_t DeviceManagerImplMini::UnInitDeviceManager(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("UnInitDeviceManager Invalid parameter, pkgName is empty.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("Start, pkgName: %{public}s", pkgName.c_str());
    int32_t ret = ipcClientProxy_->UnInit(pkgName);
    if (ret != DM_OK) {
        LOGE("UnInitDeviceManager error, proxy unInit failed ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    LOGI("Success");
    return DM_OK;
}

int32_t DeviceManagerImplMini::OnDmServiceDied()
{
    LOGI("Start");
    int32_t ret = ipcClientProxy_->OnDmServiceDied();
    if (ret != DM_OK) {
        LOGE("OnDmServiceDied failed, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DeviceManagerImplMini::GetLocalDisplayDeviceName(const std::string &pkgName, int32_t maxNameLength,
    std::string &displayName)
{
    LOGI("In pkgName:%{public}s,", pkgName.c_str());
    std::shared_ptr<IpcGetLocalDisplayDeviceNameReq> req = std::make_shared<IpcGetLocalDisplayDeviceNameReq>();
    std::shared_ptr<IpcGetLocalDisplayDeviceNameRsp> rsp = std::make_shared<IpcGetLocalDisplayDeviceNameRsp>();
    req->SetPkgName(pkgName);
    req->SetMaxNameLength(maxNameLength);
    int32_t ret = ipcClientProxy_->SendRequest(GET_LOCAL_DISPLAY_DEVICE_NAME, req, rsp);
    if (ret != DM_OK) {
        LOGE("error: Send Request failed ret: %{public}d", ret);
        return ERR_DM_IPC_SEND_REQUEST_FAILED;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("error: Failed with ret %{public}d", ret);
        return ret;
    }
    displayName = rsp->GetDisplayName();
    LOGI("Completed");
    return DM_OK;
}

} // namespace DistributedHardware
} // namespace OHOS
