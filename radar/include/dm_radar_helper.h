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
#include <vector>

#include "single_instance.h"

#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
const std::string ORGPKGNAME = "deviceManager";
const std::string SOFTBUSNAME = "dsoftbus";
const std::string HICHAINNAME = "hichain";
const std::string DM_DISCOVER_BEHAVIOR = "DM_DISCOVER_BEHAVIOR";
const std::string DM_AUTHCATION_BEHAVIOR = "DM_AUTHCATION_BEHAVIOR";
constexpr int32_t SUBSYS_DISTRIBUTEDHARDWARE_DM = 204;
constexpr int32_t INVALID_UDID_LENGTH = 10;
constexpr int32_t SUBSTR_UDID_LENGTH = 5;
enum class BizScene : int32_t {
    DM_DISCOVER = 0x1,
    DM_AUTHCATION = 0x2,
    DM_NETWORK = 0x3,
    DM_DELET_TRUST_RELATION = 0x4,
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
    DISCOVER_REGISTER_CALLBACK = 0x1,
    DISCOVER_USER_DEAL_RES = 0x2,
    DISCOVER_GET_TRUST_DEVICE_LIST = 0x3,
};

enum class AuthStage : int32_t {
    AUTH_START = 0x1,
    AUTH_OPEN_SESSION = 0x2,
    AUTH_SEND_REQUEST = 0x3,
    AUTH_PULL_AUTH_BOX = 0x4,
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
    std::string toCallPkg;
    std::string hostName;
    int32_t stageRes;
    int32_t bizState;
    std::string localSessName;
    std::string peerSessName;
    int32_t isTrust;
    int32_t commServ;
    std::string peerNetId;
    std::string localUdid;
    std::string peerUdid;
    std::string discoverDevList;
    int32_t channelId;
    int32_t errCode;
};

class IDmRadarHelper {
public:
    virtual ~IDmRadarHelper() {}
    /**
     * @tc.name: ReportDiscoverRegCallback
     * @tc.desc: report discover regsit callback
     * @tc.type: FUNC
     */
    virtual bool ReportDiscoverRegCallback(struct RadarInfo &info) = 0;
    virtual bool ReportDiscoverResCallback(struct RadarInfo &info) = 0;
    virtual bool ReportDiscoverUserRes(struct RadarInfo &info) = 0;
    virtual bool ReportAuthStart(const std::string &peerUdid, const std::string &pkgName) = 0;
    virtual bool ReportAuthOpenSession(struct RadarInfo &info) = 0;
    virtual bool ReportAuthSessionOpenCb(struct RadarInfo &info) = 0;
    virtual bool ReportAuthSendRequest(struct RadarInfo &info) = 0;
    virtual bool ReportAuthPullAuthBox(struct RadarInfo &info) = 0;
    virtual bool ReportAuthConfirmBox(struct RadarInfo &info) = 0;
    virtual bool ReportAuthCreateGroup(struct RadarInfo &info) = 0;
    virtual bool ReportAuthCreateGroupCb(std::string funcName, int32_t stageRes) = 0;
    virtual bool ReportAuthPullPinBox(struct RadarInfo &info) = 0;
    virtual bool ReportAuthInputPinBox(struct RadarInfo &info) = 0;
    virtual bool ReportAuthAddGroup(struct RadarInfo &info) = 0;
    virtual bool ReportAuthAddGroupCb(std::string funcName, int32_t stageRes) = 0;
    virtual bool ReportNetworkOnline(struct RadarInfo &info) = 0;
    virtual bool ReportNetworkOffline(struct RadarInfo &info) = 0;
    virtual bool ReportDeleteTrustRelation(struct RadarInfo &info) = 0;
    virtual bool ReportGetTrustDeviceList(struct RadarInfo &info) = 0;
    virtual std::string GetDeviceInfoList(std::vector<DmDeviceInfo> &deviceInfoList) = 0;
    virtual std::string GetUdidHashByUdid(std::string udid) = 0;
};

class DmRadarHelper : public IDmRadarHelper {
    DECLARE_SINGLE_INSTANCE(DmRadarHelper);
public:
    bool ReportDiscoverRegCallback(struct RadarInfo &info) override;
    bool ReportDiscoverResCallback(struct RadarInfo &info) override;
    bool ReportDiscoverUserRes(struct RadarInfo &info) override;
    bool ReportAuthStart(const std::string &peerUdid, const std::string &pkgName) override;
    bool ReportAuthOpenSession(struct RadarInfo &info) override;
    bool ReportAuthSessionOpenCb(struct RadarInfo &info) override;
    bool ReportAuthSendRequest(struct RadarInfo &info) override;
    bool ReportAuthPullAuthBox(struct RadarInfo &info) override;
    bool ReportAuthConfirmBox(struct RadarInfo &info) override;
    bool ReportAuthCreateGroup(struct RadarInfo &info) override;
    bool ReportAuthCreateGroupCb(std::string funcName, int32_t stageRes) override;
    bool ReportAuthPullPinBox(struct RadarInfo &info) override;
    bool ReportAuthInputPinBox(struct RadarInfo &info) override;
    bool ReportAuthAddGroup(struct RadarInfo &info) override;
    bool ReportAuthAddGroupCb(std::string funcName, int32_t stageRes) override;
    bool ReportNetworkOnline(struct RadarInfo &info) override;
    bool ReportNetworkOffline(struct RadarInfo &info) override;
    bool ReportDeleteTrustRelation(struct RadarInfo &info) override;
    bool ReportGetTrustDeviceList(struct RadarInfo &info) override;
    std::string GetDeviceInfoList(std::vector<DmDeviceInfo> &deviceInfoList) override;
    std::string GetUdidHashByUdid(std::string udid) override;
    std::string ConvertHexToString(uint16_t hex);
private:
    int32_t GetErrorCode(int32_t errCode, int32_t module);
    std::string GetAnonyUdid(std::string udid);
    std::string GetLocalUdid();
};

extern "C" IDmRadarHelper *CreateDmRadarInstance();
using CreateDmRadarFuncPtr = IDmRadarHelper *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_RADAR_HELPER_H