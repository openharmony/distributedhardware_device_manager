/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "dm_single_instance.h"

#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
DM_EXPORT extern const char* ORGPKGNAME;
DM_EXPORT extern const char* SOFTBUSNAME;
DM_EXPORT extern const char* HICHAINNAME;
enum class DiscoverScene : int32_t {
    DM_DISCOVER = 0x1,
    DM_GET_TRUST_DEVICE_LIST = 0x2,
    DM_GET_LOCAL_DEVICE_INFO = 0x3,
    DM_GET_DEVICE_INFO = 0x4,
    DM_BEHAVIOR = 0x5,
};

enum class AuthScene : int32_t {
    DM_AUTHCATION = 0x1,
    DM_NETWORK = 0x2,
    DM_DELET_TRUST_RELATION = 0x3,
    DM_PIN_HOLDER = 0x4,
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

enum class PinHolderStage : int32_t {
    CREATE_PIN_HOLDER = 0x1,
    SESSION_OPENED = 0x2,
    SEND_CREATE_PIN_HOLDER_MSG = 0x3,
    RECEIVE_CREATE_PIN_HOLDER_MSG = 0x4,
    DESTROY_PIN_HOLDER = 0x5,
    RECEIVE_DESTROY_PIN_HOLDER_MSG = 0x6,
};

enum class GetTrustDeviceList : int32_t {
    GET_TRUST_DEVICE_LIST = 0x1,
};

enum class TrustStatus : int32_t {
    NOT_TRUST = 0x0,
    IS_TRUST = 0x1,
};

enum class ApiType : int32_t {
    API_UNKNOW = 0x0,
    API_JS = 0x1,
    API_NATIVE = 0x2,
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
    virtual void ReportGetTrustDeviceList(std::string hostName, std::string funcName,
        std::vector<DmDeviceInfo> &deviceInfoList, int32_t errCode, std::string localUdid) = 0;
    virtual void ReportCreatePinHolder(std::string hostName,
        int32_t channelId, std::string peerUdid, int32_t errCode, int32_t stageRes) = 0;
    virtual void ReportDestroyPinHolder(std::string hostName,
        std::string peerUdid, int32_t errCode, int32_t stageRes) = 0;
    virtual void ReportSendOrReceiveHolderMsg(int32_t bizStage, std::string funcName, std::string peerUdid) = 0;
    virtual void ReportDmBehavior(std::string hostName, std::string funcName, int32_t errCode,
        std::string localUdid) = 0;
    virtual void ReportGetLocalDevInfo(std::string hostName,
        std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid) = 0;
    virtual void ReportGetDeviceInfo(std::string hostName,
        std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid) = 0;
    virtual std::string GetDeviceInfoList(std::vector<DmDeviceInfo> &deviceInfoList) = 0;
};

class DmRadarHelper : public IDmRadarHelper {
    DM_DECLARE_SINGLE_INSTANCE(DmRadarHelper);
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
    void ReportGetTrustDeviceList(std::string hostName, std::string funcName,
        std::vector<DmDeviceInfo> &deviceInfoList, int32_t errCode, std::string localUdid) override;
    void ReportCreatePinHolder(std::string hostName,
        int32_t channelId, std::string peerUdid, int32_t errCode, int32_t stageRes) override;
    void ReportDestroyPinHolder(std::string hostName,
        std::string peerUdid, int32_t errCode, int32_t stageRes) override;
    void ReportSendOrReceiveHolderMsg(int32_t bizStage, std::string funcName, std::string peerUdid) override;
    void ReportDmBehavior(std::string hostName, std::string funcName, int32_t errCode,
        std::string localUdid) override;
    void ReportGetLocalDevInfo(std::string hostName,
        std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid) override;
    void ReportGetDeviceInfo(std::string hostName,
        std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid) override;
    std::string GetDeviceInfoList(std::vector<DmDeviceInfo> &deviceInfoList) override;
    std::string ConvertHexToString(uint16_t hex);
    DM_EXPORT int32_t GetErrCode(int32_t errCode);
    DM_EXPORT std::string GetAnonyLocalUdid();
    int32_t ReportDiscoverRegCallbackStageIdle(struct RadarInfo &info);
    int32_t ReportDiscoverUserResStageCancel(struct RadarInfo &info);
    int32_t ReportDiscoverUserResStageSucc(struct RadarInfo &info);
    int32_t ReportDiscoverUserResStageOther(struct RadarInfo &info);
    int32_t ReportDiscoverResCallbackStageSucc(struct RadarInfo &info);
    int32_t ReportAuthOpenSessionStageIdle(struct RadarInfo &info);
    int32_t ReportAuthCreateGroupStageIdle(struct RadarInfo &info);
    int32_t ReportAuthAddGroupStageIdle(struct RadarInfo &info);
    int32_t ReportCreatePinHolderStageSucc(std::string hostName,
        int32_t channelId, std::string peerUdid, int32_t errCode, int32_t stageRes);
    int32_t ReportDestroyPinHolderStageSucc(std::string hostName,
        std::string peerUdid, int32_t errCode, int32_t stageRes);
    int32_t ReportSendOrReceiveHolderMsgStageOther(int32_t bizStage,
        std::string funcName, std::string peerUdid);
    int32_t ReportGetTrustDeviceListResultFailed(std::string hostName,
        std::string funcName, std::vector<DmDeviceInfo> &deviceInfoList,
        int32_t errCode, std::string localUdid, std::string discoverDevList);
    int32_t ReportDmBehaviorResultSucc(std::string hostName, std::string funcName,
        int32_t errCode, std::string localUdid);
    int32_t ReportGetLocalDevInfoResultSucc(std::string hostName,
        std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid);
    int32_t ReportGetDeviceInfoResultSucc(std::string hostName,
        std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid);
private:
    std::string GetAnonyUdid(std::string udid);
    int32_t GetApiType();
};

extern "C" IDmRadarHelper *CreateDmRadarInstance();
using CreateDmRadarFuncPtr = IDmRadarHelper *(*)(void);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_RADAR_HELPER_H