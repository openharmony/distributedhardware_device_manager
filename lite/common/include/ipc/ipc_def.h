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


#ifndef DM_IPC_DEF_H
#define DM_IPC_DEF_H

#include "dm_container.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

#define DM_SERVICE_NAME "dev_mgr_svc"
#define DM_MAX_IPC_LEN 8192

int GetClientIdentity(unsigned int *handle, uintptr_t *token, uintptr_t *cookie);

#endif // DM_IPC_DEF_H
