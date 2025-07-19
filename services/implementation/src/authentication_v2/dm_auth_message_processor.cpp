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
#include "dm_auth_message_processor.h"

#include <zlib.h>
#include <iostream>
#include <sstream>

#include <mbedtls/base64.h>
#include "parameter.h"

#include "access_control_profile.h"
#include "distributed_device_profile_client.h"
#include "service_info_profile.h"
#include "service_info_unique_key.h"

#include "deviceprofile_connector.h"
#include "dm_log.h"
#include "dm_constants.h"
#include "dm_anonymous.h"
#include "dm_auth_manager_base.h"
#include "dm_auth_context.h"
#include "dm_auth_state_machine.h"
#include "dm_crypto.h"

namespace OHOS {
namespace DistributedHardware {

const char* TAG_LNN_PUBLIC_KEY = "lnnPublicKey";
const char* TAG_TRANSMIT_PUBLIC_KEY = "transmitPublicKey";
const char* TAG_LNN_CREDENTIAL_ID = "lnnCredentialId";
const char* TAG_TRANSMIT_CREDENTIAL_ID = "transmitCredentialId";
const char* TAG_CONFIRM_OPERATION_V2 = "confirmOperation";
const char* TAG_AUTH_TYPE_LIST = "authTypeList";
const char* TAG_CURRENT_AUTH_TYPE_IDX = "currentAuthTypeIdx";

// IS interface input parameter json format string key
const char* TAG_METHOD = "method";
const char* TAG_PEER_USER_SPACE_ID = "peerUserSpaceId";
const char* TAG_SUBJECT = "subject";
const char* TAG_CRED_TYPE = "credType";
const char* TAG_KEY_FORMAT = "keyFormat";
const char* TAG_ALGORITHM_TYPE = "algorithmType";
const char* TAG_PROOF_TYPE = "proofType";
const char* TAG_KEY_VALUE = "keyValue";
const char* TAG_AUTHORIZED_SCOPE = "authorizedScope";
const char* TAG_AUTHORIZED_APP_LIST = "authorizedAppList";
const char* TAG_CREDENTIAL_OWNER = "credOwner";
const char* TAG_SYNC = "syncMessage";
const char* TAG_ACCESS = "dmAccess";
const char* TAG_PROXY = "proxy";
const char* TAG_ACL = "accessControlTable";
const char* TAG_ACCESSER = "dmAccesser";
const char* TAG_ACCESSEE = "dmAccessee";
const char* TAG_SERVICEINFO = "serviceInfo";
const char* TAG_USER_CONFIRM_OPT = "userConfirmOpt";
// The local SK information is synchronized to the remote end to construct acl-accesser/accessee.
const char* TAG_TRANSMIT_SK_ID = "accessAppSKId";
const char* TAG_LNN_SK_ID = "accessUserSKId";
const char* TAG_TRANSMIT_SK_TIMESTAMP = "accessAppSKTimeStamp";
const char* TAG_LNN_SK_TIMESTAMP = "accessUserSKTimeStamp";
const char* TAG_USER_ID = "userId";
const char* TAG_TOKEN_ID = "tokenId";
const char* TAG_NETWORKID_ID = "networkId";
const char* TAG_ISSUER = "issuer";

const char* TAG_DEVICE_VERSION = "deviceVersion";
const char* TAG_DEVICE_NAME = "deviceName";
const char* TAG_DEVICE_ID_HASH = "deviceIdHash";
const char* TAG_ACCOUNT_ID_HASH = "accountIdHash";
const char* TAG_TOKEN_ID_HASH = "tokenIdHash";
const char* TAG_PKG_NAME = "pkgName";
const char* TAG_ACL_CHECKSUM = "aclCheckSum";
const char* TAG_COMPRESS_ORI_LEN = "compressOriLen";
const char* TAG_COMPRESS = "compressMsg";
const char* TAG_STATE = "state";
const char* TAG_REASON = "reason";
const char* TAG_PEER_USER_ID = "peerUserId";
const char* TAG_PEER_DISPLAY_ID = "peerDisplayId";
const char* TAG_EXTRA_INFO = "extraInfo";
const char* TAG_ACL_TYPE_LIST = "aclTypeList";
const char* TAG_CERT_TYPE_LIST = "certTypeList";

const char* TAG_IS_ONLINE = "isOnline";
const char* TAG_IS_AUTHED = "isAuthed";
const char* TAG_CREDENTIAL_INFO = "credentialInfo";
const char* TAG_CERT_INFO = "certInfo";
const char* TAG_LANGUAGE = "language";
const char* TAG_ULTRASONIC_SIDE = "ultrasonicSide";
const char* TAG_REMAINING_FROZEN_TIME = "remainingFrozenTime";
constexpr const char* TAG_CUSTOM_DESCRIPTION = "CUSTOMDESC";

namespace {

constexpr const int32_t DM_HASH_LEN = 32;
const char* TAG_DEVICE_TYPE = "DEVICETYPE";
constexpr int32_t DM_ULTRASONIC_FORWARD = 0;
constexpr int32_t DM_ULTRASONIC_REVERSE = 1;
constexpr uint32_t MAX_SESSION_KEY_LENGTH = 512;

void ParseDmAccessToSync(const std::string &jsonString, DmAccess &access, bool isUseDeviceFullName)
{
    JsonObject accessjson(jsonString);
    DmAccessToSync srcAccessToSync = accessjson.Get<DmAccessToSync>();
    access.deviceName = (isUseDeviceFullName ? srcAccessToSync.deviceNameFull : srcAccessToSync.deviceName);
    access.deviceId = srcAccessToSync.deviceId;
    access.userId = srcAccessToSync.userId;
    access.accountId = srcAccessToSync.accountId;
    access.tokenId = static_cast<int64_t>(srcAccessToSync.tokenId);
    access.bundleName = srcAccessToSync.bundleName;
    access.pkgName = srcAccessToSync.pkgName;
    access.bindLevel = srcAccessToSync.bindLevel;
    access.sessionKeyId = srcAccessToSync.sessionKeyId;
    access.skTimeStamp = srcAccessToSync.skTimeStamp;
    return;
}

void SaveToDmAccessSync(DmAccessToSync &accessToSync, const std::shared_ptr<DmAuthContext> context,
    const DmAccess &accessSide)
{
    accessToSync.deviceName = accessSide.deviceName;
    accessToSync.deviceNameFull = context->softbusConnector->GetLocalDeviceName();
    accessToSync.deviceId = accessSide.deviceId;
    accessToSync.userId = accessSide.userId;
    accessToSync.accountId = accessSide.accountId;
    accessToSync.tokenId = accessSide.tokenId;
    accessToSync.bundleName = accessSide.bundleName;
    accessToSync.pkgName = accessSide.pkgName;
    accessToSync.bindLevel = accessSide.bindLevel;
}

int32_t ParseInfoToDmAccess(const JsonObject &jsonObject, DmAccess &access)
{
    // transmit session key is mandatory
    if (!IsString(jsonObject, TAG_TRANSMIT_SK_ID)) {
        LOGE("ParseSyncMessage TAG_TRANSMIT_SK_ID error");
        return ERR_DM_FAILED;
    }
    access.transmitSessionKeyId = std::atoi(jsonObject[TAG_TRANSMIT_SK_ID].Get<std::string>().c_str());

    if (!IsInt64(jsonObject, TAG_TRANSMIT_SK_TIMESTAMP)) {
        LOGE("ParseSyncMessage TAG_TRANSMIT_SK_TIMESTAMP error");
        return ERR_DM_FAILED;
    }
    access.transmitSkTimeStamp = jsonObject[TAG_TRANSMIT_SK_TIMESTAMP].Get<int64_t>();

    if (!IsString(jsonObject, TAG_TRANSMIT_CREDENTIAL_ID)) {
        LOGE("ParseSyncMessage TAG_TRANSMIT_CREDENTIAL_ID error");
        return ERR_DM_FAILED;
    }
    access.transmitCredentialId = jsonObject[TAG_TRANSMIT_CREDENTIAL_ID].Get<std::string>().c_str();

    if (!IsString(jsonObject, TAG_DMVERSION)) {
        LOGE("ParseSyncMessage TAG_DMVERSION error");
        return ERR_DM_FAILED;
    }
    access.dmVersion = jsonObject[TAG_DMVERSION].Get<std::string>();

    // lnn session key is optional
    if (IsString(jsonObject, TAG_LNN_SK_ID)) {
        access.lnnSessionKeyId = std::atoi(jsonObject[TAG_LNN_SK_ID].Get<std::string>().c_str());
    }
    if (IsInt64(jsonObject, TAG_LNN_SK_TIMESTAMP)) {
        access.lnnSkTimeStamp = jsonObject[TAG_LNN_SK_TIMESTAMP].Get<int64_t>();
    }

    if (IsString(jsonObject, TAG_LNN_CREDENTIAL_ID)) {
        access.lnnCredentialId = jsonObject[TAG_LNN_CREDENTIAL_ID].Get<std::string>().c_str();
    }

    return DM_OK;
}

bool IsMessageValid(const JsonItemObject &jsonObject)
{
    if (jsonObject.IsDiscarded()) {
        LOGE("DmAuthMessageProcessor::ParseMessage failed, decodeRequestAuth jsonStr error");
        return false;
    }
    if (!jsonObject[TAG_MSG_TYPE].IsNumberInteger()) {
        LOGE("DmAuthMessageProcessor::ParseMessage failed, message type error.");
        return false;
    }
    return true;
}
}

int32_t DmAuthMessageProcessor::SaveSessionKey(const uint8_t *sessionKey, const uint32_t keyLen)
{
    if (cryptoMgr_ == nullptr) {
        LOGE("DmAuthMessageProcessor::SaveSessionKey failed, cryptoMgr_ is nullptr.");
        return ERR_DM_FAILED;
    }
    return cryptoMgr_->ProcessSessionKey(sessionKey, keyLen);
}

int32_t DmAuthMessageProcessor::SaveSessionKeyToDP(int32_t userId, int32_t &skId)
{
    if (cryptoMgr_ == nullptr) {
        LOGE("DmAuthMessageProcessor::SaveSessionKeyToDP failed, cryptoMgr_ is nullptr.");
        return ERR_DM_FAILED;
    }
    return DeviceProfileConnector::GetInstance().PutSessionKey(userId, cryptoMgr_->GetSessionKey(), skId);
}

int32_t DmAuthMessageProcessor::SaveDerivativeSessionKeyToDP(int32_t userId, const std::string &suffix, int32_t &skId)
{
    if (cryptoMgr_ == nullptr) {
        LOGE("DmAuthMessageProcessor::SaveSessionKeyToDP failed, cryptoMgr_ is nullptr.");
        return ERR_DM_FAILED;
    }
    std::vector<unsigned char> sessionKey = cryptoMgr_->GetSessionKey();
    size_t keyLen = sessionKey.size();
    std::string keyStr;
    for (size_t i = 0; i < sessionKey.size(); ++i) {
        keyStr = keyStr + (char)sessionKey.data()[i];
    }
    std::string newKeyStr = Crypto::Sha256(keyStr + suffix);
    DMSessionKey newSessionKey;
    size_t newKeyLen = std::min(keyLen, newKeyStr.size());
    if (newKeyLen == 0 || newKeyLen > MAX_SESSION_KEY_LENGTH) {
        LOGE("newKeyLen invaild, cannot allocate memory.");
        return ERR_DM_FAILED;
    }
    newSessionKey.key = (uint8_t*)calloc(newKeyLen, sizeof(uint8_t));
    if (memcpy_s(newSessionKey.key, newKeyLen, newKeyStr.c_str(), newKeyLen) != DM_OK) {
        LOGE("copy key data failed.");
        if (newSessionKey.key != nullptr) {
            (void)memset_s(newSessionKey.key, newKeyLen, 0, newKeyLen);
            free(newSessionKey.key);
            newSessionKey.key = nullptr;
            newSessionKey.keyLen = 0;
        }
        return ERR_DM_FAILED;
    }
    newSessionKey.keyLen = newKeyLen;
    int ret = DeviceProfileConnector::GetInstance().PutSessionKey(userId,
        std::vector<unsigned char>(newSessionKey.key, newSessionKey.key + newSessionKey.keyLen), skId);
    if (ret != DM_OK) {
        LOGE("DP save user session key failed %{public}d", ret);
    }
    if (newSessionKey.key != nullptr) {
        (void)memset_s(newSessionKey.key, newSessionKey.keyLen, 0, newSessionKey.keyLen);
        free(newSessionKey.key);
        newSessionKey.key = nullptr;
        newSessionKey.keyLen = 0;
    }
    return ret;
}

int32_t DmAuthMessageProcessor::GetSessionKey(int32_t userId, int32_t &skId)
{
    std::vector<unsigned char> sessionKey;
    int32_t ret = DeviceProfileConnector::GetInstance().GetSessionKey(userId, skId, sessionKey);
    if (ret != DM_OK) {
        LOGE("failed: %{public}d", ret);
        return ret;
    }
    CHECK_NULL_RETURN(cryptoMgr_, ERR_DM_POINT_NULL);
    return cryptoMgr_->ProcessSessionKey(sessionKey.data(), sessionKey.size());
}

int32_t DmAuthMessageProcessor::DeleteSessionKeyToDP(int32_t userId, int32_t skId)
{
    return DeviceProfileConnector::GetInstance().DeleteSessionKey(userId, skId);
}

void DmAuthMessageProcessor::SetAccessControlList(std::shared_ptr<DmAuthContext> context,
    DistributedDeviceProfile::AccessControlProfile &profile)
{
    uint32_t authenticationType = ALLOW_AUTH_ONCE;
    if (context->confirmOperation == USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS) {
        authenticationType = ALLOW_AUTH_ALWAYS;
    }
    profile.SetAuthenticationType(authenticationType);
    profile.SetStatus(ACTIVE);
    profile.SetDeviceIdType((int32_t)DistributedDeviceProfile::DeviceIdType::UDID);
}

void DmAuthMessageProcessor::SetTransmitAccessControlList(std::shared_ptr<DmAuthContext> context,
    DistributedDeviceProfile::Accesser &accesser, DistributedDeviceProfile::Accessee &accessee)
{
    accesser.SetAccesserDeviceId(context->accesser.deviceId);
    accesser.SetAccesserUserId(context->accesser.userId);
    accesser.SetAccesserAccountId(context->accesser.accountId);
    accesser.SetAccesserTokenId(context->accesser.tokenId);
    accesser.SetAccesserBundleName(context->accesser.pkgName);
    accesser.SetAccesserDeviceName(context->accesser.deviceName);
    accesser.SetAccesserCredentialIdStr(context->accesser.transmitCredentialId);
    accesser.SetAccesserSessionKeyId(context->accesser.transmitSessionKeyId);
    accesser.SetAccesserSKTimeStamp(context->accesser.transmitSkTimeStamp);
    accesser.SetAccesserExtraData(context->accesser.extraInfo);

    accessee.SetAccesseeDeviceId(context->accessee.deviceId);
    accessee.SetAccesseeUserId(context->accessee.userId);
    accessee.SetAccesseeAccountId(context->accessee.accountId);
    accessee.SetAccesseeTokenId(context->accessee.tokenId);
    accessee.SetAccesseeBundleName(context->accessee.pkgName);
    accessee.SetAccesseeDeviceName(context->accessee.deviceName);
    accessee.SetAccesseeCredentialIdStr(context->accessee.transmitCredentialId); // 依赖dp
    accessee.SetAccesseeSessionKeyId(context->accessee.transmitSessionKeyId);
    accessee.SetAccesseeSKTimeStamp(context->accessee.transmitSkTimeStamp);
    accessee.SetAccesseeExtraData(context->accessee.extraInfo);
}

void DmAuthMessageProcessor::SetLnnAccessControlList(std::shared_ptr<DmAuthContext> context,
    DistributedDeviceProfile::Accesser &accesser, DistributedDeviceProfile::Accessee &accessee)
{
    accesser.SetAccesserDeviceId(context->accesser.deviceId);
    accesser.SetAccesserUserId(context->accesser.userId);
    accesser.SetAccesserAccountId(context->accesser.accountId);
    accesser.SetAccesserTokenId(0);
    accesser.SetAccesserDeviceName(context->accesser.deviceName);
    accesser.SetAccesserCredentialIdStr(context->accesser.lnnCredentialId);
    accesser.SetAccesserSessionKeyId(context->accesser.lnnSessionKeyId);
    accesser.SetAccesserSKTimeStamp(context->accesser.lnnSkTimeStamp);
    accesser.SetAccesserExtraData(context->accesser.extraInfo);

    accessee.SetAccesseeDeviceId(context->accessee.deviceId);
    accessee.SetAccesseeUserId(context->accessee.userId);
    accessee.SetAccesseeAccountId(context->accessee.accountId);
    accessee.SetAccesseeTokenId(0);
    accessee.SetAccesseeDeviceName(context->accessee.deviceName);
    accessee.SetAccesseeCredentialIdStr(context->accessee.lnnCredentialId);
    accessee.SetAccesseeSessionKeyId(context->accessee.lnnSessionKeyId);
    accessee.SetAccesseeSKTimeStamp(context->accessee.lnnSkTimeStamp);
    accessee.SetAccesseeExtraData(context->accessee.extraInfo);
}

int32_t DmAuthMessageProcessor::PutAccessControlList(std::shared_ptr<DmAuthContext> context,
    DmAccess &access, std::string trustDeviceId)
{
    LOGI("Start.");
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    SetLnnAccessControlList(context, accesser, accessee);
    DistributedDeviceProfile::AccessControlProfile profile;
    SetAccessControlList(context, profile);
    profile.SetTrustDeviceId(trustDeviceId);
    profile.SetDeviceIdHash(access.deviceIdHash);
    profile.SetBindType(access.lnnBindType);
    profile.SetAccessee(accessee);
    profile.SetAccesser(accesser);
    JsonObject extraData;
    if (access.isPutLnnAcl && access.bindLevel != static_cast<int32_t>(USER)) {
        profile.SetBindLevel(USER);
        std::string isLnnAclTrue = std::string(ACL_IS_LNN_ACL_VAL_TRUE);
        extraData[ACL_IS_LNN_ACL_KEY] = isLnnAclTrue;
        profile.SetExtraData(extraData.Dump());
        int32_t ret =
            DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance().PutAccessControlProfile(profile);
        if (ret != DM_OK) {
            LOGE("PutAccessControlProfile failed.");
        }
    }
    bool isAuthed = (context->direction == DM_AUTH_SOURCE) ? context->accesser.isAuthed : context->accessee.isAuthed;
    if (!context->IsProxyBind || context->subjectProxyOnes.empty() || (context->IsCallingProxyAsSubject && !isAuthed)) {
        std::string isLnnAclFalse = std::string(ACL_IS_LNN_ACL_VAL_FALSE);
        extraData[ACL_IS_LNN_ACL_KEY] = isLnnAclFalse;
        profile.SetExtraData(extraData.Dump());
        profile.SetBindLevel(access.bindLevel);
        SetTransmitAccessControlList(context, accesser, accessee);
        profile.SetBindLevel(access.bindLevel);
        profile.SetBindType(access.transmitBindType);
        profile.SetAccessee(accessee);
        profile.SetAccesser(accesser);
        int32_t ret =
            DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance().PutAccessControlProfile(profile);
        if (ret != DM_OK) {
            LOGE("PutAccessControlProfile failed.");
        }
    }
    return PutProxyAccessControlList(context, profile, accesser, accessee);
}

int32_t DmAuthMessageProcessor::SetProxyAccess(std::shared_ptr<DmAuthContext> context,
    DmProxyAuthContext &proxyAuthContext, DistributedDeviceProfile::Accesser &accesser,
    DistributedDeviceProfile::Accessee &accessee)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    accesser.SetAccesserTokenId(proxyAuthContext.proxyAccesser.tokenId);
    accesser.SetAccesserBundleName(proxyAuthContext.proxyAccesser.bundleName);
    accesser.SetAccesserCredentialIdStr(proxyAuthContext.proxyAccesser.transmitCredentialId);
    accesser.SetAccesserSessionKeyId(proxyAuthContext.proxyAccesser.transmitSessionKeyId);
    accesser.SetAccesserSKTimeStamp(proxyAuthContext.proxyAccesser.skTimeStamp);
    JsonObject accesserProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    accesserProxyObj.PushBack(context->accesser.tokenId);
    JsonObject accesserExtObj;
    if (!context->accesser.extraInfo.empty()) {
        accesserExtObj.Parse(context->accesser.extraInfo);
    }
    accesserExtObj[TAG_PROXY] = accesserProxyObj.Dump();
    accesser.SetAccesserExtraData(accesserExtObj.Dump());

    accessee.SetAccesseeTokenId(proxyAuthContext.proxyAccessee.tokenId);
    accessee.SetAccesseeBundleName(proxyAuthContext.proxyAccessee.bundleName);
    accessee.SetAccesseeCredentialIdStr(proxyAuthContext.proxyAccessee.transmitCredentialId);
    accessee.SetAccesseeSessionKeyId(proxyAuthContext.proxyAccessee.transmitSessionKeyId);
    accessee.SetAccesseeSKTimeStamp(proxyAuthContext.proxyAccessee.skTimeStamp);
    JsonObject accesseeProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    accesseeProxyObj.PushBack(context->accessee.tokenId);
    JsonObject accesseeExtObj;
    if (!context->accessee.extraInfo.empty()) {
        accesseeExtObj.Parse(context->accessee.extraInfo);
    }
    accesseeExtObj[TAG_PROXY] = accesseeProxyObj.Dump();
    accessee.SetAccesseeExtraData(accesseeExtObj.Dump());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::PutProxyAccessControlList(std::shared_ptr<DmAuthContext> context,
    DistributedDeviceProfile::AccessControlProfile &profile, DistributedDeviceProfile::Accesser &accesser,
    DistributedDeviceProfile::Accessee &accessee)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return DM_OK;
    }
    for (auto &app : context->subjectProxyOnes) {
        if (context->direction == DM_AUTH_SOURCE ? app.proxyAccesser.isAuthed : app.proxyAccessee.isAuthed) {
            continue;
        }
        SetProxyAccess(context, app, accesser, accessee);
        JsonObject extraData;
        std::string isLnnAclFalse = std::string(ACL_IS_LNN_ACL_VAL_FALSE);
        extraData[ACL_IS_LNN_ACL_KEY] = isLnnAclFalse;
        profile.SetExtraData(extraData.Dump());
        profile.SetBindLevel(context->direction == DM_AUTH_SOURCE ? app.proxyAccesser.bindLevel :
            app.proxyAccessee.bindLevel);
        profile.SetBindType(context->direction == DM_AUTH_SOURCE ? context->accesser.transmitBindType :
            context->accessee.transmitBindType);
        profile.SetAccessee(accessee);
        profile.SetAccesser(accesser);
        int32_t ret =
            DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance().PutAccessControlProfile(profile);
        if (ret != DM_OK) {
            LOGE("PutAccessControlProfile failed. %{public}d", ret);
            return ret;
        }
    }
    SetAclProxyRelate(context);
    return DM_OK;
}

void DmAuthMessageProcessor::ConstructCreateMessageFuncMap()
{
    createMessageFuncMap_ = {
        {DmMessageType::MSG_TYPE_REQ_ACL_NEGOTIATE, &DmAuthMessageProcessor::CreateNegotiateMessage},
        {DmMessageType::MSG_TYPE_RESP_ACL_NEGOTIATE, &DmAuthMessageProcessor::CreateRespNegotiateMessage},
        {DmMessageType::MSG_TYPE_REQ_USER_CONFIRM, &DmAuthMessageProcessor::CreateMessageReqUserConfirm},
        {DmMessageType::MSG_TYPE_RESP_USER_CONFIRM, &DmAuthMessageProcessor::CreateMessageRespUserConfirm},
        {DmMessageType::MSG_TYPE_REQ_PIN_AUTH_START, &DmAuthMessageProcessor::CreateMessageReqPinAuthStart},
        {DmMessageType::MSG_TYPE_REQ_PIN_AUTH_MSG_NEGOTIATE, &DmAuthMessageProcessor::CreateMessageReqPinAuthNegotiate},
        {DmMessageType::MSG_TYPE_RESP_PIN_AUTH_START, &DmAuthMessageProcessor::CreateMessageRespPinAuthStart},
        {DmMessageType::MSG_TYPE_RESP_PIN_AUTH_MSG_NEGOTIATE,
            &DmAuthMessageProcessor::CreateMessageRespPinAuthNegotiate},
        {DmMessageType::MSG_TYPE_REQ_CREDENTIAL_EXCHANGE, &DmAuthMessageProcessor::CreateMessageReqCredExchange},
        {DmMessageType::MSG_TYPE_RESP_CREDENTIAL_EXCHANGE, &DmAuthMessageProcessor::CreateMessageRspCredExchange},
        {DmMessageType::MSG_TYPE_REQ_SK_DERIVE, &DmAuthMessageProcessor::CreateMessageReqSKDerive},
        {DmMessageType::MSG_TYPE_RESP_SK_DERIVE, &DmAuthMessageProcessor::CreateMessageRspSKDerive},
        {DmMessageType::MSG_TYPE_REQ_CREDENTIAL_AUTH_START, &DmAuthMessageProcessor::CreateMessageReqCredAuthStart},
        {DmMessageType::MSG_TYPE_REQ_CREDENTIAL_AUTH_NEGOTIATE,
            &DmAuthMessageProcessor::CreateCredentialNegotiateMessage},
        {DmMessageType::MSG_TYPE_RESP_CREDENTIAL_AUTH_START, &DmAuthMessageProcessor::CreateCredentialNegotiateMessage},
        {DmMessageType::MSG_TYPE_RESP_CREDENTIAL_AUTH_NEGOTIATE,
            &DmAuthMessageProcessor::CreateCredentialNegotiateMessage},
        {DmMessageType::MSG_TYPE_REQ_DATA_SYNC, &DmAuthMessageProcessor::CreateSyncMessage},
        {DmMessageType::MSG_TYPE_RESP_DATA_SYNC, &DmAuthMessageProcessor::CreateMessageSyncResp},
        {DmMessageType::MSG_TYPE_AUTH_REQ_FINISH, &DmAuthMessageProcessor::CreateMessageFinish},
        {DmMessageType::MSG_TYPE_AUTH_RESP_FINISH, &DmAuthMessageProcessor::CreateMessageFinish},
    };
}

void DmAuthMessageProcessor::ConstructParseMessageFuncMap()
{
    paraseMessageFuncMap_ = {
        {DmMessageType::MSG_TYPE_REQ_ACL_NEGOTIATE, &DmAuthMessageProcessor::ParseNegotiateMessage},
        {DmMessageType::MSG_TYPE_RESP_ACL_NEGOTIATE, &DmAuthMessageProcessor::ParseMessageRespAclNegotiate},
        {DmMessageType::MSG_TYPE_REQ_USER_CONFIRM, &DmAuthMessageProcessor::ParseMessageReqUserConfirm},
        {DmMessageType::MSG_TYPE_RESP_USER_CONFIRM, &DmAuthMessageProcessor::ParseMessageRespUserConfirm},
        {DmMessageType::MSG_TYPE_REQ_PIN_AUTH_START, &DmAuthMessageProcessor::ParseMessageReqPinAuthStart},
        {DmMessageType::MSG_TYPE_REQ_PIN_AUTH_MSG_NEGOTIATE, &DmAuthMessageProcessor::ParseMessageReqPinAuthNegotiate},
        {DmMessageType::MSG_TYPE_RESP_PIN_AUTH_START, &DmAuthMessageProcessor::ParseMessageRespPinAuthStart},
        {DmMessageType::MSG_TYPE_RESP_PIN_AUTH_MSG_NEGOTIATE,
            &DmAuthMessageProcessor::ParseMessageRespPinAuthNegotiate},
        {DmMessageType::MSG_TYPE_REQ_CREDENTIAL_EXCHANGE, &DmAuthMessageProcessor::ParseMessageReqCredExchange},
        {DmMessageType::MSG_TYPE_RESP_CREDENTIAL_EXCHANGE, &DmAuthMessageProcessor::ParseMessageRspCredExchange},
        {DmMessageType::MSG_TYPE_REQ_SK_DERIVE, &DmAuthMessageProcessor::ParseMessageReqSKDerive},
        {DmMessageType::MSG_TYPE_RESP_SK_DERIVE, &DmAuthMessageProcessor::ParseMessageRspSKDerive},
        {DmMessageType::MSG_TYPE_REQ_CREDENTIAL_AUTH_START, &DmAuthMessageProcessor::ParseAuthStartMessage},
        {DmMessageType::MSG_TYPE_REQ_CREDENTIAL_AUTH_NEGOTIATE, &DmAuthMessageProcessor::ParseMessageNegotiateTransmit},
        {DmMessageType::MSG_TYPE_RESP_CREDENTIAL_AUTH_START, &DmAuthMessageProcessor::ParseMessageNegotiateTransmit},
        {DmMessageType::MSG_TYPE_RESP_CREDENTIAL_AUTH_NEGOTIATE,
            &DmAuthMessageProcessor::ParseMessageNegotiateTransmit},
        {DmMessageType::MSG_TYPE_REQ_DATA_SYNC, &DmAuthMessageProcessor::ParseMessageSyncReq},
        {DmMessageType::MSG_TYPE_RESP_DATA_SYNC, &DmAuthMessageProcessor::ParseMessageSyncResp},
        {DmMessageType::MSG_TYPE_AUTH_REQ_FINISH, &DmAuthMessageProcessor::ParseMessageSinkFinish},
        {DmMessageType::MSG_TYPE_AUTH_RESP_FINISH, &DmAuthMessageProcessor::ParseMessageSrcFinish},
    };
}

DmAuthMessageProcessor::DmAuthMessageProcessor()
{
    LOGI("DmAuthMessageProcessor constructor");
    cryptoMgr_ = std::make_shared<CryptoMgr>();
    ConstructCreateMessageFuncMap();
    ConstructParseMessageFuncMap();
    DmAuthUltrasonicMessageProcessor();
}

DmAuthMessageProcessor::~DmAuthMessageProcessor()
{
    if (cryptoMgr_ != nullptr) {
        cryptoMgr_->ClearSessionKey();
        cryptoMgr_ = nullptr;
    }
}

int32_t DmAuthMessageProcessor::ParseMessage(std::shared_ptr<DmAuthContext> context, const std::string &message)
{
    JsonObject jsonObject(message);
    if (context == nullptr || !IsMessageValid(jsonObject)) {
        return ERR_DM_FAILED;
    }
    DmMessageType msgType = static_cast<DmMessageType>(jsonObject[TAG_MSG_TYPE].Get<int32_t>());
    context->msgType = msgType;
    LOGI("DmAuthMessageProcessor::ParseMessage message type %{public}d", context->msgType);
    if (CheckLogicalSessionId(jsonObject, context) != DM_OK) {
        LOGE("CheckLogicalSessionId failed.");
        return ERR_DM_FAILED;
    }
    auto itr = paraseMessageFuncMap_.find(msgType);
    if (itr == paraseMessageFuncMap_.end()) {
        LOGI("DmAuthMessageProcessor::ParseMessage message type error %{public}d", context->msgType);
        return ERR_DM_FAILED;
    }
    return (this->*(itr->second))(jsonObject, context);
}

void DmAuthMessageProcessor::DmAuthUltrasonicMessageProcessor()
{
    LOGI("DmAuthUltrasonicMessageProcessor enter.");
    createMessageFuncMap_[DmMessageType::MSG_TYPE_REVERSE_ULTRASONIC_START] =
        &DmAuthMessageProcessor::CreateMessageReverseUltrasonicStart;
    createMessageFuncMap_[DmMessageType::MSG_TYPE_REVERSE_ULTRASONIC_DONE] =
        &DmAuthMessageProcessor::CreateMessageReverseUltrasonicDone;
    createMessageFuncMap_[DmMessageType::MSG_TYPE_FORWARD_ULTRASONIC_START] =
        &DmAuthMessageProcessor::CreateMessageForwardUltrasonicStart;
    createMessageFuncMap_[DmMessageType::MSG_TYPE_FORWARD_ULTRASONIC_NEGOTIATE] =
        &DmAuthMessageProcessor::CreateMessageForwardUltrasonicNegotiate;

    paraseMessageFuncMap_[DmMessageType::MSG_TYPE_REVERSE_ULTRASONIC_START] =
        &DmAuthMessageProcessor::ParseMessageReverseUltrasonicStart;
    paraseMessageFuncMap_[DmMessageType::MSG_TYPE_REVERSE_ULTRASONIC_DONE] =
        &DmAuthMessageProcessor::ParseMessageReverseUltrasonicDone;
    paraseMessageFuncMap_[DmMessageType::MSG_TYPE_FORWARD_ULTRASONIC_START] =
        &DmAuthMessageProcessor::ParseMessageForwardUltrasonicStart;
    paraseMessageFuncMap_[DmMessageType::MSG_TYPE_FORWARD_ULTRASONIC_NEGOTIATE] =
        &DmAuthMessageProcessor::ParseMessageForwardUltrasonicNegotiate;
    LOGI("DmAuthUltrasonicMessageProcessor leave.");
    return;
}

static std::vector<DmAuthType> stringToVectorAuthType(const std::string& str)
{
    std::vector<DmAuthType> vec;
    std::istringstream iss(str);
    int32_t num;
    while (iss >> num) {
        vec.push_back(static_cast<DmAuthType>(num));
    }
    return vec;
}

static std::string vectorAuthTypeToString(const std::vector<DmAuthType>& vec)
{
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << static_cast<int32_t>(vec[i]);
        if (i != vec.size() - 1) {
            oss << " ";  // Add a separator (e.g. space)
        }
    }
    return oss.str();
}

int32_t DmAuthMessageProcessor::ParseMessageNegotiateTransmit(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded() || !jsonObject.Contains(TAG_DATA) || !jsonObject[TAG_DATA].IsString()) {
        LOGE("DmAuthMessageProcessor::ParseMessageNegotiateTransmit Unlegal json string failed");
        return ERR_DM_FAILED;
    }

    context->transmitData = jsonObject[TAG_DATA].Get<std::string>();

    switch (context->msgType) {
        case MSG_TYPE_REQ_CREDENTIAL_AUTH_NEGOTIATE:           // 161
            context->authStateMachine->TransitionTo(std::make_shared<AuthSinkCredentialAuthNegotiateState>());
            break;
        case MSG_TYPE_RESP_CREDENTIAL_AUTH_START:              // 170
            context->authStateMachine->TransitionTo(std::make_shared<AuthSrcCredentialAuthNegotiateState>());
            break;
        case MSG_TYPE_RESP_CREDENTIAL_AUTH_NEGOTIATE:          // 171
            context->authStateMachine->TransitionTo(std::make_shared<AuthSrcCredentialAuthDoneState>());
            break;
        default:
            return ERR_DM_FAILED;
    }

    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageRespPinAuthNegotiate(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded() || !jsonObject[TAG_DATA].IsString()) {
        LOGE("DmAuthMessageProcessor::ParseMessageRespPinAuthNegotiate failed, decodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }

    context->transmitData = jsonObject[TAG_DATA].Get<std::string>();
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinAuthDoneState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageReqCredExchange(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded() || !jsonObject[TAG_DATA].IsString()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }

    std::string plainText;
    if (cryptoMgr_->DecryptMessage(jsonObject[TAG_DATA].Get<std::string>(), plainText) != DM_OK) {
        LOGE("DmAuthMessageProcessor::ParseMessageReqCredExchange() error, decrypt data failed.");
        return ERR_DM_FAILED;
    }
    JsonObject jsonData(plainText);

    // First authentication, parse lnn public key
    if (context->accessee.isGenerateLnnCredential && context->accessee.bindLevel != static_cast<int32_t>(USER)) {
        if (!jsonData[TAG_LNN_PUBLIC_KEY].IsString()) {
            LOGE("DmAuthMessageProcessor::ParseMessageReqCredExchange() error, first auth, no lnnPublicKey.");
            return ERR_DM_FAILED;
        }
        context->accesser.lnnPublicKey = jsonData[TAG_LNN_PUBLIC_KEY].Get<std::string>();
    }

    if (!jsonData[TAG_TRANSMIT_PUBLIC_KEY].IsString() ||
        !jsonData[TAG_DEVICE_ID].IsString() ||
        !jsonData[TAG_PEER_USER_SPACE_ID].IsNumberInteger() ||
        !jsonData[TAG_TOKEN_ID].IsNumberInteger()) {
        LOGE("DmAuthMessageProcessor::ParseMessageReqCredExchange, MSG_TYPE_REQ_CREDENTIAL_EXCHANGE message error.");
        return ERR_DM_FAILED;
    }
    context->accesser.transmitPublicKey = jsonData[TAG_TRANSMIT_PUBLIC_KEY].Get<std::string>();
    context->accesser.deviceId = jsonData[TAG_DEVICE_ID].Get<std::string>();
    context->accesser.userId = jsonData[TAG_PEER_USER_SPACE_ID].Get<int32_t>();
    context->accesser.tokenId = jsonData[TAG_TOKEN_ID].Get<int64_t>();
    ParseProxyCredExchangeToSync(context, jsonData);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkCredentialExchangeState>());
    return DM_OK;
}

// parse 150
int32_t DmAuthMessageProcessor::ParseMessageRspCredExchange(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    LOGI("DmAuthMessageProcessor::ParseMessageRspCredExchange start.");
    if (jsonObject.IsDiscarded() || !jsonObject[TAG_DATA].IsString()) {
        LOGE("DmAuthMessageProcessor::ParseMessageRspCredExchange, DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }

    std::string plainText;
    if (cryptoMgr_->DecryptMessage(jsonObject[TAG_DATA].Get<std::string>(), plainText) != DM_OK) {
        LOGE("DmAuthMessageProcessor::ParseMessageRspCredExchange error, decrypt data failed.");
        return ERR_DM_FAILED;
    }
    LOGI("plainText=%{public}s", GetAnonyJsonString(plainText).c_str());

    JsonObject jsonData(plainText);

    // First authentication, parse lnn public key
    std::string tmpString = "";
    if (context->accesser.isGenerateLnnCredential && context->accesser.bindLevel != static_cast<int32_t>(USER)) {
        if (!jsonData[TAG_LNN_PUBLIC_KEY].IsString()) {
            LOGE("DmAuthMessageProcessor::ParseMessageRspCredExchange failed, first auth but no lnnPublicKey.");
            return ERR_DM_FAILED;
        }
        context->accessee.lnnPublicKey = jsonData[TAG_LNN_PUBLIC_KEY].Get<std::string>();
    }

    // First authentication, parse transmit public key
    if (!jsonData[TAG_TRANSMIT_PUBLIC_KEY].IsString() ||
        !jsonData[TAG_DEVICE_ID].IsString() ||
        !jsonData[TAG_PEER_USER_SPACE_ID].IsNumberInteger() ||
        !jsonData[TAG_TOKEN_ID].IsNumberInteger()) {
        LOGE("DmAuthMessageProcessor::ParseMessageRspCredExchange failed, decode MSG_TYPE_RESP_CREDENTIAL_EXCHANGE "
            "message error.");
        return ERR_DM_FAILED;
    }
    context->accessee.transmitPublicKey = jsonData[TAG_TRANSMIT_PUBLIC_KEY].Get<std::string>();
    context->accessee.deviceId = jsonData[TAG_DEVICE_ID].Get<std::string>();
    context->accessee.userId = jsonData[TAG_PEER_USER_SPACE_ID].Get<int32_t>();
    context->accessee.tokenId = jsonData[TAG_TOKEN_ID].Get<int64_t>();
    ParseProxyCredExchangeToSync(context, jsonData);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcCredentialAuthStartState>());
    return DM_OK;
}

// parse 141
int32_t DmAuthMessageProcessor::ParseMessageReqSKDerive(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(cryptoMgr_, ERR_DM_POINT_NULL);
    if (jsonObject.IsDiscarded() || !jsonObject[TAG_DATA].IsString()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }
    std::string plainText;
    if (cryptoMgr_->DecryptMessage(jsonObject[TAG_DATA].Get<std::string>(), plainText) != DM_OK) {
        LOGE("DmAuthMessageProcessor::ParseMessageReqSKDerive() error, decrypt data failed.");
        return ERR_DM_FAILED;
    }
    JsonObject jsonData(plainText);
    // First authentication, parse lnn public key
    if (context->accessee.isGenerateLnnCredential && context->accessee.bindLevel != static_cast<int32_t>(USER)) {
        if (!jsonData[TAG_LNN_CREDENTIAL_ID].IsString()) {
            LOGE("DmAuthMessageProcessor::ParseMessageReqSKDerive() error, first auth, no lnnPublicKey.");
            return ERR_DM_FAILED;
        }
        context->accesser.lnnCredentialId = jsonData[TAG_LNN_CREDENTIAL_ID].Get<std::string>();
    }
    if (!jsonData[TAG_TRANSMIT_CREDENTIAL_ID].IsString()) {
        LOGE("DmAuthMessageProcessor::ParseMessageReqSKDerive, MSG_TYPE_REQ_CREDENTIAL_EXCHANGE message error.");
        return ERR_DM_FAILED;
    }
    context->accesser.transmitCredentialId = jsonData[TAG_TRANSMIT_CREDENTIAL_ID].Get<std::string>();
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkSKDeriveState>());
    return DM_OK;
}

// parse 151
int32_t DmAuthMessageProcessor::ParseMessageRspSKDerive(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(cryptoMgr_, ERR_DM_POINT_NULL);
    if (jsonObject.IsDiscarded() || !jsonObject[TAG_DATA].IsString()) {
        LOGE("DecodeRequestAuth jsonStr error");
        return ERR_DM_FAILED;
    }
    std::string plainText;
    if (cryptoMgr_->DecryptMessage(jsonObject[TAG_DATA].Get<std::string>(), plainText) != DM_OK) {
        LOGE("DmAuthMessageProcessor::ParseMessageRspSKDerive() error, decrypt data failed.");
        return ERR_DM_FAILED;
    }
    JsonObject jsonData(plainText);
    // First authentication, parse lnn public key
    if (context->accesser.isGenerateLnnCredential && context->accesser.bindLevel != static_cast<int32_t>(USER)) {
        if (!jsonData[TAG_LNN_CREDENTIAL_ID].IsString()) {
            LOGE("DmAuthMessageProcessor::ParseMessageRspSKDerive() error, first auth, no lnnPublicKey.");
            return ERR_DM_FAILED;
        }
        context->accessee.lnnCredentialId = jsonData[TAG_LNN_CREDENTIAL_ID].Get<std::string>();
    }
    if (!jsonData[TAG_TRANSMIT_CREDENTIAL_ID].IsString()) {
        LOGE("DmAuthMessageProcessor::ParseMessageRspSKDerive, MSG_TYPE_REQ_CREDENTIAL_EXCHANGE message error.");
        return ERR_DM_FAILED;
    }
    context->accessee.transmitCredentialId = jsonData[TAG_TRANSMIT_CREDENTIAL_ID].Get<std::string>();
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcSKDeriveState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseProxyCredExchangeToSync(std::shared_ptr<DmAuthContext> &context,
    JsonObject &jsonObject)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return DM_OK;
    }
    if (!IsString(jsonObject, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        LOGE("no subjectProxyOnes");
        return ERR_DM_FAILED;
    }
    std::string subjectProxyOnesStr = jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_PROXY_CONTEXT_ID)) {
            continue;
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = item[TAG_PROXY_CONTEXT_ID].Get<std::string>();
        auto it = std::find(context->subjectProxyOnes.begin(), context->subjectProxyOnes.end(), proxyAuthContext);
        if (it != context->subjectProxyOnes.end()) {
            if (!IsInt64(item, TAG_TOKEN_ID)) {
                LOGE("no tokenId");
                return ERR_DM_FAILED;
            }
            DmProxyAccess &access = (context->direction == DM_AUTH_SOURCE) ? it->proxyAccessee : it->proxyAccesser;
            access.tokenId = item[TAG_TOKEN_ID].Get<int64_t>();
        }
    }
    return DM_OK;
}

std::string DmAuthMessageProcessor::CreateMessage(DmMessageType msgType, std::shared_ptr<DmAuthContext> context)
{
    LOGI("DmAuthMessageProcessor::CreateMessage start. msgType is %{public}d", msgType);
    JsonObject jsonObj;
    jsonObj[TAG_MSG_TYPE] = msgType;
    jsonObj[DM_TAG_LOGICAL_SESSION_ID] = context->logicalSessionId;
    auto itr = createMessageFuncMap_.find(msgType);
    if (itr == createMessageFuncMap_.end()) {
        LOGE("DmAuthMessageProcessor::CreateMessage msgType %{public}d error.", msgType);
        return "";
    }
    int32_t ret = (this->*(itr->second))(context, jsonObj);
    LOGI("start. message is %{public}s", GetAnonyJsonString(jsonObj.Dump()).c_str());
    return (ret == DM_OK) ? jsonObj.Dump() : "";
}

int32_t DmAuthMessageProcessor::CreateCredentialNegotiateMessage(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    std::string encryptMsg;
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateNegotiateOldMessage(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_CRYPTO_SUPPORT] = false;
    jsonObject[TAG_AUTH_TYPE] = context->authType;
    jsonObject[TAG_REPLY] = context->reply;
    jsonObject[TAG_LOCAL_DEVICE_ID] = context->accesser.deviceId;
    jsonObject[TAG_ACCOUNT_GROUPID] = context->accesser.accountGroupIdHash;

    jsonObject[TAG_BIND_LEVEL] = context->accesser.oldBindLevel;    // compatible issue
    jsonObject[TAG_LOCAL_ACCOUNTID] = context->accesser.accountId;
    jsonObject[TAG_LOCAL_USERID] = context->accesser.userId;
    jsonObject[TAG_ISONLINE] = false;
    jsonObject[TAG_AUTHED] = false;
    jsonObject[TAG_HOST] = context->pkgName;
    jsonObject[TAG_TOKENID] = context->accesser.tokenId;
    jsonObject[TAG_IDENTICAL_ACCOUNT] = false;
    jsonObject[TAG_HAVE_CREDENTIAL] = false;
    jsonObject[TAG_REMOTE_DEVICE_NAME] = context->accesser.deviceName;

    return DM_OK;
}

// Create 80 message.
int32_t DmAuthMessageProcessor::CreateNegotiateMessage(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    // create old message for compatible in import auth code
    if (context->authType == DmAuthType::AUTH_TYPE_IMPORT_AUTH_CODE) {
        CreateNegotiateOldMessage(context, jsonObject);
    }
    jsonObject[TAG_DMVERSION] = "";
    jsonObject[TAG_EDITION] = DM_VERSION_5_0_5;
    jsonObject[TAG_BUNDLE_NAME] = context->accesser.bundleName;
    jsonObject[TAG_PEER_BUNDLE_NAME] = context->accessee.oldBundleName;

    jsonObject[TAG_PKG_NAME] = context->pkgName;

    jsonObject[TAG_DM_VERSION_V2] = context->accesser.dmVersion;
    jsonObject[TAG_USER_ID] = context->accesser.userId;
    jsonObject[TAG_DEVICE_NAME] = context->accesser.deviceName;
    jsonObject[TAG_DEVICE_ID_HASH] = context->accesser.deviceIdHash;
    jsonObject[TAG_ACCOUNT_ID_HASH] = context->accesser.accountIdHash;
    jsonObject[TAG_TOKEN_ID_HASH] = context->accesser.tokenIdHash;
    jsonObject[TAG_BUNDLE_NAME_V2] = context->accesser.bundleName;
    jsonObject[TAG_EXTRA_INFO] = context->accesser.extraInfo;

    jsonObject[TAG_PEER_BUNDLE_NAME_V2] = context->accessee.bundleName;
    jsonObject[TAG_ULTRASONIC_SIDE] = static_cast<int32_t>(context->ultrasonicInfo);
    jsonObject[TAG_PEER_DISPLAY_ID] = context->accessee.displayId;
    jsonObject[TAG_PEER_PKG_NAME] = context->accessee.pkgName;
    jsonObject[TAG_HOST_PKGLABEL] = context->pkgLabel;

    if (!context->businessId.empty()) {
        jsonObject[DM_BUSINESS_ID] = context->businessId;
    }
    CreateProxyNegotiateMessage(context, jsonObject);
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateProxyNegotiateMessage(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    jsonObject[PARAM_KEY_IS_PROXY_BIND] = context->IsProxyBind;
    jsonObject[PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT] = context->IsCallingProxyAsSubject;
    if (context != nullptr && context->IsProxyBind && !context->subjectProxyOnes.empty()) {
        JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
        for (const auto &app : context->subjectProxyOnes) {
            JsonObject object;
            object[TAG_PROXY_CONTEXT_ID] = app.proxyContextId;
            object[TAG_HOST_PKGLABEL] = app.pkgLabel;
            object[TAG_BUNDLE_NAME] = app.proxyAccesser.bundleName;
            object[TAG_PEER_BUNDLE_NAME] = app.proxyAccessee.bundleName;
            object[TAG_TOKEN_ID_HASH] = app.proxyAccesser.tokenIdHash;
            allProxyObj.PushBack(object);
        }
        jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = allProxyObj.Dump();
    }
    return DM_OK;
}

// Create 90 message.
int32_t DmAuthMessageProcessor::CreateRespNegotiateMessage(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_DEVICE_VERSION] = context->accessee.dmVersion;
    jsonObject[TAG_DEVICE_NAME] = context->accessee.deviceName;

    jsonObject[TAG_DEVICE_ID_HASH] = context->accessee.deviceIdHash;
    jsonObject[TAG_ACCOUNT_ID_HASH] = context->accessee.accountIdHash;
    jsonObject[TAG_TOKEN_ID_HASH] = context->accessee.tokenIdHash;
    jsonObject[TAG_USER_ID] = context->accessee.userId;
    jsonObject[TAG_ACL_TYPE_LIST] = context->accessee.aclTypeList;
    jsonObject[TAG_CERT_TYPE_LIST] = context->accessee.credTypeList;
    jsonObject[TAG_LANGUAGE] = context->accessee.language;
    jsonObject[TAG_EXTRA_INFO] = context->accessee.extraInfo;
    jsonObject[TAG_NETWORKID_ID] = context->accessee.networkId;

    jsonObject[TAG_IS_ONLINE] = context->accesser.isOnline;
    CreateProxyRespNegotiateMessage(context, jsonObject);
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateProxyRespNegotiateMessage(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    jsonObject[PARAM_KEY_IS_PROXY_BIND] = context->IsProxyBind;
    if (context->IsProxyBind && !context->subjectProxyOnes.empty()) {
        JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
        for (const auto &app : context->subjectProxyOnes) {
            JsonObject object;
            object[TAG_PROXY_CONTEXT_ID] = app.proxyContextId;
            object[TAG_TOKEN_ID_HASH] = app.proxyAccessee.tokenIdHash;
            object[TAG_ACL_TYPE_LIST] = app.proxyAccessee.aclTypeList;
            object[TAG_CERT_TYPE_LIST] = app.proxyAccessee.credTypeList;
            allProxyObj.PushBack(object);
        }
        jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = allProxyObj.Dump();
    }
    return DM_OK;
}

// Create 140 message.
int32_t DmAuthMessageProcessor::CreateMessageReqCredExchange(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    JsonObject jsonData;
    if (context->accesser.isGenerateLnnCredential && context->accesser.bindLevel != static_cast<int32_t>(USER)) {
        jsonData[TAG_LNN_PUBLIC_KEY] = context->accesser.lnnPublicKey;
    }
    jsonData[TAG_TRANSMIT_PUBLIC_KEY] = context->accesser.transmitPublicKey;
    jsonData[TAG_DEVICE_ID] = context->accesser.deviceId;
    jsonData[TAG_PEER_USER_SPACE_ID] = context->accesser.userId;
    jsonData[TAG_TOKEN_ID] = context->accesser.tokenId;
    CreateProxyCredExchangeMessage(context, jsonData);
    std::string plainText = jsonData.Dump();
    std::string cipherText;
    int32_t ret = cryptoMgr_->EncryptMessage(plainText, cipherText);
    if (ret != DM_OK) {
        LOGI("DmAuthMessageProcessor::CreateMessageReqCredExchange encryptMessage failed.");
        return ret;
    }
    jsonObject[TAG_DATA] = cipherText;
    return ret;
}

// Create 150 message.
int32_t DmAuthMessageProcessor::CreateMessageRspCredExchange(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    LOGI("DmAuthMessageProcessor::CreateMessageRspCredExchange start.");
    JsonObject jsonData;
    if (context->accessee.isGenerateLnnCredential && context->accessee.bindLevel != static_cast<int32_t>(USER)) {
        jsonData[TAG_LNN_PUBLIC_KEY] = context->accessee.lnnPublicKey;
    }
    jsonData[TAG_TRANSMIT_PUBLIC_KEY] = context->accessee.transmitPublicKey;
    jsonData[TAG_DEVICE_ID] = context->accessee.deviceId;
    jsonData[TAG_PEER_USER_SPACE_ID] = context->accessee.userId;
    jsonData[TAG_TOKEN_ID] = context->accessee.tokenId;
    CreateProxyCredExchangeMessage(context, jsonData);
    std::string plainText = jsonData.Dump();
    std::string cipherText;
    LOGI("plainText=%{public}s", GetAnonyJsonString(plainText).c_str());
    int32_t ret = cryptoMgr_->EncryptMessage(plainText, cipherText);
    if (ret != DM_OK) {
        LOGI("DmAuthMessageProcessor::CreateMessageRspCredExchange encryptMessage failed.");
        return ret;
    }
    jsonObject[TAG_DATA] = cipherText;
    return ret;
}

int32_t DmAuthMessageProcessor::CreateProxyCredExchangeMessage(std::shared_ptr<DmAuthContext> &context,
    JsonObject &jsonData)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return DM_OK;
    }
    JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    for (const auto &app : context->subjectProxyOnes) {
        const DmProxyAccess &access = (context->direction == DM_AUTH_SOURCE) ? app.proxyAccesser : app.proxyAccessee;
        JsonObject object;
        object[TAG_PROXY_CONTEXT_ID] = app.proxyContextId;
        object[TAG_TOKEN_ID] = access.tokenId;
        allProxyObj.PushBack(object);
    }
    jsonData[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = allProxyObj.Dump();
    return DM_OK;
}

// Create 141 message.
int32_t DmAuthMessageProcessor::CreateMessageReqSKDerive(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(cryptoMgr_, ERR_DM_POINT_NULL);
    JsonObject jsonData;
    jsonData[TAG_TRANSMIT_CREDENTIAL_ID] = context->accesser.transmitCredentialId;
    // First certification
    if (context->accesser.isGenerateLnnCredential && context->accesser.bindLevel != static_cast<int32_t>(USER)) {
        jsonData[TAG_LNN_CREDENTIAL_ID] = context->accesser.lnnCredentialId;
    }
    std::string plainText = jsonData.Dump();
    std::string cipherText;
    int32_t ret = cryptoMgr_->EncryptMessage(plainText, cipherText);
    if (ret != DM_OK) {
        LOGI("DmAuthMessageProcessor::CreateMessageReqCredExchange encryptMessage failed.");
        return ret;
    }
    jsonObject[TAG_DATA] = cipherText;
    return ret;
}

// Create 151 message.
int32_t DmAuthMessageProcessor::CreateMessageRspSKDerive(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    CHECK_NULL_RETURN(cryptoMgr_, ERR_DM_POINT_NULL);
    JsonObject jsonData;
    jsonData[TAG_TRANSMIT_CREDENTIAL_ID] = context->accessee.transmitCredentialId;
    // First certification
    if (context->accessee.isGenerateLnnCredential && context->accessee.bindLevel != static_cast<int32_t>(USER)) {
        jsonData[TAG_LNN_CREDENTIAL_ID] = context->accessee.lnnCredentialId;
    }
    std::string plainText = jsonData.Dump();
    std::string cipherText;
    int32_t ret = cryptoMgr_->EncryptMessage(plainText, cipherText);
    if (ret != DM_OK) {
        LOGI("DmAuthMessageProcessor::CreateMessageReqCredExchange encryptMessage failed.");
        return ret;
    }
    jsonObject[TAG_DATA] = cipherText;
    return ret;
}

// Create 160 message.
int32_t DmAuthMessageProcessor::CreateMessageReqCredAuthStart(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    std::string onTransmitData;

    JsonObject jsonData;
    jsonObject[TAG_DATA] = context->transmitData;
    return DM_OK;
}

// Create 190 message.
int32_t DmAuthMessageProcessor::CreateMessageSyncResp(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    DmAccess access;
    if (context->direction == DM_AUTH_SINK) {
        access = context->accessee;
    } else {
        access = context->accesser;
    }

    std::string encSyncMsg;
    int32_t ret = EncryptSyncMessage(context, access, encSyncMsg);
    if (ret != DM_OK) {
        LOGE("DmAuthMessageProcessor::CreateMessageSyncResp encrypt failed");
        return ret;
    }
    jsonObject[TAG_SYNC] = encSyncMsg;
    return ret;
}

// Create 200 message.
int32_t DmAuthMessageProcessor::CreateMessageFinish(std::shared_ptr<DmAuthContext> context,
    JsonObject &jsonObject)
{
    jsonObject[TAG_REPLY] = context->reply;
    jsonObject[TAG_REASON] = context->reason;
    jsonObject[TAG_REMAINING_FROZEN_TIME] = context->remainingFrozenTime;
    return DM_OK;
}

bool DmAuthMessageProcessor::CheckAccessValidityAndAssign(std::shared_ptr<DmAuthContext> &context, DmAccess &access,
    DmAccess &accessTmp)
{
    const DmAccess &selfAccess = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;

    bool isSame = accessTmp.dmVersion == access.dmVersion &&
        accessTmp.deviceName == access.deviceName &&
        Crypto::GetUdidHash(accessTmp.deviceId) == access.deviceIdHash &&
        accessTmp.userId == access.userId &&
        Crypto::GetAccountIdHash16(accessTmp.accountId) == access.accountIdHash &&
        Crypto::GetTokenIdHash(std::to_string(accessTmp.tokenId)) == access.tokenIdHash &&
        accessTmp.bundleName == access.bundleName &&
        accessTmp.pkgName == access.pkgName &&
        accessTmp.bindLevel == selfAccess.bindLevel;
    if (isSame) {
        access.transmitSessionKeyId = accessTmp.transmitSessionKeyId;
        access.transmitSkTimeStamp = accessTmp.transmitSkTimeStamp;
        access.transmitCredentialId = accessTmp.transmitCredentialId;
        access.lnnSessionKeyId = accessTmp.lnnSessionKeyId;
        access.lnnSkTimeStamp = accessTmp.lnnSkTimeStamp;
        access.lnnCredentialId = accessTmp.lnnCredentialId;
        access.tokenId = accessTmp.tokenId;
    }
    return isSame;
}

int32_t DmAuthMessageProcessor::ParseSyncMessage(std::shared_ptr<DmAuthContext> &context,
    DmAccess &access, JsonObject &jsonObject)
{
    DmAccess accessTmp;
    if (ParseInfoToDmAccess(jsonObject, accessTmp) != DM_OK) {
        LOGE("Parse DataSync prarm err");
        return ERR_DM_FAILED;
    }

    if (!IsString(jsonObject, TAG_ACCESS)) {
        LOGE("ParseSyncMessage TAG_ACCESS error");
        return ERR_DM_FAILED;
    }
    std::string srcAccessStr = jsonObject[TAG_ACCESS].Get<std::string>();
    // Parse into access
    ParseDmAccessToSync(srcAccessStr, accessTmp, false);
    // check access validity
    if (ParseProxyAccessToSync(context, jsonObject) != DM_OK) {
        LOGE("ParseProxyAccessToSync error, stop auth.");
        return ERR_DM_FAILED;
    }
    if (!CheckAccessValidityAndAssign(context, access, accessTmp)) {
        LOGE("ParseSyncMessage CheckAccessValidityAndAssign error, data between two stages different, stop auth.");
        return ERR_DM_FAILED;
    }
    ParseDmAccessToSync(srcAccessStr, access, true);
    if (!IsString(jsonObject, TAG_ACL_CHECKSUM)) { // Re-parse the acl
        LOGE("ParseSyncMessage TAG_ACL_CHECKSUM error");
        return ERR_DM_FAILED;
    }
    access.aclStrList = jsonObject[TAG_ACL_CHECKSUM].Get<std::string>();
    if (context->direction == DmAuthDirection::DM_AUTH_SOURCE) {
        LOGI("Source parse sink user confirm opt");
        int32_t userConfirmOpt = static_cast<int32_t>(USER_OPERATION_TYPE_CANCEL_AUTH);
        if (IsInt32(jsonObject, TAG_USER_CONFIRM_OPT)) {
            userConfirmOpt = jsonObject[TAG_USER_CONFIRM_OPT].Get<int32_t>();
        }
        if (userConfirmOpt == static_cast<int32_t>(USER_OPERATION_TYPE_ALLOW_AUTH) ||
            userConfirmOpt == static_cast<int32_t>(USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS)) {
            context->confirmOperation = static_cast<UiAction>(userConfirmOpt);
        }
    }
    ParseCert(jsonObject, context);
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseProxyAccessToSync(std::shared_ptr<DmAuthContext> &context,
    JsonObject &jsonObject)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return DM_OK;
    }
    if (!IsString(jsonObject, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        LOGE("no subjectProxyOnes");
        return ERR_DM_FAILED;
    }
    std::string subjectProxyOnesStr = jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_PROXY_CONTEXT_ID)) {
            LOGE("no proxyContextId");
            return ERR_DM_FAILED;
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = item[TAG_PROXY_CONTEXT_ID].Get<std::string>();
        auto it = std::find(context->subjectProxyOnes.begin(), context->subjectProxyOnes.end(), proxyAuthContext);
        if (it != context->subjectProxyOnes.end()) {
            if (!IsInt64(item, TAG_TOKEN_ID) || !IsString(item, TAG_TRANSMIT_SK_ID) ||
                !IsInt32(item, TAG_BIND_LEVEL) || !IsInt64(item, TAG_TRANSMIT_SK_TIMESTAMP) ||
                !IsString(item, TAG_TRANSMIT_CREDENTIAL_ID)) {
                LOGE("proxyContext format error");
                return ERR_DM_FAILED;
            }

            DmProxyAccess &access = (context->direction == DM_AUTH_SOURCE) ? it->proxyAccessee : it->proxyAccesser;
            DmProxyAccess &selfAccess = (context->direction == DM_AUTH_SOURCE) ? it->proxyAccesser : it->proxyAccessee;
            if (Crypto::GetTokenIdHash(std::to_string(item[TAG_TOKEN_ID].Get<int64_t>())) == access.tokenIdHash &&
                item[TAG_BIND_LEVEL].Get<int32_t>() == selfAccess.bindLevel) {
                access.tokenId  = item[TAG_TOKEN_ID].Get<int64_t>();
                access.bindLevel = item[TAG_BIND_LEVEL].Get<int32_t>();
                access.transmitSessionKeyId = std::atoi(item[TAG_TRANSMIT_SK_ID].Get<std::string>().c_str());
                access.skTimeStamp = item[TAG_TRANSMIT_SK_TIMESTAMP].Get<int64_t>();
                access.transmitCredentialId  = item[TAG_TRANSMIT_CREDENTIAL_ID].Get<std::string>();
            } else {
                LOGE("tokenId or bindLevel invaild");
                return ERR_DM_FAILED;
            }
        } else {
            LOGE("proxyContextId not exist");
            return ERR_DM_FAILED;
        }
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::DecryptSyncMessage(std::shared_ptr<DmAuthContext> &context,
    DmAccess &access, std::string &enSyncMsg)
{
    std::string syncMsgCompress = "";
    int32_t ret = cryptoMgr_->DecryptMessage(enSyncMsg, syncMsgCompress);
    if (ret != DM_OK) {
        LOGE("syncMsg error");
        return ret;
    }
    JsonObject plainJson(syncMsgCompress);
    if (plainJson.IsDiscarded()) {
        LOGE("plainJson error");
        return ERR_DM_FAILED;
    }
    if (!plainJson[TAG_COMPRESS_ORI_LEN].IsNumberInteger()) {
        LOGE("TAG_COMPRESS_ORI_LEN json error");
        return ERR_DM_FAILED;
    }
    int32_t dataLen = plainJson[TAG_COMPRESS_ORI_LEN].Get<int32_t>();
    if (!plainJson[TAG_COMPRESS].IsString()) {
        LOGE("TAG_COMPRESS_ORI_LEN json error");
        return ERR_DM_FAILED;
    }
    std::string compressMsg = plainJson[TAG_COMPRESS].Get<std::string>();
    std::string compressBase64 = Base64Decode(compressMsg);
    std::string syncMsg = DecompressSyncMsg(compressBase64, dataLen);
    JsonObject jsonObject(syncMsg);
    if (jsonObject.IsDiscarded()) {
        LOGE("jsonStr error");
        return ERR_DM_FAILED;
    }

    ret = ParseSyncMessage(context, access, jsonObject);
    if (ret != DM_OK) {
        LOGE("ParseSyncMessage jsonStr error");
        return ret;
    }
    return DM_OK;
}

// Parse 180 message, save remote encrypted quadruple, acl, sp skid
int32_t DmAuthMessageProcessor::ParseMessageSyncReq(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (!jsonObject[TAG_SYNC].IsString()) {
        LOGE("ParseMessageSyncReq json error");
        return ERR_DM_FAILED;
    }
    std::string enSyncMsg = jsonObject[TAG_SYNC].Get<std::string>();
    // Decrypt data and parse data into context
    int32_t ret = DecryptSyncMessage(context, context->accesser, enSyncMsg);
    if (ret != DM_OK) {
        LOGE("DecryptSyncMessage enSyncMsg error");
        return ret;
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkDataSyncState>());
    return DM_OK;
}

// Parse 190 message save the remote encrypted quadruple, acl sp skid
int32_t DmAuthMessageProcessor::ParseMessageSyncResp(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (!jsonObject[TAG_SYNC].IsString()) {
        LOGE("ParseMessageSyncResp json error");
        return ERR_DM_FAILED;
    }
    std::string enSyncMsg = jsonObject[TAG_SYNC].Get<std::string>();
    // Decrypt data and parse data into context
    int32_t ret = DecryptSyncMessage(context, context->accessee, enSyncMsg);
    if (ret != DM_OK) {
        LOGE("DecryptSyncMessage enSyncMsg error");
        return ret;
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcDataSyncState>());
    return DM_OK;
}

// Parse 200 message
int32_t DmAuthMessageProcessor::ParseMessageSinkFinish(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject[TAG_REPLY].IsNumberInteger()) {
        context->reply = jsonObject[TAG_REPLY].Get<int32_t>();
    }
    if (jsonObject[TAG_REASON].IsNumberInteger()) {
        context->reason = jsonObject[TAG_REASON].Get<int32_t>();
    }

    /* In case of an exception, there may be a state waiting for an event.
    In the normal process, no state is waiting for events. */
    if (context->reason != DM_OK) {
        context->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
    return DM_OK;
}

// Parse 201 message
int32_t DmAuthMessageProcessor::ParseMessageSrcFinish(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (IsInt32(jsonObject, TAG_REPLY)) {
        context->reply = jsonObject[TAG_REPLY].Get<int32_t>();
    }
    if (IsInt32(jsonObject, TAG_REASON)) {
        context->reason = jsonObject[TAG_REASON].Get<int32_t>();
    }
    if (IsInt64(jsonObject, TAG_REMAINING_FROZEN_TIME)) {
        context->remainingFrozenTime = jsonObject[TAG_REMAINING_FROZEN_TIME].Get<int64_t>();
    }

    /* In case of an exception, there may be a state waiting for an event.
    In the normal process, no state is waiting for events. */
    if (context->reason != DM_OK) {
        context->authStateMachine->NotifyEventFinish(DmEventType::ON_FAIL);
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CheckLogicalSessionId(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject[DM_TAG_LOGICAL_SESSION_ID].IsNumberInteger()) {
        if (context->logicalSessionId != 0 &&
            jsonObject[DM_TAG_LOGICAL_SESSION_ID].Get<uint64_t>() != context->logicalSessionId) {
            return ERR_DM_PARSE_MESSAGE_FAILED;
        }
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseNegotiateMessage(
    const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject[DM_TAG_LOGICAL_SESSION_ID].IsNumberInteger()) {
        context->logicalSessionId = jsonObject[DM_TAG_LOGICAL_SESSION_ID].Get<uint64_t>();
        context->requestId = static_cast<int64_t>(context->logicalSessionId);
    }
    if (IsString(jsonObject, TAG_PEER_BUNDLE_NAME_V2)) {
        context->accessee.bundleName = jsonObject[TAG_PEER_BUNDLE_NAME_V2].Get<std::string>();
    }
    if (IsInt32(jsonObject, TAG_PEER_DISPLAY_ID)) {
        context->accessee.displayId = jsonObject[TAG_PEER_DISPLAY_ID].Get<int32_t>();
    }
    if (IsString(jsonObject, TAG_HOST_PKGLABEL)) {
        context->pkgLabel = jsonObject[TAG_HOST_PKGLABEL].Get<std::string>();
    }
    if (IsString(jsonObject, DM_BUSINESS_ID)) {
        context->businessId = jsonObject[DM_BUSINESS_ID].Get<std::string>();
    }
    ParseAccesserInfo(jsonObject, context);
    ParseUltrasonicSide(jsonObject, context);
    ParseProxyNegotiateMessage(jsonObject, context);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkNegotiateStateMachine>());
    return DM_OK;
}

void DmAuthMessageProcessor::ParseAccesserInfo(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject[TAG_PKG_NAME].IsString()) {
        context->pkgName = jsonObject[TAG_PKG_NAME].Get<std::string>();
        context->accesser.pkgName = context->pkgName;
        context->accessee.pkgName = context->accesser.pkgName;
    }
    if (jsonObject[TAG_PEER_PKG_NAME].IsString()) {
        context->accessee.pkgName = jsonObject[TAG_PEER_PKG_NAME].Get<std::string>();
    }
    if (jsonObject[TAG_DM_VERSION_V2].IsString()) {
        context->accesser.dmVersion = jsonObject[TAG_DM_VERSION_V2].Get<std::string>();
    }
    if (jsonObject[TAG_USER_ID].IsNumberInteger()) {
        context->accesser.userId = jsonObject[TAG_USER_ID].Get<int32_t>();
    }
    if (jsonObject[TAG_DEVICE_ID_HASH].IsString()) {
        context->accesser.deviceIdHash = jsonObject[TAG_DEVICE_ID_HASH].Get<std::string>();
    }
    if (jsonObject[TAG_ACCOUNT_ID_HASH].IsString()) {
        context->accesser.accountIdHash = jsonObject[TAG_ACCOUNT_ID_HASH].Get<std::string>();
    }
    if (jsonObject[TAG_TOKEN_ID_HASH].IsString()) {
        context->accesser.tokenIdHash = jsonObject[TAG_TOKEN_ID_HASH].Get<std::string>();
    }
    if (jsonObject[TAG_BUNDLE_NAME_V2].IsString()) {
        context->accesser.bundleName = jsonObject[TAG_BUNDLE_NAME_V2].Get<std::string>();
    }
    if (jsonObject[TAG_EXTRA_INFO].IsString()) {
        context->accesser.extraInfo = jsonObject[TAG_EXTRA_INFO].Get<std::string>();
    }
}

void DmAuthMessageProcessor::ParseCert(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject[TAG_DM_CERT_CHAIN].IsString()) {
        context->accesser.cert = jsonObject[TAG_DM_CERT_CHAIN].Get<std::string>();
    }
    if (jsonObject[TAG_IS_COMMON_FLAG].IsBoolean()) {
        context->accesser.isCommonFlag = jsonObject[TAG_IS_COMMON_FLAG].Get<bool>();
    }
}

int32_t DmAuthMessageProcessor::ParseProxyNegotiateMessage(
    const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    context->IsProxyBind = false;
    //accesser dmVersion greater than DM_VERSION_5_1_1
    if (CompareVersion(context->accesser.dmVersion, DM_VERSION_5_1_1) && IsBool(jsonObject, PARAM_KEY_IS_PROXY_BIND)) {
        context->IsProxyBind = jsonObject[PARAM_KEY_IS_PROXY_BIND].Get<bool>();
    }
    if (!context->IsProxyBind) {
        return DM_OK;
    }
    if (IsBool(jsonObject, PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT)) {
        context->IsCallingProxyAsSubject = jsonObject[PARAM_KEY_IS_CALLING_PROXY_AS_SUBJECT].Get<bool>();
    }

    if (!IsString(jsonObject, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string subjectProxyOnesStr = jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_PROXY_CONTEXT_ID) || !IsString(item, TAG_BUNDLE_NAME) ||
            !IsString(item, TAG_PEER_BUNDLE_NAME) || !IsString(item, TAG_TOKEN_ID_HASH)) {
            continue;
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = item[TAG_PROXY_CONTEXT_ID].Get<std::string>();
        proxyAuthContext.proxyAccesser.bundleName = item[TAG_BUNDLE_NAME].Get<std::string>();
        proxyAuthContext.proxyAccessee.bundleName = item[TAG_PEER_BUNDLE_NAME].Get<std::string>();
        proxyAuthContext.proxyAccesser.tokenIdHash = item[TAG_TOKEN_ID_HASH].Get<std::string>();
        if (IsString(item, TAG_HOST_PKGLABEL)) {
            proxyAuthContext.pkgLabel = item[TAG_HOST_PKGLABEL].Get<std::string>();
        }
        context->subjectProxyOnes.push_back(proxyAuthContext);
    }
    return DM_OK;
}

void DmAuthMessageProcessor::ParseUltrasonicSide(
    const JsonObject &jsonObject, std::shared_ptr<DmAuthContext> context)
{
    if (!jsonObject[TAG_ULTRASONIC_SIDE].IsNumberInteger()) {
        context->ultrasonicInfo = DmUltrasonicInfo::DM_Ultrasonic_Invalid;
        return;
    }
    int32_t tempInfo = jsonObject[TAG_ULTRASONIC_SIDE].Get<int32_t>();
    if (tempInfo == DM_ULTRASONIC_REVERSE) {
        context->ultrasonicInfo = DmUltrasonicInfo::DM_Ultrasonic_Reverse;
    } else if (tempInfo == DM_ULTRASONIC_FORWARD) {
        context->ultrasonicInfo = DmUltrasonicInfo::DM_Ultrasonic_Forward;
    } else {
        context->ultrasonicInfo = DmUltrasonicInfo::DM_Ultrasonic_Invalid;
    }
}

int32_t DmAuthMessageProcessor::ParseMessageRespAclNegotiate(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject[TAG_DEVICE_VERSION].IsString()) {
        context->accessee.dmVersion = jsonObject[TAG_DEVICE_VERSION].Get<std::string>();
    }

    if (jsonObject[TAG_DEVICE_NAME].IsString()) {
        context->accessee.deviceName = jsonObject[TAG_DEVICE_NAME].Get<std::string>();
    }

    if (jsonObject[TAG_DEVICE_ID_HASH].IsString()) {
        context->accessee.deviceIdHash = jsonObject[TAG_DEVICE_ID_HASH].Get<std::string>();
    }

    if (jsonObject[TAG_USER_ID].IsNumberInteger()) {
        context->accessee.userId = jsonObject[TAG_USER_ID].Get<int32_t>();
    }

    if (jsonObject[TAG_ACCOUNT_ID_HASH].IsString()) {
        context->accessee.accountIdHash = jsonObject[TAG_ACCOUNT_ID_HASH].Get<std::string>();
    }

    if (jsonObject[TAG_TOKEN_ID_HASH].IsString()) {
        context->accessee.tokenIdHash = jsonObject[TAG_TOKEN_ID_HASH].Get<std::string>();
    }

    if (jsonObject[TAG_NETWORKID_ID].IsString()) {
        context->accessee.networkId = jsonObject[TAG_NETWORKID_ID].Get<std::string>();
    }

    if (jsonObject[TAG_IS_ONLINE].IsBoolean()) {
        context->isOnline = jsonObject[TAG_IS_ONLINE].Get<bool>();
    }

    if (jsonObject[TAG_ACL_TYPE_LIST].IsString()) {
        context->accessee.aclTypeList = jsonObject[TAG_ACL_TYPE_LIST].Get<std::string>();
    }

    if (jsonObject[TAG_CERT_TYPE_LIST].IsString()) {
        context->accessee.credTypeList = jsonObject[TAG_CERT_TYPE_LIST].Get<std::string>();
    }

    if (jsonObject[TAG_LANGUAGE].IsString()) {
        context->accessee.language = jsonObject[TAG_LANGUAGE].Get<std::string>();
    }

    if (jsonObject[TAG_EXTRA_INFO].IsString()) {
        context->accessee.extraInfo = jsonObject[TAG_EXTRA_INFO].Get<std::string>();
    }
    ParseMessageProxyRespAclNegotiate(jsonObject, context);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcConfirmState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageProxyRespAclNegotiate(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    //sink not support proxy
    if (!CompareVersion(context->accessee.dmVersion, DM_VERSION_5_1_1)) {
        context->IsProxyBind = false;
        LOGE("sink does not support proxy");
        return DM_OK;
    }
    if (!context->IsProxyBind) {
        return DM_OK;
    }
    if (!IsString(jsonObject, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string subjectProxyOnesStr = jsonObject[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_PROXY_CONTEXT_ID) || !IsString(item, TAG_TOKEN_ID_HASH) ||
            !IsString(item, TAG_ACL_TYPE_LIST) || !IsString(item, TAG_CERT_TYPE_LIST)) {
            continue;
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = item[TAG_PROXY_CONTEXT_ID].Get<std::string>();
        auto it = std::find(context->subjectProxyOnes.begin(), context->subjectProxyOnes.end(), proxyAuthContext);
        if (it != context->subjectProxyOnes.end()) {
            it->proxyAccessee.tokenIdHash = item[TAG_TOKEN_ID_HASH].Get<std::string>();
            it->proxyAccessee.aclTypeList = item[TAG_ACL_TYPE_LIST].Get<std::string>();
            it->proxyAccessee.credTypeList = item[TAG_CERT_TYPE_LIST].Get<std::string>();
        }
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageReqUserConfirm(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    if (json[TAG_DEVICE_TYPE].IsNumberInteger()) {
        context->accesser.deviceType = json[TAG_DEVICE_TYPE].Get<int32_t>();
    }
    if (json[TAG_DEVICE_NAME].IsString()) {
        context->accesser.deviceName = json[TAG_DEVICE_NAME].Get<std::string>();
    }
    if (json[TAG_AUTH_TYPE].IsNumberInteger()) {
        context->authType = static_cast<DmAuthType>(json[TAG_AUTH_TYPE].Get<int32_t>());
    }
    if (json[TAG_ACL_TYPE_LIST].IsString()) {
        context->accesser.aclTypeList = json[TAG_ACL_TYPE_LIST].Get<std::string>();
    }
    if (json[TAG_CERT_TYPE_LIST].IsString()) {
        context->accesser.credTypeList = json[TAG_CERT_TYPE_LIST].Get<std::string>();
    }
    if (json[TAG_EXTRA_INFO].IsString()) {
        context->accesser.extraInfo = json[TAG_EXTRA_INFO].Get<std::string>();
    }
    if (IsString(json, TAG_CUSTOM_DESCRIPTION)) {
        context->customData = json[TAG_CUSTOM_DESCRIPTION].Get<std::string>();
    }
    ParseMessageProxyReqUserConfirm(json, context);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkConfirmState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageProxyReqUserConfirm(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind) {
        return DM_OK;
    }
    if (!IsString(json, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string subjectProxyOnesStr = json[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_PROXY_CONTEXT_ID) || !IsString(item, TAG_ACL_TYPE_LIST) ||
            !IsString(item, TAG_CERT_TYPE_LIST)) {
            continue;
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = item[TAG_PROXY_CONTEXT_ID].Get<std::string>();
        auto it = std::find(context->subjectProxyOnes.begin(), context->subjectProxyOnes.end(), proxyAuthContext);
        if (it != context->subjectProxyOnes.end()) {
            it->proxyAccesser.aclTypeList = item[TAG_ACL_TYPE_LIST].Get<std::string>();
            it->proxyAccesser.credTypeList = item[TAG_CERT_TYPE_LIST].Get<std::string>();
        }
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageRespUserConfirm(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    if (json[TAG_AUTH_TYPE_LIST].IsString()) {
        auto strList = json[TAG_AUTH_TYPE_LIST].Get<std::string>();
        context->authTypeList = stringToVectorAuthType(strList);
    }
    if (json[TAG_EXTRA_INFO].IsString()) {
        context->accessee.extraInfo = json[TAG_EXTRA_INFO].Get<std::string>();
    }
    ParseMessageProxyRespUserConfirm(json, context);
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinNegotiateStartState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageProxyRespUserConfirm(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind) {
        return DM_OK;
    }
    if (!IsString(json, PARAM_KEY_SUBJECT_PROXYED_SUBJECTS)) {
        context->subjectProxyOnes.clear();
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string subjectProxyOnesStr = json[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS].Get<std::string>();
    JsonObject allProxyObj;
    allProxyObj.Parse(subjectProxyOnesStr);
    if (allProxyObj.IsDiscarded()) {
        context->subjectProxyOnes.clear();
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::vector<DmProxyAuthContext> sinksubjectProxyOnes;
    for (auto const &item : allProxyObj.Items()) {
        if (!IsString(item, TAG_PROXY_CONTEXT_ID)) {
            continue;
        }
        DmProxyAuthContext proxyAuthContext;
        proxyAuthContext.proxyContextId = item[TAG_PROXY_CONTEXT_ID].Get<std::string>();
        sinksubjectProxyOnes.push_back(proxyAuthContext);
    }
    for (auto item = context->subjectProxyOnes.begin(); item != context->subjectProxyOnes.end();) {
        if (std::find(sinksubjectProxyOnes.begin(), sinksubjectProxyOnes.end(), *item) != sinksubjectProxyOnes.end()) {
            item++;
        } else {
            item = context->subjectProxyOnes.erase(item);
        }
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageReqPinAuthStart(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    if (json[TAG_DATA].IsString()) {
        context->transmitData = json[TAG_DATA].Get<std::string>();
    }
    if (context->authType == AUTH_TYPE_PIN_ULTRASONIC && context->ultrasonicInfo == DM_Ultrasonic_Reverse) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkReverseUltrasonicDoneState>());
    } else if (context->authType == AUTH_TYPE_PIN_ULTRASONIC && context->ultrasonicInfo == DM_Ultrasonic_Forward) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkForwardUltrasonicDoneState>());
    } else {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinAuthStartState>());
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageRespPinAuthStart(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    if (json[TAG_DATA].IsString()) {
        context->transmitData = json[TAG_DATA].Get<std::string>();
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcPinAuthMsgNegotiateState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageReqPinAuthNegotiate(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    if (json[TAG_DATA].IsString()) {
        context->transmitData = json[TAG_DATA].Get<std::string>();
    }
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkPinAuthMsgNegotiateState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageReverseUltrasonicStart(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkReverseUltrasonicStartState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageReverseUltrasonicDone(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    if (json[TAG_REPLY].IsNumberInteger()) {
        context->reply = json[TAG_REPLY].Get<int32_t>();
    }
    if (context->reply == DM_OK) {
        context->authStateMachine->TransitionTo(std::make_shared<AuthSrcReverseUltrasonicDoneState>());
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageForwardUltrasonicStart(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkForwardUltrasonicStartState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseMessageForwardUltrasonicNegotiate(const JsonObject &json,
    std::shared_ptr<DmAuthContext> context)
{
    context->authStateMachine->TransitionTo(std::make_shared<AuthSrcForwardUltrasonicDoneState>());
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageReqUserConfirm(std::shared_ptr<DmAuthContext> context, JsonObject &json)
{
    json[TAG_AUTH_TYPE] = context->authType;
    json[TAG_ACL_TYPE_LIST] = context->accesser.aclTypeList;
    json[TAG_CERT_TYPE_LIST] = context->accesser.credTypeList;
    json[TAG_DEVICE_TYPE] = context->accesser.deviceType;
    json[TAG_DEVICE_NAME] = context->accesser.deviceName;
    json[TAG_EXTRA_INFO] = context->accesser.extraInfo;
    json[TAG_CUSTOM_DESCRIPTION] = context->customData;
    CreateMessageProxyReqUserConfirm(context, json);
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageProxyReqUserConfirm(std::shared_ptr<DmAuthContext> context,
    JsonObject &json)
{
    if (context != nullptr && context->IsProxyBind && !context->subjectProxyOnes.empty()) {
        JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
        for (const auto &app : context->subjectProxyOnes) {
            JsonObject object;
            object[TAG_PROXY_CONTEXT_ID] = app.proxyContextId;
            object[TAG_ACL_TYPE_LIST] = app.proxyAccesser.aclTypeList;
            object[TAG_CERT_TYPE_LIST] = app.proxyAccesser.credTypeList;
            allProxyObj.PushBack(object);
        }
        json[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = allProxyObj.Dump();
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageRespUserConfirm(std::shared_ptr<DmAuthContext> context, JsonObject &json)
{
    json[TAG_AUTH_TYPE_LIST] = vectorAuthTypeToString(context->authTypeList);
    json[TAG_EXTRA_INFO] = context->accessee.extraInfo;
    if (context != nullptr && context->IsProxyBind && !context->subjectProxyOnes.empty()) {
        JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
        for (const auto &app : context->subjectProxyOnes) {
            JsonObject object;
            object[TAG_PROXY_CONTEXT_ID] = app.proxyContextId;
            allProxyObj.PushBack(object);
        }
        json[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = allProxyObj.Dump();
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageReqPinAuthStart(std::shared_ptr<DmAuthContext> context, JsonObject &json)
{
    json[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageRespPinAuthStart(std::shared_ptr<DmAuthContext> context, JsonObject &json)
{
    json[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageReqPinAuthNegotiate(std::shared_ptr<DmAuthContext> context,
    JsonObject &json)
{
    json[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageRespPinAuthNegotiate(std::shared_ptr<DmAuthContext> context,
    JsonObject &json)
{
    json[TAG_DATA] = context->transmitData;
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageReverseUltrasonicStart(std::shared_ptr<DmAuthContext> context,
    JsonObject &json)
{
    json[TAG_REPLY] = context->reply;
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageReverseUltrasonicDone(std::shared_ptr<DmAuthContext> context,
    JsonObject &json)
{
    json[TAG_REPLY] = context->reply;
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageForwardUltrasonicStart(std::shared_ptr<DmAuthContext> context,
    JsonObject &json)
{
    json[TAG_REPLY] = context->reply;
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateMessageForwardUltrasonicNegotiate(std::shared_ptr<DmAuthContext> context,
    JsonObject &json)
{
    json[TAG_REPLY] = context->reply;
    return DM_OK;
}

void DmAuthMessageProcessor::CreateAndSendMsg(DmMessageType msgType, std::shared_ptr<DmAuthContext> context)
{
    auto message = CreateMessage(msgType, context);
    int32_t ret = context->softbusConnector->GetSoftbusSession()->SendData(context->sessionId, message);
    if (ret != DM_OK) {
        if (context->direction == DM_AUTH_SOURCE) {
            context->authStateMachine->TransitionTo(std::make_shared<AuthSrcFinishState>());
        } else {
            context->authStateMachine->TransitionTo(std::make_shared<AuthSinkFinishState>());
        }
    }
}

std::string DmAuthMessageProcessor::CompressSyncMsg(std::string &inputStr)
{
    uint32_t srcLen = inputStr.size();
    uint32_t boundSize = compressBound(srcLen);  // Maximum compression length
    if (boundSize <= 0) {
        LOGE("DmAuthMessageProcessor::CompressSyncMsg zlib compressBound failed");
        return "";
    }
    std::string compressed(boundSize, '\0');

    // Compress to reserved space
    unsigned long destSize = boundSize;  // Actual usable length
    int32_t ret = compress(reinterpret_cast<Bytef *>(&compressed[0]), &destSize,
                           reinterpret_cast<const Bytef *>(inputStr.data()), srcLen);
    if (ret != Z_OK) {
        LOGE("DmAuthMessageProcessor::CompressSyncMsg zlib compress failed");
        return "";
    }
    compressed.resize(destSize); // Actual usage length
    return compressed;
}

std::string DmAuthMessageProcessor::DecompressSyncMsg(std::string& compressed, uint32_t oriLen)
{
    if (oriLen <= 0) {
        LOGE("DmAuthMessageProcessor::DecompressSyncMsg decompress oriLen param error");
        return "";
    }
    std::string decompressed;
    decompressed.resize(oriLen);
    unsigned long destLen = oriLen; // Actual usage length
    int32_t ret = uncompress(reinterpret_cast<Bytef *>(&decompressed[0]), &destLen,
                             reinterpret_cast<const Bytef *>(compressed.data()),  // Skip header when decompressing
                             compressed.size());
    if (ret != Z_OK || destLen != oriLen) {
        LOGE("DmAuthMessageProcessor::DecompressSyncMsg decompress failed");
        return "";
    }
    return decompressed;
}

std::string DmAuthMessageProcessor::Base64Encode(std::string &inputStr)
{
    // Convert input string to binary
    const unsigned char* src = reinterpret_cast<const unsigned char*>(inputStr.data());
    size_t srcLen = inputStr.size();

    // Calculate the maximum length after base64 encoding
    size_t maxEncodeLen = ((srcLen + 2) / 3) * 4 + 1;
    std::vector<unsigned char> buffer(maxEncodeLen);

    // Actual encoding length
    size_t encodedLen = 0;
    int32_t ret = mbedtls_base64_encode(buffer.data(), buffer.size(), &encodedLen, src, srcLen);
    if (ret != 0) {
        LOGE("DmAuthMessageProcessor::Base64Encode mbedtls_base64_encode failed");
        return "";
    }
    return std::string(reinterpret_cast<const char*>(buffer.data()), encodedLen); // No terminator needed
}

std::string DmAuthMessageProcessor::Base64Decode(std::string &inputStr)
{
    // Convert input string to binary
    const unsigned char* src = reinterpret_cast<const unsigned char*>(inputStr.data());
    size_t srcLen = inputStr.size();

    // Calculate the maximum length after base64 encoding
    size_t maxEncodeLen = (srcLen / 4) *  3 + 1;
    std::vector<unsigned char> buffer(maxEncodeLen);

    // Actual encoding length
    size_t decodedLen = 0;
    int32_t ret = mbedtls_base64_decode(buffer.data(), buffer.size(), &decodedLen, src, srcLen);
    if (ret != 0) {
        LOGE("DmAuthMessageProcessor::Base64Decode mbedtls_base64_decode failed");
        return "";
    }
    return std::string(reinterpret_cast<const char*>(buffer.data()), decodedLen); // 无需终止符
}

int32_t DmAuthMessageProcessor::EncryptSyncMessage(std::shared_ptr<DmAuthContext> &context, DmAccess &accessSide,
                                                   std::string &encSyncMsg)
{
    JsonObject syncMsgJson;
    DmAccessToSync accessToSync;
    SaveToDmAccessSync(accessToSync, context, accessSide);

    syncMsgJson[TAG_TRANSMIT_SK_ID] = std::to_string(accessSide.transmitSessionKeyId);
    syncMsgJson[TAG_TRANSMIT_SK_TIMESTAMP] = accessSide.transmitSkTimeStamp;
    syncMsgJson[TAG_TRANSMIT_CREDENTIAL_ID] = accessSide.transmitCredentialId;
    // First certification
    if (accessSide.isGenerateLnnCredential && accessSide.bindLevel != static_cast<int32_t>(USER)) {
        syncMsgJson[TAG_LNN_SK_ID] = std::to_string(accessSide.lnnSessionKeyId);
        syncMsgJson[TAG_LNN_SK_TIMESTAMP] = accessSide.lnnSkTimeStamp;
        syncMsgJson[TAG_LNN_CREDENTIAL_ID] = accessSide.lnnCredentialId;
    }
    JsonObject accessJsonObj{};
    accessJsonObj = accessToSync;
    syncMsgJson[TAG_DMVERSION] = accessSide.dmVersion;
    syncMsgJson[TAG_ACCESS] = accessJsonObj.Dump();
    syncMsgJson[TAG_PROXY] = ""; // Reserved field, leave blank
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    std::string localUdid = static_cast<std::string>(localDeviceId);
    DmAccess &access = (context->accesser.deviceId == localUdid) ? context->accesser : context->accessee;
    DmAccess &remoteAccess = (context->accesser.deviceId == localUdid) ? context->accessee : context->accesser;
    std::string aclHashList;
    int32_t ret = DeviceProfileConnector::GetInstance().GetAclListHashStr({localUdid, access.userId},
        {remoteAccess.deviceId, remoteAccess.userId}, aclHashList, DM_ACL_AGING_VERSION);
    if (ret != DM_OK) {
        LOGE("DmAuthMessageProcessor::EncryptSyncMessage GetAclListHashStr failed");
        return ERR_DM_FAILED;
    }

    syncMsgJson[TAG_ACL_CHECKSUM] = aclHashList;
    if (context->direction == DmAuthDirection::DM_AUTH_SINK) {
        LOGI("Sink Send user confirm opt");
        syncMsgJson[TAG_USER_CONFIRM_OPT] = context->confirmOperation;
    }
    syncMsgJson[TAG_IS_COMMON_FLAG] = context->accesser.isCommonFlag;
    syncMsgJson[TAG_DM_CERT_CHAIN] = context->accesser.cert;
    CreateProxyAccessMessage(context, syncMsgJson);
    std::string syncMsg = syncMsgJson.Dump();
    std::string compressMsg = CompressSyncMsg(syncMsg);
    if (compressMsg.empty()) {
        LOGE("DmAuthMessageProcessor::EncryptSyncMessage compress failed");
        return ERR_DM_FAILED;
    }
    JsonObject plainJson;
    plainJson[TAG_COMPRESS_ORI_LEN] = syncMsg.size();
    plainJson[TAG_COMPRESS] = Base64Encode(compressMsg);
    return cryptoMgr_->EncryptMessage(plainJson.Dump(), encSyncMsg);
}

int32_t DmAuthMessageProcessor::CreateProxyAccessMessage(std::shared_ptr<DmAuthContext> &context,
    JsonObject &syncMsgJson)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return DM_OK;
    }
    JsonObject allProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    for (const auto &app : context->subjectProxyOnes) {
        DmProxyAccess access;
        if (context->direction == DM_AUTH_SOURCE) {
            access = app.proxyAccesser;
        } else {
            access = app.proxyAccessee;
        }
        if (access.isAuthed) {
            continue;
        }
        JsonObject object;
        object[TAG_PROXY_CONTEXT_ID] = app.proxyContextId;
        object[TAG_TOKEN_ID] = access.tokenId;
        object[TAG_BUNDLE_NAME] = access.bundleName;
        object[TAG_BIND_LEVEL] = access.bindLevel;
        object[TAG_PKG_NAME] = access.pkgName;
        object[TAG_TRANSMIT_SK_ID] = std::to_string(access.transmitSessionKeyId);
        object[TAG_TRANSMIT_SK_TIMESTAMP] = access.skTimeStamp;
        object[TAG_TRANSMIT_CREDENTIAL_ID] = access.transmitCredentialId;
        allProxyObj.PushBack(object);
    }
    syncMsgJson[PARAM_KEY_SUBJECT_PROXYED_SUBJECTS] = allProxyObj.Dump();
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ACLToStr(DistributedDeviceProfile::AccessControlProfile acl, std::string aclStr)
{
    DmAccessControlTable dmAcl;
    dmAcl.accessControlId = acl.GetAccessControlId();
    dmAcl.accesserId = acl.GetAccesserId();
    dmAcl.accesseeId = acl.GetAccesseeId();
    dmAcl.deviceId = acl.GetTrustDeviceId();
    dmAcl.sessionKey = acl.GetSessionKey();
    dmAcl.bindType = static_cast<int32_t>(acl.GetBindType());
    dmAcl.authType = acl.GetAuthenticationType();
    dmAcl.deviceType = acl.GetDeviceIdType();
    dmAcl.deviceIdHash = acl.GetDeviceIdHash();
    dmAcl.status = acl.GetStatus();
    dmAcl.validPeriod = acl.GetValidPeriod();
    dmAcl.lastAuthTime = acl.GetLastAuthTime();
    dmAcl.bindLevel = acl.GetBindType();
    JsonObject aclJsonObj{};
    aclJsonObj = dmAcl;
    aclStr = aclJsonObj.Dump();
    if (aclStr.empty()) {
        LOGE("DmAuthMessageProcessor::ACLToStr normalized acl failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DmAuthMessageProcessor::CreateSyncMessage(std::shared_ptr<DmAuthContext> context, JsonObject &jsonObject)
{
    DmAccess accessSide;
    if (context->direction == DM_AUTH_SOURCE) {
        accessSide = context->accesser;
    } else {
        accessSide = context->accessee;
    }
    std::string encSyncMsg;
    int32_t ret = EncryptSyncMessage(context, accessSide, encSyncMsg);
    if (ret != DM_OK) {
        LOGE("DmAuthMessageProcessor::CreateSyncMessage encrypt failed");
        return ret;
    }
    jsonObject[TAG_SYNC] = encSyncMsg;
    return DM_OK;
}

int32_t DmAuthMessageProcessor::ParseAuthStartMessage(const JsonObject &jsonObject,
    std::shared_ptr<DmAuthContext> context)
{
    if (jsonObject.IsDiscarded() || !jsonObject.Contains(TAG_DATA) ||
        !jsonObject[TAG_DATA].IsString()) {
        LOGE("DmAuthMessageProcessor::ParseAuthStartMessage Unlegal json string failed");
        return ERR_DM_FAILED;
    }
    context->transmitData = jsonObject[TAG_DATA].Get<std::string>();

    context->authStateMachine->TransitionTo(std::make_shared<AuthSinkCredentialAuthStartState>());
    return DM_OK;
}

bool DmAuthMessageProcessor::IsExistTheToken(JsonObject &proxyObj, int64_t tokenId)
{
    if (proxyObj.IsDiscarded()) {
        return false;
    }
    for (auto const &item : proxyObj.Items()) {
        if (tokenId == item.Get<int64_t>()) {
            return true;
        }
    }
    return false;
}

void DmAuthMessageProcessor::SetAclProxyRelate(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    if (!context->IsProxyBind || context->subjectProxyOnes.empty()) {
        return;
    }
    DmAccess &access = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    DmAccess &remoteAccess = (context->direction == DM_AUTH_SOURCE) ? context->accessee : context->accesser;
    for (auto &app : context->subjectProxyOnes) {
        DmProxyAccess &proxyAccess = (context->direction == DM_AUTH_SOURCE) ? app.proxyAccesser : app.proxyAccessee;
        if (!proxyAccess.isAuthed || proxyAccess.aclProfiles.find(DM_POINT_TO_POINT) ==
            proxyAccess.aclProfiles.end()) {
            continue;
        }
        SetAclProxyRelate(context, proxyAccess.aclProfiles[DM_POINT_TO_POINT]);
    }
}

void DmAuthMessageProcessor::SetAclProxyRelate(std::shared_ptr<DmAuthContext> context,
    DistributedDeviceProfile::AccessControlProfile &profile)
{
    DmAccess &access = (context->direction == DM_AUTH_SOURCE) ? context->accesser : context->accessee;
    DmAccess &remoteAccess = (context->direction == DM_AUTH_SOURCE) ? context->accessee : context->accesser;
    bool isNeedUpdateProxy = false;
    JsonObject accesserExtObj;
    if (!profile.GetAccesser().GetAccesserExtraData().empty()) {
        accesserExtObj.Parse(profile.GetAccesser().GetAccesserExtraData());
    }
    JsonObject accesserProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    if (IsString(accesserExtObj, TAG_PROXY)) {
        std::string proxyStr = accesserExtObj[TAG_PROXY].Get<std::string>();
        accesserProxyObj.Parse(proxyStr);
    }
    int64_t tokenId = access.deviceId == profile.GetAccesser().GetAccesserDeviceId() ? access.tokenId :
        remoteAccess.tokenId;
    if (!IsExistTheToken(accesserProxyObj, tokenId)) {
        isNeedUpdateProxy = true;
        accesserProxyObj.PushBack(tokenId);
        accesserExtObj[TAG_PROXY] = accesserProxyObj.Dump();
        DistributedDeviceProfile::Accesser accesser = profile.GetAccesser();
        accesser.SetAccesserExtraData(accesserExtObj.Dump());
        profile.SetAccesser(accesser);
    }

    JsonObject accesseeExtObj;
    if (!profile.GetAccessee().GetAccesseeExtraData().empty()) {
        accesseeExtObj.Parse(profile.GetAccessee().GetAccesseeExtraData());
    }
    JsonObject accesseeProxyObj(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    if (IsString(accesseeExtObj, TAG_PROXY)) {
        std::string proxyStr = accesseeExtObj[TAG_PROXY].Get<std::string>();
        accesseeProxyObj.Parse(proxyStr);
    }
    tokenId = access.deviceId == profile.GetAccesser().GetAccesserDeviceId() ? remoteAccess.tokenId : access.tokenId;
    if (!IsExistTheToken(accesseeProxyObj, tokenId)) {
        isNeedUpdateProxy = true;
        accesseeProxyObj.PushBack(tokenId);
        accesseeExtObj[TAG_PROXY] = accesseeProxyObj.Dump();
        DistributedDeviceProfile::Accessee accessee = profile.GetAccessee();
        accessee.SetAccesseeExtraData(accesseeExtObj.Dump());
        profile.SetAccessee(accessee);
    }
    if (isNeedUpdateProxy) {
        DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance().UpdateAccessControlProfile(profile);
    }
}

void ToJson(JsonItemObject &itemObject, const DmAccessControlTable &table)
{
    itemObject["accessControlId"] = table.accessControlId;
    itemObject["accesserId"] = table.accesserId;
    itemObject["accesseeId"] = table.accesseeId;
    itemObject["deviceId"] = table.deviceId;
    itemObject["sessionKey"] = table.sessionKey;
    itemObject["bindType"] = table.bindType;
    itemObject["authType"] = table.authType;
    itemObject["deviceType"] = table.deviceType;
    itemObject["deviceIdHash"] = table.deviceIdHash;
    itemObject["status"] = table.status;
    itemObject["validPeriod"] = table.validPeriod;
    itemObject["lastAuthTime"] = table.lastAuthTime;
    itemObject["bindLevel"] = table.bindLevel;
}

void FromJson(const JsonItemObject &itemObject, DmAccessControlTable &table)
{
    SetValueFromJson(itemObject, "accessControlId", &JsonItemObject::IsNumberInteger, table.accessControlId);
    SetValueFromJson(itemObject, "accesserId", &JsonItemObject::IsNumberInteger, table.accesserId);
    SetValueFromJson(itemObject, "accesseeId", &JsonItemObject::IsNumberInteger, table.accesseeId);
    SetValueFromJson(itemObject, "deviceId", &JsonItemObject::IsNumberInteger, table.deviceId);
    SetValueFromJson(itemObject, "sessionKey", &JsonItemObject::IsString, table.sessionKey);
    SetValueFromJson(itemObject, "bindType", &JsonItemObject::IsNumberInteger, table.bindType);
    SetValueFromJson(itemObject, "authType", &JsonItemObject::IsNumberInteger, table.authType);
    SetValueFromJson(itemObject, "deviceType", &JsonItemObject::IsNumberInteger, table.deviceType);
    SetValueFromJson(itemObject, "deviceIdHash", &JsonItemObject::IsString, table.deviceIdHash);
    SetValueFromJson(itemObject, "status", &JsonItemObject::IsNumberInteger, table.status);
    SetValueFromJson(itemObject, "validPeriod", &JsonItemObject::IsNumberInteger, table.validPeriod);
    SetValueFromJson(itemObject, "lastAuthTime", &JsonItemObject::IsNumberInteger, table.lastAuthTime);
    SetValueFromJson(itemObject, "bindLevel", &JsonItemObject::IsNumberInteger, table.bindLevel);
}

void ToJson(JsonItemObject &itemObject, const DmAccessToSync &table)
{
    itemObject["deviceName"] = table.deviceName;
    itemObject["deviceNameFull"] = table.deviceNameFull;
    itemObject["deviceId"] = table.deviceId;
    itemObject["userId"] = table.userId;
    itemObject["accountId"] = table.accountId;
    itemObject["tokenId"] = table.tokenId;
    itemObject["bundleName"] = table.bundleName;
    itemObject["pkgName"] = table.pkgName;
    itemObject["bindLevel"] = table.bindLevel;
    itemObject["sessionKeyId"] = table.sessionKeyId;
    itemObject["skTimeStamp"] = table.skTimeStamp;
}

void FromJson(const JsonItemObject &itemObject, DmAccessToSync &table)
{
    SetValueFromJson(itemObject, "deviceName", &JsonItemObject::IsString, table.deviceName);
    SetValueFromJson(itemObject, "deviceNameFull", &JsonItemObject::IsString, table.deviceNameFull);
    SetValueFromJson(itemObject, "deviceId", &JsonItemObject::IsString, table.deviceId);
    SetValueFromJson(itemObject, "userId", &JsonItemObject::IsNumberInteger, table.userId);
    SetValueFromJson(itemObject, "accountId", &JsonItemObject::IsString, table.accountId);
    SetValueFromJson(itemObject, "tokenId", &JsonItemObject::IsNumberInteger, table.tokenId);
    SetValueFromJson(itemObject, "bundleName", &JsonItemObject::IsString, table.bundleName);
    SetValueFromJson(itemObject, "pkgName", &JsonItemObject::IsString, table.pkgName);
    SetValueFromJson(itemObject, "bindLevel", &JsonItemObject::IsNumberInteger, table.bindLevel);
    SetValueFromJson(itemObject, "sessionKeyId", &JsonItemObject::IsNumberInteger, table.sessionKeyId);
    SetValueFromJson(itemObject, "skTimeStamp", &JsonItemObject::IsNumberInteger, table.skTimeStamp);
}
} // namespace DistributedHardware
} // namespace OHOS