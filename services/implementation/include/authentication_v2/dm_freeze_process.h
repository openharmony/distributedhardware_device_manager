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
#ifndef OHOS_DM_FREEZE_PROCESS_V2_H
#define OHOS_DM_FREEZE_PROCESS_V2_H
#include <map>
#include <memory>
#include <mutex>

#include "kv_adapter_manager.h"
#include "dm_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
typedef struct DeviceFreezeState {
    int64_t startFreezeTimeStamp;
    int64_t stopFreezeTimeStamp;
    explicit DeviceFreezeState() : startFreezeTimeStamp(0), stopFreezeTimeStamp(0) {}
    bool IsEmpty() const
    {
        return startFreezeTimeStamp == 0 && stopFreezeTimeStamp == 0;
    }
    void Reset()
    {
        startFreezeTimeStamp = 0;
        stopFreezeTimeStamp = 0;
    }
} DeviceFreezeState;

typedef struct BindFailedEvents {
    std::vector<int64_t> failedTimeStamps;
    std::vector<int64_t> freezeTimeStamps;
    explicit BindFailedEvents() : failedTimeStamps(), freezeTimeStamps() {}
    bool IsEmpty() const
    {
        return failedTimeStamps.empty() && freezeTimeStamps.empty();
    }
    void Reset()
    {
        failedTimeStamps.clear();
        freezeTimeStamps.clear();
    }
} BindFailedEvents;

class FreezeProcess {
DM_DECLARE_SINGLE_INSTANCE_BASE(FreezeProcess);
public:
    bool IsFrozen();
    int32_t DeleteFreezeRecord();
    int32_t UpdateFreezeRecord();
private:
    FreezeProcess() = default;
    ~FreezeProcess() = default;
    int32_t SyncFreezeData();
    int32_t ConvertJsonToDeviceFreezeState(const std::string &result, DeviceFreezeState &freezeStateObj);
    int32_t ConvertJsonToBindFailedEvents(const std::string &result, BindFailedEvents &bindFailedEvents);
    void ConvertBindFailedEventsToJson(const BindFailedEvents &value, std::string &result);
    void ConvertDeviceFreezeStateToJson(const DeviceFreezeState &value, std::string &result);
    int32_t CleanFreezeRecord(int64_t nowTime);
    int32_t CleanBindFailedEvents(int64_t reservedDataTimeStamp);
    int32_t CleanFreezeState(int64_t reservedDataTimeStamp);
    int32_t UpdateFreezeState(int64_t nowTime);
    void CalculateNextFreezeTime(int64_t nowFreezeTime, int64_t &nextFreezeTime);

private:
    bool isSynced_ = false;
    DeviceFreezeState freezeStateCache_;
    BindFailedEvents bindFailedEventsCache_;
    std::mutex freezeStateCacheMtx_;
    std::mutex bindFailedEventsCacheMtx_;
    std::mutex isSyncedMtx_;
};

} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_FREEZE_PROCESS_V2_H