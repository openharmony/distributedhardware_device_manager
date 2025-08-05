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

#include "ipc_client_manager.h"

#include "device_manager_ipc_interface_code.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_client_stub.h"
#include "ipc_register_listener_req.h"

namespace OHOS {
namespace DistributedHardware {
int32_t IpcClientManager::Init(const std::string &pkgName)
{
    if (IsInit(pkgName)) {
        LOGI("already init");
        return DM_OK;
    }
    if (serverProxy_.Init() != DM_OK) {
        LOGE("server proxy init failed.");
        return ERR_DM_INIT_FAILED;
    }
    if (IpcClientStub::GetInstance().Init() != DM_OK) {
        LOGE("ipcclientstub init failed.");
        return ERR_DM_INIT_FAILED;
    }
    CHECK_SIZE_RETURN(packageInitSet_, ERR_DM_FAILED);
    std::shared_ptr<IpcRegisterListenerReq> req = std::make_shared<IpcRegisterListenerReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    req->SetSvcIdentity(IpcClientStub::GetInstance().GetSvcIdentity());
    int32_t ret = serverProxy_.SendCmd(REGISTER_DEVICE_MANAGER_LISTENER, req, rsp);
    if (ret != DM_OK) {
        LOGE("InitDeviceManager: RegisterDeviceManagerListener Failed with ret %{public}d", ret);
        return ret;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("DeviceManager::InitDeviceManager completed, pkgName: %{public}s, ret = %{public}d", pkgName.c_str(),
            ret);
        return ret;
    }
    packageInitSet_.emplace(pkgName);
    return DM_OK;
}

int32_t IpcClientManager::UnInit(const std::string &pkgName)
{
    LOGI("UnInitDeviceManager in, pkgName %{public}s", pkgName.c_str());
    if (!IsInit(pkgName)) {
        return ERR_DM_FAILED;
    }
    std::shared_ptr<IpcReq> req = std::make_shared<IpcReq>();
    std::shared_ptr<IpcRsp> rsp = std::make_shared<IpcRsp>();
    req->SetPkgName(pkgName);
    int32_t ret = serverProxy_.SendCmd(UNREGISTER_DEVICE_MANAGER_LISTENER, req, rsp);
    if (ret != DM_OK) {
        LOGE("UnRegisterDeviceManagerListener Failed with ret %{public}d", ret);
        return ret;
    }
    ret = rsp->GetErrCode();
    if (ret != DM_OK) {
        LOGE("DeviceManager::UnInitDeviceManager completed, pkgName: %{public}s, ret = %{public}d", pkgName.c_str(),
            ret);
        return ret;
    }
    packageInitSet_.erase(pkgName);
    LOGI("UnInitDeviceManager SUCCESS");
    return DM_OK;
}

int32_t IpcClientManager::SendRequest(int32_t cmdCode, std::shared_ptr<IpcReq> req, std::shared_ptr<IpcRsp> rsp)
{
    CHECK_NULL_RETURN(req, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(rsp, ERR_DM_POINT_NULL);
    std::string pkgName = req->GetPkgName();
    if (!IsInit(pkgName)) {
        LOGE("PkgName: %s is not init.", pkgName.c_str());
        return ERR_DM_INIT_FAILED;
    }
    return serverProxy_.SendCmd(cmdCode, req, rsp);
}

bool IpcClientManager::IsInit(const std::string &pkgName)
{
    for (auto &iter : packageInitSet_) {
        size_t len = iter.size();
        if (len > pkgName.size()) {
            continue;
        }
        std::string tmp = pkgName.substr(0, len);
        if (tmp == iter) {
            return true;
        }
    }
    return false;
}

int32_t IpcClientManager::OnDmServiceDied()
{
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
