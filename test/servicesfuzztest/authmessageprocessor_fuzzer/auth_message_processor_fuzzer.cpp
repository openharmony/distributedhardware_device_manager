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

#include "auth_message_processor_fuzzer.h"

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
std::shared_ptr<DmAuthMessageProcessor> dmAuthMessageProcessor_ = std::make_shared<DmAuthMessageProcessor>();
std::shared_ptr<SoftbusConnector> softbusConnector = std::make_shared<SoftbusConnector>();
std::shared_ptr<IDeviceManagerServiceListener> listener = std::make_shared<DeviceManagerServiceListener>();
std::shared_ptr<HiChainAuthConnector> hiChainAuthConnector = std::make_shared<HiChainAuthConnector>();
std::shared_ptr<HiChainConnector> hiChainConnector = std::make_shared<HiChainConnector>();
std::shared_ptr<AuthManager> authManager = std::make_shared<AuthSrcManager>(softbusConnector,
    hiChainConnector, listener, hiChainAuthConnector);
std::shared_ptr<DmAuthContext> context_ = authManager->GetAuthContext();
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
    jsonObject[TAG_TRANSMIT_SK_ID] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_TRANSMIT_SK_TIMESTAMP] = fdp.ConsumeIntegral<int64_t>();
    jsonObject[TAG_TRANSMIT_CREDENTIAL_ID] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_DMVERSION] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_LNN_SK_ID] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_LNN_SK_TIMESTAMP] = fdp.ConsumeIntegral<int64_t>();
    jsonObject[TAG_LNN_CREDENTIAL_ID] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_MSG_TYPE] = fdp.ConsumeIntegral<int32_t>();
    jsonObject[TAG_SYNC] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_ACCESS] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_DEVICE_VERSION] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_DEVICE_NAME] = fdp.ConsumeRandomLengthString();
    jsonObject[TAG_NETWORKID_ID] = fdp.ConsumeRandomLengthString();
    jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = fdp.ConsumeRandomLengthString();
}

void AuthContextFuzzTest(FuzzedDataProvider &fdp)
{
    DmAuthSide side = DmAuthSide::DM_AUTH_LOCAL_SIDE;
    DmAuthScope scope = DmAuthScope::DM_AUTH_SCOPE_INVALID;
    std::string publicKey = fdp.ConsumeRandomLengthString();
    context_->SetPublicKey(side, scope, publicKey);
}

void AuthMessageProcessorFuzzTestNext(JsonObject &jsonObject)
{
    dmAuthMessageProcessor_ -> ParseAuthStartMessage(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseNegotiateMessage(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageRespAclNegotiate(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageReqUserConfirm(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageRespUserConfirm(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageReqPinAuthStart(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageRespPinAuthStart(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageReqPinAuthNegotiate(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageRespPinAuthNegotiate(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageReqCredExchange(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageRspCredExchange(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageNegotiateTransmit(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageSyncReq(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageSyncResp(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageSinkFinish(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageSrcFinish(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageReverseUltrasonicStart(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageReverseUltrasonicDone(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageForwardUltrasonicStart(jsonObject, context_);
    dmAuthMessageProcessor_ -> ParseMessageForwardUltrasonicNegotiate(jsonObject, context_);
    dmAuthMessageProcessor_ -> CreateNegotiateMessage(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateRespNegotiateMessage(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageReqUserConfirm(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageRespUserConfirm(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageReqPinAuthStart(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageRespPinAuthStart(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageReqPinAuthNegotiate(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageRespPinAuthNegotiate(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageReqCredExchange(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageRspCredExchange(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageReqCredAuthStart(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateCredentialNegotiateMessage(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateNegotiateOldMessage(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateSyncMessage(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageSyncResp(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageFinish(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageForwardUltrasonicStart(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageReverseUltrasonicStart(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageForwardUltrasonicNegotiate(context_, jsonObject);
    dmAuthMessageProcessor_ -> CreateMessageReverseUltrasonicDone(context_, jsonObject);
    dmAuthMessageProcessor_ -> CheckLogicalSessionId(jsonObject, context_);
    std::string message = jsonObject.Dump();
    dmAuthMessageProcessor_ -> ParseMessage(context_, message);
    DmAccessControlTable table;
    FromJson(jsonObject, table);
    DmAccessToSync sync;
    FromJson(jsonObject, sync);
}

void AuthMessageProcessorFuzzTestNextTwo(FuzzedDataProvider &fdp, JsonObject &jsonObject)
{
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t skId = fdp.ConsumeIntegral<int32_t>();
    std::string suffix = fdp.ConsumeRandomLengthString();
    int64_t tokenId = fdp.ConsumeIntegral<int64_t>();
    DistributedDeviceProfile::AccessControlProfile acl;
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    acl.SetExtraData(fdp.ConsumeRandomLengthString());
    accesser.SetAccesserExtraData(fdp.ConsumeRandomLengthString());
    accessee.SetAccesseeExtraData(fdp.ConsumeRandomLengthString());
    DmProxyAuthContext dmProxyAuthContext;
    dmProxyAuthContext.customData = fdp.ConsumeRandomLengthString();
    context_->IsProxyBind = false;
    context_->subjectProxyOnes.clear();
    dmAuthMessageProcessor_->ParseProxyCredExchangeToSync(context_, jsonObject);
    context_->IsProxyBind = true;
    context_->subjectProxyOnes.emplace_back(dmProxyAuthContext);
    dmAuthMessageProcessor_->ParseProxyCredExchangeToSync(context_, jsonObject);
    dmAuthMessageProcessor_->CreateProxyNegotiateMessage(context_, jsonObject);
    dmAuthMessageProcessor_->CreateProxyRespNegotiateMessage(context_, jsonObject);
    dmAuthMessageProcessor_->CreateProxyCredExchangeMessage(context_, jsonObject);
    dmAuthMessageProcessor_->ParseProxyAccessToSync(context_, jsonObject);
    dmAuthMessageProcessor_->ParseProxyNegotiateMessage(jsonObject, context_);
    context_->accessee.dmVersion = DM_VERSION_5_1_0;
    dmAuthMessageProcessor_->ParseMessageProxyRespAclNegotiate(jsonObject, context_);
    context_->accessee.dmVersion = DM_VERSION_5_1_1;
    dmAuthMessageProcessor_->ParseMessageProxyRespAclNegotiate(jsonObject, context_);
    dmAuthMessageProcessor_->ParseMessageProxyReqUserConfirm(jsonObject, context_);
    dmAuthMessageProcessor_->ParseMessageProxyRespUserConfirm(jsonObject, context_);
    dmAuthMessageProcessor_->CreateMessageProxyReqUserConfirm(context_, jsonObject);
    dmAuthMessageProcessor_->CreateProxyAccessMessage(context_, jsonObject);
    dmAuthMessageProcessor_->cryptoMgr_ = std::make_shared<CryptoMgr>();
    dmAuthMessageProcessor_->SaveDerivativeSessionKeyToDP(userId, suffix, skId);
    dmAuthMessageProcessor_->GetSessionKey(userId, skId);
    context_->accesser.extraInfo = "";
    dmAuthMessageProcessor_->SetProxyAccess(context_, dmProxyAuthContext, accesser, accessee);
    dmAuthMessageProcessor_->PutProxyAccessControlList(context_, acl, accesser, accessee);
    dmAuthMessageProcessor_->IsExistTheToken(jsonObject, tokenId);
    dmAuthMessageProcessor_->SetAclProxyRelate(context_);
    acl.accesser_.SetAccesserExtraData("");
    acl.accessee_.SetAccesseeExtraData("");
    dmAuthMessageProcessor_->SetAclProxyRelate(context_, acl);
}

void SetSyncMsgJsonFuzzTest(const uint8_t* data, size_t size)
{
    const int32_t minDataSize = sizeof(int64_t) * 16 + sizeof(int32_t) * 9 + 4;
    if ((data == nullptr) || (size < minDataSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    context_->accesser.deviceId = fdp.ConsumeRandomLengthString();
    context_->accessee.userId = fdp.ConsumeIntegral<int32_t>();
    context_->accesser.isCommonFlag = fdp.ConsumeBool();
    context_->accesser.cert = fdp.ConsumeRandomLengthString();
    context_->direction = static_cast<DmAuthDirection>(fdp.ConsumeIntegralInRange<int32_t>(0, 1));
    context_->confirmOperation = static_cast<UiAction>(fdp.ConsumeIntegral<int32_t>());
    context_->isServiceBind = fdp.ConsumeBool();
    context_->accessee.serviceId = fdp.ConsumeIntegral<int64_t>();

    DmAccess accessSide;
    accessSide.transmitSessionKeyId = fdp.ConsumeIntegral<int32_t>();
    accessSide.transmitSkTimeStamp = fdp.ConsumeIntegral<int64_t>();
    accessSide.transmitCredentialId = fdp.ConsumeRandomLengthString();
    accessSide.isGenerateLnnCredential = fdp.ConsumeBool();
    accessSide.bindLevel = fdp.ConsumeIntegral<int32_t>();
    accessSide.lnnSessionKeyId = fdp.ConsumeIntegral<int32_t>();
    accessSide.lnnSkTimeStamp = fdp.ConsumeIntegral<int64_t>();
    accessSide.lnnCredentialId = fdp.ConsumeRandomLengthString();
    accessSide.dmVersion = fdp.ConsumeRandomLengthString();

    DmAccessToSync accessToSync;
    accessToSync.deviceName = fdp.ConsumeRandomLengthString();
    accessToSync.deviceNameFull = fdp.ConsumeRandomLengthString();
    accessToSync.deviceId = fdp.ConsumeRandomLengthString();
    accessToSync.userId = fdp.ConsumeIntegral<int32_t>();
    accessToSync.accountId = fdp.ConsumeRandomLengthString();
    accessToSync.tokenId = fdp.ConsumeIntegral<int64_t>();
    accessToSync.bundleName = fdp.ConsumeRandomLengthString();
    accessToSync.pkgName = fdp.ConsumeRandomLengthString();
    accessToSync.bindLevel = fdp.ConsumeIntegral<int32_t>();
    accessToSync.sessionKeyId = fdp.ConsumeIntegral<int32_t>();
    accessToSync.skTimeStamp = fdp.ConsumeIntegral<int64_t>();

    JsonObject syncMsgJson;
    dmAuthMessageProcessor_->SetSyncMsgJson(context_, accessSide, accessToSync, syncMsgJson);
}

void GetAccesseeServiceInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int64_t serviceId = fdp.ConsumeIntegral<int64_t>();
    dmAuthMessageProcessor_->GetAccesseeServiceInfo(serviceId);
}

void AuthMessageProcessorFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string message = fdp.ConsumeRandomLengthString();
    std::string inputStr = fdp.ConsumeRandomLengthString();
    std::string compressed = fdp.ConsumeRandomLengthString();
    std::string trustDeviceId = fdp.ConsumeRandomLengthString();
    std::string encSyncMsg = fdp.ConsumeRandomLengthString();
    std::string enSyncMsg = fdp.ConsumeRandomLengthString();
    std::string aclStr = fdp.ConsumeRandomLengthString();
    uint32_t keyLen = fdp.ConsumeIntegralInRange<uint32_t>(0, 1024);
    uint32_t oriLen = fdp.ConsumeIntegralInRange<uint32_t>(0, 1024);
    std::vector<uint8_t> sessionKey = fdp.ConsumeBytes<uint8_t>(keyLen);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t skId = fdp.ConsumeIntegral<int32_t>();
    DistributedDeviceProfile::AccessControlProfile acl;
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    acl.SetExtraData(fdp.ConsumeRandomLengthString());
    accesser.SetAccesserExtraData(fdp.ConsumeRandomLengthString());
    accessee.SetAccesseeExtraData(fdp.ConsumeRandomLengthString());
    DmAccess access;
    JsonObject jsonObject;
    GenerateJsonObject(jsonObject, fdp);
    dmAuthMessageProcessor_ -> ParseMessage(context_, message);
    dmAuthMessageProcessor_ -> CreateMessage(DmMessageType::MSG_TYPE_REQ_ACL_NEGOTIATE, context_);
    dmAuthMessageProcessor_ -> CreateAndSendMsg(DmMessageType::MSG_TYPE_REQ_ACL_NEGOTIATE, context_);
    dmAuthMessageProcessor_ -> SaveSessionKey(sessionKey.data(), keyLen);
    dmAuthMessageProcessor_ -> SaveSessionKeyToDP(userId, skId);
    dmAuthMessageProcessor_ -> DeleteSessionKeyToDP(userId, skId);
    dmAuthMessageProcessor_ -> CompressSyncMsg(inputStr);
    dmAuthMessageProcessor_ -> DecompressSyncMsg(compressed, oriLen);
    dmAuthMessageProcessor_ -> Base64Encode(inputStr);
    dmAuthMessageProcessor_ -> Base64Decode(inputStr);
    dmAuthMessageProcessor_ -> PutAccessControlList(context_, access, trustDeviceId);
    dmAuthMessageProcessor_ -> EncryptSyncMessage(context_, access, encSyncMsg);
    dmAuthMessageProcessor_ -> DecryptSyncMessage(context_, access, enSyncMsg);
    dmAuthMessageProcessor_ -> ParseSyncMessage(context_, access, jsonObject);
    dmAuthMessageProcessor_ -> CheckAccessValidityAndAssign(context_, access, access);
    dmAuthMessageProcessor_ -> ACLToStr(acl, aclStr);
    dmAuthMessageProcessor_ -> SetAccessControlList(context_, acl);
    dmAuthMessageProcessor_ -> SetTransmitAccessControlList(context_, accesser, accessee);
    dmAuthMessageProcessor_ -> SetLnnAccessControlList(context_, accesser, accessee);
    AuthMessageProcessorFuzzTestNext(jsonObject);
    AuthMessageProcessorFuzzTestNextTwo(fdp, jsonObject);
    AuthContextFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::AuthMessageProcessorFuzzTest(data, size);
    OHOS::DistributedHardware::SetSyncMsgJsonFuzzTest(data, size);
    OHOS::DistributedHardware::GetAccesseeServiceInfoFuzzTest(data, size);

    return 0;
}
