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

#ifndef OHOS_KV_ADAPTER_MANAGER_TEST_H
#define OHOS_KV_ADAPTER_MANAGER_TEST_H

#include <gtest/gtest.h>
#include "kv_adapter_manager.h"
#include "mock_distributed_kv_data_manager.h"
#include "mock_single_kv_store.h"

using OHOS::DistributedKv::DistributedKvDataManagerMock;
using OHOS::DistributedKv::MockSingleKvStore;
namespace OHOS {
namespace DistributedHardware {
class KVAdapterManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    bool InitKvStoreEnv();
    bool UnInitKvStoreEnv();
    std::string CreateDmKVValueStr(const std::string &appId, int64_t lastModifyTime) const;
private:
    std::shared_ptr<DistributedKvDataManagerMock> kvDataMgr_;
    std::shared_ptr<MockSingleKvStore> mockSingleKvStore_;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_KV_ADAPTER_MANAGER_TEST_H
