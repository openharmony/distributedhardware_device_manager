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


#ifndef DM_IPC_CMD_REGISTER_H
#define DM_IPC_CMD_REGISTER_H

#include <stdint.h>
#include "device_manager_ipc_interface_code.h"
#include "dm_single_instance.h"
#include "dm_thread.h"
#include "dm_container.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t (*DmSetIpcRequestFunc)(void* pBaseReq, void* data);
typedef int32_t (*DmReadResponseFunc)(void* reply, void* pBaseRsp);
typedef int32_t (*DmOnIpcCmdFunc)(int32_t cmdCode, void* data, void* reply);

typedef struct {
    DmMutex setIpcRequestFuncMapLock;
    DmHmap_int_DmVoidPtr setIpcRequestFuncMap;
    DmMutex readResponseFuncMapLock;
    DmHmap_int_DmVoidPtr readResponseFuncMap;
    DmMutex onIpcCmdFuncMapLock;
    DmHmap_int_DmVoidPtr onIpcCmdFuncMap;
} DmIpcCmdRegister;

DM_DECLARE_SINGLE_INSTANCE(DmIpcCmdRegister);

void DmIpcCmdRegisterInit(DmIpcCmdRegister* inst);
void DmIpcCmdRegisterRegisterSetRequestFunc(int32_t cmdCode, DmSetIpcRequestFunc func);
void DmIpcCmdRegisterRegisterReadResponseFunc(int32_t cmdCode, DmReadResponseFunc func);
void DmIpcCmdRegisterRegisterCmdProcessFunc(int32_t cmdCode, DmOnIpcCmdFunc func);

int32_t DmIpcCmdRegisterSetRequest(int32_t cmdCode, void* pBaseReq, void* data);
int32_t DmIpcCmdRegisterReadResponse(int32_t cmdCode, void* reply, void* pBaseRsp);
int32_t DmIpcCmdRegisterOnIpcCmd(int32_t cmdCode, void* data, void* reply);
int32_t DmIpcCmdRegisterOnRemoteRequest(int32_t cmdCode, void* data, void* reply);

#define DM_ON_IPC_SET_REQUEST(cmdCode, paraA, paraB) \
    static int32_t DmIpcSetRequest_##cmdCode(paraA, paraB); \
    __attribute__((constructor)) static void DmIpcRegisterSetRequest_##cmdCode(void) { \
        DmIpcCmdRegisterRegisterSetRequestFunc(cmdCode, (DmSetIpcRequestFunc)DmIpcSetRequest_##cmdCode); \
    } \
    static int32_t DmIpcSetRequest_##cmdCode(paraA, paraB)

#define DM_ON_IPC_READ_RESPONSE(cmdCode, paraA, paraB) \
    static int32_t DmIpcReadResponse_##cmdCode(paraA, paraB); \
    __attribute__((constructor)) static void DmIpcRegisterReadResponse_##cmdCode(void) { \
        DmIpcCmdRegisterRegisterReadResponseFunc(cmdCode, (DmReadResponseFunc)DmIpcReadResponse_##cmdCode); \
    } \
    static int32_t DmIpcReadResponse_##cmdCode(paraA, paraB)

#define DM_ON_IPC_CMD(cmdCode, paraA, paraB) \
    static int32_t DmIpcCmdProcess_##cmdCode(int32_t ipcCmdCode, paraA, paraB); \
    __attribute__((constructor)) static void DmIpcRegisterCmdProcess_##cmdCode(void) { \
        DmIpcCmdRegisterRegisterCmdProcessFunc(cmdCode, (DmOnIpcCmdFunc)DmIpcCmdProcess_##cmdCode); \
    } \
    static int32_t DmIpcCmdProcess_##cmdCode(int32_t ipcCmdCode, paraA, paraB)

#ifdef __cplusplus
}
#endif

#endif
