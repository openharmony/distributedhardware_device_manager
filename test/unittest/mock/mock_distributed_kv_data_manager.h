/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_DISTRIBUTED_KV_DATA_MANAGER_MOCK_H
#define OHOS_DM_DISTRIBUTED_KV_DATA_MANAGER_MOCK_H

#include <gmock/gmock.h>

#include "distributed_kv_data_manager.h"

namespace OHOS {
namespace DistributedKv {
class IDistributedKvDataManager {
public:
    virtual ~IDistributedKvDataManager() = default;

    virtual Status GetSingleKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
                                       std::shared_ptr<SingleKvStore> &singleKvStore) = 0;
    virtual Status GetAllKvStoreId(const AppId &appId, std::vector<StoreId> &storeIds) = 0;
    virtual Status CloseKvStore(const AppId &appId, const StoreId &storeId) = 0;
    virtual Status CloseKvStore(const AppId &appId, std::shared_ptr<SingleKvStore> &kvStore) = 0;
    virtual Status CloseAllKvStore(const AppId &appId) = 0;
    virtual Status DeleteKvStore(const AppId &appId, const StoreId &storeId, const std::string &path) = 0;
    virtual Status DeleteAllKvStore(const AppId &appId, const std::string &path);
    virtual void RegisterKvStoreServiceDeathRecipient(std::shared_ptr<KvStoreDeathRecipient> deathRecipient) = 0;
    virtual void UnRegisterKvStoreServiceDeathRecipient(std::shared_ptr<KvStoreDeathRecipient> deathRecipient) = 0;

    virtual void SetExecutors(std::shared_ptr<ExecutorPool> executors) = 0;
    virtual Status SetEndpoint(std::shared_ptr<Endpoint> endpoint) = 0;
    virtual Status PutSwitch(const AppId &appId, const SwitchData &data) = 0;
    virtual std::pair<Status, SwitchData> GetSwitch(const AppId &appId, const std::string &networkId) = 0;
    virtual Status SubscribeSwitchData(const AppId &appId, std::shared_ptr<KvStoreObserver> observer) = 0;
    virtual Status UnsubscribeSwitchData(const AppId &appId, std::shared_ptr<KvStoreObserver> observer) = 0;
    static std::shared_ptr<IDistributedKvDataManager> GetOrCreateDistributedKvDataManager();
    static void ReleaseDistributedKvDataManager();
private:
    static std::shared_ptr<IDistributedKvDataManager> kvDataMgr_;
};

class DistributedKvDataManagerMock : public IDistributedKvDataManager {
public:
    MOCK_METHOD(Status, GetSingleKvStore, (const Options &, const AppId &, const StoreId &,
                            std::shared_ptr<SingleKvStore> &));
    MOCK_METHOD(Status, GetAllKvStoreId, (const AppId &, std::vector<StoreId> &));
    MOCK_METHOD(Status, CloseKvStore, (const AppId &, const StoreId &));
    MOCK_METHOD(Status, CloseKvStore, (const AppId &, std::shared_ptr<SingleKvStore> &));
    MOCK_METHOD(Status, CloseAllKvStore, (const AppId &));
    MOCK_METHOD(Status, DeleteKvStore, (const AppId &, const StoreId &, const std::string &));
    MOCK_METHOD(Status, DeleteAllKvStore, (const AppId &, const std::string &));
    MOCK_METHOD(void, RegisterKvStoreServiceDeathRecipient, (std::shared_ptr<KvStoreDeathRecipient>));
    MOCK_METHOD(void, UnRegisterKvStoreServiceDeathRecipient, (std::shared_ptr<KvStoreDeathRecipient>));
    MOCK_METHOD(void, SetExecutors, (std::shared_ptr<ExecutorPool>));
    MOCK_METHOD(Status, SetEndpoint, (std::shared_ptr<Endpoint>));
    MOCK_METHOD(Status, PutSwitch, (const AppId &, const SwitchData &));
    MOCK_METHOD((std::pair<Status, SwitchData>), GetSwitch, (const AppId &, const std::string &));
    MOCK_METHOD(Status, SubscribeSwitchData, (const AppId &, std::shared_ptr<KvStoreObserver>));
    MOCK_METHOD(Status, UnsubscribeSwitchData, (const AppId &, std::shared_ptr<KvStoreObserver>));
};
}
}
#endif  // OHOS_DM_DISTRIBUTED_KV_DATA_MANAGER_MOCK_H