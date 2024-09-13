/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cJSON.h>
#include <errors.h>

#include "hisysevent.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(DmRadarHelper);
bool DmRadarHelper::ReportDiscoverRegCallback(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", info.hostName,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_DISCOVER),
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
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_REGISTER_CALLBACK),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "TO_CALL_PKG", info.toCallPkg,
            "COMM_SERV", info.commServ,
            "ERROR_CODE", info.errCode);
    }
    if (res != DM_OK) {
        LOGE("ReportDiscoverRegCallback error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportDiscoverResCallback(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", SOFTBUSNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_REGISTER_CALLBACK),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
            "PEER_UDID", GetAnonyUdid(info.peerUdid),
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "COMM_SERV", static_cast<int32_t>(CommServ::USE_SOFTBUS),
            "PEER_NET_ID", GetAnonyUdid(info.peerNetId));
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", SOFTBUSNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_REGISTER_CALLBACK),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "PEER_UDID", GetAnonyUdid(info.peerUdid),
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "COMM_SERV", static_cast<int32_t>(CommServ::USE_SOFTBUS),
            "ERROR_CODE", info.errCode);
    }

    if (res != DM_OK) {
        LOGE("ReportDiscoverResCallback error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportDiscoverUserRes(struct RadarInfo &info)
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
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_USER_DEAL_RES),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "COMM_SERV", static_cast<int32_t>(CommServ::USE_SOFTBUS));
    } else if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_USER_DEAL_RES),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END));
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", info.hostName,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_DISCOVER),
            "BIZ_STAGE", static_cast<int32_t>(DisCoverStage::DISCOVER_USER_DEAL_RES),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "COMM_SERV", static_cast<int32_t>(CommServ::USE_SOFTBUS),
            "ERROR_CODE", info.errCode);
    }
    if (res != DM_OK) {
        LOGE("ReportDiscoverUserRes error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthStart(const std::string &peerUdid, const std::string &pkgName)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", "AuthenticateDevice",
        "HOST_PKG", pkgName,
        "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_START),
        "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
        "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_START),
        "IS_TRUST", static_cast<int32_t>(TrustStatus::NOT_TRUST),
        "PEER_UDID", GetAnonyUdid(peerUdid),
        "LOCAL_UDID", GetAnonyLocalUdid());
    if (res != DM_OK) {
        LOGE("ReportAuthStart error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthOpenSession(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_OPEN_SESSION),
            "STAGE_RES", info.stageRes,
            "PEER_UDID", GetAnonyUdid(info.peerUdid),
            "LOCAL_UDID", GetAnonyLocalUdid(),
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
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_OPEN_SESSION),
            "BIZ_STATE", info.bizState,
            "STAGE_RES", info.stageRes,
            "PEER_UDID", GetAnonyUdid(info.peerUdid),
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "CH_ID", info.channelId,
            "IS_TRUST", info.isTrust,
            "COMM_SERV", info.commServ,
            "LOCAL_SESS_NAME", info.localSessName,
            "PEER_SESS_NAME", info.peerSessName,
            "ERROR_CODE", info.errCode);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthOpenSession error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthSessionOpenCb(struct RadarInfo &info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_OPEN_SESSION),
        "STAGE_RES", info.stageRes,
        "PEER_UDID", GetAnonyUdid(info.peerUdid),
        "LOCAL_UDID", GetAnonyLocalUdid(),
        "CH_ID", info.channelId);
    if (res != DM_OK) {
        LOGE("ReportAuthSessionOpenCb error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthSendRequest(struct RadarInfo &info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_SEND_REQUEST),
        "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
        "TO_CALL_PKG", SOFTBUSNAME,
        "LOCAL_SESS_NAME", DM_SESSION_NAME,
        "IS_TRUST", static_cast<int32_t>(TrustStatus::NOT_TRUST),
        "COMM_SERV", static_cast<int32_t>(CommServ::USE_SOFTBUS));
    if (res != DM_OK) {
        LOGE("ReportAuthSessionOpenCb error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthPullAuthBox(struct RadarInfo &info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_AUTH_BOX),
        "STAGE_RES", info.stageRes);
    if (res != DM_OK) {
        LOGE("ReportAuthPullAuthBox error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthConfirmBox(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_AUTH_BOX),
            "STAGE_RES", info.stageRes);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_AUTH_BOX),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "ERROR_CODE", info.errCode);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthConfirmBox error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthCreateGroup(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP),
            "STAGE_RES", info.stageRes,
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "TO_CALL_PKG", info.toCallPkg);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "TO_CALL_PKG", info.toCallPkg,
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "ERROR_CODE", info.errCode);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthCreateGroup error, res:%{public}d", res);
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
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP),
            "STAGE_RES", stageRes,
            "HOST_PKG", HICHAINNAME);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP),
            "STAGE_RES", stageRes,
            "HOST_PKG", HICHAINNAME,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "ERROR_CODE", GetErrCode(ERR_DM_CREATE_GROUP_FAILED));
    }
    if (res != DM_OK) {
        LOGE("ReportAuthCreateGroupCb error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthPullPinBox(struct RadarInfo &info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
        "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_PIN_BOX_START),
        "STAGE_RES", info.stageRes);
    if (res != DM_OK) {
        LOGE("ReportAuthPullPinBox error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthInputPinBox(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_PIN_INPUT_BOX_END),
            "STAGE_RES", info.stageRes);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_PULL_PIN_INPUT_BOX_END),
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "ERROR_CODE", info.errCode);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthInputPinBox error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthAddGroup(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP),
            "STAGE_RES", info.stageRes,
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "PEER_UDID", GetAnonyUdid(info.peerUdid),
            "TO_CALL_PKG", HICHAINNAME);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP),
            "STAGE_RES", info.stageRes,
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "PEER_UDID", GetAnonyUdid(info.peerUdid),
            "TO_CALL_PKG", HICHAINNAME,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "ERROR_CODE", info.errCode);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthAddGroup error, res:%{public}d", res);
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
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP),
            "STAGE_RES", stageRes,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "HOST_PKG", HICHAINNAME);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_AUTHCATION),
            "BIZ_STAGE", static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP),
            "STAGE_RES", stageRes,
            "HOST_PKG", HICHAINNAME,
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "ERROR_CODE", GetErrCode(ERR_DM_ADD_GROUP_FAILED));
    }
    if (res != DM_OK) {
        LOGE("ReportAuthAddGroupCb error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportNetworkOnline(struct RadarInfo &info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_NETWORK),
        "BIZ_STAGE", static_cast<int32_t>(NetworkStage::NETWORK_ONLINE),
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState,
        "LOCAL_UDID", GetAnonyLocalUdid(),
        "PEER_UDID", GetAnonyUdid(info.peerUdid),
        "PEER_NET_ID", GetAnonyUdid(info.peerNetId),
        "IS_TRUST", info.isTrust);
    if (res != DM_OK) {
        LOGE("ReportNetworkOnline error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportNetworkOffline(struct RadarInfo &info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_NETWORK),
        "BIZ_STAGE", static_cast<int32_t>(NetworkStage::NETWORK_OFFLINE),
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState,
        "LOCAL_UDID", GetAnonyLocalUdid(),
        "PEER_UDID", GetAnonyUdid(info.peerUdid),
        "PEER_NET_ID", GetAnonyUdid(info.peerNetId));
    if (res != DM_OK) {
        LOGE("ReportNetworkOffline error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportDeleteTrustRelation(struct RadarInfo &info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR,
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ORG_PKG", ORGPKGNAME,
        "HOST_PKG", info.hostName,
        "FUNC", info.funcName,
        "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_DELET_TRUST_RELATION),
        "BIZ_STAGE", static_cast<int32_t>(DeleteTrust::DELETE_TRUST),
        "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
        "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
        "LOCAL_UDID", GetAnonyLocalUdid(),
        "PEER_UDID", GetAnonyUdid(info.peerUdid),
        "PEER_NET_ID", GetAnonyUdid(info.peerNetId),
        "TO_CALL_PKG", info.toCallPkg);
    if (res != DM_OK) {
        LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
        return false;
    }
    return true;
}

void DmRadarHelper::ReportCreatePinHolder(std::string hostName,
    int32_t channelId, std::string peerUdid, int32_t errCode, int32_t stageRes)
{
    int32_t res = DM_OK;
    if (stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", hostName,
            "FUNC", "CreatePinHolder",
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_PIN_HOLDER),
            "BIZ_STAGE", static_cast<int32_t>(PinHolderStage::CREATE_PIN_HOLDER),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_START),
            "CH_ID", channelId,
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "PEER_UDID", GetAnonyUdid(peerUdid),
            "TO_CALL_PKG", SOFTBUSNAME);
        if (res != DM_OK) {
            LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
            return;
        }
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", hostName,
            "FUNC", "CreatePinHolder",
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_PIN_HOLDER),
            "BIZ_STAGE", static_cast<int32_t>(PinHolderStage::CREATE_PIN_HOLDER),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "CH_ID", channelId,
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "PEER_UDID", GetAnonyUdid(peerUdid),
            "TO_CALL_PKG", SOFTBUSNAME,
            "ERROR_CODE", errCode);
        if (res != DM_OK) {
            LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
            return;
        }
    }
    return;
}

void DmRadarHelper::ReportDestroyPinHolder(std::string hostName,
    std::string peerUdid, int32_t errCode, int32_t stageRes)
{
    int32_t res = DM_OK;
    if (stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", hostName,
            "FUNC", "DestroyPinHolder",
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_PIN_HOLDER),
            "BIZ_STAGE", static_cast<int32_t>(PinHolderStage::DESTROY_PIN_HOLDER),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "PEER_UDID", GetAnonyUdid(peerUdid),
            "TO_CALL_PKG", SOFTBUSNAME);
        if (res != DM_OK) {
            LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
            return;
        }
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", hostName,
            "FUNC", "DestroyPinHolder",
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_PIN_HOLDER),
            "BIZ_STAGE", static_cast<int32_t>(PinHolderStage::DESTROY_PIN_HOLDER),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "PEER_UDID", GetAnonyUdid(peerUdid),
            "TO_CALL_PKG", SOFTBUSNAME,
            "ERROR_CODE", errCode);
        if (res != DM_OK) {
            LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
            return;
        }
    }
    return;
}

void DmRadarHelper::ReportSendOrReceiveHolderMsg(int32_t bizStage, std::string funcName, std::string peerUdid)
{
    int32_t res = DM_OK;
    if (bizStage == static_cast<int32_t>(PinHolderStage::RECEIVE_DESTROY_PIN_HOLDER_MSG)) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_PIN_HOLDER),
            "BIZ_STAGE", bizStage,
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "PEER_UDID", GetAnonyUdid(peerUdid),
            "TO_CALL_PKG", SOFTBUSNAME);
        if (res != DM_OK) {
            LOGE("ReportSendOrReceiveHolderMsg error, res:%{public}d", res);
            return;
        }
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", funcName,
            "BIZ_SCENE", static_cast<int32_t>(AuthScene::DM_PIN_HOLDER),
            "BIZ_STAGE", bizStage,
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "PEER_UDID", GetAnonyUdid(peerUdid),
            "TO_CALL_PKG", SOFTBUSNAME);
        if (res != DM_OK) {
            LOGE("ReportSendOrReceiveHolderMsg error, res:%{public}d", res);
            return;
        }
    }
    return;
}

void DmRadarHelper::ReportGetTrustDeviceList(std::string hostName,
    std::string funcName, std::vector<DmDeviceInfo> &deviceInfoList, int32_t errCode)
{
    int32_t res = DM_OK;
    std::string discoverDevList = GetDeviceInfoList(deviceInfoList);
    if (errCode == DM_OK) {
        int32_t deviceCount = deviceInfoList.size();
        static std::string TrustCallerName = "";
        if (deviceCount > 0 && TrustCallerName != hostName) {
            TrustCallerName = hostName;
            res = HiSysEventWrite(
                OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
                DM_DISCOVER_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "HOST_PKG", hostName,
                "FUNC", funcName,
                "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_GET_TRUST_DEVICE_LIST),
                "BIZ_STAGE", static_cast<int32_t>(GetTrustDeviceList::GET_TRUST_DEVICE_LIST),
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
                "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
                "LOCAL_UDID", GetAnonyLocalUdid(),
                "DISCOVERY_DEVICE_LIST", discoverDevList);
        }
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", hostName,
            "FUNC", funcName,
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_GET_TRUST_DEVICE_LIST),
            "BIZ_STAGE", static_cast<int32_t>(GetTrustDeviceList::GET_TRUST_DEVICE_LIST),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "DISCOVERY_DEVICE_LIST", discoverDevList,
            "ERROR_CODE", GetErrCode(errCode));
    }
    return;
}

void DmRadarHelper::ReportDmBehavior(std::string hostName, std::string funcName, int32_t errCode)
{
    int32_t res = DM_OK;
    if (errCode == DM_OK) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", hostName,
            "FUNC", funcName,
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_BEHAVIOR),
            "BIZ_STAGE", DEFAULT_STAGE,
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "LOCAL_UDID", GetAnonyLocalUdid());
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "HOST_PKG", hostName,
            "FUNC", funcName,
            "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_BEHAVIOR),
            "BIZ_STAGE", DEFAULT_STAGE,
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "LOCAL_UDID", GetAnonyLocalUdid(),
            "ERROR_CODE", GetErrCode(errCode));
    }
    return;
}

void DmRadarHelper::ReportGetLocalDevInfo(std::string hostName,
    std::string funcName, DmDeviceInfo &info, int32_t errCode)
{
    int32_t res = DM_OK;
    static std::string localCallerName = "";
    if (localCallerName != hostName) {
        localCallerName = hostName;
        if (errCode == DM_OK) {
            res = HiSysEventWrite(
                OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
                DM_DISCOVER_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "HOST_PKG", hostName,
                "FUNC", funcName,
                "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_GET_LOCAL_DEVICE_INFO),
                "BIZ_STAGE", DEFAULT_STAGE,
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
                "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
                "LOCAL_UDID", GetAnonyLocalUdid(),
                "LOCAL_NET_ID", GetAnonyUdid(info.networkId));
        } else {
            res = HiSysEventWrite(
                OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
                DM_DISCOVER_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "HOST_PKG", hostName,
                "FUNC", funcName,
                "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_GET_LOCAL_DEVICE_INFO),
                "BIZ_STAGE", DEFAULT_STAGE,
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
                "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
                "LOCAL_UDID", GetAnonyLocalUdid(),
                "LOCAL_NET_ID", GetAnonyUdid(info.networkId),
                "ERROR_CODE", GetErrCode(errCode));
        }
    }
    return;
}

void DmRadarHelper::ReportGetDeviceInfo(std::string hostName,
    std::string funcName, DmDeviceInfo &info, int32_t errCode)
{
    int32_t res = DM_OK;
    static std::string callerName = "";
    if (callerName != hostName) {
        callerName = hostName;
        if (errCode == DM_OK) {
            res = HiSysEventWrite(
                OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
                DM_DISCOVER_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "HOST_PKG", hostName,
                "FUNC", funcName,
                "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_GET_DEVICE_INFO),
                "BIZ_STAGE", DEFAULT_STAGE,
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
                "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
                "LOCAL_UDID", GetAnonyLocalUdid(),
                "PEER_UDID", GetAnonyUdid(info.deviceId),
                "PEER_NET_ID", GetAnonyUdid(info.networkId));
        } else {
            res = HiSysEventWrite(
                OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
                DM_DISCOVER_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "HOST_PKG", hostName,
                "FUNC", funcName,
                "BIZ_SCENE", static_cast<int32_t>(DiscoverScene::DM_GET_DEVICE_INFO),
                "BIZ_STAGE", DEFAULT_STAGE,
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
                "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
                "LOCAL_UDID", GetAnonyLocalUdid(),
                "PEER_UDID", GetAnonyUdid(info.deviceId),
                "PEER_NET_ID", GetAnonyUdid(info.networkId),
                "ERROR_CODE", GetErrCode(errCode));
        }
    }
    return;
}

std::string DmRadarHelper::ConvertHexToString(uint16_t hex)
{
    std::stringstream str;
    int32_t with = 3;
    str << std::hex << std::setw(with) << std::setfill('0') << hex;
    std::string hexStr = str.str();
    transform(hexStr.begin(), hexStr.end(), hexStr.begin(), ::toupper);
    return hexStr;
}

std::string DmRadarHelper::GetDeviceInfoList(std::vector<DmDeviceInfo> &deviceInfoList)
{
    cJSON *deviceInfoJson = cJSON_CreateArray();
    if (deviceInfoJson == nullptr) {
        LOGE("deviceInfoJson is nullptr.");
        return "";
    }
    for (size_t i = 0; i < deviceInfoList.size(); i++) {
        cJSON *object = cJSON_CreateObject();
        if (object == nullptr) {
            LOGE("object is nullptr.");
            cJSON_Delete(deviceInfoJson);
            return "";
        }
        std::string devType = ConvertHexToString(deviceInfoList[i].deviceTypeId);
        cJSON_AddStringToObject(object, "PEER_DEV_TYPE", devType.c_str());
        cJSON_AddItemToArray(deviceInfoJson, object);
    }
    char *deviceInfoStr = cJSON_PrintUnformatted(deviceInfoJson);
    if (deviceInfoStr == nullptr) {
        LOGE("deviceInfoStr is nullptr.");
        cJSON_Delete(deviceInfoJson);
        return "";
    }
    std::string devInfoStr = std::string(deviceInfoStr);
    cJSON_Delete(deviceInfoJson);
    cJSON_free(deviceInfoStr);
    return devInfoStr;
}

std::string DmRadarHelper::GetAnonyUdid(std::string udid)
{
    if (udid.empty() || udid.length() < INVALID_UDID_LENGTH) {
        return "unknown";
    }
    return udid.substr(0, SUBSTR_UDID_LENGTH) + "**" + udid.substr(udid.length() - SUBSTR_UDID_LENGTH);
}

std::string DmRadarHelper::GetAnonyLocalUdid()
{
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    return GetAnonyUdid(std::string(localDeviceId));
}

int32_t DmRadarHelper::GetErrCode(int32_t errCode)
{
    auto flag = MAP_ERROR_CODE.find(errCode);
    if (flag == MAP_ERROR_CODE.end()) {
        return errCode;
    }
    return flag->second;
}

IDmRadarHelper *CreateDmRadarInstance()
{
    return &DmRadarHelper::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS