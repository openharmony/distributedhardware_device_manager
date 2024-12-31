/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "component_manager.h"

#include <cinttypes>
#include <future>
#include <pthread.h>
#include <string>
#include <thread>

#include "ffrt.h"
#include "ipc_object_stub.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "component_disable.h"
#include "component_enable.h"
#include "component_loader.h"
#include "constants.h"
#include "device_manager.h"
#include "dh_context.h"
#include "dh_data_sync_trigger_listener.h"
#include "dh_state_listener.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_hisysevent.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "enabled_comps_dump.h"
#include "local_capability_info_manager.h"
#include "low_latency.h"
#include "meta_info_manager.h"
#include "publisher.h"
#include "task_executor.h"
#include "task_factory.h"
#include "version_info_manager.h"
#include "version_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentManager"

IMPLEMENT_SINGLE_INSTANCE(ComponentManager);

namespace {
    constexpr int32_t ENABLE_RETRY_MAX_TIMES = 3;
    constexpr int32_t DISABLE_RETRY_MAX_TIMES = 3;
    constexpr int32_t ENABLE_PARAM_RETRY_TIME = 500 * 1000;
    constexpr int32_t INVALID_SA_ID = -1;
    constexpr int32_t UNINIT_COMPONENT_TIMEOUT_SECONDS = 2;
    const std::string MONITOR_TASK_TIMER_ID = "monitor_task_timer_id";
}

ComponentManager::ComponentManager() : compSource_({}), compSink_({}), compSrcSaId_({}),
    compMonitorPtr_(std::make_shared<ComponentMonitor>()),
    lowLatencyListener_(sptr<LowLatencyListener>(new(std::nothrow) LowLatencyListener())),
    isUnInitTimeOut_(false), dhBizStates_({}), dhStateListener_(std::make_shared<DHStateListener>()),
    dataSyncTriggerListener_(std::make_shared<DHDataSyncTriggerListener>()),
    dhCommToolPtr_(std::make_shared<DHCommTool>()), needRefreshTaskParams_({})
{
    DHLOGI("Ctor ComponentManager");
}

ComponentManager::~ComponentManager()
{
    DHLOGD("Dtor ComponentManager");
    compMonitorPtr_.reset();
    compMonitorPtr_ = nullptr;
    lowLatencyListener_ = nullptr;
}

int32_t ComponentManager::Init()
{
    DHLOGI("start.");
    DHTraceStart(COMPONENT_INIT_START);
    InitComponentHandler();

    int32_t ret = InitSAMonitor();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Init SA monitor failed, ret: %{public}d", ret);
        return ret;
    }

    StartComponent();
    RegisterDHStateListener();
    RegisterDataSyncTriggerListener();
    InitDHCommTool();
#ifdef DHARDWARE_LOW_LATENCY
    Publisher::GetInstance().RegisterListener(DHTopic::TOPIC_LOW_LATENCY, lowLatencyListener_);
#endif
    DHLOGI("Init component success");
    DHTraceEnd();
    return DH_FWK_SUCCESS;
}

void ComponentManager::InitComponentHandler()
{
    DHLOGI("start.");
    if (!InitCompSource()) {
        DHLOGE("InitCompSource failed.");
        DHTraceEnd();
    }
    if (!InitCompSink()) {
        DHLOGE("InitCompSink failed.");
        DHTraceEnd();
    }
}

int32_t ComponentManager::InitSAMonitor()
{
    if (compMonitorPtr_ == nullptr) {
        DHLOGE("compMonitorPtr_ is null.");
        return ERR_DH_FWK_COMPONENT_MONITOR_NULL;
    }
    for (const auto &comp : compSource_) {
        if (compSrcSaId_.find(comp.first) == compSrcSaId_.end()) {
            continue;
        }
        compMonitorPtr_->AddSAMonitor(compSrcSaId_.at(comp.first));
    }
    return DH_FWK_SUCCESS;
}

void ComponentManager::StartComponent()
{
    auto sourceResult = StartSource();
    auto sinkResult = StartSink();

    if (!WaitForResult(Action::START_SOURCE, sourceResult)) {
        DHLOGE("StartSource failed, some virtual components maybe cannot work, but want to continue");
        HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "dhfwk start source failed.");
    }
    if (!WaitForResult(Action::START_SINK, sinkResult)) {
        DHLOGE("StartSink failed, some virtual components maybe cannot work, but want to continue");
        HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "dhfwk start sink failed.");
    }
}

void ComponentManager::RegisterDHStateListener()
{
    for (const auto &item : compSource_) {
        DHLOGI("Register DH State listener, dhType: %{public}" PRIu32, (uint32_t)item.first);
        if (item.second == nullptr) {
            DHLOGE("comp source ptr is null");
            continue;
        }
        item.second->RegisterDistributedHardwareStateListener(dhStateListener_);
    }
}

void ComponentManager::RegisterDataSyncTriggerListener()
{
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<ComponentManager::ComponentManagerEventHandler>(runner);

    for (const auto &item : compSource_) {
        DHLOGI("Register Data Sync Trigger listener, dhType: %{public}" PRIu32, (uint32_t)item.first);
        if (item.second == nullptr) {
            DHLOGE("comp source ptr is null");
            continue;
        }
        item.second->RegisterDataSyncTriggerListener(dataSyncTriggerListener_);
    }
}

void ComponentManager::InitDHCommTool()
{
    if (dhCommToolPtr_ == nullptr) {
        DHLOGE("DH communication tool ptr is null");
        return;
    }
    DHLOGI("Init DH communication tool");
    dhCommToolPtr_->Init();
}

int32_t ComponentManager::UnInit()
{
    DHLOGI("start.");
    UnregisterDHStateListener();
    UnregisterDataSyncTriggerListener();
    UnInitDHCommTool();
    StopPrivacy();
    UnInitSAMonitor();
    StopComponent();

#ifdef DHARDWARE_LOW_LATENCY
    Publisher::GetInstance().UnregisterListener(DHTopic::TOPIC_LOW_LATENCY, lowLatencyListener_);
    LowLatency::GetInstance().CloseLowLatency();
#endif
    DHLOGI("Release component success");

    if (isUnInitTimeOut_.load()) {
        DHLOGE("Some component stop timeout, FORCE exit!");
        _Exit(0);
    }

    return DH_FWK_SUCCESS;
}

void ComponentManager::UnInitSAMonitor()
{
    // clear SA monitor
    if (compMonitorPtr_ == nullptr) {
        DHLOGE("compMonitorPtr_ is null.");
        return;
    }
    for (const auto &comp : compSource_) {
        if (compSrcSaId_.find(comp.first) == compSrcSaId_.end()) {
            continue;
        }
        compMonitorPtr_->RemoveSAMonitor(compSrcSaId_.at(comp.first));
    }
}

void ComponentManager::UnregisterDHStateListener()
{
    for (const auto &item : compSource_) {
        DHLOGI("Unregister DH State listener, dhType: %{public}" PRIu32, (uint32_t)item.first);
        if (item.second == nullptr) {
            DHLOGE("comp source ptr is null");
            continue;
        }
        item.second->UnregisterDistributedHardwareStateListener();
    }
}

void ComponentManager::UnregisterDataSyncTriggerListener()
{
    for (const auto &item : compSource_) {
        DHLOGI("Unregister Data Sync Trigger listener, dhType: %{public}" PRIu32, (uint32_t)item.first);
        if (item.second == nullptr) {
            DHLOGE("comp source ptr is null");
            continue;
        }
        item.second->UnregisterDataSyncTriggerListener();
    }
}

void ComponentManager::UnInitDHCommTool()
{
    if (dhCommToolPtr_ == nullptr) {
        DHLOGE("DH communication tool ptr is null");
        return;
    }
    DHLOGI("UnInit DH communication tool");
    dhCommToolPtr_->UnInit();
}

void ComponentManager::StopComponent()
{
    // stop source and sink sa
    auto sourceResult = StopSource();
    auto sinkResult = StopSink();

    if (!WaitForResult(Action::STOP_SOURCE, sourceResult)) {
        DHLOGE("StopSource failed, but want to continue");
    }
    if (!WaitForResult(Action::STOP_SINK, sinkResult)) {
        DHLOGE("StopSource failed, but want to continue");
    }

    compSource_.clear();
    compSink_.clear();
}

void ComponentManager::StopPrivacy()
{
    // stop privacy
    if (cameraCompPrivacy_ != nullptr && cameraCompPrivacy_->GetPageFlag()) {
        cameraCompPrivacy_->StopPrivacePage("camera");
        cameraCompPrivacy_->SetPageFlagFalse();
    }

    if (audioCompPrivacy_ != nullptr  && audioCompPrivacy_->GetPageFlag()) {
        audioCompPrivacy_->StopPrivacePage("mic");
        audioCompPrivacy_->SetPageFlagFalse();
    }
}

ActionResult ComponentManager::StartSource()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    for (const auto &item : compSource_) {
        if (item.second == nullptr) {
            DHLOGE("comp source ptr is null");
            continue;
        }
        CompVersion compversion;
        VersionManager::GetInstance().GetCompVersion(uuid, item.first, compversion);
        auto params = compversion.sourceVersion;
        std::promise<int32_t> p;
        std::future<int32_t> f = p.get_future();
        std::thread([p = std::move(p), item, params] () mutable {
            p.set_value(item.second->InitSource(params));
        }).detach();
        futures.emplace(item.first, f.share());
    }
    return futures;
}

ActionResult ComponentManager::StartSource(DHType dhType)
{
    DHLOGI("Start Source, dhType: %{public}" PRIu32, (uint32_t)dhType);
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    if (compSource_.find(dhType) == compSource_.end()) {
        DHLOGE("Component for DHType: %{public}" PRIu32 " not init source handler", (uint32_t)dhType);
        return futures;
    }
    if (compSource_[dhType] == nullptr) {
        DHLOGE("comp source ptr is null");
        return futures;
    }
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    CompVersion compVersion;
    VersionManager::GetInstance().GetCompVersion(uuid, dhType, compVersion);
    auto params = compVersion.sourceVersion;
    std::promise<int32_t> p;
    std::future<int32_t> f = p.get_future();
    std::thread([p = std::move(p), this, dhType, params] () mutable {
        p.set_value(compSource_[dhType]->InitSource(params));
    }).detach();
    futures.emplace(dhType, f.share());

    return futures;
}

ActionResult ComponentManager::StartSink()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    for (const auto &item : compSink_) {
        if (item.second == nullptr) {
            DHLOGE("comp sink ptr is null");
            continue;
        }
        CompVersion compversion;
        VersionManager::GetInstance().GetCompVersion(uuid, item.first, compversion);
        auto params = compversion.sinkVersion;
        std::promise<int32_t> p;
        std::future<int32_t> f = p.get_future();
        std::thread([p = std::move(p), item, params] () mutable {
            p.set_value(item.second->InitSink(params));
        }).detach();
        futures.emplace(item.first, f.share());
        if (cameraCompPrivacy_ == nullptr && item.first == DHType::CAMERA) {
            cameraCompPrivacy_ = std::make_shared<ComponentPrivacy>();
            item.second->RegisterPrivacyResources(cameraCompPrivacy_);
        }
        if (audioCompPrivacy_ == nullptr && item.first == DHType::AUDIO) {
            audioCompPrivacy_ = std::make_shared<ComponentPrivacy>();
            item.second->RegisterPrivacyResources(audioCompPrivacy_);
        }
    }
    return futures;
}

ActionResult ComponentManager::StartSink(DHType dhType)
{
    DHLOGI("Start Sink, dhType: %{public}" PRIu32, (uint32_t)dhType);
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    if (compSink_.find(dhType) == compSink_.end()) {
        DHLOGE("Component for DHType: %{public}" PRIu32 " not init sink handler", (uint32_t)dhType);
        return futures;
    }
    if (compSink_[dhType] == nullptr) {
        DHLOGE("comp sink ptr is null");
        return futures;
    }
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    CompVersion compVersion;
    VersionManager::GetInstance().GetCompVersion(uuid, dhType, compVersion);
    auto params = compVersion.sinkVersion;
    std::promise<int32_t> p;
    std::future<int32_t> f = p.get_future();
    std::thread([p = std::move(p), this, dhType, params] () mutable {
        p.set_value(compSink_[dhType]->InitSink(params));
    }).detach();
    futures.emplace(dhType, f.share());
    if (cameraCompPrivacy_ == nullptr && dhType == DHType::CAMERA) {
        cameraCompPrivacy_ = std::make_shared<ComponentPrivacy>();
        compSink_[dhType]->RegisterPrivacyResources(cameraCompPrivacy_);
    }
    if (audioCompPrivacy_ == nullptr && dhType == DHType::AUDIO) {
        audioCompPrivacy_ = std::make_shared<ComponentPrivacy>();
        compSink_[dhType]->RegisterPrivacyResources(audioCompPrivacy_);
    }

    return futures;
}

ActionResult ComponentManager::StopSource()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    for (const auto &item : compSource_) {
        if (item.second == nullptr) {
            DHLOGE("comp source ptr is null");
            continue;
        }
        std::promise<int32_t> p;
        std::future<int32_t> f = p.get_future();
        std::thread([p = std::move(p), item] () mutable {
            p.set_value(item.second->ReleaseSource());
        }).detach();
        futures.emplace(item.first, f.share());
    }
    return futures;
}

ActionResult ComponentManager::StopSink()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    for (const auto &item : compSink_) {
        if (item.second == nullptr) {
            DHLOGE("comp sink ptr is null");
            continue;
        }
        std::promise<int32_t> p;
        std::future<int32_t> f = p.get_future();
        std::thread([p = std::move(p), item] () mutable {
            p.set_value(item.second->ReleaseSink());
            IHardwareHandler *hardwareHandler = nullptr;
            int32_t status = ComponentLoader::GetInstance().GetHardwareHandler(item.first, hardwareHandler);
            if (status != DH_FWK_SUCCESS || hardwareHandler == nullptr) {
                DHLOGE("GetHardwareHandler %{public}#X failed", item.first);
                return status;
            }
            hardwareHandler->UnRegisterPluginListener();
            return status;
        }).detach();
        futures.emplace(item.first, f.share());
    }
    return futures;
}

bool ComponentManager::WaitForResult(const Action &action, ActionResult actionsResult)
{
    DHLOGD("start.");
    auto ret = true;
    for (auto &iter : actionsResult) {
        std::future_status status = iter.second.wait_for(std::chrono::seconds(UNINIT_COMPONENT_TIMEOUT_SECONDS));
        if (status == std::future_status::ready) {
            auto result = iter.second.get();
            DHLOGI("action = %{public}d, compType = %{public}#X, READY, ret = %{public}d.",
                static_cast<int32_t>(action), iter.first, result);
            if (result != DH_FWK_SUCCESS) {
                ret = false;
                DHLOGE("there is error, but want to continue.");
            }
        }

        if (status == std::future_status::timeout) {
            DHLOGI("action = %{public}d, compType = %{public}#X, TIMEOUT", static_cast<int32_t>(action), iter.first);
            if (action == Action::STOP_SOURCE || action == Action::STOP_SINK) {
                isUnInitTimeOut_ = true;
            }
        }

        if (status == std::future_status::deferred) {
            DHLOGI("action = %{public}d, compType = %{public}#X, DEFERRED", static_cast<int32_t>(action), iter.first);
        }
    }
    DHLOGD("end.");
    return ret;
}

bool ComponentManager::InitCompSource()
{
    auto compTypes = ComponentLoader::GetInstance().GetAllCompTypes();
    for (const auto &type : compTypes) {
        IDistributedHardwareSource *sourcePtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSource(type, sourcePtr);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGW("GetSource failed, compType = %{public}#X, ret = %{public}d.", type, ret);
            continue;
        }
        if (sourcePtr == nullptr) {
            DHLOGW("sourcePtr is null, compType = %{public}#X.", type);
            continue;
        }
        compSource_.insert(std::make_pair(type, sourcePtr));

        int32_t saId = ComponentLoader::GetInstance().GetSourceSaId(type);
        if (saId != INVALID_SA_ID) {
            compSrcSaId_.insert(std::make_pair(type, saId));
        }
    }
    return !compSource_.empty();
}

bool ComponentManager::InitCompSink()
{
    auto compTypes = ComponentLoader::GetInstance().GetAllCompTypes();
    for (const auto &type : compTypes) {
        IDistributedHardwareSink *sinkPtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSink(type, sinkPtr);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGW("GetSink failed, compType = %{public}#X, ret = %{public}d.", type, ret);
            continue;
        }
        if (sinkPtr == nullptr) {
            DHLOGW("sinkPtr is null, compType = %{public}#X.", type);
            continue;
        }
        compSink_.insert(std::make_pair(type, sinkPtr));
    }
    return !compSink_.empty();
}

int32_t ComponentManager::Enable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("start.");
    if (compSource_.find(dhType) == compSource_.end()) {
        DHLOGE("can not find handler for dhId = %{public}s.", GetAnonyString(dhId).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    EnableParam param;
    auto ret = GetEnableParam(networkId, uuid, dhId, dhType, param);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetEnableParam failed, uuid = %{public}s, dhId = %{public}s, errCode = %{public}d",
            GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), ret);
        if (RetryGetEnableParam(networkId, uuid, dhId, dhType, param) != DH_FWK_SUCCESS) {
            return ret;
        }
    }
    ret = CheckSubtypeResource(param.subtype, networkId);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("CheckSubtypeResource failed, ret = %{public}d.", ret);
        return ret;
    }

    auto compEnable = std::make_shared<ComponentEnable>();
    auto result = compEnable->Enable(networkId, dhId, param, (compSource_.find(dhType))->second);
    if (result != DH_FWK_SUCCESS) {
        for (int32_t retryCount = 0; retryCount < ENABLE_RETRY_MAX_TIMES; retryCount++) {
            if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
                DHLOGE("device is already offline, no need try enable, uuid= %{public}s", GetAnonyString(uuid).c_str());
                return result;
            }
            if (compEnable->Enable(networkId, dhId, param, (compSource_.find(dhType))->second) == DH_FWK_SUCCESS) {
                DHLOGE("enable success, retryCount = %{public}d", retryCount);
                EnabledCompsDump::GetInstance().DumpEnabledComp(networkId, dhType, dhId);
                return DH_FWK_SUCCESS;
            }
            DHLOGE("enable failed, retryCount = %{public}d", retryCount);
        }
        return result;
    }
    DHLOGI("enable result is %{public}d, uuid = %{public}s, dhId = %{public}s", result, GetAnonyString(uuid).c_str(),
        GetAnonyString(dhId).c_str());
    EnabledCompsDump::GetInstance().DumpEnabledComp(networkId, dhType, dhId);

    return result;
}

int32_t ComponentManager::CheckSubtypeResource(const std::string &subtype, const std::string &networkId)
{
#ifdef DHARDWARE_CHECK_RESOURCE
    std::map<std::string, bool> resourceDesc = ComponentLoader::GetInstance().GetCompResourceDesc();
    if (resourceDesc.find(subtype) == resourceDesc.end()) {
        DHLOGE("GetCompResourceDesc failed, subtype: %{public}s", subtype.c_str());
        return ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY;
    }
    if (resourceDesc[subtype] && !IsIdenticalAccount(networkId)) {
        DHLOGE("Privacy resources must be logged in with the same account.");
        return ERR_DH_FWK_COMPONENT_ENABLE_FAILED;
    }
#endif
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::RetryGetEnableParam(const std::string &networkId, const std::string &uuid,
    const std::string &dhId, const DHType dhType, EnableParam &param)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    for (int32_t retryCount = 0; retryCount < ENABLE_RETRY_MAX_TIMES; retryCount++) {
        if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
            DHLOGE("device is already offline, no need try GetEnableParam, uuid = %{public}s",
                GetAnonyString(uuid).c_str());
            return ERR_DH_FWK_COMPONENT_ENABLE_FAILED;
        }
        if (GetEnableParam(networkId, uuid, dhId, dhType, param) == DH_FWK_SUCCESS) {
            DHLOGE("GetEnableParam success, retryCount = %{public}d", retryCount);
            break;
        }
        DHLOGE("GetEnableParam failed, retryCount = %{public}d", retryCount);
        usleep(ENABLE_PARAM_RETRY_TIME);
    }
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::Disable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    auto find = compSource_.find(dhType);
    if (find == compSource_.end()) {
        DHLOGE("can not find handler for dhId = %{public}s.", GetAnonyString(dhId).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    
    auto compDisable = std::make_shared<ComponentDisable>();
    auto result = compDisable->Disable(networkId, dhId, find->second);
    if (result != DH_FWK_SUCCESS) {
        for (int32_t retryCount = 0; retryCount < DISABLE_RETRY_MAX_TIMES; retryCount++) {
            if (DHContext::GetInstance().IsDeviceOnline(uuid)) {
                DHLOGE("device is already online, no need try disable, uuid = %{public}s",
                    GetAnonyString(uuid).c_str());
                return result;
            }
            if (compDisable->Disable(networkId, dhId, find->second) == DH_FWK_SUCCESS) {
                DHLOGE("disable success, retryCount = %{public}d", retryCount);
                EnabledCompsDump::GetInstance().DumpDisabledComp(networkId, dhType, dhId);
                return DH_FWK_SUCCESS;
            }
            DHLOGE("disable failed, retryCount = %{public}d", retryCount);
        }
        return result;
    }
    DHLOGI("disable result is %{public}d, uuid = %{public}s, dhId = %{public}s", result, GetAnonyString(uuid).c_str(),
        GetAnonyString(dhId).c_str());
    EnabledCompsDump::GetInstance().DumpDisabledComp(networkId, dhType, dhId);

    return result;
}

DHType ComponentManager::GetDHType(const std::string &uuid, const std::string &dhId) const
{
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(GetDeviceIdByUUID(uuid), dhId, capability);
    if ((ret == DH_FWK_SUCCESS) && (capability != nullptr)) {
        return capability->GetDHType();
    }
    DHLOGE("get dhType failed, uuid = %{public}s, dhId = %{public}s", GetAnonyString(uuid).c_str(),
        GetAnonyString(dhId).c_str());
    return DHType::UNKNOWN;
}

int32_t ComponentManager::GetEnableCapParam(const std::string &networkId, const std::string &uuid,
    DHType dhType, EnableParam &param, std::shared_ptr<CapabilityInfo> capability)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DeviceInfo sourceDeviceInfo = GetLocalDeviceInfo();
    std::vector<std::shared_ptr<CapabilityInfo>> sourceCapInfos;
    std::string sourceDHId;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(sourceDeviceInfo.deviceId, sourceCapInfos);
    for (const auto &capInfo : sourceCapInfos) {
        if (dhType == capInfo->GetDHType()) {
            param.sourceAttrs = capInfo->GetDHAttrs();
            sourceDHId = capInfo->GetDHId();
        }
    }
    std::string sourceVersion("");
    auto ret = GetVersion(sourceDeviceInfo.uuid, dhType, sourceVersion, false);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get source version failed.");
        return ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED;
    }
    param.sourceVersion = sourceVersion;

    param.sinkAttrs = capability->GetDHAttrs();
    std::string sinkVersion("");
    ret = GetVersion(uuid, dhType, sinkVersion, true);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get sink version failed.");
        // If Version DB not sync, try get sink version from meta info
        std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
        ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(DHContext::GetInstance().GetUdidHashIdByUUID(uuid),
            capability->GetDHId(), metaCapPtr);
        if ((ret == DH_FWK_SUCCESS) && (metaCapPtr != nullptr)) {
            sinkVersion = metaCapPtr->GetSinkVersion();
        } else {
            return ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED;
        }
    }
    param.sinkVersion = sinkVersion;
    param.subtype = capability->GetDHSubtype();
    DHLOGI("GetEnableCapParam success. dhType = %{public}#X, sink uuid =%{public}s,"
        "sinVersion = %{public}s, source uuid =%{public}s, source dhId = %{public}s, sourceVersion = %{public}s,"
        "subtype = %{public}s", dhType, GetAnonyString(uuid).c_str(),
        param.sinkVersion.c_str(), GetAnonyString(sourceDeviceInfo.uuid).c_str(), GetAnonyString(sourceDHId).c_str(),
        param.sourceVersion.c_str(), param.subtype.c_str());
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetEnableMetaParam(const std::string &networkId, const std::string &uuid,
    DHType dhType, EnableParam &param, std::shared_ptr<MetaCapabilityInfo> metaCapPtr)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DeviceInfo sourceDeviceInfo = GetLocalDeviceInfo();
    std::vector<std::shared_ptr<MetaCapabilityInfo>> sourceMetaInfos;
    std::string sourceDHId;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(sourceDeviceInfo.udidHash, sourceMetaInfos);
    for (const auto &metaInfo : sourceMetaInfos) {
        if (dhType == metaInfo->GetDHType()) {
            param.sourceAttrs = metaInfo->GetDHAttrs();
            sourceDHId = metaInfo->GetDHId();
        }
    }
    std::string sourceVersion("");
    auto ret = GetVersion(sourceDeviceInfo.uuid, dhType, sourceVersion, false);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get source version failed.");
        return ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED;
    }
    param.sourceVersion = sourceVersion;

    param.sinkAttrs = metaCapPtr->GetDHAttrs();
    param.sinkVersion = metaCapPtr->GetSinkVersion();
    param.subtype = metaCapPtr->GetDHSubtype();
    DHLOGI("GetEnableCapParam success. dhType = %{public}#X, sink uuid =%{public}s,"
        "sinVersion = %{public}s, source uuid =%{public}s, source dhId = %{public}s, sourceVersion = %{public}s,"
        "subtype = %{public}s", dhType, GetAnonyString(uuid).c_str(),
        param.sinkVersion.c_str(), GetAnonyString(sourceDeviceInfo.uuid).c_str(), GetAnonyString(sourceDHId).c_str(),
        param.sourceVersion.c_str(), param.subtype.c_str());
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetCapParam(const std::string &uuid, const std::string &dhId,
    std::shared_ptr<CapabilityInfo> &capability)
{
    if (!IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::string deviceId = GetDeviceIdByUUID(uuid);
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capability);
    if ((ret == DH_FWK_SUCCESS) && (capability != nullptr)) {
        DHLOGI("GetCapability success, deviceId: %{public}s, uuid: %{public}s, dhId: %{public}s, ret: %{public}d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), ret);
        return ret;
    }

    ret = LocalCapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capability);
    if ((ret == DH_FWK_SUCCESS) && (capability != nullptr)) {
        DHLOGI("Local GetCaps success, deviceId: %{public}s, uuid: %{public}s, dhId: %{public}s, ret: %{public}d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), ret);
        return ret;
    }

    return ret;
}

int32_t ComponentManager::GetMetaParam(const std::string &uuid, const std::string &dhId,
    std::shared_ptr<MetaCapabilityInfo> &metaCapPtr)
{
    if (!IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    auto ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(DHContext::GetInstance().GetUdidHashIdByUUID(uuid),
        dhId, metaCapPtr);
    if ((ret == DH_FWK_SUCCESS) && (metaCapPtr != nullptr)) {
        DHLOGI("GetCapability success, uuid =%{public}s, dhId = %{public}s, errCode = %{public}d",
            GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), ret);
        return ret;
    }
    return ret;
}

int32_t ComponentManager::GetEnableParam(const std::string &networkId, const std::string &uuid,
    const std::string &dhId, DHType dhType, EnableParam &param)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED;
    }
    DHLOGI("GetEnableParam start, networkId= %{public}s, uuid = %{public}s, dhId = %{public}s, dhType = %{public}#X,",
        GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), dhType);
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    if (GetCapParam(uuid, dhId, capability) == DH_FWK_SUCCESS) {
        auto ret = GetEnableCapParam(networkId, uuid, dhType, param, capability);
        if (ret == DH_FWK_SUCCESS) {
            return ret;
        }
        DHLOGE("GetEnableCapParam failed.");
    }

    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    if (GetMetaParam(uuid, dhId, metaCapPtr) == DH_FWK_SUCCESS) {
        auto ret = GetEnableMetaParam(networkId, uuid, dhType, param, metaCapPtr);
        if (ret == DH_FWK_SUCCESS) {
            return ret;
        }
        DHLOGE("GetEnableMetaParam failed.");
    }
    DHLOGE("GetEnableParam is failed.");
    return ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED;
}

int32_t ComponentManager::GetVersionFromVerMgr(const std::string &uuid, const DHType dhType,
    std::string &version, bool isSink)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    CompVersion compversion;
    int32_t ret = VersionManager::GetInstance().GetCompVersion(uuid, dhType, compversion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get version Manager failed, uuid =%{public}s, dhType = %{public}#X, errCode = %{public}d",
            GetAnonyString(uuid).c_str(), dhType, ret);
        return ret;
    }
    DHLOGI("Get version mgr success, sinkVersion = %{public}s, sourceVersion = %{public}s,uuid = %{public}s, "
        "dhType = %{public}#X", compversion.sinkVersion.c_str(), compversion.sourceVersion.c_str(),
        GetAnonyString(uuid).c_str(), dhType);
    version = isSink ? compversion.sinkVersion : compversion.sourceVersion;
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetVersionFromVerInfoMgr(const std::string &uuid, const DHType dhType,
    std::string &version, bool isSink)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    VersionInfo versionInfo;
    int32_t ret =  VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(GetDeviceIdByUUID(uuid), versionInfo);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get Version info Manager failed, uuid =%{public}s, dhType = %{public}#X, errCode = %{public}d",
            GetAnonyString(uuid).c_str(), dhType, ret);
        return ret;
    }
    auto iter = versionInfo.compVersions.find(dhType);
    if (iter == versionInfo.compVersions.end()) {
        DHLOGE("can not find component version for dhType = %{public}d", dhType);
        return ERR_DH_FWK_COMPONENT_DHTYPE_NOT_FOUND;
    }
    DHLOGI("Get version info mgr success, sinkVersion = %{public}s, sourceVersion = %{public}s, uuid = %{public}s, "
        "dhType = %{public}#X", iter->second.sinkVersion.c_str(), iter->second.sourceVersion.c_str(),
        GetAnonyString(uuid).c_str(), dhType);
    UpdateVersionCache(uuid, versionInfo);
    version = isSink ? iter->second.sinkVersion : iter->second.sourceVersion;
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetVersion(const std::string &uuid, DHType dhType, std::string &version, bool isSink)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = GetVersionFromVerMgr(uuid, dhType, version, isSink);
    if ((ret == DH_FWK_SUCCESS) && (!version.empty())) {
        return DH_FWK_SUCCESS;
    }

    ret = GetVersionFromVerInfoMgr(uuid, dhType, version, isSink);
    if ((ret == DH_FWK_SUCCESS) && (!version.empty())) {
        return DH_FWK_SUCCESS;
    }

    return ret;
}

void ComponentManager::UpdateVersionCache(const std::string &uuid, const VersionInfo &versionInfo)
{
    if (!IsIdLengthValid(uuid)) {
        return;
    }
    DHVersion dhVersion;
    dhVersion.uuid = uuid;
    dhVersion.dhVersion = versionInfo.dhVersion;
    dhVersion.compVersions = versionInfo.compVersions;
    VersionManager::GetInstance().AddDHVersion(uuid, dhVersion);
}

void ComponentManager::DumpLoadedComps(std::set<DHType> &compSourceType, std::set<DHType> &compSinkType)
{
    for (auto compSource : compSource_) {
        compSourceType.emplace(compSource.first);
    }
    for (auto compSink : compSink_) {
        compSinkType.emplace(compSink.first);
    }
}

void ComponentManager::Recover(DHType dhType)
{
    ffrt::submit([this, dhType]() { this->DoRecover(dhType); });
}

void ComponentManager::DoRecover(DHType dhType)
{
    int32_t ret = pthread_setname_np(pthread_self(), DO_RECOVER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoRecover setname failed.");
    }
    // step1: restart sa process
    ReStartSA(dhType);
    // step2: recover distributed hardware virtual driver
    RecoverDistributedHardware(dhType);
}

void ComponentManager::ReStartSA(DHType dhType)
{
    DHLOGI("Restart SA for DHType %{public}" PRIu32, (uint32_t)dhType);
    auto sourceResult = StartSource(dhType);
    auto sinkResult = StartSink(dhType);

    if (!WaitForResult(Action::START_SOURCE, sourceResult)) {
        DHLOGE("ReStartSource failed, DHType: %{public}" PRIu32, (uint32_t)dhType);
    }

    if (!WaitForResult(Action::START_SINK, sinkResult)) {
        DHLOGE("ReStartSink failed, DHType: %{public}" PRIu32, (uint32_t)dhType);
    }
    DHLOGI("Finish Restart");
}

void ComponentManager::RecoverDistributedHardware(DHType dhType)
{
    MetaCapInfoMap metaInfoMap;
    MetaInfoManager::GetInstance()->GetMetaDataByDHType(dhType, metaInfoMap);
    for (const auto &metaInfo : metaInfoMap) {
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(metaInfo.second->GetDeviceId());
        if (uuid.empty()) {
            DHLOGE("Can not find uuid by capability deviceId: %{public}s",
                GetAnonyString(metaInfo.second->GetDeviceId()).c_str());
            continue;
        }

        std::string networkId = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
        if (networkId.empty()) {
            DHLOGI("Can not find network id by uuid: %{public}s", GetAnonyString(uuid).c_str());
            continue;
        }

        TaskParam taskParam = {
            .networkId = networkId,
            .uuid = uuid,
            .dhId = metaInfo.second->GetDHId(),
            .dhType = metaInfo.second->GetDHType()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
}

std::map<DHType, IDistributedHardwareSink*> ComponentManager::GetDHSinkInstance()
{
    return compSink_;
}

bool ComponentManager::IsIdenticalAccount(const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return false;
    }
    DmAuthForm authForm = DmAuthForm::INVALID_TYPE;
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        DHLOGE("DeviceList size is invalid!");
        return false;
    }
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            authForm = deviceInfo.authForm;
            break;
        }
    }
    if (authForm == DmAuthForm::IDENTICAL_ACCOUNT) {
        return true;
    }
    return false;
}

void ComponentManager::UpdateBusinessState(const std::string &networkId, const std::string &dhId, BusinessState state)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(dhId)) {
        return;
    }
    DHLOGI("UpdateBusinessState, networkId: %{public}s, dhId: %{public}s, state: %{public}" PRIu32,
        GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str(), (uint32_t)state);
    {
        std::lock_guard<std::mutex> lock(bizStateMtx_);
        dhBizStates_[{networkId, dhId}] = state;
    }

    if (state == BusinessState::IDLE) {
        TaskParam taskParam;
        if (!FetchNeedRefreshTask({networkId, dhId}, taskParam)) {
            return;
        }
        DHLOGI("The dh need refresh, networkId: %{public}s, dhId: %{public}s",
            GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
}

IDistributedHardwareSource* ComponentManager::GetDHSourceInstance(DHType dhType)
{
    if (compSource_.find(dhType) == compSource_.end()) {
        DHLOGE("can not find handler for dhType = %{public}d.", dhType);
        return nullptr;
    }
    return compSource_[dhType];
}

BusinessState ComponentManager::QueryBusinessState(const std::string &uuid, const std::string &dhId)
{
    if (!IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return BusinessState::UNKNOWN;
    }
    std::lock_guard<std::mutex> lock(bizStateMtx_);
    std::pair<std::string, std::string> key = {uuid, dhId};
    if (dhBizStates_.find(key) == dhBizStates_.end()) {
        return BusinessState::UNKNOWN;
    }

    return dhBizStates_.at(key);
}

void ComponentManager::TriggerFullCapsSync(const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return;
    }
    if (dhCommToolPtr_ == nullptr) {
        DHLOGE("DH communication tool ptr is null");
        return;
    }
    dhCommToolPtr_->TriggerReqFullDHCaps(networkId);
}

void ComponentManager::SaveNeedRefreshTask(const TaskParam &taskParam)
{
    std::lock_guard<std::mutex> lock(needRefreshTaskParamsMtx_);
    needRefreshTaskParams_[{taskParam.networkId, taskParam.dhId}] = taskParam;
}

bool ComponentManager::FetchNeedRefreshTask(const std::pair<std::string, std::string> &taskKey, TaskParam &taskParam)
{
    std::lock_guard<std::mutex> lock(needRefreshTaskParamsMtx_);
    if (needRefreshTaskParams_.find(taskKey) == needRefreshTaskParams_.end()) {
        return false;
    }

    taskParam = needRefreshTaskParams_.at(taskKey);
    needRefreshTaskParams_.erase(taskKey);
    return true;
}

ComponentManager::ComponentManagerEventHandler::ComponentManagerEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> runner) : AppExecFwk::EventHandler(runner)
{
    DHLOGI("Ctor ComponentManagerEventHandler");
}

void ComponentManager::ComponentManagerEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        DHLOGE("event is nullptr");
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case EVENT_DATA_SYNC_MANUAL: {
            // do muanul sync with remote
            auto sharedObjPtr = event->GetSharedObject<std::string>();
            if (sharedObjPtr == nullptr) {
                DHLOGE("The data sync param invalid!");
                break;
            }
            std::string networkId = *sharedObjPtr;
            DHLOGI("Try receive full capabiliy info from networkId: %{public}s", GetAnonyString(networkId).c_str());
            if (networkId.empty()) {
                DHLOGE("Can not get device uuid by networkId: %{public}s", GetAnonyString(networkId).c_str());
                break;
            }
            ComponentManager::GetInstance().TriggerFullCapsSync(networkId);
            break;
        }
        default:
            DHLOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}

std::shared_ptr<ComponentManager::ComponentManagerEventHandler> ComponentManager::GetEventHandler()
{
    return this->eventHandler_;
}

} // namespace DistributedHardware
} // namespace OHOS
