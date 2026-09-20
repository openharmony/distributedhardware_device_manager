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


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "securec.h"
#include "dm_crypto.h"

#include "permission_manager.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_container.h"
#include "dm_device_info_c.h"
#include "deviceprofile_connector.h"
#include "dm_local_acl_profile.h"
#include "device_manager_service_listener.h"
#include "device_manager_service.h"
#include "softbus_bus_center.h"
#include "softbus_listener_c.h"
#include "dm_constants.h"
#include "kv_adapter_manager.h"
#include "dm_lite_client_notify.h"
#include "ipc_skeleton.h"
#include "serializer.h"
#include "samgr_lite.h"
#include "iproxy_client.h"
#include "device_auth.h"
#include "cJSON.h"
#include "ipc_def.h"
#include "dm_softbus_cache.h"

#define DM_PKG_NAME "ohos.distributedhardware.devicemanager"
#define DM_SOFTBUS_RECOVERY_SLEEP_SEC 5
#define DM_SOFTBUS_RETRY_COUNT 60

void OnNodeOnline(NodeBasicInfo* info)
{
    LOGI("OnNodeOnline networkId=%s", info->networkId);
    DmDeviceInfo deviceInfo;
    (void)memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (strncpy_s(deviceInfo.networkId, sizeof(deviceInfo.networkId),
        info->networkId, sizeof(deviceInfo.networkId) - 1) != 0) {
        return;
    }
    if (strncpy_s(deviceInfo.deviceName, sizeof(deviceInfo.deviceName),
        info->deviceName, sizeof(deviceInfo.deviceName) - 1) != 0) {
        return;
    }
    deviceInfo.deviceTypeId = info->deviceTypeId;

    char peerUdid[UDID_BUF_LEN] = {0};
    if (GetNodeKeyInfo(DM_PKG_NAME, info->networkId, NODE_KEY_UDID,
        (uint8_t*)peerUdid, sizeof(peerUdid)) == 0) {
        char localUdid[UDID_BUF_LEN] = {0};
        NodeBasicInfo localInfo;
        if (GetLocalNodeDeviceInfo(DM_PKG_NAME, &localInfo) == 0) {
            GetNodeKeyInfo(DM_PKG_NAME, localInfo.networkId, NODE_KEY_UDID,
                (uint8_t*)localUdid, sizeof(localUdid));
        }
        uint32_t bindType = DmDpConnectorCheckBindType(peerUdid, localUdid);
        if (bindType == DM_IDENTICAL_ACCOUNT_TYPE) {
            deviceInfo.authForm = DM_AUTH_FORM_IDENTICAL_ACCOUNT;
        } else if (bindType == DM_DEVICE_PEER_TO_PEER_TYPE) {
            deviceInfo.authForm = DM_AUTH_FORM_PEER_TO_PEER;
        } else if (bindType == DM_DEVICE_ACROSS_ACCOUNT_TYPE) {
            deviceInfo.authForm = DM_AUTH_FORM_ACROSS_ACCOUNT;
        }
        LOGI("OnNodeOnline bindType=%u authForm=%d", bindType, deviceInfo.authForm);
    }

    if (peerUdid[0] != '\0') {
        DmString udidStr = DmStringCreate(peerUdid);
        char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
        if (DmGetUdidHashBuf(&udidStr, (unsigned char*)udidHash) == DM_OK) {
            strncpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId),
                udidHash, sizeof(deviceInfo.deviceId) - 1);
        }
        DmStringDestroy(&udidStr);
    }
    SoftbusCache* cache = SoftbusCacheGetInstance();
    if (cache != NULL) {
        DmSoftbusCacheSaveDeviceInfo(cache, &deviceInfo);
        DmSoftbusCacheSaveDeviceSecurityLevel(cache, deviceInfo.networkId);
        DmSoftbusCacheSaveLocalDeviceInfo(cache);
    }
    DmLiteClientNotifyDeviceState(DEVICE_STATE_ONLINE, &deviceInfo, true);
    DmSoftbusListenerOnSoftbusDeviceOnline(info);
}

static bool DmBackfillOfflineDeviceInfo(DmDeviceInfo* deviceInfo, const char* networkId)
{
    SoftbusCache* cache = SoftbusCacheGetInstance();
    if (cache == NULL) {
        return true;
    }
    DmString networkIdStr = DmStringCreate(networkId);
    DmDeviceInfo cachedInfo;
    DmDeviceInfoInit(&cachedInfo);
    if (DmSoftbusCacheGetDevInfoByNetworkId(cache, &networkIdStr, &cachedInfo) == DM_OK) {
        if (strncpy_s(deviceInfo->deviceName, sizeof(deviceInfo->deviceName),
            cachedInfo.deviceName, sizeof(deviceInfo->deviceName) - 1) != 0) {
            DmDeviceInfoDestroy(&cachedInfo);
            DmStringDestroy(&networkIdStr);
            return false;
        }
        deviceInfo->deviceTypeId = cachedInfo.deviceTypeId;
        if (deviceInfo->deviceId[0] == '\0' && cachedInfo.deviceId[0] != '\0') {
            if (strncpy_s(deviceInfo->deviceId, sizeof(deviceInfo->deviceId),
                cachedInfo.deviceId, sizeof(deviceInfo->deviceId) - 1) != 0) {
                DmDeviceInfoDestroy(&cachedInfo);
                DmStringDestroy(&networkIdStr);
                return false;
            }
        }
        LOGI("OnNodeOffline backfill deviceName from SoftbusCache");
    }
    DmDeviceInfoDestroy(&cachedInfo);
    DmStringDestroy(&networkIdStr);
    return true;
}

void OnNodeOffline(NodeBasicInfo* info)
{
    LOGI("OnNodeOffline networkId=%s", info->networkId);
    DmDeviceInfo deviceInfo;
    (void)memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (strncpy_s(deviceInfo.networkId, sizeof(deviceInfo.networkId),
        info->networkId, sizeof(deviceInfo.networkId) - 1) != 0) {
        return;
    }
    if (strncpy_s(deviceInfo.deviceName, sizeof(deviceInfo.deviceName),
        info->deviceName, sizeof(deviceInfo.deviceName) - 1) != 0) {
        return;
    }
    deviceInfo.deviceTypeId = info->deviceTypeId;
    if (deviceInfo.deviceName[0] == '\0') {
        if (!DmBackfillOfflineDeviceInfo(&deviceInfo, info->networkId)) {
            return;
        }
    }
    DmLiteClientNotifyDeviceState(DEVICE_STATE_OFFLINE, &deviceInfo, false);
    SoftbusCache* cache = SoftbusCacheGetInstance();
    if (cache != NULL) {
        DmSoftbusCacheDeleteDeviceInfoByNode(cache, &deviceInfo);
        DmSoftbusCacheDeleteDeviceSecurityLevel(cache, deviceInfo.networkId);
    }
    DmSoftbusListenerOnSoftbusDeviceOffline(info);
}

void OnNodeBasicInfoChanged(NodeBasicInfoType type, NodeBasicInfo* info)
{
    LOGI("OnNodeBasicInfoChanged type=%d", type);
    if (info == NULL) {
        LOGE("OnNodeBasicInfoChanged info is NULL");
        return;
    }
    DmSoftbusListenerOnSoftbusDeviceInfoChanged(type, info);
}

static INodeStateCb g_nodeStateCb = {
    .events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE | EVENT_NODE_STATE_INFO_CHANGED,
    .onNodeOnline = OnNodeOnline,
    .onNodeOffline = OnNodeOffline,
    .onNodeBasicInfoChanged = OnNodeBasicInfoChanged,
};

int32_t OnCommand(int32_t code, const char* value, uint32_t inlen,
    char* res, uint32_t resLen)
{
    if (value == NULL || inlen == 0) {
        if (res != NULL && resLen > 0) {
            res[0] = '1';
        }
        return ERR_DM_INPUT_PARA_INVALID;
    }
    if (code != 0) {
        if (res != NULL && resLen > 0) {
            res[0] = '1';
        }
        return ERR_DM_FAILED;
    }

    DmLocalAclProfile acl;
    DmLocalAclProfileInit(&acl);
    if (!DmLocalAclProfileDeserialize(&acl, value)) {
        LOGE("onCommand deserialize ACL JSON failed");
        DmLocalAclProfileDestroy(&acl);
        if (res != NULL && resLen > 0) {
            res[0] = '1';
        }
        return ERR_DM_FAILED;
    }
    int32_t ret = DmAclStorePut(&acl);
    DmLocalAclProfileDestroy(&acl);
    LOGW("onCommand DmAclStorePut ret=%d", ret);
    if (res != NULL && resLen > 0) {
        res[0] = (ret == DM_OK) ? '0' : '1';
    }
    return ret;
}

ITrustedDeviceCb g_commandCb = {
    .onCommand = OnCommand,
};

static void DmNotifyDeviceOffline(SoftbusCache* cache, const char* remoteUdidBuf, DmDeviceInfo* deviceInfo)
{
    if (cache != NULL) {
        DmString udid = DmStringCreate(remoteUdidBuf);
        DmString networkId = DmStringCreateEmpty();
        if (DmSoftbusCacheGetNetworkIdFromCache(cache, &udid, &networkId) == DM_OK &&
            DmStringSize(&networkId) > 0) {
            if (strncpy_s(deviceInfo->networkId, sizeof(deviceInfo->networkId),
                DmStringCstr(&networkId), sizeof(deviceInfo->networkId) - 1) != 0) {
                DmStringDestroy(&networkId);
                DmStringDestroy(&udid);
                return;
            }
            if (strncpy_s(deviceInfo->deviceId, sizeof(deviceInfo->deviceId),
                remoteUdidBuf, sizeof(deviceInfo->deviceId) - 1) != 0) {
                DmStringDestroy(&networkId);
                DmStringDestroy(&udid);
                return;
            }
        }
        DmStringDestroy(&networkId);
        DmStringDestroy(&udid);
        DmSoftbusCacheDeleteDeviceInfoByNode(cache, deviceInfo);
    }
    DmLiteClientNotifyDeviceState(DEVICE_STATE_OFFLINE, deviceInfo, false);
}

static void DmProcessSingleAcl(DmLocalAclProfile* p)
{
    DmString json = DmLocalAclProfileSerialize(p);
    const char* jsonStr = DmStringCstr(&json);
    uint32_t jsonLen = (uint32_t)DmStringSize(&json);
    LOGW("OnCredDelete deleted ACL json=%s", jsonStr);

    const char* peerUdid = DmStringCstr(&p->trustDeviceId);
    char key[32];
    DmAclKeyFromProfile(key, sizeof(key), p);
    DmAclStoreDeleteByKey(key);
    DmLocalAclProfileDelete(p);

    int32_t ret = SetCommand(0, jsonStr, jsonLen);
    LOGW("OnCredDelete SetCommand(offline) ret=%d", ret);
    if (ret == 0) {
        DmDeviceInfo deviceInfo;
        (void)memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        SoftbusCache* cache = SoftbusCacheGetInstance();
        DmNotifyDeviceOffline(cache, peerUdid, &deviceInfo);
    }
    DmStringDestroy(&json);
}

void OnCredDelete(const char* credId, const char* credInfo)
{
    (void)credInfo;
    if (credId == NULL || credId[0] == '\0') {
        LOGE("OnCredDelete credId is NULL");
        return;
    }
    LOGW("OnCredDelete credId=%.10s", credId);

    DmVecVoid profiles;
    DmVecVoidInit(&profiles);
    DmAclStoreGetByCredId(&profiles, credId);
    int32_t deleted = DmVecVoidSize(&profiles);
    LOGW("OnCredDelete found %d ACL entries for credId=%.10s", deleted, credId);

    for (int32_t i = 0; i < deleted; i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&profiles, i);
        if (p == NULL) {
            continue;
        }
        DmProcessSingleAcl(p);
        profiles.data[i] = NULL;
    }
    free(profiles.data);
}

static CredChangeListener g_credChangeListener = {
    .onCredDelete = OnCredDelete,
};

int32_t DmServiceTestTriggerAclWrite(const char* json, uint32_t len)
{
    if (json == NULL || len == 0) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    LOGW("[test_stub] DmServiceTestTriggerAclWrite len=%u", len);
    char res[8] = {0};
    return OnCommand(0, json, len, res, sizeof(res));
}

int32_t DmServiceTestTriggerCredDelete(const char* credId, const char* credInfo)
{
    LOGW("[test_stub] DmServiceTestTriggerCredDelete credId=%s", credId ? credId : "NULL");
    OnCredDelete(credId, credInfo);
    return DM_OK;
}

static void DmLogCallbackReg(FILE* df, unsigned int h, uintptr_t t, uintptr_t c, int32_t aclRet)
{
    (void)fprintf(df, "[register_callbacks] softbus identity handle=%u token=%u cookie=%u\n",
        h, (unsigned int)t, (unsigned int)c);
    (void)fflush(df);
    if (c == 0) {
        (void)fprintf(df, "[register_callbacks] FAILED null cookie\n");
        (void)fflush(df);
        (void)fclose(df);
        return;
    }
    (void)fprintf(df, "[register_callbacks] SUCCESS\n");
    (void)fflush(df);
    (void)fprintf(df, "[register_callbacks] RegisterCommandCb ret=%d\n", aclRet);
    (void)fflush(df);
    (void)fclose(df);
}

int32_t DmSoftbusListenerRegisterCallbacksInner(void)
{
    FILE* df = fopen("/userdata/dm_debug.log", "a");
    if (df != NULL) {
        (void)fprintf(df, "[register_callbacks] BEGIN pid=%d\n", getpid());
        (void)fflush(df);
    }

    int32_t ret = RegNodeDeviceStateCb(DM_PKG_NAME, &g_nodeStateCb);
    if (df != NULL) {
        (void)fprintf(df, "[register_callbacks] RegNodeDeviceStateCb ret=%d\n", ret);
        (void)fflush(df);
    }
    LOGW("[register_callbacks] RegNodeDeviceStateCb ret=%d", ret);
    if (ret != 0) {
        LOGE("RegNodeDeviceStateCb failed %d", ret);
        if (df != NULL) {
            (void)fprintf(df, "[register_callbacks] RegNodeDeviceStateCb FAILED ret=%d\n", ret);
            (void)fflush(df);
            (void)fclose(df);
        }
        return ERR_DM_FAILED;
    }

    unsigned int h = 0;
    uintptr_t t = 0;
    uintptr_t c = 0;
    GetClientIdentity(&h, &t, &c);
    LOGW("[register_callbacks] softbus-managed client identity handle=%u token=%u cookie=%u",
         h, (unsigned int)t, (unsigned int)c);
    if (c == 0) {
        LOGE("[register_callbacks] softbus client cookie is NULL, callbacks cannot be delivered");
        if (df != NULL) {
            DmLogCallbackReg(df, h, t, c, 0);
        }
        return ERR_DM_FAILED;
    }

    LOGW("SoftBus callbacks registered successfully");

    int32_t aclRet = RegisterCommandCb(DM_PKG_NAME, &g_commandCb);
    LOGW("[register_callbacks] RegisterCommandCb ret=%d", aclRet);
    if (df != NULL) {
        DmLogCallbackReg(df, h, t, c, aclRet);
    }

    return DM_OK;
}

static bool g_softbusOnline = false;
static volatile bool g_recoveryRunning = true;

static void DmHandleSoftbusRegResult(int32_t regRet)
{
    if (regRet == DM_OK) {
        g_softbusOnline = true;
        SoftbusCache* cache = SoftbusCacheGetInstance();
        if (cache != NULL) {
            DmSoftbusCacheUpdateDeviceInfoCache(cache);
        }
    } else {
        LOGE("[recovery] re-register failed ret=%d, will retry next cycle", regRet);
    }
}

static void DmTrySoftbusRecovery(void)
{
    if (g_softbusOnline) {
        g_softbusOnline = false;
        LOGE("[recovery] softbus lost, will re-register when recovered");
        return;
    }
    LOGE("[recovery] softbus recovered, re-registering callbacks");
    FILE* df = fopen("/userdata/dm_debug.log", "a");
    if (df != NULL) {
        (void)fprintf(df, "[recovery] softbus recovered, re-registering\n");
        (void)fflush(df);
        (void)fclose(df);
    }
    int32_t regRet = DmSoftbusListenerRegisterCallbacksInner();
    DmHandleSoftbusRegResult(regRet);
}

static void* DmSoftbusRecoveryThread(void* arg)
{
    (void)arg;
    while (g_recoveryRunning) {
        sleep(DM_SOFTBUS_RECOVERY_SLEEP_SEC);
        NodeBasicInfo localInfo;
        int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME, &localInfo);
        if (ret == 0) {
            DmTrySoftbusRecovery();
        }
    }
    return NULL;
}

static IUnknown* DmWaitForSoftbusService(SamgrLite *samgr, FILE* df)
{
    IUnknown *proxy = NULL;
    for (int i = 0; i < DM_SOFTBUS_RETRY_COUNT; i++) {
        proxy = samgr->GetDefaultFeatureApi("softbus_service");
        if (proxy != NULL) {
            LOGE("[early_reg] softbus_service found after %d seconds", i);
            if (df != NULL) {
                (void)fprintf(df, "[early_reg] softbus_service found after %d seconds\n", i);
                (void)fflush(df);
            }
            break;
        }
        if (i == 0) {
            LOGE("[early_reg] waiting for softbus_service in SAMGR...");
            if (df != NULL) {
                (void)fprintf(df, "[early_reg] waiting for softbus_service...\n");
                (void)fflush(df);
            }
        }
        sleep(1);
    }
    if (proxy == NULL) {
        LOGE("[early_reg] softbus_service NOT found after 60s, trying registration anyway");
        if (df != NULL) {
            (void)fprintf(df, "[early_reg] softbus_service NOT found after 60s\n");
            (void)fflush(df);
        }
    }
    return proxy;
}

static void DmRegisterCredListener(void)
{
    InitDeviceAuthService();
    const CredManager* cm = GetCredMgrInstance();
    if (cm != NULL) {
        int32_t credRet = cm->registerChangeListener(DM_PKG_NAME, &g_credChangeListener);
        LOGW("[early_reg] registerChangeListener ret=%d", credRet);
    } else {
        LOGW("[early_reg] GetCredMgrInstance NULL, skip cred listener");
    }
}

static void DmLogRegResult(int32_t ret, SoftbusCache* cache, FILE* df)
{
    if (ret == DM_OK) {
        g_softbusOnline = true;
        LOGE("[early_reg] softbus registration SUCCESS");
        if (df != NULL) {
            (void)fprintf(df, "[early_reg] SUCCESS\n");
            (void)fflush(df);
        }
        if (cache != NULL) {
            DmSoftbusCacheUpdateDeviceInfoCache(cache);
        }
    } else {
        LOGE("[early_reg] softbus registration FAILED %d", ret);
        if (df != NULL) {
            (void)fprintf(df, "[early_reg] FAILED ret=%d\n", ret);
            (void)fflush(df);
        }
    }
}

void DmServiceRegisterLite(void)
{
    DmLiteClientNotifyInit();
    DmServiceInit();
    LOGE("[early_reg] DmServiceRegisterLite BEGIN pid=%d", getpid());
    FILE* df = fopen("/userdata/dm_debug.log", "a");
    if (df != NULL) {
        (void)fprintf(df, "[early_reg] BEGIN pid=%d\n", getpid());
        (void)fflush(df);
    }
    SoftbusCache* cache = SoftbusCacheGetInstance();
    if (cache != NULL) {
        DmSoftbusCacheInit(cache);
    }
    DmKvAdapterManagerInit();
    SamgrLite *samgr = SAMGR_GetInstance();
    if (samgr == NULL) {
        LOGE("[early_reg] SAMGR_GetInstance returned NULL");
        if (df != NULL) {
            (void)fprintf(df, "[early_reg] SAMGR_GetInstance NULL\n");
            (void)fflush(df);
            (void)fclose(df);
        }
        return;
    }
    IUnknown *proxy = DmWaitForSoftbusService(samgr, df);
    (void)proxy;
    sleep(1);
    int32_t ret = DmSoftbusListenerRegisterCallbacksInner();
    DmLogRegResult(ret, cache, df);
    if (df != NULL) {
        (void)fclose(df);
    }
    DmRegisterCredListener();
    pthread_t tid;
    pthread_create(&tid, NULL, DmSoftbusRecoveryThread, NULL);
    pthread_detach(tid);
}
