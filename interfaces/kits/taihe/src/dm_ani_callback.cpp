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

#include "device_manager.h"
#include "dm_ani_callback.h"
#include "dm_log.h"

#define DH_LOG_TAG "DeviceManager"

void DmAniInitCallback::OnRemoteDied()
{
    LOGI("ohos.distributedDeviceManager.cpp DmAniInitCallback::OnRemoteDied called.");

    auto &deviceManager = static_cast<OHOS::DistributedHardware::DeviceManager &>(
        OHOS::DistributedHardware::DeviceManager::GetInstance());
    deviceManager.UnInitDeviceManager(bundleName_);
}

void DmAniDiscoveryFailedCallback::IncreaseRefCount()
{
    refCount_++;
}

void DmAniDiscoveryFailedCallback::DecreaseRefCount()
{
    refCount_--;
}

int32_t DmAniDiscoveryFailedCallback::GetRefCount()
{
    return refCount_;
}

void DmAniDiscoverySuccessCallback::IncreaseRefCount()
{
    refCount_++;
}

void DmAniDiscoverySuccessCallback::DecreaseRefCount()
{
    refCount_--;
}

int32_t DmAniDiscoverySuccessCallback::GetRefCount()
{
    return refCount_;
}
