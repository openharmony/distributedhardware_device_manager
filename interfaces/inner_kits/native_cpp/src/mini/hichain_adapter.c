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
static const char * const DM_GROUP_NAME = "devicemanager";
static const char * const FIELD_CREDENTIAL_EXISTS = "isCredentialExists";

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

// It is only the id of a group operator request an has no security risk.
static const int64_t CREATE_GROUP_REQUESTID = 159357462;
static bool g_createGroupFlag = true;      // strict operating timings do not require lock protection

// maximum length of JSON character string input for credential operation
static const UINT32 DM_MAX_REQUEST_JSON_LEN = 4096;

static int DeleteCredentialAndGroup(char **returnJsonStr);
static int ImportCredentialAndGroup(const char *reqJsonStr, char **returnJsonStr);
static int GetUserId(const cJSON *const root, char *userId);
static int CreateGroupJsonAddObject(const cJSON *const root, cJSON **jsonRoot);
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
        DMLOGI("get related groups size %u.", groupNumber);
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
#ifdef MINE_HARMONY
    DMLOGI("start to notify the asynchronous operation group of the successful result.");
    if (requestId == CREATE_GROUP_REQUESTID) {
        g_createGroupFlag = true;
        UINT32 osRet = LOS_SemPost(g_createGroupSem);
        if (osRet != LOS_OK) {
            DMLOGE("failed to post create group semaphore with ret: %u.", osRet);
            return;
        }
    }
#endif
}

static void OnError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    (void)errorReturn;
    DMLOGI("OnError callback complete with requestId: %lld, operation: %d, errorCode: %d.",
        requestId, operationCode, errorCode);
#ifdef MINE_HARMONY
    if (requestId == CREATE_GROUP_REQUESTID) {
        g_createGroupFlag = false;
        UINT32 osRet = LOS_SemPost(g_createGroupSem);
        if (osRet != LOS_OK) {
            DMLOGE("failed to post create group semaphore with ret: %u.", osRet);
            return;
        }
    }
#endif
}

int RequestHichainCredential(char **returnJsonStr)
{
    if (returnJsonStr == NULL || g_deviceGroupManager == NULL) {
        DMLOGE("returnJsonStr or g_deviceGroupManager is NULL.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }

    DMLOGI("start to request hichain credential.");
#ifdef MINE_HARMONY
    int retValue = g_deviceGroupManager->getRegisterInfo(returnJsonStr);
    if (retValue != HC_SUCCESS || returnJsonStr == NULL || *returnJsonStr == NULL) {
        DMLOGE("failed to request hichain credential from hichain with ret: %d.", retValue);
        return ERR_DM_HICHAIN_GET_CREDENTIAL_INFO;
    }
    DMLOGI("request device credential info from hichain successfully with len: %u.", strlen(*returnJsonStr));
#else
    DMLOGI("request device credential info from hichain successfully.");
#endif
    return DM_OK;
}

int CheckHichainCredential(const char *reqJsonStr, char **returnJsonStr)
{
    if (reqJsonStr == NULL || returnJsonStr == NULL || g_deviceGroupManager == NULL) {
        DMLOGE("reqJsonStr or returnJsonStr or g_deviceGroupManager is NULL.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }

    size_t checkJsonStrLen = strlen(reqJsonStr);
    if (checkJsonStrLen == 0 || checkJsonStrLen >= DM_MAX_REQUEST_JSON_LEN) {
        DMLOGE("reqJsonStr is not meet the requirements with checkJsonStrLen: %u.", checkJsonStrLen);
        return ERR_DM_INPUT_INVALID_VALUE;
    }

#ifdef MINE_HARMONY
    DMLOGI("start to check device credential info.");
    int retValue = g_deviceGroupManager->processCredential(CREDENTIAL_QUERY, reqJsonStr, returnJsonStr);
    if (retValue != HC_SUCCESS || returnJsonStr == NULL || *returnJsonStr == NULL) {
        DMLOGE("failed to check device credential info with ret: %d.", retValue);
        return ERR_DM_HICHAIN_GET_CREDENTIAL_INFO;
    }
    DMLOGI("check device credential info successfully with len: %u.", strlen(*returnJsonStr));
#else
    DMLOGI("check device credential info successfully.");
#endif
    return DM_OK;
}

int ImportHichainCredential(const char *reqJsonStr, char **returnJsonStr)
{
    if (reqJsonStr == NULL || returnJsonStr == NULL || g_deviceGroupManager == NULL) {
        DMLOGE("reqJsonStr or returnJsonStr or g_deviceGroupManager is NULL.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }

    size_t importJsonStrLen = strlen(reqJsonStr);
    if (importJsonStrLen == 0 || importJsonStrLen >= DM_MAX_REQUEST_JSON_LEN) {
        DMLOGE("reqJsonStr is not meet the requirements with importJsonStrLen: %u.", importJsonStrLen);
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    if (IsHichainCredentialExist()) {
        DMLOGE("the device has already exists credential.");
        return ERR_DM_HICHAIN_CREDENTIAL_EXISTS;
    }

    DMLOGI("start to import device credential info to hichain.");
    if (ImportCredentialAndGroup(reqJsonStr, returnJsonStr) != DM_OK) {
        DMLOGE("failed to import hichain credential and create group.");
        DeleteCredentialAndGroup(returnJsonStr);
        return ERR_DM_HICHAIN_IMPORT_CREDENTIAL;
    }
    DMLOGI("import device credential info to hichain successfully.");
    return DM_OK;
}

int DeleteHichainCredential(const char *reqJsonStr, char **returnJsonStr)
{
    if (reqJsonStr == NULL || returnJsonStr == NULL || g_deviceGroupManager == NULL) {
        DMLOGE("reqJsonStr or returnJsonStr or g_deviceGroupManager is NULL.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    size_t deleteJsonStrLen = strlen(reqJsonStr);
    if (deleteJsonStrLen == 0 || deleteJsonStrLen >= DM_MAX_REQUEST_JSON_LEN) {
        DMLOGE("reqJsonStr is not meet the requirements with deleteJsonStrLen: %u.", deleteJsonStrLen);
        return ERR_DM_INPUT_INVALID_VALUE;
    }

    DMLOGI("start to delete device credential info from hichain.");
    if (DeleteCredentialAndGroup(returnJsonStr) != DM_OK) {
        DMLOGE("failed to delete device credential from hichain.");
        return ERR_DM_HICHAIN_DELETE_CREDENTIAL;
    }
    DMLOGI("delete device credential info from hichain successfully.");
    return DM_OK;
}

void FreeHichainJsonStringMemory(char **jsonStr)
{
    if (jsonStr == NULL || *jsonStr == NULL || g_deviceGroupManager == NULL) {
        DMLOGE("jsonStr or *jsonStr or g_deviceGroupManager is NULL");
        return;
    }
    g_deviceGroupManager->destroyInfo(jsonStr);
}

// note: Internal interfaces ensure the validity of input parameters.
static int CreateGroupJsonAddObject(const cJSON *const root, cJSON **jsonRoot)
{
    cJSON *groupRoot = NULL;
    char deviceUdid[DM_MAX_DEVICE_UDID_LEN + 1] = {0};
    char userId[DM_MAX_USER_ID_LEN + 1] = {0};

    int retValue = GetDevUdid(deviceUdid, DM_MAX_DEVICE_UDID_LEN);
    if (retValue != 0) {
        DMLOGE("failed to get local device udid with ret: %d.", retValue);
        return ERR_DM_FAILED;
    }
    if (GetUserId(root, userId) != DM_OK) {
        DMLOGE("failed to get credential userId.");
        return ERR_DM_FAILED;
    }

    do {
        groupRoot = cJSON_CreateObject();
        if (groupRoot == NULL) {
            DMLOGE("failed to create cjson object.");
            break;
        }
        if (cJSON_AddStringToObject(groupRoot, FIELD_DEVICE_ID, deviceUdid) == NULL) {
            DMLOGE("failed to add groupid string to cjson object.");
            break;
        }
        if (cJSON_AddStringToObject(groupRoot, FIELD_GROUP_NAME, DM_GROUP_NAME) == NULL) {
            DMLOGE("failed to add group name string to cjson object.");
            break;
        }
        if (cJSON_AddNumberToObject(groupRoot, FIELD_GROUP_TYPE, IDENTICAL_ACCOUNT_GROUP) == NULL) {
            DMLOGE("failed to add groupType number to cjson object.");
            break;
        }
        if (cJSON_AddStringToObject(groupRoot, FIELD_USER_ID, (const char*)userId) == NULL) {
            DMLOGE("failed to add group userId string to cjson object.");
            break;
        }
        *jsonRoot = groupRoot;
        return DM_OK;
    } while (false);

    cJSON_Delete(groupRoot);
    return ERR_DM_FAILED;
}

// note: Internal interfaces ensure the validity of input parameters.
static int GetUserId(const cJSON * const root, char *userId)
{
    cJSON *object = cJSON_GetObjectItem(root, FIELD_USER_ID);
    if (object == NULL || !cJSON_IsString(object)) {
        DMLOGE("failed to get cjson object or type is not string.");
        return ERR_DM_FAILED;
    }
    size_t stringLen = strlen(object->valuestring);
    if (stringLen > DM_MAX_USER_ID_LEN) {
        DMLOGE("userId in the imported credential is too long with lenght: %u.", stringLen);
        return ERR_DM_FAILED;
    }
    for (size_t i = 0; i < stringLen; i++) {
        userId[i] = object->valuestring[i];
    }
    return DM_OK;
}

static int ImportCredentialAndGroup(const char *reqJsonStr, char **returnJsonStr)
{
    cJSON *root = NULL;
    cJSON *groupRoot = NULL;
    char *createParams = NULL;
    int retValue = ERR_DM_FAILED;
#ifdef MINE_HARMONY
    do {
        root = cJSON_Parse(reqJsonStr);
        if (root == NULL) {
            DMLOGE("failed to create cjson object.");
            break;
        }
        int ret = g_deviceGroupManager->processCredential(CREDENTIAL_SAVE, reqJsonStr, returnJsonStr);
        if (ret != HC_SUCCESS || returnJsonStr == NULL || *returnJsonStr == NULL) {
            DMLOGE("failed to import credential info to hichain with ret: %d.", ret);
            break;
        }
        if (CreateGroupJsonAddObject(root, &groupRoot) != DM_OK) {
            DMLOGE("failed to add object to cjson string.");
            break;
        }
        createParams = cJSON_Print(groupRoot);
        if (createParams == NULL) {
            DMLOGE("failed to print string from cjson object.");
            break;
        }
        ret = g_deviceGroupManager->createGroup(CREATE_GROUP_REQUESTID, HICHAIN_PKG_NAME, createParams);
        if (ret != HC_SUCCESS) {
            DMLOGE("failed to create hichain group with ret: %d.", ret);
            break;
        }
        UINT32 osRet = LOS_SemPend(g_createGroupSem, HICHAIN_DELAY_TICK_COUNT);
        if (osRet != LOS_OK || (osRet == LOS_OK && !g_createGroupFlag)) {
            DMLOGE("failed to acquire semaphore or create group with ret: %d or callback notify.", (int)osRet);
            break;
        }
        retValue = DM_OK;
    } while (false);

    cJSON_Delete(root);
    cJSON_Delete(groupRoot);
    cJSON_free(createParams);
    return retValue;
#else
    return DM_OK;
#endif
}

static int DeleteCredentialAndGroup(char **returnJsonStr)
{
    cJSON *root = NULL;
    char *deleteParams = NULL;
    int retValue = ERR_DM_FAILED;

#ifdef MINE_HARMONY
    do {
        root = cJSON_CreateObject();
        if (root == NULL) {
            DMLOGE("failed to create cjson object.");
            break;
        }
        if (cJSON_AddTrueToObject(root, FIELD_IS_DELETE_ALL) == NULL) {
            DMLOGE("failed to add delete all boolean to cjson object.");
            break;
        }
        deleteParams = cJSON_Print(root);
        if (deleteParams == NULL) {
            DMLOGE("failed to print string from cjson object.");
            break;
        }
        int ret = g_deviceGroupManager->processCredential(CREDENTIAL_CLEAR, deleteParams, returnJsonStr);
        if (ret != HC_SUCCESS || returnJsonStr == NULL || *returnJsonStr == NULL) {
            DMLOGE("failed to delete hichain credential with ret: %d.", ret);
            break;
        }
        retValue = DM_OK;
    } while (false);

    cJSON_Delete(root);
    cJSON_free(deleteParams);
    return retValue;
#else
    return DM_OK;
#endif
}

bool IsHichainCredentialExist(void)
{
    cJSON *root = NULL;
    bool resultFlag = false;
    char *reqJsonStr = "{\n}";
    char *returnJsonStr = NULL;

    if (g_deviceGroupManager == NULL) {
        DMLOGE("g_deviceGroupManager is NULL.");
        return resultFlag;
    }
#ifdef MINE_HARMONY
    do {
        int retValue = g_deviceGroupManager->processCredential(CREDENTIAL_QUERY, reqJsonStr, &returnJsonStr);
        if (retValue != HC_SUCCESS || returnJsonStr == NULL) {
            DMLOGE("failed to check device credential info with ret: %d.", retValue);
            break;
        }
        root = cJSON_Parse(returnJsonStr);
        if (root == NULL) {
            DMLOGE("failed to create cjson object.");
            break;
        }
        cJSON *object = cJSON_GetObjectItem(root, FIELD_CREDENTIAL_EXISTS);
        if (object == NULL) {
            DMLOGE("failed to get object from cjson object.");
            break;
        }
        if (object->type != cJSON_False && object->type != cJSON_True) {
            DMLOGE("object type is error with type: %d.", object->type);
            break;
        }
        if (object->type == cJSON_False) {
            DMLOGI("credential information does not exist on the current device.");
            break;
        }
        DMLOGI("credential information exist on the current device.");
        resultFlag = true;
    } while (false);

    cJSON_Delete(root);
    g_deviceGroupManager->destroyInfo(&returnJsonStr);
#endif
    return resultFlag;
}
