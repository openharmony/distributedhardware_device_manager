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


#include "ipc_check_access_control.h"

void DmIpcCheckAclInit(IpcCheckAcl* obj)
{
    DmIpcReqInit(&obj->base);
    DmAccessCallerInit(&obj->caller);
    DmAccessCalleeInit(&obj->callee);
}

void DmIpcCheckAclDestroy(IpcCheckAcl* obj)
{
    DmIpcReqDestroy(&obj->base);
    DmAccessCallerDestroy(&obj->caller);
    DmAccessCalleeDestroy(&obj->callee);
}

int DmIpcCheckAclSerialize(IpcCheckAcl* obj, void* parcel)
{
    return DmIpcReqSerialize(&obj->base, parcel);
}

int DmIpcCheckAclDeserialize(void* parcel, IpcCheckAcl* obj)
{
    return DmIpcReqDeserialize(parcel, &obj->base);
}
