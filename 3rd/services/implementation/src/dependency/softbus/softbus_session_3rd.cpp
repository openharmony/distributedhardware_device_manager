/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "softbus_session_3rd.h"

#include <chrono>
#include <random>
#include <algorithm>
#include <functional>

#include "json_object.h"

#include "dm_anonymous_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_error_type_3rd.h"
#include "dm_log_3rd.h"
#include "softbus_connector_3rd.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {

namespace {
constexpr int32_t MAX_DATA_LEN = 4096000;   // Maximum MTU Value for Logical Session:4M
const char* DM_3RD_AUTH_ACL_SESSION_NAME = "ohos.distributedhardware.devicemanager.auth3rdDeviceWithAcl";
const char* DM_AUTH_3RD_SESSION_NAME = "ohos.distributedhardware.devicemanager.auth3rdDevice";
}

SoftbusSession3rd::SoftbusSession3rd()
{
    LOGD("SoftbusSession3rd constructor.");
}

SoftbusSession3rd::~SoftbusSession3rd()
{
    LOGD("SoftbusSession3rd destructor.");
}

ConnectionAddr SoftbusSession3rd::CreateWifiAddr(const PeerTargetId3rd &targetId)
{
    ConnectionAddr addrWlan = {
        .type = ConnectionAddrType::CONNECTION_ADDR_WLAN,
        .info{
            .ip{
                .port = targetId.wifiPort,
            } }
    };
    if (memcpy_s(addrWlan.info.ip.ip, IP_STR_MAX_LEN, targetId.wifiIp.c_str(), targetId.wifiIp.length()) != 0) {
        LOGE("get ip addrWlan: %{public}s failed", GetAnonyString(targetId.wifiIp).c_str());
        return addrWlan;
    }
    return addrWlan;
}

ConnectionAddr SoftbusSession3rd::CreateBleAddr(const PeerTargetId3rd &targetId)
{
    ConnectionAddr addrBle = {
        .type = ConnectionAddrType::CONNECTION_ADDR_BLE,
        .info{
            .br{} }
    };
    if (memcpy_s(addrBle.info.ble.bleMac, BT_MAC_LEN, targetId.bleMac.c_str(), targetId.bleMac.length()) != 0) {
        LOGE("get bleMac addrBle: %{public}s failed", GetAnonyString(targetId.bleMac).c_str());
        return addrBle;
    }
    return addrBle;
}

ConnectionAddr SoftbusSession3rd::CreateBrAddr(const PeerTargetId3rd &targetId)
{
    ConnectionAddr addrBr = {
        .type = ConnectionAddrType::CONNECTION_ADDR_BR,
        .info{
            .br{} }
    };
    if (memcpy_s(addrBr.info.br.brMac, BT_MAC_LEN, targetId.brMac.c_str(), targetId.brMac.length()) != 0) {
        LOGE("get brMac addrBr: %{public}s failed", GetAnonyString(targetId.brMac).c_str());
        return addrBr;
    }
    return addrBr;
}

ConnectionAddr SoftbusSession3rd::GetAddrByTargetId(const PeerTargetId3rd &targetId)
{
    if (!targetId.wifiIp.empty() && targetId.wifiIp.length() <= IP_STR_MAX_LEN) {
        return CreateWifiAddr(targetId);
    }
    if (!targetId.bleMac.empty() && targetId.bleMac.length() <= BT_MAC_LEN) {
        return CreateBleAddr(targetId);
    }
    if (!targetId.brMac.empty() && targetId.brMac.length() <= BT_MAC_LEN) {
        return CreateBrAddr(targetId);
    }
    return ConnectionAddr{};
}

int32_t SoftbusSession3rd::OpenSessionServer(const PeerTargetId3rd &targetId)
{
    int32_t sessionId = -1;
    ConnectionAddr addrInfo = GetAddrByTargetId(targetId);
    sessionId = ::OpenAuthSession(DM_3RD_AUTH_ACL_SESSION_NAME, &addrInfo, 1, nullptr);
    if (sessionId < 0) {
        LOGE("[SOFTBUS]open session error, sessionId: %{public}d.", sessionId);
        return sessionId;
    }
    return sessionId;
}

int32_t SoftbusSession3rd::SendData(int32_t sessionId, const std::string &message)
{
    if (message.size() > MAX_DATA_LEN) {
        LOGE("SendData size is %{public}zu too long.", message.size());
        return ERR_DM_FAILED;
    }

    int32_t ret = SendBytes(sessionId, message.c_str(), strlen(message.c_str()));
    if (ret != DM_OK) {
        LOGE("[SOFTBUS]SendBytes failed.");
        return ret;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS