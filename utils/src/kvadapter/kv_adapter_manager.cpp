/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "dm_library_manager.h"

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

constexpr const char* KV_ADAPTER_LIB_NAME = "libdmdb_kvstore.z.so";
constexpr const char* CREATE_KV_ADAPTER_FUNC = "CreateKVAdapter";
constexpr const char* KV_ADAPTER_TIMER = "DMKVAdapterTimer";
// if no use kvAdapter function more than 60 sec, release kvAdapter so
constexpr int32_t RELEASE_KV_ADAPTER_TIMESPAN = 60;
}

DM_IMPLEMENT_SINGLE_INSTANCE(KVAdapterManager);

std::string ComposeOsTypePrefix()
{
    std::string dmKey = std::string(DM_OSTYPE_PREFIX) + std::string(DB_KEY_DELIMITER) + std::string(DM_UDID_PREFIX) +
        std::string(DB_KEY_DELIMITER);
    return dmKey;
}

KVAdapterPtr KVAdapterManager::GetKvAdapter()
{
    std::lock_guard<ffrt::mutex> lock(kvAdapterMtx_);
    if (kvAdapter_ != nullptr) {
        refCount_++;
        return KVAdapterPtr(kvAdapter_, KVAdapterDeleter(*this));
    }

    auto& libManager = GetLibraryManager();
    auto createKVAdapterObj = libManager.GetFunction<IKVAdapter*(*)()>(KV_ADAPTER_LIB_NAME,
        CREATE_KV_ADAPTER_FUNC);
    if (createKVAdapterObj == nullptr) {
        LOGE("load kv adapter so failed");
        libManager.Release(KV_ADAPTER_LIB_NAME);
        return KVAdapterPtr(nullptr, KVAdapterDeleter(*this));
    }

    kvAdapter_ = createKVAdapterObj();
    if (kvAdapter_ == nullptr) {
        LOGE("get kv adapter ptr failed");
        libManager.Release(KV_ADAPTER_LIB_NAME);
        return KVAdapterPtr(nullptr, KVAdapterDeleter(*this));
    }

    if (kvAdapter_->Init() != DM_OK) {
        LOGE("get kv adapter init failed");
        kvAdapter_->UnInit();
        kvAdapter_ = nullptr;
        libManager.Release(KV_ADAPTER_LIB_NAME);
        return KVAdapterPtr(nullptr, KVAdapterDeleter(*this));
    }
    
    refCount_++;
    return KVAdapterPtr(kvAdapter_, KVAdapterDeleter(*this));
}

void KVAdapterManager::AfterUseKvAdapter()
{
    if (refCount_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        libTimer_->DeleteTimer(KV_ADAPTER_TIMER);
        libTimer_->StartTimer(KV_ADAPTER_TIMER, RELEASE_KV_ADAPTER_TIMESPAN,
            [this, libName = std::string(KV_ADAPTER_LIB_NAME)] (std::string timerName) {
            (void)timerName;
            LOGI("Uninit and Release kv adapter");
            std::lock_guard<ffrt::mutex> lock(kvAdapterMtx_);
            if (kvAdapter_ != nullptr) {
                kvAdapter_->UnInit();
                kvAdapter_ = nullptr;
            }
            GetLibraryManager().Release(libName);
        });
    }
}

DM_EXPORT int32_t KVAdapterManager::Init()
{
    LOGI("Init Kv-Adapter manager");
    {
        std::lock_guard<ffrt::mutex> lock(idCacheMapMtx_);
        idCacheMap_.clear();
    }

    libTimer_ = std::make_unique<DmTimer>();

    return DM_OK;
}

DM_EXPORT void KVAdapterManager::UnInit()
{
    LOGI("Uninit Kv-Adapter manager");
}

DM_EXPORT void KVAdapterManager::ReInit()
{
    LOGI("Re init kv adapter");
    std::lock_guard<ffrt::mutex> lock(idCacheMapMtx_);
    idCacheMap_.clear();
}

int32_t KVAdapterManager::PutByAnoyDeviceId(const std::string &key, const DmKVValue &value)
{
    std::string dmKey = DM_KV_STORE_PREFIX + key;
    std::string prefixKey = DM_KV_STORE_PREFIX + value.appID + DB_KEY_DELIMITER + value.udidHash;
    {
        std::lock_guard<ffrt::mutex> lock(idCacheMapMtx_);
        auto idIter = idCacheMap_.find(dmKey);
        if (idIter != idCacheMap_.end() && !IsTimeOut(idIter->second.lastModifyTime, value.lastModifyTime,
            DM_KV_STORE_REFRESH_TIME)) {
            LOGD("Kv value is existed");
            return DM_OK;
        }
        idCacheMap_[dmKey] = value;
        idCacheMap_[prefixKey] = value;
    }
    std::string valueStr = "";
    ConvertDmKVValueToJson(value, valueStr);
    {
        auto kvAdapterPtr = GetKvAdapter();
        CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);

        if (kvAdapterPtr->Put(dmKey, valueStr) != DM_OK) {
            LOGE("Insert value to DB for dmKey failed");
            return ERR_DM_FAILED;
        }
        if (kvAdapterPtr->Put(prefixKey, valueStr) != DM_OK) {
            LOGE("Insert value to DB for prefixKey failed");
            return ERR_DM_FAILED;
        }
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::Get(const std::string &key, DmKVValue &value)
{
    std::string dmKey = DM_KV_STORE_PREFIX + key;
    {
        std::lock_guard<ffrt::mutex> lock(idCacheMapMtx_);
        auto idIter = idCacheMap_.find(dmKey);
        if (idIter != idCacheMap_.end()) {
            value = idIter->second;
            return DM_OK;
        }
    }
    std::string valueStr;
    {
        auto kvAdapterPtr = GetKvAdapter();
        CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);

        if (kvAdapter_->Get(dmKey, valueStr) != DM_OK) {
            LOGE("Get kv value failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
            return ERR_DM_FAILED;
        }
    }
    ConvertJsonToDmKVValue(valueStr, value);
    {
        std::lock_guard<ffrt::mutex> lock(idCacheMapMtx_);
        idCacheMap_[dmKey] = value;
        std::string prefixKey = DM_KV_STORE_PREFIX + value.appID + DB_KEY_DELIMITER + value.udidHash;
        idCacheMap_[prefixKey] = value;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::DeleteAgedEntry()
{
    int64_t nowTime = GetSecondsSince1970ToNow();
    std::lock_guard<ffrt::mutex> lock(idCacheMapMtx_);
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

DM_EXPORT int32_t KVAdapterManager::AppUninstall(const std::string &appId)
{
    LOGI("appId %{public}s.", GetAnonyString(appId).c_str());
    {
        std::lock_guard<ffrt::mutex> lock(idCacheMapMtx_);
        for (auto it = idCacheMap_.begin(); it != idCacheMap_.end();) {
            if (it->second.appID == appId) {
                it = idCacheMap_.erase(it);
            } else {
                ++it;
            }
        }
    }
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    if (kvAdapterPtr->DeleteByAppId(appId, DM_KV_STORE_PREFIX) != DM_OK) {
        LOGE("DeleteByAppId failed");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::GetFreezeData(const std::string &key, std::string &value)
{
    std::string dmKey = DM_KV_STORE_FREEZE_PREFIX + key;
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    if (kvAdapterPtr->Get(dmKey, value) != DM_OK) {
        LOGE("Get freeze data failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::PutFreezeData(const std::string &key, std::string &value)
{
    std::string dmKey = DM_KV_STORE_FREEZE_PREFIX + key;
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    if (kvAdapterPtr->Put(dmKey, value) != DM_OK) {
        LOGE("Insert freeze data failed, k:%{public}s, v:%{public}s", dmKey.c_str(), value.c_str());
        return ERR_DM_FAILED;
    }
    LOGI("Insert freeze data success, k:%{public}s, v:%{public}s", dmKey.c_str(), value.c_str());
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::DeleteFreezeData(const std::string &key)
{
    std::string dmKey = DM_KV_STORE_FREEZE_PREFIX + key;
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    if (kvAdapterPtr->Delete(dmKey) != DM_OK) {
        LOGE("delete freeze data failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::GetAllOstypeData(std::vector<std::string> &values)
{
    std::string dmKey = ComposeOsTypePrefix();
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    if (kvAdapterPtr->GetAllOstypeData(dmKey, values) != DM_OK) {
        LOGE("Get all data failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::PutOstypeData(const std::string &key, const std::string &value)
{
    LOGI("key %{public}s, value %{public}s.", GetAnonyString(key).c_str(), value.c_str());
    std::string dmKey = ComposeOsTypePrefix() + key;
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    if (kvAdapterPtr->Put(dmKey, value) != DM_OK) {
        LOGE("Insert data failed, k:%{public}s, v:%{public}s", GetAnonyString(dmKey).c_str(), value.c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::DeleteOstypeData(const std::string &key)
{
    LOGI("key %{public}s.", GetAnonyString(key).c_str());
    std::string dmKey = ComposeOsTypePrefix() + key;
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    if (kvAdapterPtr->Delete(dmKey) != DM_OK) {
        LOGE("delete data failed, dmKey: %{public}s", GetAnonyString(dmKey).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::GetLocalUserIdData(const std::string &key, std::string &value)
{
    LOGI("key %{public}s.", GetAnonyString(key).c_str());
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    if (kvAdapterPtr->Get(key, value) != DM_OK) {
        LOGE("Get data failed, key: %{public}s", GetAnonyString(key).c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::PutLocalUserIdData(const std::string &key, const std::string &value)
{
    LOGI("key %{public}s, value %{public}s.", GetAnonyString(key).c_str(), value.c_str());
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    if (kvAdapterPtr->Put(key, value) != DM_OK) {
        LOGE("Put data failed, key:%{public}s, value:%{public}s", GetAnonyString(key).c_str(), value.c_str());
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

DM_EXPORT int32_t KVAdapterManager::GetOsTypeCount(int32_t &count)
{
    auto kvAdapterPtr = GetKvAdapter();
    CHECK_NULL_RETURN(kvAdapterPtr, ERR_DM_POINT_NULL);
    std::string osTypePrefix = ComposeOsTypePrefix();
    if (kvAdapterPtr->GetOstypeCountByPrefix(osTypePrefix, count) != DM_OK) {
        LOGE("GetOstypeCountByPrefix failed, osTypePrefix:%{public}s.", osTypePrefix.c_str());
        return ERR_DM_FAILED;
    }
    LOGI("count %{public}d.", count);
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
