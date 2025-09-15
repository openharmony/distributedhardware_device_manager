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

#include <stdio.h>
#include <unistd.h>

#include "cJSON.h"
#include "cmsis_os2.h"
#include "hichain_adapter.h"
#include "inner_session.h"
#include "los_config.h"
#include "los_mux.h"
#include "los_sem.h"
#include "los_swtmr.h"
#include "ohos_init.h"
#include "parameter.h"
#include "securec.h"
#include "session.h"
#include "softbus_bus_center.h"
#include "softbus_common.h"
#include "softbus_errcode.h"

#define NOT_FILTER (-1)
#define SESSION_SIDE_CLIENT (1)
#define SOFTBUS_DELAY_TICK_COUNT (10 * LOSCFG_BASE_CORE_TICK_PER_SECOND)  // 10s
#define DM_MAX_DEVICE_SIZE (100)

static const char * const DM_CAPABILITY_OSD = "osdCapability";
static const char * const DM_PKG_NAME = "com.ohos.devicemanager";
static const char * const FILTER_CREDIBLE = "credible";
static const char * const FILTER_RANGE = "range";
static const char * const FILTER_ISTRUSTED = "isTrusted";
static const char * const FILTER_AUTHFORM = "authform";
static const char * const FILTER_DEVICE_TYPE = "deviceType";
static const char * const FILED_MSG_TYPE = "MSG_TYPE";
static const char * const FILED_BIND_TYPE = "AUTHTYPE";
static const char * const FILED_REPLY = "REPLY";
static const char * const FILED_IS_CRE_EXISTED = "isCreExist";
static const char * const FILED_IS_BIND_TYPE_SUPPORTED = "isBindTypeSupported";
static const char * const DM_SESSION_NAME = "ohos.distributedhardware.devicemanager.resident";
static const int MSG_NEGOTIATE = 80;
static const int MSG_NEGOTIATE_RESP = 90;

static const unsigned int DEVICEMANAGER_SA_ID = 4802;

typedef struct {
    int credible;
    int range;
    int deviceType;
    int isTrusted;
    int authForm;
} FilterOption;

static int CreateSoftbusSemaphoreAndMutex(void);
static int DeleteSoftbusSemaphoreAndMutex(void);

static bool IsValidStartDiscoveryInput(const char *pkgName, const char *filterOption,
    OnTargetFound callback);
static int StartSoftbusDiscovering(const char *pkgName, const int subscribeId, OnTargetFound callback);
static int ParseDiscoverFilterOption(const char *filterOption);
static bool ImportPkgNameToDiscoveryMap(const char *pkgName, const int subscribeId, OnTargetFound callback);
static int SoftbusRefreshLNN(const char *pkgName, const int subscribeId);
static int StopSoftbusRefreshLNN(const char *pkgName, const int subscribeId);

static bool DeletePkgNameFromStateMap(const char *pkgName);
static bool ImportPkgNameToStateMap(const char *pkgName, DevStatusCallback callback);
static void DeviceInfoCopyToDmDevice(DmDeviceInfo *dmDeviceInfo, const DeviceInfo *deviceInfo);
static void NodeBasicInfoCopyToDmDevice(DmDeviceBasicInfo *dmDeviceInfo, const NodeBasicInfo *nodeBasicInfo);
static void DmDeviceInfoToDmBasicInfo(const DmDeviceInfo *dmDeviceInfo, DmDeviceBasicInfo *dmBasicInfo);

static void OnPublishLNNResult(int publishId, PublishResult reason);
static void OnRefreshDiscoveryResult(int32_t refreshId, RefreshResult reason);
static void OnDiscoveryDeviceFound(const DeviceInfo *deviceInfo);
static void OnSoftbusDeviceOnline(NodeBasicInfo *deviceInfo);
static void OnSoftbusDeviceOffline(NodeBasicInfo *deviceInfo);
static void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen);
static int OnSessionOpened(int sessionId, int result);
static void OnSessionClosed(int sessionId);
static void OnJoinLNNCallback(ConnectionAddr *addr, const char *networkId, int32_t retCode);
static void OnLeaveLNNCallback(const char *networkId, int32_t retCode);

static void ProcessSinkMsg(const char *data, unsigned int dataLen);
static void ProcessSourceMsg(const char *data, unsigned int dataLen);
static int GetConnAddrByDeviceId(const char *deviceId, ConnectionAddr *addr);
static bool ImportDeviceToAddrMap(const DmDeviceInfo *deviceInfo);
static bool IsPkgNameValid(const char *pkgName);
static bool IsDeviceIdValid(const char *deviceId);
static char* CreateRespNegotiateMsg(const int bindType);
static int AbilityNegotiate(const int bindType);

static UINT32 g_bindSem = 0;
static bool g_publishLNNFlag = false;
static bool g_startDiscoveryFlag = false;
static ConnectionAddr g_bindAddr;
static char *g_bindDeviceId = NULL;
static int g_bindType = -1;
static int g_sessionId = -1;

#define DM_MUTEX_TIMEOUT osWaitForever
static osMutexId_t g_dmGlobalLock = NULL;
static osMutexId_t g_dmBindLock = NULL;

static void InitDmGlobalLock(void)
{
    if (g_dmGlobalLock == NULL) {
        osMutexAttr_t globalMutexAttr = {
            "DmGloablLock",
            osMutexRecursive | osMutexPrioInherit,
            NULL,
            0U
        };
        g_dmGlobalLock = osMutexNew(&globalMutexAttr);
    }
    DMLOGI("InitDmGlobalLock successfully.");
}

static void InitDmBindLock(void)
{
    if (g_dmBindLock == NULL) {
        osMutexAttr_t bindMutexAttr = {
            "DmBindlLock",
            osMutexRecursive | osMutexPrioInherit,
            NULL,
            0U
        };
        g_dmBindLock = osMutexNew(&bindMutexAttr);
    }
    DMLOGI("InitDmBindLock successfully.");
}

int LockDmGlobalLock(void)
{
    if (g_dmGlobalLock == NULL) {
        InitDmGlobalLock();
    }

    osStatus_t ret = osMutexAcquire(g_dmGlobalLock, DM_MUTEX_TIMEOUT);
    if (ret != osOK) {
        printf("[dm_service] osMutexAcquire failed \n");
        return ERR_DM_FAILED;
    }

    return DM_OK;
}

int UnlockDmGlobalLock(void)
{
    if (g_dmGlobalLock == NULL) {
        return ERR_DM_FAILED;
    }

    osStatus_t ret = osMutexRelease(g_dmGlobalLock);
    if (ret != osOK) {
        printf("[dm_service] osMutexUnlock failed \n");
        return ERR_DM_FAILED;
    }

    return DM_OK;
}

int LockDmBindLock(void)
{
    if (g_dmBindLock == NULL) {
        InitDmBindLock();
    }

    osStatus_t ret = osMutexAcquire(g_dmBindLock, DM_MUTEX_TIMEOUT);
    if (ret != osOK) {
        printf("[dm_service] osMutexAcquire failed \n");
        return ERR_DM_FAILED;
    }

    return DM_OK;
}

int UnlockDmBindLock(void)
{
    if (g_dmBindLock == NULL) {
        return ERR_DM_FAILED;
    }

    osStatus_t ret = osMutexRelease(g_dmBindLock);
    if (ret != osOK) {
        printf("[dm_service] osMutexUnlock failed \n");
        return ERR_DM_FAILED;
    }

    return DM_OK;
}

static struct {
    int subscribeId;
    char pkgName[DM_MAX_PKG_NAME_LEN + 1];
    OnTargetFound discoveryCallback;
    bool valid;
    FilterOption filterOption;
} g_discoveryCallbackMap;

static struct {
    char pkgName[DM_MAX_PKG_NAME_LEN + 1];
    OnAdvertisingResult advCallback;
    bool valid;
} g_advertisingCallbackMap;

static struct {
    char pkgName[DM_MAX_PKG_NAME_LEN + 1];
    OnBindResult bindRetCallback;
    bool valid;
} g_bindRetCallbackMap;

static struct {
    char pkgName[DM_MAX_PKG_NAME_LEN + 1];
    DevStatusCallback stateCallback;
    bool valid;
} g_stateCallbackMap[DM_MAX_REG_PKG_NUMBER];

static struct {
    char deviceId[DM_MAX_DEVICE_ID_LEN + 1];
    ConnectionAddr connectAddr;
    bool valid;
} g_deviceIdAddrMap[DM_MAX_DEVICE_SIZE];

static struct {
    char deviceId[DM_MAX_DEVICE_ID_LEN + 1];
    char networkId[DM_MAX_DEVICE_NETWORKID_LEN + 1];
    bool valid;
} g_devIdMap[DM_MAX_DEVICE_SIZE];

IPublishCb g_publishLNNCallback = {
    .OnPublishResult = OnPublishLNNResult
};

INodeStateCb g_softbusStatusChangeCb = {
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE,
    .onNodeOnline = OnSoftbusDeviceOnline,
    .onNodeOffline = OnSoftbusDeviceOffline,
    .onNodeBasicInfoChanged = NULL
};

IRefreshCallback g_refreshDiscoveryCallback = {
    .OnDeviceFound = OnDiscoveryDeviceFound,
    .OnDiscoverResult = OnRefreshDiscoveryResult
};

ISessionListener g_sessionListener = {
    .OnSessionOpened = OnSessionOpened,
    .OnSessionClosed = OnSessionClosed,
    .OnBytesReceived = OnBytesReceived,
    .OnMessageReceived = NULL,
    .OnStreamReceived = NULL
};

OnJoinLNNResult g_joinLNNResult = OnJoinLNNCallback;
OnLeaveLNNResult g_leaveLNNResult = OnLeaveLNNCallback;

int InitSoftbusModle(void)
{
    int retValue = DM_OK;
    g_discoveryCallbackMap.valid = false;
    for (int i = 0; i < DM_MAX_REG_PKG_NUMBER; i++) {
        g_stateCallbackMap[i].valid = false;
    }
    for (int i = 0; i < DM_MAX_DEVICE_SIZE; i++) {
        g_deviceIdAddrMap[i].valid = false;
        g_devIdMap[i].valid = false;
    }

    do {
        InitSoftBusServer();
        DMLOGI("Softbus adapter create session session server start.");
        int32_t ret = CreateSessionServer(DM_PKG_NAME, DM_SESSION_NAME, &g_sessionListener);
        if (ret != DM_OK) {
            DMLOGE("[SOFTBUS]CreateSessionServer failed, ret: %d.", ret);
        } else {
            DMLOGI("[SOFTBUS]CreateSessionServer ok.");
        }
        retValue = RegNodeDeviceStateCb(DM_PKG_NAME, &g_softbusStatusChangeCb);
        if (retValue != SOFTBUS_OK) {
            DMLOGE("failed to Register callback to softbus with ret: %d.", retValue);
            retValue = ERR_DM_SOFTBUS_REG_STATE_CALLBACK;
            break;
        }
        if (CreateSoftbusSemaphoreAndMutex() != DM_OK) {
            DMLOGE("failed to create mutex and semaphore.");
            retValue = ERR_DM_LITEOS_CREATE_MUTEX_OR_SEM;
            break;
        }
    } while (false);

    if (retValue != DM_OK) {
        DMLOGE("failed to init softbus modle with ret: %d.", retValue);
        UnInitSoftbusModle();
        return retValue;
    }
    DMLOGI("init softbus modle successfully.");
    return DM_OK;
}

int UnInitSoftbusModle(void)
{
    int returnResult = DM_OK;
    int retValue = UnregNodeDeviceStateCb(&g_softbusStatusChangeCb);
    if (retValue != SOFTBUS_OK) {
        DMLOGE("failed to unregister device state callback with ret: %d.", retValue);
        returnResult = ERR_DM_SOFTBUS_UNREG_STATE_CALLBACK;
    }
    if (DeleteSoftbusSemaphoreAndMutex() != DM_OK) {
        DMLOGE("failed to delete semaphore.");
        returnResult = ERR_DM_LITEOS_DELETE_MUTEX_OR_SEM;
    }
    if (returnResult != DM_OK) {
        DMLOGE("failed to uninti softbus modle with ret: %d.", returnResult);
        return returnResult;
    }
    DMLOGI("uninit softbus modle successfully.");
    return DM_OK;
}

int RegisterSoftbusDevStateCallback(const char *pkgName, DevStatusCallback callback)
{
    DMLOGI("RegisterSoftbusDevStateCallback start.");
    if (!IsPkgNameValid(pkgName)) {
        DMLOGE("pkgName is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    if (callback.onTargetOnline == NULL || callback.onTargetOffline == NULL) {
        DMLOGE("callback is NULL.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }

    DMLOGI("start to register State Callback with pkgName: %s.", pkgName);
    int returnResult = DM_OK;
    if (LockDmGlobalLock() != DM_OK) {
        DMLOGE("LockDmGlobalLock failed.");
        return ERR_DM_FAILED;
    }
    if (!ImportPkgNameToStateMap(pkgName, callback)) {
        DMLOGE("no memory for a new callback register with pkgName: %s.", pkgName);
        returnResult = ERR_DM_IMPORT_PKGNAME;
    }
    if (UnlockDmGlobalLock() != DM_OK) {
        DMLOGE("UnlockDmGlobalLock failed.");
        return ERR_DM_FAILED;
    }
    if (returnResult != DM_OK) {
        DMLOGE("failed to register state callback with pkgName: %s, ret: %d.", pkgName, returnResult);
        return returnResult;
    }
    DMLOGI("register state callback successfully.");
    return DM_OK;
}

int UnRegisterSoftbusDevStateCallback(const char *pkgName)
{
    if (!IsPkgNameValid(pkgName)) {
        DMLOGE("pkgName is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }

    DMLOGI("start to unregister State Callback with pkgName: %s.", pkgName);
    int returnResult = DM_OK;
    if (LockDmGlobalLock() != DM_OK) {
        DMLOGE("LockDmGlobalLock failed.");
        return ERR_DM_FAILED;
    }
    if (!DeletePkgNameFromStateMap(pkgName)) {
        DMLOGE("no callback for this pkgName: %s.", pkgName);
        returnResult = ERR_DM_DELETE_PKGNAME;
    }
    if (UnlockDmGlobalLock() != DM_OK) {
        DMLOGE("UnlockDmGlobalLock failed.");
        return ERR_DM_FAILED;
    }
    if (returnResult != DM_OK) {
        DMLOGE("failed to unregister state callback with pkgName: %s, ret: %d.", pkgName, returnResult);
        return returnResult;
    }
    DMLOGI("unregister state callback successfully.");
    return DM_OK;
}

int GetSoftbusTrustedDeviceList(const char *pkgName, DmDeviceBasicInfo *deviceList, const int deviceListLen,
    int *trustListLen)
{
    DMLOGI("GetSoftbusTrustedDeviceList.");
    if (!IsPkgNameValid(pkgName)) {
        DMLOGE("pkgName is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    if (deviceList == NULL || trustListLen == NULL) {
        DMLOGE("input point is NULL.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    NodeBasicInfo *nodeInfo = NULL;
    int retValue = GetAllNodeDeviceInfo(DM_PKG_NAME, &nodeInfo, trustListLen);
    if (retValue != SOFTBUS_OK || *trustListLen < 0) {
        DMLOGE("get all node device info error: %d, trustListLen: %d.", retValue, *trustListLen);
        FreeNodeInfo(nodeInfo);
        return ERR_DM_SOFTBUS_GET_ALL_DEVICE_INFO;
    }
    int minLen = (deviceListLen > *trustListLen ? *trustListLen : deviceListLen);
    if (minLen > DM_MAX_DEVICE_SIZE) {
        DMLOGE("invalid device len.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    for (int i = 0; i < minLen; i++) {
        NodeBasicInfoCopyToDmDevice(&deviceList[i], &nodeInfo[i]);
    }
    FreeNodeInfo(nodeInfo);
    DMLOGI("get trusted device with trustDeviceCount: %d.", *trustListLen);
    return DM_OK;
}

int StartSoftbusDiscovery(const char *pkgName, const int subscribeId, const char *filterOption,
    OnTargetFound callback)
{
    DMLOGI("StartSoftbusDiscovery start.");
    if (!IsValidStartDiscoveryInput(pkgName, filterOption, callback)) {
        DMLOGE("input parameter is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }

    int returnResult = DM_OK;
    if (LockDmGlobalLock() != DM_OK) {
        DMLOGE("LockDmGlobalLock failed.");
        return ERR_DM_FAILED;
    }
    if (ParseDiscoverFilterOption(filterOption) != DM_OK) {
        DMLOGE("failed to parse filterOption with pkgName: %s.", pkgName);
        return ERR_DM_CJSON_PARSE_STRING;
    }
    returnResult = StartSoftbusDiscovering(pkgName, subscribeId, callback);
    if (returnResult != DM_OK) {
        DMLOGE("failed to parse filterOption and send with pkgName: %s.", pkgName);
        UnlockDmGlobalLock();
        return returnResult;
    }
    DMLOGI("start discovery successfully with pkgName: %s.", pkgName);
    return returnResult;
}

static bool IsPkgNameValid(const char *pkgName)
{
    if (pkgName == NULL) {
        DMLOGE("input point is NULL.");
        return false;
    }
    size_t pkgNameLen = strlen(pkgName);
    if (pkgNameLen == 0 || pkgNameLen >= DM_MAX_PKG_NAME_LEN) {
        DMLOGE("not meet the condition with pkgNameLen: %u.", pkgNameLen);
        return false;
    }
    return true;
}

static bool IsDeviceIdValid(const char *deviceId)
{
    if (deviceId == NULL) {
        DMLOGE("input point is NULL.");
        return false;
    }
    size_t deviceIdLen = strlen(deviceId);
    if (deviceIdLen == 0 || deviceIdLen >= DM_MAX_DEVICE_ID_LEN) {
        DMLOGE("not meet the condition with deviceIdLen: %u.", deviceIdLen);
        return false;
    }
    return true;
}

static bool IsValidStartDiscoveryInput(const char *pkgName, const char *filterOption,
    OnTargetFound callback)
{
    (void)filterOption;
    if (!IsPkgNameValid(pkgName)) {
        DMLOGE("pkgName is invalid.");
        return false;
    }
    if (callback.onTargetFound == NULL) {
        DMLOGE("callback is NULL.");
        return false;
    }
    return true;
}

static int StartSoftbusDiscovering(const char *pkgName, const int subscribeId, OnTargetFound callback)
{
    DMLOGI("StartSoftbusDiscovering start.");
    if (g_discoveryCallbackMap.valid) {
        DMLOGE("failed to start discovery because discovery behavior already exists.");
        return ERR_DM_SOFTBUS_REPEAT_DISCOVERY_DEVICE;
    }
    if (SoftbusRefreshLNN(pkgName, subscribeId) != DM_OK) {
        DMLOGE("failed to start discovery because sending broadcast info.");
        return ERR_DM_SOFTBUS_SEND_BROADCAST;
    }
    if (!ImportPkgNameToDiscoveryMap(pkgName, subscribeId, callback)) {
        DMLOGE("failed to import pkgName to discovery map.");
        return ERR_DM_IMPORT_PKGNAME;
    }
    DMLOGI("StartSoftbusDiscovering end.");
    return DM_OK;
}

int StopSoftbusDiscovery(const char *pkgName, const int subscribeId)
{
    DMLOGI("StopSoftbusDiscovery start.");
    if (!IsPkgNameValid(pkgName)) {
        DMLOGE("pkgName is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    int returnResult = DM_OK;
    if (LockDmGlobalLock() != DM_OK) {
        DMLOGE("LockDmGlobalLock failed.");
        return ERR_DM_FAILED;
    }
    returnResult = StopSoftbusRefreshLNN(pkgName, subscribeId);
    if (returnResult != DM_OK) {
        DMLOGE("failed to stop discovery with pkgName: %s.", pkgName);
    }
    if (UnlockDmGlobalLock() != DM_OK) {
        DMLOGE("UnlockDmGlobalLock failed.");
        return ERR_DM_FAILED;
    }
    if (returnResult != DM_OK) {
        DMLOGE("failed to stop discovery with pkgName: %s.", pkgName);
        return returnResult;
    }
    DMLOGI("stop discovery successfully with pkgName: %s.", pkgName);
    return returnResult;
}

static int StopSoftbusRefreshLNN(const char *pkgName, const int subscribeId)
{
    if (!g_startDiscoveryFlag) {
        DMLOGI("already stopped.");
        return DM_OK;
    }
    if (!g_discoveryCallbackMap.valid) {
        DMLOGI("pkgName has been released with pkgName: %s, subscribeId: %d.", pkgName, subscribeId);
        return DM_OK;
    }
    if (subscribeId != g_discoveryCallbackMap.subscribeId ||
        strcmp(g_discoveryCallbackMap.pkgName, pkgName) != 0) {
        DMLOGE("pkgName: %s and subscribeId: %d do not match.", pkgName, subscribeId);
        return ERR_DM_SOFTBUS_STOP_DISCOVERY_DEVICE;
    }
    g_discoveryCallbackMap.valid = false;
    DMLOGI("stop refreshLNN successfully.");
    return DM_OK;
}

static int ParseDiscoverFilterOption(const char *filterOption)
{
    int retValue = DM_OK;
    g_discoveryCallbackMap.filterOption.credible = NOT_FILTER;
    g_discoveryCallbackMap.filterOption.range = NOT_FILTER;
    g_discoveryCallbackMap.filterOption.isTrusted = NOT_FILTER;
    g_discoveryCallbackMap.filterOption.authForm = NOT_FILTER;
    g_discoveryCallbackMap.filterOption.deviceType = NOT_FILTER;
    if (filterOption == NULL) {
        g_discoveryCallbackMap.filterOption.credible = 0;
        return retValue;
    }
    cJSON *root = cJSON_Parse(filterOption);
    if (root == NULL) {
        DMLOGE("failed to parse filter option string.");
        cJSON_Delete(root);
        return ERR_DM_CJSON_CREATE_OBJECT;
    }
    cJSON *object = cJSON_GetObjectItem(root, FILTER_CREDIBLE);
    if (object != NULL && cJSON_IsNumber(object)) {
        g_discoveryCallbackMap.filterOption.credible = object->valueint;
        DMLOGI("key %s value: %d.", FILTER_CREDIBLE, g_discoveryCallbackMap.filterOption.credible);
    }
    object = cJSON_GetObjectItem(root, FILTER_RANGE);
    if (object != NULL && cJSON_IsNumber(object)) {
        g_discoveryCallbackMap.filterOption.range = object->valueint;
        DMLOGI("key %s value: %d.", FILTER_RANGE, g_discoveryCallbackMap.filterOption.range);
    }
    object = cJSON_GetObjectItem(root, FILTER_ISTRUSTED);
    if (object != NULL && cJSON_IsNumber(object)) {
        g_discoveryCallbackMap.filterOption.isTrusted = object->valueint;
        DMLOGI("key %s value: %d.", FILTER_ISTRUSTED, g_discoveryCallbackMap.filterOption.isTrusted);
    }
    object = cJSON_GetObjectItem(root, FILTER_AUTHFORM);
    if (object != NULL && cJSON_IsNumber(object)) {
        g_discoveryCallbackMap.filterOption.authForm = object->valueint;
        DMLOGI("key %s value: %d.", FILTER_AUTHFORM, g_discoveryCallbackMap.filterOption.authForm);
    }
    object = cJSON_GetObjectItem(root, FILTER_DEVICE_TYPE);
    if (object != NULL && cJSON_IsNumber(object)) {
        g_discoveryCallbackMap.filterOption.deviceType = object->valueint;
        DMLOGI("key %s value: %d.", FILTER_DEVICE_TYPE, g_discoveryCallbackMap.filterOption.deviceType);
    }
    cJSON_Delete(root);
    DMLOGI("parse filterOption json successfully.");
    return DM_OK;
}

static int SoftbusRefreshLNN(const char *pkgName, const int subscribeId)
{
    SubscribeInfo subscribeInfo;
    subscribeInfo.mode = DISCOVER_MODE_ACTIVE;
    subscribeInfo.medium = AUTO;
    subscribeInfo.freq = HIGH;
    subscribeInfo.isSameAccount = false;
    subscribeInfo.isWakeRemote = false;
    subscribeInfo.capability = DM_CAPABILITY_OSD;
    subscribeInfo.capabilityData = NULL;
    subscribeInfo.dataLen = 0;
    subscribeInfo.subscribeId = subscribeId;
    int retValue = RefreshLNN(pkgName, &subscribeInfo, &g_refreshDiscoveryCallback);
    if (retValue != SOFTBUS_OK) {
        DMLOGE("failed to start to refresh discovery with ret: %d.", retValue);
        return ERR_DM_FAILED;
    }
    DMLOGI("softbus RefreshLNN successfully.");
    return DM_OK;
}

static bool DeletePkgNameFromStateMap(const char *pkgName)
{
    for (int i = 0; i < DM_MAX_REG_PKG_NUMBER; i++) {
        if (g_stateCallbackMap[i].valid && strcmp(g_stateCallbackMap[i].pkgName, pkgName) == 0) {
            g_stateCallbackMap[i].valid = false;
            DMLOGI("pkgName: %s has been deleted from state map.", pkgName);
            return true;
        }
    }
    DMLOGE("pkgName: %s not exist in state map.", pkgName);
    return false;
}

static bool ImportPkgNameToStateMap(const char *pkgName, DevStatusCallback callback)
{
    for (int i = 0; i < DM_MAX_REG_PKG_NUMBER; i++) {
        if (g_stateCallbackMap[i].valid && strcmp(g_stateCallbackMap[i].pkgName, pkgName) == 0) {
            DMLOGE("pkgName: %s has been exist in state map.", pkgName);
            return false;
        }
    }
    for (int i = 0; i < DM_MAX_REG_PKG_NUMBER; i++) {
        if (g_stateCallbackMap[i].valid) {
            continue;
        }
        errno_t retValue = strcpy_s(g_stateCallbackMap[i].pkgName, DM_MAX_PKG_NAME_LEN, pkgName);
        if (retValue != EOK) {
            DMLOGE("failed to copy pkgName: %s to state map.", pkgName);
            return false;
        }
        g_stateCallbackMap[i].stateCallback = callback;
        g_stateCallbackMap[i].valid = true;
        return true;
    }
    DMLOGE("state map not memory for a new callback register with pkgName: %s.", pkgName);
    return false;
}

static bool ImportPkgNameToDiscoveryMap(const char *pkgName, const int subscribeId, OnTargetFound callback)
{
    errno_t retValue = strcpy_s(g_discoveryCallbackMap.pkgName, DM_MAX_PKG_NAME_LEN, pkgName);
    if (retValue != EOK) {
        DMLOGE("failed to copy pkgName: %s to discovery map.", pkgName);
        return false;
    }
    g_discoveryCallbackMap.subscribeId = subscribeId;
    g_discoveryCallbackMap.discoveryCallback = callback;
    g_discoveryCallbackMap.valid = true;
    return true;
}

static bool ImportToDevIdMap(const char *networkId, const char *deviceId)
{
    for (int i = 0; i < DM_MAX_DEVICE_SIZE; i++) {
        if (g_devIdMap[i].valid) {
            continue;
        }
        errno_t retValue = strcpy_s(g_devIdMap[i].networkId, DM_MAX_DEVICE_NETWORKID_LEN, networkId);
        if (retValue != EOK) {
            DMLOGE("failed to copy networkId to discovery map.");
            return false;
        }
        retValue = strcpy_s(g_devIdMap[i].deviceId, DM_MAX_DEVICE_ID_LEN, deviceId);
        if (retValue != EOK) {
            DMLOGE("failed to copy deviceId to discovery map.");
            return false;
        }
        g_devIdMap[i].valid = true;
        return true;
    }
    return false;
}

static int GetDeviceIdByNetworkId(const char *networkId, char *deviceId)
{
    for (int i = 0; i < DM_MAX_DEVICE_SIZE; i++) {
        if (!g_devIdMap[i].valid) {
            continue;
        }
        if (strcmp(g_devIdMap[i].networkId, networkId) != 0) {
            continue;
        }
        errno_t retValue = strcpy_s(deviceId, DM_MAX_DEVICE_NETWORKID_LEN, g_devIdMap[i].deviceId);
        if (retValue != EOK) {
            DMLOGE("failed to copy deviceId.");
            return false;
        }
        return true;
    }
    return false;
}

static int CreateSoftbusSemaphoreAndMutex(void)
{
    DMLOGI("CreateSoftbusSemaphoreAndMutex start.");
    if (g_dmGlobalLock == NULL) {
        InitDmGlobalLock();
    }
    if (g_dmBindLock == NULL) {
        InitDmBindLock();
    }
    return DM_OK;
}

static int DeleteSoftbusSemaphoreAndMutex(void)
{
    DMLOGI("DeleteSoftbusSemaphoreAndMutex start.");
    return DM_OK;
}

static int FilterDevice(const DmDeviceInfo *dmDeviceInfo)
{
    DMLOGI("FilterDevice start.");
    CHECK_NULL_RETURN(dmDeviceInfo, ERR_DM_POINT_NULL);
    int ret = DM_OK;
    if (g_discoveryCallbackMap.filterOption.isTrusted != NOT_FILTER &&
        dmDeviceInfo->isLocalExistCredential != g_discoveryCallbackMap.filterOption.isTrusted) {
        ret = ERR_DM_FAILED;
    }
    if (g_discoveryCallbackMap.filterOption.deviceType != NOT_FILTER &&
        dmDeviceInfo->deviceTypeId != (uint16_t)g_discoveryCallbackMap.filterOption.deviceType) {
        ret = ERR_DM_FAILED;
    }
    if (g_discoveryCallbackMap.filterOption.range != NOT_FILTER &&
        dmDeviceInfo->range > g_discoveryCallbackMap.filterOption.range) {
        ret = ERR_DM_FAILED;
    }
    if (g_discoveryCallbackMap.filterOption.credible != NOT_FILTER &&
        dmDeviceInfo->credible != g_discoveryCallbackMap.filterOption.credible) {
        ret = ERR_DM_FAILED;
    }
    if (g_discoveryCallbackMap.filterOption.authForm != NOT_FILTER &&
        dmDeviceInfo->authForm != g_discoveryCallbackMap.filterOption.authForm) {
        ret = ERR_DM_FAILED;
    }
    return ret;
}

static bool ImportDeviceToAddrMap(const DmDeviceInfo *deviceInfo)
{
    const char *deviceId = deviceInfo->deviceId;
    for (int i = 0; i < DM_MAX_DEVICE_SIZE; i++) {
        if (g_deviceIdAddrMap[i].valid && strcmp(g_deviceIdAddrMap[i].deviceId, deviceId) == 0) {
            DMLOGE("deviceId has been exist in addr map.");
            return false;
        }
    }
    for (int i = 0; i < DM_MAX_DEVICE_SIZE; i++) {
        if (g_deviceIdAddrMap[i].valid) {
            continue;
        }
        errno_t retValue = strcpy_s(g_deviceIdAddrMap[i].deviceId, DM_MAX_DEVICE_ID_LEN, deviceId);
        if (retValue != EOK) {
            DMLOGE("failed to copy deviceId to addr map.");
            return false;
        }
        g_deviceIdAddrMap[i].valid = true;
        g_deviceIdAddrMap[i].connectAddr = deviceInfo->connectAddr;
        return true;
    }
    DMLOGE("addr map not memory for a new deviceid.");
    return false;
}

static int GetConnAddrByDeviceId(const char *deviceId, ConnectionAddr *addr)
{
    (void)addr;
    addr = NULL;
    if (deviceId == NULL) {
        DMLOGE("get connect addr failed input param is null.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    for (int i = 0; i < DM_MAX_DEVICE_SIZE; i++) {
        if (g_deviceIdAddrMap[i].valid && strcmp(g_deviceIdAddrMap[i].deviceId, deviceId) == 0) {
            DMLOGE("deviceId has been exist in addr map.");
            addr = &(g_deviceIdAddrMap[i].connectAddr);
            return DM_OK;
        }
    }
    return ERR_DM_FAILED;
}

static void OnDiscoveryDeviceFound(const DeviceInfo *deviceInfo)
{
    if (deviceInfo == NULL) {
        DMLOGE("deviceInfo is Null.");
        return;
    }

    if (LockDmGlobalLock() != DM_OK) {
        DMLOGE("LockDmGlobalLock failed.");
        return;
    }
    DMLOGI("will notify the user that a new device has been discovered.");
    DmDeviceInfo dmDeviceInfo;
    DeviceInfoCopyToDmDevice(&dmDeviceInfo, deviceInfo);
    if (FilterDevice(&dmDeviceInfo) == DM_OK) {
        ImportToDevIdMap(dmDeviceInfo.networkId, dmDeviceInfo.deviceId);
        DmDeviceBasicInfo dmBasicInfo;
        DmDeviceInfoToDmBasicInfo(&dmDeviceInfo, &dmBasicInfo);
        ImportDeviceToAddrMap(&dmDeviceInfo);
        g_discoveryCallbackMap.discoveryCallback.onTargetFound(&dmBasicInfo);
    }

    if (UnlockDmGlobalLock() != DM_OK) {
        DMLOGE("UnlockDmGlobalLock failed.");
        return;
    }
    DMLOGI("OnDiscoveryDeviceFound callback complete.");
}

static void OnRefreshDiscoveryResult(int32_t refreshId, RefreshResult reason)
{
    if (reason == REFRESH_LNN_SUCCESS) {
        DMLOGI("refresh discovery result successfully with refreshId: %d.", refreshId);
        g_startDiscoveryFlag = true;
    } else {
        DMLOGI("failed to refresh discovery result with refreshId: %d, reason: %d.", refreshId, (int)reason);
        g_startDiscoveryFlag = false;
    }
    if (UnlockDmGlobalLock() != DM_OK) {
        DMLOGE("UnlockDmGlobalLock failed.");
        return;
    }
}

static void OnPublishLNNResult(int publishId, PublishResult reason)
{
    if (g_advertisingCallbackMap.advCallback.onAdvertisingResult != NULL) {
        g_advertisingCallbackMap.advCallback.onAdvertisingResult(publishId, reason);
    }

    if (reason == PUBLISH_LNN_SUCCESS) {
        DMLOGI("publishLNN successfully with publishId: %d.", publishId);
    } else {
        DMLOGI("failed to publishLNN with publishId: %d, reason: %d.", publishId, (int)reason);
    }
}

static bool ImportPkgNameToAdvertisingMap(const char *pkgName, OnAdvertisingResult cb)
{
    errno_t retValue = strcpy_s(g_advertisingCallbackMap.pkgName, DM_MAX_PKG_NAME_LEN, pkgName);
    if (retValue != EOK) {
        DMLOGE("failed to copy pkgName: %s to advertising map.", pkgName);
        return false;
    }
    g_advertisingCallbackMap.advCallback = cb;
    g_advertisingCallbackMap.valid = true;
    return true;
}

static bool ImportPkgNameToBindMap(const char *pkgName, OnBindResult cb)
{
    errno_t retValue = strcpy_s(g_bindRetCallbackMap.pkgName, DM_MAX_PKG_NAME_LEN, pkgName);
    if (retValue != EOK) {
        DMLOGE("failed to copy pkgName: %s to advertising map.", pkgName);
        return false;
    }
    g_bindRetCallbackMap.bindRetCallback = cb;
    g_bindRetCallbackMap.valid = true;
    return true;
}

int StartSoftbusPublish(const char *pkgName, OnAdvertisingResult cb)
{
    DMLOGI("StartSoftbusPublish start.");
    if (!IsPkgNameValid(pkgName)) {
        DMLOGE("pkgName is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    PublishInfo publishInfo;
    publishInfo.publishId = DEVICEMANAGER_SA_ID;
    publishInfo.mode = DISCOVER_MODE_PASSIVE;
    publishInfo.medium = AUTO;
    publishInfo.freq = HIGH;
    publishInfo.capability = DM_CAPABILITY_OSD;
    publishInfo.capabilityData = NULL;
    publishInfo.dataLen = 0;
    int retValue = PublishLNN(pkgName, &publishInfo, &g_publishLNNCallback);
    if (retValue != SOFTBUS_OK) {
        DMLOGE("failed to call softbus publishLNN function with ret: %d.", retValue);
        return ERR_DM_SOFTBUS_PUBLISH_LNN;
    }
    g_publishLNNFlag = true;
    if (!ImportPkgNameToAdvertisingMap(pkgName, cb)) {
        DMLOGE("failed to import pkgName to advertising map.");
        return ERR_DM_IMPORT_PKGNAME;
    }
    DMLOGI("StartSoftbusPublish end.");
    return DM_OK;
}

int StopSoftbusPublish(const char *pkgName)
{
    DMLOGI("StopSoftbusPublish start.");
    if (!g_publishLNNFlag) {
        DMLOGI("stop publish already stopped.");
        return DM_OK;
    }
    if (!IsPkgNameValid(pkgName)) {
        DMLOGE("pkgName is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    int retValue = StopPublishLNN(pkgName, DEVICEMANAGER_SA_ID);
    if (retValue != SOFTBUS_OK) {
        DMLOGE("failed to call stop softbus publishLNN function with ret: %d.", retValue);
        return ERR_DM_SOFTBUS_STOP_PUBLISH_LNN;
    }
    g_publishLNNFlag = false;
    g_advertisingCallbackMap.valid = false;
    DMLOGI("StopSoftbusPublish end.");
    return DM_OK;
}

static void OnSoftbusDeviceOnline(NodeBasicInfo *deviceInfo)
{
    if (deviceInfo == NULL) {
        DMLOGE("deviceInfo is NULL.");
        return;
    }
    DMLOGI("softbus notify that a device goes online.");
    DmDeviceBasicInfo dmDeviceInfo;
    NodeBasicInfoCopyToDmDevice(&dmDeviceInfo, deviceInfo);
    if (LockDmGlobalLock() != DM_OK) {
        DMLOGE("LockDmGlobalLock failed.");
        return;
    }
    for (int i = 0; i < DM_MAX_REG_PKG_NUMBER; i++) {
        if (g_stateCallbackMap[i].valid) {
            DMLOGI("notify device to go online with pkgName: %s.", g_stateCallbackMap[i].pkgName);
            g_stateCallbackMap[i].stateCallback.onTargetOnline(&dmDeviceInfo);
        }
    }
    if (UnlockDmGlobalLock() != DM_OK) {
        DMLOGE("UnlockDmGlobalLock failed.");
        return;
    }
    DMLOGI("OnSoftbusDeviceOnline callback complete.");
}

static void OnSoftbusDeviceOffline(NodeBasicInfo *deviceInfo)
{
    if (deviceInfo == NULL) {
        DMLOGE("deviceInfo is NULL.");
        return;
    }
    DMLOGI("softbus notify that a device goes offline.");
    DmDeviceBasicInfo dmDeviceInfo;
    NodeBasicInfoCopyToDmDevice(&dmDeviceInfo, deviceInfo);
    if (LockDmGlobalLock() != DM_OK) {
        DMLOGE("LockDmGlobalLock failed.");
        return;
    }
    for (int i = 0; i < DM_MAX_REG_PKG_NUMBER; i++) {
        if (g_stateCallbackMap[i].valid) {
            DMLOGI("notify device to go offline with pkgName: %s.", g_stateCallbackMap[i].pkgName);
            g_stateCallbackMap[i].stateCallback.onTargetOffline(&dmDeviceInfo);
        }
    }
    if (UnlockDmGlobalLock() != DM_OK) {
        DMLOGE("UnlockDmGlobalLock failed.");
        return;
    }
    DMLOGI("OnSoftbusDeviceOffline callback complete.");
}

static void NodeBasicInfoCopyToDmDevice(DmDeviceBasicInfo *dmDeviceInfo, const NodeBasicInfo *nodeBasicInfo)
{
    if (memset_s(dmDeviceInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo)) != EOK) {
        LOGE("NodeBasicInfoCopyToDmDevice memset_s failed.");
        return;
    }

    if (strcpy_s(dmDeviceInfo->deviceName, sizeof(dmDeviceInfo->deviceName), nodeBasicInfo->deviceName) != EOK) {
        DMLOGE("failed to copy device name.");
        return;
    }

    if (strcpy_s(dmDeviceInfo->networkId, sizeof(dmDeviceInfo->networkId), nodeBasicInfo->networkId) != EOK) {
        DMLOGE("failed to copy networkId.");
        return;
    }
    
    GetDeviceIdByNetworkId(nodeBasicInfo->networkId, dmDeviceInfo->deviceId);
    dmDeviceInfo->deviceTypeId = nodeBasicInfo->deviceTypeId;
}

static void DeviceInfoCopyToDmDevice(DmDeviceInfo *dmDeviceInfo, const DeviceInfo *deviceInfo)
{
    const size_t arrayStartPosition = 0;
    if (memset_s(dmDeviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo)) != EOK) {
        DMLOGE("failed to memset device id.");
        return;
    }

    if (strcpy_s(dmDeviceInfo->deviceId, sizeof(dmDeviceInfo->deviceId), deviceInfo->devId) != EOK) {
        DMLOGE("failed to copy device id.");
        return;
    }

    if (strcpy_s(dmDeviceInfo->deviceName, sizeof(dmDeviceInfo->deviceName), deviceInfo->devName) != EOK) {
        DMLOGE("failed to copy device name.");
        return;
    }
    dmDeviceInfo->credible = deviceInfo->isOnline;
    dmDeviceInfo->deviceTypeId = deviceInfo->devType;
    dmDeviceInfo->range = deviceInfo->range;
    dmDeviceInfo->connectAddr = deviceInfo->addr[arrayStartPosition];
    int ret = GetAuthFormByDeviceId(dmDeviceInfo->deviceId, dmDeviceInfo->authForm);
    if (ret != DM_OK) {
        DMLOGE("failed to get authForm from hichain, ret: %d.", ret);
        dmDeviceInfo->isLocalExistCredential = false;
        dmDeviceInfo->authForm = -1;
        return;
    }
    dmDeviceInfo->isLocalExistCredential = true;
}

static void DmDeviceInfoToDmBasicInfo(const DmDeviceInfo *dmDeviceInfo, DmDeviceBasicInfo *dmBasicInfo)
{
    if (memset_s(dmBasicInfo, sizeof(DmDeviceBasicInfo), 0, sizeof(DmDeviceBasicInfo)) != EOK) {
        DMLOGE("DmDeviceInfoToDmBasicInfo memset_s failed");
        return;
    }
    if (strcpy_s(dmBasicInfo->deviceId, sizeof(dmBasicInfo->deviceId), dmDeviceInfo->deviceId) != EOK) {
        DMLOGE("failed to copy device id.");
        return;
    }
    if (strcpy_s(dmBasicInfo->deviceName, sizeof(dmBasicInfo->deviceName), dmDeviceInfo->deviceName) != EOK) {
        DMLOGE("failed to copy device name.");
        return;
    }
    if (strcpy_s(dmBasicInfo->networkId, sizeof(dmBasicInfo->networkId), dmDeviceInfo->networkId) != EOK) {
        DMLOGE("failed to copy device networkId.");
        return;
    }
    dmBasicInfo->deviceTypeId = dmDeviceInfo->deviceTypeId;
}

static bool IsSupportBindType(const int bindType)
{
    if (bindType != SUPPORT_BIND_TYPE) {
        DMLOGE("bindType %d is not supported.", bindType);
        return false;
    }
    return true;
}

static bool IsValidBindTargetInput(const char *pkgName, const char *deviceId, OnBindResult callback)
{
    if (!IsPkgNameValid(pkgName)) {
        DMLOGE("pkgName is invalid.");
        return false;
    }
    if (!IsDeviceIdValid(deviceId)) {
        DMLOGE("deviceId is invalid.");
        return false;
    }
    if (callback.onBindResult == NULL) {
        DMLOGE("callback is null.");
        return false;
    }

    return true;
}

int SoftbusBindTarget(const char *pkgName, const char *deviceId, const int bindType, OnBindResult callback)
{
    DMLOGI("SoftbusBindTarget start.");
    if (!IsValidBindTargetInput(pkgName, deviceId, callback)) {
        DMLOGE("input parameter is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    if (!ImportPkgNameToBindMap(pkgName, callback)) {
        DMLOGE("import pkg name to bind result map failed.");
        return ERR_DM_FAILED;
    }
    errno_t retValue = strcpy_s(g_bindDeviceId, DM_MAX_DEVICE_ID_LEN, deviceId);
    if (retValue != EOK) {
        DMLOGE("failed to copy pkgName: %s to state map.", pkgName);
        return false;
    }
    if (GetConnAddrByDeviceId(deviceId, &g_bindAddr) != DM_OK) {
        DMLOGE("bind invalid addr.");
        return ERR_DM_BIND_NO_ADDR;
    }
    if (!IsSupportBindType(bindType)) {
        DMLOGE("bind type is not supported.");
        return ERR_DM_BIND_TYPE_NOT_SUPPORT;
    }
    g_bindType = bindType;
    int authForm = -1;
    if (GetAuthFormByDeviceId(deviceId, authForm) != DM_OK) {
        DMLOGE("local device is not import credential.");
        return ERR_DM_NO_CREDENTIAL;
    }

    if (OpenAuthSession(deviceId, &g_bindAddr, 1, NULL) != SOFTBUS_OK) {
        DMLOGE("open auth session failed");
        return ERR_DM_SOFTBUS_OPEN_AUTH_SESSION_FAILED;
    }

    if (LockDmBindLock() != DM_OK) {
        DMLOGE("LockDmBindLock failed.");
        return ERR_DM_FAILED;
    }
    DMLOGI("SoftbusBindTarget end.");
    return DM_OK;
}

static char* CreateNegotiateMsg(void)
{
    cJSON *msg = cJSON_CreateObject();
    if (msg == NULL) {
        DMLOGE("failed to create cjson object.");
        return NULL;
    }
    if (cJSON_AddNumberToObject(msg, FILED_MSG_TYPE, MSG_NEGOTIATE) == NULL) {
        DMLOGE("failed to add msg type to cjson object.");
        cJSON_Delete(msg);
        return NULL;
    }
    if (cJSON_AddNumberToObject(msg, FILED_BIND_TYPE, g_bindType) == NULL) {
        cJSON_Delete(msg);
        DMLOGE("failed to add bind type to cjson object.");
        return NULL;
    }
    char *retStr = cJSON_Print(msg);
    cJSON_Delete(msg);
    if (retStr == NULL) {
        DMLOGE("failed to print string from cjson object.");
        return NULL;
    }
    return retStr;
}

static int AbilityNegotiate(const int bindType)
{
    char deviceUdid[DM_MAX_DEVICE_UDID_LEN + 1] = {0};
    int retValue = GetDevUdid(deviceUdid, DM_MAX_DEVICE_UDID_LEN);
    if (retValue != DM_OK) {
        DMLOGE("failed to get local device udid with ret: %d.", retValue);
        return retValue;
    }
    int authType = -1;
    bool isCredentialExist = false;
    retValue = GetAuthFormByDeviceId(deviceUdid, authType);
    if (retValue == DM_OK) {
        isCredentialExist = true;
    }
    bool isSupportBindType = IsSupportBindType(bindType);
    int reply = ERR_DM_FAILED;
    if (isCredentialExist && isSupportBindType) {
        reply = DM_OK;
    }
    return reply;
}

static char* CreateRespNegotiateMsg(const int bindType)
{
    cJSON *msg = cJSON_CreateObject();
    if (msg == NULL) {
        DMLOGE("failed to create cjson object.");
        return NULL;
    }
    if (cJSON_AddNumberToObject(msg, FILED_MSG_TYPE, MSG_NEGOTIATE_RESP) == NULL) {
        DMLOGE("failed to add msg type to cjson object.");
        cJSON_Delete(msg);
        return NULL;
    }
    int reply = AbilityNegotiate(bindType);
    if (reply != DM_OK) {
        DMLOGE("failed to AbilityNegotiate with ret: %d.", reply);
        cJSON_Delete(msg);
        return NULL;
    }

    if (cJSON_AddNumberToObject(msg, FILED_REPLY, reply) == NULL) {
        DMLOGE("failed to add reply to cjson object.");
        cJSON_Delete(msg);
        return NULL;
    }

    char *retStr = cJSON_Print(msg);
    cJSON_Delete(msg);
    if (retStr == NULL) {
        DMLOGE("failed to print string from cjson object.");
        return NULL;
    }
    return retStr;
}

static int OnSessionOpened(int sessionId, int result)
{
    if (result != SOFTBUS_OK) {
        DMLOGE("open auth session failed, ret: %d.", result);
        CloseSession(sessionId);
        return ERR_DM_FAILED;
    }
    g_sessionId = sessionId;
    int sessionSide = GetSessionSide(sessionId);
    if (sessionSide != SESSION_SIDE_CLIENT) {
        DMLOGI("not client session.");
        return DM_OK;
    }
    char *msg = CreateNegotiateMsg();
    if (msg == NULL) {
        DMLOGE("Create negotiate empty msg.");
        return ERR_DM_FAILED;
    }
    int ret = SendBytes(sessionId, msg, strlen(msg));
    if (ret != SOFTBUS_OK) {
        DMLOGE("send byte failed, ret: %d.", ret);
        cJSON_free(msg);
        return ERR_DM_FAILED;
    }
    cJSON_free(msg);
    return DM_OK;
}

static void OnSessionClosed(int sessionId)
{
    g_sessionId = -1;
    DMLOGI("session %d closed.", sessionId);
}

static void OnBytesReceived(int sessionId, const void *data, unsigned int dataLen)
{
    if (data == NULL) {
        DMLOGE("on byte received empty msg.");
        return;
    }
    DMLOGI("on byte received sessionId: %d.", sessionId);
    cJSON *msg = cJSON_Parse(data);
    if (msg == NULL) {
        DMLOGE("on byte received parse msg failed.");
        cJSON_Delete(msg);
        return;
    }
    cJSON *object = cJSON_GetObjectItem(msg, FILED_MSG_TYPE);
    int cmd = 0;
    if (object != NULL && cJSON_IsNumber(object)) {
        cmd = object->valueint;
        DMLOGI("on byte received cmd: %d.", cmd);
    }
    cJSON_Delete(msg);
    if (cmd == MSG_NEGOTIATE) {
        ProcessSinkMsg(data, dataLen);
        return;
    }
    if (cmd == MSG_NEGOTIATE_RESP) {
        ProcessSourceMsg(data, dataLen);
        return;
    }
    DMLOGE("unrecognized message.");
}

static void ProcessSinkMsg(const char *data, unsigned int dataLen)
{
    (void)dataLen;
    cJSON *msgData = cJSON_Parse(data);
    if (msgData == NULL) {
        DMLOGE("on byte received parse msg failed.");
        return;
    }
    cJSON *object = cJSON_GetObjectItem(msgData, FILED_BIND_TYPE);
    cJSON_Delete(msgData);
    int bindType = -1;
    if (object != NULL && cJSON_IsNumber(object)) {
        bindType = object->valueint;
        DMLOGI("on byte received bindType: %d.", bindType);
    }

    char *retStr = CreateRespNegotiateMsg(bindType);
    if (retStr == NULL) {
        DMLOGE("failed to create response negotiate message.");
        return;
    }

    if (SendBytes(g_sessionId, retStr, strlen(retStr)) != SOFTBUS_OK) {
        DMLOGE("send bytes failed, cmd: %d.", MSG_NEGOTIATE_RESP);
        cJSON_free(retStr);
        return;
    }
    cJSON_free(retStr);
}

static void OnJoinLNNCallback(ConnectionAddr *addr, const char *networkId, int32_t retCode)
{
    (void)addr;
    if (g_bindRetCallbackMap.bindRetCallback.onBindResult != NULL) {
        g_bindRetCallbackMap.bindRetCallback.onBindResult(networkId, retCode);
    }
    if (retCode != SOFTBUS_OK) {
        DMLOGE("joinlnn failed, ret: %d.", retCode);
        return;
    }
    if (UnlockDmBindLock() != DM_OK) {
        DMLOGE("UnlockDmBindLock failed.");
        return;
    }
}

static void ProcessSourceMsg(const char *data, unsigned int dataLen)
{
    (void)dataLen;
    cJSON *msg = cJSON_Parse(data);
    if (msg == NULL) {
        DMLOGE("on byte received parse msg failed.");
        cJSON_Delete(msg);
        return;
    }
    cJSON *object = cJSON_GetObjectItem(msg, FILED_IS_CRE_EXISTED);
    if (object == NULL || !cJSON_IsBool(object)) {
        cJSON_Delete(msg);
        DMLOGE("on byte received get isCreExisted failed.");
        return;
    }
    bool isCreExist = object->valueint;
    object = cJSON_GetObjectItem(msg, FILED_IS_BIND_TYPE_SUPPORTED);
    if (object == NULL || !cJSON_IsBool(object)) {
        cJSON_Delete(msg);
        DMLOGE("on byte received get isBindTypeSupported failed.");
        return;
    }
    bool isBindTypeSupported = object->valueint;
    cJSON_Delete(msg);

    if (isBindTypeSupported == false || isCreExist == false) {
        DMLOGE("remote client no credential or not support bind type.");
        return;
    }
    JoinLNN(DM_PKG_NAME, &g_bindAddr, g_joinLNNResult);
    CloseSession(g_sessionId);
    UINT32 osRet = LOS_SemPost(g_bindSem);
    if (osRet != LOS_OK) {
        DMLOGE("failed to post bind sem with ret: %u.", osRet);
        return;
    }
}

static void OnLeaveLNNCallback(const char *networkId, int32_t retCode)
{
    DMLOGI("leave LNN called, retCode: %d.", retCode);
}

int SoftbusUnBindTarget(const char *pkgName, const char *networkId)
{
    DMLOGI("SoftbusUnBindTarget start.");
    if (!IsPkgNameValid(pkgName)) {
        DMLOGE("pkg name is invalid.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    if (networkId == NULL) {
        DMLOGE("input network id is null.");
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    size_t len = strlen(networkId);
    if (len == 0 || len >= DM_MAX_DEVICE_NETWORKID_LEN) {
        DMLOGE("not meet the condition with network id len: %u.", len);
        return ERR_DM_INPUT_INVALID_VALUE;
    }
    LeaveLNN(DM_PKG_NAME, networkId, g_leaveLNNResult);
    DMLOGI("SoftbusUnBindTarget stop.");
    return DM_OK;
}