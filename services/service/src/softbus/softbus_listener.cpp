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
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedHardware {
SoftbusListener::PulishStatus SoftbusListener::publishStatus = SoftbusListener::STATUS_UNKNOWN;
IPublishCallback SoftbusListener::softbusPublishCallback_ = {.OnPublishSuccess = SoftbusListener::OnPublishSuccess,
                                                             .OnPublishFail = SoftbusListener::OnPublishFail};

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
    int32_t ret = CreateSessionServer(DM_PKG_NAME.c_str(), DM_SESSION_NAME.c_str(), &sessionListener);
    if (ret != DM_OK) {
        LOGE("CreateSessionServer failed");
    } else {
        LOGI("CreateSessionServer ok");
    }
    Init();
}

SoftbusListener::~SoftbusListener()
{
    RemoveSessionServer(DM_PKG_NAME.c_str(), DM_SESSION_NAME.c_str());
    LOGI("SoftbusListener destructor");
}

int32_t SoftbusListener::Init()
{
    int32_t ret;
    int32_t retryTimes = 0;
    do {
        ret = RegNodeDeviceStateCb(DM_PKG_NAME.c_str(), &softbusNodeStateCb_);
        if (ret != DM_OK) {
            ++retryTimes;
            LOGE("RegNodeDeviceStateCb failed with ret %d, retryTimes %d", ret, retryTimes);
            usleep(SOFTBUS_CHECK_INTERVAL);
        }
    } while (ret != DM_OK);

    PublishInfo dmPublishInfo;
    dmPublishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
    dmPublishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
    dmPublishInfo.medium = ExchangeMedium::AUTO;
    dmPublishInfo.freq = ExchangeFreq::HIGH;
    dmPublishInfo.capability = DM_CAPABILITY_OSD;
    dmPublishInfo.capabilityData = nullptr;
    dmPublishInfo.dataLen = 0;
#if (defined(__LITEOS_M__) || defined(LITE_DEVICE))
    ret = PublishService(DM_PKG_NAME.c_str(), &dmPublishInfo, &softbusPublishCallback_);
    if (ret == DM_OK) {
        publishStatus = ALLOW_BE_DISCOVERY;
    }
#else
    char discoverStatus[DISCOVER_STATUS_LEN + 1] = {0};
    ret = GetParameter(DISCOVER_STATUS_KEY.c_str(), "not exist", discoverStatus, DISCOVER_STATUS_LEN);
    if (strcmp(discoverStatus, "not exist") == 0) {
        ret = SetParameter(DISCOVER_STATUS_KEY.c_str(), DISCOVER_STATUS_ON.c_str());
        LOGI("service set parameter result is : %d", ret);

        ret = PublishService(DM_PKG_NAME.c_str(), &dmPublishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            publishStatus = ALLOW_BE_DISCOVERY;
        }
        LOGI("service publish result is : %d", ret);
    } else if (ret >= 0 && strcmp(discoverStatus, DISCOVER_STATUS_ON.c_str()) == 0) {
        ret = PublishService(DM_PKG_NAME.c_str(), &dmPublishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            publishStatus = ALLOW_BE_DISCOVERY;
        }
        LOGI("service publish result is : %d", ret);
    } else if (ret >= 0 && strcmp(discoverStatus, DISCOVER_STATUS_OFF.c_str()) == 0) {
        ret = UnPublishService(DM_PKG_NAME.c_str(), DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        if (ret == DM_OK) {
            publishStatus = NOT_ALLOW_BE_DISCOVERY;
        }
        LOGI("service unpublish result is : %d", ret);
    }

    ret = WatchParameter(DISCOVER_STATUS_KEY.c_str(), &SoftbusListener::OnParameterChgCallback, nullptr);
#endif
    return ret;
}

int32_t SoftbusListener::GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList)
{
    LOGI("SoftbusListener::GetTrustDevices start");
    int32_t infoNum = 0;
    NodeBasicInfo *nodeInfo = nullptr;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME.c_str(), &nodeInfo, &infoNum);
    if (ret != 0) {
        LOGE("GetAllNodeDeviceInfo failed with ret %d", ret);
        return ERR_DM_FAILED;
    }
    DmDeviceInfo *info = (DmDeviceInfo *)malloc(sizeof(DmDeviceInfo) * (infoNum));
    if (info == nullptr) {
        FreeNodeInfo(nodeInfo);
        return ERR_DM_MALLOC_FAILED;
    }
    DmDeviceInfo **pInfoList = &info;
    for (int32_t i = 0; i < infoNum; ++i) {
        NodeBasicInfo *nodeBasicInfo = nodeInfo + i;
        DmDeviceInfo *deviceInfo = *pInfoList + i;
        CovertNodeBasicInfoToDmDevice(*nodeBasicInfo, *deviceInfo);
        deviceInfoList.push_back(*deviceInfo);
    }
    FreeNodeInfo(nodeInfo);
    free(info);
    LOGI("SoftbusListener::GetTrustDevices success, deviceCount %d", infoNum);
    return DM_OK;
}

int32_t SoftbusListener::GetLocalDeviceInfo(DmDeviceInfo &deviceInfo)
{
    LOGI("SoftbusListener::GetLocalDeviceInfo start");
    NodeBasicInfo nodeBasicInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME.c_str(), &nodeBasicInfo);
    if (ret != 0) {
        LOGE("GetLocalNodeDeviceInfo failed with ret %d", ret);
        return ERR_DM_FAILED;
    }
    CovertNodeBasicInfoToDmDevice(nodeBasicInfo, deviceInfo);
    LOGI("SoftbusListener::GetLocalDeviceInfo success");
    return DM_OK;
}

int32_t SoftbusListener::GetUdidByNetworkId(const char *networkId, std::string &udid)
{
    LOGI("GetUdidByNetworkId begin");
    uint8_t mUdid[UDID_BUF_LEN] = {0};
    int32_t ret =
        GetNodeKeyInfo(DM_PKG_NAME.c_str(), networkId, NodeDeviceInfoKey::NODE_KEY_UDID, mUdid, sizeof(mUdid));
    if (ret != DM_OK) {
        LOGE("GetUdidByNetworkId GetNodeKeyInfo failed");
        return ERR_DM_FAILED;
    }
    udid = (char *)mUdid;
    LOGI("SoftbusListener::GetUdidByNetworkId completed");
    return DM_OK;
}

int32_t SoftbusListener::GetUuidByNetworkId(const char *networkId, std::string &uuid)
{
    LOGI("GetUuidByNetworkId begin");
    uint8_t mUuid[UUID_BUF_LEN] = {0};
    int32_t ret =
        GetNodeKeyInfo(DM_PKG_NAME.c_str(), networkId, NodeDeviceInfoKey::NODE_KEY_UUID, mUuid, sizeof(mUuid));
    if (ret != DM_OK) {
        LOGE("GetUuidByNetworkId GetNodeKeyInfo failed");
        return ERR_DM_FAILED;
    }
    uuid = (char *)mUuid;
    LOGI("SoftbusListener::GetUuidByNetworkId completed");
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
    CovertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
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
    CovertNodeBasicInfoToDmDevice(*info, dmDeviceInfo);
#if defined(__LITEOS_M__)
    DmThread deviceOffLine(DeviceOffLine, dmDeviceInfo);
    deviceOffLine.DmCreatThread();
#else
    std::thread deviceOffLine(DeviceOffLine, dmDeviceInfo);
    deviceOffLine.detach();
#endif
}

int32_t SoftbusListener::CovertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeBasicInfo, DmDeviceInfo &dmDeviceInfo)
{
    (void)memset_s(&dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (memcpy_s(dmDeviceInfo.deviceId, sizeof(dmDeviceInfo.deviceId), nodeBasicInfo.networkId,
                 std::min(sizeof(dmDeviceInfo.deviceId), sizeof(nodeBasicInfo.networkId))) != DM_OK) {
        LOGE("CovertNodeBasicInfoToDmDevice copy deviceId data failed");
    }

    if (memcpy_s(dmDeviceInfo.networkId, sizeof(dmDeviceInfo.networkId), nodeBasicInfo.networkId,
                 std::min(sizeof(dmDeviceInfo.networkId), sizeof(nodeBasicInfo.networkId))) != DM_OK) {
        LOGE("CovertNodeBasicInfoToDmDevice copy networkId data failed");
    }

    if (memcpy_s(dmDeviceInfo.deviceName, sizeof(dmDeviceInfo.deviceName), nodeBasicInfo.deviceName,
                 std::min(sizeof(dmDeviceInfo.deviceName), sizeof(nodeBasicInfo.deviceName))) != DM_OK) {
        LOGE("CovertNodeBasicInfoToDmDevice copy deviceName data failed");
    }
    dmDeviceInfo.deviceTypeId = nodeBasicInfo.deviceTypeId;
    return DM_OK;
}

void SoftbusListener::OnParameterChgCallback(const char *key, const char *value, void *context)
{
    if (strcmp(value, DISCOVER_STATUS_ON.c_str()) == 0 && publishStatus != ALLOW_BE_DISCOVERY) {
        PublishInfo dmPublishInfo;
        dmPublishInfo.publishId = DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID;
        dmPublishInfo.mode = DiscoverMode::DISCOVER_MODE_ACTIVE;
        dmPublishInfo.medium = ExchangeMedium::AUTO;
        dmPublishInfo.freq = ExchangeFreq::HIGH;
        dmPublishInfo.capability = DM_CAPABILITY_OSD;
        dmPublishInfo.capabilityData = nullptr;
        dmPublishInfo.dataLen = 0;
        int32_t ret = PublishService(DM_PKG_NAME.c_str(), &dmPublishInfo, &softbusPublishCallback_);
        if (ret == DM_OK) {
            publishStatus = ALLOW_BE_DISCOVERY;
        }
        LOGI("service publish result is : %d", ret);
    } else if (strcmp(value, DISCOVER_STATUS_OFF.c_str()) == 0 && publishStatus != NOT_ALLOW_BE_DISCOVERY) {
        int32_t ret = UnPublishService(DM_PKG_NAME.c_str(), DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
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

void SoftbusListener::OnPublishSuccess(int publishId)
{
    LOGI("SoftbusListener::OnPublishSuccess, publishId: %d", publishId);
}

void SoftbusListener::OnPublishFail(int publishId, PublishFailReason reason)
{
    LOGI("SoftbusListener::OnPublishFail failed, publishId: %d, reason: %d", publishId, reason);
}

void SoftbusListener::OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info)
{
    LOGI("SoftbusListener::OnSoftbusDeviceInfoChanged.");
}
} // namespace DistributedHardware
} // namespace OHOS