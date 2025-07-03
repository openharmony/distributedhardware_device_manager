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
#include "dm_crypto.h"
#include "dm_negotiate_process.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(NegotiateProcess);

int32_t OnlyPinBind(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    context->needBind = true;
    context->needAgreeCredential = false;
    context->needAuth = false;
    return DM_OK;
}

int32_t EndBind(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    context->needBind = false;
    context->needAgreeCredential = false;
    context->needAuth = false;
    context->reason = DM_BIND_TRUST_TARGET;
    return DM_BIND_TRUST_TARGET;
}

int32_t OnlyCredAuth(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    context->needBind = false;
    context->needAgreeCredential = false;
    context->needAuth = true;
    return DM_OK;
}

int32_t PinBindAndCredAuth(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    context->needBind = true;
    context->needAgreeCredential = true;
    context->needAuth = true;
    return DM_OK;
}

NegotiateProcess::NegotiateProcess()
{
    LOGI("start.");
    handlers_[NegotiateSpec(CredType::DM_NO_CRED, AclType::DM_NO_ACL, AuthType::DM_INPUT_PINCODE)] =
        std::make_unique<NoCredNoAclInputAuthType>();
    handlers_[NegotiateSpec(CredType::DM_NO_CRED, AclType::DM_NO_ACL, AuthType::DM_IMPORT_AUTHTYPE)] =
        std::make_unique<NoCredNoAclImportAuthType>();
    handlers_[NegotiateSpec(CredType::DM_IDENTICAL_CREDTYPE, AclType::DM_NO_ACL, AuthType::DM_INPUT_PINCODE)] =
        std::make_unique<IdentCredNoAclInputAuthType>();
    handlers_[NegotiateSpec(CredType::DM_IDENTICAL_CREDTYPE, AclType::DM_NO_ACL, AuthType::DM_IMPORT_AUTHTYPE)] =
        std::make_unique<IdentCredNoAclImportAuthType>();
    handlers_[NegotiateSpec(CredType::DM_IDENTICAL_CREDTYPE, AclType::DM_IDENTICAL_ACL, AuthType::DM_INPUT_PINCODE)] =
        std::make_unique<IdentCredIdentAclInputAuthType>();
    handlers_[NegotiateSpec(CredType::DM_IDENTICAL_CREDTYPE, AclType::DM_IDENTICAL_ACL, AuthType::DM_IMPORT_AUTHTYPE)] =
        std::make_unique<IdentCredIdentAclImportAuthType>();
    handlers_[NegotiateSpec(CredType::DM_IDENTICAL_CREDTYPE, AclType::DM_P2P_ACL, AuthType::DM_INPUT_PINCODE)] =
        std::make_unique<IdentCredP2pAclInputAuthType>();
    handlers_[NegotiateSpec(CredType::DM_IDENTICAL_CREDTYPE, AclType::DM_P2P_ACL, AuthType::DM_IMPORT_AUTHTYPE)] =
        std::make_unique<IdentCredP2pAclImportAuthType>();
    handlers_[NegotiateSpec(CredType::DM_SHARE_CREDTYPE, AclType::DM_NO_ACL, AuthType::DM_INPUT_PINCODE)] =
        std::make_unique<ShareCredNoAclInputAuthType>();
    handlers_[NegotiateSpec(CredType::DM_SHARE_CREDTYPE, AclType::DM_NO_ACL, AuthType::DM_IMPORT_AUTHTYPE)] =
        std::make_unique<ShareCredNoAclImportAuthType>();
    handlers_[NegotiateSpec(CredType::DM_SHARE_CREDTYPE, AclType::DM_SHARE_ACL, AuthType::DM_INPUT_PINCODE)] =
        std::make_unique<ShareCredShareAclInputAuthType>();
    handlers_[NegotiateSpec(CredType::DM_SHARE_CREDTYPE, AclType::DM_SHARE_ACL, AuthType::DM_IMPORT_AUTHTYPE)] =
        std::make_unique<ShareCredShareAclImportAuthType>();
    handlers_[NegotiateSpec(CredType::DM_SHARE_CREDTYPE, AclType::DM_P2P_ACL, AuthType::DM_INPUT_PINCODE)] =
        std::make_unique<ShareCredP2pAclInputAuthType>();
    handlers_[NegotiateSpec(CredType::DM_SHARE_CREDTYPE, AclType::DM_P2P_ACL, AuthType::DM_IMPORT_AUTHTYPE)] =
        std::make_unique<ShareCredP2pAclImportAuthType>();
    handlers_[NegotiateSpec(CredType::DM_P2P_CREDTYPE, AclType::DM_NO_ACL, AuthType::DM_INPUT_PINCODE)] =
        std::make_unique<P2pCredNoAclInputAuthType>();
    handlers_[NegotiateSpec(CredType::DM_P2P_CREDTYPE, AclType::DM_NO_ACL, AuthType::DM_IMPORT_AUTHTYPE)] =
        std::make_unique<P2pCredNoAclImportAuthType>();
    handlers_[NegotiateSpec(CredType::DM_P2P_CREDTYPE, AclType::DM_P2P_ACL, AuthType::DM_INPUT_PINCODE)] =
        std::make_unique<P2pCredP2pAclInputAuthType>();
    handlers_[NegotiateSpec(CredType::DM_P2P_CREDTYPE, AclType::DM_P2P_ACL, AuthType::DM_IMPORT_AUTHTYPE)] =
        std::make_unique<P2pCredP2pAclImportAuthType>();
}

int32_t NegotiateProcess::HandleNegotiateResult(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    std::string credTypeList = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        context->accesser.credTypeList : context->accessee.credTypeList;
    std::string aclTypeList = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        context->accesser.aclTypeList : context->accessee.aclTypeList;
    CredType credType = ConvertCredType(credTypeList);
    AclType aclType = ConvertAclType(aclTypeList);
    AuthType authType = ConvertAuthType(context->authType);
    LOGI("credType %{public}d, aclType %{public}d, authType %{public}d.",
        static_cast<int32_t>(credType), static_cast<int32_t>(aclType), static_cast<int32_t>(authType));
    int32_t ret = ERR_DM_CAPABILITY_NEGOTIATE_FAILED;
    NegotiateSpec negotiateSpec(credType, aclType, authType);
    auto handler = handlers_.find(negotiateSpec);
    if (handler != handlers_.end() && handler->second != nullptr) {
        ret = handler->second->NegotiateHandle(context);
    } else {
        return ret;
    }
    if (!context->IsProxyBind || context->subjectProxyOnes.empty() ||
        (credType == CredType::DM_IDENTICAL_CREDTYPE && aclType == AclType::DM_IDENTICAL_ACL) ||
        (credType == CredType::DM_SHARE_CREDTYPE && aclType == AclType::DM_SHARE_ACL)) {
        return ret;
    }
    return HandleProxyNegotiateResult(context, ret);
}

int32_t NegotiateProcess::HandleProxyNegotiateResult(std::shared_ptr<DmAuthContext> context, int32_t result)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return DM_OK;
    }
    bool isTrust = true;
    for (auto &app : context->subjectProxyOnes) {
        std::string credTypeList = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
            app.proxyAccesser.credTypeList : app.proxyAccessee.credTypeList;
        std::string aclTypeList = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
            app.proxyAccesser.aclTypeList : app.proxyAccessee.aclTypeList;
        CredType credType = ConvertCredType(credTypeList);
        AclType aclType = ConvertAclType(aclTypeList);
        if (credType == CredType::DM_P2P_CREDTYPE && aclType == AclType::DM_P2P_ACL) {
            app.needBind = false;
            app.needAgreeCredential = false;
            app.needAuth = false;
            app.IsNeedSetProxyRelationShip = IsNeedSetProxyRelationShip(context, app);
        }
    }
    return DM_OK;
}

bool NegotiateProcess::IsNeedSetProxyRelationShip(std::shared_ptr<DmAuthContext> context,
    DmProxyAuthContext &proxyContext)
{
    DmProxyAccess &access = context->direction == DmAuthDirection::DM_AUTH_SOURCE ?
        proxyContext.proxyAccesser : proxyContext.proxyAccessee;
    if (access.aclProfiles.find(DM_POINT_TO_POINT) == access.aclProfiles.end()) {
        return true;
    }
    std::string accesserExtraData = access.aclProfiles[DM_POINT_TO_POINT].GetAccesser().GetAccesserExtraData();
    std::string accesseeExtraData = access.aclProfiles[DM_POINT_TO_POINT].GetAccessee().GetAccesseeExtraData();
    if ((IsExistTheTokenId(accesserExtraData, proxyContext.proxyAccesser.tokenIdHash) &&
        IsExistTheTokenId(accesseeExtraData, proxyContext.proxyAccessee.tokenIdHash)) ||
        (IsExistTheTokenId(accesserExtraData, proxyContext.proxyAccessee.tokenIdHash) &&
        IsExistTheTokenId(accesseeExtraData, proxyContext.proxyAccesser.tokenIdHash))) {
        return false;
    }
    return true;
}

bool NegotiateProcess::IsExistTheTokenId(const std::string extraData, const std::string tokenIdHash)
{
    if (extraData.empty()) {
        return false;
    }
    JsonObject extObj;
    extObj.Parse(extraData);
    if (extObj.IsDiscarded()) {
        return false;
    }
    JsonObject proxyObj;
    if (IsString(extObj, TAG_PROXY)) {
        std::string proxyStr = extObj[TAG_PROXY].Get<std::string>();
        proxyObj.Parse(proxyStr);
    }
    if (proxyObj.IsDiscarded()) {
        return false;
    }
    for (auto const &item : proxyObj.Items()) {
        if (item.IsNumber() && tokenIdHash == Crypto::GetTokenIdHash(std::to_string(item.Get<int64_t>()))) {
            return true;
        }
    }
    return false;
}

CredType NegotiateProcess::ConvertCredType(const std::string &credType)
{
    LOGI("start credType %{public}s.", credType.c_str());
    CredType credTypeTemp = CredType::DM_NO_CRED;
    if (credType.empty()) {
        return credTypeTemp;
    }
    JsonObject credTypeJson;
    credTypeJson.Parse(credType);
    if (credTypeJson.IsDiscarded()) {
        LOGE("Parse credType str failed.");
        return credTypeTemp;
    }
    if (credTypeJson.Contains("identicalCredType")) {
        credTypeTemp = CredType::DM_IDENTICAL_CREDTYPE;
        return credTypeTemp;
    } else if (credTypeJson.Contains("shareCredType")) {
        credTypeTemp = CredType::DM_SHARE_CREDTYPE;
        return credTypeTemp;
    } else if (credTypeJson.Contains("pointTopointCredType")) {
        credTypeTemp = CredType::DM_P2P_CREDTYPE;
        return credTypeTemp;
    } else {
        credTypeTemp = CredType::DM_NO_CRED;
    }
    return credTypeTemp;
}

AclType NegotiateProcess::ConvertAclType(const std::string &aclType)
{
    LOGI("start credType %{public}s.", aclType.c_str());
    AclType aclTypeTemp = AclType::DM_NO_ACL;
    if (aclType.empty()) {
        return aclTypeTemp;
    }
    JsonObject aclTypeJson;
    aclTypeJson.Parse(aclType);
    if (aclTypeJson.IsDiscarded()) {
        LOGE("Parse credType str failed.");
        return aclTypeTemp;
    }
    if (aclTypeJson.Contains("identicalAcl")) {
        aclTypeTemp = AclType::DM_IDENTICAL_ACL;
        return aclTypeTemp;
    } else if (aclTypeJson.Contains("shareAcl")) {
        aclTypeTemp = AclType::DM_SHARE_ACL;
        return aclTypeTemp;
    } else if (aclTypeJson.Contains("pointTopointAcl")) {
        aclTypeTemp = AclType::DM_P2P_ACL;
        return aclTypeTemp;
    } else {
        aclTypeTemp = AclType::DM_NO_ACL;
    }
    return aclTypeTemp;
}

AuthType NegotiateProcess::ConvertAuthType(const DmAuthType &authType)
{
    LOGI("start authType %{public}d.", static_cast<int32_t>(authType));
    AuthType authTypeTemp = AuthType::DM_INVALIED_AUTHTYPE;
    if (authType == DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE || authType == DmAuthType::AUTH_TYPE_NFC) {
        authTypeTemp = AuthType::DM_IMPORT_AUTHTYPE;
    } else {
        authTypeTemp = AuthType::DM_INPUT_PINCODE;
    }
    return authTypeTemp;
}

int32_t NoCredNoAclInputAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return PinBindAndCredAuth(context);
}

int32_t NoCredNoAclImportAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return PinBindAndCredAuth(context);
}

int32_t IdentCredNoAclInputAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return OnlyCredAuth(context);
}

int32_t IdentCredNoAclImportAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return PinBindAndCredAuth(context);
}

int32_t IdentCredIdentAclInputAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return EndBind(context);
}

int32_t IdentCredIdentAclImportAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return OnlyPinBind(context);
}

int32_t IdentCredP2pAclInputAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return OnlyCredAuth(context);
}

int32_t IdentCredP2pAclImportAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return OnlyPinBind(context);
}

int32_t ShareCredNoAclInputAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return OnlyCredAuth(context);
}

int32_t ShareCredNoAclImportAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return PinBindAndCredAuth(context);
}

int32_t ShareCredShareAclInputAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return EndBind(context);
}

int32_t ShareCredShareAclImportAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return OnlyPinBind(context);
}

int32_t ShareCredP2pAclInputAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return OnlyCredAuth(context);
}

int32_t ShareCredP2pAclImportAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return OnlyPinBind(context);
}

int32_t P2pCredNoAclInputAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return PinBindAndCredAuth(context);
}

int32_t P2pCredNoAclImportAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return PinBindAndCredAuth(context);
}

int32_t P2pCredP2pAclInputAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return EndBind(context);
}

int32_t P2pCredP2pAclImportAuthType::NegotiateHandle(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    return OnlyPinBind(context);
}
} // namespace DistributedHardware
} // namespace OHOS