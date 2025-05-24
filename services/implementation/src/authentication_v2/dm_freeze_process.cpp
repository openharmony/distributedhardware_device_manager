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
constexpr const char* CAST_BUNDLE_NAME = "cast_engine_service";
constexpr int32_t MAX_CONTINUEOUS_BIND_FAILED_NUM = 2;
constexpr int64_t CONTINUEOUS_FAILED_INTERVAL = 6 * 60;
constexpr int64_t DATA_REFRESH_INTERVAL = 20 * 60;
constexpr int64_t NOT_FREEZE_TIME = 0;
constexpr int64_t FIRST_FREEZE_TIME = 60;
constexpr int64_t SECOND_FREEZE_TIME = 3 * 60;
constexpr int64_t THIRD_FREEZE_TIME = 5 * 60;
constexpr int64_t MAX_FREEZE_TIME = 10 * 60;
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
    ConvertJsonToDeviceFreezeState(freezeStatesValue, freezeStateObj);
    {
        std::lock_guard<std::mutex> lock(freezeStateCacheMtx_);
        freezeStateCache_ = freezeStateObj;
    }
    std::string bindFailedEventsValue;
    ret = KVAdapterManager::GetInstance().GetFreezeData(BIND_FAILED_EVENTS_KEY, bindFailedEventsValue);
    if (ret != DM_OK) {
        LOGE("Get bind failed events data failed, ret: %{public}d", ret);
        return ERR_DM_FAILED;
    }
    BindFailedEvents bindFailedEventsObj;
    ConvertJsonToBindFailedEvents(bindFailedEventsValue, bindFailedEventsObj);
    std::lock_guard<std::mutex> lock(bindFailedEventsCacheMtx_);
    bindFailedEventsCache_ = bindFailedEventsObj;
    LOGI("Sync freeze data success");
    return DM_OK;
}

void FreezeProcess::ConvertJsonToDeviceFreezeState(const std::string &result, DeviceFreezeState &freezeStateObj)
{
    if (result.empty()) {
        LOGE("result is empty");
        return;
    }
    cJSON *root = cJSON_Parse(result.c_str());
    if (root == nullptr) {
        LOGE("cJSON_Parse failed");
        return;
    }
    cJSON *startFreezeTimeStampItem = cJSON_GetObjectItemCaseSensitive(root, START_FREEZE_TIME_KEY);
    if (startFreezeTimeStampItem != nullptr && cJSON_IsNumber(startFreezeTimeStampItem)) {
        freezeStateObj.startFreezeTimeStamp = startFreezeTimeStampItem->valueint;
    }
    cJSON *stopFreezeTimeStampItem = cJSON_GetObjectItemCaseSensitive(root, STOP_FREEZE_TIME_KEY);
    if (stopFreezeTimeStampItem != nullptr && cJSON_IsNumber(stopFreezeTimeStampItem)) {
        freezeStateObj.stopFreezeTimeStamp = stopFreezeTimeStampItem->valueint;
    }
    LOGI("ConvertJsonToDeviceFreezeState success");
    cJSON_Delete(root);
}

void FreezeProcess::ConvertJsonToBindFailedEvents(const std::string &result, BindFailedEvents &bindFailedEventsObj)
{
    if (result.empty()) {
        LOGE("result is empty");
        return;
    }
    cJSON *root = cJSON_Parse(result.c_str());
    if (root == nullptr) {
        LOGE("cJSON_Parse failed");
        return;
    }
    cJSON *failedTimeStampsJson = cJSON_GetObjectItem(root, FAILED_TIMES_STAMPS_KEY);
    if (failedTimeStampsJson != nullptr && cJSON_IsArray(failedTimeStampsJson)) {
        cJSON *item;
        cJSON_ArrayForEach(item, failedTimeStampsJson)
        {
            bindFailedEventsObj.failedTimeStamps.push_back(item->valueint);
        }
    }
    cJSON *freezeTimeStampsJson = cJSON_GetObjectItem(root, FREEZE_TIMES_STAMPS_KEY);
    if (freezeTimeStampsJson != nullptr && cJSON_IsArray(freezeTimeStampsJson)) {
        cJSON *item;
        cJSON_ArrayForEach(item, freezeTimeStampsJson)
        {
            bindFailedEventsObj.freezeTimeStamps.push_back(item->valueint);
        }
    }
    LOGI("ConvertJsonToBindFailedEvents success");
    cJSON_Delete(root);
}

bool FreezeProcess::IsFreezed(const std::string &bundleName, const int32_t &deviceType)
{
    if (bundleName == CAST_BUNDLE_NAME && deviceType == DEVICE_TYPE_TV) {
        LOGI("device is TV, business is cast+, no need freeze");
        return false;
    }
    std::lock_guard<std::mutex> lock(freezeStateCacheMtx_);
    if (bindFailedEventsCache_.IsEmpty()) {
        LOGI("bindFailedEventsCache is empty");
        return false;
    }
    int64_t nowTime = GetSecondsSince1970ToNow();
    int64_t stopFreezeTimeStamp = freezeStateCache_.stopFreezeTimeStamp;
    return nowTime > stopFreezeTimeStamp ? false : true;
}

int32_t FreezeProcess::CleanFreezeRecord(const std::string &bundleName, const int32_t &deviceType)
{
    if (bundleName == CAST_BUNDLE_NAME && deviceType == DEVICE_TYPE_TV) {
        LOGI("device is TV, business is cast+, no need clean");
        return DM_OK;
    }
    int64_t reservedDataTimeStamp = GetSecondsSince1970ToNow() - DATA_REFRESH_INTERVAL;
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

int32_t FreezeProcess::CleanBindFailedEvents(const int64_t &reservedDataTimeStamp)
{
    std::lock_guard<std::mutex> lock(bindFailedEventsCacheMtx_);
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

int32_t FreezeProcess::CleanFreezeState(const int64_t &reservedDataTimeStamp)
{
    std::lock_guard<std::mutex> lock(freezeStateCacheMtx_);
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
    result = SafetyDump(jsonObj);
}

void FreezeProcess::ConvertDeviceFreezeStateToJson(const DeviceFreezeState &value, std::string &result)
{
    JsonObject jsonObj;
    jsonObj[START_FREEZE_TIME_KEY] = value.startFreezeTimeStamp;
    jsonObj[STOP_FREEZE_TIME_KEY] = value.stopFreezeTimeStamp;
    result = SafetyDump(jsonObj);
}

int32_t FreezeProcess::DeleteFreezeRecord(const std::string &bundleName, const int32_t &deviceType)
{
    if (bundleName == CAST_BUNDLE_NAME && deviceType == DEVICE_TYPE_TV) {
        LOGI("device is TV, business is cast+, no need delete");
        return DM_OK;
    }
    if (KVAdapterManager::GetInstance().DeleteFreezeData(FREEZE_STATE_KEY) != DM_OK) {
        LOGE("delete freezeStates data failed");
        return ERR_DM_FAILED;
    }
    freezeStateCache_.Reset();
    if (KVAdapterManager::GetInstance().DeleteFreezeData(BIND_FAILED_EVENTS_KEY) != DM_OK) {
        LOGE("delete bindFailedEvents data failed");
        return ERR_DM_FAILED;
    }
    bindFailedEventsCache_.Reset();
    return DM_OK;
}

int32_t FreezeProcess::UpdateFreezeRecord(const std::string &bundleName, const int32_t &deviceType)
{
    if (bundleName == CAST_BUNDLE_NAME && deviceType == DEVICE_TYPE_TV) {
        LOGI("device is TV, business is cast+, no need update");
        return DM_OK;
    }
    int64_t nowTime = GetSecondsSince1970ToNow();
    std::lock_guard<std::mutex> lock(bindFailedEventsCacheMtx_);
    BindFailedEvents bindFailedEventsTmp = bindFailedEventsCache_;
    int64_t lastFreezeTimeStamps = 0;
    if (!bindFailedEventsTmp.freezeTimeStamps.empty()) {
        lastFreezeTimeStamps = bindFailedEventsTmp.freezeTimeStamps.back();
    }
    int32_t continueBindFailedNum = std::count_if(bindFailedEventsTmp.failedTimeStamps.begin(),
        bindFailedEventsTmp.failedTimeStamps.end(),
        [nowTime, lastFreezeTimeStamps](
            int64_t v) { return v > nowTime - CONTINUEOUS_FAILED_INTERVAL && v > lastFreezeTimeStamps; });
    if (continueBindFailedNum < MAX_CONTINUEOUS_BIND_FAILED_NUM) {
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

int32_t FreezeProcess::UpdateFreezeState(const int64_t &nowTime)
{
    std::lock_guard<std::mutex> lock(freezeStateCacheMtx_);
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

void FreezeProcess::CalculateNextFreezeTime(const int64_t &nowFreezeTime, int64_t &nextFreezeTime)
{
    switch (nowFreezeTime) {
        case NOT_FREEZE_TIME:
            nextFreezeTime = FIRST_FREEZE_TIME;
            break;
        case FIRST_FREEZE_TIME:
            nextFreezeTime = SECOND_FREEZE_TIME;
            break;
        case SECOND_FREEZE_TIME:
            nextFreezeTime = THIRD_FREEZE_TIME;
            break;
        default:
            nextFreezeTime = MAX_FREEZE_TIME;
            break;
    }
}
} // namespace DistributedHardware
} // namespace OHOS