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
constexpr const char* DM_KV_STORE_FREEZE_PREFIX = "anti_ddos_local_";
constexpr const char* DB_KEY_DELIMITER = "###";
constexpr int64_t DM_KV_STORE_REFRESH_TIME = 24 * 60 * 60; // one day
constexpr int64_t MAX_SUPPORTED_EXIST_TIME = 3 * 24 * 60 * 60; // 3days
constexpr const char* DM_OSTYPE_PREFIX = "ostype";
constexpr const char* DM_UDID_PREFIX = "udid";
}

DM_IMPLEMENT_SINGLE_INSTANCE(KVAdapterManager);

std::string ComposeOsTypePrefix()
{
    std::string dmKey = std::string(DM_OSTYPE_PREFIX) + std::string(DB_KEY_DELIMITER) + std::string(DM_UDID_PREFIX) +
        std::string(DB_KEY_DELIMITER);
    return dmKey;
}

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

DM_EXPORT int32_t KVAdapterManager::GetFreezeData(const std::string &key, std::string &value)
{
    std::string dmKey = DM_KV_STORE_FREEZE_PREFIX + key;
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Get(dmKey, value) != DM_OK) {
        LOGE("Get freeze data failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::PutFreezeData(const std::string &key, std::string &value)
{
    std::string dmKey = DM_KV_STORE_FREEZE_PREFIX + key;
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Put(dmKey, value) != DM_OK) {
        LOGE("Insert freeze data failed, k:%{public}s, v:%{public}s", dmKey.c_str(), value.c_str());
        return ERR_DM_FAILED;
    }
    LOGI("Insert freeze data success, k:%{public}s, v:%{public}s", dmKey.c_str(), value.c_str());
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::DeleteFreezeData(const std::string &key)
{
    std::string dmKey = DM_KV_STORE_FREEZE_PREFIX + key;
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Delete(dmKey) != DM_OK) {
        LOGE("delete freeze data failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::GetAllOstypeData(std::vector<std::string> &values)
{
    std::string dmKey = ComposeOsTypePrefix();
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->GetAllOstypeData(dmKey, values) != DM_OK) {
        LOGE("Get all data failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::PutOstypeData(const std::string &key, const std::string &value)
{
    LOGI("key %{publkic}s, value %{public}s.", GetAnonyString(key).c_str(), value.c_str());
    std::string dmKey = ComposeOsTypePrefix() + key;
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Put(dmKey, value) != DM_OK) {
        LOGE("Insert data failed, k:%{public}s, v:%{public}s", GetAnonyString(dmKey).c_str(), value.c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::DeleteOstypeData(const std::string &key)
{
    LOGI("key %{publkic}s.", GetAnonyString(key).c_str());
    std::string dmKey = ComposeOsTypePrefix() + key;
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Delete(dmKey) != DM_OK) {
        LOGE("delete data failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::GetLocalUserIdData(const std::string &key, std::string &value)
{
    LOGI("key %{public}s.", GetAnonyString(key).c_str());
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Get(key, value) != DM_OK) {
        LOGE("Get data failed, key: %{public}s", GetAnonyString(key).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::PutLocalUserIdData(const std::string &key, const std::string &value)
{
    LOGI("key %{public}s, value %{public}s.", GetAnonyString(key).c_str(), value.c_str());
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    if (kvAdapter_->Put(key, value) != DM_OK) {
        LOGE("Put data failed, key:%{public}s, value:%{public}s", GetAnonyString(key).c_str(), value.c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::GetOsTypeCount(int32_t &count)
{
    CHECK_NULL_RETURN(kvAdapter_, ERR_DM_POINT_NULL);
    std::string osTypePrefix = ComposeOsTypePrefix();
    if (kvAdapter_->GetOstypeCountByPrefix(osTypePrefix, count) != DM_OK) {
        LOGE("GetOstypeCountByPrefix failed, osTypePrefix:%{public}s.", osTypePrefix.c_str());
        return ERR_DM_FAILED;
    }
    LOGI("count %{public}d.", count);
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
