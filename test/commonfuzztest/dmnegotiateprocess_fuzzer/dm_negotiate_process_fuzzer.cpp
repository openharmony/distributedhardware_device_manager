/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dm_negotiate_process_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "device_manager_service_listener.h"
#include "dm_auth_state.h"
#include "dm_negotiate_process.h"

namespace OHOS {
namespace DistributedHardware {

void RespQueryProxyAcceseeIdsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->IsProxyBind = true;
    context->accessee.userId = fdp.ConsumeIntegral<int32_t>();
    DmProxyAuthContext dmProxyAuthContext;
    dmProxyAuthContext.proxyAccessee.bundleName = fdp.ConsumeRandomLengthString();
    dmProxyAuthContext.proxyAccessee.tokenId = fdp.ConsumeIntegral<int64_t>();
    context->subjectProxyOnes.push_back(dmProxyAuthContext);
    std::shared_ptr<AuthSinkNegotiateStateMachine> authSinkPtr = std::make_shared<AuthSinkNegotiateStateMachine>();
    authSinkPtr->RespQueryProxyAcceseeIds(context);
}

void GetSinkProxyCredTypeForP2PFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    context->IsProxyBind = true;
    context->accessee.userId = fdp.ConsumeIntegral<int32_t>();
    DmProxyAuthContext dmProxyAuthContext;
    dmProxyAuthContext.proxyAccessee.bundleName = fdp.ConsumeRandomLengthString();
    dmProxyAuthContext.proxyAccessee.tokenId = fdp.ConsumeIntegral<int64_t>();
    dmProxyAuthContext.proxyAccessee.credInfoJson = fdp.ConsumeRandomLengthString();
    dmProxyAuthContext.proxyAccessee.aclTypeList = fdp.ConsumeRandomLengthString();
    dmProxyAuthContext.proxyAccessee.credTypeList = fdp.ConsumeRandomLengthString();
    context->subjectProxyOnes.push_back(dmProxyAuthContext);
    std::shared_ptr<AuthSinkNegotiateStateMachine> authSinkPtr = std::make_shared<AuthSinkNegotiateStateMachine>();
    std::vector<std::string> deleteCredInfo;
    authSinkPtr->GetSinkProxyCredTypeForP2P(context, deleteCredInfo);
    DistributedDeviceProfile::AccessControlProfile profile;
    authSinkPtr->GetSinkProxyAclInfoForP2P(context, profile);
}

void DmNegotiateProcessFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    std::shared_ptr<DmAuthContext> context = std::make_shared<DmAuthContext>();
    NoCredNoAclImportAuthType noCredNoAclImportAuthType;
    noCredNoAclImportAuthType.NegotiateHandle(context);
    NoCredNoAclInputAuthType noCredNoAclInputAuthType;
    noCredNoAclInputAuthType.NegotiateHandle(context);
    IdentCredNoAclImportAuthType identCredNoAclImportAuthType;
    identCredNoAclImportAuthType.NegotiateHandle(context);
    IdentCredNoAclInputAuthType identCredNoAclInputAuthType;
    identCredNoAclInputAuthType.NegotiateHandle(context);
    IdentCredIdentAclImportAuthType identCredIdentAclImportAuthType;
    identCredIdentAclImportAuthType.NegotiateHandle(context);
    IdentCredIdentAclInputAuthType identCredIdentAclInputAuthType;
    identCredIdentAclInputAuthType.NegotiateHandle(context);
    IdentCredP2pAclImportAuthType identCredP2pAclImportAuthType;
    identCredP2pAclImportAuthType.NegotiateHandle(context);
    IdentCredP2pAclInputAuthType identCredP2pAclInputAuthType;
    identCredP2pAclInputAuthType.NegotiateHandle(context);
    ShareCredNoAclImportAuthType shareCredNoAclImportAuthType;
    shareCredNoAclImportAuthType.NegotiateHandle(context);
    ShareCredNoAclInputAuthType shareCredNoAclInputAuthType;
    shareCredNoAclInputAuthType.NegotiateHandle(context);
    ShareCredShareAclImportAuthType shareCredShareAclImportAuthType;
    shareCredShareAclImportAuthType.NegotiateHandle(context);
    ShareCredShareAclInputAuthType shareCredShareAclInputAuthType;
    shareCredShareAclInputAuthType.NegotiateHandle(context);
    ShareCredP2pAclImportAuthType shareCredP2pAclImportAuthType;
    shareCredP2pAclImportAuthType.NegotiateHandle(context);
    ShareCredP2pAclInputAuthType shareCredP2pAclInputAuthType;
    shareCredP2pAclInputAuthType.NegotiateHandle(context);
    P2pCredNoAclImportAuthType p2pCredNoAclImportAuthType;
    p2pCredNoAclImportAuthType.NegotiateHandle(context);
    P2pCredNoAclInputAuthType p2pCredNoAclInputAuthType;
    p2pCredNoAclInputAuthType.NegotiateHandle(context);
    P2pCredP2pAclImportAuthType p2pCredP2pAclImportAuthType;
    p2pCredP2pAclImportAuthType.NegotiateHandle(context);
    P2pCredP2pAclInputAuthType p2pCredP2pAclInputAuthType;
    p2pCredP2pAclInputAuthType.NegotiateHandle(context);

    RespQueryProxyAcceseeIdsFuzzTest(data, size);
    GetSinkProxyCredTypeForP2PFuzzTest(data, size);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmNegotiateProcessFuzzTest(data, size);
    return 0;
}