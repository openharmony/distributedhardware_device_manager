/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dm_radar_helper.h"
#include <errors.h>
#include "hisysevent.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_softbus_adapter_crypto.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DmRadarHelper);
bool DmRadarHelper::ReportDiscoverRegCallback(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_REGISTER_CALLBACK),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "TO_CALL_PKG", info.toCallPkg,
            "COMM_SERV", info.commServ);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_REGISTER_CALLBACK),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "TO_CALL_PKG", info.toCallPkg,
            "COMM_SERV", info.commServ,
            "ERROR_CODE", GetErrorCode(info.errCode, static_cast<int32_t>(Module::SOFTBUS)));
    }
    if (res != DM_OK) {
        LOGE("ReportDiscoverRegCallback error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportDiscoverResCallback(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_DISCOVER_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "HOST_PKG", SOFTBUSNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_DISCOVER),
        "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
        "PEER_UDID", GetAnonyUdid(info.peerUdid),
        "LOCAL_UDID", GetUdidHashByUdid(GetLocalUdid()),
        "COMM_SERV", static_cast<int32_t>(CommServ::USE_SOFTBUS),
        "PEER_NET_ID", info.peerNetId);
    if (res != DM_OK) {
        LOGE("ReportDiscoverResCallback error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportDiscoverUserRes(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_CANCEL)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", info.hostName,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_USER_DEAL_RES),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "COMM_SERV", info.commServ);
        if (res != DM_OK) {
            LOGE("ReportDiscoverUserRes error, res:%d", res);
            return false;
        }
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", info.hostName,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_USER_DEAL_RES),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "COMM_SERV", info.commServ,
            "ERROR_CODE", GetErrorCode(info.errCode, static_cast<int32_t>(Module::SOFTBUS)));
        if (res != DM_OK) {
            LOGE("ReportDiscoverUserRes error, res:%d", res);
            return false;
        }
    }

    return true;
}

bool DmRadarHelper::ReportAuthStart(std::string peerUdid)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", "AuthenticateDevice",
        "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_START),
        "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
        "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_START),
        "IS_TRUST", static_cast<int32_t>(TrustStatus::NOT_TRUST),
        "PEER_UDID", GetUdidHashByUdid(peerUdid),
        "LOCAL_UDID", GetUdidHashByUdid(GetLocalUdid()));
    if (res != DM_OK) {
        LOGE("ReportAuthStart error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthOpenSession(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_OPEN_SESSION),
            "STAGE_RES", info.stageRes,
            "PEER_UDID", GetUdidHashByUdid(info.peerUdid),
            "LOCAL_UDID", GetUdidHashByUdid(info.localUdid),
            "CH_ID", info.channelId,
            "IS_TRUST", info.isTrust,
            "COMM_SERV", info.commServ,
            "LOCAL_SESS_NAME", info.localSessName,
            "PEER_SESS_NAME", info.peerSessName);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_OPEN_SESSION),
            "BIZ_STATE", info.bizState,
            "STAGE_RES", info.stageRes,
            "PEER_UDID", GetUdidHashByUdid(info.peerUdid),
            "LOCAL_UDID", GetUdidHashByUdid(info.localUdid),
            "CH_ID", info.channelId,
            "IS_TRUST", info.isTrust,
            "COMM_SERV", info.commServ,
            "LOCAL_SESS_NAME", info.localSessName,
            "PEER_SESS_NAME", info.peerSessName,
            "ERROR_CODE", GetErrorCode(info.errCode, static_cast<int32_t>(Module::SOFTBUS)));
    }
    if (res != DM_OK) {
        LOGE("ReportAuthOpenSession error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthSessionOpenCb(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_OPEN_SESSION),
        "STAGE_RES", info.stageRes,
        "CH_ID", info.channelId);
    if (res != DM_OK) {
        LOGE("ReportAuthSessionOpenCb error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthSendRequest(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_SEND_REQUEST),
        "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
        "TO_CALL_PKG", SOFTBUSNAME,
        "LOCAL_SESS_NAME", DM_SESSION_NAME,
        "IS_TRUST", static_cast<int32_t>(TrustStatus::NOT_TRUST),
        "COMM_SERV", static_cast<int32_t>(CommServ::USE_SOFTBUS));
    if (res != DM_OK) {
        LOGE("ReportAuthSessionOpenCb error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthPullAuthBox(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_AUTH_BOX),
        "STAGE_RES", info.stageRes);
    if (res != DM_OK) {
        LOGE("ReportAuthPullAuthBox error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthConfirmBox(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_AUTH_BOX),
            "STAGE_RES", info.stageRes);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_AUTH_BOX),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "ERROR_CODE", GetErrorCode(info.errCode, static_cast<int32_t>(Module::USER)));
    }
    if (res != DM_OK) {
        LOGE("ReportAuthConfirmBox error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthCreateGroup(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP),
            "STAGE_RES", info.stageRes,
            "LOCAL_UDID", GetUdidHashByUdid(info.localUdid),
            "TO_CALL_PKG", info.toCallPkg);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "TO_CALL_PKG", info.toCallPkg,
            "LOCAL_UDID", GetUdidHashByUdid(info.localUdid),
            "ERROR_CODE", GetErrorCode(info.errCode, static_cast<int32_t>(Module::HICHAIN)));
    }
    if (res != DM_OK) {
        LOGE("ReportAuthCreateGroup error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthCreateGroupCb(std::string funcName, int32_t stageRes)
{
    int32_t res = DM_OK;
    if (stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP),
            "STAGE_RES", stageRes,
            "HOST_NAME", HICHAINNAME);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP),
            "STAGE_RES", stageRes,
            "HOST_NAME", HICHAINNAME,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "ERROR_CODE", GetErrorCode(ERR_DM_CREATE_GROUP_FAILED, static_cast<int32_t>(Module::HICHAIN)));
    }
    if (res != DM_OK) {
        LOGE("ReportAuthCreateGroupCb error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthPullPinBox(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_PIN_BOX_START),
        "STAGE_RES", info.stageRes);
    if (res != DM_OK) {
        LOGE("ReportAuthPullPinBox error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthInputPinBox(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_PIN_INPUT_BOX_END),
            "STAGE_RES", info.stageRes);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_PIN_INPUT_BOX_END),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "ERROR_CODE", GetErrorCode(info.errCode, static_cast<int32_t>(Module::USER)));
    }
    if (res != DM_OK) {
        LOGE("ReportAuthInputPinBox error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthAddGroup(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP),
            "STAGE_RES", info.stageRes,
            "LOCAL_UDID", GetUdidHashByUdid(GetLocalUdid()),
            "PEER_UDID", GetUdidHashByUdid(info.peerUdid),
            "TO_CALL_PKG", HICHAINNAME);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP),
            "STAGE_RES", info.stageRes,
            "LOCAL_UDID", GetUdidHashByUdid(GetLocalUdid()),
            "PEER_UDID", GetUdidHashByUdid(info.peerUdid),
            "TO_CALL_PKG", HICHAINNAME,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "ERROR_CODE", GetErrorCode(info.errCode, static_cast<int32_t>(Module::HICHAIN)));
    }
    if (res != DM_OK) {
        LOGE("ReportAuthAddGroup error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthAddGroupCb(std::string funcName, int32_t stageRes)
{
    int32_t res = DM_OK;
    if (stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP),
            "STAGE_RES", stageRes,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "HOST_NAME", HICHAINNAME);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP),
            "STAGE_RES", stageRes,
            "HOST_NAME", HICHAINNAME,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "ERROR_CODE", GetErrorCode(ERR_DM_ADD_GROUP_FAILED, static_cast<int32_t>(Module::HICHAIN)));
    }
    if (res != DM_OK) {
        LOGE("ReportAuthAddGroupCb error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportNetworkOnline(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_NETWORK),
        "BIZ_STAGE", static_cast<int32_t>(NetworkStage::NETWORK_ONLINE),
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState,
        "LOCAL_UDID", GetUdidHashByUdid(info.localUdid),
        "PEER_UDID", GetUdidHashByUdid(info.peerUdid),
        "PEER_NET_ID", info.peerNetId,
        "IS_TRUST", info.isTrust);
    if (res != DM_OK) {
        LOGE("ReportNetworkOnline error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportNetworkOffline(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_NETWORK),
        "BIZ_STAGE", static_cast<int32_t>(NetworkStage::NETWORK_OFFLINE),
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState,
        "LOCAL_UDID", GetUdidHashByUdid(info.localUdid),
        "PEER_UDID", GetUdidHashByUdid(info.peerUdid),
        "PEER_NET_ID", info.peerNetId);
    if (res != DM_OK) {
        LOGE("ReportNetworkOffline error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportDeleteTrustRelation(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_DELET_TRUST_RELATION),
        "BIZ_STAGE", static_cast<int32_t>(DeleteTrust::DELETE_TRUST),
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState,
        "LOCAL_UDID", GetUdidHashByUdid(info.localUdid),
        "PEER_UDID", GetUdidHashByUdid(info.peerUdid),
        "PEER_NET_ID", info.peerNetId,
        "TO_CALL_PKG", info.toCallPkg);
    if (res != DM_OK) {
        LOGE("ReportDeleteTrustRelation error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportGetTrustDeviceList(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(GetTrustDeviceList::GET_TRUST_DEVICE_LIST),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "LOCAL_UDID", info.localUdid,
            "PEER_UDID", info.peerUdid,
            "PEER_NET_ID", info.peerNetId);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", static_cast<int32_t>(BizScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(GetTrustDeviceList::GET_TRUST_DEVICE_LIST),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "LOCAL_UDID", info.localUdid,
            "PEER_UDID", info.peerUdid,
            "PEER_NET_ID", info.peerNetId,
            "ERROR_CODE", GetErrorCode(info.errCode, static_cast<int32_t>(Module::SOFTBUS)));
    }
    if (res != DM_OK) {
        LOGE("ReportDeleteTrustRelation error, res:%d", res);
        return false;
    }
    return true;
}

std::string DmRadarHelper::GetStringUdidList(std::vector<DmDeviceInfo> &deviceInfoList)
{
    std::string ret = "";
    for (unsigned int i = 0; i < deviceInfoList.size(); i++) {
        if (i == deviceInfoList.size() - 1) {
            ret += GetUdidHashByUdid(std::string(deviceInfoList[i].deviceId));
        } else {
            ret += GetUdidHashByUdid(std::string(deviceInfoList[i].deviceId));
            ret += ",";
        }
    }
    return ret;
}

std::string DmRadarHelper::GetStringNetIdList(std::vector<DmDeviceInfo> &deviceInfoList)
{
    std::string ret = "";
    for (unsigned int i = 0; i < deviceInfoList.size(); i++) {
        if (i == deviceInfoList.size() - 1) {
            ret += std::string(deviceInfoList[i].networkId);
        } else {
            ret += std::string(deviceInfoList[i].networkId);
            ret += ",";
        }
    }
    return ret;
}

std::string DmRadarHelper::GetUdidHashByUdid(std::string udid)
{
    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (DmSoftbusAdapterCrypto::GetUdidHash(udid, (uint8_t *)udidHash) != DM_OK) {
        return "";
    }
    return GetAnonyUdid(std::string(udidHash));
}

int32_t DmRadarHelper::GetErrorCode(int32_t errCode, int32_t module)
{
    return ErrCodeOffset(SUBSYS_DISTRIBUTEDHARDWARE_DM, module) + errCode;
}

std::string DmRadarHelper::GetAnonyUdid(std::string udid)
{
    if (udid.empty() || udid.length() < INVALID_UDID_LENGTH) {
        return "unknown";
    }
    return udid.substr(0, SUBSTR_UDID_LENGTH) + "**" + udid.substr(udid.length() - SUBSTR_UDID_LENGTH);
}

std::string DmRadarHelper::GetLocalUdid()
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    return std::string(localDeviceId);
}
} // namespace DistributedHardware
} // namespace OHOS