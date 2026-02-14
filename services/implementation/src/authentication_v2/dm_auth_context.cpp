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

#include "dm_auth_context.h"

namespace OHOS {
namespace DistributedHardware {

std::string DmAuthContext::GetDeviceId(DmAuthSide side)
{
    const DmAccess &localAccess = (direction == DM_AUTH_SOURCE) ? accesser : accessee;
    const DmAccess &remoteAccess = (direction == DM_AUTH_SOURCE) ? accessee : accesser;
    return (side == DM_AUTH_LOCAL_SIDE) ? localAccess.deviceId : remoteAccess.deviceId;
}

int32_t DmAuthContext::GetUserId(DmAuthSide side)
{
    const DmAccess &localAccess = (direction == DM_AUTH_SOURCE) ? accesser : accessee;
    const DmAccess &remoteAccess = (direction == DM_AUTH_SOURCE) ? accessee : accesser;
    return (side == DM_AUTH_LOCAL_SIDE) ? localAccess.userId : remoteAccess.userId;
}

std::string DmAuthContext::GetCredentialId(DmAuthSide side, DmAuthScope authorizedScope)
{
    const DmAccess &localAccess = (direction == DM_AUTH_SOURCE) ? accesser : accessee;
    const DmAccess &remoteAccess = (direction == DM_AUTH_SOURCE) ? accessee : accesser;
    const DmAccess &access = (side == DM_AUTH_LOCAL_SIDE) ? localAccess : remoteAccess;
    return (authorizedScope == DM_AUTH_SCOPE_LNN) ? access.lnnCredentialId : access.transmitCredentialId;
}

std::string DmAuthContext::GetPublicKey(DmAuthSide side, DmAuthScope authorizedScope)
{
    const DmAccess &localAccess = (direction == DM_AUTH_SOURCE) ? accesser : accessee;
    const DmAccess &remoteAccess = (direction == DM_AUTH_SOURCE) ? accessee : accesser;
    const DmAccess &access = (side == DM_AUTH_LOCAL_SIDE) ? localAccess : remoteAccess;
    return (authorizedScope == DM_AUTH_SCOPE_LNN) ? access.lnnPublicKey : access.transmitPublicKey;
}

void DmAuthContext::SetCredentialId(DmAuthSide side, DmAuthScope authorizedScope, const std::string &credentialId)
{
    DmAccess &localAccess = (direction == DM_AUTH_SOURCE) ? accesser : accessee;
    DmAccess &remoteAccess = (direction == DM_AUTH_SOURCE) ? accessee : accesser;
    DmAccess &access = (side == DM_AUTH_LOCAL_SIDE) ? localAccess : remoteAccess;
    std::string &credId = (authorizedScope == DM_AUTH_SCOPE_LNN) ?
        access.lnnCredentialId : access.transmitCredentialId;
    credId = credentialId;
    return;
}

void DmAuthContext::SetPublicKey(DmAuthSide side, DmAuthScope authorizedScope, const std::string &publicKey)
{
    DmAccess &localAccess = (direction == DM_AUTH_SOURCE) ? accesser : accessee;
    DmAccess &remoteAccess = (direction == DM_AUTH_SOURCE) ? accessee : accesser;
    DmAccess &access = (side == DM_AUTH_LOCAL_SIDE) ? localAccess : remoteAccess;
    std::string &key = (authorizedScope == DM_AUTH_SCOPE_LNN) ?
        access.lnnPublicKey : access.transmitPublicKey;
    key = publicKey;

    return;
}

std::string DmAuthContext::GetAccountId(DmAuthSide side)
{
    DmAccess &localAccess = (direction == DM_AUTH_SOURCE) ? accesser : accessee;
    DmAccess &remoteAccess = (direction == DM_AUTH_SOURCE) ? accessee : accesser;
    return (side == DM_AUTH_LOCAL_SIDE) ? localAccess.accountId : remoteAccess.accountId;
}

std::string DmAuthContext::GetAclBundleName(const DmAuthDirection &direction, DmProxyAuthContext &proxyAuthContext)
{
    DmProxyAccess &access =
        (direction == DM_AUTH_SOURCE) ? proxyAuthContext.proxyAccesser : proxyAuthContext.proxyAccessee;
    return !access.pkgName.empty() ? access.pkgName : access.bundleName;
}
}  // namespace DistributedHardware
}  // namespace OHOS