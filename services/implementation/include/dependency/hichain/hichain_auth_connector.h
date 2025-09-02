/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {

enum {
    /** invalid operationCode for initialize */
    CRED_OP_INVALID = -1,
    /** operationCode for ProcessCredential to query credential */
    CRED_OP_QUERY,
    /** operationCode for ProcessCredential to create credential */
    CRED_OP_CREATE,
    /** operationCode for ProcessCredential to import credential */
    CRED_OP_IMPORT,
    /** operationCode for ProcessCredential to delete credential */
    CRED_OP_DELETE,
};

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
    int32_t AuthDevice(const std::string &pinCode, int32_t osAccountId, std::string udid, int64_t requestId);
    int32_t ProcessAuthData(int64_t requestId, std::string authData, int32_t osAccountId);
    int32_t GenerateCredential(std::string &localUdid, int32_t osAccountId, std::string &publicKey);
    bool QueryCredential(std::string &localUdid, int32_t osAccountId, int32_t peerOsAccountId);
    int32_t QueryCredentialInfo(int32_t userId, const JsonObject &queryParams, JsonObject &resultJson);
    int32_t QueryCredInfoByCredId(int32_t userId, std::string credId, JsonObject &resultJson);
    int32_t ImportCredential(int32_t osAccountId, int32_t peerOsAccountId, std::string deviceId, std::string publicKey);
    int32_t DeleteCredential(const std::string &deviceId, int32_t userId, int32_t peerUserId);
    int32_t RegisterHiChainAuthCallback(std::shared_ptr<IDmDeviceAuthCallback> callback);
    int32_t UnRegisterHiChainAuthCallback();
    int32_t RegisterHiChainAuthCallbackById(int64_t id, std::shared_ptr<IDmDeviceAuthCallback> callback);
    int32_t UnRegisterHiChainAuthCallbackById(int64_t id);
    int32_t GetCredential(std::string &localUdid, int32_t osAccountId, std::string &publicKey);

    int32_t ProcessCredData(int64_t authReqId, const std::string &data);
    int32_t AddCredential(int32_t osAccountId, const std::string &authParams, std::string &creId);
    int32_t ExportCredential(int32_t osAccountId, const std::string &credId, std::string &publicKey);
    int32_t AgreeCredential(int32_t osAccountId, const std::string selfCredId, const std::string &authParams,
        std::string &credId);
    int32_t DeleteCredential(int32_t osAccountId, const std::string &creId);
    // Credential authentication pinCode Pin code (required for point-to-point temporary credentials)
    int32_t AuthCredential(int32_t osAccountId, int64_t authReqId, const std::string &credId,
        const std::string &pinCode);
    int32_t AuthCredentialPinCode(int32_t osAccountId, int64_t authReqId, const std::string &pinCode);
    int32_t AddTokensToCredential(const std::string &credId, int32_t userId, std::vector<std::string> &tokenIds);
    int32_t UpdateCredential(const std::string &credId, int32_t userId, std::vector<std::string> &tokenIds);
private:
    void FreeJsonString(char *jsonStr);
    void FreeCharArray(char *charArray);
    static std::shared_ptr<IDmDeviceAuthCallback> GetDeviceAuthCallback(int64_t id);

private:
    DeviceAuthCallback deviceAuthCallback_;
    static std::shared_ptr<IDmDeviceAuthCallback> dmDeviceAuthCallback_;
    static std::map<int64_t, std::shared_ptr<IDmDeviceAuthCallback>> dmDeviceAuthCallbackMap_;
    static std::mutex dmDeviceAuthCallbackMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HICHAIN_AUTH_CONNECTOR_H
