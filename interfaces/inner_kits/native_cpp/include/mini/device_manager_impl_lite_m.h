/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DEVICE_MANAGER_IMPL_LITE_M_H
#define OHOS_DEVICE_MANAGER_IMPL_LITE_M_H

#include <stdbool.h>

#include "device_manager_common.h"
#include "softbus_common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM_OK = 0,
    ERR_DM_FAILED = -100000,
    ERR_DM_NO_INIT,
    ERR_DM_ALREADY_INIT,
    ERR_DM_POINT_NULL,
    ERR_DM_GET_SHA256_HASH,
    ERR_DM_IMPORT_PKGNAME,
    ERR_DM_DELETE_PKGNAME,
    ERR_DM_GET_BASE64_ENCODE,
    ERR_DM_GET_BASE64_DECODE,
    ERR_DM_INPUT_INVALID_VALUE,
    ERR_DM_BIND_TYPE_NOT_SUPPORT,
    ERR_DM_BIND_NO_ADDR,
    ERR_DM_NO_CREDENTIAL,

    ERR_DM_SOFTBUS_FAILED = -9000,
    ERR_DM_SOFTBUS_PUBLISH_LNN,
    ERR_DM_SOFTBUS_STOP_PUBLISH_LNN,
    ERR_DM_SOFTBUS_REG_STATE_CALLBACK,
    ERR_DM_SOFTBUS_UNREG_STATE_CALLBACK,
    ERR_DM_SOFTBUS_NO_TRUSTED_DEVICE,
    ERR_DM_SOFTBUS_GET_ALL_DEVICE_INFO,
    ERR_DM_SOFTBUS_SEND_BROADCAST,
    ERR_DM_SOFTBUS_START_DISCOVERY_DEVICE,
    ERR_DM_SOFTBUS_STOP_DISCOVERY_DEVICE,
    ERR_DM_SOFTBUS_REPEAT_DISCOVERY_DEVICE,
    ERR_DM_SOFTBUS_OPEN_AUTH_SESSION_FAILED,

    ERR_DM_HICHAIN_FAILED = -8000,
    ERR_DM_HICHAIN_REGISTER_CALLBACK,
    ERR_DM_HICHAIN_UNREGISTER_CALLBACK,
    ERR_DM_HICHAIN_INIT_DEVICE_AUTH_SERVICE,
    ERR_DM_HICHAIN_GET_GM_INSTANCE,
    ERR_DM_HICHAIN_GET_CREDENTIAL_INFO,
    ERR_DM_HICHAIN_IMPORT_CREDENTIAL,
    ERR_DM_HICHAIN_DELETE_CREDENTIAL,
    ERR_DM_HICHAIN_CHECK_CREDENTIAL,
    ERR_DM_HICHAIN_CREATE_GROUP,
    ERR_DM_HICHAIN_CREDENTIAL_EXISTS,
    ERR_DM_HICHAIN_AUTH_DEVICE,

    ERR_DM_LITEOS_FAILED = -7000,
    ERR_DM_LITEOS_CREATE_TASK,
    ERR_DM_LITEOS_DELETE_TASK,
    ERR_DM_LITEOS_CREATE_MUTEX_OR_SEM,
    ERR_DM_LITEOS_DELETE_MUTEX_OR_SEM,
    ERR_DM_LITEOS_PEND_MUTEX,
    ERR_DM_LITEOS_POST_MUTEX,
    ERR_DM_LITEOS_CREATE_TIMER,
    ERR_DM_LITEOS_PEND_SEMAPHORE,
    ERR_DM_LITEOS_POST_SEMAPHORE,

    ERR_DM_CJSON_FAILED = -6000,
    ERR_DM_CJSON_CREATE_OBJECT,
    ERR_DM_CJSON_GET_OBJECT,
    ERR_DM_CJSON_ADD_TRUE,
    ERR_DM_CJSON_PRINT,
    ERR_DM_CJSON_KEY_TYPE,
    ERR_DM_CJSON_KEY_NO_EXIST,
    ERR_DM_CJSON_PARSE_ARRAY,
    ERR_DM_CJSON_PARSE_STRING,
    ERR_DM_CJSON_ARRAY_SIZE,
    ERR_DM_CJSON_GET_USERID,
};

static const int SUPPORT_BIND_TYPE = 0;

typedef struct {
    char deviceId[DM_MAX_DEVICE_ID_LEN + 1];
    char networkId[DM_MAX_DEVICE_NETWORKID_LEN + 1];
    char deviceName[DM_MAX_DEVICE_NAME_LEN + 1];
    unsigned int deviceTypeId;
    ConnectionAddr connectAddr;
    int range;
    int credible;
    bool isLocalExistCredential;
    int authForm;
} DmDeviceInfo;

typedef struct {
    char deviceId[DM_MAX_DEVICE_ID_LEN + 1];
    char deviceName[DM_MAX_DEVICE_NAME_LEN + 1];
    unsigned int deviceTypeId;
    char networkId[DM_MAX_DEVICE_NETWORKID_LEN + 1];
} DmDeviceBasicInfo;

typedef struct {
    void (*onTargetOnline)(const DmDeviceBasicInfo *deviceInfo);
    void (*onTargetOffline)(const DmDeviceBasicInfo *deviceInfo);
} DevStatusCallback;

typedef struct {
    void (*onTargetFound)(const DmDeviceBasicInfo *deviceInfo);
} OnTargetFound;

typedef struct {
    void (*onBindResult)(const char *networkId, const int retCode);
    void (*onUnBindResult)(const char *networkId, const int retCode);
} OnBindResult;

typedef struct {
    void (*onAdvertisingResult)(const int advertisingId, const int result);
} OnAdvertisingResult;

int InitDeviceManager(void);
bool DeviceManagerIsInit(void);
int UnInitDeviceManager(void);
int RegisterDevStateCallback(const char *pkgName, const char *extra, DevStatusCallback callback);
int UnRegisterDevStateCallback(const char *pkgName);
int GetTrustedList(const char *pkgName, DmDeviceBasicInfo *deviceList, const int deviceListLen, int *trustListLen);
int StartAdvertising(const char *advParam, const char *filterOption, OnAdvertisingResult cb);
int StopAdvertising(const char *pkgName);
int StartDiscovering(const char *discoverParam, const char *filterOption, OnTargetFound callback);
int StopDiscovering(const char *pkgName, const int subscribeId);
int BindTarget(const char *pkgName, const char *deviceId, const char *bindParam, OnBindResult cb);
int UnBindTarget(const char *pkgName, const char *networkId);
void FreeJsonStringMemory(char **jsonStr);
int RequestCredential(char **returnJsonStr);
int CheckCredential(const char *reqJsonStr, char **returnJsonStr);
int ImportCredential(const char *reqJsonStr, char **returnJsonStr);
int DeleteCredential(const char *reqJsonStr, char **returnJsonStr);

#ifdef __cplusplus
}
#endif

#endif // OHOS_DEVICE_MANAGER_IMPL_LITE_M_H
