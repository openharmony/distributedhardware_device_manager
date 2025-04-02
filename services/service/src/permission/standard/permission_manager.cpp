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

#include "accesstoken_kit.h"
#include "access_token.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace DistributedHardware {
DM_IMPLEMENT_SINGLE_INSTANCE(PermissionManager);
namespace {
constexpr const char* DM_SERVICE_ACCESS_PERMISSION = "ohos.permission.ACCESS_SERVICE_DM";
constexpr const char* DM_SERVICE_ACCESS_NEWPERMISSION = "ohos.permission.DISTRIBUTED_DATASYNC";
constexpr const char* DM_MONITOR_DEVICE_NETWORK_STATE_PERMISSION = "ohos.permission.MONITOR_DEVICE_NETWORK_STATE";
constexpr int32_t AUTH_CODE_WHITE_LIST_NUM = 6;
constexpr const static char* g_authCodeWhiteList[AUTH_CODE_WHITE_LIST_NUM] = {
    "com.huawei.msdp.hmringgenerator",
    "com.huawei.msdp.hmringdiscriminator",
    "CollaborationFwk",
    "wear_link_service",
    "watch_system_service",
    "cast_engine_service",
};

constexpr int32_t PIN_HOLDER_WHITE_LIST_NUM = 1;
constexpr const static char* g_pinHolderWhiteList[PIN_HOLDER_WHITE_LIST_NUM] = {
    "CollaborationFwk",
};

constexpr int32_t SYSTEM_SA_WHITE_LIST_NUM = 9;
constexpr const static char* SYSTEM_SA_WHITE_LIST[SYSTEM_SA_WHITE_LIST_NUM] = {
    "Samgr_Networking",
    "ohos.distributeddata.service",
    "ohos.dslm",
    "ohos.deviceprofile",
    "distributed_bundle_framework",
    "ohos.dhardware",
    "ohos.security.distributed_access_token",
    "ohos.storage.distributedfile.daemon",
    "audio_manager_service",
};

constexpr uint32_t SETDNPOLICY_WHITE_LIST_NUM = 4;
constexpr const static char* g_setDnPolicyWhiteList[SETDNPOLICY_WHITE_LIST_NUM] = {
    "collaboration_service",
    "watch_system_service",
    "com.huawei.hmos.walletservice",
    "com.ohos.distributedjstest",
};

constexpr uint32_t GETDEVICEINFO_WHITE_LIST_NUM = 3;
constexpr const static char* g_getDeviceInfoWhiteList[GETDEVICEINFO_WHITE_LIST_NUM] = {
    "gameservice_server",
    "com.huawei.hmos.slassistant",
    "token_sync_service",
};
constexpr int32_t MODIFY_LOCAL_DEVICE_NAME_WHITE_LIST_NUM = 2;
constexpr const static char* g_modifyLocalDeviceNameWhiteList[MODIFY_LOCAL_DEVICE_NAME_WHITE_LIST_NUM] = {
    "com.huawei.hmos.settings",
    "com.huawei.hmos.tvcooperation",
};
constexpr int32_t MODIFY_REMOTE_DEVICE_NAME_WHITE_LIST_NUM = 1;
constexpr const static char* g_modifyRemoteDeviceNameWhiteList[MODIFY_REMOTE_DEVICE_NAME_WHITE_LIST_NUM] = {
    "com.ohos.settings",
};

constexpr int32_t PUT_DEVICE_PROFILE_INFO_LIST_WHITE_LIST_NUM = 2;
constexpr const static char* g_putDeviceProfileInfoListWhiteList[PUT_DEVICE_PROFILE_INFO_LIST_WHITE_LIST_NUM] = {
    "com.huawei.hmos.ailifesvc",
    "com.huawei.hmos.tvcooperation",
};
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

bool PermissionManager::CheckProcessNameValidOnAuthCode(const std::string &processName)
{
    LOGI("Enter PermissionManager::CheckProcessNameValidOnAuthCode");
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }

    uint16_t index = 0;
    for (; index < AUTH_CODE_WHITE_LIST_NUM; ++index) {
        std::string tmp(g_authCodeWhiteList[index]);
        if (processName == tmp) {
            return true;
        }
    }

    LOGE("CheckProcessNameValidOnAuthCode process name: %{public}s invalid.", processName.c_str());
    return false;
}

bool PermissionManager::CheckProcessNameValidOnPinHolder(const std::string &processName)
{
    LOGI("Enter PermissionManager::CheckProcessNameValidOnPinHolder");
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }

    uint16_t index = 0;
    for (; index < PIN_HOLDER_WHITE_LIST_NUM; ++index) {
        std::string tmp(g_pinHolderWhiteList[index]);
        if (processName == tmp) {
            return true;
        }
    }

    LOGE("CheckProcessNameValidOnPinHolder process name: %{public}s invalid.", processName.c_str());
    return false;
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
    LOGI("Get calling tokenID.");
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("CheckMonitorPermission GetCallingTokenID error.");
        return false;
    }
    LOGI("Get token type flag.");
    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        return true;
    }
    return false;
}

bool PermissionManager::CheckProcessNameValidOnSetDnPolicy(const std::string &processName)
{
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }
    uint16_t index = 0;
    for (; index < SETDNPOLICY_WHITE_LIST_NUM; ++index) {
        std::string tmp(g_setDnPolicyWhiteList[index]);
        if (processName == tmp) {
            return true;
        }
    }

    LOGE("Process name: %{public}s invalid.", processName.c_str());
    return false;
}

bool PermissionManager::CheckProcessNameValidOnGetDeviceInfo(const std::string &processName)
{
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }
    uint16_t index = 0;
    for (; index < GETDEVICEINFO_WHITE_LIST_NUM; ++index) {
        std::string tmp(g_getDeviceInfoWhiteList[index]);
        if (processName == tmp) {
            return true;
        }
    }

    LOGE("Process name: %{public}s invalid.", processName.c_str());
    return false;
}

bool PermissionManager::CheckProcessNameValidModifyLocalDeviceName(const std::string &processName)
{
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }
    uint16_t index = 0;
    for (; index < MODIFY_LOCAL_DEVICE_NAME_WHITE_LIST_NUM; ++index) {
        std::string tmp(g_modifyLocalDeviceNameWhiteList[index]);
        if (processName == tmp) {
            return true;
        }
    }

    LOGE("Process name: %{public}s invalid.", processName.c_str());
    return false;
}

bool PermissionManager::CheckProcessNameValidModifyRemoteDeviceName(const std::string &processName)
{
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }
    uint16_t index = 0;
    for (; index < MODIFY_REMOTE_DEVICE_NAME_WHITE_LIST_NUM; ++index) {
        std::string tmp(g_modifyRemoteDeviceNameWhiteList[index]);
        if (processName == tmp) {
            return true;
        }
    }

    LOGE("Process name: %{public}s invalid.", processName.c_str());
    return false;
}

bool PermissionManager::CheckProcessNameValidPutDeviceProfileInfoList(const std::string &processName)
{
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }
    uint16_t index = 0;
    for (; index < PUT_DEVICE_PROFILE_INFO_LIST_WHITE_LIST_NUM; ++index) {
        std::string tmp(g_putDeviceProfileInfoListWhiteList[index]);
        if (processName == tmp) {
            return true;
        }
    }

    LOGE("Process name: %{public}s invalid.", processName.c_str());
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS
