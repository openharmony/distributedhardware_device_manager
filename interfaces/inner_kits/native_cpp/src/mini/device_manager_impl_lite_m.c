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

#include "device_manager_impl_lite_m.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cJSON.h"

#include "device_manager_common.h"
#include "hichain_adapter.h"
#include "softbus_adapter.h"

static const char * const FILED_PKG_NAME = "pkgName";
static const char * const FILED_BIND_TYPE = "bindType";
static const char * const FILED_SUBSCRIBE_ID = "subscribeId";

static volatile bool g_deviceManagerInitFlag = false;

int InitDeviceManager(void)
{
    DMLOGI("init device manager start.");
    if (g_deviceManagerInitFlag) {
        DMLOGI("device manager module has been initialized.");
        return DM_OK;
    }
    int retValue = InitSoftbusModle();
    if (retValue != DM_OK) {
        DMLOGE("failed to init softbus with ret: %d.", retValue);
        return retValue;
    }
    retValue = InitHichainModle();
    if (retValue != DM_OK) {
        DMLOGE("failed to init hichain with ret: %d.", retValue);
        return retValue;
    }
    g_deviceManagerInitFlag = true;
    return DM_OK;
}

bool DeviceManagerIsInit(void)
{
    return g_deviceManagerInitFlag;
}

int UnInitDeviceManager(void)
{
    DMLOGI("device manager UnInitDeviceManager start.");
    if (!g_deviceManagerInitFlag) {
        DMLOGI("device manager module not initialized.");
        return DM_OK;
    }
    UnInitSoftbusModle();
    UnInitHichainModle();
    g_deviceManagerInitFlag = false;
    return DM_OK;
}

int RegisterDevStateCallback(const char *pkgName, const char *extra, DevStatusCallback callback)
{
    DMLOGI("device manager RegisterDevStateCallback start.");
    (void)extra;
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return RegisterSoftbusDevStateCallback(pkgName, callback);
}

int UnRegisterDevStateCallback(const char *pkgName)
{
    DMLOGI("device manager UnRegisterDevStateCallback start.");
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return UnRegisterSoftbusDevStateCallback(pkgName);
}

int GetTrustedList(const char *pkgName, DmDeviceBasicInfo *deviceList, const int deviceListLen,
    int *trustListLen)
{
    DMLOGI("device manager GetTrustedList start.");
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return GetSoftbusTrustedDeviceList(pkgName, deviceList, deviceListLen, trustListLen);
}

int StartAdvertising(const char *advParam, const char *filterOption, OnAdvertisingResult cb)
{
    DMLOGI("device manager StartAdvertising start.");
    (void)filterOption;
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    if (advParam == NULL) {
        DMLOGE("input param is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    cJSON *obj = cJSON_Parse(advParam);
    if (obj == NULL) {
        DMLOGE("StartAdvertising parse advParam failed.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    cJSON *pkgNameObj = cJSON_GetObjectItem(obj, FILED_PKG_NAME);
    if (pkgNameObj == NULL || !cJSON_IsString(pkgNameObj)) {
        DMLOGE("parse pkgName failed.");
        cJSON_Delete(obj);
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    char *pkgName = pkgNameObj->valuestring;
    int ret = StartSoftbusPublish(pkgName, cb);
    cJSON_Delete(obj);
    return ret;
}

int StopAdvertising(const char *pkgName)
{
    DMLOGI("device manager stop advertising start.");
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return StopSoftbusPublish(pkgName);
}

int StartDiscovering(const char *discoverParam, const char *filterOption, OnTargetFound callback)
{
    DMLOGI("device manager start discovering start.");
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    if (discoverParam == NULL) {
        DMLOGE("input param is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    cJSON *obj = cJSON_Parse(discoverParam);
    if (obj == NULL) {
        DMLOGE("StartDiscovering parse advParam failed.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    cJSON *pkgNameObj = cJSON_GetObjectItem(obj, FILED_PKG_NAME);
    if (pkgNameObj == NULL || !cJSON_IsString(pkgNameObj)) {
        cJSON_Delete(obj);
        DMLOGE("parse pkgName failed.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    char *pkgName = pkgNameObj->valuestring;
    cJSON *subscribeIdObj = cJSON_GetObjectItem(obj, FILED_SUBSCRIBE_ID);
    if (subscribeIdObj == NULL || !cJSON_IsNumber(subscribeIdObj)) {
        DMLOGE("parse subscrilbe id failed.");
        cJSON_Delete(obj);
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    int subscribeId = subscribeIdObj->valueint;
    int ret = StartSoftbusDiscovery(pkgName, subscribeId, filterOption, callback);
    cJSON_Delete(obj);
    return ret;
}

int StopDiscovering(const char *pkgName, const int subscribeId)
{
    DMLOGI("device manager StopDiscovering start.");
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return StopSoftbusDiscovery(pkgName, subscribeId);
}

int BindTarget(const char *pkgName, const char *deviceId, const char *bindParam, OnBindResult cb)
{
    DMLOGI("device manager BindTarget start.");
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    if (bindParam == NULL) {
        DMLOGE("bind target bind param is null.");
        return ERR_DM_POINT_NULL;
    }
    cJSON *obj = cJSON_Parse(bindParam);
    if (obj == NULL) {
        DMLOGE("StartDiscovering parse advParam failed.");
        return ERR_DM_CJSON_PARSE_STRING;
    }
    cJSON *bindTypeObj = cJSON_GetObjectItem(obj, FILED_BIND_TYPE);
    if (bindTypeObj == NULL || !cJSON_IsNumber(bindTypeObj)) {
        DMLOGE("parse bind type failed.");
        cJSON_Delete(obj);
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    int bindType = bindTypeObj->valueint;
    int ret = SoftbusBindTarget(pkgName, deviceId, bindType, cb);
    cJSON_Delete(obj);
    return ret;
}

int UnBindTarget(const char *pkgName, const char *networkId)
{
    DMLOGI("device manager UnBindTarget start.");
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return SoftbusUnBindTarget(pkgName, networkId);
}

int RequestCredential(char **returnJsonStr)
{
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return RequestHichainCredential(returnJsonStr);
}

void FreeJsonStringMemory(char **jsonStr)
{
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return;
    }
    FreeHichainJsonStringMemory(jsonStr);
}

int CheckCredential(const char *reqJsonStr, char **returnJsonStr)
{
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return CheckHichainCredential(reqJsonStr, returnJsonStr);
}

int ImportCredential(const char *reqJsonStr, char **returnJsonStr)
{
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return ImportHichainCredential(reqJsonStr, returnJsonStr);
}

int DeleteCredential(const char *reqJsonStr, char **returnJsonStr)
{
    if (!g_deviceManagerInitFlag) {
        DMLOGE("device manager module is not initialized.");
        return ERR_DM_NO_INIT;
    }
    return DeleteHichainCredential(reqJsonStr, returnJsonStr);
}
