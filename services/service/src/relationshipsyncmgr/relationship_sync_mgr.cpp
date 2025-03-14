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
#include "dm_anonymous.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
    DM_IMPLEMENT_SINGLE_INSTANCE(ReleationShipSyncMgr);
namespace {
    /**
     * @brief account logout payload length 8 bytes
     * |      2 bytes         |         6 bytes          |
     * | userid lower 2 bytes | account id first 6 bytes |
     */
    const int32_t ACCOUNT_LOGOUT_PAYLOAD_LEN = 8;
    /**
     * @brief device unbind payload length 2 bytes
     * |      2 bytes         |
     * | userid lower 2 bytes |
     */
    const int32_t DEVICE_UNBIND_PAYLOAD_LEN = 2;
    /**
     * @brief app unbind payload length 6 bytes
     * |      2 bytes         |         4 bytes          |
     * | userid lower 2 bytes |  token id lower 4 bytes  |
     */
    const int32_t APP_UNBIND_PAYLOAD_LEN = 10;
    /**
     * @brief delete user payload length 2 bytes
     * |      2 bytes         |
     * | userid lower 2 bytes |
     */
    const int32_t DEL_USER_PAYLOAD_LEN = 2;
    const int32_t STOP_USER_PAYLOAD_LEN = 2;
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

    const char * const MSG_TYPE = "TYPE";
    const char * const MSG_VALUE = "VALUE";
    const char * const MSG_PEER_UDID = "PEER_UDID";
    const char * const MSG_ACCOUNTID = "ACCOUNTID";
    const char * const MSG_PEER_TOKENID = "PEER_TOKENID";

    // The need response mask offset, the 8th bit.
    const int32_t NEED_RSP_MASK_OFFSET = 7;
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
    const uint32_t MAX_USER_ID_NUM = 5;
}

RelationShipChangeMsg::RelationShipChangeMsg() : type(RelationShipChangeType::TYPE_MAX),
    userId(UINT32_MAX), accountId(""), tokenId(UINT64_MAX), peerUdids({}), peerUdid(""), accountName(""),
    syncUserIdFlag(false), userIdInfos({})
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
        default:
            LOGE("RelationShipChange type invalid");
            break;
    }
    return ret;
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
        case RelationShipChangeType::SYNC_USERID:
            ret = FromSyncFrontOrBackUserIdPayLoad(payloadJson);
            break;
        case RelationShipChangeType::DEL_USER:
            ret = FromDelUserPayLoad(payloadJson);
            break;
        case RelationShipChangeType::STOP_USER:
            ret = FromStopUserPayLoad(payloadJson);
            break;
        default:
            LOGE("RelationShipChange type invalid");
            break;
    }
    return ret;
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
        case RelationShipChangeType::SERVICE_UNBIND:
        case RelationShipChangeType::APP_UNINSTALL:
            // current NOT support
            ret = false;
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
        (type == RelationShipChangeType::DEL_USER) || (type == RelationShipChangeType::STOP_USER);
}

bool RelationShipChangeMsg::IsChangeTypeValid(uint32_t type)
{
    return (type == (uint32_t)RelationShipChangeType::ACCOUNT_LOGOUT) ||
        (type == (uint32_t)RelationShipChangeType::DEVICE_UNBIND) ||
        (type == (uint32_t)RelationShipChangeType::APP_UNBIND) ||
        (type == (uint32_t)RelationShipChangeType::SYNC_USERID) ||
        (type == (uint32_t)RelationShipChangeType::DEL_USER) ||
        (type == (uint32_t)RelationShipChangeType::STOP_USER);
}

void RelationShipChangeMsg::ToAccountLogoutPayLoad(uint8_t *&msg, uint32_t &len) const
{
    msg = new uint8_t[ACCOUNT_LOGOUT_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
    }

    for (int j = USERID_PAYLOAD_LEN; j < ACCOUNT_LOGOUT_PAYLOAD_LEN; j++) {
        msg[j] = accountId[j - USERID_PAYLOAD_LEN];
    }
    len = ACCOUNT_LOGOUT_PAYLOAD_LEN;
}

void RelationShipChangeMsg::ToDeviceUnbindPayLoad(uint8_t *&msg, uint32_t &len) const
{
    msg = new uint8_t[DEVICE_UNBIND_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
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

    for (int i = TOKENID_PAYLOAD_LEN; i < APP_UNBIND_PAYLOAD_LEN; i++) {
        msg[i] |= (peerTokenId >> ((i - TOKENID_PAYLOAD_LEN) * BITS_PER_BYTE)) & 0xFF;
    }

    len = APP_UNBIND_PAYLOAD_LEN;
}

bool RelationShipChangeMsg::ToSyncFrontOrBackUserIdPayLoad(uint8_t *&msg, uint32_t &len) const
{
    uint32_t userIdNum = static_cast<uint32_t>(userIdInfos.size());
    if (userIdNum > MAX_USER_ID_NUM) {
        LOGE("userIdNum too many, %{public}u", userIdNum);
        return false;
    }

    len = userIdNum * USERID_BYTES + 1;

    if (len > MAX_MEM_MALLOC_SIZE) {
        LOGE("len too long");
        return false;
    }
    msg = new uint8_t[len]();
    if (syncUserIdFlag) {
        msg[0] |= 0x1 << NEED_RSP_MASK_OFFSET;
    } else {
        msg[0] |= 0x0 << NEED_RSP_MASK_OFFSET;
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
    return true;
}

void RelationShipChangeMsg::ToDelUserPayLoad(uint8_t *&msg, uint32_t &len) const
{
    len = DEL_USER_PAYLOAD_LEN;
    msg = new uint8_t[DEL_USER_PAYLOAD_LEN]();
    for (int i = 0; i < DEL_USER_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
    }
}

void RelationShipChangeMsg::ToStopUserPayLoad(uint8_t *&msg, uint32_t &len) const
{
    len = STOP_USER_PAYLOAD_LEN;
    msg = new uint8_t[STOP_USER_PAYLOAD_LEN]();
    for (int i = 0; i < STOP_USER_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BITS_PER_BYTE)) & 0xFF;
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
    for (uint32_t j = USERID_PAYLOAD_LEN; j < ACCOUNT_LOGOUT_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            accountId += static_cast<char>(payloadItem->valueint);
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
    for (uint32_t j = TOKENID_PAYLOAD_LEN; j < APP_UNBIND_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            peerTokenId |= (static_cast<uint8_t>(payloadItem->valueint)) <<
                ((j - TOKENID_PAYLOAD_LEN) * BITS_PER_BYTE);
        }
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
        userIdNum = ((static_cast<uint8_t>(payloadItem->valueint)) & FOREGROUND_USERID_LEN_MASK);
    }

    int32_t effectiveLen = static_cast<int32_t>(userIdNum * USERID_BYTES + 1);
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
    return true;
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
    if (arrayObj == nullptr) {
        LOGE("ArrayObj is nullptr.");
        cJSON_Delete(msg);
        return "";
    }
    cJSON_AddItemToObject(msg, MSG_VALUE, arrayObj);

    cJSON *udidArrayObj = cJSON_CreateArray();
    if (udidArrayObj == nullptr) {
        LOGE("cJSON_CreateArray failed");
        cJSON_Delete(msg);
        return "";
    }
    cJSON *udidStringObj = nullptr;
    for (uint32_t index = 0; index < peerUdids.size(); index++) {
        udidStringObj = cJSON_CreateString(peerUdids[index].c_str());
        if (udidStringObj == nullptr || !cJSON_AddItemToArray(udidArrayObj, udidStringObj)) {
            cJSON_Delete(udidStringObj);
            cJSON_Delete(udidArrayObj);
            cJSON_Delete(msg);
            return "";
        }
    }
    cJSON_AddItemToObject(msg, MSG_PEER_UDID, udidArrayObj);
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

std::string ReleationShipSyncMgr::SyncTrustRelationShip(const RelationShipChangeMsg &msg)
{
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

const std::string RelationShipChangeMsg::ToString() const
{
    std::string ret;
    ret += "{ MsgType: " + std::to_string(static_cast<uint32_t>(type));
    ret += ", userId: " + std::to_string(userId);
    ret += ", accountId: " + GetAnonyString(accountId);
    ret += ", tokenId: " + std::to_string(tokenId);
    ret += ", peerUdids: " + GetAnonyStringList(peerUdids);
    ret += ", peerUdid: " + GetAnonyString(peerUdid);
    ret += ", accountName: " + GetAnonyString(accountName);
    ret += ", syncUserIdFlag: " + std::to_string(syncUserIdFlag);
    ret += ", userIds: " + GetUserIdInfoList(userIdInfos) + " }";
    return ret;
}

const std::string UserIdInfo::ToString() const
{
    std::string ret;
    ret += "{ " + std::to_string(this->isForeground);
    ret += ", userId: " + std::to_string(this->userId) + " }";
    return ret;
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