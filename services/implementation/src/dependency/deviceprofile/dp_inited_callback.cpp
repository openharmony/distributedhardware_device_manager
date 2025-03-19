/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dp_inited_callback.h"

#include <pthread.h>
#include <thread>
#include <vector>
#include <unordered_map>

#include "json_object.h"
#include "parameter.h"
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
#include "ffrt.h"
#endif

#include "deviceprofile_connector.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_softbus_cache.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string PUT_ALL_TRUSTED_DEVICES_TASK = "PutAllTrustedDevicesTask";
}

DpInitedCallback::DpInitedCallback()
{}

DpInitedCallback::~DpInitedCallback()
{}

int32_t DpInitedCallback::OnDpInited()
{
    LOGE("In.");
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ffrt::submit([=]() { PutAllTrustedDevices(); });
#else
    std::thread putAllTrustedDevicesTask([=]() { PutAllTrustedDevices(); });
    if (pthread_setname_np(putAllTrustedDevicesTask.native_handle().c_str(), PUT_ALL_TRUSTED_DEVICES_TASK) != DM_OK) {
        LOGE("putAllTrustedDevicesTask setname failed.");
    }
    putAllTrustedDevicesTask.detach();
#endif
    return DM_OK;
}

void DpInitedCallback::PutAllTrustedDevices()
{
    LOGE("In.");
    std::vector<DmDeviceInfo> dmDeviceInfos;
    int32_t ret = SoftbusCache::GetInstance().GetDeviceInfoFromCache(dmDeviceInfos);
    if (ret != DM_OK) {
        LOGE("GetDeviceInfoFromCache fail:%{public}d", ret);
        return;
    }
    if (dmDeviceInfos.empty()) {
        LOGW("dmDeviceInfos is empty");
        return;
    }
    char localUdidTemp[DEVICE_UUID_LENGTH];
    GetDevUdid(localUdidTemp, DEVICE_UUID_LENGTH);
    std::string localUdid = reinterpret_cast<char *>(localUdidTemp);
    if (localUdid.empty()) {
        LOGE("localUdid is empty");
        return;
    }
    std::vector<DistributedDeviceProfile::TrustedDeviceInfo> deviceInfos;
    std::unordered_map<std::string, DmAuthForm> authFormMap =
        DeviceProfileConnector::GetInstance().GetAppTrustDeviceList("", localUdid);
    for (const auto& item : dmDeviceInfos) {
        DistributedDeviceProfile::TrustedDeviceInfo trustedDeviceInfo;
        if (ConvertToTrustedDeviceInfo(authFormMap, item, trustedDeviceInfo)) {
            deviceInfos.push_back(trustedDeviceInfo);
        }
    }
    if (deviceInfos.empty()) {
        LOGW("deviceInfos is empty");
        return;
    }
    ret = DeviceProfileConnector::GetInstance().PutAllTrustedDevices(deviceInfos);
    LOGI("ret:%{public}d", ret);
}

bool DpInitedCallback::ConvertToTrustedDeviceInfo(const std::unordered_map<std::string, DmAuthForm> &authFormMap,
    const DmDeviceInfo &deviceInfo, DistributedDeviceProfile::TrustedDeviceInfo &trustedDeviceInfo)
{
    trustedDeviceInfo.SetNetworkId(deviceInfo.networkId);
    trustedDeviceInfo.SetDeviceTypeId(deviceInfo.deviceTypeId);

    if (deviceInfo.extraData.empty()) {
        LOGE("extraData is empty, networkId:%{public}s", GetAnonyString(deviceInfo.networkId).c_str());
        return false;
    }
    JsonObject extraJson(deviceInfo.extraData);
    if (extraJson.IsDiscarded()) {
        LOGE("extraData parse failed, networkId:%{public}s", GetAnonyString(deviceInfo.networkId).c_str());
        return false;
    }
    if (IsString(extraJson, PARAM_KEY_OS_VERSION)) {
        trustedDeviceInfo.SetOsVersion(extraJson[PARAM_KEY_OS_VERSION].Get<std::string>());
    } else {
        LOGE("osVersion parse failed, networkId:%{public}s", GetAnonyString(deviceInfo.networkId).c_str());
        return false;
    }
    if (IsInt32(extraJson, PARAM_KEY_OS_TYPE)) {
        trustedDeviceInfo.SetOsType(extraJson[PARAM_KEY_OS_TYPE].Get<int32_t>());
    } else {
        LOGE("osType parse failed, networkId:%{public}s", GetAnonyString(deviceInfo.networkId).c_str());
        return false;
    }

    std::string udid = "";
    if (SoftbusCache::GetInstance().GetUdidFromCache(deviceInfo.networkId, udid) != DM_OK) {
        LOGE("udid parse failed, networkId:%{public}s", GetAnonyString(deviceInfo.networkId).c_str());
        return false;
    }
    trustedDeviceInfo.SetUdid(udid);

    std::string uuid = "";
    if (SoftbusCache::GetInstance().GetUuidFromCache(deviceInfo.networkId, uuid) != DM_OK) {
        LOGE("uuid parse failed, networkId:%{public}s", GetAnonyString(deviceInfo.networkId).c_str());
        return false;
    }
    trustedDeviceInfo.SetUuid(uuid);

    auto it = authFormMap.find(udid);
    if (it == authFormMap.end()) {
        LOGE("authForm not exist, udid:%{public}s", GetAnonyString(udid).c_str());
        return false;
    }
    trustedDeviceInfo.SetAuthForm(static_cast<int32_t>(it->second));
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS