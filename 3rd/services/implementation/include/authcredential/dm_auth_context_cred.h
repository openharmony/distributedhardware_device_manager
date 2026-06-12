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

#ifndef OHOS_DM_AUTH_CONTEXT_CRED_H
#define OHOS_DM_AUTH_CONTEXT_CRED_H
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "auth_manager_cred.h"
#include "device_manager_data_struct_3rd.h"
#include "dm_anonymous_3rd.h"
#include "dm_auth_info_3rd.h"
#include "dm_auth_message_processor_cred.h"
#include "dm_constants_3rd.h"
#include "dm_log_3rd.h"
#include "dm_timer_3rd.h"
#include "ffrt.h"
#include "idevice_manager_service_listener_3rd.h"
#include "softbus_connector_3rd.h"
#include "softbus_session_3rd.h"

namespace OHOS {
namespace DistributedHardware {

class DmAuthStateMachineCred;
class DmAuthMessageProcessorCred;

using CleanNotifyCallback = std::function<void(uint64_t, int32_t, ProcessInfo3rd)>;
// PIN Code Authentication Type
enum DmAuthCredDirection {
    DM_AUTH_CRED_SOURCE = 0,
    DM_AUTH_CRED_SINK,
};

struct DmCredAccess {
    std::string deviceId;
    std::string deviceIdHash;
    int32_t userId{-1};
    int32_t uid{0};
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
    std::string networkId;
    std::vector<unsigned char> sessionKey;
    std::string openIdHash;
    std::string ownerId;
    uint8_t credType{0};
    std::string transmitCredentialId;
};

struct DmAuthCredContext {
    uint64_t logicalSessionId;
    int32_t sessionId;
    int64_t requestId;          // HiChain authentication ID
    int32_t reason{DM_OK};
    int32_t reply;
    std::string extraInfo;      // Expandable field, key-value structure
    DmAuthCredDirection direction;  // Indicator of authentication direction
    DmCredAccess accesser;
    DmCredAccess accessee;
    std::shared_ptr<DmAuthStateMachineCred> authStateMachine;
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener;
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector;
    std::shared_ptr<DmAuthMessageProcessorCred> authMessageProcessor;
    std::shared_ptr<SoftbusConnector3rd> softbusConnector;
    std::shared_ptr<DmTimer3rd> timer;
    CleanNotifyCallback cleanNotifyCallback;
    int32_t state;
    int32_t connDelayCloseTime;
    std::string transmitData;
    ProcessInfo3rd processInfo;
};
}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_CONTEXT_CRED_H
