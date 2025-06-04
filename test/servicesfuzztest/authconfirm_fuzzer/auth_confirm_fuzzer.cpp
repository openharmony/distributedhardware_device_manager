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
namespace {
std::shared_ptr<AuthSrcConfirmState> authSrcConfirmState_ = std::make_shared<AuthSrcConfirmState>();
std::shared_ptr<AuthSinkConfirmState> authSinkConfirmState_ = std::make_shared<AuthSinkConfirmState>();
std::shared_ptr<AuthSinkStatePinAuthComm> authSinkStatePinAuthComm_ = std::make_shared<AuthSinkStatePinAuthComm>();
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
std::shared_ptr<AuthManager> authManager = std::make_shared<AuthSrcManager>(softbusConnector, hiChainConnector,
    listener, hiChainAuthConnector);
std::shared_ptr<DmAuthContext> context_ = authManager->GetAuthContext();
std::shared_ptr<AuthSrcPinAuthStartState> authSrcPinAuthStartState_ = std::make_shared<AuthSrcPinAuthStartState>();
std::shared_ptr<AuthSinkPinAuthStartState> authSinkPinAuthStartState_ = std::make_shared<AuthSinkPinAuthStartState>();
std::shared_ptr<AuthSrcPinAuthMsgNegotiateState> authSrcPinAuthMsgNegotiateState_ =
    std::make_shared<AuthSrcPinAuthMsgNegotiateState>();
std::shared_ptr<AuthSinkPinAuthMsgNegotiateState> authSinkPinAuthMsgNegotiateState_ =
    std::make_shared<AuthSinkPinAuthMsgNegotiateState>();
std::shared_ptr<AuthSinkPinAuthDoneState> authSinkPinAuthDoneState_ = std::make_shared<AuthSinkPinAuthDoneState>();
std::shared_ptr<AuthSrcPinAuthDoneState> authSrcPinAuthDoneState_ = std::make_shared<AuthSrcPinAuthDoneState>();
std::shared_ptr<AuthSrcPinNegotiateStartState> authSrcPinNegotiateStartState_ =
    std::make_shared<AuthSrcPinNegotiateStartState>();
std::shared_ptr<AuthSrcPinInputState> authSrcPinInputState_ = std::make_shared<AuthSrcPinInputState>();
std::shared_ptr<AuthSinkPinNegotiateStartState> authSinkPinNegotiateStartState_ =
    std::make_shared<AuthSinkPinNegotiateStartState>();
std::shared_ptr<AuthSinkPinDisplayState> authSinkPinDisplayState_ = std::make_shared<AuthSinkPinDisplayState>();
std::shared_ptr<AuthSrcReverseUltrasonicStartState> authSrcReverseUltrasonicStartState_ =
    std::make_shared<AuthSrcReverseUltrasonicStartState>();
std::shared_ptr<AuthSrcReverseUltrasonicDoneState> authSrcReverseUltrasonicDoneState_ =
    std::make_shared<AuthSrcReverseUltrasonicDoneState>();
std::shared_ptr<AuthSrcForwardUltrasonicStartState> authSrcForwardUltrasonicStartState_ =
    std::make_shared<AuthSrcForwardUltrasonicStartState>();
std::shared_ptr<AuthSrcForwardUltrasonicDoneState> authSrcForwardUltrasonicDoneState_ =
    std::make_shared<AuthSrcForwardUltrasonicDoneState>();
std::shared_ptr<AuthSinkReverseUltrasonicStartState> authSinkReverseUltrasonicStartState_ =
    std::make_shared<AuthSinkReverseUltrasonicStartState>();
std::shared_ptr<AuthSinkReverseUltrasonicDoneState> authSinkReverseUltrasonicDoneState_ =
    std::make_shared<AuthSinkReverseUltrasonicDoneState>();
std::shared_ptr<AuthSinkForwardUltrasonicStartState> authSinkForwardUltrasonicStartState_ =
    std::make_shared<AuthSinkForwardUltrasonicStartState>();
std::shared_ptr<AuthSinkForwardUltrasonicDoneState> authSinkForwardUltrasonicDoneState_ =
    std::make_shared<AuthSinkForwardUltrasonicDoneState>();
std::shared_ptr<AuthSrcCredentialAuthNegotiateState> authSrcCredentialAuthNegotiateState_ =
    std::make_shared<AuthSrcCredentialAuthNegotiateState>();
std::shared_ptr<AuthSrcCredentialAuthDoneState> authSrcCredentialAuthDoneState_ =
    std::make_shared<AuthSrcCredentialAuthDoneState>();
std::shared_ptr<AuthSinkCredentialAuthStartState> authSinkCredentialAuthStartState_ =
    std::make_shared<AuthSinkCredentialAuthStartState>();
std::shared_ptr<AuthSinkCredentialAuthNegotiateState> authSinkCredentialAuthNegotiateState_ =
    std::make_shared<AuthSinkCredentialAuthNegotiateState>();
std::shared_ptr<AuthSrcCredentialExchangeState> authSrcCredentialExchangeState_ =
    std::make_shared<AuthSrcCredentialExchangeState>();
std::shared_ptr<AuthSinkCredentialExchangeState> authSinkCredentialExchangeState_ =
    std::make_shared<AuthSinkCredentialExchangeState>();
std::shared_ptr<AuthSrcCredentialAuthStartState> authSrcCredentialAuthStartState_ =
    std::make_shared<AuthSrcCredentialAuthStartState>();
std::shared_ptr<AuthSrcStartState> authSrcStartState_ = std::make_shared<AuthSrcStartState>();
std::shared_ptr<AuthSrcNegotiateStateMachine> authSrcNegotiateStateMachine_ =
    std::make_shared<AuthSrcNegotiateStateMachine>();
std::shared_ptr<AuthSinkNegotiateStateMachine> authSinkNegotiateStateMachine_ =
    std::make_shared<AuthSinkNegotiateStateMachine>();
}

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
}

void ActionFuzzTest()
{
    authSrcPinAuthStartState_ ->Action(context_);
    authSinkPinAuthStartState_ ->Action(context_);
    authSrcPinAuthMsgNegotiateState_ ->Action(context_);
    authSinkPinAuthMsgNegotiateState_ ->Action(context_);
    authSinkPinAuthDoneState_ ->Action(context_);
    authSrcPinAuthDoneState_ ->Action(context_);
    authSrcPinNegotiateStartState_ ->Action(context_);
    authSrcPinInputState_ ->Action(context_);
    authSinkPinNegotiateStartState_ ->Action(context_);
    authSinkPinDisplayState_ ->Action(context_);
    authSrcReverseUltrasonicStartState_ ->Action(context_);
    authSrcReverseUltrasonicDoneState_ ->Action(context_);
    authSrcForwardUltrasonicStartState_ ->Action(context_);
    authSrcForwardUltrasonicDoneState_ ->Action(context_);
    authSinkReverseUltrasonicStartState_ ->Action(context_);
    authSinkReverseUltrasonicDoneState_ ->Action(context_);
    authSinkForwardUltrasonicStartState_ ->Action(context_);
    authSinkForwardUltrasonicDoneState_ ->Action(context_);
    authSrcCredentialAuthNegotiateState_ ->Action(context_);
    authSrcCredentialAuthDoneState_ ->Action(context_);
    authSinkCredentialAuthStartState_ ->Action(context_);
    authSinkCredentialAuthNegotiateState_ ->Action(context_);
    authSrcCredentialExchangeState_ ->Action(context_);
    authSinkCredentialExchangeState_ ->Action(context_);
    authSrcCredentialAuthStartState_ ->Action(context_);
    authSrcConfirmState_ -> Action(context_);
    authSinkConfirmState_ -> Action(context_);
    authSrcStartState_ -> Action(context_);
    authSrcNegotiateStateMachine_ -> Action(context_);
    authSinkNegotiateStateMachine_ -> Action(context_);
}

void AuthConfirmFuzzTestNext(JsonObject &jsonObject, FuzzedDataProvider &fdp)
{
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserExtraData(fdp.ConsumeRandomLengthString());
    accessee.SetAccesseeExtraData(fdp.ConsumeRandomLengthString());
    authSrcConfirmState_ -> NegotiateCredential(context_, jsonObject);
    authSrcConfirmState_ -> NegotiateAcl(context_, jsonObject);
    authSrcConfirmState_ -> GetIdenticalCredentialInfo(context_, jsonObject);
    authSrcConfirmState_ -> GetShareCredentialInfo(context_, jsonObject);
    authSrcConfirmState_ -> GetP2PCredentialInfo(context_, jsonObject);
    authSinkConfirmState_ -> NegotiateCredential(context_, jsonObject);
    authSinkConfirmState_ -> NegotiateAcl(context_, jsonObject);
    authSinkNegotiateStateMachine_ -> GetIdenticalCredentialInfo(context_, jsonObject);
    authSinkNegotiateStateMachine_ -> GetShareCredentialInfo(context_, jsonObject);
    authSinkNegotiateStateMachine_ -> GetP2PCredentialInfo(context_, jsonObject);
    authSrcConfirmState_ -> GetCustomDescBySinkLanguage(context_);
    authSinkConfirmState_ -> ShowConfigDialog(context_);
    authSinkConfirmState_ -> ReadServiceInfo(context_);
    authSinkConfirmState_ -> ProcessBindAuthorize(context_);
    authSinkConfirmState_ -> ProcessNoBindAuthorize(context_);
    authSinkStatePinAuthComm_ -> IsAuthCodeReady(context_);
    authSinkStatePinAuthComm_ -> GeneratePincode(context_);
    authSinkStatePinAuthComm_ -> ShowAuthInfoDialog(context_);
    authSinkNegotiateStateMachine_ -> RespQueryAcceseeIds(context_);
    authSinkNegotiateStateMachine_ -> ProcRespNegotiate5_1_0(context_);
    authSrcConfirmState_ -> IdenticalAccountAclCompare(context_, accesser, accessee);
    authSrcConfirmState_ -> ShareAclCompare(context_, accesser, accessee);
    authSrcConfirmState_ -> Point2PointAclCompare(context_, accesser, accessee);
    authSrcConfirmState_ -> LnnAclCompare(context_, accesser, accessee);
    authSinkNegotiateStateMachine_ -> IdenticalAccountAclCompare(context_, accesser, accessee);
    authSinkNegotiateStateMachine_ -> ShareAclCompare(context_, accesser, accessee);
    authSinkNegotiateStateMachine_ -> Point2PointAclCompare(context_, accesser, accessee);
    authSinkNegotiateStateMachine_ -> LnnAclCompare(context_, accesser, accessee);
}

void AuthConfirmFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
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
    JsonObject jsonObject;
    JsonObject jsonObjectTwo;
    JsonObject jsonObjectThree;
    GenerateJsonObject(jsonObject, fdp);
    GenerateJsonObject(jsonObjectTwo, fdp);
    GenerateJsonObject(jsonObjectThree, fdp);
    std::vector<std::string> deleteCredInfo;
    GenerateStrings(deleteCredInfo, fdp);
    context_ -> extraInfo = fdp.ConsumeRandomLengthString();
    authSrcConfirmState_ -> GetSrcAclInfoForP2P(context_, acl, jsonObject, jsonObjectTwo);
    authSrcConfirmState_ -> CheckCredIdInAcl(context_, acl, jsonObject, bindType);
    authSrcConfirmState_ -> CheckCredIdInAclForP2P(context_, credId, acl, jsonObjectTwo, bindType, checkResult);
    authSrcConfirmState_ -> GetSrcCredType(context_, jsonObject, jsonObjectTwo, jsonObjectThree);
    authSrcConfirmState_ -> GetSrcCredTypeForP2P(context_, jsonObject, jsonObjectTwo, jsonObjectThree, credType,
        deleteCredInfo);
    authSinkConfirmState_ -> MatchFallBackCandidateList(context_, DmAuthType::AUTH_TYPE_PIN);
    authSinkConfirmState_ -> GetCredIdByCredType(context_, credType);
    authSinkStatePinAuthComm_ -> HandleSessionHeartbeat(context_, name);
    authSinkStatePinAuthComm_ -> IsPinCodeValid(numpin);
    authSinkStatePinAuthComm_ -> IsPinCodeValid(strpin);
    authSinkNegotiateStateMachine_ -> GetSinkAclInfo(context_, jsonObject, jsonObjectTwo);
    authSinkNegotiateStateMachine_ -> GetSinkAclInfoForP2P(context_, acl, jsonObject, jsonObjectTwo);
    authSinkNegotiateStateMachine_ -> CheckCredIdInAcl(context_, acl, jsonObject, bindType);
    authSinkNegotiateStateMachine_ -> CheckCredIdInAclForP2P(context_, credId, acl, jsonObject, bindType, checkResult);
    authSinkNegotiateStateMachine_ -> GetSinkCredType(context_, jsonObject, jsonObjectTwo, jsonObjectThree);
    authSinkNegotiateStateMachine_ -> GetSinkCredTypeForP2P(context_, jsonObject, jsonObjectTwo, jsonObjectThree,
        credType, deleteCredInfo);
    authSrcConfirmState_ -> GetSrcAclInfo(context_, jsonObject, jsonObjectTwo);
    ActionFuzzTest();
    AuthConfirmFuzzTestNext(jsonObject, fdp);
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
