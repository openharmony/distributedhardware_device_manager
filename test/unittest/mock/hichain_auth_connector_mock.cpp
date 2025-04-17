/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "hichain_auth_connector_mock.h"

#include "gtest/gtest.h"

namespace OHOS {
namespace DistributedHardware {
bool HiChainAuthConnector::QueryCredential(std::string &localUdid, int32_t osAccountId, int32_t peerOsAccountId)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->QueryCredential(localUdid, osAccountId, peerOsAccountId);
}

int32_t HiChainAuthConnector::AuthDevice(const std::string &pinCode, int32_t osAccountId, std::string udid,
    int64_t requestId)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->AuthDevice(pinCode, osAccountId, udid, requestId);
}

int32_t HiChainAuthConnector::ImportCredential(int32_t osAccountId, int32_t peerOsAccountId, std::string deviceId,
    std::string publicKey)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->ImportCredential(osAccountId, peerOsAccountId, deviceId,
        publicKey);
}

int32_t HiChainAuthConnector::ProcessCredData(int64_t authReqId, const std::string &data)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->ProcessCredData(authReqId, data);
}

int32_t HiChainAuthConnector::AddCredential(int32_t osAccountId, const std::string &authParams, std::string &creId)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->AddCredential(osAccountId, authParams, creId);
}

int32_t HiChainAuthConnector::ExportCredential(int32_t osAccountId, const std::string &credId, std::string &publicKey)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->ExportCredential(osAccountId, credId, publicKey);
}

int32_t HiChainAuthConnector::AgreeCredential(int32_t osAccountId, const std::string selfCredId,
    const std::string &authParams, std::string &credId)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->AgreeCredential(osAccountId, selfCredId, authParams, credId);
}

int32_t HiChainAuthConnector::DeleteCredential(int32_t osAccountId, const std::string &creId)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->DeleteCredential(osAccountId, creId);
}

int32_t HiChainAuthConnector::AuthCredential(int32_t osAccountId, int64_t authReqId, const std::string &credId,
    const std::string &pinCode)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->AuthCredential(osAccountId, authReqId, credId, pinCode);
}

int32_t HiChainAuthConnector::AuthCredentialPinCode(int32_t osAccountId, int64_t authReqId, const std::string &pinCode)
{
    return DmHiChainAuthConnector::dmHiChainAuthConnector->AuthCredentialPinCode(osAccountId, authReqId, pinCode);
}

} // namespace DistributedHardware
} // namespace OHOS