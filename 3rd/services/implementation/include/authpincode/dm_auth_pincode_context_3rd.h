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

#ifndef OHOS_DM_AUTH_PINCODE_CONTEXT_3RD_H
#define OHOS_DM_AUTH_PINCODE_CONTEXT_3RD_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ffrt.h"

#include "auth_pincode_manager_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_info_3rd.h"
#include "dm_auth_pincode_message_processor_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_log_3rd.h"
#include "dm_timer_3rd.h"
#include "hichain_auth_connector_3rd.h"
#include "idevice_manager_service_listener_3rd.h"
#include "softbus_connector_3rd.h"
#include "softbus_session_3rd.h"

namespace OHOS {
namespace DistributedHardware {

class DmAuthPincodeStateMachine3rd;
class DmAuthPincodeMessageProcessor3rd;

using CleanNotifyCallback = std::function<void(uint64_t, int32_t, ProcessInfo3rd)>;
using StopTimerAndDelDpCallback = std::function<void(const std::string&, int32_t, uint64_t)>;
// PIN Code Authentication Type
enum DmAuthPincodeDirection {
    DM_AUTH_PINCODE_SOURCE = 0,
    DM_AUTH_PINCODE_SINK,
};

struct DmPincodeAccess {
    std::string deviceId;
    std::string deviceIdHash;
    int32_t userId{-1};
    int32_t uid{-1};
    std::string accountId;
    std::string accountIdHash;
    uint32_t tokenId;
    std::string tokenIdHash;
    std::string processName;
    std::string businessName;
    int32_t transmitSessionKeyId; // Permanent application SKID on this end, returned by DP for ACL updates and aging
    int64_t transmitSkTimeStamp; // Used for aging, time is 2 days, application-level credential timestamp
    std::string dmVersion;
    std::string extraInfo;      // Expandable field, JSON format, KV structure
    int32_t bindLevel;
    std::string networkId;
    std::vector<unsigned char> sessionKey;
};

struct DmAuthPincodeContext {
    uint64_t logicalSessionId;
    DmPincodeMessageType msgType;
    int32_t sessionId;
    int64_t requestId;          // HiChain authentication ID
    int32_t reason{DM_OK};
    int32_t reply;
    std::string extraInfo;      // Expandable field, key-value structure
    DmAuthPincodeDirection direction;  // Indicator of authentication direction
    DmPincodeAccess accesser;
    DmPincodeAccess accessee;
    std::shared_ptr<DmAuthPincodeStateMachine3rd> authPinStateMac3rd;
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener;
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector;
    std::shared_ptr<DmAuthPincodeMessageProcessor3rd> authPinMsgProc3rd;
    std::shared_ptr<SoftbusConnector3rd> softbusConnector;
    std::shared_ptr<DmTimer3rd> timer;
    CleanNotifyCallback cleanNotifyCallback{nullptr};
    StopTimerAndDelDpCallback stopTimerAndDelDpCallback{nullptr};
    std::string importPkgName = "";
    std::string importAuthCode = "";
    int32_t state;
    int32_t connDelayCloseTime;
    std::string transmitData;
    ProcessInfo3rd processInfo;
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_PINCODE_CONTEXT_3RD_H