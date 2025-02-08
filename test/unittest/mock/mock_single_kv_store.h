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

#ifndef OHOS_DM_MOCK_SINGLE_KV_STORE_H
#define OHOS_DM_MOCK_SINGLE_KV_STORE_H

#include <gmock/gmock.h>

#include "single_kvstore.h"

namespace OHOS {
namespace DistributedKv {
class MockSingleKvStore : public SingleKvStore {
public:
    MOCK_METHOD(Status, GetEntries, (const Key &, std::vector<Entry> &), (const));
    MOCK_METHOD(Status, GetEntries, (const DataQuery &, std::vector<Entry> &), (const));
    MOCK_METHOD(Status, GetResultSet, (const Key &, std::shared_ptr<KvStoreResultSet> &), (const));
    MOCK_METHOD(Status, GetResultSet, (const DataQuery &, std::shared_ptr<KvStoreResultSet> &), (const));
    MOCK_METHOD(Status, CloseResultSet, (std::shared_ptr<KvStoreResultSet> &));
    MOCK_METHOD(Status, GetCount, (const DataQuery &, int &), (const));
    MOCK_METHOD(Status, RemoveDeviceData, (const std::string &));
    MOCK_METHOD(Status, GetSecurityLevel, (SecurityLevel &), (const));
    MOCK_METHOD(Status, Sync, (const std::vector<std::string> &, SyncMode, uint32_t));
    MOCK_METHOD(Status, Delete, (const Key &));
    MOCK_METHOD(Status, Put, (const Key &, const Value &));
    MOCK_METHOD(Status, Get, (const Key &, Value &));
    MOCK_METHOD(Status, SubscribeKvStore, (SubscribeType, std::shared_ptr<KvStoreObserver>));
    MOCK_METHOD(Status, UnSubscribeKvStore, (SubscribeType, std::shared_ptr<KvStoreObserver>));
    MOCK_METHOD(Status, RegisterSyncCallback, (std::shared_ptr<KvStoreSyncCallback>));
    MOCK_METHOD(Status, UnRegisterSyncCallback, ());
    MOCK_METHOD(Status, PutBatch, (const std::vector<Entry> &));
    MOCK_METHOD(Status, DeleteBatch, (const std::vector<Key> &));
    MOCK_METHOD(Status, StartTransaction, ());
    MOCK_METHOD(Status, Commit, ());
    MOCK_METHOD(Status, Rollback, ());
    MOCK_METHOD(Status, SetSyncParam, (const KvSyncParam &));
    MOCK_METHOD(Status, GetSyncParam, (KvSyncParam &));
    MOCK_METHOD(Status, SetCapabilityEnabled, (bool), (const));
    MOCK_METHOD(Status, SetCapabilityRange, (const std::vector<std::string> &,
        const std::vector<std::string> &), (const));
    MOCK_METHOD(StoreId, GetStoreId, (), (const));
    MOCK_METHOD(Status, Sync, (const std::vector<std::string> &, SyncMode, const DataQuery &,
        std::shared_ptr<KvStoreSyncCallback>));
    MOCK_METHOD(Status, SubscribeWithQuery, (const std::vector<std::string> &, const DataQuery &));
    MOCK_METHOD(Status, UnsubscribeWithQuery, (const std::vector<std::string> &, const DataQuery &));
    MOCK_METHOD(Status, Backup, (const std::string &, const std::string &));
    MOCK_METHOD(Status, Restore, (const std::string &, const std::string &));
    MOCK_METHOD(Status, DeleteBackup, (const std::vector<std::string> &, const std::string &,
        (std::map<std::string, DistributedKv::Status> &)));
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif  // OHOS_DM_MOCK_SINGLE_KV_STORE_H
