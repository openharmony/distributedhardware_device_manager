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


#ifndef OHOS_DM_DATA_STRUCT_3RD_H
#define OHOS_DM_DATA_STRUCT_3RD_H

#include <stdint.h>
#include <string>

#define DM_MAX_PIN_CODE_LEN_3RD (1025)

namespace OHOS {
namespace DistributedHardware {

typedef struct ProcessInfo3rd {
    uint32_t tokenId;
    uint32_t uid;
    std::string processName;
    std::string businessName;
    int32_t userId;

    bool operator==(const ProcessInfo3rd &other) const
    {
        return (tokenId == other.tokenId) && (uid == other.uid) &&
            (processName == other.processName) && (businessName == other.businessName) &&
            (userId == other.userId);
    }

    bool operator<(const ProcessInfo3rd &other) const
    {
        return (tokenId < other.tokenId) ||
            (tokenId == other.tokenId && uid < other.uid) ||
            (tokenId == other.tokenId && uid == other.uid && processName < other.processName) ||
            (tokenId == other.tokenId && uid == other.uid && processName == other.processName &&
                businessName < other.businessName) ||
            (tokenId == other.tokenId && uid == other.uid && processName == other.processName &&
            businessName == other.businessName && userId < other.userId);
    }
} ProcessInfo3rd;

typedef struct PeerTargetId3rd {
    std::string deviceId;
    std::string brMac;
    std::string bleMac;
    std::string wifiIp;
    uint16_t wifiPort = 0;

    bool operator==(const PeerTargetId3rd &other) const
    {
        return (deviceId == other.deviceId) && (brMac == other.brMac) &&
            (bleMac == other.bleMac) && (wifiIp == other.wifiIp) && (wifiPort == other.wifiPort);
    }

    bool operator<(const PeerTargetId3rd &other) const
    {
        return (deviceId < other.deviceId) ||
            (deviceId == other.deviceId && brMac < other.brMac) ||
            (deviceId == other.deviceId && brMac == other.brMac && bleMac < other.bleMac) ||
            (deviceId == other.deviceId && brMac == other.brMac && bleMac == other.bleMac && wifiIp < other.wifiIp) ||
            (deviceId == other.deviceId && brMac == other.brMac && bleMac == other.bleMac && wifiIp == other.wifiIp &&
                wifiPort < other.wifiPort);
    }
} PeerTargetId3rd;

typedef struct PinCodeInfo {
    bool pincodeValidFlag = false;
    char pinCode[DM_MAX_PIN_CODE_LEN_3RD] = {0};
} PinCodeInfo;
} // namespace DistributedHardware
} // namespace OHOS
#endif //OHOS_DM_DATA_STRUCT_3RD_H