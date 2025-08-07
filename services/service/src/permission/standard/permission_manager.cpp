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
constexpr const static char* AUTH_CODE_WHITE_LIST[] = {
    "CollaborationFwk",
    "wear_link_service",
    "watch_system_service",
    "cast_engine_service",
    "glasses_collaboration_service",
    "xr_glass_app_service",
};
constexpr int32_t AUTH_CODE_WHITE_LIST_NUM = std::size(AUTH_CODE_WHITE_LIST);

constexpr const static char* PIN_HOLDER_WHITE_LIST[] = {
    "CollaborationFwk",
};
constexpr int32_t PIN_HOLDER_WHITE_LIST_NUM = std::size(PIN_HOLDER_WHITE_LIST);

constexpr const static char* SYSTEM_SA_WHITE_LIST[] = {
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
};
constexpr int32_t SYSTEM_SA_WHITE_LIST_NUM = std::size(SYSTEM_SA_WHITE_LIST);

constexpr const static char* SETDNPOLICY_WHITE_LIST[] = {
    "collaboration_service",
    "watch_system_service",
    "com.huawei.hmos.walletservice",
    "com.ohos.distributedjstest",
    "glasses_collaboration_service",
};
constexpr uint32_t SETDNPOLICY_WHITE_LIST_NUM = std::size(SETDNPOLICY_WHITE_LIST);

constexpr const static char* GETDEVICEINFO_WHITE_LIST[] = {
    "gameservice_server",
    "com.huawei.hmos.slassistant",
    "token_sync_service",
};
constexpr uint32_t GETDEVICEINFO_WHITE_LIST_NUM = std::size(GETDEVICEINFO_WHITE_LIST);

constexpr const static char* MODIFY_LOCAL_DEVICE_NAME_WHITE_LIST[] = {
    "com.huawei.hmos.settings",
    "com.huawei.hmos.tvcooperation",
};
constexpr int32_t MODIFY_LOCAL_DEVICE_NAME_WHITE_LIST_NUM = std::size(MODIFY_LOCAL_DEVICE_NAME_WHITE_LIST);

constexpr const static char* MODIFY_REMOTE_DEVICE_NAME_WHITE_LIST[] = {
    "com.ohos.settings",
};
constexpr int32_t MODIFY_REMOTE_DEVICE_NAME_WHITE_LIST_NUM = std::size(MODIFY_REMOTE_DEVICE_NAME_WHITE_LIST);

constexpr const static char* PUT_DEVICE_PROFILE_INFO_LIST_WHITE_LIST[] = {
    "com.huawei.hmos.ailifesvc",
    "com.huawei.hmos.tvcooperation",
};
constexpr int32_t PUT_DEVICE_PROFILE_INFO_LIST_WHITE_LIST_NUM = std::size(PUT_DEVICE_PROFILE_INFO_LIST_WHITE_LIST);

constexpr const static char* GET_TRUSTED_DEVICE_LIST_WHITE_LIST[] = {
    "distributedsched",
};
constexpr uint32_t GET_TRUSTED_DEVICE_LIST_WHITE_LIST_NUM = std::size(GET_TRUSTED_DEVICE_LIST_WHITE_LIST);
}

bool PermissionManager::CheckPermission(void)
{
    return VerifyAccessTokenByPermissionName(DM_SERVICE_ACCESS_PERMISSION);
}

bool PermissionManager::CheckNewPermission(void)
{
    return VerifyAccessTokenByPermissionName(DM_SERVICE_ACCESS_NEWPERMISSION);
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
        if (AccessTokenKit::VerifyAccessToken(tokenCaller, DM_MONITOR_DEVICE_NETWORK_STATE_PERMISSION) ==
            PermissionState::PERMISSION_GRANTED) {
            return true;
        }
    }
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_HAP) {
        if (AccessTokenKit::VerifyAccessToken(tokenCaller, DM_SERVICE_ACCESS_PERMISSION) ==
            PermissionState::PERMISSION_GRANTED) {
            return true;
        }
    }
    LOGE("DM service access is denied, please apply for corresponding permissions.");
    return false;
}

int32_t PermissionManager::GetCallerProcessName(std::string &processName)
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("GetCallerProcessName GetCallingTokenID error.");
        return ERR_DM_FAILED;
    }
    LOGI("tokenCaller ID == %{public}s", GetAnonyInt32(tokenCaller).c_str());
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
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }
    uint16_t index = 0;
    for (; index < AUTH_CODE_WHITE_LIST_NUM; ++index) {
        std::string tmp(AUTH_CODE_WHITE_LIST[index]);
        if (processName == tmp) {
            return true;
        }
    }
    return false;
}

bool PermissionManager::CheckProcessNameValidOnPinHolder(const std::string &processName)
{
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }

    uint16_t index = 0;
    for (; index < PIN_HOLDER_WHITE_LIST_NUM; ++index) {
        std::string tmp(PIN_HOLDER_WHITE_LIST[index]);
        if (processName == tmp) {
            return true;
        }
    }
    return false;
}

bool PermissionManager::CheckWhiteListSystemSA(const std::string &pkgName)
{
    bool isInWhiteList = false;
    for (uint16_t index = 0; index < SYSTEM_SA_WHITE_LIST_NUM; ++index) {
        std::string tmp(SYSTEM_SA_WHITE_LIST[index]);
        if (pkgName == tmp) {
            isInWhiteList = true;
        }
    }
    if (isInWhiteList) {
        AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
        if (tokenCaller == 0) {
            LOGE("GetCallingTokenID error.");
            return false;
        }
        ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
        if (tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
            return true;
        }
        LOGE("callser not SA pkgName %{public}s.", GetAnonyString(pkgName).c_str());
        return false;
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

bool PermissionManager::CheckProcessNameValidOnSetDnPolicy(const std::string &processName)
{
    if (processName.empty()) {
        LOGE("ProcessName is empty");
        return false;
    }
    uint16_t index = 0;
    for (; index < SETDNPOLICY_WHITE_LIST_NUM; ++index) {
        std::string tmp(SETDNPOLICY_WHITE_LIST[index]);
        if (processName == tmp) {
            return true;
        }
    }
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
        std::string tmp(GETDEVICEINFO_WHITE_LIST[index]);
        if (processName == tmp) {
            return true;
        }
    }
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
        std::string tmp(MODIFY_LOCAL_DEVICE_NAME_WHITE_LIST[index]);
        if (processName == tmp) {
            return true;
        }
    }
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
        std::string tmp(MODIFY_REMOTE_DEVICE_NAME_WHITE_LIST[index]);
        if (processName == tmp) {
            return true;
        }
    }
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
        std::string tmp(PUT_DEVICE_PROFILE_INFO_LIST_WHITE_LIST[index]);
        if (processName == tmp) {
            return true;
        }
    }
    return false;
}

bool PermissionManager::VerifyAccessTokenByPermissionName(const std::string& permissionName)
{
    AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    if (tokenCaller == 0) {
        LOGE("GetCallingTokenID error.");
        return false;
    }
    ATokenTypeEnum tokenTypeFlag = AccessTokenKit::GetTokenTypeFlag(tokenCaller);
    if (tokenTypeFlag == ATokenTypeEnum::TOKEN_HAP || tokenTypeFlag == ATokenTypeEnum::TOKEN_NATIVE) {
        if (AccessTokenKit::VerifyAccessToken(tokenCaller, permissionName) == PermissionState::PERMISSION_GRANTED) {
            return true;
        }
    }
    LOGE("DM service access is denied, please apply for corresponding permissions");
    return false;
}

bool PermissionManager::CheckProcessValidOnGetTrustedDeviceList()
{
    std::string processName = "";
    if (PermissionManager::GetInstance().GetCallerProcessName(processName) != DM_OK) {
        LOGE("Get caller process name failed");
        return false;
    }
    uint16_t index = 0;
    for (; index < GET_TRUSTED_DEVICE_LIST_WHITE_LIST_NUM; ++index) {
        std::string tmp(GET_TRUSTED_DEVICE_LIST_WHITE_LIST[index]);
        if (processName == tmp) {
            return true;
        }
    }
    return false;
}
} // namespace DistributedHardware
} // namespace OHOS
