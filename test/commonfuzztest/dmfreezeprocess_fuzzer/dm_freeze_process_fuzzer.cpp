/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dm_freeze_process_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "device_manager_service_listener.h"
#include "dm_freeze_process.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr const char* BIND_FAILED_EVENTS_KEY = "bindFailedEvents";
constexpr const char* FREEZE_STATE_KEY = "freezeState";
constexpr const char* START_FREEZE_TIME_KEY = "startFreezeTimeStamp";
constexpr const char* STOP_FREEZE_TIME_KEY = "stopFreezeTimeStamp";
constexpr const char* FAILED_TIMES_STAMPS_KEY = "failedTimeStamps";
constexpr const char* FREEZE_TIMES_STAMPS_KEY = "freezeTimeStamps";
constexpr const char* CAST_BUNDLE_NAME = "cast_engine_service";
constexpr int32_t MAX_CONTINUEOUS_BIND_FAILED_NUM = 2;
constexpr int64_t CONTINUEOUS_FAILED_INTERVAL = 6 * 60;
constexpr int64_t DATA_REFRESH_INTERVAL = 20 * 60;
constexpr int64_t NOT_FREEZE_TIME = 0;
constexpr int64_t FIRST_FREEZE_TIME = 60;
constexpr int64_t SECOND_FREEZE_TIME = 3 * 60;
constexpr int64_t THIRD_FREEZE_TIME = 5 * 60;
constexpr int64_t MAX_FREEZE_TIME = 10 * 60;
} // namespace

void DmFreezeProcessFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    std::string result = fdp.ConsumeRandomLengthString();
    DeviceFreezeState freezeStateObj;
    BindFailedEvents bindFailedEvents;
    std::string bundleName = CAST_BUNDLE_NAME;
    int32_t deviceType = DEVICE_TYPE_TV;
    int64_t reservedDataTimeStamp = fdp.ConsumeIntegral<int64_t>();
    FreezeProcess::GetInstance().ConvertJsonToDeviceFreezeState(result, freezeStateObj);
    FreezeProcess::GetInstance().ConvertJsonToBindFailedEvents(result, bindFailedEvents);
    bundleName = fdp.ConsumeRandomLengthString();
    deviceType = fdp.ConsumeIntegral<int32_t>();
    freezeStateObj.startFreezeTimeStamp = fdp.ConsumeIntegral<int32_t>();
    FreezeProcess::GetInstance().freezeStateCache_ = freezeStateObj;
    FreezeProcess::GetInstance().CleanBindFailedEvents(reservedDataTimeStamp);
    FreezeProcess::GetInstance().CleanFreezeState(reservedDataTimeStamp);
    FreezeProcess::GetInstance().ConvertDeviceFreezeStateToJson(freezeStateObj, result);
    int64_t nowTime = NOT_FREEZE_TIME;

    int64_t nextTime = fdp.ConsumeIntegral<int64_t>();
    FreezeProcess::GetInstance().CleanFreezeRecord(nowTime);
    FreezeProcess::GetInstance().UpdateFreezeState(nowTime);
    FreezeProcess::GetInstance().CalculateNextFreezeTime(nowTime, nextTime);
    nowTime = FIRST_FREEZE_TIME;
    FreezeProcess::GetInstance().CalculateNextFreezeTime(nowTime, nextTime);
    nowTime = SECOND_FREEZE_TIME;
    FreezeProcess::GetInstance().CalculateNextFreezeTime(nowTime, nextTime);
    nowTime = MAX_FREEZE_TIME;
    FreezeProcess::GetInstance().CalculateNextFreezeTime(nowTime, nextTime);
    FreezeProcess::GetInstance().DeleteFreezeRecord();
    FreezeProcess::GetInstance().UpdateFreezeRecord();
    FreezeProcess::GetInstance().ConvertBindFailedEventsToJson(bindFailedEvents, result);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmFreezeProcessFuzzTest(data, size);
    return 0;
}