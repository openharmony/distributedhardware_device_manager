/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <fstream>

#include "accesstoken_kit.h"
#include "access_token.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "file_ex.h"
#include "hap_token_info.h"
#include "ipc_skeleton.h"
#include "native_token_info.h"
#include "securec.h"
#include "tokenid_kit.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(PermissionManager);
namespace {
constexpr const char* DM_SERVICE_ACCESS_PERMISSION = "ohos.permission.ACCESS_SERVICE_DM";
constexpr const char* DM_SERVICE_ACCESS_NEWPERMISSION = "ohos.permission.DISTRIBUTED_DATASYNC";
constexpr const char* DM_MONITOR_DEVICE_NETWORK_STATE_PERMISSION = "ohos.permission.MONITOR_DEVICE_NETWORK_STATE";
constexpr int32_t PKG_NAME_SIZE_MAX = 256;

constexpr int32_t SYSTEM_SA_WHITE_LIST_NUM = 8;
constexpr const static char SYSTEM_SA_WHITE_LIST[SYSTEM_SA_WHITE_LIST_NUM][PKG_NAME_SIZE_MAX] = {
    "Samgr_Networking",
    "ohos.distributeddata.service",
    "ohos.dslm",
    "ohos.deviceprofile",
    "distributed_bundle_framework",
    "ohos.dhardware",
    "ohos.security.distributed_access_token",
    "ohos.storage.distributedfile.daemon",
};
const std::string PERMISSION_JSON_PATH = "/system/etc/device_manager/dm_permission.json";
const std::string ALL_PROC = "all";
constexpr int32_t MAX_INTERFACE_SIZE = 20;
}

int32_t PermissionManager::Init()
{
    LOGI("PermissionManager::call!");
    if (!LoadPermissionCfg(PERMISSION_JSON_PATH)) {
        return ERR_DM_FAILED;
    }
    LOGI("PermissionManager::init succeeded");
    return DM_OK;
}

int32_t PermissionManager::UnInit()
{
    LOGI("PermissionManager::UnInit");
    std::lock_guard<std::mutex> autoLock(permissionMutex_);
    permissionMap_.clear();
    return DM_OK;
}

int32_t PermissionManager::LoadPermissionCfg(const std::string &filePath)
{
    char path[PATH_MAX + 1] = {0x00};
    if (filePath.length() == 0 || filePath.length() > PATH_MAX || realpath(filePath.c_str(), path) == nullptr) {
        LOGE("File canonicalization failed");
        return ERR_DM_FAILED;
    }
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        LOGE("load json file failed");
        return ERR_DM_FAILED;
    }
    std::string fileContent(std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{});
    ifs.close();

    cJSON *permissionJson = cJSON_Parse(fileContent.c_str());
    if (!cJSON_IsObject(permissionJson)) {
        LOGE("Permission json parse failed!");
        cJSON_Delete(permissionJson);
        return ERR_DM_FAILED;
    }
    int32_t parseResult = ParsePermissionJson(permissionJson);
    LOGI("PermissionManager::permission json load result %{public}d!", parseResult);
    cJSON_Delete(permissionJson);
    return parseResult;
}

int32_t PermissionManager::ParsePermissionJson(const cJSON *const permissionJson)
{
    int size = cJSON_GetArraySize(permissionJson);
    if (size == 0 || size > MAX_INTERFACE_SIZE) {
        LOGE("Permission json size is invalid!size: %{public}d!", size);
        return ERR_DM_FAILED;
    }
    SetPermissionMap(permissionJson, IMPORT_AUTHCODE);
    SetPermissionMap(permissionJson, EXPORT_AUTHCODE);
    SetPermissionMap(permissionJson, REGISTER_PINHOLDER_CALLBACK);
    SetPermissionMap(permissionJson, CREATE_PINHOLDER);
    SetPermissionMap(permissionJson, DESTROY_PINHOLDER);
    SetPermissionMap(permissionJson, SET_DNPOLICY);
    SetPermissionMap(permissionJson, BIND_FOR_DEVICE_LEVEL);
    char *jsonChars = cJSON_PrintUnformatted(permissionJson);
    if (jsonChars == NULL) {
        LOGW("cJSON formatted to string failed!");
    } else {
        LOGI("permission json parse success!");
        cJSON_free(jsonChars);
    }
    return DM_OK;
}

void PermissionManager::SetPermissionMap(const cJSON *const permissionJson, const std::string& interfaceName)
{
    cJSON *item = cJSON_GetObjectItem(permissionJson, interfaceName.c_str());
    int32_t itemSize = static_cast<int32_t>(cJSON_GetArraySize(item));
    if (!cJSON_IsArray(item) || itemSize == 0 || itemSize > MAX_INTERFACE_SIZE) {
        LOGE("PermissionJson not contains the key, %{public}s!", interfaceName.c_str());
        return;
    }
    std::unordered_set<std::string> interfaceNameSets;
    item = item->child;
    while (item != NULL) {
        if (cJSON_IsString(item)) {
            interfaceNameSets.emplace(item->valuestring);
        }
        item = item->next;
    }
    {
        std::lock_guard<std::mutex> autoLock(permissionMutex_);
        permissionMap_[interfaceName] = interfaceNameSets;
    }
}

bool PermissionManager::CheckInterfacePermission(const std::string &interfaceName)
{
    std::string callProcName = "";
    if (GetCallerProcessName(callProcName) != DM_OK) {
        LOGE("Get caller process name failed");
        return false;
    }
    std::unordered_set<std::string> permittedProcNames;
    {
        std::lock_guard<std::mutex> autoLock(permissionMutex_);
        permittedProcNames = permissionMap_[interfaceName];
    }
    bool checkResult = (permittedProcNames.count(callProcName) != 0 || permittedProcNames.count(ALL_PROC) != 0);
    LOGD("success interface %{public}s callProc %{public}s!", interfaceName.c_str(), callProcName.c_str());
    return checkResult;
}

bool PermissionManager::CheckPermission(void)
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("CheckPermission GetCallingTokenID error.");
        return false;
    }
    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_HAP || tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        if (AccessTokenKit::VerifyAccessToken(tokenCaller, DM_SERVICE_ACCESS_PERMISSION) !=
            PermissionState::PERMISSION_GRANTED) {
            LOGE("DM service access is denied, please apply for corresponding permissions");
            return false;
        }
    }
    return true;
}

bool PermissionManager::CheckNewPermission(void)
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("CheckNewPermission GetCallingTokenID error.");
        return false;
    }
    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_HAP || tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        if (AccessTokenKit::VerifyAccessToken(tokenCaller, DM_SERVICE_ACCESS_NEWPERMISSION) !=
            PermissionState::PERMISSION_GRANTED) {
            LOGE("DM service access is denied, please apply for corresponding new permissions");
            return false;
        }
    }
    return true;
}

bool PermissionManager::CheckMonitorPermission(void)
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("CheckMonitorPermission GetCallingTokenID error.");
        return false;
    }
    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        if (AccessTokenKit::VerifyAccessToken(tokenCaller, DM_MONITOR_DEVICE_NETWORK_STATE_PERMISSION) !=
            PermissionState::PERMISSION_GRANTED) {
            LOGE("DM service access is denied, please apply for corresponding permissions.");
            return false;
        }
    }
    return true;
}

int32_t PermissionManager::GetCallerProcessName(std::string &processName)
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("GetCallerProcessName GetCallingTokenID error.");
        return ERR_DM_FAILED;
    }
    LOGI("GetCallerProcessName::tokenCaller ID == %{public}s", GetAnonyInt32(tokenCaller).c_str());
    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_HAP) {
        HapTokenInfo tokenInfo;
        if (AccessTokenKit::GetHapTokenInfo(tokenCaller, tokenInfo) != EOK) {
            LOGE("GetHapTokenInfo failed.");
            return ERR_DM_FAILED;
        }
        processName = std::move(tokenInfo.bundleName);
        uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
        if (!OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
            LOGE("GetCallerProcessName %{public}s not system hap.", processName.c_str());
            return ERR_DM_FAILED;
        }
    } else if (tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        NativeTokenInfo tokenInfo;
        if (AccessTokenKit::GetNativeTokenInfo(tokenCaller, tokenInfo) != EOK) {
            LOGE("GetNativeTokenInfo failed.");
            return ERR_DM_FAILED;
        }
        processName = std::move(tokenInfo.processName);
    } else {
        LOGE("GetCallerProcessName failed, unsupported process.");
        return ERR_DM_FAILED;
    }

    LOGI("Get process name: %{public}s success.", processName.c_str());
    return DM_OK;
}

bool PermissionManager::CheckWhiteListSystemSA(const std::string &pkgName)
{
    for (uint16_t index = 0; index < SYSTEM_SA_WHITE_LIST_NUM; ++index) {
        std::string tmp(SYSTEM_SA_WHITE_LIST[index]);
        if (pkgName == tmp) {
            return true;
        }
    }
    return false;
}

std::unordered_set<std::string> PermissionManager::GetWhiteListSystemSA()
{
    std::unordered_set<std::string> systemSA;
    for (uint16_t index = 0; index < SYSTEM_SA_WHITE_LIST_NUM; ++index) {
        std::string tmp(SYSTEM_SA_WHITE_LIST[index]);
        systemSA.insert(tmp);
    }
    return systemSA;
}

bool PermissionManager::CheckSystemSA(const std::string &pkgName)
{
    
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("CheckMonitorPermission GetCallingTokenID error.");
        return false;
    }
    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS
