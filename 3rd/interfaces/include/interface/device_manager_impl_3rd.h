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

#ifndef OHOS_DEVICE_MANAGER_IMPL_3RD_H
#define OHOS_DEVICE_MANAGER_IMPL_3RD_H

#include <memory>
#include <mutex>
#include <atomic>

#include "system_ability_status_change_stub.h"
#include "ipc_types.h"

#include "device_manager_callback_3rd.h"
#include "ipc_interface_code_3rd.h"
#include "ipc_client_stub_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "dm_log_3rd.h"
#include "dm_service_load_3rd.h"
#include "device_manager_notify_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_info_3rd.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceManagerImpl3rd {
public:
    static DeviceManagerImpl3rd &GetInstance();

public:
    class SystemAbilityListener3rd : public SystemAbilityStatusChangeStub {
    public:
        SystemAbilityListener3rd();
        virtual ~SystemAbilityListener3rd();
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    };

public:
    int32_t RegisterAuthCallback(const std::string &businessName, std::shared_ptr<DmAuthCallback> dmAuthCallback);
    int32_t UnRegisterAuthCallback(const std::string &businessName);

    int32_t InitDeviceManager(const std::string &businessName, std::shared_ptr<DmInit3rdCallback> DmInit3rdCallback);
    int32_t UnInitDeviceManager(const std::string &businessName);

    int32_t ImportPinCode3rd(const std::string &businessName, const std::string &pinCode);
    int32_t GeneratePinCode(uint32_t pinLength, std::string &pincode);
    int32_t AuthPincode(const PeerTargetId3rd &targetId, std::map<std::string, std::string> &authParam);
    int32_t AuthDevice3rd(const PeerTargetId3rd &targetId, std::map<std::string, std::string> &authParam);
    int32_t QueryTrustRelation(const std::string &businessName, std::vector<TrustDeviceInfo3rd> &trustedDeviceList);
    int32_t DeleteTrustRelation(const std::string &businessName, const std::string &peerDeviceId,
        std::map<std::string, std::string> &unbindParam);
private:
    void SubscribeDMSAChangeListener();
    int32_t ClientInit();
    bool IsInvalidPeerTargetId(const PeerTargetId3rd &targetId);
    int32_t SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply);
    int32_t BuildSessionKey(const std::string &keyStr, TrustDeviceInfo3rd &deviceInfo);

private:
    sptr<IRemoteObject> dm3rdremoteObject_ = nullptr;
    sptr<SystemAbilityListener3rd> saListenerCallback_ = nullptr;
    std::atomic<bool> isSubscribeDMSAChangeListener_  = false;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DEVICE_MANAGER_IMPL_3RD_H