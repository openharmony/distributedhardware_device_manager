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

#include "dm_client.h"

#include "app_manager.h"
#include "device_manager.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "oh_device_manager_err_code.h"

namespace OHOS {
namespace DistributedHardware {
DmClient &DmClient::GetInstance()
{
    static DmClient instance;
    return instance;
}

int32_t DmClient::Init()
{
    std::lock_guard<std::mutex> lck(initMtx_);
    if (pkgName_.empty()) {
        std::string bundleName = "";
        int32_t ret = AppManager::GetInstance().GetBundleNameForSelf(bundleName);
        if (ret != DM_OK || bundleName.empty()) {
            LOGE("Get bundle name failed, ret=%{public}d", ret);
            return DM_ERR_OBTAIN_BUNDLE_NAME;
        }
        pkgName_ = bundleName;
    }
    if (dmInitCallback_ == nullptr) {
        dmInitCallback_ = std::make_shared<DmClient::InitCallback>();
    }
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(pkgName_, dmInitCallback_);
    if (ret != DM_OK) {
        LOGE("Init DeviceManager failed, ret=%{public}d", ret);
        return DM_ERR_OBTAIN_SERVICE;
    }
    return ERR_OK;
}

int32_t DmClient::UnInit()
{
    std::lock_guard<std::mutex> lck(initMtx_);
    if (dmInitCallback_ != nullptr) {
        DeviceManager::GetInstance().UnInitDeviceManager(pkgName_);
    }
    pkgName_ = "";
    return ERR_OK;
}

int32_t DmClient::ReInit()
{
    UnInit();
    return Init();
}

int32_t DmClient::GetLocalDeviceName(std::string &deviceName)
{
    int32_t ret = Init();
    if (ret != ERR_OK) {
        LOGE("Init dm client failed, ret=%{public}d", ret);
        return ret;
    }
    ret = DeviceManager::GetInstance().GetLocalDeviceName(deviceName);
    if (ret != DM_OK) {
        LOGE("Get local device name failed, ret=%{public}d", ret);
        return DM_ERR_FAILED;
    }
    return ERR_OK;
}

void DmClient::InitCallback::OnRemoteDied()
{
    DmClient::GetInstance().ReInit();
}
} // namespace DistributedHardware
} // namespace OHOS
