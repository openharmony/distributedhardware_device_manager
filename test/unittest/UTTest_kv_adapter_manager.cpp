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
#include "dm_constants.h"
#include "dm_anonymous.h"

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
    EXPECT_CALL(*kvDataMgr_, RegisterKvStoreServiceDeathRecipient(_)).Times(1);
    return KVAdapterManager::GetInstance().Init() == DM_OK;
}

bool KVAdapterManagerTest::UnInitKvStoreEnv()
{
    if (!kvDataMgr_ || !mockSingleKvStore_) {
        return false;
    }
    EXPECT_CALL(*kvDataMgr_, UnRegisterKvStoreServiceDeathRecipient(_)).Times(1);
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
    return SafetyDump(jsonObject);
}

HWTEST_F(KVAdapterManagerTest, Init_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(kvDataMgr_ != nullptr);
    EXPECT_CALL(*kvDataMgr_, GetSingleKvStore(_, _, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<ARG_FOURTH>(mockSingleKvStore_), Return(Status::DATA_CORRUPTED)));
    EXPECT_CALL(*kvDataMgr_, CloseKvStore(_, An<const StoreId &>())).WillRepeatedly(Return(Status::SUCCESS));
    EXPECT_CALL(*kvDataMgr_, DeleteKvStore(_, _, _)).WillRepeatedly(Return(Status::SUCCESS));
    auto ret = KVAdapterManager::GetInstance().Init();
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(KVAdapterManagerTest, ReInit_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(kvDataMgr_ != nullptr);
    EXPECT_CALL(*kvDataMgr_, GetSingleKvStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<ARG_FOURTH>(mockSingleKvStore_), Return(Status::SUCCESS)));
    EXPECT_CALL(*kvDataMgr_, RegisterKvStoreServiceDeathRecipient(_)).Times(AtLeast(1));
    EXPECT_CALL(*kvDataMgr_, UnRegisterKvStoreServiceDeathRecipient(_)).Times(AtLeast(1));
    KVAdapterManager::GetInstance().ReInit();
}

HWTEST_F(KVAdapterManagerTest, PutByAnoyDeviceId_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(mockSingleKvStore_ != nullptr);

    std::string key = "key_1";
    std::string appId = "appId_1";
    int64_t nowTime = GetSecondsSince1970ToNow();
    auto kVValueStr = CreateDmKVValueStr(appId, nowTime);
    DmKVValue kVValue;
    ConvertJsonToDmKVValue(kVValueStr, kVValue);
    {
        InSequence s;
        EXPECT_CALL(*mockSingleKvStore_, Put(_, _)).WillOnce(Return(Status::SUCCESS));
        EXPECT_CALL(*mockSingleKvStore_, Put(_, _)).WillOnce(Return(Status::SUCCESS));
    }
    auto ret = KVAdapterManager::GetInstance().PutByAnoyDeviceId(key, kVValue);
    EXPECT_EQ(ret, DM_OK);

    // @tc.expect: Kv value is existed.
    ret = KVAdapterManager::GetInstance().PutByAnoyDeviceId(key, kVValue);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(KVAdapterManagerTest, PutByAnoyDeviceId_002, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(mockSingleKvStore_ != nullptr);

    std::string key = "key_1";
    std::string appId = "appId_1";
    int64_t nowTime = GetSecondsSince1970ToNow();
    auto kVValueStr = CreateDmKVValueStr(appId, nowTime);
    DmKVValue kVValue;
    ConvertJsonToDmKVValue(kVValueStr, kVValue);
    EXPECT_CALL(*mockSingleKvStore_, Put(_, _)).WillRepeatedly(Return(Status::ERROR));
    auto ret = KVAdapterManager::GetInstance().PutByAnoyDeviceId(key, kVValue);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(KVAdapterManagerTest, PutByAnoyDeviceId_003, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(mockSingleKvStore_ != nullptr);

    std::string key = "key_1";
    std::string appId = "appId_1";
    int64_t nowTime = GetSecondsSince1970ToNow();
    auto kVValueStr = CreateDmKVValueStr(appId, nowTime);
    DmKVValue kVValue;
    ConvertJsonToDmKVValue(kVValueStr, kVValue);
    {
        InSequence s;
        EXPECT_CALL(*mockSingleKvStore_, Put(_, _)).WillOnce(Return(Status::SUCCESS));
        EXPECT_CALL(*mockSingleKvStore_, Put(_, _)).WillOnce(Return(Status::ERROR));
    }
    auto ret = KVAdapterManager::GetInstance().PutByAnoyDeviceId(key, kVValue);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(KVAdapterManagerTest, Get_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(mockSingleKvStore_ != nullptr);

    std::string key = "key_1";
    std::string appId = "appId_1";
    int64_t nowTime = GetSecondsSince1970ToNow();
    auto kVValueStr = CreateDmKVValueStr(appId, nowTime);
    DmKVValue kVValue;
    ConvertJsonToDmKVValue(kVValueStr, kVValue);
    EXPECT_CALL(*mockSingleKvStore_, Get(_, _))
        .WillOnce(DoAll(SetArgReferee<ARG_SECOND>(kVValueStr), Return(Status::SUCCESS)));
    auto ret = KVAdapterManager::GetInstance().Get(key, kVValue);
    EXPECT_EQ(ret, DM_OK);

    ret = KVAdapterManager::GetInstance().Get(key, kVValue);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(KVAdapterManagerTest, Get_002, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(mockSingleKvStore_ != nullptr);

    std::string key = "key_1";
    std::string appId = "appId_1";
    int64_t nowTime = GetSecondsSince1970ToNow();
    auto kVValueStr = CreateDmKVValueStr(appId, nowTime);
    DmKVValue kVValue;
    ConvertJsonToDmKVValue(kVValueStr, kVValue);
    EXPECT_CALL(*mockSingleKvStore_, Get(_, _))
        .WillOnce(Return(Status::ERROR));
    auto ret = KVAdapterManager::GetInstance().Get(key, kVValue);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(KVAdapterManagerTest, DeleteAgedEntry_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(mockSingleKvStore_ != nullptr);
    for (int64_t i = 0; i < 2; ++i) {
        std::string key = KEY + std::to_string(i);
        std::string appId = APPID + std::to_string(i);
        bool isTimeOutFlag = true;
        if (i % 2) {
            isTimeOutFlag = true;
        } else {
            isTimeOutFlag = false;
        }
        int64_t nowTime = !isTimeOutFlag ? GetSecondsSince1970ToNow() : ANCHOR_TIME + i;
        auto kVValueStr = CreateDmKVValueStr(appId, nowTime);
        DmKVValue kVValue;
        ConvertJsonToDmKVValue(kVValueStr, kVValue);
        {
            InSequence s;
            EXPECT_CALL(*mockSingleKvStore_, Put(_, _)).WillOnce(Return(Status::SUCCESS));
            EXPECT_CALL(*mockSingleKvStore_, Put(_, _)).WillOnce(Return(Status::SUCCESS));
        }
        KVAdapterManager::GetInstance().PutByAnoyDeviceId(key, kVValue);
    }

    auto ret = KVAdapterManager::GetInstance().DeleteAgedEntry();
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(KVAdapterManagerTest, AppUnintall_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(mockSingleKvStore_ != nullptr);
    std::map<std::string, std::string> registerMap;
    for (int64_t i = 0; i < 2; ++i) {
        std::string key = KEY + std::to_string(i);
        std::string appId = APPID + std::to_string(i);
        bool isTimeOutFlag = true;
        if (i % 2) {
            isTimeOutFlag = true;
        } else {
            isTimeOutFlag = false;
        }
        int64_t nowTime = !isTimeOutFlag ? GetSecondsSince1970ToNow() : ANCHOR_TIME + i;
        auto kVValueStr = CreateDmKVValueStr(appId, nowTime);
        DmKVValue kVValue;
        ConvertJsonToDmKVValue(kVValueStr, kVValue);
        {
            InSequence s;
            EXPECT_CALL(*mockSingleKvStore_, Put(_, _)).WillOnce(Return(Status::SUCCESS));
            EXPECT_CALL(*mockSingleKvStore_, Put(_, _)).WillOnce(Return(Status::SUCCESS));
        }
        KVAdapterManager::GetInstance().PutByAnoyDeviceId(key, kVValue);
        registerMap.emplace(key, appId);
    }
    ASSERT_FALSE(registerMap.empty());
    std::vector<DistributedKv::Entry> entrys;
    for (const auto& item : registerMap) {
        if (item.first.empty()) {
            continue;
        }
        DistributedKv::Entry entry;
        entry.key = item.first;
        entry.value = item.second;
        entrys.emplace_back(entry);
    }
    EXPECT_CALL(*mockSingleKvStore_, GetEntries(An<const Key &>(), _))
        .WillOnce(DoAll(SetArgReferee<ARG_SECOND>(entrys), Return(Status::SUCCESS)));
    EXPECT_CALL(*mockSingleKvStore_, DeleteBatch(_))
        .WillRepeatedly(Return(Status::SUCCESS));
    auto ret = KVAdapterManager::GetInstance().AppUnintall(registerMap.rbegin()->second);
    EXPECT_EQ(ret, DM_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
