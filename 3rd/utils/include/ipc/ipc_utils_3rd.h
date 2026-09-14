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

#ifndef OHOS_DM_IPC_UTILS_3RD_H
#define OHOS_DM_IPC_UTILS_3RD_H

#include <string>
#include <vector>
#include <securec.h>

#include "dm_log_3rd.h"
#include "ipc_types.h"
#include "device_manager_data_struct_3rd.h"

namespace OHOS {
namespace DistributedHardware {

inline void SecureMemZeroString(std::string &str) noexcept
{
    if (!str.empty()) {
        (void)memset_s(&str[0], str.size(), 0, str.size());
        str.clear();
    }
}

inline void SecureMemZeroVector(std::vector<unsigned char> &bytes) noexcept
{
    if (!bytes.empty()) {
        (void)memset_s(bytes.data(), bytes.size(), 0, bytes.size());
        bytes.clear();
    }
}

#define WRITE_HELPER_RET(parcel, type, value, failRet) \
    do { \
        if (!((parcel).Write##type((value)))) { \
            LOGE("write value failed!"); \
            return failRet; \
        } \
    } while (0)

#define READ_HELPER_RET(parcel, type, out, failRet) \
    do { \
        if (!((parcel).Read##type((out)))) { \
            LOGE("read value failed!"); \
            return failRet; \
        } \
    } while (0)

#define WRITE_INTERFACE_TOKEN(data, failRet) \
    do { \
        if (!(data).WriteInterfaceToken(u"ohos.distributedhardware.devicemanager")) { \
            LOGE("Write interface token failed!"); \
            return failRet; \
        } \
    } while (0)

} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_UTILS_3RD_H
