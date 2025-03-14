/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "mock_distributed_kv_data_manager.h"
#include "mock_single_kv_store.h"

namespace OHOS {
namespace DistributedKv {
std::shared_ptr<IDistributedKvDataManager> IDistributedKvDataManager::kvDataMgr_ = nullptr;

std::shared_ptr<IDistributedKvDataManager> IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()
{
    if (!kvDataMgr_) {
        kvDataMgr_ = std::make_shared<DistributedKvDataManagerMock>();
    }
    return kvDataMgr_;
}

void IDistributedKvDataManager::ReleaseDistributedKvDataManager()
{
    kvDataMgr_.reset();
    kvDataMgr_ = nullptr;
}

DistributedKvDataManager::DistributedKvDataManager()
{}

DistributedKvDataManager::~DistributedKvDataManager()
{}

Status DistributedKvDataManager::GetSingleKvStore(const Options &options, const AppId &appId, const StoreId &storeId,
    std::shared_ptr<SingleKvStore> &singleKvStore)
{
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->GetSingleKvStore(
        options, appId, storeId, singleKvStore);
}

Status DistributedKvDataManager::GetAllKvStoreId(const AppId &appId, std::vector<StoreId> &storeIds, int32_t subUser)
{
    (void)subUser;
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->GetAllKvStoreId(appId, storeIds);
}

Status DistributedKvDataManager::CloseKvStore(const AppId &appId, const StoreId &storeId, int32_t subUser)
{
    (void)subUser;
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->CloseKvStore(appId, storeId);
}

Status DistributedKvDataManager::CloseKvStore(const AppId &appId, std::shared_ptr<SingleKvStore> &kvStore)
{
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->CloseKvStore(appId, kvStore);
}

Status DistributedKvDataManager::CloseAllKvStore(const AppId &appId, int32_t subUser)
{
    (void)subUser;
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->CloseAllKvStore(appId);
}

Status DistributedKvDataManager::DeleteKvStore(const AppId &appId, const StoreId &storeId, const std::string &path,
    int32_t subUser)
{
    (void)subUser;
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->DeleteKvStore(appId, storeId, path);
}

Status DistributedKvDataManager::DeleteAllKvStore(const AppId &appId, const std::string &path, int32_t subUser)
{
    (void)subUser;
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->DeleteAllKvStore(appId, path);
}

void DistributedKvDataManager::RegisterKvStoreServiceDeathRecipient(
    std::shared_ptr<KvStoreDeathRecipient> deathRecipient)
{
    IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->RegisterKvStoreServiceDeathRecipient(
        deathRecipient);
}

void DistributedKvDataManager::UnRegisterKvStoreServiceDeathRecipient(
    std::shared_ptr<KvStoreDeathRecipient> deathRecipient)
{
    IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->UnRegisterKvStoreServiceDeathRecipient(
        deathRecipient);
}

void DistributedKvDataManager::SetExecutors(std::shared_ptr<ExecutorPool> executors)
{
    IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->SetExecutors(
        executors);
}

Status DistributedKvDataManager::SetEndpoint(std::shared_ptr<Endpoint> endpoint)
{
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->SetEndpoint(endpoint);
}

Status DistributedKvDataManager::PutSwitch(const AppId &appId, const SwitchData &data)
{
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->PutSwitch(appId, data);
}

std::pair<Status, SwitchData> DistributedKvDataManager::GetSwitch(const AppId &appId, const std::string &networkId)
{
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->GetSwitch(appId, networkId);
}

Status DistributedKvDataManager::SubscribeSwitchData(const AppId &appId, std::shared_ptr<KvStoreObserver> observer)
{
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->SubscribeSwitchData(appId, observer);
}

Status DistributedKvDataManager::UnsubscribeSwitchData(const AppId &appId, std::shared_ptr<KvStoreObserver> observer)
{
    return IDistributedKvDataManager::GetOrCreateDistributedKvDataManager()->UnsubscribeSwitchData(appId, observer);
}
}  // namespace DistributedKv
}  // namespace OHOS