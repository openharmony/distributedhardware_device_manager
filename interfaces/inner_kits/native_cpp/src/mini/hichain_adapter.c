/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "hichain_adapter.h"

#include "cJSON.h"
#include "device_auth.h"
#include "device_auth_defines.h"
#include "device_manager_impl_lite_m.h"
#include "los_compiler.h"
#include "los_config.h"
#include "los_sem.h"
#include "parameter.h"
#include "securec.h"

#define HICHAIN_DELAY_TICK_COUNT (10 * LOSCFG_BASE_CORE_TICK_PER_SECOND)  // delay 10s

static const UINT32 HICHAIN_SEM_INIT_COUNT = 0;

static const char * const HICHAIN_PKG_NAME = "com.ohos.devicemanager";
static const char * const FILED_GROUP_TYPE = "groupType";

static const int GROUP_TYPE_INVALID_GROUP = -1;
static const int GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP = 1;
static const int GROUP_TYPE_PEER_TO_PEER_GROUP = 256;
static const int GROUP_TYPE_ACROSS_ACCOUNT_GROUP = 1282;
static const int AUTH_FORM_IDENTICAL_ACCOUNT_GROUP = 1;
static const int AUTH_FORM_PEER_TO_PEER_GROUP = 0;
static const int AUTH_FORM_ACROSS_ACCOUNT_GROUP = 2;

static const DeviceGroupManager *g_deviceGroupManager = NULL;
static const GroupAuthManager *g_groupAuthManager = NULL;

static UINT32 g_createGroupSem = LOSCFG_BASE_IPC_SEM_LIMIT;

static void OnFinish(int64_t requestId, int operationCode, const char *returnData);
static void OnError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn);

static int ParseReturnJson(const char *returnJsonStr, int authForm);

static DeviceAuthCallback g_deviceAuthCallback = {
    .onTransmit = NULL,
    .onSessionKeyReturned = NULL,
    .onFinish = OnFinish,
    .onError = OnError,
    .onRequest = NULL,
};

int InitHichainModle(void)
{
    int retValue = DM_OK;
    int ret = InitDeviceAuthService();
    if (ret != HC_SUCCESS) {
        DMLOGE("failed to init device auth service with ret: %d.", ret);
        return ERR_DM_HICHAIN_INIT_DEVICE_AUTH_SERVICE;
    }
    g_deviceGroupManager = GetGmInstance();
    g_groupAuthManager = GetGaInstance();
    if (g_deviceGroupManager == NULL || g_groupAuthManager == NULL) {
        DMLOGE("failed to get Gm or Ga instance from hichain.");
        DestroyDeviceAuthService();
        return ERR_DM_HICHAIN_GET_GM_INSTANCE;
    }
    
    ret = g_deviceGroupManager->regCallback(HICHAIN_PKG_NAME, &g_deviceAuthCallback);
    if (ret != HC_SUCCESS) {
        DMLOGE("failed to register callback function to hichain with ret: %d.", ret);
        retValue = ERR_DM_HICHAIN_REGISTER_CALLBACK;
    }
    UINT32 osRet = LOS_BinarySemCreate(HICHAIN_SEM_INIT_COUNT, &g_createGroupSem);
    if (osRet != LOS_OK) {
        DMLOGE("failed to create group semaphore with ret: %d.", ret);
        retValue = ERR_DM_LITEOS_CREATE_MUTEX_OR_SEM;
    }
    if (retValue != DM_OK) {
        DMLOGE("failed to init hichain modle with retValue: %d.", retValue);
        UnInitHichainModle();
        return retValue;
    }
    DMLOGI("init hichain modle successfully.");
    return DM_OK;
}

int UnInitHichainModle(void)
{
    if (g_deviceGroupManager == NULL) {
        DMLOGE("g_deviceGroupManager is NULL.");
        return ERR_DM_POINT_NULL;
    }
    int retValue = g_deviceGroupManager->unRegCallback(HICHAIN_PKG_NAME);
    if (retValue != HC_SUCCESS) {
        DMLOGE("failed to unregister callback function to hichain with ret: %d.", retValue);
        retValue = ERR_DM_HICHAIN_UNREGISTER_CALLBACK;
    } else {
        retValue = DM_OK;
    }
    DestroyDeviceAuthService();
    g_deviceGroupManager = NULL;

    UINT32 osRet = LOS_SemDelete(g_createGroupSem);
    if (osRet != LOS_OK && osRet != LOS_ERRNO_SEM_INVALID) {
        DMLOGE("failed to delete group semaphore with ret: %d.", osRet);
        retValue = ERR_DM_LITEOS_DELETE_MUTEX_OR_SEM;
    } else {
        g_createGroupSem = LOSCFG_BASE_IPC_SEM_LIMIT;
    }
    if (retValue != DM_OK) {
        DMLOGE("failed to uninit hichain modle with retValue: %d.", retValue);
        return retValue;
    }
    DMLOGI("uninit hichain modle successfully.");
    return DM_OK;
}

int ParseReturnJson(const char *returnJsonStr, int authForm) // hichain groupType convert dm authForm
{
    if (returnJsonStr == NULL) {
        DMLOGE("return json str is null.");
        return ERR_DM_HICHAIN_FAILED;
    }
    cJSON *groupInfos = cJSON_Parse(returnJsonStr);
    if (groupInfos == NULL) {
        DMLOGE("parse return json str failed.");
        cJSON_Delete(groupInfos);
        return ERR_DM_CJSON_PARSE_STRING;
    }
    cJSON *groupInfo = NULL;
    int groupType = -1;
    cJSON_ArrayForEach(groupInfo, groupInfos) {
        cJSON *object = cJSON_GetObjectItem(groupInfo, FILED_GROUP_TYPE);
        if (object != NULL && cJSON_IsNumber(object)) {
            groupType = object->valueint;
            DMLOGI("parse group type is: %d.", authForm);
            break;
        }
    }
    cJSON_Delete(groupInfos);

    if (groupType == GROUP_TYPE_INVALID_GROUP) {
        authForm = GROUP_TYPE_INVALID_GROUP;
    }
    if (groupType == GROUP_TYPE_PEER_TO_PEER_GROUP) {
        authForm = AUTH_FORM_PEER_TO_PEER_GROUP;
    }
    if (groupType == GROUP_TYPE_IDENTICAL_ACCOUNT_GROUP) {
        authForm = AUTH_FORM_IDENTICAL_ACCOUNT_GROUP;
    }
    if (groupType == GROUP_TYPE_ACROSS_ACCOUNT_GROUP) {
        authForm = AUTH_FORM_ACROSS_ACCOUNT_GROUP;
    }
    return DM_OK;
}

int GetAuthFormByDeviceId(const char *deviceId, int authForm)
{
    int resultFlag = ERR_DM_FAILED;
    char *returnJsonStr = NULL;
    uint32_t groupNumber = 0;
    int userId = 0;
    authForm = -1;

    if (g_deviceGroupManager == NULL) {
        DMLOGE("g_deviceGroupManager is NULL.");
        return resultFlag;
    }

    do {
        int ret = g_deviceGroupManager->getRelatedGroups(userId, HICHAIN_PKG_NAME, deviceId, &returnJsonStr,
            &groupNumber);
        if (ret != HC_SUCCESS) {
            DMLOGE("failed to get related group ret: %d.", ret);
            resultFlag = ERR_DM_HICHAIN_FAILED;
            break;
        }
        if (groupNumber == 0) {
            DMLOGE("get related groups number is zero.");
            resultFlag = ERR_DM_HICHAIN_FAILED;
            break;
        }
        DMLOGI("get related groups size %ud.", groupNumber);
        ret = ParseReturnJson(returnJsonStr, authForm);
        if (ret != DM_OK) {
            DMLOGE("parse group infomation failed.");
            resultFlag = ret;
            break;
        }
        resultFlag = DM_OK;
    } while (false);

    g_deviceGroupManager->destroyInfo(&returnJsonStr);
    return resultFlag;
}

static void OnFinish(int64_t requestId, int operationCode, const char *returnData)
{
    (void)returnData;
    DMLOGI("OnFinish callback complete with requestId: %lld, operation: %d.", requestId, operationCode);
}

static void OnError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    (void)errorReturn;
    DMLOGI("OnError callback complete with requestId: %lld, operation: %d, errorCode: %d.",
        requestId, operationCode, errorCode);
}