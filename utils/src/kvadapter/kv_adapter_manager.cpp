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
#include "dm_constants.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string DM_KV_STORE_PREFIX = "DM_";
constexpr int64_t DM_KV_STORE_REFRESH_TIME = 24 * 60 * 60; // one day
constexpr int64_t MAX_SUPPORTED_EXIST_TIME = 3 * 24 * 60 * 60; // 3days
}

DM_IMPLEMENT_SINGLE_INSTANCE(KVAdapterManager);

int32_t KVAdapterManager::Init()
{
    LOGI("Init Kv-Adapter manager");
    {
        std::lock_guard<std::mutex> lock(idCacheMapMtx_);
        idCacheMap_.clear();
    }
    kvAdapter_ = std::make_shared<KVAdapter>();
    return kvAdapter_->Init();
}

void KVAdapterManager::UnInit()
{
    LOGI("Uninit Kv-Adapter manager");
    CHECK_NULL_VOID(kvAdapter_);
    kvAdapter_->UnInit();
    kvAdapter_ = nullptr;
}

void KVAdapterManager::ReInit()
{
    LOGI("Re init kv adapter");
    CHECK_NULL_VOID(kvAdapter_);
    kvAdapter_->ReInit();
}

int32_t KVAdapterManager::Put(const std::string& key, const DmKVValue& value)
{
    std::string dmKey = AddPrefix(key);
    std::lock_guard<std::mutex> lock(idCacheMapMtx_);
    auto idIter = idCacheMap_.find(dmKey);
    if (idIter != idCacheMap_.end() && !IsTimeOut(idIter->second.second, value.lastModifyTime,
        DM_KV_STORE_REFRESH_TIME)) {
        LOGD("Kv value is existed");
        return DM_OK;
    }
    std::string valueStr = "";
    ConvertDmKVValueToJson(value, valueStr);
    idCacheMap_[dmKey].first = value.udidHash;
    idCacheMap_[dmKey].second = value.lastModifyTime;
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Put(dmKey, valueStr) != DM_OK) {
        LOGE("Insert value to DB failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t KVAdapterManager::Get(const std::string& key, DmKVValue& value)
{
    std::string dmKey = AddPrefix(key);
    std::lock_guard<std::mutex> lock(idCacheMapMtx_);
    auto idIter = idCacheMap_.find(dmKey);
    if (idIter != idCacheMap_.end()) {
        value.udidHash = idIter->second.first;
        value.lastModifyTime = idIter->second.second;
        return DM_OK;
    }
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    std::string valueStr;
    if (kvAdapter_->Get(dmKey, valueStr) != DM_OK) {
        LOGE("Get kv value failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    ConvertJsonToDmKVValue(valueStr, value);
    idCacheMap_[dmKey].first = value.udidHash;
    idCacheMap_[dmKey].second = value.lastModifyTime;
    return DM_OK;
}

int32_t KVAdapterManager::DeleteAgedEntry()
{
    int64_t nowTime = GetSecondsSince1970ToNow();
    std::lock_guard<std::mutex> lock(idCacheMapMtx_);
    for (auto it = idCacheMap_.begin(); it != idCacheMap_.end();) {
        if (IsTimeOut(it->second.second, nowTime, MAX_SUPPORTED_EXIST_TIME)) {
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

inline std::string KVAdapterManager::AddPrefix(const std::string& key)
{
    return DM_KV_STORE_PREFIX + key;
}
} // namespace DistributedHardware
} // namespace OHOS
