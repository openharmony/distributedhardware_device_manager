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

#ifndef OHOS_CONSTRAINS_MANAGER_H
#define OHOS_CONSTRAINS_MANAGER_H

#include "dm_single_instance.h"
#include "os_account_constraint_subscriber.h"
#include "os_account_manager.h"

namespace OHOS {
namespace DistributedHardware {
struct DmOsAccountConstraintStateData {
    int32_t userId = -1;
    std::string constraint = "";
    bool operator==(const DmOsAccountConstraintStateData &other) const
    {
        return (userId == other.userId) && (constraint == other.constraint)
    }

    bool operator<(const DmOsAccountConstraintStateData &other) const
    {
        return (userId < other.userId) ||
            (userId == other.userId && constraint < other.constraint);
    }
};
class DmConstrainsManager;
class DmOsAccountConstraintSubscriber : public AccountSA::OsAccountConstraintSubscriber {
public:
    DmOsAccountConstraintSubscriber(DmConstrainsManager &manager, const std::set<std::string> &constraintSet)
        : OsAccountConstraintSubscriber(constraintSet), manager_(manager) {}
    ~DmOsAccountConstraintSubscriber() override = default;
    void OnConstraintChanged(const AccountSA::OsAccountConstraintStateData &constrainData) override;
private:
    DmConstrainsManager &manager_;
};

class DmConstrainsManager {
    DM_DECLARE_SINGLE_INSTANCE_BASE(DmConstrainsManager);
public:
    DmConstrainsManager() {}
    int32_t SubscribeOsAccountConstraints(const std::set<std::string> &constraintSet);
    bool CheckOsAccountConstraintEnabled(int32_t userId, const std::string &constraint);
    void AddConstraint(const AccountSA::OsAccountConstraintStateData &constrainData);
    void DeleteConstraint(int32_t userId);

private:
    std::mutex constraintSubscriberLock_;
    std::shared_ptr<DmOsAccountConstraintSubscriber> constraintSubscriber_;
    std::mutex dmConstrainDataLock_;
    std::map<DmOsAccountConstraintStateData, OsAccountConstraintStateData> dmConstrainDatas_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_CONSTRAINS_MANAGER_H