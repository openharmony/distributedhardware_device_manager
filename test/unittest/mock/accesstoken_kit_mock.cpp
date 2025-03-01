/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "accesstoken_kit_mock.h"

using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace Security {
namespace AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return AccessTokenKitInterface::GetOrCreateAccessTokenKit()->GetTokenTypeFlag(tokenID);
}

int AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapTokenInfoRes)
{
    return AccessTokenKitInterface::GetOrCreateAccessTokenKit()->GetHapTokenInfo(tokenID, hapTokenInfoRes);
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfoRes)
{
    return AccessTokenKitInterface::GetOrCreateAccessTokenKit()->GetNativeTokenInfo(tokenID, nativeTokenInfoRes);
}

AccessTokenID AccessTokenKit::GetNativeTokenId(const std::string &GetNativeTokenId)
{
    return AccessTokenKitInterface::GetOrCreateAccessTokenKit()->GetNativeTokenId(GetNativeTokenId);
}

AccessTokenID AccessTokenKit::GetHapTokenID(int32_t userID, const std::string &bundleName, int32_t instIndex)
{
    return AccessTokenKitInterface::GetOrCreateAccessTokenKit()->GetHapTokenID(userID, bundleName, instIndex);
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    return AccessTokenKitInterface::GetOrCreateAccessTokenKit()->VerifyAccessToken(tokenID, permissionName);
}
} // namespace AccessToken
} // namespace Security

namespace DistributedHardware {
std::shared_ptr<AccessTokenKitInterface> AccessTokenKitInterface::token_ = nullptr;

std::shared_ptr<AccessTokenKitInterface> AccessTokenKitInterface::GetOrCreateAccessTokenKit()
{
    if (!token_) {
        token_ = std::make_shared<AccessTokenKitMock>();
    }
    return token_;
}

void AccessTokenKitInterface::ReleaseAccessTokenKit()
{
    token_.reset();
    token_ = nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS
