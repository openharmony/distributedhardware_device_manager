/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "dm_config_manager.h"

#include <dlfcn.h>

#include "dm_anonymous.h"
#include "dm_log.h"
#include "json_config.h"
#include "json_object.h"

#ifdef __LP64__
constexpr const char* DM_LIB_LOAD_PATH = "/system/lib64/";
#else
#if !(defined(__LITEOS_M__) || defined(LITE_DEVICE))
constexpr const char* DM_LIB_LOAD_PATH = "/system/lib/";
#else
constexpr const char* DM_LIB_LOAD_PATH = "/usr/lib/";
#endif
#endif

namespace OHOS {
namespace DistributedHardware {
constexpr const char* AUTH_LOAD_JSON_KEY = "devicemanager_auth_components";
constexpr const char* ADAPTER_LOAD_JSON_KEY = "devicemanager_adapter_components";
constexpr const char* AUTH_JSON_TYPE_KEY = "AUTHENTICATE";
constexpr const char* CPYPTO_JSON_TYPE_KEY = "CPYPTO";
void FromJson(const JsonItemObject &jsonObject, AdapterSoLoadInfo &soLoadInfo)
{
    if (!IsString(jsonObject, "name") || !IsString(jsonObject, "type") || !IsString(jsonObject, "version") ||
        !IsString(jsonObject, "funcName") || !IsString(jsonObject, "soName") || !IsString(jsonObject, "soPath")) {
        LOGE("AdapterSoLoadInfo json key Not complete");
        return;
    }
    soLoadInfo.name = jsonObject["name"].Get<std::string>();
    soLoadInfo.type = jsonObject["type"].Get<std::string>();
    soLoadInfo.version = jsonObject["version"].Get<std::string>();
    soLoadInfo.funcName = jsonObject["funcName"].Get<std::string>();
    soLoadInfo.soName = jsonObject["soName"].Get<std::string>();
    soLoadInfo.soPath = jsonObject["soPath"].Get<std::string>();
}

void FromJson(const JsonItemObject &jsonObject, AuthSoLoadInfo &soLoadInfo)
{
    if (!IsString(jsonObject, "name") || !IsString(jsonObject, "type") || !IsString(jsonObject, "version") ||
        !IsString(jsonObject, "funcName") || !IsString(jsonObject, "soName") || !IsString(jsonObject, "soPath") ||
        !IsInt32(jsonObject, "authType")) {
        LOGE("AdapterSoLoadInfo json key Not complete");
        return;
    }
    soLoadInfo.authType = jsonObject["authType"].Get<int32_t>();
    soLoadInfo.name = jsonObject["name"].Get<std::string>();
    soLoadInfo.type = jsonObject["type"].Get<std::string>();
    soLoadInfo.version = jsonObject["version"].Get<std::string>();
    soLoadInfo.funcName = jsonObject["funcName"].Get<std::string>();
    soLoadInfo.soName = jsonObject["soName"].Get<std::string>();
    soLoadInfo.soPath = jsonObject["soPath"].Get<std::string>();
}

DmConfigManager &DmConfigManager::GetInstance()
{
    static DmConfigManager instance;
    return instance;
}

void DmConfigManager::ParseAdapterConfig()
{
    JsonObject adapterJsonObject(adapterJsonConfigString);
    if (adapterJsonObject.IsDiscarded()) {
        LOGE("adapter json config string parse error");
        return;
    }
    if (!IsArray(adapterJsonObject, ADAPTER_LOAD_JSON_KEY)) {
        LOGE("adapter json config string key not exist");
        return;
    }
    std::vector<AdapterSoLoadInfo> soLoadInfo;
    adapterJsonObject[ADAPTER_LOAD_JSON_KEY].Get(soLoadInfo);
    for (uint32_t i = 0; i < soLoadInfo.size(); i++) {
        if (soLoadInfo[i].name.size() == 0 || soLoadInfo[i].type.size() == 0 || soLoadInfo[i].version.size() == 0 ||
            soLoadInfo[i].funcName.size() == 0 || soLoadInfo[i].soName.size() == 0 ||
            soLoadInfo[i].soPath.size() == 0) {
            LOGE("adapter json config string exist invalid members");
            continue;
        }
        soLoadInfo[i].soPath = std::string(DM_LIB_LOAD_PATH);
        soAdapterLoadInfo_[soLoadInfo[i].soName] = soLoadInfo[i];
    }
}

void DmConfigManager::ParseAuthConfig()
{
    JsonObject authJsonObject(authJsonConfigString);
    if (authJsonObject.IsDiscarded()) {
        LOGE("auth json config string parse error!\n");
        return;
    }
    if (!IsArray(authJsonObject, AUTH_LOAD_JSON_KEY)) {
        LOGE("auth json config string key not exist!\n");
        return;
    }
    std::vector<AuthSoLoadInfo> soLoadInfo;
    authJsonObject[AUTH_LOAD_JSON_KEY].Get(soLoadInfo);
    for (uint32_t i = 0; i < soLoadInfo.size(); i++) {
        if (soLoadInfo[i].name.size() == 0 || soLoadInfo[i].type.size() == 0 || soLoadInfo[i].version.size() == 0 ||
            soLoadInfo[i].funcName.size() == 0 || soLoadInfo[i].soName.size() == 0 ||
            soLoadInfo[i].soPath.size() == 0) {
            LOGE("adapter json config string exist invalid members");
            continue;
        }
        soLoadInfo[i].soPath = std::string(DM_LIB_LOAD_PATH);
        soAuthLoadInfo_[soLoadInfo[i].authType] = soLoadInfo[i];
    }
}

DmConfigManager::DmConfigManager()
{
    LOGI("DmConfigManager constructor");
    do {
        ParseAdapterConfig();
    } while (0);

    do {
        ParseAuthConfig();
    } while (0);
}

DmConfigManager::~DmConfigManager()
{
    void *so_handle = nullptr;
    for (auto iter = soAdapterLoadInfo_.begin(); iter != soAdapterLoadInfo_.end(); iter++) {
        std::string soPathName = (iter->second).soName;
        if ((soPathName.length() == 0) || (soPathName.length() > PATH_MAX)) {
            LOGE("File %{public}s canonicalization failed.", soPathName.c_str());
            continue;
        }
        so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
        if (so_handle != nullptr) {
            LOGI("DmConfigManager so_handle is not nullptr first.");
            dlclose(so_handle);
        }
    }
    for (auto iter = soAuthLoadInfo_.begin(); iter != soAuthLoadInfo_.end(); iter++) {
        std::string soPathName = (iter->second).soName;
        if ((soPathName.length() == 0) || (soPathName.length() > PATH_MAX)) {
            LOGE("File %{public}s canonicalization failed.", soPathName.c_str());
            continue;
        }
        so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
        if (so_handle != nullptr) {
            LOGI("DmConfigManager so_handle is not nullptr second.");
            dlclose(so_handle);
        }
    }
    LOGI("DmAdapterManager destructor");
}

std::shared_ptr<ICryptoAdapter> DmConfigManager::GetCryptoAdapter(const std::string &soName)
{
    if (soName.empty()) {
        LOGE("soName size is zero");
        return nullptr;
    }

    auto soInfoIter = soAdapterLoadInfo_.find(soName);
    if (soInfoIter == soAdapterLoadInfo_.end() || (soInfoIter->second).type != std::string(CPYPTO_JSON_TYPE_KEY)) {
        LOGE("not find so info or type key not match");
        return nullptr;
    }

    std::unique_lock<ffrt::mutex> locker(cryptoAdapterMutex_);
    auto ptrIter = cryptoAdapterPtr_.find(soName);
    if (ptrIter != cryptoAdapterPtr_.end()) {
        return cryptoAdapterPtr_[soName];
    }

    void *so_handle = nullptr;
    std::string soPathName = (soInfoIter->second).soName;
    if ((soPathName.length() == 0) || (soPathName.length() > PATH_MAX)) {
        LOGE("File %{public}s canonicalization failed.", soPathName.c_str());
        return nullptr;
    }
    so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NODELETE | RTLD_NOLOAD);
    if (so_handle == nullptr) {
        so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NODELETE);
    }
    if (so_handle == nullptr) {
        LOGE("load crypto so failed.");
        return nullptr;
    }

    dlerror();
    auto func = (CreateICryptoAdapterFuncPtr)dlsym(so_handle, (soInfoIter->second).funcName.c_str());
    if (dlerror() != nullptr || func == nullptr) {
        LOGE("Create object function is not exist");
        return nullptr;
    }

    std::shared_ptr<ICryptoAdapter> iCryptoAdapter(func());
    cryptoAdapterPtr_[soName] = iCryptoAdapter;
    return cryptoAdapterPtr_[soName];
}

void DmConfigManager::GetAuthAdapter(std::map<int32_t, std::shared_ptr<IAuthentication>> &authAdapter)
{
    authAdapter.clear();
    for (auto iter = soAuthLoadInfo_.begin(); iter != soAuthLoadInfo_.end(); iter++) {
        if ((iter->second).type != std::string(AUTH_JSON_TYPE_KEY)) {
            LOGE("type key not match");
            continue;
        }

        void *so_handle = nullptr;
        std::string soPathName = (iter->second).soName;
        if ((soPathName.length() == 0) || (soPathName.length() > PATH_MAX)) {
            LOGE("File %{public}s canonicalization failed.", soPathName.c_str());
            continue;
        }
        so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NODELETE | RTLD_NOLOAD);
        if (so_handle == nullptr) {
            so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NODELETE);
        }
        if (so_handle == nullptr) {
            LOGE("load auth so %{public}s failed", (iter->second).soName.c_str());
            continue;
        }

        dlerror();
        auto func = (CreateIAuthAdapterFuncPtr)dlsym(so_handle, (iter->second).funcName.c_str());
        if (dlerror() != nullptr || func == nullptr) {
            LOGE("Create object function is not exist");
            continue;
        }

        std::shared_ptr<IAuthentication> iAuthentication(func());
        authAdapter[iter->first] = iAuthentication;
        LOGI("so name: %{public}s, auth type: %{public}d", (iter->second).soName.c_str(), iter->first);
    }
}
} // namespace DistributedHardware
} // namespace OHOS