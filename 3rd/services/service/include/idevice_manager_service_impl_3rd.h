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

#ifndef OHOS_I_DM_SERVICE_IMPL_3RD_H
#define OHOS_I_DM_SERVICE_IMPL_3RD_H

#include <string>
#include <map>

#include "idevice_manager_service_listener_3rd.h"
#include "device_manager_data_struct_3rd.h"

namespace OHOS {
namespace DistributedHardware {
class IDeviceManagerServiceImpl3rd {
public:
    virtual ~IDeviceManagerServiceImpl3rd() {}

    virtual int32_t Initialize(const std::shared_ptr<IDeviceManagerServiceListener3rd> &listener) = 0;
    virtual void Release() = 0;
    virtual int32_t ImportPinCode3rd(const std::string &businessName, const std::string &pinCode) = 0;
    virtual int32_t AuthPincode(const PeerTargetId3rd &targetId, std::map<std::string, std::string> &authparam) = 0;
    virtual int32_t AuthDevice3rd(const PeerTargetId3rd &targetId, const std::map<std::string,
        std::string> &authParam) = 0;
    virtual int OnAuth3rdAclSessionOpened(int sessionId, int result) = 0;
    virtual void OnAuth3rdAclSessionClosed(int sessionId) = 0;
    virtual void OnAuth3rdAclBytesReceived(int sessionId, const void *data, unsigned int dataLen) = 0;

    virtual int OnAuth3rdSessionOpened(int sessionId, int result) = 0;
    virtual void OnAuth3rdSessionClosed(int sessionId) = 0;
    virtual void OnAuth3rdBytesReceived(int sessionId, const void *data, unsigned int dataLen) = 0;
};

using CreateDMServiceImpl3rdFuncPtr = IDeviceManagerServiceImpl3rd *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_I_DM_SERVICE_IMPL_3RD_H