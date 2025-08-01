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

#include "relationship_sync_mgr.h"

#include <ctime>
#include <sstream>

#include "dm_anonymous.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
    DM_IMPLEMENT_SINGLE_INSTANCE(ReleationShipSyncMgr);
namespace {
    /**
     * @brief account logout payload length 9 bytes
     * |      2 bytes         |         6 bytes          | 1 bytes     |
     * | userid lower 2 bytes | account id first 6 bytes | broadcastId |
     */
    const int32_t ACCOUNT_LOGOUT_PAYLOAD_LEN = 9;
    /**
     * @brief device unbind payload length 2 bytes
     * |      2 bytes         | 1 bytes     |
     * | userid lower 2 bytes | broadcastId |
     */
    const int32_t DEVICE_UNBIND_PAYLOAD_LEN = 3;
    /**
     * @brief app unbind payload length 6 bytes
     * |      2 bytes         |         4 bytes          |         4 bytes              | 1 bytes     |
     * | userid lower 2 bytes |  token id lower 4 bytes  |  peertoken id lower 4 bytes  | broadcastId |
     */
    const int32_t APP_UNBIND_PAYLOAD_LEN = 11;
    /**
     * @brief delete user payload length 2 bytes
     * |      2 bytes         | 1 bytes     |
     * | userid lower 2 bytes | broadcastId |
     */
    const int32_t DEL_USER_PAYLOAD_LEN = 3;
    const int32_t STOP_USER_PAYLOAD_LEN = 3;
    const int32_t SHARE_UNBIND_PAYLOAD_LEN = 9;
    const int32_t SHARE_UNBIND_BROADCAST_LEN = 8;
    const int32_t APP_UNINSTALL_PAYLOAD_LEN = 7;
    /**
     * @brief the userid payload cost 2 bytes.
     *
     */
    const int32_t USERID_PAYLOAD_LEN = 2;
    const int32_t TOKENID_PAYLOAD_LEN = 6;
    const int32_t ACCOUNTID_PAYLOAD_LEN = 6;
    const int32_t SYNC_FRONT_OR_BACK_USERID_PAYLOAD_MAX_LEN = 11;
    const int32_t SYNC_FRONT_OR_BACK_USERID_PAYLOAD_MIN_LEN = 3;
    const int32_t USERID_BYTES = 2;
    const int32_t BITS_PER_BYTE = 8;
    const int32_t INVALIED_PAYLOAD_SIZE = 12;
    const int32_t CREDID_PAYLOAD_LEN = 8;
    const int32_t ACCOUNT_LOGOUT_BROADCAST_LEN = 8;
    const int32_t BROADCAST_PAYLOAD_LEN = 10;
    const int32_t BROADCAST_TIMEOUT_S = 5;
    const int32_t CURRENT_TIME_SEC_FLAG = 10;
    const int32_t GET_CURRENT_TIME_MAX_NUM = 3;

    const char * const MSG_TYPE = "TYPE";
    const char * const MSG_VALUE = "VALUE";
    const char * const MSG_PEER_UDID = "PEER_UDID";
    const char * const MSG_ACCOUNTID = "ACCOUNTID";
    const char * const MSG_PEER_TOKENID = "PEER_TOKENID";

    // The need response mask offset, the 8th bit.
    const int32_t NEED_RSP_MASK_OFFSET = 7;
    const int32_t IS_NEW_USER_SYNC_MASK_OFFSET = 6;
    /**
     * @brief The userid cost 2 byte, the heigher part set on the 2th byte.
     *        The Max user id is just above 10000+, cost 15bits. We use the first bit
     *        to mark the user id foreground or background.
     *        1 means foreground user, 0 means background user.
     * @example foreground userid: 100 -> 0000 0000 0110 0100
     *          we save it in payload:
     *          |----first byte----|----second byte----|
     *          ----------------------------------------
     *          |     0110 0100    |    1000 0000      |
     */
    const int32_t FRONT_OR_BACK_USER_FLAG_OFFSET = 7;
    const uint8_t FRONT_OR_BACK_USER_FLAG_MASK = 0b01111111;
    const uint8_t FRONT_OR_BACK_FLAG_MASK = 0b10000000;
    // The total number of foreground and background userids offset, the 3th ~ 5th bits.
    const uint16_t ALL_USERID_NUM_MASK_OFFSET = 3;
    const uint16_t FOREGROUND_USERID_LEN_MASK = 0b00000111;
    const uint16_t ALL_USERID_NUM_MASK = 0b00111000;
    const uint32_t MAX_MEM_MALLOC_SIZE = 4 * 1024;
    const uint32_t MAX_USER_ID_NUM = 4;
}

RelationShipChangeMsg::RelationShipChangeMsg() : type(RelationShipChangeType::TYPE_MAX),
    userId(UINT32_MAX), accountId(""), tokenId(UINT64_MAX), peerUdids({}), peerUdid(""), accountName(""),
    syncUserIdFlag(false), userIdInfos({}), isNewEvent(false), broadCastId(UINT8_MAX)
{
}

bool RelationShipChangeMsg::ToBroadcastPayLoad(uint8_t *&msg, uint32_t &len) const
{
    if (!IsValid()) {
        LOGE("invalid");
        return false;
    }

    bool ret = false;
    switch (type) {
        case RelationShipChangeType::ACCOUNT_LOGOUT:
            ToAccountLogoutPayLoad(msg, len);
            ret = true;
            break;
        case RelationShipChangeType::DEVICE_UNBIND:
            ToDeviceUnbindPayLoad(msg, len);
            ret = true;
            break;
        case RelationShipChangeType::APP_UNBIND:
            ToAppUnbindPayLoad(msg, len);
            ret = true;
            break;
        case RelationShipChangeType::SERVICE_UNBIND:
            ToServiceUnbindPayLoad(msg, len);
            ret = true;
            break;
        case RelationShipChangeType::SYNC_USERID:
            ret = ToSyncFrontOrBackUserIdPayLoad(msg, len);
            break;
        case RelationShipChangeType::DEL_USER:
            ToDelUserPayLoad(msg, len);
            ret = true;
            break;
        case RelationShipChangeType::STOP_USER:
            ToStopUserPayLoad(msg, len);
            ret = true;
            break;
        case RelationShipChangeType::SHARE_UNBIND:
            ToShareUnbindPayLoad(msg, len);
            ret = true;
            break;
        case RelationShipChangeType::APP_UNINSTALL:
            ToAppUninstallPayLoad(msg, len);
            ret = true;
            break;
        default:
            LOGE("RelationShipChange type invalid");
            break;
    }
    return ret;
}

void RelationShipChangeMsg::ToShareUnbindPayLoad(uint8_t *&msg, uint32_t &len) const
{
    if (credId.length() <= (CREDID_PAYLOAD_LEN - USERID_PAYLOAD_LEN)) {
        LOGE("ToShareUnbindPayLoad credId length is invalid.");
        len = 0;
        return;
    }
    msg = new uint8_t[SHARE_UNBIND_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
    }

    for (int i = USERID_PAYLOAD_LEN; i < CREDID_PAYLOAD_LEN; i++) {
        msg[i] = credId[i - USERID_PAYLOAD_LEN];
    }
    for (int i = CREDID_PAYLOAD_LEN; i < SHARE_UNBIND_PAYLOAD_LEN; i++) {
        msg[i] |= (broadCastId >> ((i - CREDID_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }
    len = SHARE_UNBIND_PAYLOAD_LEN;
}

bool RelationShipChangeMsg::FromBroadcastPayLoad(const cJSON *payloadJson, RelationShipChangeType type)
{
    LOGI("FromBroadcastPayLoad type %{public}d.", type);
    if (type == RelationShipChangeType::TYPE_MAX) {
        LOGE("ChangeType invalid, type: %{public}d", type);
        return false;
    }
    bool ret = false;
    switch (type) {
        case RelationShipChangeType::ACCOUNT_LOGOUT:
            ret = FromAccountLogoutPayLoad(payloadJson);
            break;
        case RelationShipChangeType::DEVICE_UNBIND:
            ret = FromDeviceUnbindPayLoad(payloadJson);
            break;
        case RelationShipChangeType::APP_UNBIND:
            ret = FromAppUnbindPayLoad(payloadJson);
            break;
        case RelationShipChangeType::SERVICE_UNBIND:
            ret = FromServiceUnbindPayLoad(payloadJson);
            break;
        case RelationShipChangeType::SYNC_USERID:
            ret = FromSyncFrontOrBackUserIdPayLoad(payloadJson);
            break;
        case RelationShipChangeType::DEL_USER:
            ret = FromDelUserPayLoad(payloadJson);
            break;
        case RelationShipChangeType::STOP_USER:
            ret = FromStopUserPayLoad(payloadJson);
            break;
        case RelationShipChangeType::SHARE_UNBIND:
            ret = FromShareUnbindPayLoad(payloadJson);
            break;
        case RelationShipChangeType::APP_UNINSTALL:
            ret = FromAppUninstallPayLoad(payloadJson);
            break;
        default:
            LOGE("RelationShipChange type invalid");
            break;
    }
    return ret;
}

bool RelationShipChangeMsg::FromShareUnbindPayLoad(const cJSON *payloadJson)
{
    if (payloadJson == NULL) {
        LOGE("Share unbind payloadJson is null.");
        return false;
    }
    int32_t arraySize = cJSON_GetArraySize(payloadJson);
    if (arraySize < SHARE_UNBIND_PAYLOAD_LEN || arraySize >= INVALIED_PAYLOAD_SIZE) {
        LOGE("Payload invalied,the size is %{public}d.", arraySize);
        return false;
    }
    userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BITS_PER_BYTE);
        }
    }
    credId = "";
    for (uint32_t j = USERID_PAYLOAD_LEN; j < CREDID_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            credId += static_cast<char>(payloadItem->valueint);
        }
    }
    this->broadCastId = 0;
    for (uint32_t j = CREDID_PAYLOAD_LEN; j < SHARE_UNBIND_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            this->broadCastId |= (static_cast<uint8_t>(payloadItem->valueint)) <<
                ((j - CREDID_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
    }
    return true;
}

bool RelationShipChangeMsg::IsValid() const
{
    bool ret = false;
    switch (type) {
        case RelationShipChangeType::ACCOUNT_LOGOUT:
            ret = (userId != UINT32_MAX && accountId.length() >= ACCOUNTID_PAYLOAD_LEN);
            break;
        case RelationShipChangeType::DEVICE_UNBIND:
            ret = (userId != UINT32_MAX);
            break;
        case RelationShipChangeType::APP_UNBIND:
            ret = (userId != UINT32_MAX && tokenId != UINT64_MAX);
            break;
        case RelationShipChangeType::DEL_USER:
            ret = (userId != UINT32_MAX);
            break;
        case RelationShipChangeType::STOP_USER:
            ret = (userId != UINT32_MAX);
            break;
        case RelationShipChangeType::SHARE_UNBIND:
            ret = (userId != UINT32_MAX);
            break;
        case RelationShipChangeType::SERVICE_UNBIND:
            ret = (userId != UINT32_MAX);
            break;
        case RelationShipChangeType::APP_UNINSTALL:
            ret = (userId != UINT32_MAX && tokenId != UINT64_MAX);
            break;
        case RelationShipChangeType::SYNC_USERID:
            ret = (!userIdInfos.empty() &&
                (static_cast<uint32_t>(userIdInfos.size()) <= MAX_USER_ID_NUM));
            break;
        case RelationShipChangeType::TYPE_MAX:
            ret = false;
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

bool RelationShipChangeMsg::IsChangeTypeValid()
{
    return (type == RelationShipChangeType::ACCOUNT_LOGOUT) || (type == RelationShipChangeType::DEVICE_UNBIND) ||
        (type == RelationShipChangeType::APP_UNBIND) || (type == RelationShipChangeType::SYNC_USERID) ||
        (type == RelationShipChangeType::DEL_USER) || (type == RelationShipChangeType::STOP_USER) ||
        (type == RelationShipChangeType::SERVICE_UNBIND) || (type == RelationShipChangeType::APP_UNINSTALL);
}

bool RelationShipChangeMsg::IsChangeTypeValid(uint32_t type)
{
    return (type == (uint32_t)RelationShipChangeType::ACCOUNT_LOGOUT) ||
        (type == (uint32_t)RelationShipChangeType::DEVICE_UNBIND) ||
        (type == (uint32_t)RelationShipChangeType::APP_UNBIND) ||
        (type == (uint32_t)RelationShipChangeType::SYNC_USERID) ||
        (type == (uint32_t)RelationShipChangeType::DEL_USER) ||
        (type == (uint32_t)RelationShipChangeType::STOP_USER) ||
        (type == (uint32_t)RelationShipChangeType::SHARE_UNBIND) ||
        (type == (uint32_t)RelationShipChangeType::SERVICE_UNBIND) ||
        (type == (uint32_t)RelationShipChangeType::APP_UNINSTALL);
}

void RelationShipChangeMsg::ToAccountLogoutPayLoad(uint8_t *&msg, uint32_t &len) const
{
    msg = new uint8_t[ACCOUNT_LOGOUT_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
    }

    for (int j = USERID_PAYLOAD_LEN; j < ACCOUNT_LOGOUT_BROADCAST_LEN; j++) {
        msg[j] = accountId[j - USERID_PAYLOAD_LEN];
    }

    for (int j = ACCOUNT_LOGOUT_BROADCAST_LEN; j < ACCOUNT_LOGOUT_PAYLOAD_LEN; j++) {
        msg[j] |= (broadCastId >> ((j - ACCOUNT_LOGOUT_BROADCAST_LEN) * BITS_PER_BYTE)) & 0xFF;
    }
    len = ACCOUNT_LOGOUT_PAYLOAD_LEN;
}

void RelationShipChangeMsg::ToDeviceUnbindPayLoad(uint8_t *&msg, uint32_t &len) const
{
    msg = new uint8_t[DEVICE_UNBIND_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
    }
    for (int i = USERID_PAYLOAD_LEN; i < DEVICE_UNBIND_PAYLOAD_LEN; i++) {
        msg[i] |= (broadCastId >> ((i - USERID_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }
    len = DEVICE_UNBIND_PAYLOAD_LEN;
}

void RelationShipChangeMsg::ToAppUnbindPayLoad(uint8_t *&msg, uint32_t &len) const
{
    msg = new uint8_t[APP_UNBIND_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
    }

    for (int i = USERID_PAYLOAD_LEN; i < TOKENID_PAYLOAD_LEN; i++) {
        msg[i] |= (tokenId >> ((i - USERID_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }

    for (int i = TOKENID_PAYLOAD_LEN; i < BROADCAST_PAYLOAD_LEN; i++) {
        msg[i] |= (peerTokenId >> ((i - TOKENID_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }

    for (int i = BROADCAST_PAYLOAD_LEN; i < APP_UNBIND_PAYLOAD_LEN; i++) {
        msg[i] |= (broadCastId >> ((i - BROADCAST_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }

    len = APP_UNBIND_PAYLOAD_LEN;
}

void RelationShipChangeMsg::ToAppUninstallPayLoad(uint8_t *&msg, uint32_t &len) const
{
    msg = new uint8_t[APP_UNINSTALL_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
    }

    for (int i = USERID_PAYLOAD_LEN; i < TOKENID_PAYLOAD_LEN; i++) {
        msg[i] |= (tokenId >> ((i - USERID_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }

    for (int i = TOKENID_PAYLOAD_LEN; i < APP_UNINSTALL_PAYLOAD_LEN; i++) {
        msg[i] |= (broadCastId >> ((i - TOKENID_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }
    len = APP_UNINSTALL_PAYLOAD_LEN;
}

void RelationShipChangeMsg::ToServiceUnbindPayLoad(uint8_t *&msg, uint32_t &len) const
{
    ToAppUnbindPayLoad(msg, len);
}

bool RelationShipChangeMsg::ToSyncFrontOrBackUserIdPayLoad(uint8_t *&msg, uint32_t &len) const
{
    uint32_t userIdNum = static_cast<uint32_t>(userIdInfos.size());
    if (userIdNum > MAX_USER_ID_NUM) {
        LOGE("userIdNum too many, %{public}u", userIdNum);
        return false;
    }

    len = (userIdNum + 1) * USERID_BYTES;

    if (len > INVALIED_PAYLOAD_SIZE) {
        LOGE("len too long");
        return false;
    }
    msg = new uint8_t[len]();
    if (syncUserIdFlag) {
        msg[0] |= 0x1 << NEED_RSP_MASK_OFFSET;
    } else {
        msg[0] |= 0x0 << NEED_RSP_MASK_OFFSET;
    }
    if (isNewEvent) {
        msg[0] |= 0x1 << IS_NEW_USER_SYNC_MASK_OFFSET;
    }

    msg[0] |= userIdNum;
    int32_t userIdIdx = 0;
    for (uint32_t idx = 1; idx <=  (len - USERID_BYTES);) {
        msg[idx] |= userIdInfos[userIdIdx].userId & 0xFF;
        msg[idx + 1] |= (userIdInfos[userIdIdx].userId >> BITS_PER_BYTE) & 0xFF;
        if (userIdInfos[userIdIdx].isForeground) {
            msg[idx + 1] |= (0x1 << FRONT_OR_BACK_USER_FLAG_OFFSET);
        }
        idx += USERID_BYTES;
        userIdIdx++;
    }
    msg[userIdNum * USERID_BYTES + 1] |= broadCastId & 0xFF;
    return true;
}

void RelationShipChangeMsg::ToDelUserPayLoad(uint8_t *&msg, uint32_t &len) const
{
    len = DEL_USER_PAYLOAD_LEN;
    msg = new uint8_t[DEL_USER_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
    }
    for (int i = USERID_PAYLOAD_LEN; i < DEL_USER_PAYLOAD_LEN; i++) {
        msg[i] |= (broadCastId >> ((i - USERID_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }
}

void RelationShipChangeMsg::ToStopUserPayLoad(uint8_t *&msg, uint32_t &len) const
{
    len = STOP_USER_PAYLOAD_LEN;
    msg = new uint8_t[STOP_USER_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
    }
    for (int i = USERID_PAYLOAD_LEN; i < STOP_USER_PAYLOAD_LEN; i++) {
        msg[i] |= (broadCastId >> ((i - USERID_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }
}

bool RelationShipChangeMsg::FromAccountLogoutPayLoad(const cJSON *payloadJson)
{
    if (payloadJson == NULL) {
        LOGE("Account logout payloadJson is null.");
        return false;
    }
    int32_t arraySize = cJSON_GetArraySize(payloadJson);
    if (arraySize < ACCOUNT_LOGOUT_PAYLOAD_LEN || arraySize >= INVALIED_PAYLOAD_SIZE) {
        LOGE("Payload invalied,the size is %{public}d.", arraySize);
        return false;
    }
    userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BITS_PER_BYTE);
        }
    }
    accountId = "";
    for (uint32_t j = USERID_PAYLOAD_LEN; j < ACCOUNT_LOGOUT_BROADCAST_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            accountId += static_cast<char>(payloadItem->valueint);
        }
    }
    broadCastId = 0;
    for (uint32_t j = ACCOUNT_LOGOUT_BROADCAST_LEN; j < ACCOUNT_LOGOUT_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            broadCastId |= (static_cast<uint8_t>(payloadItem->valueint)) <<
                ((j - ACCOUNT_LOGOUT_BROADCAST_LEN) * BITS_PER_BYTE);
        }
    }
    return true;
}

bool RelationShipChangeMsg::FromDeviceUnbindPayLoad(const cJSON *payloadJson)
{
    if (payloadJson == NULL) {
        LOGE("Device unbind payloadJson is null.");
        return false;
    }
    int32_t arraySize = cJSON_GetArraySize(payloadJson);
    if (arraySize < DEVICE_UNBIND_PAYLOAD_LEN || arraySize >= INVALIED_PAYLOAD_SIZE) {
        LOGE("Payload invalied,the size is %{public}d.", arraySize);
        return false;
    }
    userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BITS_PER_BYTE);
        }
    }
    broadCastId = 0;
    for (uint32_t j = USERID_PAYLOAD_LEN; j < DEVICE_UNBIND_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            broadCastId |= (static_cast<uint8_t>(payloadItem->valueint)) <<
                ((j - USERID_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
    }
    return true;
}

bool RelationShipChangeMsg::FromAppUnbindPayLoad(const cJSON *payloadJson)
{
    if (payloadJson == NULL) {
        LOGE("App unbind payloadJson is null.");
        return false;
    }
    int32_t arraySize = cJSON_GetArraySize(payloadJson);
    if (arraySize < ACCOUNT_LOGOUT_PAYLOAD_LEN || arraySize >= INVALIED_PAYLOAD_SIZE) {
        LOGE("Payload invalied,the size is %{public}d.", arraySize);
        return false;
    }
    userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BITS_PER_BYTE);
        }
    }
    tokenId = 0;
    for (uint32_t j = USERID_PAYLOAD_LEN; j < TOKENID_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            tokenId |= (static_cast<uint8_t>(payloadItem->valueint)) <<  ((j - USERID_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
    }
    peerTokenId = 0;
    for (uint32_t j = TOKENID_PAYLOAD_LEN; j < BROADCAST_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            peerTokenId |= (static_cast<uint8_t>(payloadItem->valueint)) <<
                ((j - TOKENID_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
    }
    broadCastId = 0;
    for (uint32_t j = BROADCAST_PAYLOAD_LEN; j < APP_UNBIND_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            broadCastId |= (static_cast<uint8_t>(payloadItem->valueint)) <<
                ((j - BROADCAST_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
    }
    return true;
}

bool RelationShipChangeMsg::FromAppUninstallPayLoad(const cJSON *payloadJson)
{
    if (payloadJson == NULL) {
        LOGE("App unbind payloadJson is null.");
        return false;
    }
    int32_t arraySize = cJSON_GetArraySize(payloadJson);
    if (arraySize < APP_UNINSTALL_PAYLOAD_LEN || arraySize >= INVALIED_PAYLOAD_SIZE) {
        LOGE("Payload invalied,the size is %{public}d.", arraySize);
        return false;
    }
    userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BITS_PER_BYTE);
        }
    }
    tokenId = 0;
    for (uint32_t j = USERID_PAYLOAD_LEN; j < TOKENID_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            tokenId |= (static_cast<uint8_t>(payloadItem->valueint)) <<  ((j - USERID_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
    }
    this->broadCastId = 0;
    for (uint32_t j = TOKENID_PAYLOAD_LEN; j < APP_UNINSTALL_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            this->broadCastId |= (static_cast<uint8_t>(payloadItem->valueint)) <<
                ((j - TOKENID_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
    }
    return true;
}

bool RelationShipChangeMsg::FromServiceUnbindPayLoad(const cJSON *payloadJson)
{
    return FromAppUnbindPayLoad(payloadJson);
}

bool RelationShipChangeMsg::GetBroadCastId(const cJSON *payloadJson, uint32_t userIdNum)
{
    broadCastId = 0;
    cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, userIdNum * USERID_BYTES + 1);
    CHECK_NULL_RETURN(payloadItem, false);
    if (cJSON_IsNumber(payloadItem)) {
        broadCastId |= static_cast<uint8_t>(payloadItem->valueint);
    }
    return true;
}

bool RelationShipChangeMsg::FromSyncFrontOrBackUserIdPayLoad(const cJSON *payloadJson)
{
    if (payloadJson == NULL) {
        LOGE("payloadJson is null.");
        return false;
    }

    int32_t arraySize = cJSON_GetArraySize(payloadJson);
    if (arraySize < SYNC_FRONT_OR_BACK_USERID_PAYLOAD_MIN_LEN ||
        arraySize > SYNC_FRONT_OR_BACK_USERID_PAYLOAD_MAX_LEN) {
        LOGE("Payload invalid, the size is %{public}d.", arraySize);
        return false;
    }

    cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, 0);
    CHECK_NULL_RETURN(payloadItem, false);
    uint32_t userIdNum = 0;
    if (cJSON_IsNumber(payloadItem)) {
        uint8_t val = static_cast<uint8_t>(payloadItem->valueint);
        this->syncUserIdFlag = (((val >> NEED_RSP_MASK_OFFSET) & 0x1) == 0x1);
        this->isNewEvent = (((val >> IS_NEW_USER_SYNC_MASK_OFFSET) & 0x1) == 0x1);
        userIdNum = ((static_cast<uint8_t>(payloadItem->valueint)) & FOREGROUND_USERID_LEN_MASK);
    }

    int32_t effectiveLen = static_cast<int32_t>((userIdNum + 1) * USERID_BYTES);
    if (effectiveLen > arraySize) {
        LOGE("payload userIdNum invalid, userIdNum: %{public}u, arraySize: %{public}d", userIdNum, arraySize);
        return false;
    }

    uint16_t tempUserId = 0;
    bool isForegroundUser = false;
    for (int32_t idx = 1; idx < effectiveLen; idx++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, idx);
        CHECK_NULL_RETURN(payloadItem, false);
        if (!cJSON_IsNumber(payloadItem)) {
            LOGE("Payload invalid, user id not integer");
            return false;
        }
        if ((idx - 1) % USERID_BYTES == 0) {
            tempUserId |= (static_cast<uint8_t>(payloadItem->valueint));
        }
        if ((idx - 1) % USERID_BYTES == 1) {
            tempUserId |= (static_cast<uint8_t>(payloadItem->valueint) << BITS_PER_BYTE);
            tempUserId &= FRONT_OR_BACK_USER_FLAG_MASK;
            isForegroundUser = (((static_cast<uint8_t>(payloadItem->valueint) & FRONT_OR_BACK_FLAG_MASK) >>
                FRONT_OR_BACK_USER_FLAG_OFFSET) == 0b1);
            UserIdInfo userIdInfo(isForegroundUser, tempUserId);
            this->userIdInfos.push_back(userIdInfo);
            tempUserId = 0;
            isForegroundUser = false;
        }
    }
    return GetBroadCastId(payloadJson, userIdNum);
}

bool RelationShipChangeMsg::FromDelUserPayLoad(const cJSON *payloadJson)
{
    if (payloadJson == NULL) {
        LOGE("FromDelUserPayLoad payloadJson is null.");
        return false;
    }

    int32_t arraySize = cJSON_GetArraySize(payloadJson);
    if (arraySize < DEL_USER_PAYLOAD_LEN) {
        LOGE("Payload invalid, the size is %{public}d.", arraySize);
        return false;
    }
    this->userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            this->userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BITS_PER_BYTE);
        }
    }
    this->broadCastId = 0;
    for (uint32_t j = USERID_PAYLOAD_LEN; j < DEL_USER_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            this->broadCastId |= (static_cast<uint8_t>(payloadItem->valueint)) <<
                ((j - USERID_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
    }
    return true;
}

bool RelationShipChangeMsg::FromStopUserPayLoad(const cJSON *payloadJson)
{
    if (payloadJson == NULL) {
        LOGE("FromStopUserPayLoad payloadJson is null.");
        return false;
    }

    int32_t arraySize = cJSON_GetArraySize(payloadJson);
    if (arraySize < STOP_USER_PAYLOAD_LEN) {
        LOGE("Payload invalid, the size is %{public}d.", arraySize);
        return false;
    }
    this->userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            this->userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BITS_PER_BYTE);
        }
    }
    this->broadCastId = 0;
    for (uint32_t j = USERID_PAYLOAD_LEN; j < STOP_USER_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            this->broadCastId |= (static_cast<uint8_t>(payloadItem->valueint)) <<
                ((j - USERID_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
    }
    return true;
}

cJSON *RelationShipChangeMsg::ToPayLoadJson() const
{
    uint8_t *payload = nullptr;
    uint32_t len = 0;
    if (!this->ToBroadcastPayLoad(payload, len)) {
        LOGE("Get broadcast payload failed");
        return nullptr;
    }
    if (payload == nullptr || len == 0) {
        LOGE("payload is null or len is 0.");
        return nullptr;
    }
    cJSON *arrayObj = cJSON_CreateArray();
    if (arrayObj == nullptr) {
        LOGE("cJSON_CreateArray failed");
        if (payload != nullptr) {
            delete[] payload;
        }
        return nullptr;
    }
    cJSON *numberObj = nullptr;
    for (uint32_t index = 0; index < len; index++) {
        numberObj = cJSON_CreateNumber(payload[index]);
        if (numberObj == nullptr || !cJSON_AddItemToArray(arrayObj, numberObj)) {
            cJSON_Delete(numberObj);
            cJSON_Delete(arrayObj);
            if (payload != nullptr) {
                delete[] payload;
            }
            return nullptr;
        }
    }
    if (payload != nullptr) {
        delete[] payload;
    }
    return arrayObj;
}

std::string RelationShipChangeMsg::ToJson() const
{
    cJSON *msg = cJSON_CreateObject();
    if (msg == NULL) {
        LOGE("failed to create cjson object");
        return "";
    }
    cJSON_AddNumberToObject(msg, MSG_TYPE, (uint32_t)type);
    cJSON *arrayObj = ToPayLoadJson();
    if (arrayObj == nullptr || !cJSON_AddItemToObject(msg, MSG_VALUE, arrayObj)) {
        LOGE("ArrayObj is nullptr.");
        cJSON_Delete(arrayObj);
        cJSON_Delete(msg);
        return "";
    }

    cJSON *udidArrayObj = cJSON_CreateArray();
    if (udidArrayObj == nullptr || !cJSON_AddItemToObject(msg, MSG_PEER_UDID, udidArrayObj)) {
        LOGE("cJSON_CreateArray failed");
        cJSON_Delete(udidArrayObj);
        cJSON_Delete(msg);
        return "";
    }
    cJSON *udidStringObj = nullptr;
    for (uint32_t index = 0; index < peerUdids.size(); index++) {
        udidStringObj = cJSON_CreateString(peerUdids[index].c_str());
        if (udidStringObj == nullptr) {
            cJSON_Delete(msg);
            return "";
        }
        if (!cJSON_AddItemToArray(udidArrayObj, udidStringObj)) {
            cJSON_Delete(udidStringObj);
            cJSON_Delete(msg);
            return "";
        }
    }
    cJSON_AddStringToObject(msg, MSG_ACCOUNTID, accountName.c_str());
    char *retStr = cJSON_PrintUnformatted(msg);
    if (retStr == nullptr) {
        LOGE("to json is nullptr.");
        cJSON_Delete(msg);
        return "";
    }
    std::string ret = std::string(retStr);
    cJSON_Delete(msg);
    cJSON_free(retStr);
    return ret;
}

bool RelationShipChangeMsg::FromJson(const std::string &msgJson)
{
    cJSON *msgObj = cJSON_Parse(msgJson.c_str());
    if (msgObj == NULL) {
        LOGE("parse msg failed");
        return false;
    }

    cJSON *typeJson = cJSON_GetObjectItem(msgObj, MSG_TYPE);
    if (typeJson == NULL || !cJSON_IsNumber(typeJson) || !IsChangeTypeValid(typeJson->valueint)) {
        LOGE("parse type failed.");
        cJSON_Delete(msgObj);
        return false;
    }
    this->type = (RelationShipChangeType)typeJson->valueint;

    cJSON *payloadJson = cJSON_GetObjectItem(msgObj, MSG_VALUE);
    if (payloadJson == NULL || !cJSON_IsArray(payloadJson)) {
        LOGE("parse payload failed.");
        cJSON_Delete(msgObj);
        return false;
    }
    if (!this->FromBroadcastPayLoad(payloadJson, type)) {
        LOGE("parse payload error.");
        cJSON_Delete(msgObj);
        return false;
    }

    cJSON *peerUdidJson = cJSON_GetObjectItem(msgObj, MSG_PEER_UDID);
    if (peerUdidJson == NULL || !cJSON_IsString(peerUdidJson)) {
        LOGE("parse peer udid failed.");
        cJSON_Delete(msgObj);
        return false;
    }
    this->peerUdid = std::string(peerUdidJson->valuestring);
    cJSON_Delete(msgObj);
    return true;
}

const std::string RelationShipChangeMsg::ToMapKey() const
{
    std::ostringstream ret;
    std::string isNewEventStr = isNewEvent ? "true" : "false";
    ret << "_" << std::to_string(static_cast<uint32_t>(type));
    ret << "_" << isNewEventStr;
    ret << "_" << std::to_string(userId);
    ret << "_" << GetAnonyString(accountId);
    ret << "_" << GetAnonyString(peerUdid);
    ret << "_" << std::to_string(tokenId);
    ret << "_" << std::to_string(peerTokenId);
    ret << "_" << GetUserIdInfoList(userIdInfos);
    ret << "_" << std::to_string(syncUserIdFlag);
    ret << "_" << credId;
    return ret.str();
}

void ReleationShipSyncMgr::HandleRecvBroadCastTimeout(const std::string &key)
{
    std::lock_guard<std::mutex> autoLock(lock_);
    auto iter = recvBroadCastIdMap_.find(key);
    if (iter != recvBroadCastIdMap_.end()) {
        recvBroadCastIdMap_.erase(iter);
    }
}

bool ReleationShipSyncMgr::GetCurrentTimeSec(int32_t &sec)
{
    int32_t retryNum = 0;
    time_t now;
    struct tm ltm;
    while (retryNum < GET_CURRENT_TIME_MAX_NUM) {
        time(&now);
        struct tm *res = localtime_r(&now, &ltm);
        if (res == nullptr) {
            retryNum++;
            LOGE("get current time failed. retryNum: %{public}d", retryNum);
            continue;
        }
        sec = ltm.tm_sec % CURRENT_TIME_SEC_FLAG;
        if (sec == 0) {
            sec = CURRENT_TIME_SEC_FLAG;
        }
        return true;
    }
    return false;
}

std::string ReleationShipSyncMgr::SyncTrustRelationShip(RelationShipChangeMsg &msg)
{
    int32_t currentTimeSec = 0;
    msg.broadCastId = 0;
    if (!GetCurrentTimeSec(currentTimeSec)) {
        LOGE("get current time failed, use default value");
        return msg.ToJson();
    }
    msg.broadCastId = static_cast<uint8_t>(currentTimeSec);
    LOGI("send trust change msg: %{public}s", msg.ToString().c_str());
    return msg.ToJson();
}

RelationShipChangeMsg ReleationShipSyncMgr::ParseTrustRelationShipChange(const std::string &msgJson)
{
    RelationShipChangeMsg msgObj;
    if (!msgObj.FromJson(msgJson)) {
        LOGE("Parse json failed");
    }
    return msgObj;
}

bool ReleationShipSyncMgr::IsNewBroadCastId(const RelationShipChangeMsg &msg)
{
    if (msg.broadCastId == 0) {
        return true;
    }
    std::string key = msg.ToMapKey();
    std::lock_guard<std::mutex> autoLock(lock_);
    if (timer_ == nullptr) {
        timer_ = std::make_shared<DmTimer>();
    }
    auto iter = recvBroadCastIdMap_.find(key);
    if (iter == recvBroadCastIdMap_.end()) {
        recvBroadCastIdMap_[key] = msg.broadCastId;
        timer_->StartTimer(key, BROADCAST_TIMEOUT_S, [this](std::string key) {
            ReleationShipSyncMgr::HandleRecvBroadCastTimeout(key);
        });
        return true;
    } else if (iter != recvBroadCastIdMap_.end() && recvBroadCastIdMap_[key] != msg.broadCastId) {
        timer_->DeleteTimer(key);
        recvBroadCastIdMap_[key] = msg.broadCastId;
        timer_->StartTimer(key, BROADCAST_TIMEOUT_S, [this](std::string key) {
            ReleationShipSyncMgr::HandleRecvBroadCastTimeout(key);
        });
        return true;
    } else {
        return false;
    }
    return false;
}

const std::string RelationShipChangeMsg::ToString() const
{
    std::ostringstream ret;
    std::string isNewEventStr = isNewEvent ? "true" : "false";
    ret << "{ MsgType: " << std::to_string(static_cast<uint32_t>(type));
    ret << "{ isNewEvent: " << isNewEventStr;
    ret << ", userId: " << std::to_string(userId);
    ret << ", accountId: " << GetAnonyString(accountId);
    ret << ", tokenId: " << std::to_string(tokenId);
    ret << ", peerUdids: " << GetAnonyStringList(peerUdids);
    ret << ", peerUdid: " << GetAnonyString(peerUdid);
    ret << ", accountName: " << GetAnonyString(accountName);
    ret << ", syncUserIdFlag: " << std::to_string(syncUserIdFlag);
    ret << ", userIds: " << GetUserIdInfoList(userIdInfos);
    ret << ", broadCastId: " << std::to_string(broadCastId) << " }";
    return ret.str();
}

const std::string UserIdInfo::ToString() const
{
    std::ostringstream ret;
    ret << "{ " << std::to_string(this->isForeground);
    ret << ", userId: " << std::to_string(this->userId) << " }";
    return ret.str();
}

const std::string GetUserIdInfoList(const std::vector<UserIdInfo> &list)
{
    std::string temp = "[ ";
    bool flag = false;
    for (auto const &userId : list) {
        temp += userId.ToString() + PRINT_LIST_SPLIT;
        flag = true;
    }
    if (flag) {
        temp.erase(temp.length() - LIST_SPLIT_LEN);
    }
    temp += " ]";
    return temp;
}

void GetFrontAndBackUserIdInfos(const std::vector<UserIdInfo> &remoteUserIdInfos,
    std::vector<UserIdInfo> &foregroundUserIdInfos, std::vector<UserIdInfo> &backgroundUserIdInfos)
{
    foregroundUserIdInfos.clear();
    backgroundUserIdInfos.clear();
    for (auto const &u : remoteUserIdInfos) {
        if (u.isForeground) {
            foregroundUserIdInfos.push_back(u);
        } else {
            backgroundUserIdInfos.push_back(u);
        }
    }
}
} // DistributedHardware
} // OHOS