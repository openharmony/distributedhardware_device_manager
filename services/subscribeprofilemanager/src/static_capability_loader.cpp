/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dlfcn.h>
#include <fstream>
#include <regex>

#include "static_capability_loader.h"

#include "config_policy_utils.h"
#include "content_sensor_manager_utils.h"
#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_enums.h"
#include "distributed_device_profile_errors.h"
#include "i_static_capability_collector.h"
#include "profile_utils.h"

namespace OHOS {
namespace DistributedDeviceProfile {
IMPLEMENT_SINGLE_INSTANCE(StaticCapabilityLoader);
namespace {
    const std::string TAG = "StaticCapabilityLoader";
}
using StaticCapabilityHandler = IStaticCapabilityCollector *(*)();
int32_t StaticCapabilityLoader::Init()
{
    HILOGI("call!");
    return DP_SUCCESS;
}

int32_t StaticCapabilityLoader::UnInit()
{
    HILOGI("call!");
    return DP_SUCCESS;
}

int32_t StaticCapabilityLoader::LoadStaticCapability(std::string& staticCapability)
{
    HILOGD("call!");
    std::string fileContent = "";
    int32_t loadJsonResult = LoadJsonFile(STATIC_CAPABILITY_PATH, fileContent);
    if (loadJsonResult != DP_SUCCESS) {
        HILOGE("Load json failed, result: %{public}d!", loadJsonResult);
        return loadJsonResult;
    }
    cJSON* staticCapabilityJson = cJSON_Parse(fileContent.c_str());
    if (!cJSON_IsObject(staticCapabilityJson)) {
        HILOGE("Static capability json parse failed!");
        cJSON_Delete(staticCapabilityJson);
        return DP_LOAD_STATIC_CAP_FAIL;
    }
    int32_t getCapResult = GetStaticCapability(staticCapabilityJson, staticCapability);
    if (getCapResult != DP_SUCCESS) {
        HILOGE("Get static capability result %{public}d!", getCapResult);
        cJSON_Delete(staticCapabilityJson);
        return getCapResult;
    }
    HILOGI("success!");
    cJSON_Delete(staticCapabilityJson);
    return DP_SUCCESS;
}

int32_t StaticCapabilityLoader::LoadJsonFile(const std::string& filePath, std::string& fileContent)
{
    HILOGD("call!");
    if (filePath.empty() || filePath.size() > MAX_STRING_LEN) {
        HILOGE("filePath is invalid!");
        return DP_INVALID_PARAM;
    }
    char buf[MAX_PATH_LEN] = {0};
    char targetPath[PATH_MAX + 1] = {0x00};
    char *srcPath = GetOneCfgFile(filePath.c_str(), buf, MAX_PATH_LEN);
    if (srcPath == nullptr) {
        HILOGE("srcPath is invalid!");
        return DP_LOAD_JSON_FILE_FAIL;
    }
    if (strlen(srcPath) == 0 || strlen(srcPath) > PATH_MAX || realpath(srcPath, targetPath) == nullptr) {
        HILOGE("File canonicalization failed!");
        return DP_LOAD_JSON_FILE_FAIL;
    }
    std::ifstream ifs(targetPath);
    if (!ifs.is_open()) {
        HILOGE("load json file failed");
        return DP_LOAD_JSON_FILE_FAIL;
    }
    fileContent = std::string(std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>{});
    ifs.close();
    return DP_SUCCESS;
}

int32_t StaticCapabilityLoader::GetStaticCapability(const cJSON* const staticCapabilityJson,
    std::string& staticCapability)
{
    HILOGD("call!");
    if (!cJSON_IsObject(staticCapabilityJson)) {
        HILOGE("staticInfoJson is json object!");
        return DP_INVALID_PARAM;
    }
    cJSON* staticCapabilities = cJSON_GetObjectItem(staticCapabilityJson, STATIC_CAPABILITY_ATTRIBUTE.c_str());
    if (!cJSON_IsArray(staticCapabilities)) {
        HILOGE("StaticCapabilities is not Array!");
        return DP_PARSE_STATIC_CAP_FAIL;
    }
    int32_t capabilityNum = static_cast<int32_t>(cJSON_GetArraySize(staticCapabilities));
    if (capabilityNum == 0 || capabilityNum > MAX_STATIC_CAPABILITY_SIZE) {
        HILOGE("CapabilityNum is invalid, nums: %{public}d!", capabilityNum);
        return DP_PARSE_STATIC_CAP_FAIL;
    }
    InitStaticCapability(capabilityNum, staticCapability);
    SetStaticCapability(staticCapabilities, staticCapability);
    return DP_SUCCESS;
}

int32_t StaticCapabilityLoader::GetStaticInfo(const cJSON* const staticInfoJson, const std::string& staticCapability,
    std::string& staticVersion, std::unordered_map<std::string, CharacteristicProfile>& charProfiles)
{
    HILOGD("call!");
    if (!cJSON_IsObject(staticInfoJson)) {
        HILOGE("staticInfoJson is json object!");
        return DP_INVALID_PARAM;
    }
    cJSON* lastStaticInfoJson = GetLatestStaticInfoJson(staticInfoJson);
    if (lastStaticInfoJson == NULL) {
        HILOGE("lastStaticInfo is nullptr!");
        return DP_GET_STATIC_INFO_FAIL;
    }
    GetStaticVersion(lastStaticInfoJson, staticVersion);
    std::string localDeviceId = ContentSensorManagerUtils::GetInstance().ObtainLocalUdid();
    GenerateStaticProfiles(localDeviceId, staticCapability, lastStaticInfoJson, charProfiles);
    return DP_SUCCESS;
}

int32_t StaticCapabilityLoader::GetStaticInfoByVersion(const std::string& deviceId,
    const std::string& staticCapability, const cJSON* const staticInfoJson,
    const std::string& staticVersion, std::unordered_map<std::string, CharacteristicProfile>& charProfiles)
{
    HILOGD("call!");
    if (deviceId.empty() || deviceId.size() > MAX_STRING_LEN) {
        HILOGE("deviceId is invalid!");
        return DP_INVALID_PARAM;
    }
    if (!cJSON_IsObject(staticInfoJson)) {
        HILOGE("staticInfoJson is json object!");
        return DP_INVALID_PARAM;
    }
    if (staticVersion.empty() || staticVersion.size() > MAX_STRING_LEN) {
        HILOGE("staticVersion is invalid!");
        return DP_INVALID_PARAM;
    }
    cJSON* json = GetStaticInfoJsonByVersion(staticInfoJson, staticVersion);
    if (json == NULL) {
        HILOGE("staticInfoJson is nullptr!");
        return DP_GET_STATIC_INFO_FAIL;
    }
    GenerateStaticProfiles(deviceId, staticCapability, json, charProfiles);
    return DP_SUCCESS;
}

cJSON* StaticCapabilityLoader::GetLatestStaticInfoJson(const cJSON* const staticInfoJson)
{
    HILOGD("call!");
    if (!cJSON_IsObject(staticInfoJson)) {
        HILOGE("staticInfoJson is not object!");
        return NULL;
    }
    cJSON* staticInfos = cJSON_GetObjectItem(staticInfoJson, STATIC_INFO.c_str());
    if (!cJSON_IsArray(staticInfos)) {
        HILOGE("StaticInfos is not Array!");
        return NULL;
    }
    int32_t staticInfoNum = static_cast<int32_t>(cJSON_GetArraySize(staticInfos));
    if (staticInfoNum == 0 || staticInfoNum > MAX_STATIC_CAPABILITY_SIZE) {
        HILOGE("staticInfoNum is invalid, nums: %{public}d!", staticInfoNum);
        return NULL;
    }
    return cJSON_GetArrayItem(staticInfos, staticInfoNum - 1);
}

cJSON* StaticCapabilityLoader::GetStaticInfoJsonByVersion(const cJSON* const staticInfoJson,
    const std::string& staticVersion)
{
    HILOGD("call!");
    if (!cJSON_IsObject(staticInfoJson)) {
        HILOGE("staticInfoJson is not object!");
        return NULL;
    }
    if (staticVersion.empty() || staticVersion.size() > MAX_STRING_LEN) {
        HILOGE("staticVersion is invalid!");
        return NULL;
    }
    cJSON* staticInfos = cJSON_GetObjectItem(staticInfoJson, STATIC_INFO.c_str());
    if (!cJSON_IsArray(staticInfos)) {
        HILOGE("StaticInfos is not Array!");
        return NULL;
    }
    int32_t staticInfoNum = static_cast<int32_t>(cJSON_GetArraySize(staticInfos));
    if (staticInfoNum == 0 || staticInfoNum > MAX_STATIC_CAPABILITY_SIZE) {
        HILOGE("staticInfoNum is invalid, nums: %{public}d!", staticInfoNum);
        return NULL;
    }
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, staticInfos) {
        if (!cJSON_IsObject(item)) {
            HILOGE("Item is not object!");
            continue;
        }
        cJSON* versionItem = cJSON_GetObjectItemCaseSensitive(item, DP_VERSION.c_str());
        if (!cJSON_IsString(versionItem) || versionItem->valuestring == NULL) {
            HILOGE("Get DP_Version fail!");
            continue;
        }
        std::string version = versionItem->valuestring;
        if (StaticVersionCheck(staticVersion, version)) {
            HILOGI("Get staticInfoJson Success!");
            return item;
        }
    }
    HILOGE("staticInfoJson not found");
    return NULL;
}

int32_t StaticCapabilityLoader::GetStaticVersion(const cJSON* const lastStaticInfo, std::string& staticVersion)
{
    HILOGD("call!");
    if (!cJSON_IsObject(lastStaticInfo)) {
        HILOGE("LastStaticInfoItem is not object!");
        return DP_GET_STATIC_INFO_FAIL;
    }
    cJSON* dpVersionJson = cJSON_GetObjectItemCaseSensitive(lastStaticInfo, DP_VERSION.c_str());
    if (!cJSON_IsString(dpVersionJson) || dpVersionJson->valuestring == NULL) {
        HILOGE("dpVersionJson is invalid!");
        return DP_GET_STATIC_INFO_FAIL;
    }
    staticVersion = dpVersionJson->valuestring;
    return DP_SUCCESS;
}

int32_t StaticCapabilityLoader::GenerateStaticProfiles(const std::string& deviceId, const std::string& staticCapability,
    const cJSON* const staticInfoJson, std::unordered_map<std::string, CharacteristicProfile>& charProfiles)
{
    HILOGD("call!");
    if (deviceId.empty() || deviceId.size() > MAX_STRING_LEN) {
        HILOGE("deviceId is invalid!");
        return DP_INVALID_PARAM;
    }
    if (!cJSON_IsObject(staticInfoJson)) {
        HILOGE("staticInfoJson is not object!");
        return DP_GET_STATIC_INFO_FAIL;
    }
    cJSON* abilitiesJson = cJSON_GetObjectItemCaseSensitive(staticInfoJson, ABILITIES.c_str());
    if (!cJSON_IsArray(abilitiesJson)) {
        HILOGE("abilitiesJson is not array!");
        return DP_GET_STATIC_INFO_FAIL;
    }
    cJSON* abilityItem = NULL;
    cJSON_ArrayForEach(abilityItem, abilitiesJson) {
        if (!cJSON_IsObject(abilityItem)) {
            HILOGE("abilityItem is not object!");
            continue;
        }
        cJSON* abilityKeyItem = cJSON_GetObjectItemCaseSensitive(abilityItem, ABILITY_KEY.c_str());
        if (!cJSON_IsString(abilityKeyItem) || abilityKeyItem->valuestring == NULL) {
            HILOGE("Get abilityKeyItem fail!");
            continue;
        }
        cJSON* abilityValueItem = cJSON_GetObjectItemCaseSensitive(abilityItem, ABILITY_VALUE.c_str());
        if (!cJSON_IsObject(abilityValueItem)) {
            HILOGE("Get abilityValueItem fail!");
            continue;
        }
        std::string serviceId = abilityKeyItem->valuestring;
        if (!HasStaticCapability(serviceId, staticCapability)) {
            HILOGW("service: %{public}s does not have static capability", serviceId.c_str());
            continue;
        }
        HILOGD("service: %{public}s has static capability", serviceId.c_str());
        char* abilityValue = cJSON_Print(abilityValueItem);
        if (abilityValue == NULL) {
            HILOGE("Get abilityValue fail!");
            continue;
        }
        std::string charValue = abilityValue;
        cJSON_free(abilityValue);
        CharacteristicProfile characteristicProfile(deviceId, serviceId, STATIC_CHARACTERISTIC_KEY, charValue);
        charProfiles[ProfileUtils::GenerateCharProfileKey(deviceId, serviceId, STATIC_CHARACTERISTIC_KEY)] =
            characteristicProfile;
    }
    return DP_SUCCESS;
}

int32_t StaticCapabilityLoader::LoadStaticInfo(const std::string& staticCapability, std::string& staticVersion,
    std::unordered_map<std::string, CharacteristicProfile>& charProfiles)
{
    HILOGD("call!");
    if (staticCapability.empty() || staticCapability.size() > MAX_STRING_LEN) {
        HILOGE("staticCapability is invalid!");
        return DP_INVALID_PARAM;
    }
    std::string fileContent = "";
    int32_t loadJsonResult = LoadJsonFile(STATIC_INFO_PATH, fileContent);
    if (loadJsonResult != DP_SUCCESS) {
        HILOGE("Load staticInfo json failed, result: %{public}d!", loadJsonResult);
        return loadJsonResult;
    }
    cJSON* staticInfoJson = cJSON_Parse(fileContent.c_str());
    if (!cJSON_IsObject(staticInfoJson)) {
        HILOGE("Static info json parse failed!");
        cJSON_Delete(staticInfoJson);
        return DP_PARSE_STATIC_INFO_FAIL;
    }
    int32_t getInfoResult = GetStaticInfo(staticInfoJson, staticCapability, staticVersion, charProfiles);
    if (getInfoResult != DP_SUCCESS) {
        HILOGE("Get static info result %{public}d!", getInfoResult);
        cJSON_Delete(staticInfoJson);
        return getInfoResult;
    }
    HILOGI("success!");
    cJSON_Delete(staticInfoJson);
    return DP_SUCCESS;
}

int32_t StaticCapabilityLoader::LoadStaticProfiles(const std::string& deviceId, const std::string& staticCapability,
    const std::string& staticVersion, std::unordered_map<std::string, CharacteristicProfile>& charProfiles)
{
    HILOGD("call!");
    if (deviceId.empty() || deviceId.size() > MAX_STRING_LEN) {
        HILOGE("deviceId is invalid!");
        return DP_INVALID_PARAM;
    }
    if (staticCapability.empty() || staticCapability.size() > MAX_STRING_LEN) {
        HILOGE("staticCapability is invalid!");
        return DP_INVALID_PARAM;
    }
    if (staticVersion.empty() || staticVersion.size() > MAX_STRING_LEN) {
        HILOGE("staticVersion is invalid!");
        return DP_INVALID_PARAM;
    }
    std::string fileContent = "";
    int32_t loadJsonResult = LoadJsonFile(STATIC_INFO_PATH, fileContent);
    if (loadJsonResult != DP_SUCCESS) {
        HILOGE("Load staticInfo json failed, result: %{public}d!", loadJsonResult);
        return loadJsonResult;
    }
    cJSON* staticInfoJson = cJSON_Parse(fileContent.c_str());
    if (!cJSON_IsObject(staticInfoJson)) {
        HILOGE("Static info json parse failed!");
        cJSON_Delete(staticInfoJson);
        return DP_PARSE_STATIC_INFO_FAIL;
    }
    int32_t getInfoResult = GetStaticInfoByVersion(deviceId, staticCapability, staticInfoJson,
        staticVersion, charProfiles);
    if (getInfoResult != DP_SUCCESS) {
        HILOGE("Get static info result %{public}d!", getInfoResult);
        cJSON_Delete(staticInfoJson);
        return getInfoResult;
    }
    HILOGI("success!");
    cJSON_Delete(staticInfoJson);
    return DP_SUCCESS;
}

void StaticCapabilityLoader::InitStaticCapability(int32_t size, std::string& staticCapability)
{
    HILOGI("InitStaticCapability size %{public}d!", size);
    staticCapability = EMPTY_STRING;
    for (int32_t i = 0; i < size; i++) {
        staticCapability += DEFAULT_STATIC_VAL;
    }
    HILOGI("InitStaticCapability value %{public}s!", staticCapability.c_str());
}

void StaticCapabilityLoader::SetStaticCapability(const cJSON* const staticCapabilityItems,
    std::string& staticCapability)
{
    HILOGD("call!");
    if (!cJSON_IsArray(staticCapabilityItems)) {
        HILOGE("staticCapabilityItems is not json array!");
        return;
    }
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, staticCapabilityItems) {
        if (!cJSON_IsObject(item)) {
            HILOGE("Item is not object!");
            continue;
        }
        cJSON* nameItem = cJSON_GetObjectItemCaseSensitive(item, STATIC_CAP_HANDLER_NAME.c_str());
        if (!cJSON_IsString(nameItem) || nameItem->valuestring == NULL) {
            HILOGE("Get handler_name fail!");
            continue;
        }
        cJSON* locItem = cJSON_GetObjectItemCaseSensitive(item, STATIC_CAP_HANDLER_LOC.c_str());
        if (!cJSON_IsString(locItem) || locItem->valuestring == NULL) {
            HILOGE("Get handler_loc fail!");
            continue;
        }
        std::string handlerName = nameItem->valuestring;
        std::string handlerLoc = locItem->valuestring;
        SetStaticCapabilityFlag(handlerName, handlerLoc, staticCapability);
    }
}

void StaticCapabilityLoader::SetStaticCapabilityFlag(const std::string& handlerName, const std::string& handlerLoc,
    std::string& staticCapability)
{
    HILOGD("call!");
    if (handlerName.empty() || handlerName.size() > MAX_STRING_LEN) {
        HILOGE("handlerName is invalid!");
        return;
    }
    if (handlerLoc.empty() || handlerLoc.size() > MAX_STRING_LEN) {
        HILOGE("handlerLoc is invalid!");
        return;
    }
    if (CAPABILITY_FLAG_MAP.count(handlerName) == 0) {
        HILOGE("SetStaticCapabilityFlag fail, handlerName: %{public}s!", handlerName.c_str());
        return;
    }
    int32_t capabilityFlag = static_cast<int32_t>(CAPABILITY_FLAG_MAP.at(handlerName));
    if (capabilityFlag >= static_cast<int32_t>(staticCapability.size())) {
        HILOGE("SetStaticCapabilityFlag fail, handlerName: %{public}s!", handlerName.c_str());
        return;
    }
    char capabilityValue = GetStaticCapabilityValue(handlerLoc) ? SUPPORT_STATIC_VAL : NOT_SUPPORT_STATIC_VAL;
    staticCapability[capabilityFlag] = capabilityValue;
    HILOGI("handlerName: %{public}s, staticCapability: %{public}c", handlerName.c_str(), capabilityValue);
}

bool StaticCapabilityLoader::GetStaticCapabilityValue(const std::string& handlerLoc)
{
    HILOGD("call!");
    if (handlerLoc.length() == 0 || handlerLoc.length() > PATH_MAX) {
        HILOGE("File canonicalization failed!");
        return false;
    }
    void *so_handler = dlopen(handlerLoc.c_str(), RTLD_LAZY | RTLD_NODELETE);
    if (so_handler == nullptr) {
        HILOGE("%{public}s handler load failed, failed reason : %{public}s", handlerLoc.c_str(), dlerror());
        return false;
    }
    auto func = (StaticCapabilityHandler)dlsym(so_handler, "GetStaticCapabilityCollector");
    if (func == nullptr) {
        dlclose(so_handler);
        HILOGE("Get StaticCapabilityHandler is null, failed reason : %{public}s", dlerror());
        return false;
    }
    bool isSupportStaticCapability = func();
    HILOGI("GetStaticCapabilityValue %{public}d", isSupportStaticCapability);
    dlclose(so_handler);
    return isSupportStaticCapability;
}

bool StaticCapabilityLoader::HasStaticCapability(const std::string& serviceId, const std::string& staticCapability)
{
    HILOGD("call!");
    if (CAPABILITY_FLAG_MAP.find(serviceId) == CAPABILITY_FLAG_MAP.end()) {
        HILOGE("serviceId doesn't exist map, serviceId: %{public}s",
            ProfileUtils::GetAnonyString(serviceId).c_str());
        return false;
    }
    int32_t capabilityFlag = static_cast<int32_t>(CAPABILITY_FLAG_MAP.at(serviceId));
    if (capabilityFlag >= static_cast<int32_t>(staticCapability.size())) {
        HILOGE("HasStaticCapability fail, capabilityFlag is out of range, serviceId: %{public}s",
            ProfileUtils::GetAnonyString(serviceId).c_str());
        return false;
    }
    return staticCapability[capabilityFlag] == SUPPORT_STATIC_VAL;
}

bool StaticCapabilityLoader::StaticVersionCheck(const std::string& peerVersion, const std::string& localVersion)
{
    HILOGD("call!");
    if (peerVersion == localVersion) {
        HILOGI("staticVersion equal");
        return true;
    }
    if (!IsValidVersion(peerVersion) || !IsValidVersion(localVersion)) {
        HILOGE("Params are valid");
        return false;
    }
    return true;
}

bool StaticCapabilityLoader::IsValidVersion(const std::string& version)
{
    std::regex rule(STATIC_VERSION_RULES);
    return std::regex_match(version, rule);
}

} // namespace DistributedDeviceProfile
} // namespace OHOS
