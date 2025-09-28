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
#include "dm_freeze_process.h"

#include "cJSON.h"
#include "datetime_ex.h"
#include "dm_anonymous.h"
#include "dm_device_info.h"
#include "dm_error_type.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr const char* BIND_FAILED_EVENTS_KEY = "bindFailedEvents";
constexpr const char* FREEZE_STATE_KEY = "freezeState";
constexpr const char* START_FREEZE_TIME_KEY = "startFreezeTimeStamp";
constexpr const char* STOP_FREEZE_TIME_KEY = "stopFreezeTimeStamp";
constexpr const char* FAILED_TIMES_STAMPS_KEY = "failedTimeStamps";
constexpr const char* FREEZE_TIMES_STAMPS_KEY = "freezeTimeStamps";
constexpr const char* IS_NEED_FREEZE = "IsNeedFreeze";
constexpr const char* NEED_FREEZE = "0";
constexpr const char* NO_NEED_FREEZE = "1";
constexpr int32_t MAX_CONTINUOUS_BIND_FAILED_NUM = 2;
constexpr int64_t CONTINUEOUS_FAILED_INTERVAL = 6 * 60;
constexpr int64_t DATA_REFRESH_INTERVAL = 20 * 60;
constexpr int64_t NOT_FREEZE_DURATION_SEC = 0;
constexpr int64_t FIRST_FREEZE_DURATION_SEC = 60;
constexpr int64_t SECOND_FREEZE_DURATION_SEC = 3 * 60;
constexpr int64_t THIRD_FREEZE_DURATION_SEC = 5 * 60;
constexpr int64_t MAX_FREEZE_DURATION_SEC = 10 * 60;
}

DM_IMPLEMENT_SINGLE_INSTANCE(FreezeProcess);

int32_t FreezeProcess::SyncFreezeData()
{
    LOGI("called");
    std::string freezeStatesValue;
    int32_t ret = KVAdapterManager::GetInstance().GetFreezeData(FREEZE_STATE_KEY, freezeStatesValue);
    if (ret != DM_OK) {
        LOGE("Get freeze states data failed, ret: %{public}d", ret);
        return ret;
    }
    DeviceFreezeState freezeStateObj;
    ret = ConvertJsonToDeviceFreezeState(freezeStatesValue, freezeStateObj);
    if (ret != DM_OK) {
        LOGE("ConvertJsonToDeviceFreezeState, ret: %{public}d", ret);
        return ret;
    }
    {
        std::lock_guard<ffrt::mutex> lock(freezeStateCacheMtx_);
        freezeStateCache_ = freezeStateObj;
    }
    std::string bindFailedEventsValue;
    ret = KVAdapterManager::GetInstance().GetFreezeData(BIND_FAILED_EVENTS_KEY, bindFailedEventsValue);
    if (ret != DM_OK) {
        LOGE("Get bind failed events data failed, ret: %{public}d", ret);
        return ret;
    }
    BindFailedEvents bindFailedEventsObj;
    ret = ConvertJsonToBindFailedEvents(bindFailedEventsValue, bindFailedEventsObj);
    if (ret != DM_OK) {
        LOGE("ConvertJsonToBindFailedEvents, ret: %{public}d", ret);
        return ret;
    }
    std::lock_guard<ffrt::mutex> lock(bindFailedEventsCacheMtx_);
    bindFailedEventsCache_ = bindFailedEventsObj;
    LOGI("Sync freeze data success");
    return DM_OK;
}

int32_t FreezeProcess::ConvertJsonToDeviceFreezeState(const std::string &result, DeviceFreezeState &freezeStateObj)
{
    if (result.empty()) {
        LOGE("result is empty");
        return ERR_DM_FAILED;
    }
    JsonObject resultJson(result);
    if (resultJson.IsDiscarded()) {
        LOGE("resultJson parse failed");
        return ERR_DM_FAILED;
    }
    if (IsInt64(resultJson, START_FREEZE_TIME_KEY)) {
        freezeStateObj.startFreezeTimeStamp = resultJson[START_FREEZE_TIME_KEY].Get<int64_t>();
    }
    if (IsInt64(resultJson, STOP_FREEZE_TIME_KEY)) {
        freezeStateObj.stopFreezeTimeStamp = resultJson[STOP_FREEZE_TIME_KEY].Get<int64_t>();
    }
    LOGI("ConvertJsonToDeviceFreezeState success");
    return DM_OK;
}

int32_t FreezeProcess::ConvertJsonToBindFailedEvents(const std::string &result, BindFailedEvents &bindFailedEventsObj)
{
    if (result.empty()) {
        LOGE("result is empty");
        return ERR_DM_FAILED;
    }
    JsonObject resultJson(result);
    if (resultJson.IsDiscarded()) {
        LOGE("resultJson parse failed");
        return ERR_DM_FAILED;
    }
    if (IsArray(resultJson, FAILED_TIMES_STAMPS_KEY)) {
        std::vector<int64_t> failedTimeStampsTmp;
        resultJson[FAILED_TIMES_STAMPS_KEY].Get(failedTimeStampsTmp);
        bindFailedEventsObj.failedTimeStamps = failedTimeStampsTmp;
    }
    if (IsArray(resultJson, FREEZE_TIMES_STAMPS_KEY)) {
        std::vector<int64_t> freezeTimeStampsTmp;
        resultJson[FREEZE_TIMES_STAMPS_KEY].Get(freezeTimeStampsTmp);
        bindFailedEventsObj.freezeTimeStamps = freezeTimeStampsTmp;
    }
    LOGI("ConvertJsonToBindFailedEvents success");
    return DM_OK;
}

bool FreezeProcess::IsFrozen(int64_t &remainingFrozenTime)
{
    {
        std::lock_guard<ffrt::mutex> lock(isSyncedMtx_);
        if (!isSynced_) {
            SyncFreezeData();
            isSynced_ = true;
        }
    }
    int64_t stopFreezeTimeStamp = 0;
    {
        std::lock_guard<ffrt::mutex> lock(freezeStateCacheMtx_);
        if (bindFailedEventsCache_.IsEmpty()) {
            LOGI("bindFailedEventsCache is empty");
            return false;
        }
        stopFreezeTimeStamp = freezeStateCache_.stopFreezeTimeStamp;
    }
    int64_t nowTime = GetSecondsSince1970ToNow();
    bool isFrozen = nowTime > stopFreezeTimeStamp ? false : true;
    if (isFrozen) {
        remainingFrozenTime = stopFreezeTimeStamp - nowTime;
    }
    if (CleanFreezeRecord(nowTime) != DM_OK) {
        LOGE("CleanFreezeRecord failed");
    }
    return isFrozen;
}

int32_t FreezeProcess::CleanFreezeRecord(int64_t nowTime)
{
    int64_t reservedDataTimeStamp = nowTime - DATA_REFRESH_INTERVAL;
    if (CleanBindFailedEvents(reservedDataTimeStamp) != DM_OK) {
        LOGE("CleanBindFailedEvents failed");
        return ERR_DM_FAILED;
    }
    if (CleanFreezeState(reservedDataTimeStamp) != DM_OK) {
        LOGE("CleanFreezeState failed");
        return ERR_DM_FAILED;
    }
    LOGI("CleanFreezeRecord success");
    return DM_OK;
}

int32_t FreezeProcess::CleanBindFailedEvents(int64_t reservedDataTimeStamp)
{
    std::lock_guard<ffrt::mutex> lock(bindFailedEventsCacheMtx_);
    if (bindFailedEventsCache_.IsEmpty()) {
        LOGI("bindFailedEventsCache is empty, no need to clean");
        return DM_OK;
    }
    BindFailedEvents bindFailedEventsTmp = bindFailedEventsCache_;
    int32_t needCleanBindFailedStampNum = std::count_if(bindFailedEventsTmp.failedTimeStamps.begin(),
        bindFailedEventsTmp.failedTimeStamps.end(),
        [reservedDataTimeStamp](int64_t v) { return v < reservedDataTimeStamp; });
    if (needCleanBindFailedStampNum == 0) {
        LOGI("no stamp is before 20mins, no need to clean");
        return DM_OK;
    }
    bindFailedEventsTmp.failedTimeStamps.erase(
        std::remove_if(bindFailedEventsTmp.failedTimeStamps.begin(),
            bindFailedEventsTmp.failedTimeStamps.end(),
            [reservedDataTimeStamp](int64_t n) { return n < reservedDataTimeStamp; }),
        bindFailedEventsTmp.failedTimeStamps.end());
    bindFailedEventsTmp.freezeTimeStamps.erase(
        std::remove_if(bindFailedEventsTmp.freezeTimeStamps.begin(),
            bindFailedEventsTmp.freezeTimeStamps.end(),
            [reservedDataTimeStamp](int64_t n) { return n < reservedDataTimeStamp; }),
        bindFailedEventsTmp.freezeTimeStamps.end());
    std::string valueStr = "";
    ConvertBindFailedEventsToJson(bindFailedEventsTmp, valueStr);
    if (KVAdapterManager::GetInstance().PutFreezeData(BIND_FAILED_EVENTS_KEY, valueStr) != DM_OK) {
        LOGE("CleanBindFailedEvents within 20mins failed");
        return ERR_DM_FAILED;
    }
    bindFailedEventsCache_ = bindFailedEventsTmp;
    LOGI("CleanBindFailedEvents success");
    return DM_OK;
}

int32_t FreezeProcess::CleanFreezeState(int64_t reservedDataTimeStamp)
{
    std::lock_guard<ffrt::mutex> lock(freezeStateCacheMtx_);
    if (freezeStateCache_.IsEmpty()) {
        LOGI("freezeStateCache is empty, no need to clean");
        return DM_OK;
    }
    if (freezeStateCache_.startFreezeTimeStamp >= reservedDataTimeStamp) {
        LOGI("startFreezeTimeStamp is in 20 mins, no need to clean");
        return DM_OK;
    }
    if (KVAdapterManager::GetInstance().DeleteFreezeData(FREEZE_STATE_KEY) != DM_OK) {
        LOGE("delete freeze states data within 20mins failed");
        return ERR_DM_FAILED;
    }
    freezeStateCache_.Reset();
    LOGI("CleanFreezeState success");
    return DM_OK;
}

void FreezeProcess::ConvertBindFailedEventsToJson(const BindFailedEvents &value, std::string &result)
{
    JsonObject jsonObj;
    jsonObj[FAILED_TIMES_STAMPS_KEY] = value.failedTimeStamps;
    jsonObj[FREEZE_TIMES_STAMPS_KEY] = value.freezeTimeStamps;
    result = jsonObj.Dump();
}

void FreezeProcess::ConvertDeviceFreezeStateToJson(const DeviceFreezeState &value, std::string &result)
{
    JsonObject jsonObj;
    jsonObj[START_FREEZE_TIME_KEY] = value.startFreezeTimeStamp;
    jsonObj[STOP_FREEZE_TIME_KEY] = value.stopFreezeTimeStamp;
    result = jsonObj.Dump();
}

int32_t FreezeProcess::DeleteFreezeRecord()
{
    if (KVAdapterManager::GetInstance().DeleteFreezeData(FREEZE_STATE_KEY) != DM_OK) {
        LOGE("delete freezeStates data failed");
        return ERR_DM_FAILED;
    }
    {
        std::lock_guard<ffrt::mutex> lock(freezeStateCacheMtx_);
        freezeStateCache_.Reset();
    }
    if (KVAdapterManager::GetInstance().DeleteFreezeData(BIND_FAILED_EVENTS_KEY) != DM_OK) {
        LOGE("delete bindFailedEvents data failed");
        return ERR_DM_FAILED;
    }
    std::lock_guard<ffrt::mutex> lock(bindFailedEventsCacheMtx_);
    bindFailedEventsCache_.Reset();
    return DM_OK;
}

int32_t FreezeProcess::UpdateFreezeRecord()
{
    int64_t nowTime = GetSecondsSince1970ToNow();
    std::lock_guard<ffrt::mutex> lock(bindFailedEventsCacheMtx_);
    BindFailedEvents bindFailedEventsTmp = bindFailedEventsCache_;
    int64_t lastFreezeTimeStamps = 0;
    if (!bindFailedEventsTmp.freezeTimeStamps.empty()) {
        lastFreezeTimeStamps = bindFailedEventsTmp.freezeTimeStamps.back();
    }
    int32_t continueBindFailedNum = std::count_if(bindFailedEventsTmp.failedTimeStamps.begin(),
        bindFailedEventsTmp.failedTimeStamps.end(),
        [nowTime, lastFreezeTimeStamps](
            int64_t v) { return v > nowTime - CONTINUEOUS_FAILED_INTERVAL && v > lastFreezeTimeStamps; });
    if (continueBindFailedNum < MAX_CONTINUOUS_BIND_FAILED_NUM) {
        bindFailedEventsTmp.failedTimeStamps.push_back(nowTime);
        std::string bindFailedEventsStr = "";
        ConvertBindFailedEventsToJson(bindFailedEventsTmp, bindFailedEventsStr);
        if (KVAdapterManager::GetInstance().PutFreezeData(BIND_FAILED_EVENTS_KEY, bindFailedEventsStr) != DM_OK) {
            LOGE("UpdateBindFailedEvents failed");
            return ERR_DM_FAILED;
        }
        bindFailedEventsCache_ = bindFailedEventsTmp;
        return DM_OK;
    }
    bindFailedEventsTmp.failedTimeStamps.push_back(nowTime);
    bindFailedEventsTmp.freezeTimeStamps.push_back(nowTime);
    std::string bindFailedEventsStr = "";
    ConvertBindFailedEventsToJson(bindFailedEventsTmp, bindFailedEventsStr);
    if (KVAdapterManager::GetInstance().PutFreezeData(BIND_FAILED_EVENTS_KEY, bindFailedEventsStr) != DM_OK) {
        LOGE("UpdateBindFailedEvents failed");
        return ERR_DM_FAILED;
    }
    bindFailedEventsCache_ = bindFailedEventsTmp;
    return UpdateFreezeState(nowTime);
}

int32_t FreezeProcess::UpdateFreezeState(int64_t nowTime)
{
    std::lock_guard<ffrt::mutex> lock(freezeStateCacheMtx_);
    DeviceFreezeState freezeStateTmp = freezeStateCache_;
    int64_t nextFreezeTime = 0;
    CalculateNextFreezeTime(freezeStateTmp.stopFreezeTimeStamp - freezeStateTmp.startFreezeTimeStamp, nextFreezeTime);
    freezeStateTmp.startFreezeTimeStamp = nowTime;
    freezeStateTmp.stopFreezeTimeStamp = nowTime + nextFreezeTime;
    std::string freezeStateStr = "";
    ConvertDeviceFreezeStateToJson(freezeStateTmp, freezeStateStr);
    if (KVAdapterManager::GetInstance().PutFreezeData(FREEZE_STATE_KEY, freezeStateStr) != DM_OK) {
        LOGE("UpdateFreezeState failed");
        return ERR_DM_FAILED;
    }
    freezeStateCache_ = freezeStateTmp;
    return DM_OK;
}

void FreezeProcess::CalculateNextFreezeTime(int64_t nowFreezeTime, int64_t &nextFreezeTime)
{
    switch (nowFreezeTime) {
        case NOT_FREEZE_DURATION_SEC:
            nextFreezeTime = FIRST_FREEZE_DURATION_SEC;
            break;
        case FIRST_FREEZE_DURATION_SEC:
            nextFreezeTime = SECOND_FREEZE_DURATION_SEC;
            break;
        case SECOND_FREEZE_DURATION_SEC:
            nextFreezeTime = THIRD_FREEZE_DURATION_SEC;
            break;
        default:
            nextFreezeTime = MAX_FREEZE_DURATION_SEC;
            break;
    }
}

bool FreezeProcess::IsNeedFreeze(std::shared_ptr<DmAuthContext> context)
{
    LOGI("called");
    bool isNeedFreeze = true;
    if (context == nullptr) {
        LOGE("context is null");
        return isNeedFreeze;
    }
    OHOS::DistributedDeviceProfile::LocalServiceInfo srvInfo;
    auto ret = DeviceProfileConnector::GetInstance().GetLocalServiceInfoByBundleNameAndPinExchangeType(
        context->accessee.pkgName, context->authType, srvInfo);
    if (ret != DM_OK) {
        LOGE("ServiceInfo not found");
        return isNeedFreeze;
    }
    std::string extraInfo = srvInfo.GetExtraInfo();
    if (extraInfo.empty()) {
        LOGE("no extraInfo");
        return isNeedFreeze;
    }
    JsonObject jsonObj;
    jsonObj.Parse(extraInfo);
    if (jsonObj.IsDiscarded()) {
        LOGE("extraInfo invalid");
        return isNeedFreeze;
    }
    std::string isNeedFreezeStr;
    if (IsString(jsonObj, IS_NEED_FREEZE)) {
        isNeedFreezeStr = jsonObj[IS_NEED_FREEZE].Get<std::string>();
        LOGI("isNeedFreezeStr: %{public}s.", isNeedFreezeStr.c_str());
    }
    if (isNeedFreezeStr == NEED_FREEZE || isNeedFreezeStr == NO_NEED_FREEZE) {
        isNeedFreeze = std::atoi(isNeedFreezeStr.c_str());
    }
    LOGI("isNeedFreeze: %{public}d.", isNeedFreeze);
    return isNeedFreeze;
}
} // namespace DistributedHardware
} // namespace OHOS