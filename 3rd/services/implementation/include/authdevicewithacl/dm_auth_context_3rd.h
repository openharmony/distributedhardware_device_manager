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

#ifndef OHOS_DM_AUTH_CONTEXT_3RD_H
#define OHOS_DM_AUTH_CONTEXT_3RD_H
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "auth_manager_3rd.h"
#include "device_manager_data_struct_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_info_3rd.h"
#include "dm_auth_message_processor_3rd.h"
#include "dm_constants_3rd.h"
#include "dm_log_3rd.h"
#include "dm_timer_3rd.h"
#include "ffrt.h"
#include "idevice_manager_service_listener_3rd.h"
#include "softbus_connector_3rd.h"
#include "softbus_session_3rd.h"

namespace OHOS {
namespace DistributedHardware {

class DmAuthStateMachine3rd;
class DmAuthMessageProcessor3rd;

using CleanNotifyCallback = std::function<void(uint64_t, int32_t)>;
using StopTimerAndDelDpCallback = std::function<void(const std::string&, int32_t, uint64_t)>;
// PIN Code Authentication Type
enum DmAuthDirection {
    DM_AUTH_SOURCE = 0,
    DM_AUTH_SINK,
};

enum DmAuthSide {
    DM_AUTH_LOCAL_SIDE = 0,
    DM_AUTH_REMOTE_SIDE,
};

enum DmAuthScope {
    DM_AUTH_SCOPE_INVALID = 0,
    DM_AUTH_SCOPE_DEVICE,
    DM_AUTH_SCOPE_USER,
    DM_AUTH_SCOPE_APP,
    DM_AUTH_SCOPE_LNN,
    DM_AUTH_SCOPE_MAX,
};

typedef struct DmProxyAccess {
    int32_t uid;
    int32_t tokenId;
    std::string businessName;
    std::string processName;
    std::string peerBusinessName;
    std::string peerProcessName;
    std::string tokenIdHash;
    int32_t transmitSessionKeyId;
    int64_t transmitSkTimeStamp;
    int32_t bindLevel;
    std::vector<unsigned char> sessionKey;
} DmProxyAccess;

struct DmProxyAuthContext {
    std::string proxyContextId;
    DmProxyAccess proxyAccesser;
    DmProxyAccess proxyAccessee;
    bool operator==(const DmProxyAuthContext &other) const
    {
        return (proxyContextId == other.proxyContextId);
    }

    bool operator<(const DmProxyAuthContext &other) const
    {
        return proxyContextId < other.proxyContextId;
    }
};

struct DmAccess {
    std::string deviceId;
    std::string deviceIdHash;
    int32_t userId{-1};
    int32_t uid{-1};
    std::string accountId;
    std::string accountIdHash;
    int32_t tokenId;
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

struct DmAuthContext {
    uint64_t logicalSessionId;
    DmMessageType msgType;
    int32_t sessionId;
    int64_t requestId;          // HiChain authentication ID
    int32_t reason{DM_OK};
    int32_t reply;
    std::string extraInfo;      // Expandable field, key-value structure
    DmAuthDirection direction;  // Indicator of authentication direction
    DmAccess accesser;
    DmAccess accessee;
    std::vector<DmProxyAuthContext> subjectProxyOnes;
    std::shared_ptr<DmAuthStateMachine3rd> authStateMachine;
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener;
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector;
    std::shared_ptr<DmAuthMessageProcessor3rd> authMessageProcessor;
    std::shared_ptr<SoftbusConnector3rd> softbusConnector;
    std::shared_ptr<DmTimer3rd> timer;
    CleanNotifyCallback cleanNotifyCallback{nullptr};
    StopTimerAndDelDpCallback stopTimerAndDelDpCallback{nullptr};
    std::string importPkgName = "";
    std::string importAuthCode = "";
    int32_t state;
    int32_t connDelayCloseTime;
    bool IsProxyBind{false};
    bool IsCallingProxyAsSubject{true};
    std::string transmitData;
    ProcessInfo3rd processInfo;
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_CONTEXT_3RD_H
