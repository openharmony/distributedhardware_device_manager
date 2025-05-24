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

#include "dm_transport_msg.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const int32_t MAX_USER_ID_NUM = 5;
const int32_t MAX_BACKGROUND_USER_ID_NUM = 5;
const char* const FOREGROUND_USERIDS_MSG_USERIDS_KEY = "foregroundUserIds";
const char* const BACKGROUND_USERIDS_MSG_USERIDS_KEY = "backgroundUserIds";
const char* const IS_NEW_EVENT_KEY = "isNewEvent";
const char* const COMM_MSG_CODE_KEY = "code";
const char* const COMM_MSG_MSG_KEY = "msg";
const char* const DSOFTBUS_NOTIFY_USERIDS_UDIDKEY = "remoteUdid";
const char* const DSOFTBUS_NOTIFY_USERIDS_USERIDKEY = "foregroundUserIds";
const char* const DSOFTBUS_NOTIFY_ACCOUNTID_KEY = "accountId";
const char* const DSOFTBUS_NOTIFY_USERID_KEY = "userId";
const char* const DSOFTBUS_NOTIFY_TOKENID_KEY = "tokenId";
const char* const DSOFTBUS_NOTIFY_EXTRA_KEY = "extra";
const char* const DSOFTBUS_NOTIFY_UDID_KEY = "udid";
}
void ToJson(cJSON *jsonObject, const UserIdsMsg &userIdsMsg)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *foregroundUserIdArr = cJSON_CreateArray();
    if (foregroundUserIdArr == nullptr) {
        return;
    }
    cJSON *numberObj = nullptr;
    for (auto const &userId : userIdsMsg.foregroundUserIds) {
        numberObj = cJSON_CreateNumber(userId);
        if (numberObj == nullptr || !cJSON_AddItemToArray(foregroundUserIdArr, numberObj)) {
            cJSON_Delete(numberObj);
            cJSON_Delete(foregroundUserIdArr);
            return;
        }
    }
    cJSON_AddItemToObject(jsonObject, FOREGROUND_USERIDS_MSG_USERIDS_KEY, foregroundUserIdArr);

    cJSON *backgroundUserIdArr = cJSON_CreateArray();
    if (backgroundUserIdArr == nullptr) {
        return;
    }
    cJSON *backgroundNumberObj = nullptr;
    for (auto const &userId : userIdsMsg.backgroundUserIds) {
        backgroundNumberObj = cJSON_CreateNumber(userId);
        if (backgroundNumberObj == nullptr || !cJSON_AddItemToArray(backgroundUserIdArr, backgroundNumberObj)) {
            cJSON_Delete(backgroundNumberObj);
            cJSON_Delete(backgroundUserIdArr);
            return;
        }
    }
    cJSON_AddItemToObject(jsonObject, BACKGROUND_USERIDS_MSG_USERIDS_KEY, backgroundUserIdArr);
    cJSON_AddBoolToObject(jsonObject, IS_NEW_EVENT_KEY, userIdsMsg.isNewEvent);
}

void FromJson(const cJSON *jsonObject, UserIdsMsg &userIdsMsg)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *foregroundUserIdsArr = cJSON_GetObjectItem(jsonObject, FOREGROUND_USERIDS_MSG_USERIDS_KEY);
    if (cJSON_IsArray(foregroundUserIdsArr)) {
        int32_t arrSize = cJSON_GetArraySize(foregroundUserIdsArr);
        if (arrSize > MAX_USER_ID_NUM) {
            LOGE("Receive too many foreground userids, %{public}d", arrSize);
            return;
        }
        for (int32_t i = 0; i < arrSize; i++) {
            cJSON *userIdItem = cJSON_GetArrayItem(foregroundUserIdsArr, i);
            if (cJSON_IsNumber(userIdItem)) {
                uint32_t userId = static_cast<uint32_t>(userIdItem->valueint);
                userIdsMsg.foregroundUserIds.push_back(userId);
            }
        }
    }

    cJSON *backgroundUserIdsArr = cJSON_GetObjectItem(jsonObject, BACKGROUND_USERIDS_MSG_USERIDS_KEY);
    if (cJSON_IsArray(backgroundUserIdsArr)) {
        int32_t arrSize = cJSON_GetArraySize(backgroundUserIdsArr);
        if (arrSize > MAX_BACKGROUND_USER_ID_NUM) {
            LOGE("Receive too many background userids, %{public}d", arrSize);
            return;
        }
        for (int32_t i = 0; i < arrSize; i++) {
            cJSON *userIdItem = cJSON_GetArrayItem(backgroundUserIdsArr, i);
            if (cJSON_IsNumber(userIdItem)) {
                uint32_t userId = static_cast<uint32_t>(userIdItem->valueint);
                userIdsMsg.backgroundUserIds.push_back(userId);
            }
        }
    }
    cJSON *isNewEventJson = cJSON_GetObjectItem(jsonObject, IS_NEW_EVENT_KEY);
    if (cJSON_IsBool(isNewEventJson)) {
        userIdsMsg.isNewEvent = (isNewEventJson->valueint != 0);
    }
}

void ToJson(cJSON *jsonObject, const CommMsg &commMsg)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, commMsg.code);
    const char *msg = commMsg.msg.c_str();
    cJSON_AddStringToObject(jsonObject, COMM_MSG_MSG_KEY, msg);
}

void FromJson(const cJSON *jsonObject, CommMsg &commMsg)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *codeObj = cJSON_GetObjectItem(jsonObject, COMM_MSG_CODE_KEY);
    if (cJSON_IsNumber(codeObj)) {
        commMsg.code = codeObj->valueint;
    }

    cJSON *msgObj = cJSON_GetObjectItem(jsonObject, COMM_MSG_MSG_KEY);
    if (cJSON_IsString(msgObj)) {
        commMsg.msg = msgObj->valuestring;
    }
}

std::string GetCommMsgString(const CommMsg &commMsg)
{
    cJSON *rootMsg = cJSON_CreateObject();
    if (rootMsg == nullptr) {
        LOGE("Create cJSON object failed.");
        return "";
    }
    ToJson(rootMsg, commMsg);
    char *msg = cJSON_PrintUnformatted(rootMsg);
    if (msg == nullptr) {
        cJSON_Delete(rootMsg);
        return "";
    }
    std::string msgStr = std::string(msg);
    cJSON_free(msg);
    cJSON_Delete(rootMsg);

    return msgStr;
}

void ToJson(cJSON *jsonObject, const NotifyUserIds &notifyUserIds)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }

    cJSON_AddStringToObject(jsonObject, DSOFTBUS_NOTIFY_USERIDS_UDIDKEY, notifyUserIds.remoteUdid.c_str());

    cJSON *userIdArr = cJSON_CreateArray();
    if (userIdArr == nullptr) {
        return;
    }
    cJSON *userIdNumberObj = nullptr;
    for (auto const &userId : notifyUserIds.userIds) {
        userIdNumberObj = cJSON_CreateNumber(userId);
        if (userIdNumberObj == nullptr || !cJSON_AddItemToArray(userIdArr, userIdNumberObj)) {
            cJSON_Delete(userIdNumberObj);
            cJSON_Delete(userIdArr);
            return;
        }
    }
    cJSON_AddItemToObject(jsonObject, DSOFTBUS_NOTIFY_USERIDS_USERIDKEY, userIdArr);
}

void FromJson(const cJSON *jsonObject, NotifyUserIds &notifyUserIds)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }

    cJSON *msgObj = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_USERIDS_UDIDKEY);
    if (cJSON_IsString(msgObj)) {
        notifyUserIds.remoteUdid = msgObj->valuestring;
    }

    cJSON *userIdsArr = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_USERIDS_USERIDKEY);
    if (cJSON_IsArray(userIdsArr)) {
        int32_t arrSize = cJSON_GetArraySize(userIdsArr);
        if (arrSize > MAX_USER_ID_NUM) {
            LOGE("Receive too many userids, %{public}d", arrSize);
            return;
        }
        for (int32_t i = 0; i < arrSize; i++) {
            cJSON *userIdItem = cJSON_GetArrayItem(userIdsArr, i);
            if (cJSON_IsNumber(userIdItem)) {
                uint32_t userId = static_cast<uint32_t>(userIdItem->valueint);
                notifyUserIds.userIds.push_back(userId);
            }
        }
    }
}

std::string NotifyUserIds::ToString()
{
    cJSON *msg = cJSON_CreateObject();
    if (msg == NULL) {
        LOGE("failed to create cjson object");
        return "";
    }

    ToJson(msg, *this);
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

void ToJson(cJSON *jsonObject, const LogoutAccountMsg &accountInfo)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }

    cJSON_AddStringToObject(jsonObject, DSOFTBUS_NOTIFY_ACCOUNTID_KEY, accountInfo.accountId.c_str());
    cJSON_AddNumberToObject(jsonObject, DSOFTBUS_NOTIFY_USERID_KEY, accountInfo.userId);
}

void FromJson(const cJSON *jsonObject, LogoutAccountMsg &accountInfo)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *accountIdObj = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_ACCOUNTID_KEY);
    if (cJSON_IsString(accountIdObj)) {
        accountInfo.accountId = accountIdObj->valuestring;
    }

    cJSON *userIdObj = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_USERID_KEY);
    if (cJSON_IsNumber(userIdObj)) {
        accountInfo.userId = userIdObj->valueint;
    }
}

void ToJson(cJSON *jsonObject, const UninstAppMsg &uninstAppMsg)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }

    cJSON_AddNumberToObject(jsonObject, DSOFTBUS_NOTIFY_USERID_KEY, uninstAppMsg.userId_);
    cJSON_AddNumberToObject(jsonObject, DSOFTBUS_NOTIFY_TOKENID_KEY, uninstAppMsg.tokenId_);
}

void FromJson(const cJSON *jsonObject, UninstAppMsg &uninstAppMsg)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }

    cJSON *userIdObj = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_USERID_KEY);
    if (cJSON_IsNumber(userIdObj)) {
        uninstAppMsg.userId_ = userIdObj->valueint;
    }
    cJSON *tokenIdObj = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_TOKENID_KEY);
    if (cJSON_IsNumber(tokenIdObj)) {
        uninstAppMsg.tokenId_ = tokenIdObj->valueint;
    }
}

void ToJson(cJSON *jsonObject, const UnBindAppMsg &unBindAppMsg)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }

    cJSON_AddNumberToObject(jsonObject, DSOFTBUS_NOTIFY_USERID_KEY, unBindAppMsg.userId_);
    cJSON_AddNumberToObject(jsonObject, DSOFTBUS_NOTIFY_TOKENID_KEY, unBindAppMsg.tokenId_);
    cJSON_AddStringToObject(jsonObject, DSOFTBUS_NOTIFY_EXTRA_KEY, unBindAppMsg.extra_.c_str());
    cJSON_AddStringToObject(jsonObject, DSOFTBUS_NOTIFY_UDID_KEY, unBindAppMsg.udid_.c_str());
}

void FromJson(const cJSON *jsonObject, UnBindAppMsg &unBindAppMsg)
{
    if (jsonObject == nullptr) {
        LOGE("Json pointer is nullptr!");
        return;
    }

    cJSON *userIdObj = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_USERID_KEY);
    if (cJSON_IsNumber(userIdObj)) {
        unBindAppMsg.userId_ = userIdObj->valueint;
    }
    cJSON *tokenIdObj = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_TOKENID_KEY);
    if (cJSON_IsNumber(tokenIdObj)) {
        unBindAppMsg.tokenId_ = tokenIdObj->valueint;
    }
    cJSON *extraObj = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_EXTRA_KEY);
    if (cJSON_IsString(extraObj)) {
        unBindAppMsg.extra_ = extraObj->valuestring;
    }
    cJSON *udidObj = cJSON_GetObjectItem(jsonObject, DSOFTBUS_NOTIFY_UDID_KEY);
    if (cJSON_IsString(udidObj)) {
        unBindAppMsg.udid_ = udidObj->valuestring;
    }
}
} // DistributedHardware
} // OHOS