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

#ifndef OHOS_DEFAULT_DM_SERVICE_IMPL_EXT_H
#define OHOS_DEFAULT_DM_SERVICE_IMPL_EXT_H

#include "i_dm_service_impl_ext.h"

namespace OHOS {
namespace DistributedHardware {
class DefaultDMServiceImplExt : public IDMServiceImplExt {
public:
    DefaultDMServiceImplExt();
    virtual ~DefaultDMServiceImplExt();

    int32_t Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener) override;

    int32_t Release() override;

    int32_t StartDiscoveringExt(const std::string &pkgName, const std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions) override;

    int32_t StopDiscoveringExt(const std::string &pkgName,
        const std::map<std::string, std::string> &discoverParam) override;

    int32_t EnableDiscoveryListenerExt(const std::string &pkgName,
        const std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions) override;

    int32_t DisableDiscoveryListenerExt(const std::string &pkgName,
        const std::map<std::string, std::string> &extraParam) override;

    int32_t StartAdvertisingExt(const std::string &pkgName,
        const std::map<std::string, std::string> &advertiseParam) override;

    int32_t StopAdvertisingExt(const std::string &pkgName,
        const std::map<std::string, std::string> &advertiseParam) override;

    int32_t BindTargetExt(const std::string &pkgName, const std::string &targetId,
        const std::map<std::string, std::string> &bindParam) override;

    int32_t UnbindTargetExt(const std::string &pkgName, const std::string &targetId,
        const std::map<std::string, std::string> &unbindParam) override;

    int32_t HandleDeviceStatusEvent(int32_t eventCode, const DmDeviceInfo &devInfo) override;
};

using CreateDMServiceImplExtFuncPtr = IDMServiceImplExt *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEFAULT_DM_SERVICE_IMPL_EXT_H