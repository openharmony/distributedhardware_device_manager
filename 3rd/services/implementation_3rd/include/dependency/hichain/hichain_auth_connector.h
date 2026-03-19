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

#ifndef OHOS_HICHAIN_AUTH_CONNECTOR_H
#define OHOS_HICHAIN_AUTH_CONNECTOR_H

#include <map>
#include <mutex>

#include "device_auth.h"
#include "device_auth_defines.h"
#include "hichain_connector_callback.h"
#include "ffrt.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {

class HiChainAuthConnector {
public:
    static bool onTransmit(int64_t requestId, const uint8_t *data, uint32_t dataLen);
    static void onFinish(int64_t requestId, int operationCode, const char *returnData);
    static void onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn);
    static char *onRequest(int64_t requestId, int operationCode, const char *reqParams);
    static void onSessionKeyReturned(int64_t requestId, const uint8_t *sessionKey, uint32_t sessionKeyLen);

public:
    HiChainAuthConnector();
    ~HiChainAuthConnector();
    int32_t RegisterHiChainAuthCallbackById(int64_t id, std::shared_ptr<IDmDeviceAuthCallback> callback);
    int32_t UnRegisterHiChainAuthCallbackById(int64_t id);

    int32_t AuthCredentialPinCode(int32_t osAccountId, int64_t authReqId, const std::string &pinCode);
    int32_t ProcessCredData(int64_t authReqId, const std::string &data);

private:
    static std::shared_ptr<IDmDeviceAuthCallback> GetDeviceAuthCallback(int64_t id);
    static std::map<int64_t, std::shared_ptr<IDmDeviceAuthCallback>> dmDeviceAuthCallbackMap_;
    static ffrt::mutex dmDeviceAuthCallbackMutex_;
    DeviceAuthCallback deviceAuthCallback_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HICHAIN_AUTH_CONNECTOR_H
