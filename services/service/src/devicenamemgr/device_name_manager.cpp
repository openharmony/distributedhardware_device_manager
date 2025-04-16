/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "device_name_manager.h"

#include "iservice_registry.h"
#include "multiple_user_connector.h"
#include "system_ability_definition.h"
#include "device_manager_service.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "parameter.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t USERID_HELPER_NUMBER = 100;
const std::string SETTING_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/";
const std::string SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
const std::string URI_PROXY_SUFFIX = "?Proxy=true";
const std::string SETTINGSDATA_GLOBAL = "SETTINGSDATA";
const std::string SETTINGS_GENERAL_DEVICE_NAME = "settings.general.device_name";
const std::string SETTINGSDATA_SYSTEM = "USER_SETTINGSDATA_";
const std::string SETTINGSDATA_SECURE = "USER_SETTINGSDATA_SECURE_";
const std::string DISPLAY_DEVICE_NAME = "display_device_name";
const std::string SETTINGS_GENERAL_DISPLAY_DEVICE_NAME = "settings.general." + DISPLAY_DEVICE_NAME;
const std::string USER_DEFINED_DEVICE_NAME = "user_defined_device_name";
const std::string SETTINGS_GENERAL_USER_DEFINED_DEVICE_NAME = "settings.general." + USER_DEFINED_DEVICE_NAME;
const std::string SETTINGS_GENERAL_DISPLAY_DEVICE_NAME_STATE = "settings.general.display_device_name_state";
const std::string SETTING_COLUMN_VALUE = "VALUE";
const std::string SETTING_COLUMN_KEYWORD = "KEYWORD";
constexpr int32_t NUM1 = 1;
constexpr int32_t NUM2 = 2;
constexpr int32_t NUM3 = 3;
constexpr int32_t NUM4 = 4;
constexpr int32_t NUM18 = 18;
constexpr int32_t NUM21 = 21;
constexpr int32_t NUM24 = 24;
constexpr int32_t DEFAULT_DEVICE_NAME_MAX_LENGTH = 12;
constexpr int32_t NAME_LENGTH_MIN = 18;
constexpr int32_t NAME_LENGTH_MAX = 100;
const std::string DEFAULT_CONCATENATION_CHARACTER = "...";
const std::string NAME_SEPARATOR_ZH = "的";
const std::string NAME_SEPARATOR_OTHER = "-";
constexpr const char *SOFTBUS_NAME_RAW_JSON_KEY = "raw";
constexpr const char *SOFTBUS_NAME_18_JSON_KEY = "name18";
constexpr const char *SOFTBUS_NAME_21_JSON_KEY = "name21";
constexpr const char *SOFTBUS_NAME_24_JSON_KEY = "name24";
constexpr int32_t SYSPARA_SIZE = 128;
constexpr const char *PERSIST_GLOBAL_LANGUAGE = "persist.global.language";
constexpr const char *DEFAULT_LANGUAGE_KEY  = "const.global.language";
const std::string LANGUAGE_ZH_HANS = "zh-Hans";
constexpr const char *PERSIST_GLOBAL_LOCALE = "persist.global.locale";
constexpr const char *DEFAULT_LOCALE_KEY = "const.global.locale";
const std::string LOCAL_ZH_HANS_CN = "zh-Hans-CN";
constexpr int32_t DEFAULT_USER_ID = -1;
}

IMPLEMENT_SINGLE_INSTANCE(DeviceNameManager);

void DeviceNameManager::DataShareReady()
{
    LOGI("In");
    isDataShareReady_ = true;
    if (DependsIsReady()) {
        int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
        InitDeviceName(userId);
        RegisterDeviceNameChangeMonitor(userId, DEFAULT_USER_ID);
    }
}

void DeviceNameManager::AccountSysReady(int32_t userId)
{
    LOGI("In userId : %{public}d", userId);
    isAccountSysReady_ = true;
    if ((userId != -1) && DependsIsReady()) {
        InitDeviceName(userId);
    }
}

bool DeviceNameManager::DependsIsReady()
{
    if (!isDataShareReady_) {
        LOGE("data share not ready");
        return false;
    }
    if (!isAccountSysReady_) {
        LOGE("Account system not ready");
        return false;
    }
    if (GetRemoteObj() == nullptr) {
        LOGE("dm sa not publish");
        return false;
    }
    return true;
}

int32_t DeviceNameManager::InitDeviceNameWhenSoftBusReady()
{
    LOGI("In");
    if (DependsIsReady()) {
        int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
        InitDeviceName(userId);
    }
    return DM_OK;
}

int32_t DeviceNameManager::UnInit()
{
    {
        std::lock_guard<std::mutex> lock(remoteObjMtx_);
        remoteObj_ = nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(monitorMapMtx_);
        monitorMap_.clear();
    }
    return DM_OK;
}

int32_t DeviceNameManager::InitDeviceNameWhenUserSwitch(int32_t curUserId, int32_t preUserId)
{
    isAccountSysReady_ = true;
    LOGI("In");
    if (DependsIsReady()) {
        InitDeviceName(curUserId);
        RegisterDeviceNameChangeMonitor(curUserId, preUserId);
    }
    return DM_OK;
}

int32_t DeviceNameManager::InitDeviceNameWhenLogout()
{
    LOGI("In");
    if (DependsIsReady()) {
        int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
        InitDeviceName(userId);
    }
    return DM_OK;
}

int32_t DeviceNameManager::InitDeviceNameWhenLogin()
{
    LOGI("In");
    if (DependsIsReady()) {
        int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
        InitDeviceName(userId);
    }
    return DM_OK;
}

int32_t DeviceNameManager::InitDeviceNameWhenNickChange()
{
    LOGI("In");
    if (DependsIsReady()) {
        int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
        InitDeviceName(userId);
    }
    return DM_OK;
}

int32_t DeviceNameManager::InitDeviceNameWhenNameChange(int32_t userId)
{
    LOGI("In");
    if (DependsIsReady()) {
        InitDeviceName(userId);
    }
    return DM_OK;
}

void DeviceNameManager::RegisterDeviceNameChangeMonitor(int32_t curUserId, int32_t preUserId)
{
    LOGI("In");
    UnRegisterDeviceNameChangeMonitor(preUserId);
    if (curUserId == DEFAULT_USER_ID) {
        LOGW("userId invalid");
        return;
    }
    sptr<DeviceNameChangeMonitor> monitor = nullptr;
    {
        std::lock_guard<std::mutex> lock(monitorMapMtx_);
        auto iter = monitorMap_.find(curUserId);
        if (iter != monitorMap_.end()) {
            return;
        }
        monitor = sptr<DeviceNameChangeMonitor>(new DeviceNameChangeMonitor(curUserId));
        if (monitor == nullptr) {
            LOGE("monitor is nullptr");
            return;
        }
        monitorMap_[curUserId] = monitor;
    }
    std::string proxyUri = GetProxyUriStr(SETTINGSDATA_SECURE, curUserId);
    auto helper = CreateDataShareHelper(proxyUri);
    if (helper == nullptr) {
        LOGE("helper is nullptr");
        {
            std::lock_guard<std::mutex> lock(monitorMapMtx_);
            auto iter = monitorMap_.find(curUserId);
            if (iter != monitorMap_.end()) {
                monitorMap_.erase(iter);
            }
        }
        return;
    }
    Uri uri = MakeUri(proxyUri, SETTINGS_GENERAL_USER_DEFINED_DEVICE_NAME);
    helper->RegisterObserver(uri, monitor);
    ReleaseDataShareHelper(helper);
}

void DeviceNameManager::UnRegisterDeviceNameChangeMonitor(int32_t userId)
{
    LOGI("In");
    if (userId == DEFAULT_USER_ID) {
        LOGW("userId invalid");
        return;
    }
    sptr<DeviceNameChangeMonitor> monitor = nullptr;
    {
        std::lock_guard<std::mutex> lock(monitorMapMtx_);
        auto iter = monitorMap_.find(userId);
        if (iter != monitorMap_.end()) {
            monitor = iter->second;
            monitorMap_.erase(iter);
        }
    }
    if (monitor == nullptr) {
        LOGW("monitor is nullptr");
        return;
    }
    std::string proxyUri = GetProxyUriStr(SETTINGSDATA_SECURE, userId);
    auto helper = CreateDataShareHelper(proxyUri);
    if (helper == nullptr) {
        LOGE("helper is nullptr");
        return;
    }
    Uri uri = MakeUri(proxyUri, SETTINGS_GENERAL_USER_DEFINED_DEVICE_NAME);
    helper->UnregisterObserver(uri, monitor);
    ReleaseDataShareHelper(helper);
}

void DeviceNameManager::InitDeviceName(int32_t userId)
{
    LOGI("In userId:%{public}d", userId);
    if (userId == -1) {
        LOGI("userId:%{public}d is invalid", userId);
        return;
    }
    std::string userDefinedDeviceName = "";
    GetUserDefinedDeviceName(userId, userDefinedDeviceName);
    if (!userDefinedDeviceName.empty()) {
        LOGI("userDefinedDeviceName:%{public}s", GetAnonyString(userDefinedDeviceName).c_str());
        InitDeviceNameToSoftBus("", userDefinedDeviceName);
        InitDisplayDeviceNameToSettingsData("", userDefinedDeviceName, userId);
        return;
    }
    std::string deviceName = GetLocalMarketName();
    if (deviceName.empty()) {
        LOGE("deviceName is empty");
        return;
    }
    std::string nickName = MultipleUserConnector::GetAccountNickName(userId);
    InitDeviceNameToSoftBus(nickName, deviceName);
    InitDisplayDeviceNameToSettingsData(nickName, deviceName, userId);
}

void DeviceNameManager::InitDeviceNameToSoftBus(const std::string &prefixName, const std::string &subffixName)
{
    LOGI("In prefixName:%{public}s, subffixName:%{public}s",
        GetAnonyString(prefixName).c_str(), GetAnonyString(subffixName).c_str());
    std::string raw = GetLocalDisplayDeviceName(prefixName, subffixName, 0);
    std::string name18 = GetLocalDisplayDeviceName(prefixName, subffixName, NUM18);
    std::string name21 = GetLocalDisplayDeviceName(prefixName, subffixName, NUM21);
    std::string name24 = GetLocalDisplayDeviceName(prefixName, subffixName, NUM24);
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        LOGE("cJSON_CreateObject fail!");
        return;
    }
    cJSON_AddStringToObject(root, SOFTBUS_NAME_RAW_JSON_KEY, raw.c_str());
    cJSON_AddStringToObject(root, SOFTBUS_NAME_18_JSON_KEY, name18.c_str());
    cJSON_AddStringToObject(root, SOFTBUS_NAME_21_JSON_KEY, name21.c_str());
    cJSON_AddStringToObject(root, SOFTBUS_NAME_24_JSON_KEY, name24.c_str());
    char *jsonChar = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (jsonChar == NULL) {
        LOGE("cJSON_PrintUnformatted fail!");
        return;
    }
    std::string displayName = jsonChar;
    cJSON_free(jsonChar);
    DeviceManagerService::GetInstance().SetLocalDisplayNameToSoftbus(displayName);
}

int32_t DeviceNameManager::GetLocalDisplayDeviceName(int32_t maxNamelength, std::string &displayName)
{
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    if (maxNamelength < 0 || (maxNamelength > 0 && maxNamelength < NAME_LENGTH_MIN) ||
        maxNamelength > NAME_LENGTH_MAX) {
        LOGE("maxNamelength:%{public}d is invalid", maxNamelength);
        return ERR_DM_INPUT_PARA_INVALID;
    }
    std::string userDefinedDeviceName = "";
    GetUserDefinedDeviceName(userId, userDefinedDeviceName);
    if (!userDefinedDeviceName.empty()) {
        LOGI("userDefinedDeviceName:%{public}s", GetAnonyString(userDefinedDeviceName).c_str());
        displayName = GetLocalDisplayDeviceName("", userDefinedDeviceName, maxNamelength);
        return DM_OK;
    }
    std::string nickName = MultipleUserConnector::GetAccountNickName(userId);
    std::string localMarketName = GetLocalMarketName();
    displayName = GetLocalDisplayDeviceName(nickName, localMarketName, maxNamelength);
    return DM_OK;
}

std::string DeviceNameManager::GetLocalDisplayDeviceName(const std::string &prefixName, const std::string &subffixName,
    int32_t maxNameLength)
{
    if (prefixName.empty()) {
        if (maxNameLength == 0 || static_cast<int32_t>(subffixName.size()) <= maxNameLength) {
            return subffixName;
        }
        return SubstrByBytes(subffixName, maxNameLength - NUM3) + DEFAULT_CONCATENATION_CHARACTER;
    }
    int32_t defaultNameMaxLength = DEFAULT_DEVICE_NAME_MAX_LENGTH;
    if (maxNameLength >= NUM21) {
        defaultNameMaxLength = DEFAULT_DEVICE_NAME_MAX_LENGTH + NUM3;
    }
    std::string nameSeparator = NAME_SEPARATOR_ZH;
    if (GetSystemLanguage() != LANGUAGE_ZH_HANS || GetSystemRegion() != LOCAL_ZH_HANS_CN) {
        nameSeparator = NAME_SEPARATOR_OTHER;
    }
    std::string displayName = prefixName + nameSeparator + subffixName;
    if (maxNameLength == 0 || static_cast<int32_t>(displayName.size()) <= maxNameLength) {
        return displayName;
    }
    std::string subffix = subffixName;
    if (static_cast<int32_t>(subffixName.size()) > defaultNameMaxLength) {
        subffix = SubstrByBytes(subffixName, defaultNameMaxLength - NUM3) + DEFAULT_CONCATENATION_CHARACTER;
    }
    int32_t remainingLen = maxNameLength - static_cast<int32_t>(subffix.size());
    if (remainingLen <= 0) {
        return subffix;
    }
    displayName = prefixName + nameSeparator + subffix;
    if (static_cast<int32_t>(displayName.size()) <= maxNameLength) {
        return displayName;
    }
    remainingLen = remainingLen - NUM3;
    std::string prefix = prefixName;
    if (static_cast<int32_t>(prefix.size()) > remainingLen) {
        prefix = SubstrByBytes(prefix, remainingLen) + DEFAULT_CONCATENATION_CHARACTER;
    }
    displayName = prefix + subffix;
    return displayName;
}

int32_t DeviceNameManager::ModifyUserDefinedName(const std::string &deviceName)
{
    LOGI("In");
    if (deviceName.empty()) {
        LOGE("deviceName is empty");
        return ERR_DM_NAME_EMPTY;
    }
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    SetUserDefinedDeviceName(deviceName, userId);
    SetDisplayDeviceNameState(USER_DEFINED_DEVICE_NAME, userId);
    SetDisplayDeviceName(deviceName, userId);
    InitDeviceNameToSoftBus("", deviceName);
    return DM_OK;
}

int32_t DeviceNameManager::RestoreLocalDeviceName()
{
    LOGI("In");
    int32_t userId = MultipleUserConnector::GetCurrentAccountUserID();
    SetUserDefinedDeviceName("", userId);
    SetDisplayDeviceNameState("", userId);
    std::string nickName = MultipleUserConnector::GetAccountNickName(userId);
    std::string localMarketName = GetLocalMarketName();
    InitDeviceNameToSoftBus(nickName, localMarketName);
    InitDisplayDeviceNameToSettingsData(nickName, localMarketName, userId);
    return DM_OK;
}

int32_t DeviceNameManager::InitDisplayDeviceNameToSettingsData(const std::string &nickName,
    const std::string &deviceName, int32_t userId)
{
    std::string newDisplayName = GetLocalDisplayDeviceName(nickName, deviceName, 0);
    std::string oldDisplayName = "";
    GetDisplayDeviceName(userId, oldDisplayName);
    if (oldDisplayName != newDisplayName) {
        SetDisplayDeviceName(newDisplayName, userId);
    }
    return DM_OK;
}

int32_t DeviceNameManager::GetUserDefinedDeviceName(int32_t userId, std::string &deviceName)
{
    return GetValue(SETTINGSDATA_SECURE, userId, SETTINGS_GENERAL_USER_DEFINED_DEVICE_NAME, deviceName);
}

std::string DeviceNameManager::SubstrByBytes(const std::string &str, int32_t maxNumBytes)
{
    int32_t length = static_cast<int32_t>(str.size());
    if (length <= maxNumBytes || maxNumBytes <= 0) {
        return str;
    }
    std::vector<std::string> substrVec;
    for (int32_t i = 0; i < length;) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        int numBytes = NUM1;
        if ((c & 0x80) == 0) {
            numBytes = NUM1;
        } else if ((c & 0xE0) == 0xC0) {
            numBytes = NUM2;
        } else if ((c & 0xF0) == 0xE0) {
            numBytes = NUM3;
        } else if ((c & 0xF8) == 0xF0) {
            numBytes = NUM4;
        } else {
            LOGE("Invalid characters");
            return "";
        }
        if (i + numBytes > length) {
            break;
        }
        std::string substr(str.begin() + i, str.begin() + i + numBytes);
        substrVec.emplace_back(substr);
        i += numBytes;
    }
    std::string result = "";
    int32_t totalNumBytes = 0;
    for (const auto &item : substrVec) {
        int32_t cnt = totalNumBytes + static_cast<int32_t>(item.size());
        if (cnt > maxNumBytes) {
            break;
        }
        totalNumBytes = cnt;
        result = result + item;
    }
    return result;
}

std::string DeviceNameManager::GetSystemLanguage()
{
    char param[SYSPARA_SIZE] = {0};
    int status = GetParameter(PERSIST_GLOBAL_LANGUAGE, "", param, SYSPARA_SIZE);
    if (status > 0) {
        return param;
    }
    status = GetParameter(DEFAULT_LANGUAGE_KEY, "", param, SYSPARA_SIZE);
    if (status > 0) {
        return param;
    }
    LOGE("Failed to get system language");
    return "";
}

std::string DeviceNameManager::GetSystemRegion()
{
    char param[SYSPARA_SIZE] = {0};
    int status = GetParameter(PERSIST_GLOBAL_LOCALE, "", param, SYSPARA_SIZE);
    if (status > 0) {
        return param;
    }
    status = GetParameter(DEFAULT_LOCALE_KEY, "", param, SYSPARA_SIZE);
    if (status > 0) {
        return param;
    }
    LOGE("Failed to get system local");
    return "";
}

std::string DeviceNameManager::GetLocalMarketName()
{
    std::lock_guard<std::mutex> lock(localMarketNameMtx_);
    if (localMarketName_.empty()) {
        const char *marketName = GetMarketName();
        if (marketName == nullptr) {
            LOGE("get marketName fail!");
            return "";
        }
        localMarketName_ = marketName;
        free((char *)marketName);
    }
    std::vector<std::string> prefixs = DeviceManagerService::GetInstance().GetDeviceNamePrefixs();
    for (const auto &item : prefixs) {
        localMarketName_ = TrimStr(ReplaceStr(localMarketName_, item, ""));
    }
    LOGI("localMarketName : %{public}s", GetAnonyString(localMarketName_).c_str());
    return localMarketName_;
}

int32_t DeviceNameManager::SetUserDefinedDeviceName(const std::string &deviceName, int32_t userId)
{
    LOGI("SetUserDefinedDeviceName:%{public}s, userId:%{public}d", GetAnonyString(deviceName).c_str(), userId);
    return SetValue(SETTINGSDATA_SECURE, userId, SETTINGS_GENERAL_USER_DEFINED_DEVICE_NAME, deviceName);
}

int32_t DeviceNameManager::GetDisplayDeviceName(int32_t userId, std::string &deviceName)
{
    return GetValue(SETTINGSDATA_SECURE, userId, SETTINGS_GENERAL_DISPLAY_DEVICE_NAME, deviceName);
}

int32_t DeviceNameManager::SetDisplayDeviceNameState(const std::string &state, int32_t userId)
{
    LOGI("SetDisplayDeviceNameState:%{public}s, userId:%{public}d", state.c_str(), userId);
    return SetValue(SETTINGSDATA_SECURE, userId, SETTINGS_GENERAL_DISPLAY_DEVICE_NAME_STATE, state);
}

int32_t DeviceNameManager::SetDisplayDeviceName(const std::string &deviceName, int32_t userId)
{
    if (deviceName.empty()) {
        LOGE("SetDisplayDeviceName deviceName is empty, userId:%{public}d", userId);
        return ERR_DM_NAME_EMPTY;
    }
    LOGI("SetDisplayDeviceName:%{public}s, userId:%{public}d",  GetAnonyString(deviceName).c_str(), userId);
    return SetValue(SETTINGSDATA_SECURE, userId, SETTINGS_GENERAL_DISPLAY_DEVICE_NAME, deviceName);
}

int32_t DeviceNameManager::GetDeviceName(std::string &deviceName)
{
    return GetValue(SETTINGSDATA_GLOBAL, 0, SETTINGS_GENERAL_DEVICE_NAME, deviceName);
}

int32_t DeviceNameManager::SetDeviceName(const std::string &deviceName)
{
    if (deviceName.empty()) {
        LOGE("deviceName is empty");
        return ERR_DM_NAME_EMPTY;
    }
    return SetValue(SETTINGSDATA_GLOBAL, 0, SETTINGS_GENERAL_DEVICE_NAME, deviceName);
}

sptr<IRemoteObject> DeviceNameManager::GetRemoteObj()
{
    std::lock_guard<std::mutex> lock(remoteObjMtx_);
    if (remoteObj_ != nullptr) {
        return remoteObj_;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("get sa manager return nullptr");
        return nullptr;
    }
    auto remoteObj = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (remoteObj == nullptr) {
        LOGE("get system ability failed, id=%{public}d", DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
        return nullptr;
    }
    remoteObj_ = remoteObj;
    return remoteObj_;
}

int32_t DeviceNameManager::GetValue(const std::string &tableName, int32_t userId,
    const std::string &key, std::string &value)
{
    std::string proxyUri = GetProxyUriStr(tableName, userId);
    auto helper = CreateDataShareHelper(proxyUri);
    if (helper == nullptr) {
        LOGE("helper is nullptr, proxyUri=%{public}s", proxyUri.c_str());
        return ERR_DM_POINT_NULL;
    }
    std::vector<std::string> columns = { SETTING_COLUMN_VALUE };
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri = MakeUri(proxyUri, key);
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        LOGE("Query failed key=%{public}s, proxyUri=%{public}s", key.c_str(), proxyUri.c_str());
        return ERR_DM_POINT_NULL;
    }
    int32_t count = 0;
    resultSet->GetRowCount(count);
    if (count == 0) {
        LOGW("no value, key=%{public}s, proxyUri=%{public}s", key.c_str(), proxyUri.c_str());
        resultSet->Close();
        return DM_OK;
    }
    int32_t index = 0;
    resultSet->GoToRow(index);
    int32_t ret = resultSet->GetString(index, value);
    if (ret != DataShare::E_OK) {
        LOGE("get value failed, ret=%{public}d, proxyUri=%{public}s", ret, proxyUri.c_str());
        resultSet->Close();
        return ret;
    }
    resultSet->Close();
    LOGI("proxyUri=%{public}s, value=%{public}s", proxyUri.c_str(), GetAnonyString(value).c_str());
    return DM_OK;
}

int32_t DeviceNameManager::SetValue(const std::string &tableName, int32_t userId,
    const std::string &key, const std::string &value)
{
    std::string proxyUri = GetProxyUriStr(tableName, userId);
    auto helper = CreateDataShareHelper(proxyUri);
    if (helper == nullptr) {
        LOGE("helper is nullptr, proxyUri=%{public}s, value=%{public}s",
            proxyUri.c_str(), GetAnonyString(value).c_str());
        return ERR_DM_POINT_NULL;
    }
    DataShare::DataShareValuesBucket val;
    val.Put(SETTING_COLUMN_KEYWORD, key);
    val.Put(SETTING_COLUMN_VALUE, value);
    Uri uri = MakeUri(proxyUri, key);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    int32_t ret = helper->Update(uri, predicates, val);
    if (ret <= 0) {
        LOGW("Update failed, ret=%{public}d, proxyUri=%{public}s, value=%{public}s",
            ret, proxyUri.c_str(), GetAnonyString(value).c_str());
        ret = helper->Insert(uri, val);
    }
    ReleaseDataShareHelper(helper);
    if (ret <= 0) {
        LOGE("set value failed, ret=%{public}d, proxyUri=%{public}s, value=%{public}s",
            ret, proxyUri.c_str(), GetAnonyString(value).c_str());
        return ret;
    }
    return ret;
}

std::shared_ptr<DataShare::DataShareHelper> DeviceNameManager::CreateDataShareHelper(const std::string &proxyUri)
{
    if (proxyUri.empty()) {
        LOGE("proxyUri is empty");
        return nullptr;
    }
    auto [ret, helper] = DataShare::DataShareHelper::Create(GetRemoteObj(), proxyUri, SETTINGS_DATA_EXT_URI);
    if (ret != 0) {
        LOGE("create helper failed ret %{public}d", ret);
        return nullptr;
    }
    return helper;
}

std::string DeviceNameManager::GetProxyUriStr(const std::string &tableName, int32_t userId)
{
    if (userId < USERID_HELPER_NUMBER) {
        userId = USERID_HELPER_NUMBER;
    }
    if (tableName == SETTINGSDATA_GLOBAL) {
        return SETTING_URI_PROXY + SETTINGSDATA_GLOBAL + URI_PROXY_SUFFIX;
    } else {
        return SETTING_URI_PROXY + tableName + std::to_string(userId) + URI_PROXY_SUFFIX;
    }
}

Uri DeviceNameManager::MakeUri(const std::string &proxyUri, const std::string &key)
{
    if (proxyUri.empty() || key.empty()) {
        LOGE("Invalid parameter.");
    }
    Uri uri(proxyUri + "&key=" + key);
    return uri;
}

bool DeviceNameManager::ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    if (helper == nullptr) {
        LOGE("helper is nullptr");
        return false;
    }
    if (!helper->Release()) {
        LOGE("release helper fail");
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS