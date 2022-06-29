/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <string>

#include "dm_anonymous.h"
#include "dm_dfx_constants.h"
#include "dm_device_info.h"
#include "dm_hidumper.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(HiDumpHelper);
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

void HiDumpHelper::SetNodeInfo(const DmDeviceInfo& deviceInfo, const bool deviceStates)
{
    LOGI("HiDumpHelper::SetNodeInfo");
    nodeInfos_.push_back(deviceInfo);
    std::string deviceState = "offline";
    if (deviceStates) {
        deviceState = "online";
    }
    deviceState_.push_back(deviceState);
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
    LOGI("Dump Show All Load Trust List");
    int32_t ret = DM_OK;

    if (nodeInfos_.size() == 0) {
        LOGE("Hidumper get trusted list is empty");
        result.append("Hidumper get trusted list is empty");
    }
    for (unsigned int i = 0; i < nodeInfos_.size(); ++i) {
        result.append("\n{\n    deviceId          : ").append(GetAnonyString(nodeInfos_[i].deviceId).c_str());
        result.append("\n{\n    deviceName        : ").append(nodeInfos_[i].deviceName);
        result.append("\n{\n    networkId         : ").append(GetAnonyString(nodeInfos_[i].networkId).c_str());
        result.append("\n{\n    deviceState         : ").append(deviceState_[i]);
    }

    nodeInfos_.clear();
    LOGI("HiDumpHelper ShowAllLoadTrustedList %s", result.c_str());
    return ret;
}

int32_t HiDumpHelper::ShowHelp(std::string &result)
{
    LOGI("Show hidumper help");
    result.append("DistributedHardwareDeviceManager hidumper options:\n");
    result.append(" -help                    ");
    result.append(": Show help\n");
    result.append(" -getTrustlist            ");
    result.append(": Show all get trusted list:\n\n");
    LOGI("result is %s", result.c_str());
    return DM_OK;
}

int32_t HiDumpHelper::ShowIllealInfomation(std::string &result)
{
    LOGI("ShowIllealInfomation Dump");
    result.clear();
    result.append("Unrecognized option, -h for help.");
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
