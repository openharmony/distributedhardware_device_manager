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

#include "default_dm_service_impl_ext.h"

#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
DefaultDMServiceImplExt::DefaultDMServiceImplExt()
{
    LOGD("DefaultDMServiceImplExt constructor");
}

DefaultDMServiceImplExt::~DefaultDMServiceImplExt()
{
    LOGD("DefaultDMServiceImplExt destructor");
}

int32_t DefaultDMServiceImplExt::Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener)
{
    (void)listener;
    return ERR_DM_UNSUPPORTED_METHOD;
}

int32_t DefaultDMServiceImplExt::Release()
{
    return ERR_DM_UNSUPPORTED_METHOD;
}

int32_t DefaultDMServiceImplExt::StartDiscoveringExt(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    (void)pkgName;
    (void)discoverParam;
    (void)filterOptions;
    return ERR_DM_UNSUPPORTED_METHOD;
}

int32_t DefaultDMServiceImplExt::StopDiscoveringExt(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam)
{
    (void)pkgName;
    (void)discoverParam;
    return ERR_DM_UNSUPPORTED_METHOD;
}

int32_t DefaultDMServiceImplExt::EnableDiscoveryListenerExt(const std::string &pkgName,
    const std::map<std::string, std::string> &discoverParam, const std::map<std::string, std::string> &filterOptions)
{
    (void)pkgName;
    (void)discoverParam;
    (void)filterOptions;
    return ERR_DM_UNSUPPORTED_METHOD;
}

int32_t DefaultDMServiceImplExt::DisableDiscoveryListenerExt(const std::string &pkgName,
    const std::map<std::string, std::string> &extraParam)
{
    (void)pkgName;
    (void)extraParam;
    return ERR_DM_UNSUPPORTED_METHOD;
}

int32_t DefaultDMServiceImplExt::StartAdvertisingExt(const std::string &pkgName,
    const std::map<std::string, std::string> &advertiseParam)
{
    (void)pkgName;
    (void)advertiseParam;
    return ERR_DM_UNSUPPORTED_METHOD;
}

int32_t DefaultDMServiceImplExt::StopAdvertisingExt(const std::string &pkgName,
    const std::map<std::string, std::string> &advertiseParam)
{
    (void)pkgName;
    (void)advertiseParam;
    return ERR_DM_UNSUPPORTED_METHOD;
}
} // namespace DistributedHardware
} // namespace OHOS