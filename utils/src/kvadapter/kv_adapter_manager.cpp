/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "kv_adapter_manager.h"

#include <mutex>
#include <unistd.h>

#include "datetime_ex.h"
#include "string_ex.h"

#include "dm_anonymous.h"
#include "dm_error_type.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr const char* DM_KV_STORE_PREFIX = "DM2_";
constexpr const char* DB_KEY_DELIMITER = "###";
constexpr int64_t DM_KV_STORE_REFRESH_TIME = 24 * 60 * 60; // one day
constexpr int64_t MAX_SUPPORTED_EXIST_TIME = 3 * 24 * 60 * 60; // 3days
}

DM_IMPLEMENT_SINGLE_INSTANCE(KVAdapterManager);

DM_EXPORT int32_t KVAdapterManager::Init()
{
    LOGI("Init Kv-Adapter manager");
    {
        std::lock_guard<std::mutex> lock(idCacheMapMtx_);
        idCacheMap_.clear();
    }
    kvAdapter_ = std::make_shared<KVAdapter>();
    return kvAdapter_->Init();
}

DM_EXPORT void KVAdapterManager::UnInit()
{
    LOGI("Uninit Kv-Adapter manager");
    CHECK_NULL_VOID(kvAdapter_);
    kvAdapter_->UnInit();
    kvAdapter_ = nullptr;
}

DM_EXPORT void KVAdapterManager::ReInit()
{
    LOGI("Re init kv adapter");
    CHECK_NULL_VOID(kvAdapter_);
    kvAdapter_->ReInit();
}

int32_t KVAdapterManager::PutByAnoyDeviceId(const std::string &key, const DmKVValue &value)
{
    std::string dmKey = DM_KV_STORE_PREFIX + key;
    std::lock_guard<std::mutex> lock(idCacheMapMtx_);
    auto idIter = idCacheMap_.find(dmKey);
    if (idIter != idCacheMap_.end() && !IsTimeOut(idIter->second.lastModifyTime, value.lastModifyTime,
        DM_KV_STORE_REFRESH_TIME)) {
        LOGD("Kv value is existed");
        return DM_OK;
    }
    idCacheMap_[dmKey] = value;
    std::string prefixKey = DM_KV_STORE_PREFIX + value.appID + DB_KEY_DELIMITER + value.udidHash;
    idCacheMap_[prefixKey] = value;
    std::string valueStr = "";
    ConvertDmKVValueToJson(value, valueStr);
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Put(dmKey, valueStr) != DM_OK) {
        LOGE("Insert value to DB for dmKey failed");
        return ERR_DM_FAILED;
    }
    if (kvAdapter_->Put(prefixKey, valueStr) != DM_OK) {
        LOGE("Insert value to DB for prefixKey failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::Get(const std::string &key, DmKVValue &value)
{
    std::string dmKey = DM_KV_STORE_PREFIX + key;
    std::lock_guard<std::mutex> lock(idCacheMapMtx_);
    auto idIter = idCacheMap_.find(dmKey);
    if (idIter != idCacheMap_.end()) {
        value = idIter->second;
        return DM_OK;
    }
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    std::string valueStr;
    if (kvAdapter_->Get(dmKey, valueStr) != DM_OK) {
        LOGE("Get kv value failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    ConvertJsonToDmKVValue(valueStr, value);
    idCacheMap_[dmKey] = value;
    std::string prefixKey = DM_KV_STORE_PREFIX + value.appID + DB_KEY_DELIMITER + value.udidHash;
    idCacheMap_[prefixKey] = value;
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::DeleteAgedEntry()
{
    int64_t nowTime = GetSecondsSince1970ToNow();
    std::lock_guard<std::mutex> lock(idCacheMapMtx_);
    for (auto it = idCacheMap_.begin(); it != idCacheMap_.end();) {
        if (IsTimeOut(it->second.lastModifyTime, nowTime, MAX_SUPPORTED_EXIST_TIME)) {
            it = idCacheMap_.erase(it);
        } else {
            ++it;
        }
    }
    return DM_OK;
}

inline bool KVAdapterManager::IsTimeOut(int64_t sourceTime, int64_t targetTime, int64_t timeOut)
{
    return targetTime - sourceTime >= timeOut ? true : false;
}

DM_EXPORT int32_t KVAdapterManager::AppUnintall(const std::string &appId)
{
    LOGI("appId %{public}s.", GetAnonyString(appId).c_str());
    std::lock_guard<std::mutex> lock(idCacheMapMtx_);
    for (auto it = idCacheMap_.begin(); it != idCacheMap_.end();) {
        if (it->second.appID == appId) {
            it = idCacheMap_.erase(it);
        } else {
            ++it;
        }
    }
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->DeleteByAppId(appId, DM_KV_STORE_PREFIX) != DM_OK) {
        LOGE("DeleteByAppId failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
