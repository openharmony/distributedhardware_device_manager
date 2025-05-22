/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_DM_TRANSPORT_MSG_H
#define OHOS_DM_TRANSPORT_MSG_H

#include <string>
#include <cJSON.h>

namespace OHOS {
namespace DistributedHardware {
struct UserIdsMsg {
    std::vector<uint32_t> foregroundUserIds;
    std::vector<uint32_t> backgroundUserIds;
    bool isNewEvent;
    UserIdsMsg() : foregroundUserIds({}), backgroundUserIds({}), isNewEvent(false) {}
    UserIdsMsg(std::vector<uint32_t> foregroundUserIds, std::vector<uint32_t> backgroundUserIds, bool isNewEvent)
        : foregroundUserIds(foregroundUserIds), backgroundUserIds(backgroundUserIds), isNewEvent(isNewEvent) {}
};

void ToJson(cJSON *jsonObject, const UserIdsMsg &userIdsMsg);
void FromJson(const cJSON *jsonObject, UserIdsMsg &userIdsMsg);

struct CommMsg {
    int32_t code;
    std::string msg;
    CommMsg() : code(-1), msg("") {}
    CommMsg(int32_t code, std::string msg) : code(code), msg(msg) {}
};

void ToJson(cJSON *jsonObject, const CommMsg &commMsg);
void FromJson(const cJSON *jsonObject, CommMsg &commMsg);

std::string GetCommMsgString(const CommMsg &commMsg);

struct InnerCommMsg {
    std::string remoteNetworkId;
    std::shared_ptr<CommMsg> commMsg;
    int32_t socketId = 0;
    InnerCommMsg(std::string remoteNetworkId, std::shared_ptr<CommMsg> commMsg, int32_t socketId)
        : remoteNetworkId(remoteNetworkId), commMsg(commMsg), socketId(socketId) {}
};

struct NotifyUserIds {
    std::string remoteUdid;
    std::vector<uint32_t> userIds;
    NotifyUserIds() : remoteUdid(""), userIds({}) {}
    NotifyUserIds(std::string remoteUdid, std::vector<uint32_t> userIds)
        : remoteUdid(remoteUdid), userIds(userIds) {}

    std::string ToString();
};

void ToJson(cJSON *jsonObject, const NotifyUserIds &userIds);
void FromJson(const cJSON *jsonObject, NotifyUserIds &userIds);

struct LogoutAccountMsg {
    std::string accountId;
    int32_t userId;
    LogoutAccountMsg() : userId(-1) {}
    LogoutAccountMsg(const std::string &accountId, int32_t userId)
        : accountId(accountId), userId(userId) {}
};

void ToJson(cJSON *jsonObject, const LogoutAccountMsg &accountInfo);
void FromJson(const cJSON *jsonObject, LogoutAccountMsg &accountInfo);

struct UninstAppMsg {
    int32_t userId_;
    int32_t tokenId_;
    UninstAppMsg() : userId_(-1), tokenId_(-1) {}
    UninstAppMsg(const int32_t &userId, const int32_t &tokenId)
        : userId_(userId), tokenId_(tokenId) {}
};

void ToJson(cJSON *jsonObject, const UninstAppMsg &uninstAppMsg);
void FromJson(const cJSON *jsonObject, UninstAppMsg &uninstAppMsg);

struct UnBindAppMsg {
    int32_t userId_;
    int32_t tokenId_;
    std::string extra_;
    std::string udid_;
    UnBindAppMsg() : userId_(-1), tokenId_(-1), extra_(""), udid_("") {}
    UnBindAppMsg(const int32_t &userId, const int32_t &tokenId, const std::string &extra, const std::string &udid)
        : userId_(userId), tokenId_(tokenId), extra_(extra), udid_(udid) {}
};

void ToJson(cJSON *jsonObject, const UnBindAppMsg &unBindAppMsg);
void FromJson(const cJSON *jsonObject, UnBindAppMsg &unBindAppMsg);
} // DistributedHardware
} // OHOS
#endif