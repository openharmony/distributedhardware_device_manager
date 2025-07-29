/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "dm_hidumper.h"

#include <unordered_map>       // for __hash_map_const_iterator, unordered_map
#include <utility>             // for pair
#include "dm_constants.h"
#include "dm_anonymous.h"      // for GetAnonyString
#include "dm_error_type.h"
#include "dm_log.h"            // for LOGI, LOGE

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(HiDumpHelper);
namespace {
static DumperInfo g_dumperDeviceType[] = {
    {DEVICE_TYPE_UNKNOWN, "DEVICE_TYPE_UNKNOWN"},
    {DEVICE_TYPE_WIFI_CAMERA, "DEVICE_TYPE_WIFI_CAMERA"},
    {DEVICE_TYPE_AUDIO, "DEVICE_TYPE_AUDIO"},
    {DEVICE_TYPE_PC, "DEVICE_TYPE_PC"},
    {DEVICE_TYPE_PHONE, "DEVICE_TYPE_PHONE"},
    {DEVICE_TYPE_PAD, "DEVICE_TYPE_PAD"},
    {DEVICE_TYPE_WATCH, "DEVICE_TYPE_WATCH"},
    {DEVICE_TYPE_CAR, "DEVICE_TYPE_CAR"},
    {DEVICE_TYPE_TV, "DEVICE_TYPE_TV"},
};

// HiDumper info
constexpr const char* ARGS_HELP_INFO = "-help";
constexpr const char* HIDUMPER_GET_TRUSTED_LIST_INFO = "-getTrustlist";

// HiDumper command
const std::unordered_map<std::string, HidumperFlag> MAP_ARGS = {
    { std::string(ARGS_HELP_INFO), HidumperFlag::HIDUMPER_GET_HELP },
    { std::string(HIDUMPER_GET_TRUSTED_LIST_INFO), HidumperFlag::HIDUMPER_GET_TRUSTED_LIST },
};

} // namespace
int32_t HiDumpHelper::HiDump(const std::vector<std::string>& args, std::string &result)
{
    LOGI("HiDumpHelper start.");
    result.clear();
    int32_t errCode = ERR_DM_FAILED;

    if (args.empty()) {
        return ProcessDump(HidumperFlag::HIDUMPER_GET_HELP, result);
    }
    auto flag = MAP_ARGS.find(args[0]);
    if ((args.size() > 1) || (flag == MAP_ARGS.end())) {
        errCode = ProcessDump(HidumperFlag::HIDUMPER_UNKNOWN, result);
    } else {
        errCode = ProcessDump(flag->second, result);
    }
    return errCode;
}

void HiDumpHelper::SetNodeInfo(const DmDeviceInfo& deviceInfo)
{
    LOGI("HiDumpHelper::SetNodeInfo");
    if (nodeInfos_.size() >= MAX_CONTAINER_SIZE) {
        LOGE("nodeInfos_ size is more than max size");
        return;
    }
    nodeInfos_.push_back(deviceInfo);
}

int32_t HiDumpHelper::ProcessDump(const HidumperFlag &flag, std::string &result)
{
    LOGI("Process Dump.");
    int32_t ret = ERR_DM_FAILED;
    switch (flag) {
        case HidumperFlag::HIDUMPER_GET_HELP: {
            ret = ShowHelp(result);
            break;
        }
        case HidumperFlag::HIDUMPER_GET_TRUSTED_LIST: {
            ret = ShowAllLoadTrustedList(result);
            break;
        }
        default: {
            ret = ShowIllealInfomation(result);
            break;
        }
    }
    return ret;
}

int32_t HiDumpHelper::ShowAllLoadTrustedList(std::string &result)
{
    LOGI("dump all trusted device List");
    int32_t ret = DM_OK;

    if (nodeInfos_.size() == 0) {
        LOGE("dump trusted device list is empty");
        result.append("dump trusted device list is empty");
    }
    for (unsigned int i = 0; i < nodeInfos_.size(); ++i) {
        result.append("\n{\n    deviceId          : ").append(GetAnonyString(nodeInfos_[i].deviceId).c_str());
        result.append("\n{\n    deviceName        : ").append(GetAnonyString(nodeInfos_[i].deviceName).c_str());
        result.append("\n{\n    networkId         : ").append(GetAnonyString(nodeInfos_[i].networkId).c_str());
        std::string deviceType = GetDeviceType(nodeInfos_[i].deviceTypeId);
        result.append("\n{\n    deviceType        : ").append(deviceType);
    }

    nodeInfos_.clear();
    LOGI("HiDumpHelper ShowAllLoadTrustedList %{public}s", result.c_str());
    return ret;
}

std::string HiDumpHelper::GetDeviceType(int32_t deviceTypeId)
{
    std::string dmDeviceTypeIdString = "";
    for (uint32_t i = 0; i < (sizeof(g_dumperDeviceType) / sizeof(g_dumperDeviceType[0])); i++) {
        if (deviceTypeId == g_dumperDeviceType[i].deviceTypeId) {
            dmDeviceTypeIdString = g_dumperDeviceType[i].deviceTypeInfo;
            break;
        }
    }
    return dmDeviceTypeIdString;
}

int32_t HiDumpHelper::ShowHelp(std::string &result)
{
    LOGI("Show hidumper help");
    result.append("DistributedHardwareDeviceManager hidumper options:\n");
    result.append(" -help                    ");
    result.append(": show help\n");
    result.append(" -getTrustlist            ");
    result.append(": show all trusted device list\n\n");
    return DM_OK;
}

int32_t HiDumpHelper::ShowIllealInfomation(std::string &result)
{
    LOGI("ShowIllealInfomation Dump");
    result.clear();
    result.append("unrecognized option, -help for help.");
    return DM_OK;
}

int32_t HiDumpHelper::GetArgsType(const std::vector<std::string>& args, std::vector<HidumperFlag> &Flag)
{
    LOGI("HiDumpHelper::GetArgsType");
    int32_t ret = ERR_DM_FAILED;
    if (args.empty()) {
        Flag.push_back(HidumperFlag::HIDUMPER_GET_HELP);
        return ret;
    }

    auto flag = MAP_ARGS.find(args[0]);
    if (flag != MAP_ARGS.end()) {
        Flag.push_back(flag->second);
    }
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
