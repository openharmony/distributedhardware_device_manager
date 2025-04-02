/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_RELATIONSHIP_SYNC_MGR_H
#define OHOS_RELATIONSHIP_SYNC_MGR_H

#include <string>
#include "dm_cJSON.h"
#include "dm_single_instance.h"
namespace OHOS {
namespace DistributedHardware {
using namespace DmJson;
enum class RelationShipChangeType : uint32_t {
    ACCOUNT_LOGOUT = 0,
    DEVICE_UNBIND = 1,
    APP_UNBIND = 2,
    SERVICE_UNBIND = 3,
    DEL_USER = 4,
    APP_UNINSTALL = 5,
    SYNC_USERID = 6,
    STOP_USER = 7,
    TYPE_MAX = 8
};

struct UserIdInfo {
    /**
     * @brief true for foreground userid, false for background
     */
    bool isForeground;
    std::uint16_t userId;
    UserIdInfo(bool isForeground, std::uint16_t userId) : isForeground(isForeground), userId(userId) {}
    const std::string ToString() const;
};

struct RelationShipChangeMsg {
    RelationShipChangeType type;
    uint32_t userId;
    std::string accountId;
    uint64_t tokenId;
    uint64_t peerTokenId = 0;
    // The broadcast need send to these devices with the udids
    std::vector<std::string> peerUdids;
    // The broadcast from which device with the udid.
    std::string peerUdid;
    std::string accountName;
    // true: request, false: response
    bool syncUserIdFlag;
    // The foreground and background user id infos
    std::vector<UserIdInfo> userIdInfos;

    explicit RelationShipChangeMsg();
    bool ToBroadcastPayLoad(uint8_t *&msg, uint32_t &len) const;
    bool FromBroadcastPayLoad(const cJSON *payloadJson, RelationShipChangeType type);
    bool IsValid() const;
    bool IsChangeTypeValid();
    bool IsChangeTypeValid(uint32_t type);
    void ToAccountLogoutPayLoad(uint8_t *&msg, uint32_t &len) const;
    void ToDeviceUnbindPayLoad(uint8_t *&msg, uint32_t &len) const;
    void ToAppUnbindPayLoad(uint8_t *&msg, uint32_t &len) const;
    bool ToSyncFrontOrBackUserIdPayLoad(uint8_t *&msg, uint32_t &len) const;
    void ToDelUserPayLoad(uint8_t *&msg, uint32_t &len) const;
    void ToStopUserPayLoad(uint8_t *&msg, uint32_t &len) const;
    cJSON *ToPayLoadJson() const;

    bool FromAccountLogoutPayLoad(const cJSON *payloadJson);
    bool FromDeviceUnbindPayLoad(const cJSON *payloadJson);
    bool FromAppUnbindPayLoad(const cJSON *payloadJson);
    bool FromSyncFrontOrBackUserIdPayLoad(const cJSON *payloadJson);
    bool FromDelUserPayLoad(const cJSON *payloadJson);
    bool FromStopUserPayLoad(const cJSON *payloadJson);

    std::string ToJson() const;
    bool FromJson(const std::string &msgJson);

    const std::string ToString() const;
};

class ReleationShipSyncMgr {
DM_DECLARE_SINGLE_INSTANCE(ReleationShipSyncMgr);
public:
    std::string SyncTrustRelationShip(const RelationShipChangeMsg &msg);
    RelationShipChangeMsg ParseTrustRelationShipChange(const std::string &msgJson);
};

const std::string GetUserIdInfoList(const std::vector<UserIdInfo> &list);
void GetFrontAndBackUserIdInfos(const std::vector<UserIdInfo> &remoteUserIdInfos,
    std::vector<UserIdInfo> &foregroundUserIdInfos, std::vector<UserIdInfo> &backgroundUserIdInfos);
} // DistributedHardware
} // OHOS
#endif // OHOS_RELATIONSHIP_SYNC_MGR_H