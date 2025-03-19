/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "mine_hichain_connector.h"

#include <cstdlib>
#include <chrono>
#include <unistd.h>
#include <securec.h>

#include "device_auth.h"
#include "device_auth_defines.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "json_object.h"
#include "parameter.h"


namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DEVICE_UDID_LENGTH = 65;
constexpr int64_t CREATE_GROUP_REQUESTID = 159357462;
constexpr int32_t MAX_HICHAIN_DELAY_TIME = 10;

std::mutex g_createGroupMutex;
std::condition_variable g_createGroupNotify;

bool g_mineCreateGroupFlag = false;

const DeviceGroupManager *g_deviceGroupManager = nullptr;
DeviceAuthCallback g_deviceAuthCallback = {
    .onTransmit = nullptr,
    .onFinish = MineHiChainConnector::onFinish,
    .onError = MineHiChainConnector::onError,
    .onRequest = nullptr
};
MineHiChainConnector::MineHiChainConnector()
{
    Init();
    LOGI("HiChainConnector::constructor success.");
}

MineHiChainConnector::~MineHiChainConnector()
{
    UnInit();
    LOGI("HiChainConnector::destructor.");
}

void MineHiChainConnector::onFinish(int64_t requestId, int operationCode, const char *returnData)
{
    (void)returnData;
    LOGD("start to notify the asynchronous operation group of the successful result.");
    if (requestId == CREATE_GROUP_REQUESTID) {
        std::unique_lock<std::mutex> locker(g_createGroupMutex);
        g_mineCreateGroupFlag = true;
        g_createGroupNotify.notify_one();
        LOGI("Create group success");
    }
    LOGI("OnFinish callback complete with requestId: %{public}" PRId64 ", operation: %{public}d.", requestId,
        operationCode);
}

void MineHiChainConnector::onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn)
{
    (void)errorReturn;
    LOGD("start to notify the asynchronous operation group of the successful result.");
    if (requestId == CREATE_GROUP_REQUESTID) {
        std::unique_lock<std::mutex> locker(g_createGroupMutex);
        g_mineCreateGroupFlag = false;
        g_createGroupNotify.notify_one();
        LOGI("failed to create group");
    }
    LOGI("OnError callback complete with requestId: %{public}" PRId64 ", operation: %{public}d, errorCode: %{public}d.",
        requestId, operationCode, errorCode);
}

int32_t MineHiChainConnector::Init(void)
{
    int retValue = InitDeviceAuthService();
    if (retValue != HC_SUCCESS) {
        LOGE("failed to init device auth service with ret:%{public}d.", retValue);
        return ERR_DM_FAILED;
    }
    g_deviceGroupManager = GetGmInstance();
    if (g_deviceGroupManager == nullptr) {
        LOGE("failed to get Gm instance from hichain");
        return ERR_DM_FAILED;
    }
#if (defined(MINE_HARMONY))
    retValue = g_deviceGroupManager->unRegCallback(DM_PKG_NAME);
    if (retValue != HC_SUCCESS) {
        LOGE("failed to register callback function to hichain with ret:%{public}d.", retValue);
    }
    retValue = g_deviceGroupManager->regCallback(DM_PKG_NAME, &g_deviceAuthCallback);
    if (retValue != HC_SUCCESS) {
        LOGE("failed to register callback function to hichain with ret:%{public}d.", retValue);
        return ERR_DM_FAILED;
    }
#endif
    LOGI("init hichain modle successfully.");
    return DM_OK;
}


int32_t MineHiChainConnector::UnInit(void)
{
    if (g_deviceGroupManager == nullptr) {
        LOGE("g_deviceGroupManager os nullptr");
        return ERR_DM_POINT_NULL;
    }
#if (defined(MINE_HARMONY))
    if (g_deviceGroupManager->unRegCallback(DM_PKG_NAME) != HC_SUCCESS) {
        LOGE("failed to unregister callback to hichain");
        return ERR_DM_HICHAIN_UNREGISTER_CALLBACK;
    }
    DestroyDeviceAuthService();
#endif
    LOGI("uninit hichain modle successfully");
    return DM_OK;
}

int32_t MineHiChainConnector::DeleteCredentialAndGroup(void)
{
    JsonObject jsonObj;
    jsonObj[FIELD_IS_DELETE_ALL] = true;
    std::string params = SafetyDump(jsonObj);
#if (defined(MINE_HARMONY))
    char *returnInfo = nullptr;
    int32_t retValue = g_deviceGroupManager->processCredential(DELETE_SELF_CREDENTIAL, params.c_str(), &returnInfo);
    if (retValue != HC_SUCCESS) {
        LOGE("failed to delete hichain credential and group with ret:%{public}d.", retValue);
        return ERR_DM_FAILED;
    }
    g_deviceGroupManager->destroyInfo(&returnInfo);
#endif
    return DM_OK;
}

int32_t MineHiChainConnector::CreateGroup(const std::string &reqJsonStr)
{
    int64_t requestId = CREATE_GROUP_REQUESTID;
    char deviceUdid[DEVICE_UDID_LENGTH + 1] = {0};

    JsonObject jsonObject(reqJsonStr);
    if (jsonObject.IsDiscarded()) {
        LOGE("reqJsonStr string not a json type.");
        return ERR_DM_FAILED;
    }
    if (!jsonObject.Contains(FIELD_USER_ID) || !jsonObject[FIELD_USER_ID].IsString()) {
        LOGE("userId key is not exist in reqJsonStr.");
        return ERR_DM_FAILED;
    }
    int32_t retValue = GetDevUdid(deviceUdid, DEVICE_UDID_LENGTH);
    if (retValue != 0) {
        LOGE("failed to local device Udid with ret: %{public}d", retValue);
        return ERR_DM_FAILED;
    }

    JsonObject jsonObj;
    jsonObj[FIELD_USER_ID] = jsonObject[FIELD_USER_ID];
    jsonObj[FIELD_GROUP_NAME] = DEVICE_MANAGER_GROUPNAME;
    jsonObj[FIELD_DEVICE_ID] = std::string(deviceUdid);
    jsonObj[FIELD_GROUP_TYPE] = IDENTICAL_ACCOUNT_GROUP;
    std::string createParams = SafetyDump(jsonObj);
    retValue = g_deviceGroupManager->createGroup(DEFAULT_OS_ACCOUNT, requestId, DM_PKG_NAME, createParams.c_str());
    if (retValue != HC_SUCCESS) {
        LOGE("failed to create group with ret:%{public}d.", retValue);
        return ERR_DM_FAILED;
    }

    std::chrono::seconds timeout = std::chrono::seconds(MAX_HICHAIN_DELAY_TIME);
    std::unique_lock<std::mutex> locker(g_createGroupMutex);
    if (g_createGroupNotify.wait_for(locker, timeout, [] { return g_mineCreateGroupFlag; })) {
        g_mineCreateGroupFlag = false;
        return DM_OK;
    }
    g_mineCreateGroupFlag = false;
    return ERR_DM_FAILED;
}

int MineHiChainConnector::RequestCredential(std::string &returnJsonStr)
{
    if (g_deviceGroupManager == nullptr) {
        LOGE("HiChainConnector::g_deviceGroupManager is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("start to request device credential.");
#if (defined(MINE_HARMONY))
    char *returnInfo = nullptr;
    int32_t retValue = g_deviceGroupManager->getRegisterInfo(&returnInfo);
    if (retValue != HC_SUCCESS || returnInfo == nullptr) {
        LOGE("failed to request hichain credential with ret:%{public}d.", retValue);
        return ERR_DM_HICHAIN_GET_REGISTER_INFO;
    }
    returnJsonStr = returnInfo;
    g_deviceGroupManager->destroyInfo(&returnInfo);
#endif
    LOGI("request hichain device credential successfully with JsonStrLen:%{public}zu", returnJsonStr.size());
    return DM_OK;
}

int MineHiChainConnector::CheckCredential(std::string reqJsonStr, std::string &returnJsonStr)
{
    if (reqJsonStr.empty() || g_deviceGroupManager == nullptr) {
        LOGE("HiChainConnector::g_deviceGroupManager is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGI("start to check device credential.");
#if (defined(MINE_HARMONY))
    char *returnInfo = nullptr;
    int32_t retValue = g_deviceGroupManager->processCredential(QUERY_SELF_CREDENTIAL_INFO,
        reqJsonStr.c_str(), &returnInfo);
    if (retValue != HC_SUCCESS) {
        LOGE("failed to check device credential info with ret:%{public}d.", retValue);
        return ERR_DM_HICHAIN_GET_REGISTER_INFO;
    }
    returnJsonStr = returnInfo;
    g_deviceGroupManager->destroyInfo(&returnInfo);
#endif
    LOGI("check device credential info successfully with JsonStrLen:%{public}zu", returnJsonStr.size());
    return DM_OK;
}

int MineHiChainConnector::ImportCredential(std::string reqJsonStr, std::string &returnJsonStr)
{
    if (reqJsonStr.empty() || g_deviceGroupManager == nullptr) {
        LOGE("reqJsonStr is empty or g_deviceGroupManager is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    JsonObject jsonObject(reqJsonStr);
    if (jsonObject.IsDiscarded()) {
        LOGE("import credenfial input reqJsonStr string not a json string type.");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (IsCredentialExist()) {
        LOGE("the device has already exists credential.");
        return ERR_DM_HICHAIN_CREDENTIAL_EXISTS;
    }
    int32_t retValue = g_deviceGroupManager->regCallback(DM_PKG_NAME, &g_deviceAuthCallback);
    if (retValue != HC_SUCCESS) {
        LOGE("failed to register callback function to hichain with ret:%{public}d.", retValue);
        return ERR_DM_HICHAIN_REGISTER_CALLBACK;
    }
    LOGI("start to import device credential info to hichain.");
#if (defined(MINE_HARMONY))
    char *returnInfo = nullptr;
    retValue = g_deviceGroupManager->processCredential(IMPORT_SELF_CREDENTIAL, reqJsonStr.c_str(), &returnInfo);
    if (retValue != HC_SUCCESS) {
        LOGE("failed to import hichain credential with ret:%{public}d.", retValue);
        return ERR_DM_HICHAIN_GROUP_CREATE_FAILED;
    }
    if (CreateGroup(reqJsonStr) != DM_OK) {
        DeleteCredentialAndGroup();
        g_deviceGroupManager->destroyInfo(&returnInfo);
        LOGE("fail to import hichain credential bacause create group unsuccessfully.");
        return ERR_DM_HICHAIN_GROUP_CREATE_FAILED;
    }
    g_deviceGroupManager->destroyInfo(&returnInfo);
#endif
    LOGI("import device credential info successfully.");
    return DM_OK;
}

int MineHiChainConnector::DeleteCredential(std::string reqJsonStr, std::string &returnJsonStr)
{
    if (reqJsonStr.empty() || g_deviceGroupManager == nullptr) {
        LOGE("reqJsonStr is empty or g_deviceGroupManager is nullptr.");
        return ERR_DM_INPUT_PARA_INVALID;
    }

    LOGI("start to delete device credential from hichain.");
    if (DeleteCredentialAndGroup() != DM_OK) {
        LOGE("failed to delete hichain credential.");
        return ERR_DM_HICHAIN_CREDENTIAL_DELETE_FAILED;
    }
    LOGI("delete device credential info from hichain successfully.");
    return DM_OK;
}

bool MineHiChainConnector::IsCredentialExist(void)
{
    bool resultFlag = false;
    std::string reqJsonStr = "{\n}";

    if (g_deviceGroupManager == nullptr) {
        LOGE("g_deviceGroupManager is nullptr.");
        return resultFlag;
    }
#if (defined(MINE_HARMONY))
    char *returnInfo = nullptr;
    int32_t retValue = g_deviceGroupManager->processCredential(QUERY_SELF_CREDENTIAL_INFO,
        reqJsonStr.c_str(), &returnInfo);
    if (retValue != HC_SUCCESS || returnInfo == nullptr) {
        LOGE("failed to check device credential info with ret:%{public}d.", retValue);
        return resultFlag;
    }

    do {
        JsonObject jsonObject(returnInfo);
        if (jsonObject.IsDiscarded()) {
            LOGE("reqJsonStr is not a json string type.");
            break;
        }
        if (!jsonObject.Contains(FIELD_CREDENTIAL_EXISTS) || !jsonObject[FIELD_CREDENTIAL_EXISTS].IsBoolean()) {
            LOGE("failed to get key:%{public}s from import json object.", FIELD_CREDENTIAL_EXISTS);
            break;
        }
        if (!jsonObject[FIELD_CREDENTIAL_EXISTS]) {
            LOGI("credential information does not exist on the current device.");
            break;
        }
        LOGI("credential information exist on the current device.");
        resultFlag = true;
    } while (false);

    g_deviceGroupManager->destroyInfo(&returnInfo);
#endif
    return resultFlag;
}
} // namespace DistributedHardware
} // namespace OHOS