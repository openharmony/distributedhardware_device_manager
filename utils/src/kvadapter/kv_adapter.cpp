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

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "dm_thread_manager.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::DistributedKv;
namespace {
    const std::string APP_ID = "distributed_device_manager_service";
    const std::string STORE_ID = "dm_kv_store";
    const std::string DATABASE_DIR = "/data/service/el1/public/database/distributed_device_manager_service";
    const std::string KV_REINIT_THREAD = "reinit_kv_store";
    const std::string UDID_HASH_KEY = "udidHash";
    const std::string APP_ID_KEY = "appID";
    const std::string UDID_ID_KEY = "udid";
    const std::string LAST_MODIFY_TIME_KEY = "lastModifyTime";
    constexpr uint32_t MAX_BATCH_SIZE = 128;
    constexpr int32_t MAX_DEVICE_ITEM_SIZE = 1000;
    constexpr int32_t MAX_STRING_LEN = 4096;
    constexpr int32_t MAX_INIT_RETRY_TIMES = 20;
    constexpr int32_t INIT_RETRY_SLEEP_INTERVAL = 200 * 1000; // 200ms
}

void ConvertDmKVValueToJson(const DmKVValue &kvValue, std::string &result)
{
    nlohmann::json jsonObj;
    jsonObj[UDID_HASH_KEY] = kvValue.udidHash;
    jsonObj[APP_ID_KEY] = kvValue.appID;
    jsonObj[UDID_ID_KEY] = kvValue.udid;
    jsonObj[LAST_MODIFY_TIME_KEY] = kvValue.lastModifyTime;
    result = jsonObj.dump();
}

void ConvertJsonToDmKVValue(const std::string &result, DmKVValue &kvValue)
{
    if (result.empty()) {
        return;
    }
    nlohmann::json resultJson = nlohmann::json::parse(result, nullptr, false);
    if (resultJson.is_discarded()) {
        return;
    }
    if (IsString(resultJson, UDID_HASH_KEY)) {
        kvValue.udidHash = resultJson[UDID_HASH_KEY].get<std::string>();
    }
    if (IsString(resultJson, APP_ID_KEY)) {
        kvValue.appID = resultJson[APP_ID_KEY].get<std::string>();
    }
    if (IsString(resultJson, UDID_ID_KEY)) {
        kvValue.udid = resultJson[UDID_ID_KEY].get<std::string>();
    }
    if (IsString(resultJson, LAST_MODIFY_TIME_KEY)) {
        kvValue.lastModifyTime = resultJson[LAST_MODIFY_TIME_KEY].get<int64_t>();
    }
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
        LOGD("CheckKvStore, left times: %{public}d", tryTimes);
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

int32_t KVAdapter::Put(const std::string& key, const std::string& value)
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

int32_t KVAdapter::Get(const std::string& key, std::string& value)
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
    ThreadManager::GetInstance().Submit(KV_REINIT_THREAD.c_str(), reInitTask);
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
} // namespace DistributedHardware
} // namespace OHOS
