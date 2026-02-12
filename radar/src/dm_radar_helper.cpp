/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "accesstoken_kit.h"
#include "access_token.h"
#include "hap_token_info.h"
#include "ipc_skeleton.h"
#include "native_token_info.h"
#include "tokenid_kit.h"
using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace DistributedHardware {
const char* ORGPKGNAME = "deviceManager";
const char* SOFTBUSNAME = "dsoftbus";
const char* HICHAINNAME = "hichain";
constexpr int32_t INVALID_UDID_LENGTH = 10;
constexpr int32_t SUBSTR_UDID_LENGTH = 5;
constexpr int32_t DEFAULT_STAGE = 1;
constexpr const char* DM_DISCOVER_BEHAVIOR = "DM_DISCOVER_BEHAVIOR";
constexpr const char* DM_AUTHCATION_BEHAVIOR = "DM_AUTHCATION_BEHAVIOR";
DM_IMPLEMENT_SINGLE_INSTANCE(DmRadarHelper);

int32_t DmRadarHelper::ReportDiscoverRegCallbackStageIdle(struct RadarInfo &info)
{
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)info.hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_DISCOVER), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DisCoverStage::DISCOVER_REGISTER_CALLBACK), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
        {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)info.toCallPkg.c_str() }, .arraySize = 0, },
        {.name = "COMM_SERV", .t = HISYSEVENT_INT32, .v = { .i32 = info.commServ, }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_DISCOVER_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

bool DmRadarHelper::ReportDiscoverRegCallback(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = ReportDiscoverRegCallbackStageIdle(info);
    } else {
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)info.hostName.c_str() }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
            {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_DISCOVER), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(DisCoverStage::DISCOVER_REGISTER_CALLBACK), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
            {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)info.toCallPkg.c_str() }, .arraySize = 0, },
            {.name = "COMM_SERV", .t = HISYSEVENT_INT32, .v = { .i32 = info.commServ, }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = info.errCode, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    }
    if (res != DM_OK) {
        LOGE("ReportDiscoverRegCallback error, res:%{public}d", res);
        return false;
    }
    return true;
}

int32_t DmRadarHelper::ReportDiscoverResCallbackStageSucc(struct RadarInfo &info)
{
    std::string peerUdid = GetAnonyUdid(info.peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    std::string peerNetId = GetAnonyUdid(info.peerNetId);
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)SOFTBUSNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_DISCOVER), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DisCoverStage::DISCOVER_REGISTER_CALLBACK), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "COMM_SERV", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(CommServ::USE_SOFTBUS), }, .arraySize = 0, },
        {.name = "PEER_NET_ID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerNetId.c_str() }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_DISCOVER_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

bool DmRadarHelper::ReportDiscoverResCallback(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = ReportDiscoverResCallbackStageSucc(info);
    } else {
        std::string peerUdid = GetAnonyUdid(info.peerUdid);
        std::string localUdid = GetAnonyLocalUdid();
        std::string peerNetId = GetAnonyUdid(info.peerNetId);
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)SOFTBUSNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
            {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_DISCOVER), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(DisCoverStage::DISCOVER_REGISTER_CALLBACK), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_FAIL), }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
            {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
            {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
            {.name = "COMM_SERV", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(CommServ::USE_SOFTBUS), }, .arraySize = 0, },
            {.name = "PEER_NET_ID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerNetId.c_str() }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = info.errCode, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_DISCOVER_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    }
    if (res != DM_OK) {
        LOGE("ReportDiscoverResCallback error, res:%{public}d", res);
        return false;
    }
    return true;
}

int32_t DmRadarHelper::ReportDiscoverUserResStageCancel(struct RadarInfo &info)
{
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)info.hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_DISCOVER), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DisCoverStage::DISCOVER_USER_DEAL_RES), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "COMM_SERV", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(CommServ::USE_SOFTBUS), }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_DISCOVER_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

int32_t DmRadarHelper::ReportDiscoverUserResStageSucc(struct RadarInfo &info)
{
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)info.hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_DISCOVER), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DisCoverStage::DISCOVER_USER_DEAL_RES), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "COMM_SERV", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(CommServ::USE_SOFTBUS), }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_DISCOVER_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

int32_t DmRadarHelper::ReportDiscoverUserResStageOther(struct RadarInfo &info)
{
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)info.hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_DISCOVER), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DisCoverStage::DISCOVER_USER_DEAL_RES), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "COMM_SERV", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(CommServ::USE_SOFTBUS), }, .arraySize = 0, },
        {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = info.errCode, }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_DISCOVER_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

bool DmRadarHelper::ReportDiscoverUserRes(struct RadarInfo info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_CANCEL)) {
        res = ReportDiscoverUserResStageCancel(info);
    } else if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = ReportDiscoverUserResStageSucc(info);
    } else {
        res = ReportDiscoverUserResStageOther(info);
    }
    if (res != DM_OK) {
        LOGE("ReportDiscoverUserRes error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthStart(const std::string &peerUdid, const std::string &pkgName)
{
    char funcName[] = "AuthenticateDevice";
    std::string peerUdidTmp = GetAnonyUdid(peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = funcName }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)pkgName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_START), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_START), }, .arraySize = 0, },
        {.name = "IS_TRUST", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(TrustStatus::NOT_TRUST), }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdidTmp.c_str() }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    if (res != DM_OK) {
        LOGE("ReportAuthStart error, res:%{public}d", res);
        return false;
    }
    return true;
}

int32_t DmRadarHelper::ReportAuthOpenSessionStageIdle(struct RadarInfo &info)
{
    std::string peerUdid = GetAnonyUdid(info.peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_OPEN_SESSION), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "CH_ID", .t = HISYSEVENT_INT32, .v = { .i32 = info.channelId, }, .arraySize = 0, },
        {.name = "IS_TRUST", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(TrustStatus::NOT_TRUST), }, .arraySize = 0, },
        {.name = "COMM_SERV", .t = HISYSEVENT_INT32, .v = { .i32 = info.commServ, }, .arraySize = 0, },
        {.name = "LOCAL_SESS_NAME", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)info.localSessName.c_str() }, .arraySize = 0, },
        {.name = "PEER_SESS_NAME", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)info.peerSessName.c_str() }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

bool DmRadarHelper::ReportAuthOpenSession(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = ReportAuthOpenSessionStageIdle(info);
    } else {
        std::string peerUdid = GetAnonyUdid(info.peerUdid);
        std::string localUdid = GetAnonyLocalUdid();
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_OPEN_SESSION), }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
            {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
            {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
            {.name = "CH_ID", .t = HISYSEVENT_INT32, .v = { .i32 = info.channelId, }, .arraySize = 0, },
            {.name = "IS_TRUST", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(TrustStatus::NOT_TRUST), }, .arraySize = 0, },
            {.name = "COMM_SERV", .t = HISYSEVENT_INT32, .v = { .i32 = info.commServ, }, .arraySize = 0, },
            {.name = "LOCAL_SESS_NAME", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)info.localSessName.c_str() }, .arraySize = 0, },
            {.name = "PEER_SESS_NAME", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)info.peerSessName.c_str() }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = info.errCode, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthOpenSession error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthSessionOpenCb(struct RadarInfo &info)
{
    std::string peerUdid = GetAnonyUdid(info.peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_OPEN_SESSION), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "CH_ID", .t = HISYSEVENT_INT32, .v = { .i32 = info.channelId, }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    if (res != DM_OK) {
        LOGE("ReportAuthSessionOpenCb error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthSendRequest(struct RadarInfo &info)
{
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_SEND_REQUEST), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "LOCAL_SESS_NAME", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)DM_SESSION_NAME }, .arraySize = 0, },
        {.name = "IS_TRUST", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(TrustStatus::NOT_TRUST), }, .arraySize = 0, },
        {.name = "COMM_SERV", .t = HISYSEVENT_INT32, .v = { .i32 = info.commServ, }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    if (res != DM_OK) {
        LOGE("ReportAuthSessionOpenCb error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthPullAuthBox(struct RadarInfo &info)
{
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_PULL_AUTH_BOX), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
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
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_PULL_AUTH_BOX), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    } else {
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_PULL_AUTH_BOX), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = info.errCode, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthConfirmBox error, res:%{public}d", res);
        return false;
    }
    return true;
}

int32_t DmRadarHelper::ReportAuthCreateGroupStageIdle(struct RadarInfo &info)
{
    std::string localUdid = GetAnonyLocalUdid();
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
        {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)info.toCallPkg.c_str() }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

bool DmRadarHelper::ReportAuthCreateGroup(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = ReportAuthCreateGroupStageIdle(info);
    } else {
        std::string localUdid = GetAnonyLocalUdid();
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
            {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)info.toCallPkg.c_str() }, .arraySize = 0, },
            {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = info.errCode, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
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
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = stageRes, }, .arraySize = 0, },
            {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)HICHAINNAME }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    } else {
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_CREATE_HICHAIN_GROUP), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = stageRes, }, .arraySize = 0, },
            {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)HICHAINNAME }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32,
                .v = { .i32 = GetErrCode(ERR_DM_CREATE_GROUP_FAILED), }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthCreateGroupCb error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportAuthPullPinBox(struct RadarInfo &info)
{
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_PULL_PIN_BOX_START), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
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
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_PULL_PIN_INPUT_BOX_END), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    } else {
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_PULL_PIN_INPUT_BOX_END), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = info.errCode, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthInputPinBox error, res:%{public}d", res);
        return false;
    }
    return true;
}

int32_t DmRadarHelper::ReportAuthAddGroupStageIdle(struct RadarInfo &info)
{
    std::string peerUdid = GetAnonyUdid(info.peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
        {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)HICHAINNAME }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

bool DmRadarHelper::ReportAuthAddGroup(struct RadarInfo &info)
{
    int32_t res = DM_OK;
    if (info.stageRes == static_cast<int32_t>(StageRes::STAGE_IDLE)) {
        res = ReportAuthAddGroupStageIdle(info);
    } else {
        std::string peerUdid = GetAnonyUdid(info.peerUdid);
        std::string localUdid = GetAnonyLocalUdid();
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
            {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
            {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
            {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)HICHAINNAME }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = info.errCode, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
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
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = stageRes, }, .arraySize = 0, },
            {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)HICHAINNAME }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    } else {
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_AUTHCATION), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthStage::AUTH_ADD_HICHAIN_GROUP), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = stageRes, }, .arraySize = 0, },
            {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)HICHAINNAME }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32,
                .v = { .i32 = GetErrCode(ERR_DM_ADD_GROUP_FAILED), }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    }
    if (res != DM_OK) {
        LOGE("ReportAuthAddGroupCb error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportNetworkOnline(struct RadarInfo &info)
{
    std::string peerUdid = GetAnonyUdid(info.peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    std::string peerNetId = GetAnonyUdid(info.peerNetId);
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_NETWORK), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(NetworkStage::NETWORK_ONLINE), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_NET_ID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerNetId.c_str() }, .arraySize = 0, },
        {.name = "IS_TRUST", .t = HISYSEVENT_INT32, .v = { .i32 = info.isTrust, }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    if (res != DM_OK) {
        LOGE("ReportNetworkOnline error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportNetworkOffline(struct RadarInfo &info)
{
    std::string peerUdid = GetAnonyUdid(info.peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    std::string peerNetId = GetAnonyUdid(info.peerNetId);
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_NETWORK), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(NetworkStage::NETWORK_OFFLINE), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = info.stageRes, }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = info.bizState, }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_NET_ID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerNetId.c_str() }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    if (res != DM_OK) {
        LOGE("ReportNetworkOffline error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DmRadarHelper::ReportDeleteTrustRelation(struct RadarInfo &info)
{
    std::string peerUdid = GetAnonyUdid(info.peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    std::string peerNetId = GetAnonyUdid(info.peerNetId);
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)info.funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_DELET_TRUST_RELATION), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DeleteTrust::DELETE_TRUST), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_NET_ID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerNetId.c_str() }, .arraySize = 0, },
        {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)info.toCallPkg.c_str() }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    if (res != DM_OK) {
        LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
        return false;
    }
    return true;
}

int32_t DmRadarHelper::ReportCreatePinHolderStageSucc(std::string hostName,
    int32_t channelId, std::string peerUdid, int32_t errCode, int32_t stageRes)
{
    char funcName[] =  "CreatePinHolder";
    std::string peerUdidTmp = GetAnonyUdid(peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = funcName }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_PIN_HOLDER), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(PinHolderStage::CREATE_PIN_HOLDER), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_START), }, .arraySize = 0, },
        {.name = "CH_ID", .t = HISYSEVENT_INT32, .v = { .i32 = channelId, }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdidTmp.c_str() }, .arraySize = 0, },
        {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)SOFTBUSNAME }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

void DmRadarHelper::ReportCreatePinHolder(std::string hostName,
    int32_t channelId, std::string peerUdid, int32_t errCode, int32_t stageRes)
{
    int32_t res = DM_OK;
    char funcName[] =  "CreatePinHolder";
    if (stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = ReportCreatePinHolderStageSucc(hostName, channelId, peerUdid, errCode, stageRes);
        if (res != DM_OK) {
            LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
            return;
        }
    } else {
        std::string peerUdidTmp = GetAnonyUdid(peerUdid);
        std::string localUdid = GetAnonyLocalUdid();
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = funcName }, .arraySize = 0, },
            {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_PIN_HOLDER), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(PinHolderStage::CREATE_PIN_HOLDER), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_FAIL), }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
            {.name = "CH_ID", .t = HISYSEVENT_INT32, .v = { .i32 = channelId, }, .arraySize = 0, },
            {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
            {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdidTmp.c_str() }, .arraySize = 0, },
            {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)SOFTBUSNAME }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = errCode, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
        if (res != DM_OK) {
            LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
            return;
        }
    }
    return;
}

int32_t DmRadarHelper::ReportDestroyPinHolderStageSucc(std::string hostName,
    std::string peerUdid, int32_t errCode, int32_t stageRes)
{
    char funcName[] = "DestroyPinHolder";
    std::string peerUdidTmp = GetAnonyUdid(peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = funcName }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_PIN_HOLDER), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(PinHolderStage::DESTROY_PIN_HOLDER), }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdidTmp.c_str() }, .arraySize = 0, },
        {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)SOFTBUSNAME }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

void DmRadarHelper::ReportDestroyPinHolder(std::string hostName,
    std::string peerUdid, int32_t errCode, int32_t stageRes)
{
    int32_t res = DM_OK;
    char funcName[] = "DestroyPinHolder";
    if (stageRes == static_cast<int32_t>(StageRes::STAGE_SUCC)) {
        res = ReportDestroyPinHolderStageSucc(hostName, peerUdid, errCode, stageRes);
        if (res != DM_OK) {
            LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
            return;
        }
    } else {
        std::string peerUdidTmp = GetAnonyUdid(peerUdid);
        std::string localUdid = GetAnonyLocalUdid();
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = funcName }, .arraySize = 0, },
            {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_PIN_HOLDER), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(PinHolderStage::DESTROY_PIN_HOLDER), }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_FAIL), }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
            {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
            {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdidTmp.c_str() }, .arraySize = 0, },
            {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)SOFTBUSNAME }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = errCode, }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
        if (res != DM_OK) {
            LOGE("ReportDeleteTrustRelation error, res:%{public}d", res);
            return;
        }
    }
    return;
}

int32_t DmRadarHelper::ReportSendOrReceiveHolderMsgStageOther(int32_t bizStage,
    std::string funcName, std::string peerUdid)
{
    std::string peerUdidTmp = GetAnonyUdid(peerUdid);
    std::string localUdid = GetAnonyLocalUdid();
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(AuthScene::DM_PIN_HOLDER), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v = { .i32 = bizStage, }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdidTmp.c_str() }, .arraySize = 0, },
        {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)SOFTBUSNAME }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

void DmRadarHelper::ReportSendOrReceiveHolderMsg(int32_t bizStage, std::string funcName, std::string peerUdid)
{
    int32_t res = DM_OK;
    if (bizStage == static_cast<int32_t>(PinHolderStage::RECEIVE_DESTROY_PIN_HOLDER_MSG)) {
        std::string peerUdidTmp = GetAnonyUdid(peerUdid);
        std::string localUdid = GetAnonyLocalUdid();
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(AuthScene::DM_PIN_HOLDER), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v = { .i32 = bizStage, }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
            {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
            {.name = "PEER_UDID", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)peerUdidTmp.c_str() }, .arraySize = 0, },
            {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)SOFTBUSNAME }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
        if (res != DM_OK) {
            LOGE("ReportSendOrReceiveHolderMsg error, res:%{public}d", res);
            return;
        }
    } else {
        res = ReportSendOrReceiveHolderMsgStageOther(bizStage, funcName, peerUdid);
        if (res != DM_OK) {
            LOGE("ReportSendOrReceiveHolderMsg error, res:%{public}d", res);
            return;
        }
    }
    return;
}

int32_t DmRadarHelper::ReportGetTrustDeviceListResultFailed(std::string hostName,
    std::string funcName, std::vector<DmDeviceInfo> &deviceInfoList,
    int32_t errCode, std::string localUdid, std::string discoverDevList)
{
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_GET_TRUST_DEVICE_LIST), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(GetTrustDeviceList::GET_TRUST_DEVICE_LIST), },
            .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_FAIL), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "DISCOVERY_DEVICE_LIST", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)discoverDevList.c_str() }, .arraySize = 0, },
        {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = GetErrCode(errCode), }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

void DmRadarHelper::ReportGetTrustDeviceList(std::string hostName,
    std::string funcName, std::vector<DmDeviceInfo> &deviceInfoList, int32_t errCode, std::string localUdid)
{
    int32_t res = DM_OK;
    std::string discoverDevList = GetDeviceInfoList(deviceInfoList);
    if (errCode == DM_OK) {
        int32_t deviceCount = static_cast<int32_t>(deviceInfoList.size());
        static std::string TrustCallerName = "";
        if (deviceCount > 0 && TrustCallerName != hostName) {
            TrustCallerName = hostName;
            HiSysEventParam params[] = {
                {.name = "ORG_PKG", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
                {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
                {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
                {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
                {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_GET_TRUST_DEVICE_LIST), }, .arraySize = 0, },
                {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(GetTrustDeviceList::GET_TRUST_DEVICE_LIST), },
                    .arraySize = 0, },
                {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
                {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
                {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
                {.name = "DISCOVERY_DEVICE_LIST", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)discoverDevList.c_str() }, .arraySize = 0, },
            };
            size_t len = sizeof(params) / sizeof(params[0]);
            res = OH_HiSysEvent_Write(
                OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
                DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
        }
    } else {
        res = ReportGetTrustDeviceListResultFailed(hostName, funcName,
            deviceInfoList, errCode, localUdid, discoverDevList);
    }
    if (res != DM_OK) {
        LOGE("ReportGetTrustDeviceList error, res:%{public}d", res);
        return;
    }
}

int32_t DmRadarHelper::ReportDmBehaviorResultSucc(std::string hostName, std::string funcName,
    int32_t errCode, std::string localUdid)
{
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_BEHAVIOR), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v = { .i32 = DEFAULT_STAGE, }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING,
            .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

void DmRadarHelper::ReportDmBehavior(std::string hostName, std::string funcName, int32_t errCode,
    std::string localUdid)
{
    int32_t res = DM_OK;
    if (errCode == DM_OK) {
        res = ReportDmBehaviorResultSucc(hostName, funcName, errCode, localUdid);
    } else {
        HiSysEventParam params[] = {
            {.name = "ORG_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
            {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
            {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
            {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
            {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_BEHAVIOR), }, .arraySize = 0, },
            {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v = { .i32 = DEFAULT_STAGE, }, .arraySize = 0, },
            {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_FAIL), }, .arraySize = 0, },
            {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
            {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING,
                .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
            {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = GetErrCode(errCode), }, .arraySize = 0, },
        };
        size_t len = sizeof(params) / sizeof(params[0]);
        res = OH_HiSysEvent_Write(
            OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
            DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
    }
    if (res != DM_OK) {
        LOGE("ReportDmBehavior error, res:%{public}d", res);
        return;
    }
}

int32_t DmRadarHelper::ReportGetLocalDevInfoResultSucc(std::string hostName,
    std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid)
{
    std::string devType = ConvertHexToString(info.deviceTypeId);
    std::string localNetId = GetAnonyUdid(info.networkId);
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_GET_LOCAL_DEVICE_INFO), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v = { .i32 = DEFAULT_STAGE, }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "DEV_TYPE", .t = HISYSEVENT_STRING, .v = { .s = (char *)devType.c_str() }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "LOCAL_NET_ID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localNetId.c_str() }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

void DmRadarHelper::ReportGetLocalDevInfo(std::string hostName,
    std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid)
{
    int32_t res = DM_OK;
    bool compare = false;
    {
        std::lock_guard<std::mutex> autoLock(lock_);
        if (localCallerName_ != hostName) {
            compare = true;
            localCallerName_ = hostName;
        }
    }
    if (compare) {
        if (errCode == DM_OK) {
            res = ReportGetLocalDevInfoResultSucc(hostName, funcName, info, errCode, localUdid);
        } else {
            std::string devType = ConvertHexToString(info.deviceTypeId);
            std::string localNetId = GetAnonyUdid(info.networkId);
            HiSysEventParam params[] = {
                {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
                {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
                {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
                {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
                {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_GET_LOCAL_DEVICE_INFO), }, .arraySize = 0, },
                {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v = { .i32 = DEFAULT_STAGE, }, .arraySize = 0, },
                {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_FAIL), }, .arraySize = 0, },
                {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
                {.name = "DEV_TYPE", .t = HISYSEVENT_STRING, .v = { .s = (char *)devType.c_str() }, .arraySize = 0, },
                {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
                {.name = "LOCAL_NET_ID", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)localNetId.c_str() }, .arraySize = 0, },
                {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = GetErrCode(errCode), }, .arraySize = 0, },
            };
            size_t len = sizeof(params) / sizeof(params[0]);
            res = OH_HiSysEvent_Write(
                OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
                DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
        }
    }
    if (res != DM_OK) {
        LOGE("ReportGetLocalDevInfo error, res:%{public}d", res);
        return;
    }
}

int32_t DmRadarHelper::ReportGetDeviceInfoResultSucc(std::string hostName,
    std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid)
{
    std::string devType = ConvertHexToString(info.deviceTypeId);
    std::string peerUdid = GetAnonyUdid(info.deviceId);
    std::string peerNetId = GetAnonyUdid(info.networkId);
    HiSysEventParam params[] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
        {.name = "HOST_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
        {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_GET_DEVICE_INFO), }, .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v = { .i32 = DEFAULT_STAGE, }, .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC), }, .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
            .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
        {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
        {.name = "DEV_TYPE", .t = HISYSEVENT_STRING, .v = { .s = (char *)devType.c_str() }, .arraySize = 0, },
        {.name = "PEER_UDID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
        {.name = "PEER_NET_ID", .t = HISYSEVENT_STRING, .v = { .s = (char *)peerNetId.c_str() }, .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    return OH_HiSysEvent_Write(
        OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
        DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
}

void DmRadarHelper::ReportGetDeviceInfo(std::string hostName,
    std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid)
{
    int32_t res = DM_OK;
    static std::string callerName = "";
    if (callerName != hostName) {
        callerName = hostName;
        if (errCode == DM_OK) {
            res = ReportGetDeviceInfoResultSucc(hostName, funcName, info, errCode, localUdid);
        } else {
            std::string devType = ConvertHexToString(info.deviceTypeId);
            std::string peerUdid = GetAnonyUdid(info.deviceId);
            std::string peerNetId = GetAnonyUdid(info.networkId);
            HiSysEventParam params[] = {
                {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)ORGPKGNAME }, .arraySize = 0, },
                {.name = "HOST_PKG", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)hostName.c_str() }, .arraySize = 0, },
                {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)funcName.c_str() }, .arraySize = 0, },
                {.name = "API_TYPE", .t = HISYSEVENT_INT32, .v = { .i32 = GetApiType(), }, .arraySize = 0, },
                {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(DiscoverScene::DM_GET_DEVICE_INFO), }, .arraySize = 0, },
                {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v = { .i32 = DEFAULT_STAGE, }, .arraySize = 0, },
                {.name = "STAGE_RES", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_FAIL), }, .arraySize = 0, },
                {.name = "BIZ_STATE", .t = HISYSEVENT_INT32,
                    .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_END), }, .arraySize = 0, },
                {.name = "LOCAL_UDID", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)localUdid.c_str() }, .arraySize = 0, },
                {.name = "DEV_TYPE", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)devType.c_str() }, .arraySize = 0, },
                {.name = "PEER_UDID", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)peerUdid.c_str() }, .arraySize = 0, },
                {.name = "PEER_NET_ID", .t = HISYSEVENT_STRING,
                    .v = { .s = (char *)peerNetId.c_str() }, .arraySize = 0, },
                {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = GetErrCode(errCode), }, .arraySize = 0, },
            };
            size_t len = sizeof(params) / sizeof(params[0]);
            res = OH_HiSysEvent_Write(
                OHOS::HiviewDFX::HiSysEvent::Domain::DISTRIBUTED_DEVICE_MANAGER,
                DM_AUTHCATION_BEHAVIOR, HISYSEVENT_BEHAVIOR, params, len);
        }
    }
    if (res != DM_OK) {
        LOGE("ReportGetDeviceInfo error, res:%{public}d", res);
        return;
    }
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

int32_t DmRadarHelper::GetApiType()
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("GetApiType GetCallingTokenID error.");
        return static_cast<int32_t>(ApiType::API_UNKNOW);
    }
    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_HAP) {
        return static_cast<int32_t>(ApiType::API_JS);
    } else if (tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        return static_cast<int32_t>(ApiType::API_NATIVE);
    }
    return static_cast<int32_t>(ApiType::API_UNKNOW);
}

IDmRadarHelper *CreateDmRadarInstance()
{
    return &DmRadarHelper::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS