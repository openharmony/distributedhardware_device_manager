/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_3RD_IPC_INTERFACE_CODE_H
#define OHOS_DEVICE_MANAGER_3RD_IPC_INTERFACE_CODE_H

#include <cstdint>

/* SAID: 4802 */
namespace OHOS {
namespace DistributedHardware {
enum DM3rdIpcInterfaceCode {
    INIT_DEVICE_MANAGER = 1000,
    UNINIT_DEVICE_MANAGER,
    IMPORT_PINCODE_3RD,
    GENERATE_PINCODE_3RD,
    AUTH_PINCODE_3RD,
    AUTH_DEVICE_3RD,
    SAVE_TRUST_RELATION_3RD,
    QUERY_TRUST_RELATION_3RD,
    DELETE_TRUST_RELATION_3RD,
    ON_AUTH_RESULT_3RD,
    IPC_MSG_BUTT_3RD
};
} // namespace DistributedHardware
} // namespace OHOS
#endif