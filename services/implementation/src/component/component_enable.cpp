/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "component_enable.h"

#include "anonymous_string.h"
#include "constants.h"
#include "dh_utils_hisysevent.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentEnable"

ComponentEnable::ComponentEnable() : status_(std::numeric_limits<int32_t>::max()) {}

ComponentEnable::~ComponentEnable() {}

int32_t ComponentEnable::Enable(const std::string &networkId, const std::string &dhId, const EnableParam &param,
    IDistributedHardwareSource *handler)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGD("networkId = %{public}s dhId = %{public}s.", GetAnonyString(networkId).c_str(),
        GetAnonyString(dhId).c_str());
    if (handler == nullptr) {
        DHLOGE("handler is null, networkId = %{public}s dhId = %{public}s.", GetAnonyString(networkId).c_str(),
            GetAnonyString(dhId).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }

    auto ret = handler->RegisterDistributedHardware(networkId, dhId, param, shared_from_this());
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("RegisterDistributedHardware failed, networkId = %{public}s dhId = %{public}s.",
            GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
        HiSysEventWriteCompMgrFailedMsg(DHFWK_DH_REGISTER_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            GetAnonyString(dhId), ret, "dhfwk register distributed hardware failed.");
        return ERR_DH_FWK_COMPONENT_REGISTER_FAILED;
    }

    // wait for callback until timeout
    std::unique_lock<std::mutex> lock(mutex_);
    auto waitStatus = conVar_.wait_for(lock, std::chrono::milliseconds(ENABLE_TIMEOUT_MS),
        [this]() { return status_ != std::numeric_limits<int32_t>::max(); });
    if (!waitStatus) {
        DHLOGE("enable timeout, networkId = %{public}s dhId = %{public}s", GetAnonyString(networkId).c_str(),
            GetAnonyString(dhId).c_str());
        HiSysEventWriteCompMgrFailedMsg(DHFWK_DH_REGISTER_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            GetAnonyString(dhId), ERR_DH_FWK_COMPONENT_ENABLE_TIMEOUT,
            "dhfwk distributed hardware enable timeout.");
        return ERR_DH_FWK_COMPONENT_ENABLE_TIMEOUT;
    }
    return (status_ == DH_FWK_SUCCESS) ? DH_FWK_SUCCESS : ERR_DH_FWK_COMPONENT_ENABLE_FAILED;
}

int32_t ComponentEnable::OnRegisterResult(const std::string &networkId, const std::string &dhId, int32_t status,
    const std::string &data)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (status == DH_FWK_SUCCESS) {
        DHLOGI("enable success, networkId = %{public}s, dhId = %{public}s.", GetAnonyString(networkId).c_str(),
            GetAnonyString(dhId).c_str());
    } else {
        DHLOGE("enable failed, networkId = %{public}s, dhId = %{public}s, status = %{public}d.",
            GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str(), status);
    }

    std::unique_lock<std::mutex> lock(mutex_);
    status_ = status;
    conVar_.notify_all();
    return status_;
}
} // namespace DistributedHardware
} // namespace OHOS
