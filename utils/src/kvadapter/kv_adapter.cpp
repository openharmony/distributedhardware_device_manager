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
#include "kv_adapter.h"

#include <cinttypes>
#include <mutex>
#include <unistd.h>

#include "cJSON.h"
#include "datetime_ex.h"
#include "string_ex.h"

#include "data_query.h"
#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_error_type.h"
#include "dm_log.h"
#include "ffrt.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::DistributedKv;
namespace {
    const std::string APP_ID = "distributed_device_manager_service";
    const std::string STORE_ID = "dm_kv_store";
    const std::string DATABASE_DIR = "/data/service/el1/public/database/distributed_device_manager_service";
    const std::string KV_REINIT_THREAD = "reinit_kv_store";
    constexpr uint32_t MAX_BATCH_SIZE = 128;
    constexpr int32_t MAX_STRING_LEN = 4096;
    constexpr int32_t MAX_INIT_RETRY_TIMES = 20;
    constexpr int32_t INIT_RETRY_SLEEP_INTERVAL = 200 * 1000; // 200ms
    constexpr uint32_t DM_OSTYPE_PREFIX_LEN = 16;
}

int32_t KVAdapter::Init()
{
    LOGI("Init local DB, dataType: %{public}d", static_cast<int32_t>(dataType_));
    if (isInited_.load()) {
        LOGI("Local DB already inited.");
        return DM_OK;
    }
    this->appId_.appId = APP_ID;
    this->storeId_.storeId = STORE_ID;
    std::lock_guard<std::mutex> lock(kvAdapterMutex_);
    int32_t tryTimes = MAX_INIT_RETRY_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetLocalKvStorePtr();
        if (status == DistributedKv::Status::SUCCESS && kvStorePtr_) {
            LOGI("Init KvStorePtr Success");
            RegisterKvStoreDeathListener();
            isInited_.store(true);
            return DM_OK;
        }
        LOGE("CheckKvStore, left times: %{public}d, status: %{public}d", tryTimes, status);
        if (status == DistributedKv::Status::STORE_META_CHANGED ||
            status == DistributedKv::Status::SECURITY_LEVEL_ERROR ||
            status == DistributedKv::Status::DATA_CORRUPTED) {
            LOGE("init db error, remove and rebuild it");
            DeleteKvStore();
        }
        usleep(INIT_RETRY_SLEEP_INTERVAL);
        tryTimes--;
    }
    CHECK_NULL_RETURN(kvStorePtr_, ERR_DM_INIT_FAILED);
    isInited_.store(true);
    return DM_OK;
}

void KVAdapter::UnInit()
{
    LOGI("KVAdapter Uninted");
    if (isInited_.load()) {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        CHECK_NULL_VOID(kvStorePtr_);
        UnregisterKvStoreDeathListener();
        kvStorePtr_.reset();
        isInited_.store(false);
    }
}

int32_t KVAdapter::ReInit()
{
    LOGI("KVAdapter ReInit");
    UnInit();
    return Init();
}

int32_t KVAdapter::Put(const std::string &key, const std::string &value)
{
    if (key.empty() || key.size() > MAX_STRING_LEN || value.empty() || value.size() > MAX_STRING_LEN) {
        LOGE("Param is invalid!");
        return ERR_DM_FAILED;
    }
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        CHECK_NULL_RETURN(kvStorePtr_, ERR_DM_POINT_NULL);

        DistributedKv::Key kvKey(key);
        DistributedKv::Value kvValue(value);
        status = kvStorePtr_->Put(kvKey, kvValue);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        LOGE("Put kv to db failed, ret: %{public}d", status);
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t KVAdapter::Get(const std::string &key, std::string &value)
{
    LOGI("Get data by key: %{public}s", GetAnonyString(key).c_str());
    DistributedKv::Key kvKey(key);
    DistributedKv::Value kvValue;
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        CHECK_NULL_RETURN(kvStorePtr_, ERR_DM_POINT_NULL);
        status = kvStorePtr_->Get(kvKey, kvValue);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        LOGE("Get data from kv failed, key: %{public}s", GetAnonyString(key).c_str());
        return ERR_DM_FAILED;
    }
    value = kvValue.ToString();
    return DM_OK;
}

void KVAdapter::OnRemoteDied()
{
    LOGI("OnRemoteDied, recover db begin");
    auto reInitTask = [this]() {
        LOGI("ReInit, storeId:%{public}s", storeId_.storeId.c_str());
        ReInit();
    };
    ffrt::submit(reInitTask);
}

DistributedKv::Status KVAdapter::GetLocalKvStorePtr()
{
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .securityLevel = DistributedKv::SecurityLevel::S1,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = DATABASE_DIR
    };
    DistributedKv::Status status = kvDataMgr_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);
    return status;
}

void KVAdapter::RegisterKvStoreDeathListener()
{
    LOGI("Register syncCompleted listener");
    kvDataMgr_.RegisterKvStoreServiceDeathRecipient(shared_from_this());
}

void KVAdapter::UnregisterKvStoreDeathListener()
{
    LOGI("UnRegister death listener");
    kvDataMgr_.UnRegisterKvStoreServiceDeathRecipient(shared_from_this());
}

int32_t KVAdapter::DeleteKvStore()
{
    LOGI("Delete KvStore!");
    kvDataMgr_.CloseKvStore(appId_, storeId_);
    kvDataMgr_.DeleteKvStore(appId_, storeId_, DATABASE_DIR);
    return DM_OK;
}

int32_t KVAdapter::DeleteByAppId(const std::string &appId, const std::string &prefix)
{
    if (appId.empty()) {
        LOGE("appId is empty");
        return ERR_DM_FAILED;
    }
    std::vector<DistributedKv::Entry> localEntries;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LOGE("kvStoragePtr_ is null");
            return ERR_DM_POINT_NULL;
        }
        if (kvStorePtr_->GetEntries(prefix + appId, localEntries) != DistributedKv::Status::SUCCESS) {
            LOGE("Get entrys from DB failed.");
            return ERR_DM_FAILED;
        }
    }
    std::vector<std::string> delKeys;
    for (const auto &entry : localEntries) {
        delKeys.emplace_back(entry.key.ToString());
        DmKVValue kvValue;
        ConvertJsonToDmKVValue(entry.value.ToString(), kvValue);
        delKeys.emplace_back(prefix + kvValue.anoyDeviceId);
    }
    return DeleteBatch(delKeys);
}

int32_t KVAdapter::DeleteBatch(const std::vector<std::string> &keys)
{
    if (keys.empty()) {
        LOGE("keys size(%{public}zu) is invalid!", keys.size());
        return ERR_DM_FAILED;
    }
    uint32_t keysSize = static_cast<uint32_t>(keys.size());
    std::vector<std::vector<DistributedKv::Key>> delKeyBatches;
    for (uint32_t i = 0; i < keysSize; i += MAX_BATCH_SIZE) {
        uint32_t end = (i + MAX_BATCH_SIZE) > keysSize ? keysSize : (i + MAX_BATCH_SIZE);
        auto batch = std::vector<std::string>(keys.begin() + i, keys.begin() + end);
        std::vector<DistributedKv::Key> delKeys;
        for (auto item : batch) {
            DistributedKv::Key key(item);
            delKeys.emplace_back(key);
        }
        delKeyBatches.emplace_back(delKeys);
    }

    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LOGE("kvStorePtr is nullptr!");
            return ERR_DM_POINT_NULL;
        }
        for (auto delKeys : delKeyBatches) {
            DistributedKv::Status status = kvStorePtr_->DeleteBatch(delKeys);
            if (status != DistributedKv::Status::SUCCESS) {
                LOGE("DeleteBatch failed!");
                return ERR_DM_FAILED;
            }
        }
    }
    return DM_OK;
}

int32_t KVAdapter::Delete(const std::string& key)
{
    DistributedKv::Status status;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        if (kvStorePtr_ == nullptr) {
            LOGE("kvStorePtr is nullptr!");
            return ERR_DM_POINT_NULL;
        }
        DistributedKv::Key kvKey(key);
        status = kvStorePtr_->Delete(kvKey);
    }
    if (status != DistributedKv::Status::SUCCESS) {
        LOGE("Delete kv by key failed!");
        return ERR_DM_FAILED;
    }
    return DM_OK;
}

int32_t KVAdapter::GetAllOstypeData(const std::string &key, std::vector<std::string> &values)
{
    if (key.empty()) {
        LOGE("key is empty");
        return ERR_DM_FAILED;
    }
    std::vector<DistributedKv::Entry> localEntries;
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        CHECK_NULL_RETURN(kvStorePtr_, ERR_DM_POINT_NULL);
        if (kvStorePtr_->GetEntries(key, localEntries) != DistributedKv::Status::SUCCESS) {
            LOGE("Get entrys from DB failed.");
            return ERR_DM_FAILED;
        }
    }
    values.clear();
    for (const auto &entry : localEntries) {
        JsonObject osTyoeJson(entry.value.ToString());
        if (osTyoeJson.IsDiscarded() || !IsInt32(osTyoeJson, PEER_OSTYPE) || !IsInt64(osTyoeJson, TIME_STAMP)) {
            LOGE("entry parse error.");
            continue;
        }
        if (entry.key.ToString().size() < DM_OSTYPE_PREFIX_LEN) {
            LOGE("entry value invalid.");
            continue;
        }
        JsonObject jsonObj;
        jsonObj[PEER_UDID] = entry.key.ToString().substr(DM_OSTYPE_PREFIX_LEN);
        jsonObj[PEER_OSTYPE] = osTyoeJson[PEER_OSTYPE].Get<int32_t>();
        jsonObj[TIME_STAMP] = osTyoeJson[TIME_STAMP].Get<int64_t>();
        values.push_back(SafetyDump(jsonObj));
    }
    return DM_OK;
}

int32_t KVAdapter::GetOstypeCountByPrefix(const std::string &prefix, int32_t &count)
{
    LOGI("prefix %{public}s.", prefix.c_str());
    if (prefix.empty()) {
        LOGE("prefix is empty.");
        return ERR_DM_FAILED;
    }
    {
        std::lock_guard<std::mutex> lock(kvAdapterMutex_);
        CHECK_NULL_RETURN(kvStorePtr_, ERR_DM_POINT_NULL);
        DataQuery prefixQuery;
        prefixQuery.KeyPrefix(prefix);
        if (kvStorePtr_->GetCount(prefixQuery, count) != DistributedKv::Status::SUCCESS) {
            LOGE("GetCount failed.");
            return ERR_DM_FAILED;
        }
    }
    return DM_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
