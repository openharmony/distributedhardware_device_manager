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

#ifndef OHOS_DM_KV_ADAPTER_H
#define OHOS_DM_KV_ADAPTER_H

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "ffrt.h"

#include "ikv_adapter.h"
#include "distributed_kv_data_manager.h"
#include "kvstore_death_recipient.h"
#include "kvstore_observer.h"

namespace OHOS {
namespace DistributedHardware {
class KVAdapter : public IKVAdapter {
public:
    KVAdapter() = default;
    virtual ~KVAdapter() = default;

    int32_t Init() override;
    void UnInit() override;
    int32_t ReInit() override;
    int32_t Put(const std::string &key, const std::string &value) override;
    int32_t Get(const std::string &key, std::string &value) override;
    int32_t DeleteKvStore() override;
    int32_t DeleteByAppId(const std::string &appId, const std::string &prefix) override;
    int32_t DeleteBatch(const std::vector<std::string> &keys) override;
    int32_t Delete(const std::string& key) override;
    int32_t GetAllOstypeData(const std::string &key, std::vector<std::string> &values) override;
    int32_t GetOstypeCountByPrefix(const std::string &prefix, int32_t &count) override;

private:
    DistributedKv::Status GetLocalKvStorePtr();

private:
    DistributedKv::AppId appId_;
    DistributedKv::StoreId storeId_;
    DistributedKv::DistributedKvDataManager kvDataMgr_;
    DistributedKv::DataType dataType_ = DistributedKv::DataType::TYPE_STATICS;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_ = nullptr;
    ffrt::mutex kvAdapterMutex_;
    ffrt::mutex kvDataMgrMutex_;
    std::atomic<bool> isInited_ = false;
};

extern "C" IKVAdapter* CreateKVAdapter();
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_KV_ADAPTER_H
