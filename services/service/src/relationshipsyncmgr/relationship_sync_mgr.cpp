/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <vector>
#include "dm_crypto.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
    DM_IMPLEMENT_SINGLE_INSTANCE(ReleationShipSyncMgr);
namespace {
    /**
     * @brief account logout payload length 8
     * |      2 bytes         |         6 bytes          |
     * | userid lower 2 bytes | account id first 6 bytes |
     */
    const int32_t ACCOUNT_LOGOUT_PAYLOAD_LEN = 8;
    /**
     * @brief device unbind payload length 2
     * |      2 bytes         |
     * | userid lower 2 bytes |
     */
    const int32_t DEVICE_UNBIND_PAYLOAD_LEN = 2;
    /**
     * @brief app unbind payload length 6
     * |      2 bytes         |         4 bytes          |
     * | userid lower 2 bytes |  token id lower 4 bytes  |
     */
    const int32_t APP_UNBIND_PAYLOAD_LEN = 6;
    const int32_t USERID_PAYLOAD_LEN = 2;
    const int32_t ACCOUNTID_PAYLOAD_LEN = 6;
    const int32_t BIT_PER_BYTES = 8;
    const int32_t INVALIED_PAYLOAD_SIZE = 12;

    const char * const MSG_TYPE = "TYPE";
    const char * const MSG_VALUE = "VALUE";
    const char * const MSG_PEER_UDID = "PEER_UDID";
    const char * const MSG_ACCOUNTID = "ACCOUNTID";
}

RelationShipChangeMsg::RelationShipChangeMsg() : type(RelationShipChangeType::TYPE_MAX),
    userId(UINT32_MAX), accountId(""), tokenId(UINT64_MAX), peerUdid("")
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
        case RelationShipChangeType::SERVICE_UNBIND:
        case RelationShipChangeType::DEL_USER:
        case RelationShipChangeType::APP_UNINSTALL:
            // current NOT support
            ret = false;
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
        (type == RelationShipChangeType::APP_UNBIND);
}

bool RelationShipChangeMsg::IsChangeTypeValid(uint32_t type)
{
    return (type == (uint32_t)RelationShipChangeType::ACCOUNT_LOGOUT) ||
        (type == (uint32_t)RelationShipChangeType::DEVICE_UNBIND) ||
        (type == (uint32_t)RelationShipChangeType::APP_UNBIND);
}

void RelationShipChangeMsg::ToAccountLogoutPayLoad(uint8_t *&msg, uint32_t &len) const
{
    msg = new uint8_t[ACCOUNT_LOGOUT_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BIT_PER_BYTES)) & 0xFF;
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
        msg[i] |= (userId >> (i * BIT_PER_BYTES)) & 0xFF;
    }
    len = DEVICE_UNBIND_PAYLOAD_LEN;
}

void RelationShipChangeMsg::ToAppUnbindPayLoad(uint8_t *&msg, uint32_t &len) const
{
    msg = new uint8_t[APP_UNBIND_PAYLOAD_LEN]();
    for (int i = 0; i < USERID_PAYLOAD_LEN; i++) {
        msg[i] |= (userId >> (i * BIT_PER_BYTES)) & 0xFF;
    }

    for (int i = USERID_PAYLOAD_LEN; i < APP_UNBIND_PAYLOAD_LEN; i++) {
        msg[i] |= (tokenId >> ((i - USERID_PAYLOAD_LEN) * BIT_PER_BYTES)) & 0xFF;
    }

    len = APP_UNBIND_PAYLOAD_LEN;
}

bool RelationShipChangeMsg::FromAccountLogoutPayLoad(const cJSON *payloadJson)
{
    if (payloadJson == NULL) {
        LOGE("Account logout payloadJson is null.");
        return false;
    }
    int32_t arraySize = cJSON_GetArraySize(payloadJson);
    if (arraySize < ACCOUNT_LOGOUT_PAYLOAD_LEN || arraySize > INVALIED_PAYLOAD_SIZE) {
        LOGE("Payload invalied,the size is %{public}d.", arraySize);
        return false;
    }
    userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BIT_PER_BYTES);
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
    if (arraySize < ACCOUNT_LOGOUT_PAYLOAD_LEN || arraySize > INVALIED_PAYLOAD_SIZE) {
        LOGE("Payload invalied,the size is %{public}d.", arraySize);
        return false;
    }
    userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BIT_PER_BYTES);
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
    if (arraySize < ACCOUNT_LOGOUT_PAYLOAD_LEN || arraySize > INVALIED_PAYLOAD_SIZE) {
        LOGE("Payload invalied,the size is %{public}d.", arraySize);
        return false;
    }
    userId = 0;
    for (uint32_t i = 0; i < USERID_PAYLOAD_LEN; i++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, i);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            userId |= (static_cast<uint8_t>(payloadItem->valueint)) << (i * BIT_PER_BYTES);
        }
    }
    tokenId = 0;
    for (uint32_t j = USERID_PAYLOAD_LEN; j < APP_UNBIND_PAYLOAD_LEN; j++) {
        cJSON *payloadItem = cJSON_GetArrayItem(payloadJson, j);
        CHECK_NULL_RETURN(payloadItem, false);
        if (cJSON_IsNumber(payloadItem)) {
            tokenId |= (static_cast<uint8_t>(payloadItem->valueint)) <<  ((j - USERID_PAYLOAD_LEN) * BIT_PER_BYTES);
        }
    }
    return true;
}

cJSON *RelationShipChangeMsg::ToArrayJson(cJSON *msg) const
{
    if (msg == nullptr) {
        LOGE("Msg is nullptr.");
        return nullptr;
    }
    uint8_t *payload = nullptr;
    uint32_t len = 0;
    if (!this->ToBroadcastPayLoad(payload, len)) {
        LOGE("Get broadcast payload failed");
        cJSON_Delete(msg);
        return nullptr;
    }
    cJSON *arrayObj = cJSON_CreateArray();
    if (arrayObj == nullptr) {
        LOGE("cJSON_CreateArray failed");
        cJSON_Delete(msg);
        return nullptr;
    }
    for (uint32_t index = 0; index < len; index++) {
        cJSON_AddItemToArray(arrayObj, cJSON_CreateNumber(payload[index]));
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
    cJSON *arrayObj = ToArrayJson(msg);
    if (arrayObj == nullptr) {
        LOGE("ArrayObj is nullptr.");
        return "";
    }
    cJSON_AddItemToObject(msg, MSG_VALUE, arrayObj);

    cJSON *udidArrayObj = cJSON_CreateArray();
    if (udidArrayObj == nullptr) {
        LOGE("cJSON_CreateArray failed");
        cJSON_Delete(msg);
        return "";
    }
    for (uint32_t index = 0; index < peerUdids.size(); index++) {
        cJSON_AddItemToArray(udidArrayObj, cJSON_CreateString(peerUdids[index].c_str()));
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
} // DistributedHardware
} // OHOS