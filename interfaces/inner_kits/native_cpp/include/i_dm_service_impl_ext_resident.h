/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_I_DM_SERVICE_IMPL_EXT_RESIDENT_H
#define OHOS_I_DM_SERVICE_IMPL_EXT_RESIDENT_H

#include "idevice_manager_service_listener.h"

namespace OHOS {
namespace DistributedHardware {
class IDMServiceImplExtResident {
public:
    virtual ~IDMServiceImplExtResident() = default;
    virtual int32_t Initialize(const std::shared_ptr<IDeviceManagerServiceListener> &listener) = 0;
    virtual int32_t Release() = 0;
};

using CreateDMServiceExtResidentFuncPtr = IDMServiceImplExtResident *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_IMPL_EXT_RESIDENT_H
