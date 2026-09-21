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


#include "softbus_listener_c.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "securec.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_constants.h"
#include "dm_local_acl_profile.h"
#include "dm_softbus_cache.h"
#include "dm_crypto.h"

#include "softbus_bus_center.h"
#include "dm_lite_client_notify.h"
#include "cJSON.h"

static void DmFillDeviceIdHash(const char* networkId, DmDeviceInfo* info);

void OnRefreshResult_lite(int32_t refreshId, RefreshResult reason)
{
    LOGI("OnRefreshResult refreshId=%d reason=%d", refreshId, (int)reason);
    DmSoftbusListenerOnSoftbusDiscoveryResult(refreshId, (int)reason);
    DmLiteClientNotifyDiscoverResult(DM_PKG_NAME_LITE, (uint16_t)refreshId, (int32_t)reason);
}

static void DmLiteParseConnAddrInfo(const DeviceInfo* device, DmDeviceInfo* info)
{
    if (device == NULL || info == NULL) {
        return;
    }
    cJSON* json = cJSON_CreateObject();
    if (json == NULL) {
        return;
    }
    if (device->addrNum > 0) {
        for (unsigned int i = 0; i < device->addrNum && i < CONNECTION_ADDR_MAX; i++) {
            const ConnectionAddr* addr = &device->addr[i];
            if (addr->type == CONNECTION_ADDR_WLAN || addr->type == CONNECTION_ADDR_ETH) {
                cJSON_AddStringToObject(json, "WIFI_IP", addr->info.ip.ip);
                cJSON_AddNumberToObject(json, "WIFI_PORT", addr->info.ip.port);
                cJSON_AddStringToObject(json, "CONN_ADDR_TYPE",
                    addr->type == CONNECTION_ADDR_WLAN ? "WLAN_IP_TYPE" : "ETH_IP_TYPE");
                break;
            }
        }
    }
    if (device->custData[0] != '\0') {
        cJSON_AddStringToObject(json, "CUSTOM_DATA", device->custData);
    }
    if (device->capabilityBitmapNum > 0) {
        cJSON_AddNumberToObject(json, "CAPABILITY", device->capabilityBitmap[0]);
    }
    char* jsonStr = cJSON_PrintUnformatted(json);
    if (jsonStr != NULL) {
        DmStringAssignCstr(&info->extraData, jsonStr);
        cJSON_free(jsonStr);
    }
    cJSON_Delete(json);
}

static volatile uint16_t g_activeSubscribeId = 0;

static void DmTryFillNetworkId(SoftbusCache* cache, DmString* udid, char* networkId, size_t netLen)
{
    DmString networkIdStr = DmStringCreateEmpty();
    if (DmSoftbusCacheGetNetworkIdFromCache(cache, udid, &networkIdStr) == DM_OK &&
        DmStringSize(&networkIdStr) > 0) {
        (void)strncpy_s(networkId, netLen, DmStringCstr(&networkIdStr), netLen - 1);
    }
    DmStringDestroy(&networkIdStr);
}

static void DmFillNetworkIdFromCache(DmDeviceInfo* info)
{
    SoftbusCache* cache = SoftbusCacheGetInstance();
    if (cache == NULL) {
        return;
    }
    DmString udidHash = DmStringCreate(info->deviceId);
    DmString udid = DmStringCreateEmpty();
    if (DmSoftbusCacheGetUdidByUdidHash(cache, &udidHash, &udid) == DM_OK &&
        DmStringSize(&udid) > 0) {
        DmTryFillNetworkId(cache, &udid, info->networkId, sizeof(info->networkId));
    }
    DmStringDestroy(&udid);
    DmStringDestroy(&udidHash);
}

void OnDeviceFound_lite(const DeviceInfo* device)
{
    DmSoftbusListenerOnSoftbusDeviceFound(device);
    DmDeviceInfo info;
    (void)memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    if (device != NULL) {
        (void)strncpy_s(info.deviceId, sizeof(info.deviceId), device->devId, sizeof(info.deviceId) - 1);
        (void)strncpy_s(info.deviceName, sizeof(info.deviceName), device->devName, sizeof(info.deviceName) - 1);
        info.deviceTypeId = device->devType;
        info.range = device->range;
        DmLiteParseConnAddrInfo(device, &info);
        DmFillNetworkIdFromCache(&info);
    }
    uint16_t subscribeId = g_activeSubscribeId;
    LOGW("OnDeviceFound_lite subscribeId=%u devType=%d networkId=%s",
         subscribeId, device ? device->devType : -1, info.networkId[0] ? "found" : "empty");
    DmLiteClientNotifyDeviceFound(subscribeId, &info);
    DmDeviceInfoDestroy(&info);
}

static const IRefreshCallback g_refreshCallback = {
    .OnDiscoverResult = OnRefreshResult_lite,
    .OnDeviceFound = OnDeviceFound_lite,
};

void OnPublishResult_lite(int publishId, PublishResult reason)
{
    DmSoftbusListenerOnSoftbusPublishResult(publishId, (int)reason);
    DmLiteClientNotifyPublishResult(publishId, (int32_t)reason);
}

static const IPublishCb g_publishCallback = {
    .OnPublishResult = OnPublishResult_lite,
};

int32_t DmSoftbusListenerGetTrustedDeviceList(DmVec_DmDeviceInfo* deviceInfoList)
{
    CHECK_NULL_RETURN(deviceInfoList, ERR_DM_FAILED);
    SoftbusCache* cache = SoftbusCacheGetInstance();
    if (cache == NULL) {
        LOGE("GetTrustedDeviceList softbus cache instance is NULL");
        return ERR_DM_FAILED;
    }

    DmVecVoid aclList;
    DmVecVoidInit(&aclList);
    DmAclStoreGetAll(&aclList);
    int32_t aclCount = DmVecVoidSize(&aclList);

    for (int32_t i = 0; i < aclCount; i++) {
        DmLocalAclProfile* acl = (DmLocalAclProfile*)*DmVecVoidAt(&aclList, i);
        if (acl == NULL) {
            continue;
        }
        const char* trustUdid = DmStringCstr(&acl->trustDeviceId);
        if (trustUdid == NULL || trustUdid[0] == '\0') {
            continue;
        }

        DmString udid = DmStringCreate(trustUdid);
        if (DmSoftbusCacheCheckIsOnlineByPeerUdid(cache, &udid)) {
            DmString uuid = DmStringCreateEmpty();
            DmDeviceInfo dev;
            DmDeviceInfoInit(&dev);
            if (DmSoftbusCacheGetDeviceInfoByDeviceId(cache, &udid, &uuid, &dev)) {
                DmVec_DmDeviceInfo_Push(deviceInfoList, dev);
            } else {
                DmDeviceInfoDestroy(&dev);
            }
            DmStringDestroy(&uuid);
        }
        DmStringDestroy(&udid);
    }
    for (int32_t i = 0; i < DmVecVoidSize(&aclList); i++) {
        DmLocalAclProfile* p = (DmLocalAclProfile*)*DmVecVoidAt(&aclList, i);
        if (p != NULL) {
            DmLocalAclProfileDelete(p);
            aclList.data[i] = NULL;
        }
    }
    free(aclList.data);

    int32_t count = DmVec_DmDeviceInfo_Size(deviceInfoList);
    LOGW("GetTrustedDeviceList trusted-online count=%d", count);
    return DM_OK;
}

int32_t DmSoftbusListenerGetLocalDeviceInfo(DmDeviceInfo* deviceInfo)
{
    CHECK_NULL_RETURN(deviceInfo, ERR_DM_FAILED);
    NodeBasicInfo localInfo;
    int32_t ret = GetLocalNodeDeviceInfo(DM_PKG_NAME_LITE, &localInfo);
    if (ret != 0) {
        LOGE("GetLocalNodeDeviceInfo failed %d", ret);
        return ERR_DM_FAILED;
    }
    (void)memset_s(deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    (void)strncpy_s(deviceInfo->networkId, sizeof(deviceInfo->networkId),
        localInfo.networkId, sizeof(deviceInfo->networkId) - 1);
    (void)strncpy_s(deviceInfo->deviceName, sizeof(deviceInfo->deviceName),
        localInfo.deviceName, sizeof(deviceInfo->deviceName) - 1);
    deviceInfo->deviceTypeId = localInfo.deviceTypeId;
    DmFillDeviceIdHash(localInfo.networkId, deviceInfo);
    return DM_OK;
}

static void DmFillDeviceIdHash(const char* networkId, DmDeviceInfo* info)
{
    uint8_t udid[UDID_BUF_LEN] = {0};
    if (GetNodeKeyInfo(DM_PKG_NAME_LITE, networkId, NODE_KEY_UDID, udid, sizeof(udid)) != 0) {
        return;
    }
    DmString udidStr = DmStringCreate((const char*)udid);
    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (DmGetUdidHashBuf(&udidStr, (unsigned char*)udidHash) == DM_OK) {
        (void)strncpy_s(info->deviceId, sizeof(info->deviceId), udidHash, sizeof(info->deviceId) - 1);
    }
    DmStringDestroy(&udidStr);
}

static bool DmFindAndFillDeviceInfo(NodeBasicInfo* infoList, int32_t infoNum,
    const char* targetNetId, DmDeviceInfo* info)
{
    for (int32_t i = 0; i < infoNum; i++) {
        if (strcmp(infoList[i].networkId, targetNetId) != 0) {
            continue;
        }
        (void)memset_s(info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
        (void)strncpy_s(info->networkId, sizeof(info->networkId),
            infoList[i].networkId, sizeof(info->networkId) - 1);
        (void)strncpy_s(info->deviceName, sizeof(info->deviceName),
            infoList[i].deviceName, sizeof(info->deviceName) - 1);
        info->deviceTypeId = infoList[i].deviceTypeId;
        DmFillDeviceIdHash(infoList[i].networkId, info);
        return true;
    }
    return false;
}

static int32_t DmGetDeviceInfoFromTrustedList(const DmString* networkId, DmDeviceInfo* info)
{
    NodeBasicInfo* infoList = NULL;
    int32_t infoNum = 0;
    int32_t ret = GetAllNodeDeviceInfo(DM_PKG_NAME_LITE, &infoList, &infoNum);
    if (ret != 0) {
        LOGE("GetAllNodeDeviceInfo failed %d", ret);
        return ERR_DM_FAILED;
    }
    if (infoNum <= 0 || infoList == NULL) {
        FreeNodeInfo(infoList);
        return ERR_DM_FAILED;
    }
    const char* targetNetId = DmStringCstr(networkId);
    bool found = DmFindAndFillDeviceInfo(infoList, infoNum, targetNetId, info);
    FreeNodeInfo(infoList);
    if (!found) {
        LOGE("networkId %s not found in node list", targetNetId);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

static void DmFillDeviceIdFromUdid(DmDeviceInfo* info)
{
    uint8_t udid[UDID_BUF_LEN] = {0};
    if (GetNodeKeyInfo(DM_PKG_NAME_LITE, info->networkId, NODE_KEY_UDID, udid, sizeof(udid)) != 0) {
        return;
    }
    DmString udidStr = DmStringCreate((const char*)udid);
    char udidHash[DM_MAX_DEVICE_ID_LEN] = {0};
    if (DmGetUdidHashBuf(&udidStr, (unsigned char*)udidHash) == DM_OK) {
        (void)strncpy_s(info->deviceId, sizeof(info->deviceId), udidHash, sizeof(info->deviceId) - 1);
    }
    DmStringDestroy(&udidStr);
}

static int32_t DmGetDeviceInfoFromCache(const DmString* networkId, DmDeviceInfo* info)
{
    SoftbusCache* cache = SoftbusCacheGetInstance();
    if (cache == NULL) {
        return ERR_DM_FAILED;
    }
    if (DmSoftbusCacheGetDevInfoByNetworkId(cache, networkId, info) != DM_OK) {
        return ERR_DM_FAILED;
    }
    if (info->deviceId[0] == '\0' && info->networkId[0] != '\0') {
        DmFillDeviceIdFromUdid(info);
    }
    return DM_OK;
}

int32_t DmSoftbusListenerGetDeviceInfo(const DmString* networkId, DmDeviceInfo* info)
{
    CHECK_NULL_RETURN(networkId, ERR_DM_FAILED);
    if (DmGetDeviceInfoFromCache(networkId, info) == DM_OK) {
        return DM_OK;
    }
    return DmGetDeviceInfoFromTrustedList(networkId, info);
}

int32_t DmSoftbusListenerGetUuidByNetworkId(const char* networkId, DmString* uuid)
{
    CHECK_NULL_RETURN(networkId, ERR_DM_FAILED);
    uint8_t buf[UUID_BUF_LEN] = {0};
    if (GetNodeKeyInfo(DM_PKG_NAME_LITE, networkId, NODE_KEY_UUID, buf, sizeof(buf)) == 0) {
        DmStringAssignCstr(uuid, (const char*)buf);
        return DM_OK;
    }
    LOGE("GetNodeKeyInfo UUID failed");
    return ERR_DM_FAILED;
}

int32_t DmSoftbusListenerGetUdidByNetworkId(const char* networkId, DmString* udid)
{
    CHECK_NULL_RETURN(networkId, ERR_DM_FAILED);
    uint8_t buf[UDID_BUF_LEN] = {0};
    if (GetNodeKeyInfo(DM_PKG_NAME_LITE, networkId, NODE_KEY_UDID, buf, sizeof(buf)) == 0) {
        DmStringAssignCstr(udid, (const char*)buf);
        return DM_OK;
    }
    LOGE("GetNodeKeyInfo UDID failed");
    return ERR_DM_FAILED;
}

int32_t DmSoftbusListenerGetDeviceSecurityLevel(const char* networkId, int32_t* securityLevel)
{
    CHECK_NULL_RETURN(networkId, ERR_DM_FAILED);
    uint8_t buf[64] = {0};
    if (GetNodeKeyInfo(DM_PKG_NAME_LITE, networkId, NODE_KEY_DEVICE_SECURITY_LEVEL, buf, sizeof(buf)) == 0) {
        *securityLevel = atoi((const char*)buf);
        return DM_OK;
    }
    LOGE("GetNodeKeyInfo security level failed");
    return ERR_DM_FAILED;
}

int32_t DmSoftbusListenerGetNetworkTypeByNetworkId(const char* networkId, int32_t* networkType)
{
    CHECK_NULL_RETURN(networkId, ERR_DM_FAILED);
    uint8_t buf[64] = {0};
    if (GetNodeKeyInfo(DM_PKG_NAME_LITE, networkId, NODE_KEY_NETWORK_TYPE, buf, sizeof(buf)) == 0) {
        *networkType = atoi((const char*)buf);
        return DM_OK;
    }
    LOGE("GetNodeKeyInfo network type failed");
    return ERR_DM_FAILED;
}

int32_t DmSoftbusListenerRefreshSoftbusLnn(const char* pkgName,
    const DmSubscribeInfo* dmSubInfo, const DmString* customData)
{
    (void)pkgName;
    (void)customData;
    CHECK_NULL_RETURN(dmSubInfo, ERR_DM_FAILED);
    SubscribeInfo subInfo;
    (void)memset_s(&subInfo, sizeof(SubscribeInfo), 0, sizeof(SubscribeInfo));
    subInfo.subscribeId = dmSubInfo->subscribeId;
    subInfo.mode = (DiscoverMode)dmSubInfo->mode;
    subInfo.medium = (ExchangeMedium)dmSubInfo->medium;
    subInfo.freq = (ExchangeFreq)dmSubInfo->freq;
    subInfo.isSameAccount = dmSubInfo->isSameAccount;
    subInfo.isWakeRemote = dmSubInfo->isWakeRemote;
    subInfo.capability = dmSubInfo->capability;
    subInfo.dataLen = 0;
    int32_t ret = RefreshLNN(DM_PKG_NAME_LITE, &subInfo, &g_refreshCallback);
    if (ret != 0) {
        return ERR_DM_FAILED;
    }
    g_activeSubscribeId = (uint16_t)subInfo.subscribeId;
    return DM_OK;
}

int32_t DmSoftbusListenerStopRefreshSoftbusLnn(uint16_t subscribeId)
{
    int32_t ret = StopRefreshLNN(DM_PKG_NAME_LITE, (int32_t)subscribeId);
    if (ret != 0) {
        LOGE("StopRefreshLNN failed %d", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DmSoftbusListenerPublishSoftbusLnn(const DmPublishInfo* dmPubInfo,
    const DmString* capability, const DmString* customData)
{
    PublishInfo pubInfo;
    (void)memset_s(&pubInfo, sizeof(PublishInfo), 0, sizeof(PublishInfo));
    pubInfo.publishId = dmPubInfo->publishId;
    pubInfo.mode = (DiscoverMode)dmPubInfo->mode;
    pubInfo.medium = (ExchangeMedium)dmPubInfo->medium;
    pubInfo.freq = (ExchangeFreq)dmPubInfo->freq;
    pubInfo.capability = DmStringCstr(capability);
    pubInfo.ranging = dmPubInfo->ranging;
    int32_t ret = PublishLNN(DM_PKG_NAME_LITE, &pubInfo, &g_publishCallback);
    if (ret != 0) {
        LOGE("PublishLNN failed %d", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DmSoftbusListenerStopPublishSoftbusLnn(int32_t publishId)
{
    int32_t ret = StopPublishLNN(DM_PKG_NAME_LITE, publishId);
    if (ret != 0) {
        LOGE("StopPublishLNN failed %d", ret);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t DmSoftbusListenerSetLocalDisplayName(const DmString* displayName)
{
    CHECK_NULL_RETURN(displayName, ERR_DM_INPUT_PARA_INVALID);
    const char* name = DmStringCstr(displayName);
    if (name == NULL || name[0] == '\0') {
        LOGE("invalid display name");
        return ERR_DM_INPUT_PARA_INVALID;
    }
    cJSON* json = cJSON_CreateObject();
    if (json == NULL) {
        LOGE("create json failed");
        return ERR_DM_FAILED;
    }
    cJSON_AddStringToObject(json, "raw", name);
    cJSON_AddStringToObject(json, "name18", name);
    cJSON_AddStringToObject(json, "name21", name);
    cJSON_AddStringToObject(json, "name24", name);
    char* jsonStr = cJSON_PrintUnformatted(json);
    if (jsonStr == NULL) {
        cJSON_Delete(json);
        LOGE("print json failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = SetDisplayName(DM_PKG_NAME_LITE, jsonStr, strlen(jsonStr));
    LOGW("SetDisplayName ret=%d name=%s", ret, name);
    cJSON_free(jsonStr);
    cJSON_Delete(json);
    if (ret == 0) {
        char localUdid[UDID_BUF_LEN] = {0};
        NodeBasicInfo localInfo;
        if (GetLocalNodeDeviceInfo(DM_PKG_NAME_LITE, &localInfo) == 0) {
            GetNodeKeyInfo(DM_PKG_NAME_LITE, localInfo.networkId, NODE_KEY_UDID,
                (uint8_t*)localUdid, sizeof(localUdid));
            if (localUdid[0] != '\0') {
                DmAclStoreUpdateDeviceName(localUdid, name);
            }
        }
        return DM_OK;
    }
    return ERR_DM_FAILED;
}

int32_t DmSoftbusListenerRegisterSoftbusLnnOpsCbk(const DmString* pkgName)
{
    CHECK_NULL_RETURN(pkgName, ERR_DM_FAILED);
    LOGI("RegisterSoftbusLnnOpsCbk.");
    return DM_OK;
}

int32_t DmSoftbusListenerUnregisterSoftbusLnnOpsCbk(const DmString* pkgName)
{
    CHECK_NULL_RETURN(pkgName, ERR_DM_FAILED);
    LOGI("UnRegisterSoftbusLnnOpsCbk.");
    return DM_OK;
}

void DmSoftbusListenerOnSoftbusDeviceFound(const void* device)
{
    LOGI("OnSoftbusDeviceFound.");
}

void DmSoftbusListenerOnSoftbusDiscoveryResult(int subscribeId, int result)
{
    LOGI("OnSoftbusDiscoveryResult subscribeId: %d, result: %d.", subscribeId, result);
}

void DmSoftbusListenerOnSoftbusDeviceOnline(void* info)
{
    LOGI("OnSoftbusDeviceOnline.");
}

void DmSoftbusListenerOnSoftbusDeviceOffline(void* info)
{
    LOGI("OnSoftbusDeviceOffline.");
}

void DmSoftbusListenerOnSoftbusDeviceInfoChanged(int type, void* info)
{
    LOGI("OnSoftbusDeviceInfoChanged type: %d.", type);
    if (info == NULL) {
        LOGE("OnSoftbusDeviceInfoChanged info is NULL");
        return;
    }
    if (type != TYPE_DEVICE_NAME) {
        LOGI("not device name change, skip. type=%d", type);
        return;
    }
    NodeBasicInfo* nodeInfo = (NodeBasicInfo*)info;
    DmDeviceInfo deviceInfo;
    (void)memset_s(&deviceInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    (void)strncpy_s(deviceInfo.networkId, sizeof(deviceInfo.networkId),
        nodeInfo->networkId, sizeof(deviceInfo.networkId) - 1);
    (void)strncpy_s(deviceInfo.deviceName, sizeof(deviceInfo.deviceName),
        nodeInfo->deviceName, sizeof(deviceInfo.deviceName) - 1);
    deviceInfo.deviceTypeId = nodeInfo->deviceTypeId;

    SoftbusCache* cache = SoftbusCacheGetInstance();
    if (cache != NULL) {
        DmSoftbusCacheSaveDeviceInfo(cache, &deviceInfo);
        DmString networkIdStr = DmStringCreate(nodeInfo->networkId);
        DmDeviceInfo cachedInfo;
        DmDeviceInfoInit(&cachedInfo);
        if (DmSoftbusCacheGetDevInfoByNetworkId(cache, &networkIdStr, &cachedInfo) == DM_OK) {
            if (cachedInfo.deviceId[0] != '\0') {
                (void)strncpy_s(deviceInfo.deviceId, sizeof(deviceInfo.deviceId),
                    cachedInfo.deviceId, sizeof(deviceInfo.deviceId) - 1);
            }
            if (cachedInfo.authForm != DM_AUTH_FORM_INVALID_TYPE) {
                deviceInfo.authForm = cachedInfo.authForm;
            }
        }
        DmStringDestroy(&networkIdStr);
        DmDeviceInfoDestroy(&cachedInfo);
    }
    DmLiteClientNotifyDeviceState(DEVICE_INFO_CHANGED, &deviceInfo, true);
    LOGI("notify DEVICE_INFO_CHANGED deviceName=%s", nodeInfo->deviceName);
}

void DmSoftbusListenerOnSoftbusPublishResult(int publishId, int result)
{
    LOGI("OnSoftbusPublishResult publishId: %d, result: %d.", publishId, result);
}
