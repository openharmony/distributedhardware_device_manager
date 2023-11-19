/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_RADAR_HELPER_H
#define OHOS_DM_RADAR_HELPER_H

#include <cstdint>
#include <chrono>
#include <string>

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
constexpr const char* ORGPKGNAME = "ohos.distributedhardware.devicemanager";
constexpr const char* SOFTBUSNAME = "ohos.softbus";
constexpr const char* HICHAINNAME = "ohos.hichain";
constexpr int32_t SUBSYS_DISTRIBUTEDHARDWARE_DM = 204;
enum class BizScene : int32_t {
    DM_DISCOVER = 0x1,
    DM_AUTHCATION = 0x2,
    DM_NETWORK = 0x3,
    DM_DELET_TRUST_RELATION = 0x4,
    DM_GET_TRUST_DEVICE_LIST = 0x5,
};

enum class StageRes : int32_t {
    STAGE_IDLE = 0x0,
    STAGE_SUCC = 0x1,
    STAGE_FAIL = 0x2,
    STAGE_CANCEL = 0x3,
    STAGE_UNKNOW = 0x4,
};

enum class BizState : int32_t {
    BIZ_STATE_START = 0x1,
    BIZ_STATE_END = 0x2,
    BIZ_STATE_CANCEL = 0x3,
};

enum class DisCoverStage : int32_t {
    DISCOVER_REGISTER_CB_START = 0x1,
    DISCOVER_CB_END = 0x2,
    DISCOVER_USER_ACTION_RES = 0x3,
};

enum class AuthStage : int32_t {
    AUTH_START = 0x1,
    AUTH_OPEN_SESSION = 0x2,
    AUTH_PULL_AUTH_BOX = 0x3,
    AUTH_USER_CONFIRM_BOX_RESULT = 0x4,
    AUTH_CREATE_HICHAIN_GROUP = 0x5,
    AUTH_PULL_PIN_BOX_START = 0x6,
    AUTH_PULL_PIN_INPUT_BOX_END = 0x7,
    AUTH_ADD_HICHAIN_GROUP = 0x8,
};

enum class NetworkStage : int32_t {
    NETWORK_ONLINE = 0x1,
    NETWORK_OFFLINE = 0x2,
};

enum class DeleteTrust : int32_t {
    DELETE_TRUST = 0x1,
};

enum class GetTrustDeviceList : int32_t {
    GET_TRUST_DEVICE_LIST = 0x1,
};

enum class TrustStatus : int32_t {
    NOT_TRUST = 0x0,
    IS_TRUST = 0x1,
};

enum class CommServ : int32_t {
    NOT_USE_SOFTBUS = 0x0,
    USE_SOFTBUS = 0x1,
};

enum class Module : int32_t {
    DEVICE_MANAGER = 0x0,
    HICHAIN = 0x1,
    SOFTBUS = 0x2,
    USER = 0x3
};

struct RadarInfo {
    std::string funcName;
    int32_t stageRes;
    int32_t bizState;
    std::string toCallPkg;
    std::string hostName;
    int32_t errCode;
    std::string peerNetId;
    std::string localSessName;
    int32_t isTrust;
    int32_t commServ;
    std::string localUdid;
    std::string peerUdid;
    int32_t channelId;
};

class DmRadarHelper {
    DECLARE_SINGLE_INSTANCE(DmRadarHelper);
public:
    bool ReportDiscoverRegCallback(struct RadarInfo info);
    bool ReportDiscoverResCallback(struct RadarInfo info);
    bool ReportDiscoverUserRes(struct RadarInfo info);
    bool ReportAuthStart(struct RadarInfo info);
    bool ReportAuthOpenSession(struct RadarInfo info);
    bool ReportAuthSessionOpenCb(struct RadarInfo info);
    bool ReportAuthPullAuthBox(struct RadarInfo info);
    bool ReportAuthConfirmBox(struct RadarInfo info);
    bool ReportAuthCreateGroup(struct RadarInfo info);
    bool ReportAuthCreateGroupCb(struct RadarInfo info);
    bool ReportAuthPullPinBox(struct RadarInfo info);
    bool ReportAuthInputPinBox(struct RadarInfo info);
    bool ReportAuthAddGroup(struct RadarInfo info);
    bool ReportAuthAddGroupCb(struct RadarInfo info);
    bool ReportNetworkOnline(struct RadarInfo info);
    bool ReportNetworkOffline(struct RadarInfo info);
    bool ReportDeleteTrustRelation(struct RadarInfo info);
    bool ReportGetTrustDeviceList(struct RadarInfo info);
private:
    int32_t GetErrorCode(int32_t errCode, int32_t module);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_RADAR_HELPER_H