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


#include "ipc_req.h"

void DmIpcReqInit(IpcReq* obj)
{
    obj->pkgName = DmStringCreateEmpty();
    DmProcessInfoInit(&obj->processInfo);
}

void DmIpcReqDestroy(IpcReq* obj)
{
    DmStringDestroy(&obj->pkgName);
    DmProcessInfoDestroy(&obj->processInfo);
}

int DmIpcReqSerialize(IpcReq* obj, void* parcel)
{
    return 0;
}

int DmIpcReqDeserialize(void* parcel, IpcReq* obj)
{
    return 0;
}
