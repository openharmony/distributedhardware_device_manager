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

#include <algorithm>
#include <vector>

#include "auth_manager.h"
#include "deviceprofile_connector.h"
#include "dm_auth_attest_common.h"
#include "dm_auth_cert.h"
#include "dm_auth_context.h"
#include "dm_auth_manager_base.h"
#include "dm_auth_state.h"
#include "dm_auth_state_machine.h"
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_freeze_process.h"
#include "multiple_user_connector.h"

namespace OHOS {
namespace DistributedHardware {
int32_t AuthSinkDataSyncState::VerifyCertificate(std::shared_ptr<DmAuthContext> context)
{
#ifdef DEVICE_MANAGER_COMMON_FLAG
    (void)context;
    LOGI("open source device do not verify cert!");
    return DM_OK;
#else
    if (context == nullptr) {
        LOGE("context_ is nullptr!");
        return ERR_DM_POINT_NULL;
    }
    // Compatible with 5.1.0 and earlier
    if (!CompareVersion(context->accesser.dmVersion, DM_VERSION_5_1_0)) {
        LOGI("cert verify is not supported");
        return DM_OK;
    }
    // Compatible common device
    if (CompareVersion(context->accesser.dmVersion, DM_VERSION_5_1_0) &&
        context->accesser.isCommonFlag == true) {
        LOGI("src is common device.");
        if (DeviceProfileConnector::GetInstance().
            CheckIsSameAccountByUdidHash(context->accesser.deviceIdHash) == DM_OK) {
            LOGE("src is common device, but the udidHash is identical in acl!");
            return ERR_DM_VERIFY_CERT_FAILED;
        }
        return DM_OK;
    }
    DmCertChain dmCertChain{nullptr, 0};
    if (!AuthAttestCommon::GetInstance().
        DeserializeDmCertChain(context->accesser.cert, &dmCertChain)) {
        LOGE("cert deserialize fail!");
        return ERR_DM_DESERIAL_CERT_FAILED;
    }
    int32_t certRet = -1;
    if (CompareVersion(context->accesser.dmVersion, DM_VERSION_5_1_3)) {
        certRet = AuthCert::GetInstance().
            VerifyCertificateV2(dmCertChain, context->accesser.deviceIdHash.c_str(), context->accessee.certRandom);
    } else {
        certRet = AuthCert::GetInstance().
            VerifyCertificate(dmCertChain, context->accesser.deviceIdHash.c_str());
    }
    // free dmCertChain memory
    AuthAttestCommon::GetInstance().FreeDmCertChain(dmCertChain);
    if (certRet != DM_OK) {
        LOGE("validate cert fail, certRet = %{public}d", certRet);
        return ERR_DM_VERIFY_CERT_FAILED;
    }
    return DM_OK;
#endif
}

static void SetServiceInfos(std::shared_ptr<DmAuthContext> context)
{
    LOGI("SetServiceInfos::start");
    CHECK_NULL_VOID(context);
    if (!context->isServiceBind) {
        return;
    }
    std::vector<DistributedDeviceProfile::ServiceInfo> dpServiceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().GetServiceInfosByUdid(context->accessee.deviceId,
        dpServiceInfos);
    if (ret != DM_OK) {
        LOGE("SetServiceInfos::Get all serviceinfos");
        return;
    }
    std::unordered_set<int32_t> seenServiceIds;
    if (!context->IsProxyBind || context->IsCallingProxyAsSubject) {
        DistributedDeviceProfile::ServiceInfo dpServiceInfo;
        for (auto info : dpServiceInfos) {
            if (info.GetServiceId() == context->accessee.serviceId) {
                LOGI("SetServiceInfos::servicecode:%{public}s", info.GetServiceCode().c_str());
                context->serviceInfos.emplace_back(info);
                seenServiceIds.insert(info.GetServiceId());
                break;
            }
        }
    }
    if (context->IsProxyBind && context->subjectServiceOnes.empty()) {
        LOGE("SetServiceInfos::Is proxy bind but no subject list");
        return;
    }
    for (auto &item : context->subjectServiceOnes) {
        DistributedDeviceProfile::ServiceInfo dpServiceInfo;
        if (seenServiceIds.find(item.proxyAccessee.serviceId) != seenServiceIds.end()) {
            continue;
        }
        for (auto info : dpServiceInfos) {
            if (info.GetServiceId() == item.proxyAccessee.serviceId) {
                LOGI("SetServiceInfos::servicecode:%{public}s", info.GetServiceCode().c_str());
                context->serviceInfos.emplace_back(info);
                seenServiceIds.insert(info.GetServiceId());
                break;
            }
        }
    }
}

static void CompareServiceInfos(std::shared_ptr<DmAuthContext> context)
{
    LOGI("CompareServiceInfos::start");
    CHECK_NULL_VOID(context);
    if (!context->isServiceBind || !context->IsProxyBind) {
        return;
    }
    if (context->subjectServiceOnes.empty()) {
        LOGE("SetServiceInfos::Is proxy bind but no subject list");
        return;
    }
    for (auto &item : context->serviceInfos) {
        DistributedDeviceProfile::ServiceInfo dpServiceInfo;
        int32_t ret = DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(
            context->accessee.deviceId, item.GetServiceId(), dpServiceInfo);
        if (ret != DM_OK) {
            LOGE("SetServiceInfos::Get subject service failed");
            context->serviceId.push_back(dpServiceInfo.GetServiceId());
        }
    }
    context->serviceInfos.clear();
}

// Received 180 synchronization message, send 190 message
int32_t AuthSinkDataSyncState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkDataSyncState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_FAILED);
    if (DmAuthState::IsImportAuthCodeCompatibility(context->authType) &&
        CompareVersion(context->accesser.dmVersion, DM_VERSION_5_1_4)) {
        bool verifyRet = VerifyFlagXor(context);
        if (!verifyRet) {
            LOGE("VerifyFlagXor failed, ret: %{public}d", verifyRet);
            return ERR_DM_BIND_PIN_XOR_MISMATCH;
        }
    }
    // verify device cert
    int32_t ret = VerifyCertificate(context);
    if (ret != DM_OK) {
        LOGE("AuthSinkNegotiateStateMachine::Action cert verify fail!");
        context->reason = ret;
        return ret;
    }
    // Query the ACL of the sink end. Compare the ACLs at both ends.
    CHECK_NULL_RETURN(context->softbusConnector, ERR_DM_POINT_NULL);
    context->softbusConnector->SyncLocalAclListProcess({context->accessee.deviceId, context->accessee.userId},
        {context->accesser.deviceId, context->accesser.userId}, context->accesser.aclStrList, false);
    if (GetSessionKey(context)) {
        DerivativeSessionKey(context);
    }
    // Synchronize the local SP information, the format is uncertain, not done for now
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
    CompareServiceInfos(context);
    SetServiceInfos(context);
    SetAclInfo(context);
    if (NeedAgreeAcl(context)) {
        UpdateCredInfo(context);
        context->authMessageProcessor->PutAccessControlList(context,
            context->accessee, context->accesser.deviceId);
    }
    context->softbusConnector->SyncAclList();
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_RESP_DATA_SYNC, context);
    context->accessee.deviceName = context->softbusConnector->GetLocalDeviceName();
    LOGI("AuthSinkDataSyncState::Action ok");
    return DM_OK;
}

//LCOV_EXCL_START
DmAuthStateType AuthSinkDataSyncState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_DATA_SYNC_STATE;
}
//LCOV_EXCL_STOP

void AuthSinkDataSyncState::ProcessMainAccesseeSessionKey(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_VOID(context);
    context->accessee.transmitCredentialId = context->reUseCreId;
    if (context->IsCallingProxyAsSubject && !context->accessee.isAuthed) {
        std::string suffix = "";
        if (context->isServiceBind) {
            suffix = context->accesser.deviceIdHash + context->accessee.deviceIdHash +
                context->accesser.tokenIdHash + context->accessee.tokenIdHash +
                std::to_string(context->accessee.serviceId);
        } else {
            suffix = context->accesser.deviceIdHash + context->accessee.deviceIdHash +
                context->accesser.tokenIdHash + context->accessee.tokenIdHash;
        }

        CHECK_NULL_VOID(context->authMessageProcessor);
        int32_t skId = 0;
        int32_t ret = context->authMessageProcessor->SaveDerivativeSessionKeyToDP(
            context->accessee.userId, suffix, skId);
        if (ret == DM_OK) {
            context->accessee.transmitSkTimeStamp = static_cast<int64_t>(DmAuthState::GetSysTimeMs());
            context->accessee.transmitSessionKeyId = skId;
        } else {
            LOGE("AuthSinkDataSyncState::Action DP save user session key failed");
        }
    }
}

void AuthSinkDataSyncState::ProcessSingleProxyAccessee(std::shared_ptr<DmAuthContext> context, DmProxyAuthContext& app)
{
    CHECK_NULL_VOID(context);
    std::string suffix = "";
    if (context->isServiceBind) {
        suffix = context->accesser.deviceIdHash + context->accessee.deviceIdHash +
            app.proxyAccesser.tokenIdHash + app.proxyAccessee.tokenIdHash +
            std::to_string(app.proxyAccessee.serviceId);
    } else {
        suffix =  context->accesser.deviceIdHash + context->accessee.deviceIdHash +
            app.proxyAccesser.tokenIdHash + app.proxyAccessee.tokenIdHash;
    }

    CHECK_NULL_VOID(context->authMessageProcessor);
    int32_t skId = 0;
    int32_t ret = context->authMessageProcessor->SaveDerivativeSessionKeyToDP(
        context->accessee.userId, suffix, skId);
    if (ret != DM_OK) {
        LOGE("AuthSrcCredentialAuthDoneState::Action DP save user session key failed");
        return;
    }

    app.proxyAccessee.skTimeStamp = static_cast<int64_t>(DmAuthState::GetSysTimeMs());
    app.proxyAccessee.transmitSessionKeyId = skId;

    if (!context->reUseCreId.empty()) {
        app.proxyAccessee.transmitCredentialId = context->reUseCreId;
    } else {
        app.proxyAccessee.transmitCredentialId = context->accessee.transmitCredentialId;
    }
}

int32_t AuthSinkDataSyncState::DerivativeSessionKey(std::shared_ptr<DmAuthContext> context)
{
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);

    auto &targetList = context->isServiceBind ? context->subjectServiceOnes : context->subjectProxyOnes;
    if (!context->IsProxyBind || targetList.empty()) {
        return DM_OK;
    }

    ProcessMainAccesseeSessionKey(context);
    for (auto &app : targetList) {
        if (app.proxyAccessee.isAuthed) {
            continue;
        }

        ProcessSingleProxyAccessee(context, app);
    }

    return DM_OK;
}

static void PutServiceInfos(const std::shared_ptr<DmAuthContext> context)
{
    LOGI("PutServiceInfos::Action start");
    CHECK_NULL_VOID(context);
    if (!context->isServiceBind || context->serviceInfos.empty()) {
        return;
    }
    for (auto &item : context->serviceInfos) {
        LOGI("PutServiceInfos::Put service info");
        LOGI("PutServiceInfos::servicecode:%{public}s", item.GetServiceCode().c_str());
        int32_t ret = DeviceProfileConnector::GetInstance().PutServiceInfo(item);
        if (ret != DM_OK) {
            LOGE("PutServiceInfos::Put service info failed, service id:%{public}" PRId64, item.GetServiceId());
        }
    }
}

static void DeleteServiceInfos(const std::shared_ptr<DmAuthContext> context)
{
    LOGI("DeleteServiceInfos::Action start");
    CHECK_NULL_VOID(context);
    if (!context->isServiceBind || context->serviceId.empty()) {
        return;
    }
    for (auto &item : context->serviceId) {
        LOGI("DeleteServiceInfos::Delete service info");
        DistributedDeviceProfile::ServiceInfo info;
        int32_t ret = DeviceProfileConnector::GetInstance().GetServiceInfoByUdidAndServiceId(context->accessee.deviceId,
            item, info);
        if (ret != DM_OK) {
            LOGE("DeleteServiceInfos::Get service info failed, service id:%{public}" PRId64, item);
        }
        ret = DeviceProfileConnector::GetInstance().DeleteServiceInfo(info);
    }
}

// Received 190 message, sent 200 message
int32_t AuthSrcDataSyncState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcDataSyncState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_FAILED);
    CHECK_NULL_RETURN(context->softbusConnector, ERR_DM_FAILED);
    CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_FAILED);
    DeleteServiceInfos(context);
    if (NeedAgreeAcl(context)) {
        if (DmAuthState::IsImportAuthCodeCompatibility(context->authType) &&
            CompareVersion(context->accessee.dmVersion, DM_VERSION_5_1_4)) {
            bool verifyRet = VerifyFlagXor(context);
            if (!verifyRet) {
                LOGE("VerifyFlagXor failed, ret: %{public}d", verifyRet);
                return ERR_DM_FAILED;
            }
        }
        // Query the ACL of the sink end. Compare the ACLs at both ends.
        context->softbusConnector->SyncLocalAclListProcess({context->accesser.deviceId, context->accesser.userId},
            {context->accessee.deviceId, context->accessee.userId}, context->accessee.aclStrList, false);
        context->accesser.deviceName = context->softbusConnector->GetLocalDeviceName();
        PutServiceInfos(context);
        // Save this acl
        SetAclInfo(context);
        UpdateCredInfo(context);
        context->authMessageProcessor->PutAccessControlList(context, context->accesser, context->accessee.deviceId);
        // Synchronize the local SP information, the format is uncertain, not done for now
        context->softbusConnector->SyncAclList();
    }

    std::string peerDeviceId = "";
    GetPeerDeviceId(context, peerDeviceId);
    bool isNeedJoinLnn = context->softbusConnector->CheckIsNeedJoinLnn(peerDeviceId, context->accessee.addr);
    // Trigger networking
    if ((!context->accesser.isOnline || isNeedJoinLnn) && context->isNeedJoinLnn) {
        JoinLnn(context);
    }
    context->reason = DM_OK;
    context->reply = DM_OK;
    context->state = static_cast<int32_t>(GetStateType());
    context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_AUTH_REQ_FINISH, context);
    LOGI("AuthSrcDataSyncState::Action ok");
    return DM_OK;
}

//LCOV_EXCL_START
DmAuthStateType AuthSrcDataSyncState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_DATA_SYNC_STATE;
}
//LCOV_EXCL_STOP

// Received 200 end message, send 201
int32_t AuthSinkFinishState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSinkFinishState::Action start");
    int32_t ret = DM_OK;
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    LOGI("reason: %{public}d", context->reason);
    if (context->reason == DM_OK) {
        context->state = static_cast<int32_t>(GetStateType());
        ret = FreezeProcess::GetInstance().DeleteFreezeRecord();
        LOGI("DeleteFreezeRecord ret: %{public}d", ret);
    }
    bool isNeedFreeze = FreezeProcess::GetInstance().IsNeedFreeze(context);
    bool isInWhiteList = AuthManagerBase::CheckProcessNameInWhiteList(context->accessee.bundleName);
    LOGI("isInWhiteList: %{public}d.", isInWhiteList);
    if (context->reason == ERR_DM_BIND_PIN_CODE_ERROR && (isNeedFreeze || !isInWhiteList)) {
        ret = FreezeProcess::GetInstance().UpdateFreezeRecord();
        LOGI("UpdateFreezeData ret: %{public}d", ret);
    }
    context->isNeedJoinLnn = true;
    SinkFinish(context);
    LOGI("AuthSinkFinishState::Action ok");
    if (context->cleanNotifyCallback != nullptr) {
        context->cleanNotifyCallback(context->logicalSessionId, context->connDelayCloseTime);
    }

    return DM_OK;
}

DmAuthStateType AuthSinkFinishState::GetStateType()
{
    return DmAuthStateType::AUTH_SINK_FINISH_STATE;
}

// Received 201 end message
int32_t AuthSrcFinishState::Action(std::shared_ptr<DmAuthContext> context)
{
    LOGI("AuthSrcFinishState::Action start");
    CHECK_NULL_RETURN(context, ERR_DM_POINT_NULL);
    if (context->reason == ERR_DM_SKIP_AUTHENTICATE && !context->isNeedAuthenticate) {
        CHECK_NULL_RETURN(context->authMessageProcessor, ERR_DM_POINT_NULL);
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_AUTH_REQ_FINISH, context);
        context->state = static_cast<int32_t>(GetStateType());
    } else if (context->reason != DM_OK && context->reason != DM_BIND_TRUST_TARGET) {
        context->authMessageProcessor->CreateAndSendMsg(MSG_TYPE_AUTH_REQ_FINISH, context);
    } else {
        context->state = static_cast<int32_t>(GetStateType());
    }
    context->isNeedJoinLnn = true;
    std::string peerDeviceId = "";
    GetPeerDeviceId(context, peerDeviceId);
    if (DeviceProfileConnector::GetInstance().IsAllowAuthAlways(context->accesser.deviceId, context->accesser.userId,
        peerDeviceId, context->pkgName, context->accesser.tokenId)) {
        context->confirmOperation = UiAction::USER_OPERATION_TYPE_ALLOW_AUTH_ALWAYS;
    }
    SourceFinish(context);
    CHECK_NULL_RETURN(context->softbusConnector, ERR_DM_POINT_NULL);
    bool isNeedJoinLnn = context->softbusConnector->CheckIsNeedJoinLnn(peerDeviceId, context->accessee.addr);
    // Trigger networking
    if (context->reason == DM_BIND_TRUST_TARGET && (!context->accesser.isOnline || isNeedJoinLnn)) {
        JoinLnn(context);
    }
    LOGI("AuthSrcFinishState::Action ok");
    if (context->reason != DM_OK && context->reason != DM_ALREADY_AUTHED) {
        context->connDelayCloseTime = 0;
    }
    if (context->cleanNotifyCallback != nullptr) {
        context->cleanNotifyCallback(context->logicalSessionId, context->connDelayCloseTime);
    }
    return DM_OK;
}

DmAuthStateType AuthSrcFinishState::GetStateType()
{
    return DmAuthStateType::AUTH_SRC_FINISH_STATE;
}
} // namespace DistributedHardware
} // namespace OHOS
