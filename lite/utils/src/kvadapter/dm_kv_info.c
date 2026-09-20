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


#include "dm_kv_info.h"

#include "cJSON.h"
#include "dm_anonymous.h"

#define DM_UDID_HASH_KEY "udidHash"
#define DM_APP_ID_KEY "appID"
#define DM_ANOY_DEVICE_ID_KEY "anoyDeviceId"
#define DM_SALT_KEY "salt"
#define DM_LAST_MODIFY_TIME_KEY "lastModifyTime"

void DmConvertKvValueToJson(const DmKVValue* kvValue, DmString* result)
{
    cJSON* jsonObj = cJSON_CreateObject();
    if (jsonObj == NULL) {
        *result = DmStringCreateEmpty();
        return;
    }
    cJSON_AddStringToObject(jsonObj, DM_UDID_HASH_KEY, DmStringCstr(&kvValue->udidHash));
    cJSON_AddStringToObject(jsonObj, DM_APP_ID_KEY, DmStringCstr(&kvValue->appID));
    cJSON_AddStringToObject(jsonObj, DM_ANOY_DEVICE_ID_KEY, DmStringCstr(&kvValue->anoyDeviceId));
    cJSON_AddStringToObject(jsonObj, DM_SALT_KEY, DmStringCstr(&kvValue->salt));
    cJSON_AddNumberToObject(jsonObj, DM_LAST_MODIFY_TIME_KEY, (double)kvValue->lastModifyTime);
    char* jsonStr = cJSON_PrintUnformatted(jsonObj);
    if (jsonStr != NULL) {
        *result = DmStringCreate(jsonStr);
        cJSON_free(jsonStr);
    } else {
        *result = DmStringCreateEmpty();
    }
    cJSON_Delete(jsonObj);
}

void DmConvertJsonToKvValue(const DmString* result, DmKVValue* kvValue)
{
    if (DmStringEmpty(result)) {
        return;
    }
    cJSON* resultJson = cJSON_Parse(DmStringCstr(result));
    if (resultJson == NULL) {
        return;
    }
    cJSON* udidHashItem = cJSON_GetObjectItemCaseSensitive(resultJson, DM_UDID_HASH_KEY);
    if (udidHashItem != NULL && cJSON_IsString(udidHashItem)) {
        kvValue->udidHash = DmStringCreate(udidHashItem->valuestring);
    }
    cJSON* appIDItem = cJSON_GetObjectItemCaseSensitive(resultJson, DM_APP_ID_KEY);
    if (appIDItem != NULL && cJSON_IsString(appIDItem)) {
        kvValue->appID = DmStringCreate(appIDItem->valuestring);
    }
    cJSON* anoyDeviceIdItem = cJSON_GetObjectItemCaseSensitive(resultJson, DM_ANOY_DEVICE_ID_KEY);
    if (anoyDeviceIdItem != NULL && cJSON_IsString(anoyDeviceIdItem)) {
        kvValue->anoyDeviceId = DmStringCreate(anoyDeviceIdItem->valuestring);
    }
    cJSON* saltItem = cJSON_GetObjectItemCaseSensitive(resultJson, DM_SALT_KEY);
    if (saltItem != NULL && cJSON_IsString(saltItem)) {
        kvValue->salt = DmStringCreate(saltItem->valuestring);
    }
    cJSON* lastModifyTimeItem = cJSON_GetObjectItemCaseSensitive(resultJson, DM_LAST_MODIFY_TIME_KEY);
    if (lastModifyTimeItem != NULL && cJSON_IsNumber(lastModifyTimeItem)) {
        kvValue->lastModifyTime = (int64_t)lastModifyTimeItem->valuedouble;
    }
    cJSON_Delete(resultJson);
}
