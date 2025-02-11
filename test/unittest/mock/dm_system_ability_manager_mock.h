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

#ifndef OHOS_DM_SYSTEM_ABILITY_MANAGER_H
#define OHOS_DM_SYSTEM_ABILITY_MANAGER_H

#include <gmock/gmock.h>

#include "if_system_ability_manager.h"

namespace OHOS {
namespace DistributedHardware {
class SystemAbilityManagerMock : public ISystemAbilityManager {
public:
    MOCK_METHOD0(AsObject, sptr<IRemoteObject>());
    MOCK_METHOD1(ListSystemAbilities, std::vector<std::u16string>(unsigned int dumpFlags));
    MOCK_METHOD1(GetSystemAbility, sptr<IRemoteObject>(int32_t systemAbilityId));
    MOCK_METHOD1(CheckSystemAbility, sptr<IRemoteObject>(int32_t systemAbilityId));
    MOCK_METHOD1(RemoveSystemAbility, int32_t(int32_t systemAbilityId));
    MOCK_METHOD2(SubscribeSystemAbility, int32_t(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener));
    MOCK_METHOD2(UnSubscribeSystemAbility, int32_t(int32_t systemAbilityId,
        const sptr<ISystemAbilityStatusChange>& listener));
    MOCK_METHOD2(GetSystemAbility, sptr<IRemoteObject>(int32_t systemAbilityId, const std::string& deviceId));
    MOCK_METHOD2(CheckSystemAbility, sptr<IRemoteObject>(int32_t systemAbilityId, const std::string& deviceId));
    MOCK_METHOD2(AddOnDemandSystemAbilityInfo, int32_t(int32_t systemAbilityId,
        const std::u16string& localAbilityManagerName));
    MOCK_METHOD2(CheckSystemAbility, sptr<IRemoteObject>(int32_t systemAbilityId, bool& isExist));
    MOCK_METHOD3(AddSystemAbility, int32_t(int32_t systemAbilityId, const sptr<IRemoteObject>& ability,
        const SAExtraProp& extraProp));
    MOCK_METHOD2(AddSystemProcess, int32_t(const std::u16string& procName, const sptr<IRemoteObject>& procObject));
    MOCK_METHOD2(LoadSystemAbility, sptr<IRemoteObject>(int32_t systemAbilityId, int32_t timeout));
    MOCK_METHOD2(LoadSystemAbility, int32_t(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback));
    MOCK_METHOD3(LoadSystemAbility, int32_t(int32_t systemAbilityId, const std::string& deviceId,
        const sptr<ISystemAbilityLoadCallback>& callback));
    MOCK_METHOD1(UnloadSystemAbility, int32_t(int32_t systemAbilityId));
    MOCK_METHOD1(CancelUnloadSystemAbility, int32_t(int32_t systemAbilityId));
    MOCK_METHOD0(UnloadAllIdleSystemAbility, int32_t());
    MOCK_METHOD2(GetSystemProcessInfo, int32_t(int32_t systemAbilityId, SystemProcessInfo& systemProcessInfo));
    MOCK_METHOD1(GetRunningSystemProcess, int32_t(std::list<SystemProcessInfo>& systemProcessInfos));
    MOCK_METHOD1(SubscribeSystemProcess, int32_t(const sptr<ISystemProcessStatusChange>& listener));
    MOCK_METHOD4(SendStrategy, int32_t(int32_t type, std::vector<int32_t>& systemAbilityIds,
        int32_t level, std::string& action));
    MOCK_METHOD1(UnSubscribeSystemProcess, int32_t(const sptr<ISystemProcessStatusChange>& listener));
    MOCK_METHOD2(GetOnDemandReasonExtraData, int32_t(int64_t extraDataId, MessageParcel& extraDataParcel));
    MOCK_METHOD3(GetOnDemandPolicy, int32_t(int32_t systemAbilityId, OnDemandPolicyType type,
        std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents));
    MOCK_METHOD3(UpdateOnDemandPolicy, int32_t(int32_t systemAbilityId, OnDemandPolicyType type,
        const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents));
    MOCK_METHOD1(GetOnDemandSystemAbilityIds, int32_t(std::vector<int32_t>& systemAbilityIds));
    MOCK_METHOD2(GetExtensionSaIds, int32_t(const std::string& extension, std::vector<int32_t> &saIds));
    MOCK_METHOD2(GetExtensionRunningSaList, int32_t(const std::string& extension,
        std::vector<sptr<IRemoteObject>>& saList));
    MOCK_METHOD2(GetRunningSaExtensionInfoList, int32_t(const std::string& extension,
        std::vector<SaExtensionInfo>& infoList));
    MOCK_METHOD3(GetCommonEventExtraDataIdlist, int32_t(int32_t systemAbilityId,
        std::vector<int64_t>& extraDataIdList, const std::string& eventName));
};
}
} // namespace OHOS

#endif // OHOS_DM_SYSTEM_ABILITY_MANAGER_H
