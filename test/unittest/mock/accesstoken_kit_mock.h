/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License") = 0;
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

#ifndef OHOS_DM_ACCESS_TOKEN_KIT_MOCK_H
#define OHOS_DM_ACCESS_TOKEN_KIT_MOCK_H

#include <gmock/gmock.h>

#include "accesstoken_kit.h"
#include "tokenid_kit.h"

using OHOS::Security::AccessToken::ATokenTypeEnum;
using OHOS::Security::AccessToken::AccessTokenID;
using OHOS::Security::AccessToken::HapTokenInfo;
using OHOS::Security::AccessToken::NativeTokenInfo;

namespace OHOS {
namespace DistributedHardware {
class AccessTokenKitInterface {
public:
    AccessTokenKitInterface() = default;
    virtual ~AccessTokenKitInterface() = default;

    virtual ATokenTypeEnum GetTokenTypeFlag(AccessTokenID) = 0;
    virtual int GetHapTokenInfo(AccessTokenID, HapTokenInfo &) = 0;
    virtual int GetNativeTokenInfo(AccessTokenID, NativeTokenInfo &) = 0;
    virtual AccessTokenID GetNativeTokenId(const std::string &) = 0;
    virtual AccessTokenID GetHapTokenID(int32_t, const std::string &, int32_t) = 0;
    virtual int VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName) = 0;
    virtual bool IsSystemAppByFullTokenID(uint64_t) = 0;

    static std::shared_ptr<AccessTokenKitInterface> GetOrCreateAccessTokenKit();
    static void ReleaseAccessTokenKit();
private:
    static std::shared_ptr<AccessTokenKitInterface> token_;
};

class AccessTokenKitMock : public AccessTokenKitInterface {
public:
    AccessTokenKitMock() = default;
    ~AccessTokenKitMock() override = default;

    MOCK_METHOD(ATokenTypeEnum, GetTokenTypeFlag, (AccessTokenID));
    MOCK_METHOD(int, GetHapTokenInfo, (AccessTokenID, HapTokenInfo &));
    MOCK_METHOD(int, GetNativeTokenInfo, (AccessTokenID, NativeTokenInfo &));
    MOCK_METHOD(AccessTokenID, GetNativeTokenId, (const std::string &));
    MOCK_METHOD(AccessTokenID, GetHapTokenID, (int32_t, const std::string &, int32_t));
    MOCK_METHOD(int, VerifyAccessToken, (AccessTokenID, const std::string&));
    MOCK_METHOD(bool, IsSystemAppByFullTokenID, (uint64_t));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_ACCESS_TOKEN_KIT_MOCK_H
