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
#include <fuzzer/FuzzedDataProvider.h>
#include <vector>

#include "auth_confirm_fuzzer.h"

#include "auth_manager.h"
#include "deviceprofile_connector.h"
#include "device_manager_service_listener.h"
#include "dm_anonymous.h"
#include "dm_auth_context.h"
#include "dm_auth_manager_base.h"
#include "dm_auth_message_processor.h"
#include "dm_auth_state_machine.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"


namespace OHOS {
namespace DistributedHardware {

class FuzzTestContext {
public:
    FuzzTestContext()
    {
        softbusConnector = std::make_shared<SoftbusConnector>();
        listener = std::make_shared<DeviceManagerServiceListener>();
        hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
        hiChainConnector = std::make_shared<HiChainConnector>();
        authManager = std::make_shared<AuthSrcManager>(
            softbusConnector, hiChainConnector, listener, hiChainAuthConnector);
        context_ = authManager->GetAuthContext();

        // 初始化状态机相关对象
        authSrcConfirmState_ = std::make_shared<AuthSrcConfirmState>();
        authSinkConfirmState_ = std::make_shared<AuthSinkConfirmState>();
        authSinkStatePinAuthComm_ = std::make_shared<AuthSinkStatePinAuthComm>();
        authSrcPinAuthStartState_ = std::make_shared<AuthSrcPinAuthStartState>();
        authSinkPinAuthStartState_ = std::make_shared<AuthSinkPinAuthStartState>();
        authSrcPinAuthMsgNegotiateState_ = std::make_shared<AuthSrcPinAuthMsgNegotiateState>();
        authSinkPinAuthMsgNegotiateState_ = std::make_shared<AuthSinkPinAuthMsgNegotiateState>();
        authSinkPinAuthDoneState_ = std::make_shared<AuthSinkPinAuthDoneState>();
        authSrcPinAuthDoneState_ = std::make_shared<AuthSrcPinAuthDoneState>();
        authSrcPinNegotiateStartState_ = std::make_shared<AuthSrcPinNegotiateStartState>();
        authSrcPinInputState_ = std::make_shared<AuthSrcPinInputState>();
        authSinkPinNegotiateStartState_ = std::make_shared<AuthSinkPinNegotiateStartState>();
        authSinkPinDisplayState_ = std::make_shared<AuthSinkPinDisplayState>();
        authSrcReverseUltrasonicStartState_ = std::make_shared<AuthSrcReverseUltrasonicStartState>();
        authSrcReverseUltrasonicDoneState_ = std::make_shared<AuthSrcReverseUltrasonicDoneState>();
        authSrcForwardUltrasonicStartState_ = std::make_shared<AuthSrcForwardUltrasonicStartState>();
        authSrcForwardUltrasonicDoneState_ = std::make_shared<AuthSrcForwardUltrasonicDoneState>();
        authSinkReverseUltrasonicStartState_ = std::make_shared<AuthSinkReverseUltrasonicStartState>();
        authSinkReverseUltrasonicDoneState_ = std::make_shared<AuthSinkReverseUltrasonicDoneState>();
        authSinkForwardUltrasonicStartState_ = std::make_shared<AuthSinkForwardUltrasonicStartState>();
        authSinkForwardUltrasonicDoneState_ = std::make_shared<AuthSinkForwardUltrasonicDoneState>();
        authSrcCredentialAuthNegotiateState_ = std::make_shared<AuthSrcCredentialAuthNegotiateState>();
        authSrcCredentialAuthDoneState_ = std::make_shared<AuthSrcCredentialAuthDoneState>();
        authSinkCredentialAuthStartState_ = std::make_shared<AuthSinkCredentialAuthStartState>();
        authSinkCredentialAuthNegotiateState_ = std::make_shared<AuthSinkCredentialAuthNegotiateState>();
        authSrcCredentialExchangeState_ = std::make_shared<AuthSrcCredentialExchangeState>();
        authSinkCredentialExchangeState_ = std::make_shared<AuthSinkCredentialExchangeState>();
        authSrcCredentialAuthStartState_ = std::make_shared<AuthSrcCredentialAuthStartState>();
        authSrcStartState_ = std::make_shared<AuthSrcStartState>();
        authSrcNegotiateStateMachine_ = std::make_shared<AuthSrcNegotiateStateMachine>();
        authSinkNegotiateStateMachine_ = std::make_shared<AuthSinkNegotiateStateMachine>();
    }

    ~FuzzTestContext()
    {
        authSrcNegotiateStateMachine_.reset();
        authSinkNegotiateStateMachine_.reset();

        authSrcConfirmState_.reset();
        authSinkConfirmState_.reset();
        authSinkStatePinAuthComm_.reset();
        authSrcPinAuthStartState_.reset();
        authSinkPinAuthStartState_.reset();
        authSrcPinAuthMsgNegotiateState_.reset();
        authSinkPinAuthMsgNegotiateState_.reset();
        authSinkPinAuthDoneState_.reset();
        authSrcPinAuthDoneState_.reset();
        authSrcPinNegotiateStartState_.reset();
        authSrcPinInputState_.reset();
        authSinkPinNegotiateStartState_.reset();
        authSinkPinDisplayState_.reset();
        authSrcReverseUltrasonicStartState_.reset();
        authSrcReverseUltrasonicDoneState_.reset();
        authSrcForwardUltrasonicStartState_.reset();
        authSrcForwardUltrasonicDoneState_.reset();
        authSinkReverseUltrasonicStartState_.reset();
        authSinkReverseUltrasonicDoneState_.reset();
        authSinkForwardUltrasonicStartState_.reset();
        authSinkForwardUltrasonicDoneState_.reset();
        authSrcCredentialAuthNegotiateState_.reset();
        authSrcCredentialAuthDoneState_.reset();
        authSinkCredentialAuthStartState_.reset();
        authSinkCredentialAuthNegotiateState_.reset();
        authSrcCredentialExchangeState_.reset();
        authSinkCredentialExchangeState_.reset();
        authSrcCredentialAuthStartState_.reset();
        authSrcStartState_.reset();
        context_.reset();
        authManager.reset();
    }

public:
    std::shared_ptr<SoftbusConnector> softbusConnector;
    std::shared_ptr<IDeviceManagerServiceListener> listener;
    std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector;
    std::shared_ptr<HiChainConnector> hiChainConnector;
    std::shared_ptr<AuthManager> authManager;
    std::shared_ptr<DmAuthContext> context_;

    std::shared_ptr<AuthSrcConfirmState> authSrcConfirmState_;
    std::shared_ptr<AuthSinkConfirmState> authSinkConfirmState_;
    std::shared_ptr<AuthSinkStatePinAuthComm> authSinkStatePinAuthComm_;
    std::shared_ptr<AuthSrcPinAuthStartState> authSrcPinAuthStartState_;
    std::shared_ptr<AuthSinkPinAuthStartState> authSinkPinAuthStartState_;
    std::shared_ptr<AuthSrcPinAuthMsgNegotiateState> authSrcPinAuthMsgNegotiateState_;
    std::shared_ptr<AuthSinkPinAuthMsgNegotiateState> authSinkPinAuthMsgNegotiateState_;
    std::shared_ptr<AuthSinkPinAuthDoneState> authSinkPinAuthDoneState_;
    std::shared_ptr<AuthSrcPinAuthDoneState> authSrcPinAuthDoneState_;
    std::shared_ptr<AuthSrcPinNegotiateStartState> authSrcPinNegotiateStartState_;
    std::shared_ptr<AuthSrcPinInputState> authSrcPinInputState_;
    std::shared_ptr<AuthSinkPinNegotiateStartState> authSinkPinNegotiateStartState_;
    std::shared_ptr<AuthSinkPinDisplayState> authSinkPinDisplayState_;
    std::shared_ptr<AuthSrcReverseUltrasonicStartState> authSrcReverseUltrasonicStartState_;
    std::shared_ptr<AuthSrcReverseUltrasonicDoneState> authSrcReverseUltrasonicDoneState_;
    std::shared_ptr<AuthSrcForwardUltrasonicStartState> authSrcForwardUltrasonicStartState_;
    std::shared_ptr<AuthSrcForwardUltrasonicDoneState> authSrcForwardUltrasonicDoneState_;
    std::shared_ptr<AuthSinkReverseUltrasonicStartState> authSinkReverseUltrasonicStartState_;
    std::shared_ptr<AuthSinkReverseUltrasonicDoneState> authSinkReverseUltrasonicDoneState_;
    std::shared_ptr<AuthSinkForwardUltrasonicStartState> authSinkForwardUltrasonicStartState_;
    std::shared_ptr<AuthSinkForwardUltrasonicDoneState> authSinkForwardUltrasonicDoneState_;
    std::shared_ptr<AuthSrcCredentialAuthNegotiateState> authSrcCredentialAuthNegotiateState_;
    std::shared_ptr<AuthSrcCredentialAuthDoneState> authSrcCredentialAuthDoneState_;
    std::shared_ptr<AuthSinkCredentialAuthStartState> authSinkCredentialAuthStartState_;
    std::shared_ptr<AuthSinkCredentialAuthNegotiateState> authSinkCredentialAuthNegotiateState_;
    std::shared_ptr<AuthSrcCredentialExchangeState> authSrcCredentialExchangeState_;
    std::shared_ptr<AuthSinkCredentialExchangeState> authSinkCredentialExchangeState_;
    std::shared_ptr<AuthSrcCredentialAuthStartState> authSrcCredentialAuthStartState_;
    std::shared_ptr<AuthSrcStartState> authSrcStartState_;
    std::shared_ptr<AuthSrcNegotiateStateMachine> authSrcNegotiateStateMachine_;
    std::shared_ptr<AuthSinkNegotiateStateMachine> authSinkNegotiateStateMachine_;
};


void GenerateStrings(std::vector<std::string> &strings, FuzzedDataProvider &fdp)
{
    size_t vectorSize = fdp.ConsumeIntegralInRange<size_t>(2, 10);
    
    for (size_t i = 0; i < vectorSize; ++i) {
        size_t strLen = fdp.ConsumeIntegralInRange<size_t>(0, 50);
        strings.push_back(fdp.ConsumeBytesAsString(strLen));
    }
}

void GenerateJsonObject(JsonObject &jsonObject, FuzzedDataProvider &fdp)
{
    jsonObject[TAG_DATA] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_PEER_PKG_NAME] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_DM_VERSION_V2] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_USER_ID] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_DEVICE_ID_HASH] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_ACCOUNT_ID_HASH] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_TOKEN_ID_HASH] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_BUNDLE_NAME_V2] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_EXTRA_INFO] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_PEER_BUNDLE_NAME_V2] = fdp.ConsumeRandomLengthString();
    jsonObject[DM_TAG_LOGICAL_SESSION_ID] = fdp.ConsumeIntegral<uint64_t>();
    jsonObject[TAG_PEER_DISPLAY_ID] = fdp.ConsumeIntegral<int32_t>();
    jsonObject[DM_BUSINESS_ID] = fdp.ConsumeRandomLengthString();
}

void ActionFuzzTest(FuzzTestContext &ctx)
{
    ctx.authSrcPinAuthStartState_->Action(ctx.context_);
    ctx.authSinkPinAuthStartState_->Action(ctx.context_);
    ctx.authSrcPinAuthMsgNegotiateState_->Action(ctx.context_);
    ctx.authSinkPinAuthMsgNegotiateState_->Action(ctx.context_);
    ctx.authSinkPinAuthDoneState_->Action(ctx.context_);
    ctx.authSrcPinAuthDoneState_->Action(ctx.context_);
    ctx.authSrcPinNegotiateStartState_->Action(ctx.context_);
    ctx.authSinkPinNegotiateStartState_->Action(ctx.context_);
    ctx.authSinkPinDisplayState_->Action(ctx.context_);
    ctx.authSrcReverseUltrasonicStartState_->Action(ctx.context_);
    ctx.authSrcReverseUltrasonicDoneState_->Action(ctx.context_);
    ctx.authSrcForwardUltrasonicStartState_->Action(ctx.context_);
    ctx.authSrcForwardUltrasonicDoneState_->Action(ctx.context_);
    ctx.authSinkReverseUltrasonicStartState_->Action(ctx.context_);
    ctx.authSinkReverseUltrasonicDoneState_->Action(ctx.context_);
    ctx.authSinkForwardUltrasonicStartState_->Action(ctx.context_);
    ctx.authSinkForwardUltrasonicDoneState_->Action(ctx.context_);
    ctx.authSrcCredentialAuthNegotiateState_->Action(ctx.context_);
    ctx.authSrcCredentialAuthDoneState_->Action(ctx.context_);
    ctx.authSinkCredentialAuthStartState_->Action(ctx.context_);
    ctx.authSinkCredentialAuthNegotiateState_->Action(ctx.context_);
    ctx.authSrcCredentialExchangeState_->Action(ctx.context_);
    ctx.authSinkCredentialExchangeState_->Action(ctx.context_);
    ctx.authSrcCredentialAuthStartState_->Action(ctx.context_);
    ctx.authSrcConfirmState_->Action(ctx.context_);
    ctx.authSinkConfirmState_->Action(ctx.context_);
    ctx.authSrcStartState_->Action(ctx.context_);
    ctx.authSrcNegotiateStateMachine_->Action(ctx.context_);
    ctx.authSinkNegotiateStateMachine_->Action(ctx.context_);
}

void AuthConfirmFuzzTestNext(FuzzTestContext &ctx, JsonObject &jsonObject, FuzzedDataProvider &fdp)
{
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserExtraData(fdp.ConsumeRandomLengthString());
    accessee.SetAccesseeExtraData(fdp.ConsumeRandomLengthString());
    ctx.authSrcConfirmState_->NegotiateCredential(ctx.context_, jsonObject);
    ctx.authSrcConfirmState_->NegotiateAcl(ctx.context_, jsonObject);
    ctx.authSrcConfirmState_->GetIdenticalCredentialInfo(ctx.context_, jsonObject);
    ctx.authSrcConfirmState_->GetShareCredentialInfo(ctx.context_, jsonObject);
    ctx.authSrcConfirmState_->GetP2PCredentialInfo(ctx.context_, jsonObject);
    ctx.authSinkConfirmState_->CreateProxyData(ctx.context_, jsonObject);
    ctx.authSinkConfirmState_->NegotiateCredential(ctx.context_, jsonObject);
    ctx.authSinkConfirmState_->NegotiateAcl(ctx.context_, jsonObject);
    ctx.authSinkNegotiateStateMachine_->GetIdenticalCredentialInfo(ctx.context_, jsonObject);
    ctx.authSinkNegotiateStateMachine_->GetShareCredentialInfo(ctx.context_, jsonObject);
    ctx.authSinkNegotiateStateMachine_->GetP2PCredentialInfo(ctx.context_, jsonObject);
    ctx.authSrcConfirmState_->GetCustomDescBySinkLanguage(ctx.context_);
    ctx.authSinkConfirmState_->ShowConfigDialog(ctx.context_);
    ctx.authSinkConfirmState_->ReadServiceInfo(ctx.context_);
    ctx.authSinkConfirmState_->ProcessBindAuthorize(ctx.context_);
    ctx.authSinkConfirmState_->ProcessNoBindAuthorize(ctx.context_);
    ctx.authSinkStatePinAuthComm_->IsAuthCodeReady(ctx.context_);
    ctx.authSinkStatePinAuthComm_->GeneratePincode(ctx.context_);
    ctx.authSinkStatePinAuthComm_->ShowAuthInfoDialog(ctx.context_);
    ctx.authSinkNegotiateStateMachine_->RespQueryAcceseeIds(ctx.context_);
    ctx.authSinkNegotiateStateMachine_->ProcRespNegotiate5_1_0(ctx.context_);
    ctx.authSrcConfirmState_->IdenticalAccountAclCompare(ctx.context_, accesser, accessee);
    ctx.authSrcConfirmState_->ShareAclCompare(ctx.context_, accesser, accessee);
    ctx.authSrcConfirmState_->Point2PointAclCompare(ctx.context_, accesser, accessee);
    ctx.authSrcConfirmState_->LnnAclCompare(ctx.context_, accesser, accessee);
    ctx.authSinkNegotiateStateMachine_->IdenticalAccountAclCompare(ctx.context_, accesser, accessee);
    ctx.authSinkNegotiateStateMachine_->ShareAclCompare(ctx.context_, accesser, accessee);
    ctx.authSinkNegotiateStateMachine_->Point2PointAclCompare(ctx.context_, accesser, accessee);
    ctx.authSinkNegotiateStateMachine_->LnnAclCompare(ctx.context_, accesser, accessee);
    ctx.authSrcPinInputState_->ShowStartAuthDialog(ctx.context_);
    ctx.authSrcPinInputState_->Action(ctx.context_);
    ctx.context_->importAuthCode = "123456";
    ctx.context_->importPkgName = "pkgName";
    ctx.authSinkStatePinAuthComm_->IsAuthCodeReady(ctx.context_);
    ctx.authSinkStatePinAuthComm_->IsPinCodeValid(ctx.context_->importAuthCode);
}

void AuthConfirmFuzzTestThird(FuzzTestContext &ctx, FuzzedDataProvider &fdp)
{
    std::string businessId = fdp.ConsumeRandomLengthString();
    std::string businessValue = fdp.ConsumeRandomLengthString();
    std::string authorizeInfo = fdp.ConsumeRandomLengthString();
    std::vector<std::string> deleteCredInfo;
    DistributedDeviceProfile::AccessControlProfile acl;
    ctx.authSinkNegotiateStateMachine_->IsAntiDisturbanceMode(businessId);
    ctx.authSinkNegotiateStateMachine_->IsAntiDisturbanceMode("");
    ctx.authSinkNegotiateStateMachine_->ParseAndCheckAntiDisturbanceMode(businessId, businessValue);
    businessId = "test_business_id";
    businessValue = "{\"business_id\":\"test_business_id\",\"is_in_anti_disturbance_mode\":true}";
    ctx.authSinkNegotiateStateMachine_->ParseAndCheckAntiDisturbanceMode(businessId, businessValue);
    ctx.authSrcNegotiateStateMachine_->GetStateType();
    ctx.authSrcPinAuthStartState_->GetStateType();
    ctx.authSinkPinAuthDoneState_->GetStateType();
    ctx.authSrcReverseUltrasonicStartState_->GetStateType();
    ctx.authSrcForwardUltrasonicStartState_->GetStateType();
    ctx.authSinkReverseUltrasonicStartState_->GetStateType();
    ctx.authSinkForwardUltrasonicDoneState_->GetStateType();
    ctx.authSrcPinNegotiateStartState_->ProcessCredAuth(ctx.context_);
    int32_t credType = fdp.ConsumeIntegral<int32_t>();
    ctx.authSrcPinNegotiateStartState_->GetCredIdByCredType(ctx.context_, credType);
    ctx.context_->IsProxyBind = true;
    ctx.authSrcConfirmState_->NegotiateProxyCredential(ctx.context_);
    ctx.authSrcConfirmState_->NegotiateProxyAcl(ctx.context_);
    ctx.authSrcConfirmState_->ResetBindLevel(ctx.context_);
    ctx.authSinkConfirmState_->GetBundleLabel(ctx.context_);
    ctx.authSinkConfirmState_->NegotiateProxyAcl(ctx.context_);
    ctx.authSinkConfirmState_->ProcessUserAuthorize(ctx.context_);
    ctx.authSinkConfirmState_->ProcessServerAuthorize(ctx.context_);
    ctx.authSinkConfirmState_->NegotiateProxyCredential(ctx.context_);
    ctx.authSrcConfirmState_->GetSrcProxyCredTypeForP2P(ctx.context_, deleteCredInfo);
    ctx.authSinkConfirmState_->ProcessUserOption(ctx.context_, authorizeInfo);
    ctx.authSrcConfirmState_->GetSrcProxyAclInfoForP2P(ctx.context_, acl);
}

void SetupContextAndGenerateData(FuzzTestContext &ctx, FuzzedDataProvider &fdp, JsonObject &jsonObject,
    JsonObject &jsonObjectTwo, JsonObject &jsonObjectThree)
{
    uint32_t bindType = fdp.ConsumeIntegralInRange<uint32_t>(0, 1024);
    int32_t credType = fdp.ConsumeIntegral<int32_t>();
    int32_t numpin = fdp.ConsumeIntegral<int32_t>();
    std::string credId = fdp.ConsumeRandomLengthString();
    std::string name = fdp.ConsumeRandomLengthString();
    std::string strpin = fdp.ConsumeRandomLengthString();
    bool checkResult = fdp.ConsumeBool();
    DistributedDeviceProfile::AccessControlProfile acl;
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    acl.SetExtraData(fdp.ConsumeRandomLengthString());
    accesser.SetAccesserExtraData(fdp.ConsumeRandomLengthString());
    accessee.SetAccesseeExtraData(fdp.ConsumeRandomLengthString());
    std::vector<std::string> deleteCredInfo;
    GenerateStrings(deleteCredInfo, fdp);
    ctx.context_->extraInfo = fdp.ConsumeRandomLengthString();
    ctx.authSrcConfirmState_->GetSrcAclInfoForP2P(ctx.context_, acl, jsonObject, jsonObjectTwo);
    ctx.authSrcConfirmState_->CheckCredIdInAcl(ctx.context_, acl, jsonObject, bindType);
    ctx.authSrcConfirmState_->CheckCredIdInAclForP2P(ctx.context_, credId, acl, jsonObjectTwo, bindType, checkResult);
    ctx.authSrcConfirmState_->GetSrcCredType(ctx.context_, jsonObject, jsonObjectTwo, jsonObjectThree);
    ctx.authSrcConfirmState_->GetSrcCredTypeForP2P(ctx.context_, jsonObject, jsonObjectTwo, jsonObjectThree, credType,
        deleteCredInfo);
    ctx.authSinkConfirmState_->MatchFallBackCandidateList(ctx.context_, DmAuthType::AUTH_TYPE_PIN);
    ctx.authSinkConfirmState_->GetCredIdByCredType(ctx.context_, credType);
    ctx.authSinkStatePinAuthComm_->HandleSessionHeartbeat(ctx.context_, name);
    ctx.authSinkStatePinAuthComm_->IsPinCodeValid(numpin);
    ctx.authSinkStatePinAuthComm_->IsPinCodeValid(strpin);
    ctx.authSinkNegotiateStateMachine_->GetSinkAclInfo(ctx.context_, jsonObject, jsonObjectTwo);
    ctx.authSinkNegotiateStateMachine_->GetSinkAclInfoForP2P(ctx.context_, acl, jsonObject, jsonObjectTwo);
    ctx.authSinkNegotiateStateMachine_->CheckCredIdInAcl(ctx.context_, acl, jsonObject, bindType);
    ctx.authSinkNegotiateStateMachine_->CheckCredIdInAclForP2P(ctx.context_, credId, acl, jsonObject, bindType,
        checkResult);
    ctx.authSinkNegotiateStateMachine_->GetSinkCredType(ctx.context_, jsonObject, jsonObjectTwo, jsonObjectThree);
    ctx.authSinkNegotiateStateMachine_->GetSinkCredTypeForP2P(ctx.context_, jsonObject, jsonObjectTwo, jsonObjectThree,
        credType, deleteCredInfo);
    ctx.authSrcConfirmState_->GetSrcAclInfo(ctx.context_, jsonObject, jsonObjectTwo);
}

void AuthConfirmFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    FuzzTestContext ctx;
    FuzzedDataProvider fdp(data, size);

    JsonObject jsonObject;
    JsonObject jsonObjectTwo;
    JsonObject jsonObjectThree;
    GenerateJsonObject(jsonObject, fdp);
    GenerateJsonObject(jsonObjectTwo, fdp);
    GenerateJsonObject(jsonObjectThree, fdp);

    SetupContextAndGenerateData(ctx, fdp, jsonObject, jsonObjectTwo, jsonObjectThree);

    ActionFuzzTest(ctx);
    AuthConfirmFuzzTestNext(ctx, jsonObject, fdp);
    AuthConfirmFuzzTestThird(ctx, fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthConfirmFuzzTest(data, size);

    return 0;
}
