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


#include "dm_device_info_c.h"
#include <stdlib.h>
#include <string.h>
#include "securec.h"

DM_VEC_IMPL(DmDeviceInfo)
DM_VEC_IMPL(DmDeviceBasicInfo)
DM_VEC_IMPL(DMLocalServiceInfo)
DM_VEC_IMPL(DmServiceInfo)
DM_VEC_IMPL(DmRegisterServiceInfo)

int DmProcessInfoCmp(ProcessInfo a, ProcessInfo b)
{
    return a.userId != b.userId;
}

DM_SET_IMPL(ProcessInfo, DmProcessInfoCmp)
DM_MAP_IMPL(int_DmSet_ProcessInfo, int, DmSet_ProcessInfo, DmCmpInt)

void DmSet_ProcessInfo_Clear(DmSet_ProcessInfo* s)
{
    for (int i = 0; i < s->size; i++) {
        DmProcessInfoDestroy(&s->data[i]);
    }
    s->size = 0;
}
void DmSet_ProcessInfo_Destroy(DmSet_ProcessInfo* s)
{
    DmSet_ProcessInfo_Clear(s);
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

void DmMap_int_DmSet_ProcessInfo_Clear(DmMap_int_DmSet_ProcessInfo* m)
{
    for (int i = 0; i < m->size; i++) {
        DmSet_ProcessInfo_Destroy(&m->data[i].val);
    }
    m->size = 0;
}
void DmMap_int_DmSet_ProcessInfo_Destroy(DmMap_int_DmSet_ProcessInfo* m)
{
    DmMap_int_DmSet_ProcessInfo_Clear(m);
    free(m->data);
    m->data = NULL;
    m->size = 0;
    m->cap = 0;
}

void DmPublishInfoInit(DmPublishInfo* info)
{
    info->publishId = 0;
    info->mode = DM_DISCOVER_MODE_PASSIVE;
    info->freq = DM_LOW;
    info->ranging = false;
    info->medium = DM_AUTO;
    (void)memset_s(info->capability, sizeof(info->capability), 0, sizeof(info->capability));
}

void DmPublishInfoDestroy(DmPublishInfo* info)
{
}

void DmDeviceInfoInit(DmDeviceInfo* info)
{
    (void)memset_s(info->deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
    (void)memset_s(info->deviceName, DM_MAX_DEVICE_NAME_LEN, 0, DM_MAX_DEVICE_NAME_LEN);
    info->deviceTypeId = DEVICE_TYPE_UNKNOWN;
    (void)memset_s(info->networkId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
    info->range = 0;
    info->networkType = 0;
    info->authForm = DM_AUTH_FORM_INVALID_TYPE;
    info->extraData = DmStringCreateEmpty();
}

void DmDeviceInfoDestroy(DmDeviceInfo* info)
{
    DmStringDestroy(&info->extraData);
}

void DmDeviceBasicInfoInit(DmDeviceBasicInfo* info)
{
    (void)memset_s(info->deviceId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
    (void)memset_s(info->deviceName, DM_MAX_DEVICE_NAME_LEN, 0, DM_MAX_DEVICE_NAME_LEN);
    info->deviceTypeId = DEVICE_TYPE_UNKNOWN;
    (void)memset_s(info->networkId, DM_MAX_DEVICE_ID_LEN, 0, DM_MAX_DEVICE_ID_LEN);
    info->extraData = DmStringCreateEmpty();
}

void DmDeviceBasicInfoDestroy(DmDeviceBasicInfo* info)
{
    DmStringDestroy(&info->extraData);
}

DmDeviceInfo DmDeviceBasicInfoToDeviceInfo(const DmDeviceBasicInfo* basic)
{
    DmDeviceInfo info;
    DmDeviceInfoInit(&info);
    if (basic == NULL) {
        return info;
    }
    (void)memcpy_s(info.deviceId, DM_MAX_DEVICE_ID_LEN, basic->deviceId, DM_MAX_DEVICE_ID_LEN);
    (void)memcpy_s(info.deviceName, DM_MAX_DEVICE_NAME_LEN, basic->deviceName, DM_MAX_DEVICE_NAME_LEN);
    info.deviceTypeId = basic->deviceTypeId;
    (void)memcpy_s(info.networkId, DM_MAX_DEVICE_ID_LEN, basic->networkId, DM_MAX_DEVICE_ID_LEN);
    DmStringDestroy(&info.extraData);
    info.extraData = DmStringCopy(&basic->extraData);
    return info;
}

void DmPeerTargetIdInit(PeerTargetId* id)
{
    id->networkId = DmStringCreateEmpty();
    id->deviceId = DmStringCreateEmpty();
    id->brMac = DmStringCreateEmpty();
    id->bleMac = DmStringCreateEmpty();
    id->wifiIp = DmStringCreateEmpty();
    id->wifiPort = 0;
    id->serviceId = 0;
}

void DmPeerTargetIdDestroy(PeerTargetId* id)
{
    DmStringDestroy(&id->networkId);
    DmStringDestroy(&id->deviceId);
    DmStringDestroy(&id->brMac);
    DmStringDestroy(&id->bleMac);
    DmStringDestroy(&id->wifiIp);
}

void DmAccessCallerInit(DmAccessCaller* caller)
{
    caller->accountId = DmStringCreateEmpty();
    caller->pkgName = DmStringCreateEmpty();
    caller->networkId = DmStringCreateEmpty();
    caller->userId = 0;
    caller->tokenId = 0;
    caller->extra = DmStringCreateEmpty();
}

void DmAccessCallerDestroy(DmAccessCaller* caller)
{
    DmStringDestroy(&caller->accountId);
    DmStringDestroy(&caller->pkgName);
    DmStringDestroy(&caller->networkId);
    DmStringDestroy(&caller->extra);
}

void DmAccessCalleeInit(DmAccessCallee* callee)
{
    callee->accountId = DmStringCreateEmpty();
    callee->networkId = DmStringCreateEmpty();
    callee->peerId = DmStringCreateEmpty();
    callee->pkgName = DmStringCreateEmpty();
    callee->userId = 0;
    callee->tokenId = 0;
    callee->extra = DmStringCreateEmpty();
}

void DmAccessCalleeDestroy(DmAccessCallee* callee)
{
    DmStringDestroy(&callee->accountId);
    DmStringDestroy(&callee->networkId);
    DmStringDestroy(&callee->peerId);
    DmStringDestroy(&callee->pkgName);
    DmStringDestroy(&callee->extra);
}

void DmProcessInfoInit(ProcessInfo* info)
{
    info->userId = 0;
    info->pkgName = DmStringCreateEmpty();
    info->tokenId = 0;
}

void DmProcessInfoDestroy(ProcessInfo* info)
{
    DmStringDestroy(&info->pkgName);
}

void DmNotifyKeyInit(DmNotifyKey* key)
{
    key->processUserId = 0;
    key->processPkgName = DmStringCreateEmpty();
    key->notifyUserId = 0;
    key->udid = DmStringCreateEmpty();
}

void DmNotifyKeyDestroy(DmNotifyKey* key)
{
    DmStringDestroy(&key->processPkgName);
    DmStringDestroy(&key->udid);
}

void DmLocalServiceInfoInit(DMLocalServiceInfo* info)
{
    info->bundleName = DmStringCreateEmpty();
    info->authBoxType = 0;
    info->authType = 0;
    info->pinExchangeType = 0;
    info->pinCode = DmStringCreateEmpty();
    info->description = DmStringCreateEmpty();
    info->extraInfo = DmStringCreateEmpty();
}

void DmLocalServiceInfoDestroy(DMLocalServiceInfo* info)
{
    DmStringDestroy(&info->bundleName);
    DmStringDestroy(&info->pinCode);
    DmStringDestroy(&info->description);
    DmStringDestroy(&info->extraInfo);
}

void DmDevUserInfoInit(DevUserInfo* info)
{
    info->deviceId = DmStringCreateEmpty();
    info->userId = 0;
}

void DmDevUserInfoDestroy(DevUserInfo* info)
{
    DmStringDestroy(&info->deviceId);
}

void DmAclQuadInfoInit(DMAclQuadInfo* info)
{
    info->localUdid = DmStringCreateEmpty();
    info->localUserId = 0;
    info->peerUdid = DmStringCreateEmpty();
    info->peerUserId = 0;
}

void DmAclQuadInfoDestroy(DMAclQuadInfo* info)
{
    DmStringDestroy(&info->localUdid);
    DmStringDestroy(&info->peerUdid);
}

void DmServiceInfoStructInit(ServiceInfo* info)
{
    info->serviceId = 0;
    info->serviceType = DmStringCreateEmpty();
    info->serviceName = DmStringCreateEmpty();
    info->serviceDisplayName = DmStringCreateEmpty();
}

void DmServiceInfoStructDestroy(ServiceInfo* info)
{
    DmStringDestroy(&info->serviceType);
    DmStringDestroy(&info->serviceName);
    DmStringDestroy(&info->serviceDisplayName);
}

void DmDiscoveryServiceInfoInit(DiscoveryServiceInfo* info)
{
    DmServiceInfoStructInit(&info->serviceInfo);
    info->pkgName = DmStringCreateEmpty();
}

void DmDiscoveryServiceInfoDestroy(DiscoveryServiceInfo* info)
{
    DmServiceInfoStructDestroy(&info->serviceInfo);
    DmStringDestroy(&info->pkgName);
}

void DmServiceInfoProfileInit(ServiceInfoProfile* info)
{
    info->regServiceId = 0;
    info->deviceId = DmStringCreateEmpty();
    info->userId = 0;
    info->tokenId = 0;
    info->publishState = 0;
    info->serviceId = 0;
    info->serviceType = DmStringCreateEmpty();
    info->serviceName = DmStringCreateEmpty();
    info->serviceDisplayName = DmStringCreateEmpty();
}

void DmServiceInfoProfileDestroy(ServiceInfoProfile* info)
{
    DmStringDestroy(&info->deviceId);
    DmStringDestroy(&info->serviceType);
    DmStringDestroy(&info->serviceName);
    DmStringDestroy(&info->serviceDisplayName);
}

void DmServiceRegInfoInit(ServiceRegInfo* info)
{
    DmServiceInfoStructInit(&info->serviceInfo);
    info->customData = DmStringCreateEmpty();
    info->dataLen = 0;
}

void DmServiceRegInfoDestroy(ServiceRegInfo* info)
{
    DmServiceInfoStructDestroy(&info->serviceInfo);
    DmStringDestroy(&info->customData);
}

void DmPublishServiceParamInit(PublishServiceParam* param)
{
    DmServiceInfoStructInit(&param->serviceInfo);
    param->discoverMode = SERVICE_PUBLISH_MODE_PASSIVE;
    param->regServiceId = 0;
    param->media = SERVICE_MEDIUM_TYPE_AUTO;
    param->freq = DM_LOW;
}

void DmPublishServiceParamDestroy(PublishServiceParam* param)
{
    DmServiceInfoStructDestroy(&param->serviceInfo);
}

void DmServiceInfoInit(DmServiceInfo* info)
{
    info->userId = -1;
    info->serviceId = -1;
    info->displayId = -1;
    info->publishState = -1;
    info->deviceId = DmStringCreateEmpty();
    info->networkId = DmStringCreateEmpty();
    info->authform = DM_AUTH_FORM_INVALID_TYPE;
    info->serviceOwnerTokenId = 0;
    info->serviceOwnerPkgName = DmStringCreateEmpty();
    info->serviceRegisterTokenId = 0;
    info->serviceType = DmStringCreateEmpty();
    info->serviceName = DmStringCreateEmpty();
    info->serviceDisplayName = DmStringCreateEmpty();
    info->serviceCode = DmStringCreateEmpty();
    info->customData = DmStringCreateEmpty();
    info->dataLen = 0;
    info->timeStamp = -1;
    info->description = DmStringCreateEmpty();
}

void DmServiceInfoDestroy(DmServiceInfo* info)
{
    DmStringDestroy(&info->deviceId);
    DmStringDestroy(&info->networkId);
    DmStringDestroy(&info->serviceOwnerPkgName);
    DmStringDestroy(&info->serviceType);
    DmStringDestroy(&info->serviceName);
    DmStringDestroy(&info->serviceDisplayName);
    DmStringDestroy(&info->serviceCode);
    DmStringDestroy(&info->customData);
    DmStringDestroy(&info->description);
}

void DmServiceSyncInfoInit(ServiceSyncInfo* info)
{
    info->pkgName = DmStringCreateEmpty();
    info->localUserId = 0;
    info->networkId = DmStringCreateEmpty();
    info->serviceId = 0;
    info->callerUserId = 0;
    info->callerTokenId = 0;
}

void DmServiceSyncInfoDestroy(ServiceSyncInfo* info)
{
    DmStringDestroy(&info->pkgName);
    DmStringDestroy(&info->networkId);
}

void DmRegisterServiceInfoInit(DmRegisterServiceInfo* info)
{
    info->userId = -1;
    info->displayId = -1;
    info->serviceOwnerTokenId = 0;
    info->serviceOwnerPkgName = DmStringCreateEmpty();
    info->serviceRegisterTokenId = 0;
    info->serviceType = DmStringCreateEmpty();
    info->serviceName = DmStringCreateEmpty();
    info->serviceDisplayName = DmStringCreateEmpty();
    info->customData = DmStringCreateEmpty();
    info->serviceCode = DmStringCreateEmpty();
    info->dataLen = 0;
    info->timeStamp = -1;
    info->description = DmStringCreateEmpty();
}

void DmRegisterServiceInfoDestroy(DmRegisterServiceInfo* info)
{
    DmStringDestroy(&info->serviceOwnerPkgName);
    DmStringDestroy(&info->serviceType);
    DmStringDestroy(&info->serviceName);
    DmStringDestroy(&info->serviceDisplayName);
    DmStringDestroy(&info->customData);
    DmStringDestroy(&info->serviceCode);
    DmStringDestroy(&info->description);
}

void DmPublishServiceParamCInit(DmPublishServiceParam* param)
{
    param->discoverMode = SERVICE_PUBLISH_MODE_PASSIVE;
    param->media = SERVICE_MEDIUM_TYPE_AUTO;
    param->freq = DM_LOW;
}

void DmPublishServiceParamCDestroy(DmPublishServiceParam* param)
{
}

void DmDiscoveryServiceParamInit(DmDiscoveryServiceParam* param)
{
    param->serviceType = DmStringCreateEmpty();
    param->serviceName = DmStringCreateEmpty();
    param->serviceDisplayName = DmStringCreateEmpty();
    param->freq = DM_LOW;
    param->medium = SERVICE_MEDIUM_TYPE_AUTO;
    param->mode = SERVICE_PUBLISH_MODE_PASSIVE;
}

void DmDiscoveryServiceParamDestroy(DmDiscoveryServiceParam* param)
{
    DmStringDestroy(&param->serviceType);
    DmStringDestroy(&param->serviceName);
    DmStringDestroy(&param->serviceDisplayName);
}

void DmUserRemovedServiceInfoInit(DmUserRemovedServiceInfo* info)
{
    info->localTokenId = 0;
    info->localPkgName = DmStringCreateEmpty();
    info->bindType = 0;
    info->peerUdid = DmStringCreateEmpty();
    DmVecInt64_t_Init(&info->serviceIds);
    info->isActive = false;
    info->peerUserId = 0;
}

void DmUserRemovedServiceInfoDestroy(DmUserRemovedServiceInfo* info)
{
    DmStringDestroy(&info->localPkgName);
    DmStringDestroy(&info->peerUdid);
    DmVecInt64_t_Destroy(&info->serviceIds);
}

void DmRegisterServiceStateInit(DmRegisterServiceState* state)
{
    state->userId = 0;
    state->tokenId = 0;
    state->pkgName = DmStringCreateEmpty();
    state->serviceId = 0;
}

void DmRegisterServiceStateDestroy(DmRegisterServiceState* state)
{
    DmStringDestroy(&state->pkgName);
}

void DmSubscribeInfoInit(DmSubscribeInfo* info)
{
    info->subscribeId = 0;
    info->mode = DM_DISCOVER_MODE_PASSIVE;
    info->medium = DM_AUTO;
    info->freq = DM_LOW;
    info->isSameAccount = false;
    info->isWakeRemote = false;
    (void)memset_s(info->capability, DM_MAX_DEVICE_CAPABILITY_LEN, 0, DM_MAX_DEVICE_CAPABILITY_LEN);
}

void DmSubscribeInfoDestroy(DmSubscribeInfo* info)
{
}
