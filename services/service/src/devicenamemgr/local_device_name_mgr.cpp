/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "local_device_name_mgr.h"

#include "datashare_result_set.h"
#include "ohos_account_kits.h"
#include "os_account_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "uri.h"

#include "device_manager_service.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "settings_data_event_monitor.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const std::string SETTINGS_DATA_BASE_URI =
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
    const std::string SETTINGS_DATA_SECURE_URI =
        "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_SECURE_";
    constexpr const char *SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
    constexpr const char *SETTINGS_DATA_FIELD_KEYWORD = "KEYWORD";
    constexpr const char *SETTINGS_DATA_FIELD_VALUE = "VALUE";
    constexpr const char *PREDICATES_STRING = "settings.general.device_name";
    constexpr const char *USER_DEFINED_STRING = "settings.general.user_defined_device_name";
    constexpr const char *DISPLAY_DEVICE_NAME_STRING = "settings.general.display_device_name";
}

std::mutex LocalDeviceNameMgr::devNameMtx_;

LocalDeviceNameMgr::LocalDeviceNameMgr() : localDeviceName_(""), localDisplayName_("")
{
    LOGI("Ctor LocalDeviceNameMgr");
}

LocalDeviceNameMgr::~LocalDeviceNameMgr()
{
    std::lock_guard<std::mutex> lock(devNameMtx_);
    localDeviceName_ = "";
    localDisplayName_ = "";
    LOGI("Dtor LocalDeviceNameMgr");
}

std::shared_ptr<DataShare::DataShareHelper> LocalDeviceNameMgr::GetDataShareHelper()
{
    sptr<ISystemAbilityManager> saManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saManager == nullptr) {
        LOGE("saManager NULL");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = saManager->GetSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (remoteObject == nullptr) {
        LOGE("remoteObject NULL");
        return nullptr;
    }
    return DataShare::DataShareHelper::Creator(remoteObject, SETTINGS_DATA_BASE_URI, SETTINGS_DATA_EXT_URI);
}

int32_t LocalDeviceNameMgr::GetDeviceNameFromDataShareHelper(
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper, std::shared_ptr<Uri> uri,
    const char *key, std::string &deviceName)
{
    if (dataShareHelper == nullptr || uri == nullptr || key == nullptr) {
        LOGE("dataShareHelper or uri or key is null, param is error!");
        return ERR_DM_FAILED;
    }
    int32_t numRows = 0;
    std::string val;
    std::vector<std::string> columns;
    columns.emplace_back(SETTINGS_DATA_FIELD_VALUE);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTINGS_DATA_FIELD_KEYWORD, key);

    auto resultSet = dataShareHelper->Query(*uri, predicates, columns);
    if (resultSet == nullptr) {
        LOGE("query fail.");
        return ERR_DM_FAILED;
    }
    resultSet->GetRowCount(numRows);
    if (numRows <= 0) {
        LOGE("row zero.");
        resultSet->Close();
        return ERR_DM_FAILED;
    }

    int columnIndex;
    resultSet->GoToFirstRow();
    resultSet->GetColumnIndex(SETTINGS_DATA_FIELD_VALUE, columnIndex);
    if (resultSet->GetString(columnIndex, val) != DM_OK) {
        LOGE("GetString val fail");
        resultSet->Close();
        return ERR_DM_FAILED;
    }
    deviceName = val;
    LOGI("deviceName=%{public}s.", deviceName.c_str());
    resultSet->Close();
    return DM_OK;
}

int32_t LocalDeviceNameMgr::GetDefaultDeviceName(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    std::string &deviceName)
{
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper is null, param is error!");
        return ERR_DM_FAILED;
    }
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(SETTINGS_DATA_BASE_URI + "&key=" + PREDICATES_STRING);
    LOGI("get default deviceName");
    return GetDeviceNameFromDataShareHelper(dataShareHelper, uri, PREDICATES_STRING, deviceName);
}

int32_t LocalDeviceNameMgr::GetUserDefinedDeviceName(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    std::string &deviceName)
{
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper is null, param is error!");
        return ERR_DM_FAILED;
    }
    int32_t osAccountId = GetActiveOsAccountIds();
    if (osAccountId == ERR_DM_FAILED) {
        LOGE("osAccountId acquire fail!");
        return ERR_DM_FAILED;
    }
    std::string accountIdStr = std::to_string(osAccountId);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(SETTINGS_DATA_SECURE_URI + accountIdStr + "?Proxy=true&key=" +
        USER_DEFINED_STRING);
    LOGI("get user defined deviceName, accountId=%{public}s", accountIdStr.c_str());
    return GetDeviceNameFromDataShareHelper(dataShareHelper, uri, USER_DEFINED_STRING, deviceName);
}

int32_t LocalDeviceNameMgr::GetDisplayDeviceName(std::shared_ptr<DataShare::DataShareHelper> dataShareHelper,
    std::string &deviceName)
{
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper is null, param is error!");
        return ERR_DM_FAILED;
    }
    int32_t osAccountId = GetActiveOsAccountIds();
    if (osAccountId == ERR_DM_FAILED) {
        LOGE("osAccountId acquire fail!");
        return ERR_DM_FAILED;
    }
    std::string accountIdStr = std::to_string(osAccountId);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>(SETTINGS_DATA_SECURE_URI + accountIdStr + "?Proxy=true&key=" +
        DISPLAY_DEVICE_NAME_STRING);
    LOGI("get user defined deviceName, accountId=%{public}s", accountIdStr.c_str());
    return GetDeviceNameFromDataShareHelper(dataShareHelper, uri, DISPLAY_DEVICE_NAME_STRING, deviceName);
}

int32_t LocalDeviceNameMgr::GetActiveOsAccountIds()
{
    std::vector<int32_t> accountId;
    int32_t ret = OHOS::AccountSA::OsAccountManager::QueryActiveOsAccountIds(accountId);
    if (ret != DM_OK || accountId.empty()) {
        LOGE("QueryActiveOsAccountIds failed");
        return ERR_DM_FAILED;
    }
    LOGI("account id=%{public}d", accountId[0]);
    return accountId[0];
}

int32_t LocalDeviceNameMgr::QueryLocalDeviceName()
{
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper is null");
        return ERR_DM_FAILED;
    }
    std::string localDeviceName = "";
    int32_t ret = GetUserDefinedDeviceName(dataShareHelper, localDeviceName);
    if (ret == DM_OK && !localDeviceName.empty()) {
        std::lock_guard<std::mutex> lock(devNameMtx_);
        localDeviceName_ = localDeviceName;
        dataShareHelper->Release();
        LOGI("get user defined deviceName=%{public}s", localDeviceName.c_str());

        return DM_OK;
    }
    ret = GetDefaultDeviceName(dataShareHelper, localDeviceName);
    if (ret != DM_OK || localDeviceName.empty()) {
        LOGE("get default deviceName failed");
        return ERR_DM_FAILED;
    }
    std::lock_guard<std::mutex> lock(devNameMtx_);
    localDeviceName_ = localDeviceName;
    dataShareHelper->Release();
    LOGI("get default deviceName=%{public}s", localDeviceName.c_str());

    return DM_OK;
}

void LocalDeviceNameMgr::RegisterDeviceNameChangeCb()
{
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper is null");
        return;
    }

    auto uri = std::make_shared<Uri>(SETTINGS_DATA_BASE_URI + "&key=" + PREDICATES_STRING);
    sptr<SettingsDataEventMonitor> settingDataObserver =
        sptr<SettingsDataEventMonitor>(new SettingsDataEventMonitor(shared_from_this(),
            SettingsDataMonitorType::USER_DEFINED_DEVICE_NAME_MONITOR));
    dataShareHelper->RegisterObserver(*uri, settingDataObserver);

    int32_t osAccountId = GetActiveOsAccountIds();
    if (osAccountId == ERR_DM_FAILED) {
        LOGE("Get OsAccountId error");
        return;
    }
    std::string accountIdStr = std::to_string(osAccountId);
    uri = std::make_shared<Uri>(SETTINGS_DATA_SECURE_URI + accountIdStr +
        "?Proxy=true&key=" + USER_DEFINED_STRING);

    dataShareHelper->RegisterObserver(*uri, settingDataObserver);
    dataShareHelper->Release();
    LOGI("register device name change cb success");
}

int32_t LocalDeviceNameMgr::QueryLocalDisplayName()
{
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper is null");
        return ERR_DM_FAILED;
    }
    std::string localDisplayName = "";
    int32_t ret = GetDisplayDeviceName(dataShareHelper, localDisplayName);
    if (ret != DM_OK || localDisplayName.empty()) {
        LOGE("get display device name failed");
        return ERR_DM_FAILED;
    }
    std::lock_guard<std::mutex> lock(devNameMtx_);
    localDisplayName_ = localDisplayName;
    dataShareHelper->Release();
    LOGI("get display deviceName=%{public}s", localDisplayName.c_str());
    return DM_OK;
}

void LocalDeviceNameMgr::RegisterDisplayNameChangeCb()
{
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper == nullptr) {
        LOGE("dataShareHelper is null");
        return;
    }

    int32_t osAccountId = GetActiveOsAccountIds();
    if (osAccountId == ERR_DM_FAILED) {
        LOGE("Get OsAccountId error");
        return;
    }
    std::string accountIdStr = std::to_string(osAccountId);
    auto uri = std::make_shared<Uri>(SETTINGS_DATA_SECURE_URI + accountIdStr +
        "?Proxy=true&key=" + DISPLAY_DEVICE_NAME_STRING);
    sptr<SettingsDataEventMonitor> settingDataObserver =
        sptr<SettingsDataEventMonitor>(new SettingsDataEventMonitor(shared_from_this(),
            SettingsDataMonitorType::DISPLAY_DEVICE_NAME_MONITOR));
    dataShareHelper->RegisterObserver(*uri, settingDataObserver);
    dataShareHelper->Release();
    LOGI("register display device name change cb success");
}

std::string LocalDeviceNameMgr::GetLocalDisplayName() const
{
    std::lock_guard<std::mutex> lock(devNameMtx_);
    return localDisplayName_;
}

std::string LocalDeviceNameMgr::GetLocalDeviceName() const
{
    std::lock_guard<std::mutex> lock(devNameMtx_);
    return localDeviceName_;
}
} // namespace DistributedHardware
} // namespace OHOS