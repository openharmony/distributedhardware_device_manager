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

#ifndef OHOS_DM_SOFTBUS_LISTENER_H
#define OHOS_DM_SOFTBUS_LISTENER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "softbus_bus_center.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "inner_session.h"
#include "session.h"

namespace OHOS {
namespace DistributedHardware {
void DeviceOnLine(DmDeviceInfo deviceInfo);
void DeviceOffLine(DmDeviceInfo deviceInfo);
class SoftbusListener {
public:
    static int OnSessionOpened(int sessionId, int result);
    static void OnSessionClosed(int sessionId);
    static void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen);

public:
    /**
     * @tc.name: SoftbusListener::OnPublishResult
     * @tc.desc: OnPublishResult of the SoftbusListener
     * @tc.type: FUNC
     */
    static void OnPublishResult(int publishId, PublishResult result);

    /**
     * @tc.name: SoftbusListener::OnSoftBusDeviceOnline
     * @tc.desc: OnSoftBus DeviceOnline of the SoftbusListener
     * @tc.type: FUNC
     */
    static void OnSoftBusDeviceOnline(NodeBasicInfo *info);

    /**
     * @tc.name: SoftbusListener::OnSoftbusDeviceOffline
     * @tc.desc: OnSoftbus DeviceOffline of the SoftbusListener
     * @tc.type: FUNC
     */
    static void OnSoftbusDeviceOffline(NodeBasicInfo *info);

    /**
     * @tc.name: SoftbusListener::OnSoftbusDeviceInfoChanged
     * @tc.desc: OnSoftbus DeviceInfoChanged of the SoftbusListener
     * @tc.type: FUNC
     */
    static void OnSoftbusDeviceInfoChanged(NodeBasicInfoType type, NodeBasicInfo *info);

    /**
     * @tc.name: SoftbusListener::OnParameterChgCallback
     * @tc.desc: OnParameter Chg Callback of the SoftbusListener
     * @tc.type: FUNC
     */
    static void OnParameterChgCallback(const char *key, const char *value, void *context);

    /**
     * @tc.name: SoftbusListener::GetUdidByNetworkId
     * @tc.desc: Get Udid By NetworkId of the SoftbusListener
     * @tc.type: FUNC
     */
    static int32_t GetUdidByNetworkId(const char *networkId, std::string &udid);

    /**
     * @tc.name: SoftbusListener::GetUuidByNetworkId
     * @tc.desc: Get Uuid By NetworkId of the SoftbusListener
     * @tc.type: FUNC
     */
    static int32_t GetUuidByNetworkId(const char *networkId, std::string &uuid);

public:
    SoftbusListener();
    ~SoftbusListener();
    int32_t GetTrustedDeviceList(std::vector<DmDeviceInfo> &deviceInfoList);
    int32_t GetLocalDeviceInfo(DmDeviceInfo &deviceInfo);

private:
    int32_t Init();
    void SetPublishInfo(PublishInfo &dmPublishInfo);
    static int32_t ConvertNodeBasicInfoToDmDevice(const NodeBasicInfo &nodeBasicInfo, DmDeviceInfo &dmDeviceInfo);

private:
    enum PulishStatus {
        STATUS_UNKNOWN = 0,
        ALLOW_BE_DISCOVERY = 1,
        NOT_ALLOW_BE_DISCOVERY = 2,
    };
    static PulishStatus publishStatus;
    static INodeStateCb softbusNodeStateCb_;
    static IPublishCb softbusPublishCallback_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SOFTBUS_LISTENER_H
