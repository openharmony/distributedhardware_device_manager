/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_SERVICE_3RD_H
#define OHOS_DM_SERVICE_3RD_H

#include <map>
#include <memory>
#include <string>

#include "ffrt.h"
#include "dm_auth_info_3rd.h"
#include "dm_single_instance_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "device_manager_service_listener_3rd.h"
#include "idevice_manager_service_impl_3rd.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerService3rd {
DM_DECLARE_SINGLE_INSTANCE_3RD_BASE_3RD(DeviceManagerService3rd);
public:
    DeviceManagerService3rd();

    ~DeviceManagerService3rd();

    int32_t InitDeviceManager();
    int32_t ImportPinCode3rd(const std::string &businessName, const std::string &pinCode);
    int32_t GeneratePinCode(uint32_t pinLength, std::string &pincode);
    int32_t AuthPincode(const PeerTargetId3rd &targetId, std::map<std::string, std::string> &authParam);
    int32_t AuthDevice3rd(const PeerTargetId3rd &targetId, const std::map<std::string, std::string> &authParam);
    int32_t QueryTrustRelation(const std::string &businessName, std::vector<TrustDeviceInfo3rd> &trustedDeviceList);
    int32_t DeleteTrustRelation(const std::string &businessName, const std::string &peerDeviceId,
        const std::map<std::string, std::string> &unbindParam);
    int32_t HandleUserRemoved(int32_t removedUserId);
    int32_t HandleAccountLogoutEvent(int32_t userId, const std::string &accountId);
    int OnAuth3rdAclSessionOpened(int sessionId, int result);
    void OnAuth3rdAclSessionClosed(int sessionId);
    void OnAuth3rdAclBytesReceived(int sessionId, const void *data, unsigned int dataLen);

    int OnAuth3rdSessionOpened(int sessionId, int result);
    void OnAuth3rdSessionClosed(int sessionId);
    void OnAuth3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen);

private:
    bool IsDMServiceImpl3rdReady();
    int32_t GenRandInt(int32_t randMin, int32_t randMax);
    std::string GeneratePinCode(uint32_t pinLength);
    bool IsInvalidPeerTargetId(const PeerTargetId3rd &targetId);
    void QuerySessionKey(int32_t userId, int32_t skId, TrustDeviceInfo3rd &deviceInfo);
    int32_t GetProxyDelInfo(const std::string &peerDeviceId,
        const std::map<std::string, std::string> &unbindParam, std::vector<std::string> &delKeyVec);
private:
    void *dmServiceImpl3rdSoHandle_ = nullptr;
    bool isImpl3rdSoLoaded_ = false;
    ffrt::mutex isImpl3rdLoadLock_;
    std::shared_ptr<IDeviceManagerServiceImpl3rd> dmServiceImpl3rd_ = nullptr;
    std::shared_ptr<DeviceManagerServiceListener3rd> listener_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_3RD_H