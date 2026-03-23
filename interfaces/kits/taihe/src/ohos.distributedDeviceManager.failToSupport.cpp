/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ohos.distributedDeviceManager.failToSupport.h"
#include "ani_error_utils.h"
#include <string>

#include "ani_utils.h"
#include "ani_dm_utils.h"
#include "dm_log.h"


using namespace OHOS::DistributedHardware;
using namespace ani_utils;
using namespace ani_errorutils;

namespace ANI::distributedDeviceManager {

std::mutex g_initCallbackMapMutex;
const int32_t DM_NAPI_BUF_LENGTH = 256;
const int32_t DM_ERR_DEVICE_NOT_SUPPORT = 801;
bool CheckJsParamStringValid(const std::string &param)
{
    if (param.size() == 0 || param.size() >= DM_NAPI_BUF_LENGTH) {
        return false;
    }
    return true;
}

void ThrowErrorLite(int32_t code, const char* message)
{
    if (message == nullptr) {
        return;
    }
    std::string errMsg(message);
    taihe::set_business_error(code, errMsg);
}

DmImpFaiToSupport::DmImpFaiToSupport(const std::string& bundleName)
    : bundleName_(bundleName)
{
}

DmImpFaiToSupport::DmImpFaiToSupport(std::shared_ptr<DmImpFaiToSupport> impl)
{
    std::lock_guard<std::mutex> autoLock(g_initCallbackMapMutex);
    if (impl != nullptr) {
        bundleName_ = impl->bundleName_;
    }
}

DmImpFaiToSupport::~DmImpFaiToSupport()
{
}

int64_t DmImpFaiToSupport::GetInner()
{
    return reinterpret_cast<int64_t>(this);
}

ohos::distributedDeviceManager::DeviceManager CreateDeviceManager(taihe::string_view bundleName)
{
    bool checkRet = CheckJsParamStringValid(std::string(bundleName));
    if (!checkRet) {
        return taihe::make_holder<DmImpFaiToSupport, ohos::distributedDeviceManager::DeviceManager>();
    }
    std::string stdBundleName(bundleName);
    return taihe::make_holder<DmImpFaiToSupport,
        ohos::distributedDeviceManager::DeviceManager>(stdBundleName);
}

void ReleaseDeviceManager(::ohos::distributedDeviceManager::weak::DeviceManager deviceManager)
{
    DmImpFaiToSupport *impl = reinterpret_cast<DmImpFaiToSupport *>(deviceManager->GetInner());
    if (impl) {
        impl->ReleaseDeviceManager();
    }
}

::ohos::distributedDeviceManager::DeviceBasicInfo MakeDeviceBasicInfo(::taihe::string_view deviceId,
    ::taihe::string_view deviceName, ::taihe::string_view deviceType,
    ::taihe::optional_view<::taihe::string> networkId)
{
    LOGI("device not support");
    return {deviceId, deviceName, deviceType, networkId};
}

ohos::distributedDeviceManager::DeviceNameChangeResult MakeDeviceNameChangeResult(taihe::string_view deviceName)
{
    LOGI("device not support");
    return {deviceName};
}

ohos::distributedDeviceManager::ReplyResult MakeReplyResult(taihe::string_view param)
{
    LOGI("device not support");
    return {param};
}

ohos::distributedDeviceManager::DiscoveryFailureResult MakeDiscoveryFailureResult(int32_t reason)
{
    LOGI("device not support");
    return {reason};
}

ohos::distributedDeviceManager::DiscoverySuccessResult MakeDiscoverySuccessResult(
    ohos::distributedDeviceManager::DeviceBasicInfo const& device)
{
    LOGI("device not support");
    return {device};
}

ohos::distributedDeviceManager::DeviceStateChangeResult MakeDeviceStateChangeResult(
    ohos::distributedDeviceManager::DeviceStateChange deviceStateChange,
    ohos::distributedDeviceManager::DeviceBasicInfo const& deviceBasicInfo)
{
    LOGI("device not support");
    return {deviceStateChange, deviceBasicInfo};
}

::taihe::string DmImpFaiToSupport::GetLocalDeviceId()
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return "";
}

::taihe::string DmImpFaiToSupport::GetLocalDeviceNetworkId()
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return "";
}

::taihe::string DmImpFaiToSupport::GetLocalDeviceName()
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return "";
}

int32_t DmImpFaiToSupport::GetLocalDeviceType()
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return 0;
}

::taihe::string DmImpFaiToSupport::GetDeviceName(taihe::string_view networkId)
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return "";
}

int32_t DmImpFaiToSupport::GetDeviceType(taihe::string_view networkId)
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return 0;
}

void DmImpFaiToSupport::ReplyUiAction(int32_t action, ::taihe::string_view actionResult)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

uintptr_t DmImpFaiToSupport::GetDeviceProfileInfoList(
    ::ohos::distributedDeviceManager::DeviceProfileInfoFilterOptions const& filterOptions)
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return 0;
}

int32_t DmImpFaiToSupport::PutDeviceProfileInfoListSync(
    ::taihe::array_view<::ohos::distributedDeviceManager::DeviceProfileInfo> deviceProfileInfoList)
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return 0;
}

uintptr_t DmImpFaiToSupport::GetDeviceIconInfo(
    ::ohos::distributedDeviceManager::DeviceIconInfoFilterOptions const& filterOptions)
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return 0;
}

::taihe::string DmImpFaiToSupport::GetLocalDisplayDeviceNameSync(int32_t maxNameLength)
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return "";
}

uintptr_t DmImpFaiToSupport::SetLocalDeviceName(::taihe::string_view deviceName)
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return 0;
}

void DmImpFaiToSupport::SetHeartbeatPolicy(::ohos::distributedDeviceManager::StrategyForHeartbeat policy,
    int32_t delayTime)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::RestoreLocalDeviceName()
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

::taihe::array<::taihe::string> DmImpFaiToSupport::GetDeviceNetworkIdListSync(
    ::ohos::distributedDeviceManager::NetworkIdQueryFilter const& filterOptions)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return {};
}

void DmImpFaiToSupport::ReleaseDeviceManager()
{
    LOGI("device not support");
    return;
}

::taihe::array<::ohos::distributedDeviceManager::DeviceBasicInfo> DmImpFaiToSupport::GetAvailableDeviceListSync()
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    ::taihe::array<::ohos::distributedDeviceManager::DeviceBasicInfo> empty = {};
    return empty;
}

void DmImpFaiToSupport::BindTarget(::taihe::string_view deviceId,
    ::taihe::map_view<::taihe::string, ::ohos::distributedDeviceManager::IntAndStrUnionType> bindParam,
    ::taihe::callback_view<
        void(uintptr_t err, ::ohos::distributedDeviceManager::BindTargetResult const& data)> callback)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::UnbindTarget(taihe::string_view deviceId)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::StartDiscovering(
    ::taihe::map_view<::taihe::string, ::ohos::distributedDeviceManager::IntAndStrUnionType> discoverParam,
    ::taihe::optional_view<::taihe::map<::taihe::string,
        ohos::distributedDeviceManager::IntAndStrUnionType>> filterOptions)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::StopDiscovering()
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

::taihe::array<::ohos::distributedDeviceManager::DeviceIdentification> DmImpFaiToSupport::GetIdentificationByDeviceIds(
    ::taihe::array_view<::taihe::string> deviceIdList)
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return {};
}

uintptr_t DmImpFaiToSupport::SetRemoteDeviceName(
    ::taihe::string_view deviceId, ::taihe::string_view deviceName)
{
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
    return 0;
}

void DmImpFaiToSupport::OnDiscoverFailure(::taihe::callback_view<void(
    ::ohos::distributedDeviceManager::DiscoveryFailureResult const&)> onDiscoverFailurecb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OnDiscoverSuccess(::taihe::callback_view<void(
    ::ohos::distributedDeviceManager::DiscoverySuccessResult const&)> onDiscoverSuccesscb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OffDiscoverFailure(::taihe::optional_view<::taihe::callback<void(
    ::ohos::distributedDeviceManager::DiscoveryFailureResult const&)>> offDiscoverFailurecb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OffDiscoverSuccess(::taihe::optional_view<::taihe::callback<void(
    ::ohos::distributedDeviceManager::DiscoverySuccessResult const&)>> offDiscoverSuccesscb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OnDeviceNameChange(::taihe::callback_view<void(
    ::ohos::distributedDeviceManager::DeviceNameChangeResult const&)> onDeviceNameChangecb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OnReplyResult(::taihe::callback_view<void(
    ::ohos::distributedDeviceManager::ReplyResult const&)> onReplyResultcb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OnDeviceStateChange(::taihe::callback_view<void(
    ::ohos::distributedDeviceManager::DeviceStateChangeResult const&)> onDeviceStateChangecb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OnServiceDie(::taihe::callback_view<void(uintptr_t)> onServiceDiecb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OffDeviceNameChange(::taihe::optional_view<::taihe::callback<void(
    ::ohos::distributedDeviceManager::DeviceNameChangeResult const&)>> offDeviceNameChangecb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OffReplyResult(::taihe::optional_view<::taihe::callback<void(
    ::ohos::distributedDeviceManager::ReplyResult const&)>> offReplyResultcb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OffDeviceStateChange(::taihe::optional_view<::taihe::callback<void(
    ::ohos::distributedDeviceManager::DeviceStateChangeResult const&)>> offDeviceStateChangecb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
}

void DmImpFaiToSupport::OffServiceDie(
    ::taihe::optional_view<::taihe::callback<void(uintptr_t data)>> offServiceDiecb)
{
    LOGI("device not support");
    ThrowErrorLite(DM_ERR_DEVICE_NOT_SUPPORT, "device not support");
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
