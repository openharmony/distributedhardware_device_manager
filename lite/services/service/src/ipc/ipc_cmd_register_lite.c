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


#include "device_manager_ipc_interface_code.h"
#include "device_manager_service.h"
#include "dm_log.h"
#include "dm_error_type.h"
#include "securec.h"
#include "ipc_cmd_register.h"
#include "serializer.h"
#include "dm_constants.h"
#include "dm_device_info_c.h"
#include "dm_container.h"
#include "ipc_server_stub.h"
#include "dm_lite_client_notify.h"
#include "softbus_listener_c.h"

static int32_t DmLiteIpcOnGetTrustDeviceList(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmVec_DmDeviceInfo deviceList;
    DmVec_DmDeviceInfo_Init(&deviceList);
    DmString extra = DmStringCreateEmpty();
    int32_t ret = DmServiceGetTrustedDeviceList(&pkgNameStr, &extra, &deviceList);
    WriteInt32(rsp, ret);
    if (ret == DM_OK) {
        WriteInt32(rsp, DmVec_DmDeviceInfo_Size(&deviceList));
        for (int32_t i = 0; i < DmVec_DmDeviceInfo_Size(&deviceList); i++) {
            DmDeviceInfo* dev = DmVec_DmDeviceInfo_At(&deviceList, i);
            WriteString(rsp, dev->deviceId);
            WriteString(rsp, dev->deviceName);
            WriteInt32(rsp, dev->deviceTypeId);
            WriteString(rsp, dev->networkId);
            WriteInt32(rsp, (int32_t)dev->authForm);
        }
    }
    DmVec_DmDeviceInfo_Destroy(&deviceList);
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&extra);
    return ret;
}

static int32_t DmLiteIpcOnGetLocalDeviceInfo(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    (void)pkgName;
    DmDeviceInfo info;
    (void)memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    int32_t ret = DmServiceGetLocalDeviceInfo(&info);
    WriteInt32(rsp, ret);
    if (ret == DM_OK) {
        WriteString(rsp, info.deviceId);
        WriteString(rsp, info.deviceName);
        WriteInt32(rsp, info.deviceTypeId);
        WriteString(rsp, info.networkId);
    }
    return ret;
}

static int32_t DmLiteIpcOnGetUdid(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* networkId = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString networkIdStr = DmStringCreate(networkId ? networkId : "");
    DmString udid = DmStringCreateEmpty();
    int32_t ret = DmServiceGetUdidByNetworkId(&pkgNameStr, &networkIdStr, &udid);
    WriteInt32(rsp, ret);
    if (ret == DM_OK) {
        WriteString(rsp, DmStringCstr(&udid));
    }
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&networkIdStr);
    DmStringDestroy(&udid);
    return ret;
}

static int32_t DmLiteIpcOnGetUuid(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* networkId = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString networkIdStr = DmStringCreate(networkId ? networkId : "");
    DmString uuid = DmStringCreateEmpty();
    int32_t ret = DmServiceGetUuidByNetworkId(&pkgNameStr, &networkIdStr, &uuid);
    WriteInt32(rsp, ret);
    if (ret == DM_OK) {
        WriteString(rsp, DmStringCstr(&uuid));
    }
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&networkIdStr);
    DmStringDestroy(&uuid);
    return ret;
}

static int32_t DmLiteIpcOnGetSecurityLevel(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* networkId = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString networkIdStr = DmStringCreate(networkId ? networkId : "");
    int32_t securityLevel = 0;
    int32_t ret = DmServiceGetDeviceSecurityLevel(&pkgNameStr, &networkIdStr, &securityLevel);
    WriteInt32(rsp, ret);
    if (ret == DM_OK) {
        WriteInt32(rsp, securityLevel);
    }
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&networkIdStr);
    return ret;
}

static int32_t DmLiteIpcOnGetNetworkType(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* networkId = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString networkIdStr = DmStringCreate(networkId ? networkId : "");
    int32_t networkType = 0;
    int32_t ret = DmServiceGetNetworkTypeByNetworkId(&pkgNameStr, &networkIdStr, &networkType);
    WriteInt32(rsp, ret);
    if (ret == DM_OK) {
        WriteInt32(rsp, networkType);
    }
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&networkIdStr);
    return ret;
}

static int32_t DmLiteIpcOnCheckAccessControl(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* srcUdid = ReadString(req, &readLen);
    const char* sinkUdid = ReadString(req, &readLen);
    DmAccessCaller caller;
    DmAccessCallee callee;
    DmAccessCallerInit(&caller);
    DmAccessCalleeInit(&callee);
    const char* callerAccountId = ReadString(req, &readLen);
    const char* callerPkgName = ReadString(req, &readLen);
    const char* callerNetworkId = ReadString(req, &readLen);
    int32_t callerUserId = 0;
    ReadInt32(req, &callerUserId);
    DmStringAssignCstr(&caller.accountId, callerAccountId ? callerAccountId : "");
    DmStringAssignCstr(&caller.pkgName, callerPkgName ? callerPkgName : "");
    DmStringAssignCstr(&caller.networkId, callerNetworkId ? callerNetworkId : "");
    caller.userId = callerUserId;
    const char* calleeAccountId = ReadString(req, &readLen);
    const char* calleeNetworkId = ReadString(req, &readLen);
    const char* calleePeerId = ReadString(req, &readLen);
    const char* calleePkgName = ReadString(req, &readLen);
    int32_t calleeUserId = 0;
    ReadInt32(req, &calleeUserId);
    DmStringAssignCstr(&callee.accountId, calleeAccountId ? calleeAccountId : "");
    DmStringAssignCstr(&callee.networkId, calleeNetworkId ? calleeNetworkId : "");
    DmStringAssignCstr(&callee.peerId, calleePeerId ? calleePeerId : "");
    DmStringAssignCstr(&callee.pkgName, calleePkgName ? calleePkgName : "");
    callee.userId = calleeUserId;
    bool result = DmServiceCheckAccessControl(&caller, &callee);
    WriteInt32(rsp, result ? DM_OK : ERR_DM_FAILED);
    DmAccessCallerDestroy(&caller);
    DmAccessCalleeDestroy(&callee);
    return DM_OK;
}

static int32_t DmLiteIpcOnPublishDeviceDiscovery(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmPublishInfo pubInfo;
    (void)memset_s(&pubInfo, sizeof(DmPublishInfo), 0, sizeof(DmPublishInfo));
    ReadInt32(req, &pubInfo.publishId);
    int32_t mode = 0;
    ReadInt32(req, &mode);
    pubInfo.mode = mode;
    int32_t freq = 0;
    ReadInt32(req, &freq);
    pubInfo.freq = freq;
    int32_t medium = 0;
    ReadInt32(req, &medium);
    pubInfo.medium = medium;
    int32_t ranging = 0;
    ReadInt32(req, &ranging);
    pubInfo.ranging = ranging;
    const char* capStr = ReadString(req, &readLen);
    if (capStr != NULL && capStr[0] != '\0') {
        (void)strncpy_s(pubInfo.capability, sizeof(pubInfo.capability), capStr, sizeof(pubInfo.capability) - 1);
        pubInfo.capability[sizeof(pubInfo.capability) - 1] = '\0';
    }
    int32_t ret = DmServicePublishDeviceDiscovery(&pkgNameStr, &pubInfo);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    return ret;
}

static int32_t DmLiteIpcOnRegisterListener(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    SvcIdentity svc;
    svc.handle = IPC_INVALID_HANDLE;
    svc.token = 0;
    svc.cookie = 0;
    if (pkgName != NULL) {
        DmString pkgNameStr = DmStringCreate(pkgName);
        IpcServerStub* stub = IpcServerStubGetInstance();
        DmIpcServerStubAddSystemSa(stub, &pkgNameStr);
        ReadRemoteObject(req, &svc);
        DmLiteClientNotifyRegister(&pkgNameStr, &svc);
        static bool s_callbacksRegistered = false;
        if (!s_callbacksRegistered) {
            int32_t ret = DmSoftbusListenerRegisterCallbacksInner();
            if (ret == DM_OK) {
                s_callbacksRegistered = true;
                LOGI("SoftBus node state callbacks registered");
            } else {
                LOGW("SoftBus callbacks registration failed %d, will retry", ret);
            }
        }
        LOGI("Listener registered pkgName=%s handle=%d", pkgName, svc.handle);
        DmStringDestroy(&pkgNameStr);
    }
    WriteInt32(rsp, DM_OK);
    return DM_OK;
}

static int32_t DmLiteIpcOnUnregisterListener(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    if (pkgName != NULL) {
        DmString pkgNameStr = DmStringCreate(pkgName);
        DmLiteClientNotifyUnregister(&pkgNameStr);
        DmStringDestroy(&pkgNameStr);
    }
    WriteInt32(rsp, DM_OK);
    return DM_OK;
}

static int32_t DmLiteIpcOnUnpublish(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    int32_t publishId = 0;
    ReadInt32(req, &publishId);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    int32_t ret = DmServiceUnpublishDeviceDiscovery(&pkgNameStr, publishId);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    return ret;
}

static int32_t DmLiteIpcOnStartDiscovering(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmSubscribeInfo subInfo;
    (void)memset_s(&subInfo, sizeof(DmSubscribeInfo), 0, sizeof(DmSubscribeInfo));
    int32_t subscribeIdTmp = 0;
    ReadInt32(req, &subscribeIdTmp);
    subInfo.subscribeId = (uint16_t)subscribeIdTmp;
    int32_t mode = 0;
    ReadInt32(req, &mode);
    subInfo.mode = mode;
    int32_t medium = 0;
    ReadInt32(req, &medium);
    subInfo.medium = medium;
    int32_t freq = 0;
    ReadInt32(req, &freq);
    subInfo.freq = freq;
    const char* capStr = ReadString(req, &readLen);
    if (capStr != NULL && capStr[0] != '\0') {
        (void)strncpy_s(subInfo.capability, sizeof(subInfo.capability), capStr, sizeof(subInfo.capability) - 1);
    } else {
        (void)strncpy_s(subInfo.capability, sizeof(subInfo.capability),
            DM_CAPABILITY_OSD, sizeof(subInfo.capability) - 1);
    }
    subInfo.capability[sizeof(subInfo.capability) - 1] = '\0';
    int32_t ret = DmSoftbusListenerRefreshSoftbusLnn(DmStringCstr(&pkgNameStr), &subInfo, NULL);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    return ret;
}

static int32_t DmLiteIpcOnStopDiscovering(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    int32_t subscribeId = 0;
    ReadInt32(req, &subscribeId);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    int32_t ret = DmSoftbusListenerStopRefreshSoftbusLnn((uint16_t)subscribeId);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    return ret;
}

static int32_t DmLiteIpcOnGetDeviceInfo(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* networkId = ReadString(req, &readLen);
    DmString networkIdStr = DmStringCreate(networkId ? networkId : "");
    DmDeviceInfo info;
    (void)memset_s(&info, sizeof(DmDeviceInfo), 0, sizeof(DmDeviceInfo));
    int32_t ret = DmServiceGetDeviceInfo(&networkIdStr, &info);
    WriteInt32(rsp, ret);
    if (ret == DM_OK) {
        WriteString(rsp, info.deviceId);
        WriteString(rsp, info.networkId);
        WriteString(rsp, info.deviceName);
        WriteInt32(rsp, info.deviceTypeId);
    }
    DmStringDestroy(&networkIdStr);
    return ret;
}

static int32_t DmLiteIpcOnCheckApiPermission(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    int32_t permissionLevel = 0;
    ReadInt32(req, &permissionLevel);
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    int32_t ret = DmServiceCheckApiPermission(permissionLevel, &pkgNameStr);
    DmStringDestroy(&pkgNameStr);
    WriteInt32(rsp, ret);
    return ret;
}

static int32_t DmLiteIpcOnRegisterDevState(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    SvcIdentity svc;
    svc.handle = IPC_INVALID_HANDLE;
    ReadRemoteObject(req, &svc);
    if (pkgName != NULL && svc.handle != IPC_INVALID_HANDLE) {
        DmString pkgNameStr = DmStringCreate(pkgName);
        DmLiteClientNotifyRegister(&pkgNameStr, &svc);
        DmLiteClientNotifyOnlineDevicesToNewClient(svc.handle);
        DmStringDestroy(&pkgNameStr);
    } else {
        LOGW("DevStateCallback invalid pkgName or handle");
    }
    WriteInt32(rsp, DM_OK);
    return DM_OK;
}

static int32_t DmLiteIpcOnUnregisterDevState(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    if (pkgName != NULL) {
        DmString pkgNameStr = DmStringCreate(pkgName);
        DmLiteClientNotifyUnregister(&pkgNameStr);
        DmStringDestroy(&pkgNameStr);
    }
    WriteInt32(rsp, DM_OK);
    return DM_OK;
}

static int32_t DmLiteIpcOnCommonCmd(int32_t cmdCode, void* data, void* reply)
{
    IpcIo* rsp = (IpcIo*)reply;
    WriteInt32(rsp, DM_OK);
    return DM_OK;
}

static int32_t DmLiteIpcOnTestTriggerAclWrite(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* json = ReadString(req, &readLen);
    uint32_t len = (uint32_t)(readLen > 0 ? readLen - 1 : 0);
    int32_t ret = DmServiceTestTriggerAclWrite(json ? json : "", len);
    WriteInt32(rsp, ret);
    return ret;
}

static int32_t DmLiteIpcOnTestTriggerCredDelete(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* credId = ReadString(req, &readLen);
    const char* credInfo = ReadString(req, &readLen);
    int32_t ret = DmServiceTestTriggerCredDelete(credId ? credId : "", credInfo ? credInfo : "");
    WriteInt32(rsp, ret);
    return ret;
}

static int32_t DmLiteIpcOnRequestCredential(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* reqParaStr = ReadString(req, &readLen);
    (void)pkgName;
    DmString reqJsonStr = DmStringCreate(reqParaStr ? reqParaStr : "");
    DmString returnJsonStr = DmStringCreateEmpty();
    int32_t ret = DmServiceRequestCredential(&reqJsonStr, &returnJsonStr);
    WriteInt32(rsp, ret);
    if (ret == DM_OK) {
        WriteString(rsp, DmStringCstr(&returnJsonStr));
    }
    DmStringDestroy(&reqJsonStr);
    DmStringDestroy(&returnJsonStr);
    return ret;
}

static int32_t DmLiteIpcOnImportCredential(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* reqParaStr = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString credInfoStr = DmStringCreate(reqParaStr ? reqParaStr : "");
    int32_t ret = DmServiceImportCredential(&pkgNameStr, &credInfoStr);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&credInfoStr);
    return ret;
}

static int32_t DmLiteIpcOnDeleteCredential(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* reqParaStr = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString deleteInfoStr = DmStringCreate(reqParaStr ? reqParaStr : "");
    int32_t ret = DmServiceDeleteCredential(&pkgNameStr, &deleteInfoStr);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&deleteInfoStr);
    return ret;
}

static int32_t DmLiteIpcOnRegisterCredentialCallback(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    int32_t ret = DmServiceRegisterCredentialCallback(&pkgNameStr);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    return ret;
}

static int32_t DmLiteIpcOnUnregisterCredentialCallback(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    int32_t ret = DmServiceUnregisterCredentialCallback(&pkgNameStr);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    return ret;
}

static int32_t DmLiteIpcOnNotifyEvent(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    int32_t eventId = 0;
    ReadInt32(req, &eventId);
    const char* event = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString eventStr = DmStringCreate(event ? event : "");
    int32_t ret = DmServiceNotifyEvent(&pkgNameStr, eventId, &eventStr);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&eventStr);
    return ret;
}

static int32_t DmLiteIpcOnShiftLnnGear(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString callerId = DmStringCreateEmpty();
    int32_t ret = DmServiceShiftLnnGear(&pkgNameStr, &callerId, false, false);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&callerId);
    return ret;
}

static int32_t DmLiteIpcOnLeaveLnn(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* networkId = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString networkIdStr = DmStringCreate(networkId ? networkId : "");
    int32_t ret = DmServiceLeaveLnn(&pkgNameStr, &networkIdStr);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&networkIdStr);
    return ret;
}


static int32_t DmLiteIpcOnStartAdvertising(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* jsonStr = ReadString(req, &readLen);
    (void)jsonStr;
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmMap_DmString_DmString advertiseParam;
    DmMap_DmString_DmString_Init(&advertiseParam);
    int32_t ret = DmServiceStartAdvertising(&pkgNameStr, &advertiseParam);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    DmMap_DmString_DmString_Destroy(&advertiseParam);
    return ret;
}

static int32_t DmLiteIpcOnStopAdvertising(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* jsonStr = ReadString(req, &readLen);
    (void)jsonStr;
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmMap_DmString_DmString advertiseParam;
    DmMap_DmString_DmString_Init(&advertiseParam);
    int32_t ret = DmServiceStopAdvertising(&pkgNameStr, &advertiseParam);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    DmMap_DmString_DmString_Destroy(&advertiseParam);
    return ret;
}

static int32_t DmLiteIpcOnSetLocalDeviceName(int32_t cmdCode, void* data, void* reply)
{
    (void)cmdCode;
    IpcIo* req = (IpcIo*)data;
    IpcIo* rsp = (IpcIo*)reply;
    size_t readLen = 0;
    const char* pkgName = ReadString(req, &readLen);
    const char* deviceName = ReadString(req, &readLen);
    DmString pkgNameStr = DmStringCreate(pkgName ? pkgName : "");
    DmString nameStr = DmStringCreate(deviceName ? deviceName : "");
    int32_t ret = DmServiceSetLocalDisplayNameToSoftbus(&nameStr);
    WriteInt32(rsp, ret);
    DmStringDestroy(&pkgNameStr);
    DmStringDestroy(&nameStr);
    return ret;
}

void DmLiteIpcCmdRegisterAll(void)
{
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_GET_TRUST_DEVICE_LIST, DmLiteIpcOnGetTrustDeviceList);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_GET_LOCAL_DEVICE_INFO, DmLiteIpcOnGetLocalDeviceInfo);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_GET_UDID_BY_NETWORK, DmLiteIpcOnGetUdid);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_GET_UUID_BY_NETWORK, DmLiteIpcOnGetUuid);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_GET_NETWORKTYPE_BY_NETWORK, DmLiteIpcOnGetNetworkType);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_GET_SECURITY_LEVEL, DmLiteIpcOnGetSecurityLevel);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_ACCESS_CONTROL, DmLiteIpcOnCheckAccessControl);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_PUBLISH_DEVICE_DISCOVER, DmLiteIpcOnPublishDeviceDiscovery);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SAME_ACCOUNT, DmLiteIpcOnCheckAccessControl);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SRC_ACCESS_CONTROL, DmLiteIpcOnCheckAccessControl);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SINK_ACCESS_CONTROL, DmLiteIpcOnCheckAccessControl);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SRC_SAME_ACCOUNT, DmLiteIpcOnCheckAccessControl);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_SINK_SAME_ACCOUNT, DmLiteIpcOnCheckAccessControl);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_UNPUBLISH_DEVICE_DISCOVER, DmLiteIpcOnUnpublish);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_START_DISCOVERING, DmLiteIpcOnStartDiscovering);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_STOP_DISCOVERING, DmLiteIpcOnStopDiscovering);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_REGISTER_DEVICE_MANAGER_LISTENER, DmLiteIpcOnRegisterListener);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_UNREGISTER_DEVICE_MANAGER_LISTENER, DmLiteIpcOnUnregisterListener);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_NOTIFY_EVENT, DmLiteIpcOnNotifyEvent);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_GET_DEVICE_INFO, DmLiteIpcOnGetDeviceInfo);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_SHIFT_LNN_GEAR, DmLiteIpcOnShiftLnnGear);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_CHECK_API_PERMISSION, DmLiteIpcOnCheckApiPermission);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_REGISTER_DEV_STATE_CALLBACK, DmLiteIpcOnRegisterDevState);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_UNREGISTER_DEV_STATE_CALLBACK, DmLiteIpcOnUnregisterDevState);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_REQUEST_CREDENTIAL, DmLiteIpcOnRequestCredential);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_IMPORT_CREDENTIAL, DmLiteIpcOnImportCredential);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_DELETE_CREDENTIAL, DmLiteIpcOnDeleteCredential);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_REGISTER_CREDENTIAL_CALLBACK, DmLiteIpcOnRegisterCredentialCallback);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_UNREGISTER_CREDENTIAL_CALLBACK, DmLiteIpcOnUnregisterCredentialCallback);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_LEAVE_LNN, DmLiteIpcOnLeaveLnn);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_START_ADVERTISING, DmLiteIpcOnStartAdvertising);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_STOP_ADVERTISING, DmLiteIpcOnStopAdvertising);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_SET_LOCAL_DEVICE_NAME, DmLiteIpcOnSetLocalDeviceName);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_TEST_TRIGGER_ACL_WRITE, DmLiteIpcOnTestTriggerAclWrite);
    DmIpcCmdRegisterRegisterCmdProcessFunc(DM_TEST_TRIGGER_CRED_DELETE, DmLiteIpcOnTestTriggerCredDelete);
}
