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

#include "constrains_manager.h"

#include "dm_constants.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "multiple_user_connector.h"
#include "ffrt.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t DEFAULT_USERID = -1;
DM_IMPLEMENT_SINGLE_INSTANCE(DmOsAccountConstraintSubscriber);
void DmOsAccountConstraintSubscriber::OnConstraintChanged(const AccountSA::OsAccountConstraintStateData &constrainData)
{
    LOGI("userId %{public}d, constraint %{public}s, isEnable %{public}d.",
        constrainData.localId, constrainData.constraint.c_str(), constrainData.isEnabled);
    manager_.AddConstraint(constrainData);
}

int32_t DmConstrainsManager::SubscribeOsAccountConstraints(const std::set<std::string> &constraintSet)
{
    LOGI("start.");
    if (constraintSet.empty()) {
        LOGE("input param constraintSet is empty.");
        return ERR_DM_FAILED;
    }
    {
        std::lock_guard<std::mutex> lock(constraintSubscriberLock_);
        constraintSubscriber_ = std::make_shared<DmOsAccountConstraintSubscriber>(*this, constraintSet);
    }
    ErrCode errcode = OHOS::AccountSA::SubscribeOsAccountConstraints(constraintSubscriber_);
    if (errcode != ERR_OK) {
        LOGE("subscribe constraints failed errcode %{public}d.", errcode);
        return ERR_DM_FAILED;
    }
    LOGI("success.");
    return DM_OK;
}

bool DmConstrainsManager::CheckOsAccountConstraintEnabled(int32_t userId,
    const std::string &constraint)
{
    {
        std::lock_guard<std::mutex> lock(dmConstrainDataLock_);
        for (const auto &item : dmConstrainDatas_) {
            if (item.second.localId == userId && item.second.constraint == constraint) {
                return item.second.isEnabled;
            }
        }
    }
    LOGI("get constraint from cache failed, retry from account manager.");
    bool isEnabled = false;
    ErrCode errcode = OHOS::AccountSA::CheckOsAccountConstraintEnabled(userId, constraint, isEnabled);
    if (errcode != ERR_OK) {
        LOGE("check constraint failed errcode %{public}d.", errcode);
        return false;
    }

    AccountSA::OsAccountConstraintStateData constrainData;
    constrainData.localId = userId;
    constrainData.constraint = constraint;
    constrainData.isEnabled = isEnabled;

    DmOsAccountConstraintStateData dmConstrainData;
    dmConstrainData.userId = userId;
    dmConstrainData.constraint = constraint;
    {
        std::lock_guard<std::mutex> lock(dmConstrainDataLock_); 
        dmConstrainDatas_[dmConstrainData] = constrainData;
    }
    return isEnabled;
}

void DmConstrainsManager::AddConstraint(const AccountSA::OsAccountConstraintStateData &constrainData)
{
    LOGI("userId %{public}d, constraint %{public}s, isEnable %{public}d.",
        constrainData.localId, constrainData.constraint.c_str(), constrainData.isEnabled);
    DmOsAccountConstraintStateData dmConstraint;
    dmConstraint.userId = constrainData.localId;
    dmConstraint.constraint = constrainData.constraint;
    {
        std::lock_guard<std::mutex> lock(dmConstrainDataLock_);
        dmConstrainDatas_[dmConstraint] = constrainData;
    }
}

void DmConstrainsManager::DeleteConstraint(int32_t userId)
{
    LOGI("userId %{public}d.", userId);
    std::lock_guard<std::mutex> lock(dmConstrainDataLock_);
    for (auto it = dmConstrainDatas_.begin(); it != dmConstrainDatas_.end(); ) {
        if (it->first.userId == userId) {
            it = dmConstrainDatas_.erase(it);
        } else {
            ++it;
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS