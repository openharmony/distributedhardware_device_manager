/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "dm_constants.h"
#include "dm_log.h"
#include "json_config.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
void from_json(const nlohmann::json &jsonObject, AdapterSoLoadInfo &soLoadInfo)
{
    if (!IsString(jsonObject, "name") || !IsString(jsonObject, "type") || !IsString(jsonObject, "version") ||
        !IsString(jsonObject, "funcName") || !IsString(jsonObject, "soName") || !IsString(jsonObject, "soPath")) {
        LOGE("AdapterSoLoadInfo json key Not complete");
        return;
    }
    soLoadInfo.name = jsonObject["name"].get<std::string>();
    soLoadInfo.type = jsonObject["type"].get<std::string>();
    soLoadInfo.version = jsonObject["version"].get<std::string>();
    soLoadInfo.funcName = jsonObject["funcName"].get<std::string>();
    soLoadInfo.soName = jsonObject["soName"].get<std::string>();
    soLoadInfo.soPath = jsonObject["soPath"].get<std::string>();
}

void from_json(const nlohmann::json &jsonObject, AuthSoLoadInfo &soLoadInfo)
{
    if (!IsString(jsonObject, "name") || !IsString(jsonObject, "type") || !IsString(jsonObject, "version") ||
        !IsString(jsonObject, "funcName") || !IsString(jsonObject, "soName") || !IsString(jsonObject, "soPath") ||
        !IsInt32(jsonObject, "authType")) {
        LOGE("AuthSoLoadInfo json key Not complete");
        return;
    }
    soLoadInfo.authType = jsonObject["authType"].get<int32_t>();
    soLoadInfo.name = jsonObject["name"].get<std::string>();
    soLoadInfo.type = jsonObject["type"].get<std::string>();
    soLoadInfo.version = jsonObject["version"].get<std::string>();
    soLoadInfo.funcName = jsonObject["funcName"].get<std::string>();
    soLoadInfo.soName = jsonObject["soName"].get<std::string>();
    soLoadInfo.soPath = jsonObject["soPath"].get<std::string>();
}

DmConfigManager &DmConfigManager::GetInstance()
{
    static DmConfigManager instance;
    return instance;
}

DmConfigManager::DmConfigManager()
{
    LOGI("DmConfigManager constructor");
    do {
        nlohmann::json adapterJsonObject = nlohmann::json::parse(adapterJsonConfigString, nullptr, false);
        if (adapterJsonObject.is_discarded()) {
            LOGE("adapter json config string parse error");
            break;
        }
        if (!IsArray(adapterJsonObject, ADAPTER_LOAD_JSON_KEY)) {
            LOGE("adapter json config string key not exist");
            break;
        }
        auto soLoadInfo = adapterJsonObject[ADAPTER_LOAD_JSON_KEY].get<std::vector<AdapterSoLoadInfo>>();
        for (uint32_t i = 0; i < soLoadInfo.size(); i++) {
            if (soLoadInfo[i].name.size() == 0 || soLoadInfo[i].type.size() == 0 || soLoadInfo[i].version.size() == 0 ||
                soLoadInfo[i].funcName.size() == 0 || soLoadInfo[i].soName.size() == 0 ||
                soLoadInfo[i].soPath.size() == 0) {
                LOGE("adapter json config string exist invalid members");
                continue;
            }
            soLoadInfo[i].soPath = std::string(LIB_LOAD_PATH);
            soAdapterLoadInfo_[soLoadInfo[i].soName] = soLoadInfo[i];
            LOGI("soAdapterLoadInfo name is: %s", soLoadInfo[i].name.c_str());
            LOGI("soAdapterLoadInfo type is: %s", soLoadInfo[i].type.c_str());
            LOGI("soAdapterLoadInfo version is: %s", soLoadInfo[i].version.c_str());
            LOGI("soAdapterLoadInfo funcName is: %s", soLoadInfo[i].funcName.c_str());
            LOGI("soAdapterLoadInfo soName is: %s", soLoadInfo[i].soName.c_str());
            LOGI("soAdapterLoadInfo soPath is: %s", soLoadInfo[i].soPath.c_str());
        }
    } while (0);

    do {
        nlohmann::json authJsonObject = nlohmann::json::parse(authJsonConfigString, nullptr, false);
        if (authJsonObject.is_discarded()) {
            LOGE("auth json config string parse error!\n");
            break;
        }
        if (!IsArray(authJsonObject, AUTH_LOAD_JSON_KEY)) {
            LOGE("auth json config string key not exist!\n");
            break;
        }
        auto soLoadInfo = authJsonObject[AUTH_LOAD_JSON_KEY].get<std::vector<AuthSoLoadInfo>>();
        for (uint32_t i = 0; i < soLoadInfo.size(); i++) {
            if (soLoadInfo[i].name.size() == 0 || soLoadInfo[i].type.size() == 0 || soLoadInfo[i].version.size() == 0 ||
                soLoadInfo[i].funcName.size() == 0 || soLoadInfo[i].soName.size() == 0 ||
                soLoadInfo[i].soPath.size() == 0) {
                LOGE("adapter json config string exist invalid members");
                continue;
            }
            soLoadInfo[i].soPath = std::string(LIB_LOAD_PATH);
            soAuthLoadInfo_[soLoadInfo[i].authType] = soLoadInfo[i];
            LOGI("soAuthLoadInfo name is: %s", soLoadInfo[i].name.c_str());
            LOGI("soAuthLoadInfo type is: %s", soLoadInfo[i].type.c_str());
            LOGI("soAuthLoadInfo version is: %s", soLoadInfo[i].version.c_str());
            LOGI("soAuthLoadInfo funcName is: %s", soLoadInfo[i].funcName.c_str());
            LOGI("soAuthLoadInfo soName is: %s", soLoadInfo[i].soName.c_str());
            LOGI("soAuthLoadInfo soPath is: %s", soLoadInfo[i].soPath.c_str());
            LOGI("soAuthLoadInfo authType is: %d", soLoadInfo[i].authType);
        }
    } while (0);
}

DmConfigManager::~DmConfigManager()
{
    void *so_handle = nullptr;
    for (auto iter = soAdapterLoadInfo_.begin(); iter != soAdapterLoadInfo_.end(); iter++) {
        std::string soPathName = (iter->second).soPath + (iter->second).soName;
        so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
        if (so_handle != nullptr) {
            dlclose(so_handle);
        }
    }
    for (auto iter = soAuthLoadInfo_.begin(); iter != soAuthLoadInfo_.end(); iter++) {
        std::string soPathName = (iter->second).soPath + (iter->second).soName;
        so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
        if (so_handle != nullptr) {
            dlclose(so_handle);
        }
    }
    LOGI("DmAdapterManager destructor");
}

std::shared_ptr<IDecisionAdapter> DmConfigManager::GetDecisionAdapter(const std::string &soName)
{
    if (soName.empty()) {
        LOGE("soName size is zero");
        return nullptr;
    }
    auto soInfoIter = soAdapterLoadInfo_.find(soName);
    if (soInfoIter == soAdapterLoadInfo_.end() || (soInfoIter->second).type != std::string(DECISION_JSON_TYPE_KEY)) {
        LOGE("not find so info or type key not match");
        return nullptr;
    }
    std::unique_lock<std::mutex> locker(decisionAdapterMutex_);
    auto ptrIter = decisionAdapterPtr_.find(soName);
    if (ptrIter != decisionAdapterPtr_.end()) {
        return decisionAdapterPtr_[soName];
    }
    void *so_handle = nullptr;
    std::string soPathName = (soInfoIter->second).soPath + (soInfoIter->second).soName;
    so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
    if (so_handle == nullptr) {
        so_handle = dlopen(soPathName.c_str(), RTLD_NOW);
        if (so_handle == nullptr) {
            LOGE("load decision so %s failed", soName.c_str());
            return nullptr;
        }
    }
    dlerror();
    auto func = (CreateIDecisionAdapterFuncPtr)dlsym(so_handle, (soInfoIter->second).funcName.c_str());
    if (dlerror() != nullptr || func == nullptr) {
        LOGE("Create object function is not exist");
        return nullptr;
    }
    std::shared_ptr<IDecisionAdapter> iDecisionAdapter(func());
    decisionAdapterPtr_[soName] = iDecisionAdapter;
    return decisionAdapterPtr_[soName];
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

    std::unique_lock<std::mutex> locker(cryptoAdapterMutex_);
    auto ptrIter = cryptoAdapterPtr_.find(soName);
    if (ptrIter != cryptoAdapterPtr_.end()) {
        return cryptoAdapterPtr_[soName];
    }

    void *so_handle = nullptr;
    std::string soPathName = (soInfoIter->second).soPath + (soInfoIter->second).soName;
    so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
    if (so_handle == nullptr) {
        so_handle = dlopen(soPathName.c_str(), RTLD_NOW);
        if (so_handle == nullptr) {
            LOGE("load crypto so %s failed", soName.c_str());
            return nullptr;
        }
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
        std::string soPathName = (iter->second).soPath + (iter->second).soName;
        so_handle = dlopen(soPathName.c_str(), RTLD_NOW | RTLD_NOLOAD);
        if (so_handle == nullptr) {
            so_handle = dlopen(soPathName.c_str(), RTLD_NOW);
            if (so_handle == nullptr) {
                LOGE("load auth so %s failed", (iter->second).soName.c_str());
                continue;
            }
        }

        dlerror();
        auto func = (CreateIAuthAdapterFuncPtr)dlsym(so_handle, (iter->second).funcName.c_str());
        if (dlerror() != nullptr || func == nullptr) {
            LOGE("Create object function is not exist");
            continue;
        }

        std::shared_ptr<IAuthentication> iAuthentication(func());
        authAdapter[iter->first] = iAuthentication;
        LOGI("so name: %s, auth type: %d", (iter->second).soName.c_str(), iter->first);
    }
}
} // namespace DistributedHardware
} // namespace OHOS