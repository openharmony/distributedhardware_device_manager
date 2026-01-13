/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_HICHAIN_AUTH_CONNECTOR_MOCK_H
#define OHOS_HICHAIN_AUTH_CONNECTOR_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "hichain_auth_connector.h"

namespace OHOS {
namespace DistributedHardware {
class DmHiChainAuthConnector {
public:
    virtual ~DmHiChainAuthConnector() = default;
public:
    virtual bool QueryCredential(std::string &localUdid, int32_t osAccountId, int32_t peerOsAccountId) = 0;
    virtual int32_t AuthDevice(const std::string &pinCode, int32_t osAccountId,
        std::string udid, int64_t requestId) = 0;
    virtual int32_t ImportCredential(int32_t osAccountId, int32_t peerOsAccountId, std::string deviceId,
        std::string publicKey) = 0;

    virtual int32_t ProcessCredData(int64_t authReqId, const std::string &data) = 0;
    virtual int32_t AddCredential(int32_t osAccountId, const std::string &authParams, std::string &creId) = 0;
    virtual int32_t ExportCredential(int32_t osAccountId, const std::string &credId, std::string &publicKey) = 0;
    virtual int32_t AgreeCredential(int32_t osAccountId, const std::string selfCredId, const std::string &authParams,
        std::string &credId) = 0;
    virtual int32_t DeleteCredential(int32_t osAccountId, const std::string &creId) = 0;
    virtual int32_t AuthCredential(int32_t osAccountId, int64_t authReqId, const std::string &credId,
        const std::string &pinCode) = 0;
    virtual int32_t AuthCredentialPinCode(int32_t osAccountId, int64_t authReqId, const std::string &pinCode) = 0;
    virtual int32_t QueryCredentialInfo(int32_t userId, const JsonObject &queryParams, JsonObject &resultJson) = 0;
    virtual int32_t DeleteCredential(const std::string &deviceId, int32_t userId, int32_t peerUserId) = 0;
    virtual int32_t QueryCredInfoByCredId(int32_t userId, const std::string &credId, JsonObject &resultJson) = 0;
    virtual int32_t UpdateCredential(const std::string &credId, int32_t userId, std::vector<std::string> &tokenIds) = 0;
public:
    static inline std::shared_ptr<DmHiChainAuthConnector> dmHiChainAuthConnector = nullptr;
};

class HiChainAuthConnectorMock : public DmHiChainAuthConnector {
public:
    MOCK_METHOD(bool, QueryCredential, (std::string &, int32_t, int32_t));
    MOCK_METHOD(int32_t, AuthDevice, (const std::string &, int32_t, std::string, int64_t));
    MOCK_METHOD(int32_t, ImportCredential, (int32_t, int32_t, std::string, std::string));

    MOCK_METHOD(int32_t, ProcessCredData, (int64_t, const std::string &));
    MOCK_METHOD(int32_t, AddCredential, (int32_t, const std::string &, std::string &));
    MOCK_METHOD(int32_t, ExportCredential, (int32_t, const std::string &, std::string &));
    MOCK_METHOD(int32_t, AgreeCredential, (int32_t, const std::string, const std::string &, std::string &));
    MOCK_METHOD(int32_t, DeleteCredential, (int32_t, const std::string &));
    MOCK_METHOD(int32_t, AuthCredential, (int32_t, int64_t, const std::string &, const std::string &));
    MOCK_METHOD(int32_t, AuthCredentialPinCode, (int32_t, int64_t, const std::string &));
    MOCK_METHOD(int32_t, QueryCredentialInfo, (int32_t, const JsonObject &, JsonObject &));
    MOCK_METHOD(int32_t, DeleteCredential, (const std::string &, int32_t, int32_t));
    MOCK_METHOD(int32_t, QueryCredInfoByCredId, (int32_t, const std::string &, JsonObject &));
    MOCK_METHOD(int32_t, UpdateCredential, (const std::string &, int32_t, std::vector<std::string> &));
};
}
}
#endif
