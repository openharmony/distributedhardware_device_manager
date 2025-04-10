/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_DM_HISYSEVENT_H
#define OHOS_DISTRIBUTED_DM_HISYSEVENT_H

#include <string>

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__ ((visibility ("default")))
#endif // DM_EXPORT

namespace OHOS {
namespace DistributedHardware {
DM_EXPORT void SysEventWrite(
    const std::string &status, int32_t eventType, const std::string &msg);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_DM_HISYSEVENT_H
