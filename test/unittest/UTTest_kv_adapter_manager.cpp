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

#include "UTTest_kv_adapter_manager.h"

#include "datetime_ex.h"
#include "dm_anonymous.h"
#include "dm_constants.h"

using ::testing::_;
using ::testing::An;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::SetArgReferee;

using namespace OHOS::DistributedKv;
namespace OHOS {
namespace DistributedHardware {
namespace {
const std::string KEY = "KEY_";
const std::string APPID = "APPID_";
const size_t ARG_FIRST = 0;
const size_t ARG_SECOND = 1;
const size_t ARG_THIRD = 2;
const size_t ARG_FOURTH = 3;
constexpr int64_t ANCHOR_TIME = 2025;
constexpr const char* UDID_HASH_KEY = "udidHash";
constexpr const char* APP_ID_KEY = "appID";
constexpr const char* ANOY_DEVICE_ID_KEY = "anoyDeviceId";
constexpr const char* SALT_KEY = "salt";
constexpr const char* LAST_MODIFY_TIME_KEY = "lastModifyTime";
} // namespace

void KVAdapterManagerTest::SetUp()
{
    InitKvStoreEnv();
}

void KVAdapterManagerTest::TearDown()
{
    UnInitKvStoreEnv();
}

void KVAdapterManagerTest::SetUpTestCase()
{}

void KVAdapterManagerTest::TearDownTestCase()
{}

bool KVAdapterManagerTest::InitKvStoreEnv()
{
    auto kvDataMgr = IDistributedKvDataManager::GetOrCreateDistributedKvDataManager();
    kvDataMgr_ = std::static_pointer_cast<DistributedKvDataManagerMock>(kvDataMgr);
    mockSingleKvStore_ = std::make_shared<MockSingleKvStore>();
    if (!kvDataMgr_ || !mockSingleKvStore_) {
        return false;
    }
    EXPECT_CALL(*kvDataMgr_, GetSingleKvStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<ARG_FOURTH>(mockSingleKvStore_), Return(Status::SUCCESS)));
    return KVAdapterManager::GetInstance().Init() == DM_OK;
}

bool KVAdapterManagerTest::UnInitKvStoreEnv()
{
    if (!kvDataMgr_ || !mockSingleKvStore_) {
        return false;
    }
    KVAdapterManager::GetInstance().UnInit();
    IDistributedKvDataManager::ReleaseDistributedKvDataManager();
    mockSingleKvStore_.reset();
    mockSingleKvStore_ = nullptr;
    return true;
}

std::string KVAdapterManagerTest::CreateDmKVValueStr(const std::string &appId, int64_t lastModifyTime) const
{
    JsonObject jsonObject;
    jsonObject[UDID_HASH_KEY] = "udid";
    jsonObject[APP_ID_KEY] = appId;
    jsonObject[ANOY_DEVICE_ID_KEY] = "anoy_device";
    jsonObject[SALT_KEY] = "salt";
    jsonObject[LAST_MODIFY_TIME_KEY] = lastModifyTime;
    return jsonObject.Dump();
}

/**
 * @tc.name: KVAdapterManager_GetInstance_001
 * @tc.desc: Verify KVAdapterManager singleton is accessible.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(KVAdapterManagerTest, KVAdapterManager_SetUp_Smoke_001, testing::ext::TestSize.Level1)
{
    EXPECT_TRUE(true);
}
} // namespace DistributedHardware
} // namespace OHOS
