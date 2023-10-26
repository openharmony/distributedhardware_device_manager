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

#ifndef OHOS_I_DM_SERVICE_IMPL_EXT_H
#define OHOS_I_DM_SERVICE_IMPL_EXT_H

#include <map>
#include <string>
#include <vector>

#include "idevice_manager_service_listener.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
class IDMServiceImplExt {
public:
    virtual ~IDMServiceImplExt() {}

    /**
     * @tc.name: IDMServiceImplExt::Initialize
     * @tc.desc: Initialize the device manager service impl ext
     * @tc.type: FUNC
     */
    virtual int32_t Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener) = 0;

    /**
     * @tc.name: IDMServiceImplExt::Release
     * @tc.desc: Release the device manager service impl ext
     * @tc.type: FUNC
     */
    virtual int32_t Release() = 0;

    /**
     * @tc.name: IDMServiceImplExt::StartDiscoveringExt
     * @tc.desc: StartDiscoveringExt
     * @tc.type: FUNC
     */
    virtual int32_t StartDiscoveringExt(const std::string &pkgName,
        const std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions) = 0;

    /**
     * @tc.name: IDMServiceImplExt::StopDiscoveringExt
     * @tc.desc: StopDiscoveringExt
     * @tc.type: FUNC
     */
    virtual int32_t StopDiscoveringExt(const std::string &pkgName,
        const std::map<std::string, std::string> &discoverParam) = 0;

    /**
     * @tc.name: IDMServiceImplExt::EnableDiscoveryListenerExt
     * @tc.desc: EnableDiscoveryListenerExt
     * @tc.type: FUNC
     */
    virtual int32_t EnableDiscoveryListenerExt(const std::string &pkgName,
        const std::map<std::string, std::string> &discoverParam,
        const std::map<std::string, std::string> &filterOptions) = 0;

    /**
     * @tc.name: IDMServiceImplExt::DisableDiscoveryListenerExt
     * @tc.desc: DisableDiscoveryListenerExt
     * @tc.type: FUNC
     */
    virtual int32_t DisableDiscoveryListenerExt(const std::string &pkgName,
        const std::map<std::string, std::string> &extraParam) = 0;

    /**
     * @tc.name: IDMServiceImplExt::StartAdvertisingExt
     * @tc.desc: StartAdvertisingExt
     * @tc.type: FUNC
     */
    virtual int32_t StartAdvertisingExt(const std::string &pkgName,
        const std::map<std::string, std::string> &advertiseParam) = 0;

    /**
     * @tc.name: IDMServiceImplExt::StopAdvertisingExt
     * @tc.desc: StopAdvertisingExt
     * @tc.type: FUNC
     */
    virtual int32_t StopAdvertisingExt(const std::string &pkgName,
        const std::map<std::string, std::string> &advertiseParam) = 0;

    /**
     * @tc.name: IDMServiceImplExt::BindTargetExt
     * @tc.desc: BindTargetExt
     * @tc.type: FUNC
     */
    virtual int32_t BindTargetExt(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &bindParam) = 0;

    /**
     * @tc.name: IDMServiceImplExt::UnbindTargetExt
     * @tc.desc: UnbindTargetExt
     * @tc.type: FUNC
     */
    virtual int32_t UnbindTargetExt(const std::string &pkgName, const PeerTargetId &targetId,
        const std::map<std::string, std::string> &unbindParam) = 0;

    /**
     * @tc.name: IDMServiceImplExt::HandleDeviceStatusEvent
     * @tc.desc: HandleDeviceStatusEvent
     * @tc.type: FUNC
     */
    virtual int32_t HandleDeviceStatusEvent(int32_t eventCode, const DmDeviceInfo &devInfo) = 0;
};

using CreateDMServiceImplExtFuncPtr = IDMServiceImplExt *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_IMPL_EXT_H
