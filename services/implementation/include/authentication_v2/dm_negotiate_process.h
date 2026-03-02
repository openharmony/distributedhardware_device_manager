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
#ifndef OHOS_DM_NEGOTIATE_PROCESS_V2_H
#define OHOS_DM_NEGOTIATE_PROCESS_V2_H
#include <iostream>
#include <memory>
#include <map>

#include "dm_auth_context.h"
#include "dm_single_instance.h"
namespace OHOS {
namespace DistributedHardware {
enum CredType  : uint8_t {
    DM_NO_CRED = 0,
    DM_IDENTICAL_CREDTYPE = 1,
    DM_SHARE_CREDTYPE = 2,
    DM_P2P_CREDTYPE = 3,
};

enum AclType  : uint8_t {
    DM_NO_ACL = 0,
    DM_IDENTICAL_ACL = 1,
    DM_SHARE_ACL = 2,
    DM_P2P_ACL = 3,
};

enum AuthType  : uint8_t {
    DM_INVALIED_AUTHTYPE = 0,
    DM_INPUT_PINCODE = 1,
    DM_IMPORT_AUTHTYPE = 2,
};

class NegotiateSpec {
public:
    CredType credType;
    AclType aclType;
    AuthType authType;

    NegotiateSpec(CredType credTypeTemp, AclType aclTypeTemp, AuthType authTypeTemp)
        : credType(credTypeTemp), aclType(aclTypeTemp), authType(authTypeTemp) {}

    // 重载<运算符用于map排序
    bool operator<(const NegotiateSpec& other) const
    {
        if (credType != other.credType) return credType < other.credType;
        if (aclType != other.aclType) return aclType < other.aclType;
        return authType < other.authType;
    }
};

class NegotiateHandler {
public:
    virtual int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) = 0;
    virtual ~NegotiateHandler() {};
};

class NoCredNoAclInputAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~NoCredNoAclInputAuthType() {};
};

class NoCredNoAclImportAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~NoCredNoAclImportAuthType() {};
};

class IdentCredNoAclInputAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~IdentCredNoAclInputAuthType() {};
};

class IdentCredNoAclImportAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~IdentCredNoAclImportAuthType() {};
};

class IdentCredIdentAclInputAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~IdentCredIdentAclInputAuthType() {};
};

class IdentCredIdentAclImportAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~IdentCredIdentAclImportAuthType() {};
};

class IdentCredP2pAclInputAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~IdentCredP2pAclInputAuthType() {};
};

class IdentCredP2pAclImportAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~IdentCredP2pAclImportAuthType() {};
};

class ShareCredNoAclInputAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~ShareCredNoAclInputAuthType() {};
};

class ShareCredNoAclImportAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~ShareCredNoAclImportAuthType() {};
};

class ShareCredShareAclInputAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~ShareCredShareAclInputAuthType() {};
};

class ShareCredShareAclImportAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~ShareCredShareAclImportAuthType() {};
};

class ShareCredP2pAclInputAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~ShareCredP2pAclInputAuthType() {};
};

class ShareCredP2pAclImportAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~ShareCredP2pAclImportAuthType() {};
};

class P2pCredNoAclInputAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~P2pCredNoAclInputAuthType() {};
};

class P2pCredNoAclImportAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~P2pCredNoAclImportAuthType() {};
};

class P2pCredP2pAclInputAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~P2pCredP2pAclInputAuthType() {};
};

class P2pCredP2pAclImportAuthType : public NegotiateHandler {
public:
    int32_t NegotiateHandle(std::shared_ptr<DmAuthContext> context) override;
    virtual ~P2pCredP2pAclImportAuthType() {};
};

class NegotiateProcess {
DM_DECLARE_SINGLE_INSTANCE_BASE(NegotiateProcess);
public:
    NegotiateProcess();
    ~NegotiateProcess();
    int32_t HandleNegotiateResult(std::shared_ptr<DmAuthContext> context);
    int32_t HandleProxyNegotiateResult(std::shared_ptr<DmAuthContext> context, int32_t result);
    int32_t HandleServiceNegotiateResult(std::shared_ptr<DmAuthContext> context, int32_t result);
    bool IsNeedSetProxyRelationShip(std::shared_ptr<DmAuthContext> context, DmProxyAuthContext &proxyContext);
    bool IsExistTheTokenId(const std::string extraData, const std::string tokenIdHash);
private:
    CredType ConvertCredType(const std::string &credType);
    AclType ConvertAclType(const std::string &aclType);
    AuthType ConvertAuthType(const DmAuthType &authType);
private:
    std::map<NegotiateSpec, std::unique_ptr<NegotiateHandler>> handlers_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_STATE_V2_H
