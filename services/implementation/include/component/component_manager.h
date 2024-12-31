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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_H

#include <atomic>
#include <map>
#include <set>
#include <unordered_map>
#include <mutex>
#include <future>

#include "single_instance.h"
#include "component_monitor.h"
#include "capability_info.h"
#include "device_type.h"
#include "dh_comm_tool.h"
#include "event_handler.h"
#include "idistributed_hardware.h"
#include "idistributed_hardware_sink.h"
#include "idistributed_hardware_source.h"
#include "impl_utils.h"
#include "low_latency_listener.h"
#include "meta_capability_info.h"
#include "task_board.h"
#include "task_factory.h"
#include "version_info.h"
#include "component_privacy.h"

namespace OHOS {
namespace DistributedHardware {
using ActionResult = std::unordered_map<DHType, std::shared_future<int32_t>>;
class ComponentManager {
    DECLARE_SINGLE_INSTANCE_BASE(ComponentManager);

public:
    ComponentManager();
    ~ComponentManager();

public:
    int32_t Init();
    int32_t UnInit();
    int32_t Enable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        const DHType dhType);
    int32_t Disable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        const DHType dhType);
    void UpdateBusinessState(const std::string &uuid, const std::string &dhId, BusinessState state);
    BusinessState QueryBusinessState(const std::string &uuid, const std::string &dhId);
    void DumpLoadedComps(std::set<DHType> &compSourceType, std::set<DHType> &compSinkType);
    void Recover(DHType dhType);
    std::map<DHType, IDistributedHardwareSink*> GetDHSinkInstance();
    void TriggerFullCapsSync(const std::string &networkId);
    void SaveNeedRefreshTask(const TaskParam &taskParam);
    IDistributedHardwareSource* GetDHSourceInstance(DHType dhType);
    /**
     * @brief find the task param and return it.
     *        If the task param exist, get and remove from the cached task params,
     *        save it at the second task param, then return true.
     *        If the task param not exist, return false.
     *
     * @param taskKey the task param uuid and dhid pair.
     * @param taskParam if the task param exist, save it.
     * @return true if the task param exist, return true.
     * @return false if the task param not exist, return false.
     */
    bool FetchNeedRefreshTask(const std::pair<std::string, std::string> &taskKey, TaskParam &taskParam);

    class ComponentManagerEventHandler : public AppExecFwk::EventHandler {
    public:
        ComponentManagerEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner);
        ~ComponentManagerEventHandler() override = default;
        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    };
    std::shared_ptr<ComponentManager::ComponentManagerEventHandler> GetEventHandler();

private:
    enum class Action : int32_t {
        START_SOURCE,
        START_SINK,
        STOP_SOURCE,
        STOP_SINK
    };

    DHType GetDHType(const std::string &uuid, const std::string &dhId) const;
    bool InitCompSource();
    bool InitCompSink();
    ActionResult StartSource();
    ActionResult StartSource(DHType dhType);
    ActionResult StopSource();
    ActionResult StartSink();
    ActionResult StartSink(DHType dhType);
    ActionResult StopSink();
    bool WaitForResult(const Action &action, ActionResult result);
    int32_t GetEnableParam(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        DHType dhType, EnableParam &param);
    int32_t GetVersionFromVerMgr(const std::string &uuid, const DHType dhType, std::string &version, bool isSink);
    int32_t GetVersionFromVerInfoMgr(const std::string &uuid, const DHType dhType, std::string &version, bool isSink);
    int32_t GetVersion(const std::string &uuid, DHType dhType, std::string &version, bool isSink);
    void UpdateVersionCache(const std::string &uuid, const VersionInfo &versionInfo);

    void DoRecover(DHType dhType);
    void ReStartSA(DHType dhType);
    void RecoverDistributedHardware(DHType dhType);
    bool IsIdenticalAccount(const std::string &networkId);
    int32_t RetryGetEnableParam(const std::string &networkId, const std::string &uuid,
        const std::string &dhId, const DHType dhType, EnableParam &param);
    void InitComponentHandler();
    int32_t InitSAMonitor();
    void StartComponent();
    void RegisterDHStateListener();
    void RegisterDataSyncTriggerListener();
    void InitDHCommTool();

    void UnInitSAMonitor();
    void UnregisterDHStateListener();
    void UnregisterDataSyncTriggerListener();
    void UnInitDHCommTool();
    void StopComponent();
    void StopPrivacy();
    int32_t GetEnableCapParam(const std::string &networkId, const std::string &uuid, DHType dhType, EnableParam &param,
        std::shared_ptr<CapabilityInfo> capability);
    int32_t GetEnableMetaParam(const std::string &networkId, const std::string &uuid, DHType dhType, EnableParam &param,
        std::shared_ptr<MetaCapabilityInfo> metaCapPtr);
    int32_t GetCapParam(const std::string &uuid, const std::string &dhId, std::shared_ptr<CapabilityInfo> &capability);
    int32_t GetMetaParam(const std::string &uuid, const std::string &dhId,
        std::shared_ptr<MetaCapabilityInfo> &metaCapPtr);
    int32_t CheckSubtypeResource(const std::string &subtype, const std::string &networkId);

private:
    std::map<DHType, IDistributedHardwareSource*> compSource_;
    std::map<DHType, IDistributedHardwareSink*> compSink_;
    std::map<DHType, int32_t> compSrcSaId_;
    std::shared_ptr<ComponentPrivacy> audioCompPrivacy_ = nullptr;
    std::shared_ptr<ComponentPrivacy> cameraCompPrivacy_ = nullptr;
    std::shared_ptr<ComponentMonitor> compMonitorPtr_ = nullptr;
    sptr<LowLatencyListener> lowLatencyListener_ = nullptr;

    std::atomic<bool> isUnInitTimeOut_;
    // record the remote device business state, {{deviceUUID, dhId}, BusinessState}.
    std::map<std::pair<std::string, std::string>, BusinessState> dhBizStates_;
    std::mutex bizStateMtx_;
    std::shared_ptr<DistributedHardwareStateListener> dhStateListener_;
    std::shared_ptr<DataSyncTriggerListener> dataSyncTriggerListener_;

    std::shared_ptr<ComponentManager::ComponentManagerEventHandler> eventHandler_;
    std::shared_ptr<DHCommTool> dhCommToolPtr_;

    // save those remote dh that need refresh by full capability, {{device networkId, dhId}, TaskParam}.
    std::map<std::pair<std::string, std::string>, TaskParam> needRefreshTaskParams_;
    std::mutex needRefreshTaskParamsMtx_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
