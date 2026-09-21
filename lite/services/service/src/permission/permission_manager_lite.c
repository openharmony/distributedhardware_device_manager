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


#include "permission_manager.h"
#include "dm_log.h"
#include "dm_constants.h"
#include <string.h>

static const char* g_systemSaWhitist[] = {
    "ohos.distributedhardware.devicemanager",
    "ohos.dmsdp",
    "com.ohos.dmsdp",
    "Samgr_Networking",
    "ohos.distributeddata.service",
    "ohos.dslm",
    "ohos.deviceprofile",
    "distributed_bundle_framework",
    "ohos.dhardware",
    "ohos.security.distributed_access_token",
    "ohos.storage.distributedfile.daemon",
    "audio_manager_service",
    "hmos.collaborationfwk.deviceDetect",
    "distributedsched",
    NULL
};

bool DmPermissionCheckSystemSa(const DmString* pkgName)
{
    if (pkgName == NULL) {
        return false;
    }
    const char* name = DmStringCstr(pkgName);
    if (name == NULL) {
        return false;
    }
    for (int i = 0; g_systemSaWhitist[i] != NULL; i++) {
        if (strcmp(name, g_systemSaWhitist[i]) == 0) {
            return true;
        }
    }
    LOGE("pkgName not in whitelist: %s", name);
    return false;
}

bool DmPermissionCheckAccessService(const DmString* pkgName)
{
    return DmPermissionCheckSystemSa(pkgName);
}

bool DmPermissionCheckDataSync(const DmString* pkgName)
{
    return DmPermissionCheckSystemSa(pkgName);
}

bool DmPermissionCheckMonitor(const DmString* pkgName)
{
    return DmPermissionCheckSystemSa(pkgName);
}
