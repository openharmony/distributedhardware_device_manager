/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_LOAD_FWK_H
#define OHOS_DM_LOAD_FWK_H
#include "distributed_hardware_load_callback.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DmLoadFwk {
    DECLARE_SINGLE_INSTANCE(DmLoadFwk);
public:
    int32_t LoadFwk(void);
    void ResetLoadCallback(void);
private:
    sptr<DistributedHardwareLoadCallback> loadCallback_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PERMISSION_MANAGER_H
