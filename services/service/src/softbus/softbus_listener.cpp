/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "softbus_listener.h"

#include <securec.h>
#include <unistd.h>
#if defined(__LITEOS_M__)
#include "dm_mutex.h"
#include "dm_thread.h"
#else
#include <thread>
#include <mutex>
#endif

#include "device_manager_service.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
const int32_t DISCOVER_STATUS_LEN = 20;
const int32_t SOFTBUS_CHECK_INTERVAL = 100000; // 100ms

constexpr const char* DISCOVER_STATUS_KEY = "persist.distributed_hardware.device_manager.discover_status";
constexpr const char* DISCOVER_STATUS_ON = "1";
constexpr const char* DISCOVER_STATUS_OFF = "0";

SoftbusListener::PulishStatus SoftbusListener::publishStatus = SoftbusListener::STATUS_UNKNOWN;
IPublishCb SoftbusListener::softbusPublishCallback_ = {
    .OnPublishResult = SoftbusListener::OnPublishResult,
};

INodeStateCb SoftbusListener::softbusNodeStateCb_ = {
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE | EVENT_NODE_STATE_INFO_CHANGED,
    .onNodeOnline = SoftbusListener::OnSoftBusDeviceOnline,
    .onNodeOffline = SoftbusListener::OnSoftbusDeviceOffline,
    .onNodeBasicInfoChanged = SoftbusListener::OnSoftbusDeviceInfoChanged};

void DeviceOnLine(DmDeviceInfo deviceInfo)
{
#if defined(__LITEOS_M__)
    DmMutex lockDeviceOnLine;
#else
    std::mutex lockDeviceOnLine;
    std::lock_guard<std::mutex> lock(lockDeviceOnLine);
#endif
    DeviceManagerService::GetInstance().HandleDeviceOnline(deviceInfo);
}

void DeviceOffLine(DmDeviceInfo deviceInfo)
{
#if defined(__LITEOS_M__)
    DmMutex lockDeviceOffLine;
#else
    std::mutex lockDeviceOffLine;
    std::lock_guard<std::mutex> lock(lockDeviceOffLine);
#endif
    DeviceManagerService::GetInstance().HandleDeviceOffline(deviceInfo);
}

SoftbusListener::SoftbusListener()
{
    ISessionListener sessionListener = {.OnSessionOpened = SoftbusListener::OnSessionOpened,
                                        .OnSessionClosed = SoftbusListener::OnSessionClosed,
                                        .OnBytesReceived = SoftbusListener::OnBytesReceived,
                                        .OnMessageReceived = nullptr,
                                        .OnStreamReceived = nullptr};
    int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_SESSION_NAME, &sessionListener);
    if (ret != DM_OK) {
        LOGE("CreateSessionServer failed");
    } else {
        LOGI("CreateSessionServer ok");
    }
    Init();
}

SoftbusListener::~SoftbusListener()
{
    RemoveSessionServer(DM_PKG_NAME, DM_SESSION_NAME);
    LOGI("SoftbusListener destructor");
}

void SoftbusListener::SetPublishInfo(PublishInfo &dmPublishInfo)
{
    dmPublishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    dmPublishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
    dmPublishInfo.medium = ExchangeMedium::AUTO;
    dmPublishInfo.freq = ExchangeFreq::HIGH;
    dmPublishInfo.capability = DM_CAPABILITY_OSD;
    dmPublishInfo.ranging = false;
    return;
}

int32_t SoftbusListener::Init()
{
    int32_t ret;
    int32_t retryTimes = 0;
    do {
        ret = RegNodeDeviceStateCb(DM_PKG_NAME, &softbusNodeStateCb_);
        if (ret != DM_OK) {
            ++retryTimes;
            LOGE("RegNodeDeviceStateCb failed with ret %d, retryTimes %d", ret, retryTimes);
            usleep(SOFTBUS_CHECK_INTERVAL);
        }
    } while (ret != DM_OK);

    PublishInfo dmPublishInfo;
    (void)memset_s(&dmPublishInfo, sizeof(PublishInfo), 0, sizeof(PublishInfo));
    SetPublishInfo(dmPublishInfo);
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ret = PublishLNN(DM_PKG_NAME, &dmPublishInfo, &softbusPublishCallback_);
    if (ret == DM_OK) {
        publishStatus = ALLOW_BE_DISCOVERY;
    }
#else
    char discoverStatus[DISCOVER_STATUS_LEN + 1] = {0};
    ret = GetParameter(DISCOVER_STATUS_KEY, "not exist", discoverStatus, DISCOVER_STATUS_LEN);
    if (strcmp(discoverStatus, "not exist") == 0) {
        ret = SetParameter(DISCOVER_STATUS_KEY, DISCOVER_STATUS_ON);
        LOGI("service set parameter result is : %d", ret);

        ret = PublishLNN(DM_PKG_NAME, &dmPublishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            publishStatus = ALLOW_BE_DISCOVERY;
        }
        LOGI("service publish result is : %d", ret);
    } else if (ret >= 0 && strcmp(discoverStatus, DISCOVER_STATUS_ON) == 0) {
        ret = PublishLNN(DM_PKG_NAME, &dmPublishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            publishStatus = ALLOW_BE_DISCOVERY;
        }
        LOGI("service publish result is : %d", ret);
    } else if (ret >= 0 && strcmp(discoverStatus, DISCOVER_STATUS_OFF) == 0) {
        ret = StopPublishLNN(DM_PKG_NAME, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (ret == DM_OK) {
            publishStatus = NOT_ALLOW_BE_DISCOVERY;
        }
        LOGI("service unpublish result is : %d", ret);
    }

    ret = WatchParameter(DISCOVER_STATUS_KEY, &SoftbusListener::OnParameterChgCallback, nullptr);
#endif
    return ret;
}

int32_t SoftbusListener::GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList)
{
    int32_t infoNum = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, &infoNum);
    if (ret != 0) {
        LOGE("GetAllNodeDeviceInfo failed with ret %d", ret);
        return ERR_DM_FAILED;
    }
    DmDeviceInfo *info = static_cast<DmDeviceInfo *>(malloc(sizeof(DmDeviceInfo) * (infoNum)));
    if (info == nullptr) {
        FreeNodeInfo(nodeInfo);
        return ERR_DM_MALLOC_FAILED;
    }
    DmDeviceInfo **pInfoList = &info;
    for (int32_t i = 0; i < infoNum; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        DmDeviceInfo *deviceInfo = *pInfoList + i;
        ConvertNodeBasicInfoToDmDevice(*nodeBasicInfo, *deviceInfo);
        deviceInfoList.push_back(*deviceInfo);
    }
    FreeNodeInfo(nodeInfo);
    free(info);
    LOGI("SoftbusListener::GetTrustDevices success, deviceCount %d", infoNum);
    return DM_OK;
}

int32_t SoftbusListener::GetLocalDeviceInfo(DmDeviceInfo &deviceInfo)
{
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &nodeBasicInfo);
    if (ret != 0) {
        LOGE("GetLocalNodeDeviceInfo failed with ret %d", ret);
        return ERR_DM_FAILED;
    }
    ConvertNodeBasicInfoToDmDevice(nodeBasicInfo, deviceInfo);
    return DM_OK;
}

int32_t SoftbusListener::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    uint8_t mUdid[UDID_BUF_LEN] = {0};
    int32_t ret =
        GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid));
    if (ret != DM_OK) {
        LOGE("GetUdidByNetworkId GetNodeKeyInfo failed");
        return ERR_DM_FAILED;
    }
    udid = reinterpret_cast<char *>(mUdid);
    return DM_OK;
}

int32_t SoftbusListener::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    uint8_t mUuid[UUID_BUF_LEN] = {0};
    int32_t ret =
        GetNodeKeyInfo(DM_PKG_NAME, networkId, NodeDeviceInfoKey::NODE_KEY_UUID, mUuid, sizeof(mUuid));
    if (ret != DM_OK) {
        LOGE("GetUuidByNetworkId GetNodeKeyInfo failed");
        return ERR_DM_FAILED;
    }
    uuid = reinterpret_cast<char *>(mUuid);
    return DM_OK;
}

void SoftbusListener::OnSoftBusDeviceOnline(NodeBasicInfo *info)
{
    LOGI("OnSoftBusDeviceOnline: received device online callback from softbus.");
    if (info == nullptr) {
        LOGE("SoftbusListener::OnSoftbusDeviceOffline NodeBasicInfo is nullptr");
        return;
    }
    DmDeviceInfo dmDeviceInfo;
    ConvertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
#if defined(__LITEOS_M__)
    DmThread deviceOnLine(DeviceOnLine, dmDeviceInfo);
    deviceOnLine.DmCreatThread();
#else
    std::thread deviceOnLine(DeviceOnLine, dmDeviceInfo);
    deviceOnLine.detach();
#endif
}

void SoftbusListener::OnSoftbusDeviceOffline(NodeBasicInfo *info)
{
    LOGI("OnSoftBusDeviceOnline: received device offline callback from softbus.");
    if (info == nullptr) {
        LOGE("OnSoftbusDeviceOffline NodeBasicInfo is nullptr");
        return;
    }
    DmDeviceInfo dmDeviceInfo;
    ConvertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
#if defined(__LITEOS_M__)
    DmThread deviceOffLine(DeviceOffLine, dmDeviceInfo);
    deviceOffLine.DmCreatThread();
#else
    std::thread deviceOffLine(DeviceOffLine, dmDeviceInfo);
    deviceOffLine.detach();
#endif
}

int32_t SoftbusListener::ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeBasicInfo, DmDeviceInfo &dmDeviceInfo)
{
    (void)memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (memcpy_s(dmDeviceInfo.deviceId, sizeof(dmDeviceInfo.deviceId), nodeBasicInfo.networkId,
                 std::min(sizeof(dmDeviceInfo.deviceId), sizeof(nodeBasicInfo.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceId data failed");
    }

    if (memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), nodeBasicInfo.networkId,
                 std::min(sizeof(dmDeviceInfo.networkId), sizeof(nodeBasicInfo.networkId))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy networkId data failed");
    }

    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), nodeBasicInfo.deviceName,
                 std::min(sizeof(dmDeviceInfo.deviceName), sizeof(nodeBasicInfo.deviceName))) != DM_OK) {
        LOGE("ConvertNodeBasicInfoToDmDevice copy deviceName data failed");
    }
    dmDeviceInfo.deviceTypeId = nodeBasicInfo.deviceTypeId;
    return DM_OK;
}

void SoftbusListener::OnParameterChgCallback(const char *key, const char *value, void *context)
{
    if (strcmp(value, DISCOVER_STATUS_ON) == 0 && publishStatus != ALLOW_BE_DISCOVERY) {
        PublishInfo dmPublishInfo;
        (void)memset_s(&dmPublishInfo, sizeof(PublishInfo), 0, sizeof(PublishInfo));
        dmPublishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
        dmPublishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
        dmPublishInfo.medium = ExchangeMedium::AUTO;
        dmPublishInfo.freq = ExchangeFreq::HIGH;
        dmPublishInfo.capability = DM_CAPABILITY_OSD;
        dmPublishInfo.ranging = false;
        int32_t ret = ::PublishLNN(DM_PKG_NAME, &dmPublishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            publishStatus = ALLOW_BE_DISCOVERY;
        }
        LOGI("service publish result is : %d", ret);
    } else if (strcmp(value, DISCOVER_STATUS_OFF) == 0 && publishStatus != NOT_ALLOW_BE_DISCOVERY) {
        int32_t ret = ::StopPublishLNN(DM_PKG_NAME, DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (ret == DM_OK) {
            publishStatus = NOT_ALLOW_BE_DISCOVERY;
        }
        LOGI("service unpublish result is : %d", ret);
    }
}

int SoftbusListener::OnSessionOpened(int sessionId, int result)
{
    return DeviceManagerService::GetInstance().OnSessionOpened(sessionId, result);
}

void SoftbusListener::OnSessionClosed(int sessionId)
{
    DeviceManagerService::GetInstance().OnSessionClosed(sessionId);
}

void SoftbusListener::OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    DeviceManagerService::GetInstance().OnBytesReceived(sessionId, data, dataLen);
}

void SoftbusListener::OnPublishResult(int publishId, PublishResult result)
{
    LOGD("SoftbusListener::OnPublishResult, publishId: %d, result: %d", publishId, result);
}

void SoftbusListener::OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info)
{
    LOGD("SoftbusListener::OnSoftbusDeviceInfoChanged.");
}
} // namespace DistributedHardware
} // namespace OHOS