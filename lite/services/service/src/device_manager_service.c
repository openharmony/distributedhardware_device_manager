/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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


#include "device_manager_service.h"
#include <stdlib.h>
#include <string.h>
#include "securec.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "dm_constants.h"

#include "softbus_listener_c.h"
#include "deviceprofile_connector.h"
#include "softbus_bus_center.h"
#include "discovery_manager_c.h"
#include "device_manager_service_listener.h"
#include "dm_local_acl_profile.h"
#include "permission_manager.h"
#include "device_auth.h"

#define DM_PERMISSION_LEVEL_MONITOR 2

DM_IMPLEMENT_SINGLE_INSTANCE(DeviceManagerService)

static DeviceManagerServiceListener g_serviceListener;

int32_t DmServiceInit(void)
{
    LOGI("DeviceManagerService init lite");
    DiscoveryManager* discMgr = DiscoveryManagerGetInstance();
    DmDiscoveryManagerInit(discMgr, NULL, &g_serviceListener);
    DmServiceListenerInit(&g_serviceListener);
    DmAclStoreInit();
    LOGI("DiscoveryManager, ServiceListener, ACL store initialized");
    return DM_OK;
}

int32_t DmServiceGetTrustedDeviceList(const DmString* pkgName, const DmString* extra, DmVec_DmDeviceInfo* deviceList)
{
    (void)extra;
    CHECK_NULL_RETURN(pkgName, ERR_DM_FAILED);
    DmVec_DmDeviceInfo onlineList;
    DmVec_DmDeviceInfo_Init(&onlineList);
    int32_t ret = DmSoftbusListenerGetTrustedDeviceList(&onlineList);
    if (ret != DM_OK || DmVec_DmDeviceInfo_Size(&onlineList) == 0) {
        DmVec_DmDeviceInfo_Destroy(&onlineList);
        return ret;
    }
    bool isOnlyShowNetworkId = !(DmPermissionCheckAccessService(pkgName) ||
        DmPermissionCheckDataSync(pkgName));
    for (int32_t i = 0; i < DmVec_DmDeviceInfo_Size(&onlineList); i++) {
        DmDeviceInfo* dev = DmVec_DmDeviceInfo_At(&onlineList, i);
        if (isOnlyShowNetworkId) {
            DmDeviceInfo tempInfo;
            (void)memset_s(&tempInfo, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
            (void)strncpy_s(tempInfo.networkId, sizeof(tempInfo.networkId),
                dev->networkId, sizeof(tempInfo.networkId) - 1);
            DmVec_DmDeviceInfo_Push(deviceList, tempInfo);
        } else {
            DmVec_DmDeviceInfo_Push(deviceList, *dev);
        }
    }
    DmVec_DmDeviceInfo_Destroy(&onlineList);
    LOGI("GetTrustedDeviceList trusted=%d", DmVec_DmDeviceInfo_Size(deviceList));
    return DM_OK;
}

int32_t DmServiceShiftLnnGear(const DmString* pkgName, const DmString* callerId, bool isRefresh, bool isWakeUp)
{
    LOGI("ShiftLNNGear.");
    return DM_OK;
}

int32_t DmServiceGetLocalDeviceInfo(DmDeviceInfo* info)
{
    CHECK_NULL_RETURN(info, ERR_DM_FAILED);
    return DmSoftbusListenerGetLocalDeviceInfo(info);
}

int32_t DmServiceGetDeviceInfo(const DmString* networkId, DmDeviceInfo* info)
{
    CHECK_NULL_RETURN(networkId, ERR_DM_FAILED);
    return DmSoftbusListenerGetDeviceInfo(networkId, info);
}

int32_t DmServiceGetUdidByNetworkId(const DmString* pkgName, const DmString* netWorkId, DmString* udid)
{
    CHECK_NULL_RETURN(pkgName, ERR_DM_FAILED);
    return DmSoftbusListenerGetUdidByNetworkId(DmStringCstr(netWorkId), udid);
}

int32_t DmServiceGetUuidByNetworkId(const DmString* pkgName, const DmString* netWorkId, DmString* uuid)
{
    CHECK_NULL_RETURN(pkgName, ERR_DM_FAILED);
    return DmSoftbusListenerGetUuidByNetworkId(DmStringCstr(netWorkId), uuid);
}

int32_t DmServicePublishDeviceDiscovery(const DmString* pkgName, const DmPublishInfo* publishInfo)
{
    CHECK_NULL_RETURN(pkgName, ERR_DM_FAILED);
    CHECK_NULL_RETURN(publishInfo, ERR_DM_FAILED);
    const char* reqCap = publishInfo->capability;
    bool capEmpty = (reqCap == NULL || reqCap[0] == '\0');
    DmString capability = DmStringCreate(capEmpty ? DM_CAPABILITY_OSD : reqCap);
    if (capEmpty) {
        LOGW("PublishDeviceDiscovery empty capability from caller, fallback to %s", DM_CAPABILITY_OSD);
    }
    LOGW("PublishDeviceDiscovery publishId=%d mode=%d medium=%d capability=%s",
         publishInfo->publishId, publishInfo->mode, publishInfo->medium, DmStringCstr(&capability));
    int32_t ret = DmSoftbusListenerPublishSoftbusLnn(publishInfo, &capability, NULL);
    DmStringDestroy(&capability);
    return ret;
}

int32_t DmServiceUnpublishDeviceDiscovery(const DmString* pkgName, int32_t publishId)
{
    CHECK_NULL_RETURN(pkgName, ERR_DM_FAILED);
    return DmSoftbusListenerStopPublishSoftbusLnn(publishId);
}

int32_t DmServiceCheckApiPermission(int32_t permissionLevel, const DmString* pkgName)
{
    LOGI("CheckApiPermission lite permissionLevel=%d", permissionLevel);
    if (pkgName == NULL || DmStringEmpty(pkgName)) {
        LOGE("CheckApiPermission invalid pkgName");
        return ERR_DM_NO_PERMISSION;
    }
    bool allowed = false;
    if (permissionLevel == 0) {
        allowed = DmPermissionCheckDataSync(pkgName);
    } else if (permissionLevel == 1) {
        allowed = DmPermissionCheckAccessService(pkgName);
    } else if (permissionLevel == DM_PERMISSION_LEVEL_MONITOR) {
        allowed = DmPermissionCheckMonitor(pkgName);
    }
    return allowed ? DM_OK : ERR_DM_NO_PERMISSION;
}

int32_t DmServiceNotifyEvent(const DmString* pkgName, int32_t eventId, const DmString* event)
{
    LOGI("NotifyEvent pkgName=%s eventId=%d", DmStringCstr(pkgName), eventId);
    return DM_OK;
}

int32_t DmServiceRequestCredential(const DmString* reqJsonStr, DmString* returnJsonStr)
{
    LOGI("RequestCredential lite");
    const DeviceGroupManager *gm = GetGmInstance();
    if (gm == NULL) {
        LOGE("GetGmInstance failed");
        return ERR_DM_FAILED;
    }
    char *returnData = NULL;
    int32_t ret = gm->getRegisterInfo(DmStringCstr(reqJsonStr), &returnData);
    if (ret != 0 || returnData == NULL) {
        LOGE("getRegisterInfo failed ret=%d", ret);
        if (returnData) gm->destroyInfo(&returnData);
        return ERR_DM_FAILED;
    }
    DmStringAssignCstr(returnJsonStr, returnData);
    gm->destroyInfo(&returnData);
    LOGI("RequestCredential success");
    return DM_OK;
}

int32_t DmServiceImportCredential(const DmString* pkgName, const DmString* credentialInfo)
{
    LOGI("ImportCredential lite pkgName=%s", DmStringCstr(pkgName));
    const CredManager *cm = GetCredMgrInstance();
    if (cm == NULL) {
        LOGE("GetCredMgrInstance failed");
        return ERR_DM_FAILED;
    }
    char *returnData = NULL;
    int32_t ret = cm->addCredential(0, DmStringCstr(credentialInfo), &returnData);
    if (ret != 0) {
        LOGE("addCredential failed ret=%d", ret);
        if (returnData) cm->destroyInfo(&returnData);
        return ERR_DM_FAILED;
    }
    if (returnData) cm->destroyInfo(&returnData);
    LOGI("ImportCredential success");
    return DM_OK;
}

int32_t DmServiceDeleteCredential(const DmString* pkgName, const DmString* deleteInfo)
{
    LOGI("DeleteCredential lite pkgName=%s", DmStringCstr(pkgName));
    const CredManager *cm = GetCredMgrInstance();
    if (cm == NULL) {
        LOGE("GetCredMgrInstance failed");
        return ERR_DM_FAILED;
    }
    int32_t ret = cm->deleteCredential(0, DmStringCstr(deleteInfo));
    if (ret != 0) {
        LOGE("deleteCredential failed ret=%d", ret);
        return ERR_DM_FAILED;
    }
    LOGI("DeleteCredential success");
    return DM_OK;
}

int32_t DmServiceRegisterCredentialCallback(const DmString* pkgName)
{
    LOGI("RegisterCredentialCallback.");
    return DM_OK;
}

int32_t DmServiceUnregisterCredentialCallback(const DmString* pkgName)
{
    LOGI("UnRegisterCredentialCallback.");
    return DM_OK;
}

int32_t DmServiceGetNetworkTypeByNetworkId(const DmString* pkgName, const DmString* netWorkId, int32_t* networkType)
{
    return DmSoftbusListenerGetNetworkTypeByNetworkId(DmStringCstr(netWorkId), networkType);
}

int32_t DmServiceStartAdvertising(const DmString* pkgName, const DmMap_DmString_DmString* advertiseParam)
{
    LOGI("StartAdvertising pkgName=%s", DmStringCstr(pkgName));
    return DM_OK;
}

int32_t DmServiceStopAdvertising(const DmString* pkgName, const DmMap_DmString_DmString* advertiseParam)
{
    LOGI("StopAdvertising pkgName=%s", DmStringCstr(pkgName));
    return DM_OK;
}

int32_t DmServiceLeaveLnn(const DmString* pkgName, const DmString* networkId)
{
    LOGI("LeaveLNN lite networkId=%s", DmStringCstr(networkId));
    int32_t ret = LeaveLNN(DmStringCstr(pkgName), DmStringCstr(networkId), NULL);
    if (ret != 0) {
        LOGE("LeaveLNN failed ret=%d", ret);
        return ERR_DM_FAILED;
    }
    LOGI("LeaveLNN success");
    return DM_OK;
}

int32_t DmServiceGetDeviceSecurityLevel(const DmString* pkgName, const DmString* networkId, int32_t* securityLevel)
{
    return DmSoftbusListenerGetDeviceSecurityLevel(DmStringCstr(networkId), securityLevel);
}

bool DmServiceCheckAccessControl(const DmAccessCaller* caller, const DmAccessCallee* callee)
{
    char localUdid[UDID_BUF_LEN] = {0};
    NodeBasicInfo localInfo;
    if (GetLocalNodeDeviceInfo("ohos.distributedhardware.devicemanager", &localInfo) == 0) {
        GetNodeKeyInfo("ohos.distributedhardware.devicemanager", localInfo.networkId,
            NODE_KEY_UDID, (uint8_t*)localUdid, sizeof(localUdid));
    }
    return DmDpConnectorCheckAccessControl(caller, localUdid, callee, "");
}

int32_t DmServiceSetLocalDisplayNameToSoftbus(const DmString* displayName)
{
    CHECK_NULL_RETURN(displayName, ERR_DM_FAILED);
    return DmSoftbusListenerSetLocalDisplayName(displayName);
}
