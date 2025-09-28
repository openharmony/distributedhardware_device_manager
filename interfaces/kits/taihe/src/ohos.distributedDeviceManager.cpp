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

#include "ohos.distributedDeviceManager.h"
#include "ani_error_utils.h"
#include <iostream>
#include <string>
#include "device_manager.h"
#include "dm_constants.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "ani_utils.h"
#include "ani_dm_utils.h"
#include "dm_anonymous.h"

const int32_t DM_NAPI_DISCOVER_EXTRA_INIT_ONE = -1;
const int32_t DM_NAPI_DISCOVER_EXTRA_INIT_TWO = -2;
const std::string DM_NAPI_EVENT_DEVICE_DISCOVER_SUCCESS = "discoverSuccess";
const std::string DM_NAPI_EVENT_DEVICE_DISCOVER_FAIL = "discoverFail";

using namespace OHOS::DistributedHardware;
using namespace ani_utils;
using namespace ani_errorutils;

namespace ANI::distributedDeviceManager {
const int32_t DM_NAPI_BUF_LENGTH = 256;

constexpr int32_t DEVICE_TYPE_UNKNOWN = -1;
constexpr const char *DEVICE_TYPE_EMPTY_STR = "";
constexpr const char *ERROR_DEVICE_ID = "error deviceId";
constexpr const char *ERROR_NETWORK_ID = "error networkId";

std::mutex g_deviceManagerMapMutex;
std::mutex g_initCallbackMapMutex;
std::mutex g_deviceNameChangeCallbackMapMutex;
std::mutex g_discoveryCallbackMapMutex;
std::mutex g_deviceStateChangeDataCallbackMapMutex;
std::mutex g_dmUiCallbackMapMutex;
std::mutex g_bindCallbackMapMutex;
std::mutex g_authCallbackMapMutex;

std::map<std::string, DeviceManagerImpl*> g_deviceManagerMap;
std::map<std::string, std::shared_ptr<DmAniInitCallback>> g_initCallbackMap;
std::map<std::string, std::shared_ptr<DmAniDeviceNameChangeCallback>> g_deviceNameChangeCallbackMap;
std::map<std::string, std::shared_ptr<DmAniDeviceStateChangeResultCallback>> g_deviceStateChangeDataCallbackMap;
std::map<std::string, std::shared_ptr<DmAniDiscoveryCallback>> g_discoveryCallbackMap;
std::map<std::string, std::shared_ptr<DmAniDeviceManagerUiCallback>> g_dmUiCallbackMap;
std::map<std::string, std::shared_ptr<DmAniBindTargetCallback>> g_bindCallbackMap;
std::map<std::string, std::shared_ptr<DmAniAuthenticateCallback>> g_authCallbackMap;

DeviceManagerImpl::DeviceManagerImpl(const std::string& bundleName)
    : bundleName_(bundleName)
{
}

DeviceManagerImpl::DeviceManagerImpl(std::shared_ptr<DeviceManagerImpl> impl)
{
    std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
    if (impl == nullptr) {
        LOGE("Create DeviceManagerImpl for shared_ptr failed.");
    } else {
        bundleName_ = impl->bundleName_;
    }
}

DeviceManagerImpl::~DeviceManagerImpl()
{
    ClearBundleCallbacks(bundleName_);
}

int64_t DeviceManagerImpl::GetInner()
{
    return reinterpret_cast<int64_t>(this);
}

ohos::distributedDeviceManager::DeviceManager CreateDeviceManager(taihe::string_view bundleName)
{
    std::string stdBundleName(bundleName);
    std::shared_ptr<DmAniInitCallback> initCallback = std::make_shared<DmAniInitCallback>(bundleName);
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(
        std::string(bundleName), initCallback);
    if (ret != 0) {
        LOGE("CreateDeviceManager for bundleName %{public}s failed, ret %{public}d.", bundleName.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
        return taihe::make_holder<DeviceManagerImpl, ohos::distributedDeviceManager::DeviceManager>();
    }
    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        g_initCallbackMap[stdBundleName] = initCallback;
    }
    return taihe::make_holder<DeviceManagerImpl, ohos::distributedDeviceManager::DeviceManager>(stdBundleName);
}

void ReleaseDeviceManager(::ohos::distributedDeviceManager::weak::DeviceManager deviceManager)
{
    LOGI("ReleaseDeviceManager");
    DeviceManagerImpl* impl = reinterpret_cast<DeviceManagerImpl*>(deviceManager->GetInner());
    if (impl) {
        impl->ReleaseDeviceManager();
    }
}

::ohos::distributedDeviceManager::DeviceBasicInfo MakeDeviceBasicInfo(::taihe::string_view deviceId,
    ::taihe::string_view deviceName, ::taihe::string_view deviceType,
    ::taihe::optional_view<::taihe::string> networkId)
{
    return {deviceId, deviceName, deviceType, networkId};
}

ohos::distributedDeviceManager::DeviceNameChangeResult MakeDeviceNameChangeResult(taihe::string_view deviceName)
{
    return {deviceName};
}

ohos::distributedDeviceManager::ReplyResult MakeReplyResult(taihe::string_view param)
{
    return {param};
}

ohos::distributedDeviceManager::DiscoveryFailureResult MakeDiscoveryFailureResult(int32_t reason)
{
    return {reason};
}

ohos::distributedDeviceManager::DiscoverySuccessResult MakeDiscoverySuccessResult(
    ohos::distributedDeviceManager::DeviceBasicInfo const& device)
{
    return {device};
}

ohos::distributedDeviceManager::DeviceStateChangeResult MakeDeviceStateChangeResult(
    ohos::distributedDeviceManager::DeviceStateChange deviceStateChange,
    ohos::distributedDeviceManager::DeviceBasicInfo const& deviceBasicInfo)
{
    return {deviceStateChange, deviceBasicInfo};
}

std::string DeviceManagerImpl::GetLocalDeviceId()
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "GetLocalDeviceId for failed1");
        return DEVICE_TYPE_EMPTY_STR;
    }
    std::string deviceId;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceId(bundleName_, deviceId);
    if (ret != 0) {
        taihe::set_business_error(DM_ERR_FAILED, "GetLocalDeviceId for failed2");
        return ERROR_DEVICE_ID;
    }
    return std::string(deviceId);
}

int32_t DeviceManagerImpl::GetDeviceType(taihe::string_view networkId)
{
    int32_t deviceType;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceType(
        bundleName_, std::string(networkId), deviceType);
    if (ret != 0) {
        taihe::set_business_error(DM_ERR_FAILED, "GetDeviceType for failed");
        return DEVICE_TYPE_UNKNOWN;
    }
    return deviceType;
}

std::string DeviceManagerImpl::GetDeviceName(taihe::string_view networkId)
{
    std::string deviceName;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceName(
        bundleName_, std::string(networkId), deviceName);
    if (ret != 0) {
        taihe::set_business_error(DM_ERR_FAILED, "GetDeviceName for failed");
        return DEVICE_TYPE_EMPTY_STR;
    }
    return std::string(deviceName);
}

std::string DeviceManagerImpl::GetLocalDeviceNetworkId()
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION, "GetLocalDeviceNetworkId failed");
        return DEVICE_TYPE_EMPTY_STR;
    }
    std::string networkId;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceNetWorkId(
        bundleName_, networkId);
    if (ret != 0) {
        LOGE("GetLocalDeviceNetworkId for failed, ret %{public}d", ret);
        taihe::set_business_error(DM_ERR_FAILED, "GetLocalDeviceNetworkId failed");
        return ERROR_NETWORK_ID;
    }
    return std::string(networkId);
}

void DeviceManagerImpl::OnDeviceNameChange(taihe::callback_view<void(
    ohos::distributedDeviceManager::DeviceNameChangeResult const&)> onDeviceNameChangecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "ondeviceNameChange check permission failed.");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_deviceNameChangeCallbackMapMutex);
        if (g_deviceNameChangeCallbackMap.find(bundleName_) != g_deviceNameChangeCallbackMap.end()) {
            LOGI("bundleName already register.");
            return;
        }
    }

    auto dmonDeviceNameChangecb = std::make_shared<DmAniDeviceNameChangeCallback>(bundleName_, onDeviceNameChangecb);
    std::string extra = "";
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName_, extra,
        dmonDeviceNameChangecb) != 0) {
        LOGE("RegisterDevStatusCallback failed");
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceNameChangeCallbackMapMutex);
        g_deviceNameChangeCallbackMap[bundleName_] = dmonDeviceNameChangecb;
    }
    return;
}

void DeviceManagerImpl::OnReplyResult(taihe::callback_view<void(
    ohos::distributedDeviceManager::ReplyResult const&)> onReplyResultcb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "OnReplyResult check permission failed");
        return;
    }

    auto dmcallback = std::make_shared<DmAniDeviceManagerUiCallback>(onReplyResultcb, bundleName_);
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().RegisterDeviceManagerFaCallback(
        bundleName_, dmcallback);
    if (ret != 0) {
        LOGE("RegisterDeviceManagerFaCallback failed for bundleName %{public}s", bundleName_.c_str());
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        g_dmUiCallbackMap[bundleName_] = dmcallback;
    }
    return;
}

void DeviceManagerImpl::OnDeviceStateChange(taihe::callback_view<void(
    ohos::distributedDeviceManager::DeviceStateChangeResult const&)> onDeviceStateChangecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "ondeviceStateChange check Permission failed");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateChangeDataCallbackMapMutex);
        if (g_deviceStateChangeDataCallbackMap.find(bundleName_) != g_deviceStateChangeDataCallbackMap.end()) {
            LOGI("bundleName already register.");
            return;
        }
    }

    auto dmCallback = std::make_shared<DmAniDeviceStateChangeResultCallback>(bundleName_, onDeviceStateChangecb);
    std::string extra = "";
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().RegisterDevStatusCallback(bundleName_,
        extra, dmCallback);
    if (ret != 0) {
        LOGE("RegisterDevStatusCallback failed ret %{public}d", ret);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateChangeDataCallbackMapMutex);
        g_deviceStateChangeDataCallbackMap[bundleName_] = dmCallback;
    }
    return;
}

void DeviceManagerImpl::OnServiceDie(taihe::callback_view<void()> onServiceDiecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "OnReplyResult check permission failed");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        auto dmCallback = g_initCallbackMap[bundleName_];
        dmCallback->SetServiceDieCallback(std::make_shared<taihe::callback<void()>>(onServiceDiecb));
    }
    return;
}

void DeviceManagerImpl::OffDeviceNameChange(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceNameChangeResult const&)>> offDeviceNameChangecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "offdeviceNameChange check permission failed.");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_deviceNameChangeCallbackMapMutex);
        auto iter = g_deviceNameChangeCallbackMap.find(bundleName_);
        if (iter == g_deviceNameChangeCallbackMap.end()) {
            LOGE("ReleaseDmCallback: cannot find statusCallback for bundleName "
                "%{public}s",
                bundleName_.c_str());
            return;
        }
    }

    if (OHOS::DistributedHardware::DeviceManager::GetInstance().UnRegisterDevStatusCallback(bundleName_) != 0) {
        LOGE("UnRegisterDevStatusCallback failed for bundleName %{public}s", bundleName_.c_str());
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceNameChangeCallbackMapMutex);
        g_deviceNameChangeCallbackMap.erase(bundleName_);
    }
    return;
}

void DeviceManagerImpl::OffReplyResult(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::ReplyResult const&)>> offReplyResultcb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "OffReplyResult check permission failed");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        auto iter = g_dmUiCallbackMap.find(bundleName_);
        if (iter == g_dmUiCallbackMap.end()) {
            LOGE("cannot find dmFaCallback for bundleName %{public}s", bundleName_.c_str());
            return;
        }
    }

    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnRegisterDeviceManagerFaCallback(
        bundleName_);
    if (ret != 0) {
        LOGE("UnRegisterDeviceManagerFaCallback failed for bundleName %{public}s", bundleName_.c_str());
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        g_dmUiCallbackMap.erase(bundleName_);
    }
}

void DeviceManagerImpl::OffDeviceStateChange(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceStateChangeResult const&)>> offDeviceStateChangecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "offdeviceStateChange check Permission failed");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateChangeDataCallbackMapMutex);
        auto iter = g_deviceStateChangeDataCallbackMap.find(bundleName_);
        if (iter == g_deviceStateChangeDataCallbackMap.end()) {
            LOGE("Cannot find stateChangeDataCallback for bundleName %{public}s", bundleName_.c_str());
            return;
        }
    }

    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnRegisterDevStatusCallback(bundleName_);
    if (ret != 0) {
        LOGE("UnRegisterDevStatusCallback failed for bundleName %{public}s", bundleName_.c_str());
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateChangeDataCallbackMapMutex);
        g_deviceStateChangeDataCallbackMap.erase(bundleName_);
    }
    return;
}

void DeviceManagerImpl::OffServiceDie(taihe::optional_view<taihe::callback<void()>> offServiceDiecb)
{
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        taihe::set_business_error(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION,
            "offdiscoverFailure check permission failed.");
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        auto iter = g_initCallbackMap.find(bundleName_);
        if (iter == g_initCallbackMap.end() || !iter->second) {
            LOGE("Cannot find ServiceDieCallback for bundleName %{public}s", bundleName_.c_str());
            return;
        }
        iter->second->ReleaseServiceDieCallback();
    }
}

std::shared_ptr<DmAniBindTargetCallback> DeviceManagerImpl::GetBindTargetCallback(std::string bundleName)
{
    ani_env *env = taihe::get_env();
    std::lock_guard<std::mutex> lock(g_bindCallbackMapMutex);
    std::shared_ptr<DmAniBindTargetCallback> bindTargetCallback;
    auto iter = g_bindCallbackMap.find(bundleName);
    if (iter == g_bindCallbackMap.end()) {
        bindTargetCallback = std::make_shared<DmAniBindTargetCallback>(env, bundleName);
        g_bindCallbackMap[bundleName] = bindTargetCallback;
    } else {
        bindTargetCallback = iter->second;
    }
    return bindTargetCallback;
}

std::shared_ptr<DmAniAuthenticateCallback> DeviceManagerImpl::GetAuthenticateTargetCallback(std::string bundleName)
{
    ani_env *env = taihe::get_env();
    std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
    std::shared_ptr<DmAniAuthenticateCallback> bindDeviceCallback;
    auto iter = g_authCallbackMap.find(bundleName);
    if (iter == g_authCallbackMap.end()) {
        bindDeviceCallback = std::make_shared<DmAniAuthenticateCallback>(env, bundleName);
        g_authCallbackMap[bundleName] = bindDeviceCallback;
    } else {
        bindDeviceCallback = iter->second;
    }
    return bindDeviceCallback;
}

void DeviceManagerImpl::ReleaseDeviceManager()
{
    int32_t ret = DeviceManager::GetInstance().UnInitDeviceManager(bundleName_);
    if (ret != 0) {
        taihe::set_business_error(ret, "ReleaseDeviceManager for bundleName failed");
        return;
    }
    ClearBundleCallbacks(bundleName_);
}

::taihe::array<::ohos::distributedDeviceManager::DeviceBasicInfo> DeviceManagerImpl::GetAvailableDeviceListSync()
{
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
        ::taihe::array<::ohos::distributedDeviceManager::DeviceBasicInfo> empty = {};
        return empty;
    }
    std::vector<DmDeviceBasicInfo> devList;
    ret = DeviceManager::GetInstance().GetAvailableDeviceList(bundleName_, devList);
    if (ret != 0) {
        LOGE("GetTrustedDeviceList for bundleName %{public}s failed, ret %{public}d",
            bundleName_.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
        ::taihe::array<::ohos::distributedDeviceManager::DeviceBasicInfo> empty = {};
        return empty;
    }
    LOGD("DeviceManager::GetAvailableDeviceListSync");
    std::vector<::ohos::distributedDeviceManager::DeviceBasicInfo> deviceBasicInfos;
    if (devList.size() > 0) {
        for (size_t i = 0; i != devList.size(); ++i) {
            ::ohos::distributedDeviceManager::DeviceBasicInfo deviceBasicInfo = {};
            deviceBasicInfo.deviceId = ::taihe::string(devList[i].deviceId);
            deviceBasicInfo.deviceName = ::taihe::string(devList[i].deviceName);
            deviceBasicInfo.deviceType = ::taihe::string(std::to_string(devList[i].deviceTypeId));
            deviceBasicInfo.networkId = ::taihe::optional<::taihe::string>::make(::taihe::string(devList[i].networkId));
            deviceBasicInfos.emplace_back(deviceBasicInfo);
        }
    }
    return ::taihe::array<::ohos::distributedDeviceManager::DeviceBasicInfo>(taihe::copy_data_t{},
        deviceBasicInfos.data(), deviceBasicInfos.size());
}

void DeviceManagerImpl::OnDiscoverFailure(taihe::callback_view<void(
    ohos::distributedDeviceManager::DiscoveryFailureResult const&)> onDiscoverFailurecb)
{
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
        return;
    }
    std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
    std::shared_ptr<DmAniDiscoveryCallback> dmCallback;
    if (g_discoveryCallbackMap.find(bundleName_) != g_discoveryCallbackMap.end()) {
        dmCallback = g_discoveryCallbackMap[bundleName_];
    } else {
        dmCallback = std::make_shared<DmAniDiscoveryCallback>(bundleName_);
        g_discoveryCallbackMap[bundleName_] = dmCallback;
    }
    dmCallback->SetFailedCallback(onDiscoverFailurecb);
}

void DeviceManagerImpl::OnDiscoverSuccess(taihe::callback_view<void(
    ohos::distributedDeviceManager::DiscoverySuccessResult const&)> onDiscoverSuccesscb)
{
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
        return;
    }
    std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
    std::shared_ptr<DmAniDiscoveryCallback> dmCallback;
    if (g_discoveryCallbackMap.find(bundleName_) != g_discoveryCallbackMap.end()) {
        dmCallback = g_discoveryCallbackMap[bundleName_];
    } else {
        dmCallback = std::make_shared<DmAniDiscoveryCallback>(bundleName_);
        g_discoveryCallbackMap[bundleName_] = dmCallback;
    }
    dmCallback->SetSuccessCallback(onDiscoverSuccesscb);
}

void DeviceManagerImpl::OffDiscoverFailure(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DiscoveryFailureResult const&)>> offDiscoverFailurecb)
{
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
        return;
    }
    std::shared_ptr<DmAniDiscoveryCallback> discoveryCallback = nullptr;
    std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
    auto iter = g_discoveryCallbackMap.find(bundleName_);
    if (iter == g_discoveryCallbackMap.end()) {
        return;
    }
    discoveryCallback = iter->second;

    discoveryCallback->ResetFailedCallback();
    if (discoveryCallback->GetRefCount() == 0) {
        g_discoveryCallbackMap.erase(bundleName_);
    }
}

void DeviceManagerImpl::OffDiscoverSuccess(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DiscoverySuccessResult const&)>> offDiscoverSuccesscb)
{
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
        return;
    }
    std::shared_ptr<DmAniDiscoveryCallback> disCoveryCallback = nullptr;
    std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
    auto iter = g_discoveryCallbackMap.find(bundleName_);
    if (iter == g_discoveryCallbackMap.end()) {
        LOGE("Cannot find discoveryCallback for bundleName %{public}s", bundleName_.c_str());
        return;
    }
    disCoveryCallback = iter->second;
    disCoveryCallback->ResetSuccessCallback();
    if (disCoveryCallback->GetRefCount() == 0) {
        g_discoveryCallbackMap.erase(bundleName_);
    }
}

int32_t DeviceManagerImpl::BindTargetWarpper(const std::string &pkgName, const std::string &deviceId,
    const std::string &bindParam, std::shared_ptr<DmAniBindTargetCallback> callback)
{
    if (bindParam.empty()) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    JsonObject bindParamObj(bindParam);
    if (bindParamObj.IsDiscarded()) {
        return ERR_DM_INPUT_PARA_INVALID;
    }
    PeerTargetId targetId;
    targetId.deviceId = deviceId;
    if (IsString(bindParamObj, PARAM_KEY_BR_MAC)) {
        targetId.brMac = bindParamObj[PARAM_KEY_BR_MAC].Get<std::string>();
    }
    if (IsString(bindParamObj, PARAM_KEY_BLE_MAC)) {
        targetId.bleMac = bindParamObj[PARAM_KEY_BLE_MAC].Get<std::string>();
    }
    if (IsString(bindParamObj, PARAM_KEY_WIFI_IP)) {
        targetId.wifiIp = bindParamObj[PARAM_KEY_WIFI_IP].Get<std::string>();
    }
    if (IsInt32(bindParamObj, PARAM_KEY_WIFI_PORT)) {
        targetId.wifiPort = (uint16_t)(bindParamObj[PARAM_KEY_WIFI_PORT].Get<int32_t>());
    }

    std::map<std::string, std::string> bindParamMap;
    ani_dmutils::InsertMapParames(bindParamObj, bindParamMap);
    return DeviceManager::GetInstance().BindTarget(pkgName, targetId, bindParamMap, callback);
}

void DeviceManagerImpl::BindTarget(::taihe::string_view deviceId,
    ::taihe::map_view<::taihe::string, uintptr_t> bindParam,
    ::taihe::callback_view<
        void(uintptr_t err, ::ohos::distributedDeviceManager::BindTargetResult const& data)> callback)
{
    std::string stdDeviceId(deviceId);
    if (stdDeviceId.size() == 0) {
        ani_errorutils::ThrowError(ERR_INVALID_PARAMS, "len == 0");
        return;
    }
    if (stdDeviceId.size() >= DM_NAPI_BUF_LENGTH) {
        ani_errorutils::ThrowError(ERR_INVALID_PARAMS, "len >= MAXLEN");
        return;
    }
    int32_t authType = -1;
    bool isMetaType = false;
    std::string strBindParam;
    ani_env *env = taihe::get_env();
    JsToBindParam(env, bindParam, strBindParam, authType, isMetaType);
    if (isMetaType) {
        std::shared_ptr<DmAniBindTargetCallback> bindTargetCallback = GetBindTargetCallback(bundleName_);
        bindTargetCallback->SetTaiheCallback(callback);
        int32_t ret = BindTargetWarpper(bundleName_, stdDeviceId, strBindParam, bindTargetCallback);
        if (ret != 0) {
            LOGE("BindTarget for bundleName %{public}s failed, ret %{public}d",
                bundleName_.c_str(), ret);
            ani_errorutils::CreateBusinessError(ret);
            return;
        }
    }
    std::shared_ptr<DmAniAuthenticateCallback> bindDeviceCallback = GetAuthenticateTargetCallback(bundleName_);
    bindDeviceCallback->SetTaiheCallback(callback);
    int32_t ret = DeviceManager::GetInstance().BindDevice(bundleName_, authType, stdDeviceId,
        strBindParam, bindDeviceCallback);
    if (ret != 0) {
        LOGE("BindDevice for bundleName %{public}s failed, ret %{public}d",
            bundleName_.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
    }
}

void DeviceManagerImpl::UnbindTarget(taihe::string_view deviceId)
{
    std::string stdDeviceId(deviceId);
    if (stdDeviceId.size() == 0) {
        ani_errorutils::ThrowError(ERR_INVALID_PARAMS, "len == 0");
        return;
    }
    if (stdDeviceId.size() >= DM_NAPI_BUF_LENGTH) {
        ani_errorutils::ThrowError(ERR_INVALID_PARAMS, "len >= MAXLEN");
        return;
    }
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().UnBindDevice(
        bundleName_, std::string(deviceId));
    if (ret != 0) {
        LOGE("UnBindDevice for bundleName %{public}s failed, ret %{public}d",
            bundleName_.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
    }
}

void DeviceManagerImpl::LockSuccDiscoveryCallbackMutex(std::string &bundleName,
    std::map<std::string, std::string> discParam, std::string &extra, uint32_t subscribeId)
{
    std::shared_ptr<DmAniDiscoveryCallback> discoveryCallback = nullptr;
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        auto iter = g_discoveryCallbackMap.find(bundleName);
        if (iter == g_discoveryCallbackMap.end()) {
            discoveryCallback = std::make_shared<DmAniDiscoveryCallback>(bundleName);
            g_discoveryCallbackMap[bundleName] = discoveryCallback;
        } else {
            discoveryCallback = iter->second;
        }
    }
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_SUBSCRIBE_ID, std::to_string(tokenId)));
    std::map<std::string, std::string> filterOps;
    filterOps.insert(std::pair<std::string, std::string>(PARAM_KEY_FILTER_OPTIONS, extra));
    int32_t ret = DeviceManager::GetInstance().StartDiscovering(bundleName, discParam, filterOps, discoveryCallback);
    if (ret != 0) {
        LOGE("Discovery failed, bundleName %{public}s, ret %{public}d", bundleName.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
        discoveryCallback->OnDiscoveryFailed(static_cast<uint16_t>(subscribeId), ret);
    }
}

void DeviceManagerImpl::StartDiscovering(::taihe::map_view<::taihe::string, uintptr_t> discoverParam,
    ::taihe::optional_view<::taihe::map<::taihe::string, uintptr_t>> filterOptions)
{
    ani_env *env = taihe::get_env();
    std::map<std::string, std::string> discParam;
    int32_t discoverTargetType = -1;
    if (!JsToDiscoverTargetType(env, discoverParam, discoverTargetType) || discoverTargetType != 1) {
        LOGE("discoverTargetType not 1");
        return;
    }
    JsToDiscoveryParam(env, discoverParam, discParam);
    std::string extra;
    if (filterOptions.has_value()) {
        JsToDmDiscoveryExtra(env, filterOptions.value(), extra);
    }
    uint32_t subscribeId = 0;
    LockSuccDiscoveryCallbackMutex(bundleName_, discParam, extra, subscribeId);
}

void DeviceManagerImpl::StopDiscovering()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(tokenId, bundleName_);
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
    }
}

void DeviceManagerImpl::JsToBindParam(ani_env* env, ::taihe::map_view<::taihe::string, uintptr_t> const& object,
    std::string &bindParam, int32_t &bindType, bool &isMetaType)
{
    int32_t bindTypeTemp = -1;
    ani_utils::AniGetMapItem(env, object, "bindType", bindTypeTemp);
    bindType = bindTypeTemp;

    std::string appOperation;
    ani_utils::AniGetMapItem(env, object, "appOperation", appOperation);
    std::string customDescription;
    ani_utils::AniGetMapItem(env, object, "customDescription", customDescription);
    std::string targetPkgName;
    ani_utils::AniGetMapItem(env, object, "targetPkgName", targetPkgName);
    std::string metaType;
    ani_utils::AniGetMapItem(env, object, "metaType", metaType);
    isMetaType = !metaType.empty();

    std::string pinCode;
    ani_utils::AniGetMapItem(env, object, "pinCode", pinCode);
    std::string authToken;
    ani_utils::AniGetMapItem(env, object, "authToken", authToken);
    std::string brMac;
    ani_utils::AniGetMapItem(env, object, "brMac", brMac);
    std::string isShowTrustDialog;
    ani_utils::AniGetMapItem(env, object, "isShowTrustDialog", isShowTrustDialog);
    std::string bleMac;
    ani_utils::AniGetMapItem(env, object, "bleMac", bleMac);
    std::string wifiIP;
    ani_utils::AniGetMapItem(env, object, "wifiIP", wifiIP);
    int32_t wifiPort = -1;
    ani_utils::AniGetMapItem(env, object, "wifiPort", wifiPort);
    int32_t bindLevel = 0;
    ani_utils::AniGetMapItem(env, object, "bindLevel", bindLevel);

    JsonObject jsonObj;
    jsonObj[AUTH_TYPE] = bindType;
    jsonObj[APP_OPERATION] = appOperation;
    jsonObj[CUSTOM_DESCRIPTION] = customDescription;
    jsonObj[PARAM_KEY_TARGET_PKG_NAME] = targetPkgName;
    jsonObj[PARAM_KEY_META_TYPE] = metaType;
    jsonObj[PARAM_KEY_PIN_CODE] = pinCode;
    jsonObj[PARAM_KEY_AUTH_TOKEN] = authToken;
    jsonObj[PARAM_KEY_IS_SHOW_TRUST_DIALOG] = isShowTrustDialog;
    jsonObj[PARAM_KEY_BR_MAC] = brMac;
    jsonObj[PARAM_KEY_BLE_MAC] = bleMac;
    jsonObj[PARAM_KEY_WIFI_IP] = wifiIP;
    jsonObj[PARAM_KEY_WIFI_PORT] = wifiPort;
    jsonObj[BIND_LEVEL] = bindLevel;
    bindParam = SafetyDump(jsonObj);
}

bool DeviceManagerImpl::JsToDiscoverTargetType(ani_env* env,
    ::taihe::map_view<::taihe::string, uintptr_t> const&object, int32_t &discoverTargetType)
{
    return ani_utils::AniGetMapItem(env, object, "discoverTargetType", discoverTargetType);
}

void DeviceManagerImpl::JsToDmDiscoveryExtra(ani_env* env,
    ::taihe::map_view<::taihe::string, uintptr_t> const&object, std::string &extra)
{
    JsonObject jsonObj;
    int32_t availableStatus = DM_NAPI_DISCOVER_EXTRA_INIT_ONE;
    ani_utils::AniGetMapItem(env, object, "availableStatus", availableStatus);
    if (availableStatus != DM_NAPI_DISCOVER_EXTRA_INIT_ONE) {
        jsonObj["credible"] = availableStatus;
    }

    int32_t discoverDistance = DM_NAPI_DISCOVER_EXTRA_INIT_ONE;
    ani_utils::AniGetMapItem(env, object, "discoverDistance", discoverDistance);
    if (discoverDistance != DM_NAPI_DISCOVER_EXTRA_INIT_ONE) {
        jsonObj["range"] = discoverDistance;
    }

    int32_t authenticationStatus = DM_NAPI_DISCOVER_EXTRA_INIT_ONE;
    ani_utils::AniGetMapItem(env, object, "authenticationStatus", authenticationStatus);
    if (authenticationStatus != DM_NAPI_DISCOVER_EXTRA_INIT_ONE) {
        jsonObj["isTrusted"] = authenticationStatus;
    }

    int32_t authorizationType = DM_NAPI_DISCOVER_EXTRA_INIT_TWO;
    ani_utils::AniGetMapItem(env, object, "authorizationType", authorizationType);
    if (authorizationType != DM_NAPI_DISCOVER_EXTRA_INIT_TWO) {
        jsonObj["authForm"] = authorizationType;
    }

    int32_t deviceType = DM_NAPI_DISCOVER_EXTRA_INIT_ONE;
    ani_utils::AniGetMapItem(env, object, "deviceType", deviceType);
    if (deviceType != DM_NAPI_DISCOVER_EXTRA_INIT_ONE) {
        jsonObj["deviceType"] = deviceType;
    }
    extra = SafetyDump(jsonObj);
    LOGI("JsToDmDiscoveryExtra, extra :%{public}s", extra.c_str());
}

void DeviceManagerImpl::JsToDiscoveryParam(ani_env* env, ::taihe::map_view<::taihe::string, uintptr_t> const&object,
    std::map<std::string, std::string> &discParam)
{
    std::string customData;
    ani_utils::AniGetMapItem(env, object, "CUSTOM_DATA", customData);
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_CUSTOM_DATA, customData));
    std::string capability;
    ani_utils::AniGetMapItem(env, object, "DISC_CAPABILITY", capability);
    discParam.insert(std::pair<std::string, std::string>(PARAM_KEY_DISC_CAPABILITY, capability));
}

bool DeviceManagerImpl::IsSystemApp()
{
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    return OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

void DeviceManagerImpl::ClearBundleCallbacks(std::string &bundleName)
{
    {
        std::lock_guard<std::mutex> autoLock(g_deviceManagerMapMutex);
        g_deviceManagerMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        g_initCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_discoveryCallbackMapMutex);
        auto iter = g_discoveryCallbackMap.find(bundleName);
        if (iter != g_discoveryCallbackMap.end()) {
            std::shared_ptr<DmAniDiscoveryCallback> callback = iter->second;
            if (callback != nullptr) {
                callback->ResetSuccessCallback();
                callback->ResetFailedCallback();
            }
        }
        g_discoveryCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_authCallbackMapMutex);
        auto iter = g_authCallbackMap.find(bundleName);
        if (iter != g_authCallbackMap.end()) {
            std::shared_ptr<DmAniAuthenticateCallback> callback = iter->second;
            if (callback != nullptr) {
                callback->Release();
            }
        }
        g_authCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_bindCallbackMapMutex);
        auto iter = g_bindCallbackMap.find(bundleName);
        if (iter != g_bindCallbackMap.end()) {
            std::shared_ptr<DmAniBindTargetCallback> callback = iter->second;
            if (callback != nullptr) {
                callback->Release();
            }
        }
        g_bindCallbackMap.erase(bundleName);
    }
}

} // namespace ANI::distributedDeviceManager

TH_EXPORT_CPP_API_CreateDeviceManager(ANI::distributedDeviceManager::CreateDeviceManager);
TH_EXPORT_CPP_API_ReleaseDeviceManager(ANI::distributedDeviceManager::ReleaseDeviceManager);
TH_EXPORT_CPP_API_MakeDeviceBasicInfo(ANI::distributedDeviceManager::MakeDeviceBasicInfo);
TH_EXPORT_CPP_API_MakeDeviceNameChangeResult(ANI::distributedDeviceManager::MakeDeviceNameChangeResult);
TH_EXPORT_CPP_API_MakeReplyResult(ANI::distributedDeviceManager::MakeReplyResult);
TH_EXPORT_CPP_API_MakeDiscoveryFailureResult(ANI::distributedDeviceManager::MakeDiscoveryFailureResult);
TH_EXPORT_CPP_API_MakeDiscoverySuccessResult(ANI::distributedDeviceManager::MakeDiscoverySuccessResult);
TH_EXPORT_CPP_API_MakeDeviceStateChangeResult(ANI::distributedDeviceManager::MakeDeviceStateChangeResult);
