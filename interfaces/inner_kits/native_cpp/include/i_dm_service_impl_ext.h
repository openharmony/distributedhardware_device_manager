/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "dm_device_info.h"
#include "idevice_manager_service_listener.h"
#include "i_dm_bind_manager_ext_resident.h"

#include <memory>

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
     * @tc.name: IDMServiceImplExt::HandleDeviceStatusChange
     * @tc.desc: HandleDeviceStatusChange
     * @tc.type: FUNC
     */
    virtual int32_t HandleDeviceStatusChange(DmDeviceState devState, const DmDeviceInfo &devInfo) = 0;

    /**
     * @tc.name: IDMServiceImplExt::ReplyUiAction
     * @tc.desc: ReplyUiAction
     * @tc.type: FUNC
     */
    virtual int32_t ReplyUiAction(const std::string &pkgName, int32_t action, const std::string &result) = 0;

    /**
     * @tc.name: IDMServiceImplExt::AccountIdLogout
     * @tc.desc: AccountIdLogout
     * @tc.type: FUNC
     */
    virtual int32_t AccountIdLogout(int32_t userId, const std::string &oldAccountId,
        const std::vector<std::string> &peerUdids) = 0;
    /**
     * @tc.name: IDMServiceImplExt::HandleDeviceNotTrust
     * @tc.desc: HandleDeviceNotTrust
     * @tc.type: FUNC
     */
    virtual void HandleDeviceNotTrust(const std::string &udid) = 0;

    /**
     * @tc.name: IDMServiceImplExt::SetDnPolicy
     * @tc.desc: SetDnPolicy
     * @tc.type: FUNC
     */
    virtual int32_t SetDnPolicy(int32_t policy, int32_t timeOut) = 0;

    /**
     * @tc.name: IDMServiceImplExt::AccountUserSwitched
     * @tc.desc: AccountUserSwitched
     * @tc.type: FUNC
     */
    virtual int32_t AccountUserSwitched(int32_t userId, const std::string &accountId) = 0;
    virtual int32_t InitResident(std::shared_ptr<IDMBindManagerExtResident> residentCallback,
        const std::shared_ptr<IDeviceManagerServiceListener> &listener) = 0;
    virtual void OnResidentBytesReceived(const int32_t sessionId, const std::string &message) = 0;

    virtual void OnSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) = 0;
    virtual void OnSessionClosed(const int32_t sessionId) = 0;
    virtual void OnBytesReceived(const int32_t sessionId, const std::string message) = 0;
    virtual int32_t OpenAuthSessionWithPara(const std::string &deviceId, int32_t actionId, bool isEnable160m) = 0;
    virtual int32_t OpenAuthSessionWithPara(int64_t serviceId) = 0;
    virtual void OnDynamicSessionOpened(int32_t sessionId, int32_t sessionSide, int32_t result) = 0;
    virtual void OnDynamicSessionClosed(const int32_t sessionId) = 0;
};

using CreateDMServiceImplExtFuncPtr = IDMServiceImplExt *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_IMPL_EXT_H
