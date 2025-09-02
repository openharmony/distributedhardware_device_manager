/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_SERVICE_IMPL_EXT_RESIDENT_MOCK_H
#define OHOS_DM_SERVICE_IMPL_EXT_RESIDENT_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "i_dm_service_impl_ext_resident.h"

namespace OHOS {
namespace DistributedHardware {
class DMServiceImplExtResidentMock : public IDMServiceImplExtResident {
public:
    MOCK_METHOD(int32_t, Initialize, (const std::shared_ptr<IDeviceManagerServiceListener> &));
    MOCK_METHOD(int32_t, Release, ());
    MOCK_METHOD(bool, IsDMServiceAdapterLoad, ());
    MOCK_METHOD(bool, IsDMServiceAdapterSoLoaded, ());
    MOCK_METHOD(int32_t, BindTargetExt, (const std::string &, const PeerTargetId &,
        (const std::map<std::string, std::string> &)));
    MOCK_METHOD(int32_t, UnbindTargetExt, (const std::string &, const PeerTargetId &,
        (const std::map<std::string, std::string> &)));
    MOCK_METHOD(int32_t, HandleDeviceStatusChange, (DmDeviceState, const DmDeviceInfo &));
    MOCK_METHOD(int32_t, ReplyUiAction, (const std::string &, int32_t, const std::string &));
    MOCK_METHOD(int32_t, AccountIdLogout, (int32_t, const std::string &, (const std::vector<std::string> &)));
    MOCK_METHOD(void, HandleDeviceNotTrust, (const std::string &));
    MOCK_METHOD(int32_t, SetDnPolicy, (int32_t, int32_t));
    MOCK_METHOD(int32_t, AccountUserSwitched, (int32_t, const std::string &));
    MOCK_METHOD(int32_t, GetDeviceProfileInfoList, (const std::string &, const DmDeviceProfileInfoFilterOptions &));
    MOCK_METHOD(int32_t, GetDeviceIconInfo, (const std::string &, const DmDeviceIconInfoFilterOptions &));
    MOCK_METHOD(int32_t, PutDeviceProfileInfoList, (const std::string &,
        (const std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> &)));
    MOCK_METHOD((std::vector<std::string>), GetDeviceNamePrefixs, ());
    MOCK_METHOD(void, HandleNetworkConnected, (int32_t));
    MOCK_METHOD(int32_t, SetLocalDeviceName, (const std::string &, const std::string &));
    MOCK_METHOD(int32_t, SetRemoteDeviceName, (const std::string &, const std::string &, const std::string &));
    MOCK_METHOD(int32_t, GetDeviceProfileInfosFromLocalCache, (const NetworkIdQueryFilter &,
        (std::vector<DmDeviceProfileInfo> &)));
    MOCK_METHOD(int32_t, RestoreLocalDeviceName, ());
    MOCK_METHOD(void, ClearCacheWhenLogout, (int32_t, const std::string &));
    MOCK_METHOD(void, HandleScreenLockEvent, (bool));
    MOCK_METHOD(int32_t, OpenAuthSessionWithPara, (const std::string &, int32_t, bool));
    MOCK_METHOD(void, HandleUserSwitchEvent, (int32_t, int32_t));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SERVICE_IMPL_EXT_RESIDENT_MOCK_H
