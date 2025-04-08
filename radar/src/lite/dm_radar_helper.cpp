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
#include <sstream>
#include <iomanip>
#include "dm_constants.h"
#include "dm_log.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t INVALID_UDID_LENGTH = 10;
constexpr int32_t SUBSTR_UDID_LENGTH = 5;
const char* ORGPKGNAME = "deviceManager";
const char* SOFTBUSNAME = "dsoftbus";
const char* HICHAINNAME = "hichain";

DM_IMPLEMENT_SINGLE_INSTANCE(DmRadarHelper);
bool DmRadarHelper::ReportDiscoverRegCallback(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportDiscoverResCallback(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportDiscoverUserRes(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthStart(const std::string &peerUdid, const std::string &pkgName)
{
    return true;
}

bool DmRadarHelper::ReportAuthOpenSession(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthSessionOpenCb(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthSendRequest(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthPullAuthBox(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthConfirmBox(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthCreateGroup(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthCreateGroupCb(std::string funcName, int32_t stageRes)
{
    return true;
}

bool DmRadarHelper::ReportAuthPullPinBox(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthInputPinBox(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthAddGroup(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportAuthAddGroupCb(std::string funcName, int32_t stageRes)
{
    return true;
}

bool DmRadarHelper::ReportNetworkOnline(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportNetworkOffline(struct RadarInfo &info)
{
    return true;
}

bool DmRadarHelper::ReportDeleteTrustRelation(struct RadarInfo &info)
{
    return true;
}

void DmRadarHelper::ReportGetTrustDeviceList(std::string hostName,
    std::string funcName, std::vector<DmDeviceInfo> &deviceInfoList, int32_t errCode, std::string localUdid)
{
    return;
}

void DmRadarHelper::ReportCreatePinHolder(std::string hostName,
    int32_t channelId, std::string peerUdid, int32_t errCode, int32_t stageRes)
{
    return;
}

void DmRadarHelper::ReportDestroyPinHolder(std::string hostName,
    std::string peerUdid, int32_t errCode, int32_t stageRes)
{
    return;
}

void DmRadarHelper::ReportSendOrReceiveHolderMsg(int32_t bizStage, std::string funcName, std::string peerUdid)
{
    return;
}

void DmRadarHelper::ReportDmBehavior(std::string hostName, std::string funcName,
    int32_t errCode, std::string localUdid)
{
    return;
}

void DmRadarHelper::ReportGetLocalDevInfo(std::string hostName,
    std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid)
{
    return;
}

void DmRadarHelper::ReportGetDeviceInfo(std::string hostName,
    std::string funcName, DmDeviceInfo &info, int32_t errCode, std::string localUdid)
{
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
    return "";
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
