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
#include <errors.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cJSON.h>
#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_log.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DmRadarHelper);
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

bool DmRadarHelper::ReportGetTrustDeviceList(struct RadarInfo &info)
{
    return true;
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
    cJSON* deviceInfoJson = cJSON_CreateArray();
    for (size_t i = 0; i < deviceInfoList.size(); i++) {
        cJSON* object = cJSON_CreateObject();
        std::string udidHash = GetUdidHashByUdid(std::string(deviceInfoList[i].deviceId));
        cJSON_AddStringToObject(object, "PEER_UDID", udidHash.c_str());
        cJSON_AddStringToObject(object, "PEER_NET_ID", deviceInfoList[i].networkId);
        std::string devType = ConvertHexToString(deviceInfoList[i].deviceTypeId);
        cJSON_AddStringToObject(object, "PEER_DEV_TYPE", devType.c_str());
        cJSON_AddStringToObject(object, "PEER_DEV_NAME", deviceInfoList[i].deviceName);
        cJSON_AddItemToArray(deviceInfoJson, object);
    }
    return std::string(cJSON_PrintUnformatted(deviceInfoJson));
}

std::string DmRadarHelper::GetUdidHashByUdid(std::string udid)
{
    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (Crypto::GetUdidHash(udid, reinterpret_cast<uint8_t *>(udidHash)) != DM_OK) {
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

IDmRadarHelper *CreateDmRadarInstance()
{
    return &DmRadarHelper::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS
