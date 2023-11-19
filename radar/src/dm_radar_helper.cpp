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

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DmRadarHelper);
bool DmRadarHelper::ReportDiscoverRegCallback(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == StageRes::STAGE_IDLE) {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            "DM_RADAR_EVENT",
            HiSysEvent::EventType::FAULT,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", BizScene::DM_DISCOVER,
            "BIZ_STAGE", DisCoverStage::DISCOVER_REGISTER_CB_START,
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "TO_CALL_PKG", info.toCallPkg,
            "COMM_SERV", info.commServ);
    } else {
        res = HiSysEventWrite(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            "DM_RADAR_EVENT",
            HiSysEvent::EventType::FAULT,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", info.funcName,
            "BIZ_SCENCE", BizScene::DM_DISCOVER,
            "BIZ_STAGE", DisCoverStage::DISCOVER_REGISTER_CB_START,
            "STAGE_RES", info.stageRes,
            "BIZ_STATE", info.bizState,
            "TO_CALL_PKG", info.toCallPkg,
            "COMM_SERV", info.commServ,
            "ERROR_CODE", GetErrorCode(info.errCode, Module::SOFTBUS));
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
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_DISCOVER,
        "BIZ_STAGE", DisCoverStage::DISCOVER_CB_END,
        "STAGE_RES", info.stageRes);
    if (res != DM_OK) {
        LOGE("ReportDiscoverResCallback error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportDiscoverUserRes(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_DISCOVER,
        "BIZ_STAGE", DisCoverStage::DISCOVER_USER_ACTION_RES,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
    if (res != DM_OK) {
        LOGE("ReportDiscoverUserRes error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthStart(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_START,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState,
        "IS_TRUST", info.isTrust);
    if (res != DM_OK) {
        LOGE("ReportAuthStart error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthOpenSession(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_OPEN_SESSION,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
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
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_OPEN_SESSION,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
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
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_PULL_AUTH_BOX,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
    if (res != DM_OK) {
        LOGE("ReportAuthPullAuthBox error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthConfirmBox(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_USER_CONFIRM_BOX_RESULT,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
    if (res != DM_OK) {
        LOGE("ReportAuthConfirmBox error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthCreateGroup(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_CREATE_HICHAIN_GROUP,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
    if (res != DM_OK) {
        LOGE("ReportAuthCreateGroup error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthCreateGroupCb(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_CREATE_HICHAIN_GROUP,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
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
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_PULL_PIN_BOX_START,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
    if (res != DM_OK) {
        LOGE("ReportAuthPullPinBox error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthInputPinBox(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_PULL_PIN_INPUT_BOX_END,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
    if (res != DM_OK) {
        LOGE("ReportAuthInputPinBox error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthAddGroup(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_ADD_HICHAIN_GROUP,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
    if (res != DM_OK) {
        LOGE("ReportAuthAddGroup error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthAddGroupCb(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_AUTHCATION,
        "BIZ_STAGE", AuthStage::AUTH_ADD_HICHAIN_GROUP,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
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
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_NETWORK,
        "BIZ_STAGE", NetworkStage::NETWORK_ONLINE,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
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
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_NETWORK,
        "BIZ_STAGE", NetworkStage::NETWORK_OFFLINE,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
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
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_DELET_TRUST_RELATION,
        "BIZ_STAGE", DeleteTrust::DELETE_TRUST,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
    if (res != DM_OK) {
        LOGE("ReportDeleteTrustRelation error, res:%d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportGetTrustDeviceList(struct RadarInfo info)
{
    int32_t res = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        "DM_RADAR_EVENT",
        HiSysEvent::EventType::FAULT,
        "ORG_PKG", ORGPKGNAME,
        "FUNC", info.funcName,
        "BIZ_SCENCE", BizScene::DM_GET_TRUST_DEVICE_LIST,
        "BIZ_STAGE", GetTrustDeviceList::GET_TRUST_DEVICE_LIST,
        "STAGE_RES", info.stageRes,
        "BIZ_STATE", info.bizState);
    if (res != DM_OK) {
        LOGE("ReportDeleteTrustRelation error, res:%d", res);
        return false;
    }
    return true;
}

int32_t DmRadarHelper::GetErrorCode(int32_t errCode, int32_t module)
{
    return ErrCodeOffset(SUBSYS_DISTRIBUTEDHARDWARE_DM, module) + errCode;
}
} // namespace DistributedHardware
} // namespace OHOS