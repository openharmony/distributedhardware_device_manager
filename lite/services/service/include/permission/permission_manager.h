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


#ifndef DM_PERMISSION_MANAGER_H
#define DM_PERMISSION_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_container.h"
#include "dm_single_instance.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PermissionManager {
    int initialized;
} PermissionManager;

bool DmPermissionCheckSystemSa(const DmString* pkgName);
bool DmPermissionCheckAccessService(const DmString* pkgName);
bool DmPermissionCheckDataSync(const DmString* pkgName);
bool DmPermissionCheckMonitor(const DmString* pkgName);

#ifdef __cplusplus
}
#endif

#endif
