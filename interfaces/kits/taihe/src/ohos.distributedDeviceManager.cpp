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
const uint32_t DM_MAX_DEVICESLIST_SIZE = 50;
const std::string DM_NAPI_EVENT_DEVICE_DISCOVER_SUCCESS = "discoverSuccess";
const std::string DM_NAPI_EVENT_DEVICE_DISCOVER_FAIL = "discoverFail";

using namespace OHOS::DistributedHardware;
using namespace ani_utils;
using namespace ani_errorutils;

namespace ANI::distributedDeviceManager {

std::mutex g_initCallbackMapMutex;
std::mutex g_deviceNameChangeCallbackMapMutex;
std::mutex g_discoveryCallbackMapMutex;
std::mutex g_deviceStateChangeDataCallbackMapMutex;
std::mutex g_dmUiCallbackMapMutex;
std::mutex g_bindCallbackMapMutex;
std::mutex g_authCallbackMapMutex;

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

bool DeviceManagerImpl::IsInit()
{
    std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
    if (bundleName_.empty()) {
        return false;
    }
    if (g_initCallbackMap.find(bundleName_) == g_initCallbackMap.end()) {
        return false;
    }
    return true;
}

DeviceManagerImpl::~DeviceManagerImpl()
{
    ClearBundleCallbacks(bundleName_);
}

int64_t DeviceManagerImpl::GetInner()
{
    return reinterpret_cast<int64_t>(this);
}

bool DeviceManagerImpl::CreatePromise(ani_env* &env, ani_vm* &vm, ani_object &promise, ani_resolver &deferred)
{
    env = nullptr;
    vm = nullptr;
    promise = nullptr;
    deferred = nullptr;

    env = taihe::get_env();
    if (env == nullptr) {
        LOGE("get_env failed");
        return false;
    }
    ani_status status = ANI_OK;
    if ((status = env->Promise_New(&deferred, &promise)) != ANI_OK) {
        LOGE("create promise object failed, status = %{public}d", status);
        return false;
    }
    status = env->GetVM(&vm);
    if (status != ANI_OK || vm == nullptr) {
        LOGE("GetVM failed, status = %{public}d", status);
        return false;
    }
    return true;
}

ohos::distributedDeviceManager::DeviceManager CreateDeviceManager(taihe::string_view bundleName)
{
    bool checkRet = ani_dmutils::CheckJsParamStringValid(std::string(bundleName));
    if (!checkRet) {
        return taihe::make_holder<DeviceManagerImpl, ohos::distributedDeviceManager::DeviceManager>();
    }
    ani_env *env = taihe::get_env();
    std::string stdBundleName(bundleName);
    std::shared_ptr<DmAniInitCallback> initCallback = std::make_shared<DmAniInitCallback>(env, bundleName);
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().InitDeviceManager(
        stdBundleName, initCallback);
    if (ret != 0) {
        LOGE("CreateDeviceManager for bundleName %{public}s failed, ret %{public}d.", stdBundleName.c_str(), ret);
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
    DeviceManagerImpl *impl = reinterpret_cast<DeviceManagerImpl *>(deviceManager->GetInner());
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

::taihe::string DeviceManagerImpl::GetLocalDeviceId()
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return "";
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(ERR_DM_NO_PERMISSION);
        return "";
    }
    std::string deviceId;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceId(bundleName_, deviceId);
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
        return "";
    }
    return ::taihe::string(deviceId);
}

::taihe::string DeviceManagerImpl::GetLocalDeviceNetworkId()
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return "";
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(ERR_DM_NO_PERMISSION);
        return "";
    }
    std::string networkId;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceNetWorkId(
        bundleName_, networkId);
    if (ret != 0) {
        LOGE("GetLocalDeviceNetworkId for failed, ret %{public}d", ret);
        ani_errorutils::CreateBusinessError(ret);
        return "";
    }
    return ::taihe::string(networkId);
}

::taihe::string DeviceManagerImpl::GetLocalDeviceName()
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return "";
    }
    std::string deviceName;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceName(bundleName_, deviceName);
    if (ret != 0) {
        LOGE("GetLocalDeviceName for failed, ret %{public}d", ret);
        ani_errorutils::CreateBusinessError(ret);
    }
    return ::taihe::string(deviceName);
}

int32_t DeviceManagerImpl::GetLocalDeviceType()
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return 0;
    }
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ERR_DM_NO_PERMISSION);
        return 0;
    }
    int32_t deviceType = 0;
    ret = DeviceManager::GetInstance().GetLocalDeviceType(bundleName_, deviceType);
    if (ret != 0) {
        LOGE("GetLocalDeviceType for failed, ret %{public}d", ret);
        ani_errorutils::CreateBusinessError(ret);
        return 0;
    }
    return deviceType;
}

::taihe::string DeviceManagerImpl::GetDeviceName(taihe::string_view networkId)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return "";
    }
    bool checkRet = ani_dmutils::CheckJsParamStringValid(std::string(networkId));
    if (!checkRet) {
        return "";
    }

    std::string deviceName;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceName(
        bundleName_, std::string(networkId), deviceName);
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
        return "";
    }
    return ::taihe::string(deviceName);
}

int32_t DeviceManagerImpl::GetDeviceType(taihe::string_view networkId)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return 0;
    }
    bool checkRet = ani_dmutils::CheckJsParamStringValid(std::string(networkId));
    if (!checkRet) {
        return 0;
    }

    int32_t deviceType;
    int32_t ret = OHOS::DistributedHardware::DeviceManager::GetInstance().GetDeviceType(
        bundleName_, std::string(networkId), deviceType);
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
        return 0;
    }
    return deviceType;
}

void DeviceManagerImpl::ReplyUiAction(int32_t action, ::taihe::string_view actionResult)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (!IsSystemApp()) {
        LOGE("ReplyUiAction not SystemApp");
        ani_errorutils::CreateBusinessError(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP);
        return;
    }
    bool checkRet = ani_dmutils::CheckJsParamStringValid(std::string(actionResult));
    if (!checkRet) {
        return;
    }
    int32_t ret = DeviceManager::GetInstance().SetUserOperation(bundleName_, action, std::string(actionResult));
    if (ret != 0) {
        LOGE("SetUserOperation for bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
    }
}

uintptr_t DeviceManagerImpl::GetDeviceProfileInfoList(
    ::ohos::distributedDeviceManager::DeviceProfileInfoFilterOptions const& filterOptions)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return 0;
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return 0;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != DM_OK) {
        LOGE("CheckAPIAccessPermission failed");
        ani_errorutils::CreateBusinessError(ret);
        return 0;
    }
    ani_env *env = nullptr;
    ani_vm *vm = nullptr;
    ani_object promise = nullptr;
    ani_resolver deferred = nullptr;
    bool bret = CreatePromise(env, vm, promise, deferred);
    if (!bret) {
        LOGE("CreatePromise failed");
        return reinterpret_cast<uintptr_t>(promise);
    }
    OHOS::DistributedHardware::DmDeviceProfileInfoFilterOptions dmOptions;
    dmOptions.isCloud = filterOptions.isCloud;
    if (filterOptions.deviceIdList.has_value()) {
        dmOptions.deviceIdList = std::vector<std::string>(
            filterOptions.deviceIdList.value().begin(), filterOptions.deviceIdList.value().end());
    }
    AsyncUtilBase::AsyncExecute([vm, deferred, dmOptions, bundleName = bundleName_] {
        auto callback = std::make_shared<DmAniGetDeviceProfileInfoListCallback>(vm, deferred);
        int32_t ret = DeviceManager::GetInstance().GetDeviceProfileInfoList(bundleName, dmOptions, callback);
        if (ret == ERR_DM_CALLBACK_REGISTER_FAILED) {
            LOGE("GetDeviceProfileInfoList failed, bundleName:%{public}s, ret=%{public}d",
                bundleName.c_str(), ret);
            callback->OnResult({}, ret);
        }
    });
    return reinterpret_cast<uintptr_t>(promise);
}

int32_t DeviceManagerImpl::PutDeviceProfileInfoListSync(
    ::taihe::array_view<::ohos::distributedDeviceManager::DeviceProfileInfo> deviceProfileInfoList)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return 0;
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return 0;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != DM_OK) {
        LOGE("CheckAPIAccessPermission failed");
        ani_errorutils::CreateBusinessError(ret);
        return 0;
    }
    std::vector<OHOS::DistributedHardware::DmDeviceProfileInfo> dmInfoList;
    ani_dmutils::DeviceProfileInfoArrayToNative(deviceProfileInfoList, dmInfoList);
    ret = DeviceManager::GetInstance().PutDeviceProfileInfoList(bundleName_, dmInfoList);
    if (ret != DM_OK) {
        LOGE("PutDeviceProfileInfoList failed, bundleName:%{public}s, code=%{public}d",
            bundleName_.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
    }

    return ret;
}

uintptr_t DeviceManagerImpl::GetDeviceIconInfo(
    ::ohos::distributedDeviceManager::DeviceIconInfoFilterOptions const& filterOptions)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return 0;
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return 0;
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != DM_OK) {
        LOGE("CheckAPIAccessPermission failed");
        ani_errorutils::CreateBusinessError(ret);
        return 0;
    }
    ani_env *env = nullptr;
    ani_vm *vm = nullptr;
    ani_object promise = nullptr;
    ani_resolver deferred = nullptr;
    bool bret = CreatePromise(env, vm, promise, deferred);
    if (!bret) {
        LOGE("CreatePromise failed");
        return reinterpret_cast<uintptr_t>(promise);
    }
    OHOS::DistributedHardware::DmDeviceIconInfoFilterOptions dmOptions;
    dmOptions.productId = std::string(filterOptions.productId);
    if (filterOptions.subProductId.has_value()) {
        dmOptions.subProductId = std::string(filterOptions.subProductId.value());
    }
    dmOptions.imageType = std::string(filterOptions.imageType);
    dmOptions.specName = std::string(filterOptions.specName);
    if (filterOptions.internalModel.has_value()) {
        dmOptions.internalModel = std::string(filterOptions.internalModel.value());
    }
    AsyncUtilBase::AsyncExecute([vm, deferred, dmOptions, bundleName = bundleName_] {
        auto callback = std::make_shared<DmAniGetDeviceIconInfoCallback>(vm, deferred);
        int32_t ret = DeviceManager::GetInstance().GetDeviceIconInfo(bundleName,
            dmOptions, callback);
        if (ret != DM_OK) {
            LOGE("GetDeviceIconInfoPromise failed, bundleName:%{public}s, ret=%{public}d",
                bundleName.c_str(), ret);
            callback->OnResult({}, ret);
        }
    });
    return reinterpret_cast<uintptr_t>(promise);
}

::taihe::string DeviceManagerImpl::GetLocalDisplayDeviceNameSync(int32_t maxNameLength)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return "";
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return "";
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != DM_OK) {
        LOGE("CheckAPIAccessPermission failed");
        ani_errorutils::CreateBusinessError(ret);
        return "";
    }
    std::string displayName;
    ret = DeviceManager::GetInstance().GetLocalDisplayDeviceName(bundleName_, maxNameLength, displayName);
    if (ret != DM_OK) {
        LOGE("GetLocalDisplayDeviceName failed, bundleName:%{public}s, ret=%{public}d",
            bundleName_.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
    }
    return ::taihe::string(displayName);
}

uintptr_t DeviceManagerImpl::SetLocalDeviceName(::taihe::string_view deviceName)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return 0;
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        ani_errorutils::CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return 0;
    }
    bool checkRet = ani_dmutils::CheckJsParamStringValid(std::string(deviceName));
    if (!checkRet) {
        return 0;
    }
    if (deviceName.size() > DEVICE_NAME_MAX_BYTES) {
        LOGE("deviceName is too long");
        ani_errorutils::CreateBusinessError(ERR_DM_INPUT_PARA_INVALID);
        return 0;
    }
    ani_env *env = nullptr;
    ani_vm *vm = nullptr;
    ani_object promise = nullptr;
    ani_resolver deferred = nullptr;
    bool ret = CreatePromise(env, vm, promise, deferred);
    if (!ret) {
        LOGE("CreatePromise failed");
        return reinterpret_cast<uintptr_t>(promise);
    }
    std::string stdname(deviceName);
    AsyncUtilBase::AsyncExecute([vm, deferred, stdname, bundleName = bundleName_] {
        auto callback = std::make_shared<DmAniSetLocalDeviceNameCallback>(vm, deferred);
        int32_t ret = DeviceManager::GetInstance().SetLocalDeviceName(bundleName, stdname, callback);
        if (ret == ERR_DM_CALLBACK_REGISTER_FAILED) {
            LOGE("SetLocalDeviceName failed, bundleName:%{public}s, ret=%{public}d",
                bundleName.c_str(), ret);
            callback->OnResult(ret);
        }
    });
    return reinterpret_cast<uintptr_t>(promise);
}

uintptr_t DeviceManagerImpl::SetRemoteDeviceName(::taihe::string_view deviceId, ::taihe::string_view deviceName)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return 0;
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        ani_errorutils::CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return 0;
    }
    bool checkRet = ani_dmutils::CheckJsParamStringValid(std::string(deviceId));
    if (!checkRet) {
        return 0;
    }
    checkRet = ani_dmutils::CheckJsParamStringValid(std::string(deviceName));
    if (!checkRet) {
        return 0;
    }
    if (deviceName.size() > DEVICE_NAME_MAX_BYTES) {
        LOGE("deviceName is too long");
        ani_errorutils::CreateBusinessError(ERR_DM_INPUT_PARA_INVALID);
        return 0;
    }
    ani_env *env = nullptr;
    ani_vm *vm = nullptr;
    ani_object promise = nullptr;
    ani_resolver deferred = nullptr;
    bool ret = CreatePromise(env, vm, promise, deferred);
    if (!ret) {
        LOGE("CreatePromise failed");
        return reinterpret_cast<uintptr_t>(promise);
    }
    std::string stdId(deviceId);
    std::string stdName(deviceName);
    AsyncUtilBase::AsyncExecute([vm, deferred, stdId, stdName, bundleName = bundleName_] {
        auto callback = std::make_shared<DmAniSetRemoteDeviceNameCallback>(vm, deferred);
        int32_t ret = DeviceManager::GetInstance().SetRemoteDeviceName(bundleName, stdId, stdName, callback);
        if (ret == ERR_DM_CALLBACK_REGISTER_FAILED) {
            LOGE("SetRemoteDeviceName failed, bundleName:%{public}s, ret=%{public}d",
                bundleName.c_str(), ret);
            callback->OnResult(ret);
        }
    });
    return reinterpret_cast<uintptr_t>(promise);
}

void DeviceManagerImpl::SetHeartbeatPolicy(::ohos::distributedDeviceManager::StrategyForHeartbeat policy,
    int32_t delayTime)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        ani_errorutils::CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return;
    }
    std::map<std::string, std::string> policyParam;
    policyParam[PARAM_KEY_POLICY_STRATEGY_FOR_BLE] = std::to_string(policy.get_value());
    policyParam[PARAM_KEY_POLICY_TIME_OUT] = std::to_string(delayTime);
    int32_t ret = DeviceManager::GetInstance().SetDnPolicy(bundleName_, policyParam);
    if (ret != 0) {
        LOGE("SetDnPolicy bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
    }
}

void DeviceManagerImpl::RestoreLocalDeviceName()
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        ani_errorutils::CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return;
    }
    int32_t ret = DeviceManager::GetInstance().RestoreLocalDeviceName(bundleName_);
    if (ret != 0) {
        LOGE("RestoreLocalDeviceName, bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
    }
}

::taihe::array<::taihe::string> DeviceManagerImpl::GetDeviceNetworkIdListSync(
    ::ohos::distributedDeviceManager::NetworkIdQueryFilter const& filterOptions)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return {};
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        ani_errorutils::CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return {};
    }
    int32_t ret = DeviceManager::GetInstance().CheckAPIAccessPermission();
    if (ret != DM_OK) {
        LOGE("CheckAPIAccessPermission failed");
        ani_errorutils::CreateBusinessError(ret);
        return {};
    }

    OHOS::DistributedHardware::NetworkIdQueryFilter dmFilter;
    dmFilter.wiseDeviceId = std::string(filterOptions.wiseDeviceId);
    dmFilter.onlineStatus = filterOptions.onlineStatus;
    std::vector<std::string> deviceNetworkIds;
    ret = DeviceManager::GetInstance().GetDeviceNetworkIdList(bundleName_, dmFilter, deviceNetworkIds);
    if (ret != 0) {
        LOGE("GetDeviceNetworkIdList, bundleName %{public}s failed, ret %{public}d", bundleName_.c_str(), ret);
        ani_errorutils::CreateBusinessError(ret);
        return {};
    }
    if (deviceNetworkIds.size() == 0) {
        return {};
    }
    return ::taihe::array<::taihe::string>(::taihe::copy_data_t{}, deviceNetworkIds.data(), deviceNetworkIds.size());
}

void DeviceManagerImpl::OnDeviceNameChange(taihe::callback_view<void(
    ohos::distributedDeviceManager::DeviceNameChangeResult const&)> onDeviceNameChangecb)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION);
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
}

void DeviceManagerImpl::OnReplyResult(taihe::callback_view<void(
    ohos::distributedDeviceManager::ReplyResult const&)> onReplyResultcb)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION);
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
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION);
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

void DeviceManagerImpl::OnServiceDie(::taihe::callback_view<void(uintptr_t)> onServiceDiecb)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION);
        return;
    }

    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        auto iter = g_initCallbackMap.find(bundleName_);
        if (iter == g_initCallbackMap.end() || iter->second == nullptr) {
            LOGW("DmAniInitCallback not exists");
            return;
        }
        iter->second->SetServiceDieCallback(onServiceDiecb);
    }
}

void DeviceManagerImpl::OffDeviceNameChange(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::DeviceNameChangeResult const&)>> offDeviceNameChangecb)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION);
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
}

void DeviceManagerImpl::OffReplyResult(taihe::optional_view<taihe::callback<void(
    ohos::distributedDeviceManager::ReplyResult const&)>> offReplyResultcb)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION);
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
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION);
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
}

void DeviceManagerImpl::OffServiceDie(taihe::optional_view<taihe::callback<void(uintptr_t data)>> offServiceDiecb)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    if (OHOS::DistributedHardware::DeviceManager::GetInstance().CheckNewAPIAccessPermission() != 0) {
        ani_errorutils::CreateBusinessError(OHOS::DistributedHardware::ERR_DM_NO_PERMISSION);
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

std::shared_ptr<DmAniBindTargetCallback> DeviceManagerImpl::GetBindTargetCallback(const std::string &bundleName)
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

std::shared_ptr<DmAniAuthenticateCallback> DeviceManagerImpl::GetAuthenticateTargetCallback(
    const std::string &bundleName)
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
        ani_errorutils::CreateBusinessError(ret);
        return;
    }
    ClearBundleCallbacks(bundleName_);
}

::taihe::array<::ohos::distributedDeviceManager::DeviceBasicInfo> DeviceManagerImpl::GetAvailableDeviceListSync()
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        ::taihe::array<::ohos::distributedDeviceManager::DeviceBasicInfo> empty = {};
        return empty;
    }
    int32_t ret = DeviceManager::GetInstance().CheckNewAPIAccessPermission();
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
        ::taihe::array<::ohos::distributedDeviceManager::DeviceBasicInfo> empty = {};
        return empty;
    }
    std::vector<DmDeviceBasicInfo> devList;
    ret = DeviceManager::GetInstance().GetAvailableDeviceList(bundleName_, devList);
    if (ret != 0) {
        LOGE("GetAvailableDeviceList for bundleName %{public}s failed, ret %{public}d",
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
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
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
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
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
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
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
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
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

int32_t DeviceManagerImpl::BindTargetWrapper(const std::string &pkgName, const std::string &deviceId,
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
    if (IsUint16(bindParamObj, PARAM_KEY_WIFI_PORT)) {
        targetId.wifiPort = bindParamObj[PARAM_KEY_WIFI_PORT].Get<uint16_t>();
    }

    std::map<std::string, std::string> bindParamMap;
    ani_dmutils::InsertMapParams(bindParamObj, bindParamMap);
    return DeviceManager::GetInstance().BindTarget(pkgName, targetId, bindParamMap, callback);
}

void DeviceManagerImpl::BindTarget(::taihe::string_view deviceId,
    ::taihe::map_view<::taihe::string, ::ohos::distributedDeviceManager::IntAndStrUnionType> bindParam,
    ::taihe::callback_view<
        void(uintptr_t err, ::ohos::distributedDeviceManager::BindTargetResult const& data)> callback)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    std::string stdDeviceId(deviceId);
    bool checkRet = ani_dmutils::CheckJsParamStringValid(stdDeviceId);
    if (!checkRet) {
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
        int32_t ret = BindTargetWrapper(bundleName_, stdDeviceId, strBindParam, bindTargetCallback);
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
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    std::string stdDeviceId(deviceId);
    bool checkRet = ani_dmutils::CheckJsParamStringValid(stdDeviceId);
    if (!checkRet) {
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

void DeviceManagerImpl::LockSuccDiscoveryCallbackMutex(const std::string &bundleName,
    std::map<std::string, std::string> &discParam, const std::string &extra)
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
        discoveryCallback->OnDiscoveryFailed(0, ret);
    }
}

void DeviceManagerImpl::StartDiscovering(
    ::taihe::map_view<::taihe::string, ::ohos::distributedDeviceManager::IntAndStrUnionType> discoverParam,
    ::taihe::optional_view<::taihe::map<::taihe::string,
        ohos::distributedDeviceManager::IntAndStrUnionType>> filterOptions)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
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
    LockSuccDiscoveryCallbackMutex(bundleName_, discParam, extra);
}

void DeviceManagerImpl::StopDiscovering()
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return;
    }
    uint64_t tokenId = OHOS::IPCSkeleton::GetSelfTokenID();
    int32_t ret = DeviceManager::GetInstance().StopDeviceDiscovery(tokenId, bundleName_);
    if (ret != 0) {
        ani_errorutils::CreateBusinessError(ret);
    }
}

void DeviceManagerImpl::JsToBindParam(ani_env *env,
    const ::taihe::map_view<::taihe::string, ohos::distributedDeviceManager::IntAndStrUnionType> &object,
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
    bindParam = jsonObj.Dump();
}

bool DeviceManagerImpl::JsToDiscoverTargetType(ani_env *env,
    const ::taihe::map_view<::taihe::string, ::ohos::distributedDeviceManager::IntAndStrUnionType> &object,
    int32_t &discoverTargetType)
{
    return ani_utils::AniGetMapItem(env, object, "discoverTargetType", discoverTargetType);
}

void DeviceManagerImpl::JsToDmDiscoveryExtra(ani_env *env,
    const ::taihe::map_view<::taihe::string, ::ohos::distributedDeviceManager::IntAndStrUnionType> &object,
    std::string &extra)
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
    extra = jsonObj.Dump();
    LOGI("JsToDmDiscoveryExtra, extra :%{public}s", extra.c_str());
}

void DeviceManagerImpl::JsToDiscoveryParam(ani_env *env,
    const ::taihe::map_view<taihe::string, ::ohos::distributedDeviceManager::IntAndStrUnionType> &object,
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

void DeviceManagerImpl::ClearDiscoverCallbacks(const std::string &bundleName)
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

void DeviceManagerImpl::ClearAuthCallbacks(const std::string &bundleName)
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

void DeviceManagerImpl::ClearBindCallbacks(const std::string &bundleName)
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

void DeviceManagerImpl::ClearBundleCallbacks(const std::string &bundleName)
{
    LOGI("ClearBundleCallbacks");
    {
        std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
        g_initCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceNameChangeCallbackMapMutex);
        g_deviceNameChangeCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_deviceStateChangeDataCallbackMapMutex);
        g_deviceStateChangeDataCallbackMap.erase(bundleName);
    }
    {
        std::lock_guard<std::mutex> autoLock(g_dmUiCallbackMapMutex);
        g_dmUiCallbackMap.erase(bundleName);
    }
    ClearBindCallbacks(bundleName);
    ClearAuthCallbacks(bundleName);
    ClearDiscoverCallbacks(bundleName);
}

::taihe::array<::ohos::distributedDeviceManager::DeviceIdentification>
DeviceManagerImpl::GetIdentificationByDeviceIds(::taihe::array_view<::taihe::string> deviceIdList)
{
    if (!IsInit()) {
        ani_errorutils::CreateBusinessError(DM_ERR_FAILED);
        return {};
    }
    if (!IsSystemApp()) {
        LOGE("Caller is not systemApp");
        CreateBusinessError(static_cast<int32_t>(DMBusinessErrorCode::ERR_NOT_SYSTEM_APP));
        return {};
    }
    if (deviceIdList.size() == 0 || deviceIdList.size() > DM_MAX_DEVICESLIST_SIZE) {
        LOGE("Invalid param, the size of deviceIdList is %{public}zu", deviceIdList.size());
        CreateBusinessError(ERR_DM_INPUT_PARA_INVALID);
        return {};
    }

    std::vector<std::string> deviceIdListVec;
    for (const auto& deviceId : deviceIdList) {
        deviceIdListVec.push_back(std::string(deviceId));
    }

    std::map<std::string, std::string> deviceIdentificationMap;
    int32_t ret = DeviceManager::GetInstance().GetIdentificationByDeviceIds(
        bundleName_, deviceIdListVec, deviceIdentificationMap);
    if (ret != DM_OK) {
        LOGE("GetIdentificationByDeviceIds failed, ret %{public}d", ret);
        CreateBusinessError(ret);
        return {};
    }

    std::vector<::ohos::distributedDeviceManager::DeviceIdentification> result;
    for (const auto& pair : deviceIdentificationMap) {
        result.emplace_back(::ohos::distributedDeviceManager::DeviceIdentification{
            ::taihe::string(pair.first), ::taihe::string(pair.second)});
    }
    return ::taihe::array<::ohos::distributedDeviceManager::DeviceIdentification>(
        ::taihe::copy_data_t{}, result.data(), result.size());
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
