/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_SPATIAL_LOCATION_CALLBACK_IMPL_DM_H
#define OHOS_SPATIAL_LOCATION_CALLBACK_IMPL_DM_H

#include "spatial_location_callback_stub.h"
#include "dm_auth_state.h"
#include "dm_auth_context.h"
#include "spatial_awareness_mgr_client.h"

namespace OHOS {
namespace DistributedHardware {
class SpatialLocationCallbackImpl : public Msdp::SpatialLocationCallbackStub {
public:
    SpatialLocationCallbackImpl(std::shared_ptr<DmAuthContext> context);
    ~SpatialLocationCallbackImpl();
    void OnPinCodeChanged(const Msdp::PinCodeResponse &pinCodeResponse) override;

private:
    bool IsValidPinCodeStr(const std::string &pinCodeStr);
    std::shared_ptr<DmAuthContext> context_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_SPATIAL_LOCATION_CALLBACK_IMPL_DM_H