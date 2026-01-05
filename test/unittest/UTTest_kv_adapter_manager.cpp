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

#include <algorithm>
#include <thread>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include "datetime_ex.h"

#include "dm_crypto.h"
#include "dm_constants.h"
#include "dm_anonymous.h"
#include "dm_library_manager.h"

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
constexpr const char* KV_ADAPTER_LIB_NAME = "libdmdb_kvstore.z.so";
constexpr int64_t KV_TIME_OFFSET_SECONDS = 4 * 24 * 60 * 60; // 4days
constexpr int32_t WAITFOR_RESOURCE_RELEASE_SECONDS = 130;
} // namespace

void KVAdapterManagerTest::SetUp()
{
    const int32_t permsNum = 3;
    const int32_t indexZero = 0;
    const int32_t indexOne = 1;
    const int32_t indexTwo = 2;
    uint64_t tokenId;
    const char *perms[permsNum];
    perms[indexZero] = "ohos.permission.ACCESS_SERVICE_DM";
    perms[indexOne] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[indexTwo] = "ohos.permission.DISTRIBUTED_SOFTBUS_CENTER";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "device_manager",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void KVAdapterManagerTest::TearDown()
{
}

void KVAdapterManagerTest::SetUpTestCase()
{}

void KVAdapterManagerTest::TearDownTestCase()
{
    std::this_thread::sleep_for(std::chrono::seconds(WAITFOR_RESOURCE_RELEASE_SECONDS));
}

std::string KVAdapterManagerTest::CreateDmKVValueStr(const std::string &appId, int64_t lastModifyTime) const
{
    JsonObject jsonObject;
    jsonObject[UDID_HASH_KEY] = Crypto::GetSecSalt();
    jsonObject[APP_ID_KEY] = appId;
    jsonObject[ANOY_DEVICE_ID_KEY] = Crypto::GetSecSalt();
    jsonObject[SALT_KEY] = Crypto::GetSecSalt();
    jsonObject[LAST_MODIFY_TIME_KEY] = lastModifyTime;
    return jsonObject.Dump();
}

void KVAdapterManagerTest::ConvertOsTypeToJson(int32_t osType, std::string &osTypeStr)
{
    int64_t nowTime = GetSecondsSince1970ToNow();
    JsonObject jsonObj;
    jsonObj[PEER_OSTYPE] = osType;
    jsonObj[TIME_STAMP] = nowTime;
    osTypeStr = jsonObj.Dump();
}

void KVAdapterManagerTest::RemoveDuplicates(const std::vector<std::string> &source, std::vector<std::string> &target)
{
    target.erase(
        std::remove_if(target.begin(), target.end(),
            [&source](std::string value) {
                return std::find(source.begin(), source.end(), value) != source.end();
            }),
        target.end()
    );
}

HWTEST_F(KVAdapterManagerTest, Init_001, testing::ext::TestSize.Level1)
{
    auto ret = KVAdapterManager::GetInstance().Init();
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(KVAdapterManagerTest, TestDMKVRW_001, testing::ext::TestSize.Level1)
{
    std::string appId = "com.ohos.appID_1";
    int64_t nowTime = GetSecondsSince1970ToNow();
    auto kvValueStr = CreateDmKVValueStr(appId, nowTime);
    DmKVValue kvValue;
    ConvertJsonToDmKVValue(kvValueStr, kvValue);
    auto ret = KVAdapterManager::GetInstance().PutByAnoyDeviceId(kvValue.anoyDeviceId, kvValue);
    EXPECT_EQ(ret, DM_OK);
    ret = KVAdapterManager::GetInstance().PutByAnoyDeviceId(kvValue.anoyDeviceId, kvValue);
    EXPECT_EQ(ret, DM_OK);

    DmKVValue kvValueOut1;
    ret = KVAdapterManager::GetInstance().Get(kvValue.anoyDeviceId, kvValueOut1);
    EXPECT_EQ(ret, DM_OK);
    std::string kvValueOutStr1;
    ConvertDmKVValueToJson(kvValueOut1, kvValueOutStr1);
    EXPECT_EQ(kvValueOutStr1, kvValueStr);

    ret = KVAdapterManager::GetInstance().DeleteAgedEntry();
    EXPECT_EQ(ret, DM_OK);

    DmKVValue kvValueOut2;
    ret = KVAdapterManager::GetInstance().Get(kvValue.anoyDeviceId, kvValueOut2);
    EXPECT_EQ(ret, DM_OK);
    std::string kvValueOutStr2;
    ConvertDmKVValueToJson(kvValueOut2, kvValueOutStr2);
    EXPECT_EQ(kvValueOutStr2, kvValueStr);

    ret = KVAdapterManager::GetInstance().AppUninstall(appId);
    EXPECT_EQ(ret, DM_OK);

    DmKVValue kvValueOut3;
    ret = KVAdapterManager::GetInstance().Get(kvValue.anoyDeviceId, kvValueOut3);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(KVAdapterManagerTest, TestDMKVRW_002, testing::ext::TestSize.Level1)
{
    std::string appId = "com.ohos.appID_2";
    int64_t lastModifyTime = GetSecondsSince1970ToNow() - KV_TIME_OFFSET_SECONDS;
    auto kvValueStr = CreateDmKVValueStr(appId, lastModifyTime);
    DmKVValue kvValue;
    ConvertJsonToDmKVValue(kvValueStr, kvValue);
    auto ret = KVAdapterManager::GetInstance().PutByAnoyDeviceId(kvValue.anoyDeviceId, kvValue);
    EXPECT_EQ(ret, DM_OK);
    ret = KVAdapterManager::GetInstance().PutByAnoyDeviceId(kvValue.anoyDeviceId, kvValue);
    EXPECT_EQ(ret, DM_OK);

    ret = KVAdapterManager::GetInstance().DeleteAgedEntry();
    EXPECT_EQ(ret, DM_OK);

    DmKVValue kvValueOut;
    ret = KVAdapterManager::GetInstance().Get(kvValue.anoyDeviceId, kvValueOut);
    EXPECT_EQ(ret, DM_OK);
    std::string kvValueOutStr;
    ConvertDmKVValueToJson(kvValueOut, kvValueOutStr);
    EXPECT_EQ(kvValueOutStr, kvValueStr);

    ret = KVAdapterManager::GetInstance().AppUninstall(appId);
    EXPECT_EQ(ret, DM_OK);

    DmKVValue kvValueOut2;
    ret = KVAdapterManager::GetInstance().Get(kvValue.anoyDeviceId, kvValueOut2);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(KVAdapterManagerTest, TestFreezeData_001, testing::ext::TestSize.Level1)
{
    std::string freezeKey = "freezeKey1";
    std::string freezeValue = "freezeValue1";
    auto ret = KVAdapterManager::GetInstance().PutFreezeData(freezeKey, freezeValue);
    EXPECT_EQ(ret, DM_OK);

    std::string freezeValueOut;
    ret = KVAdapterManager::GetInstance().GetFreezeData(freezeKey, freezeValueOut);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(freezeValueOut, freezeValue);

    ret = KVAdapterManager::GetInstance().DeleteFreezeData(freezeKey);
    EXPECT_EQ(ret, DM_OK);

    ret = KVAdapterManager::GetInstance().GetFreezeData(freezeKey, freezeValueOut);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(KVAdapterManagerTest, TestOSType_001, testing::ext::TestSize.Level1)
{
    std::vector<std::string> osTypeValsExist;
    KVAdapterManager::GetInstance().GetAllOstypeData(osTypeValsExist);
    int32_t osTypeExistCnt = osTypeValsExist.size();

    std::vector<std::pair<std::string, std::string>> osTypes;
    const int32_t osTypeCnt = 10;
    for (int32_t i = 0; i < osTypeCnt; i++) {
        std::string udid = Crypto::GetSecSalt();
        int32_t osType = (i % 2 == 0 ? 10 : 11);
        std::string osTypeVal;
        ConvertOsTypeToJson(osType, osTypeVal);
        osTypes.push_back({udid, osTypeVal});
    }

    for (const auto &osType : osTypes) {
        auto ret = KVAdapterManager::GetInstance().PutOstypeData(osType.first, osType.second);
        EXPECT_EQ(ret, DM_OK);
    }

    std::vector<std::string> osTypeVals;
    int32_t count = 0;
    for (int32_t j = 0; j < osTypeCnt; j++) {
        osTypeVals.clear();
        auto ret = KVAdapterManager::GetInstance().GetAllOstypeData(osTypeVals);
        EXPECT_EQ(ret, DM_OK);
        RemoveDuplicates(osTypeValsExist, osTypeVals);
        EXPECT_EQ(osTypeVals.size(), osTypeCnt - j);
        ret = KVAdapterManager::GetInstance().GetOsTypeCount(count);
        EXPECT_EQ(ret, DM_OK);
        EXPECT_EQ(count, osTypeCnt - j + osTypeExistCnt);
        ret = KVAdapterManager::GetInstance().DeleteOstypeData(osTypes[j].first);
        EXPECT_EQ(ret, DM_OK);
    }

    osTypeVals.clear();
    auto ret = KVAdapterManager::GetInstance().GetAllOstypeData(osTypeVals);
    EXPECT_EQ(osTypeVals.size(), osTypeExistCnt);
    RemoveDuplicates(osTypeValsExist, osTypeVals);
    EXPECT_EQ(osTypeVals.size(), 0);
}

HWTEST_F(KVAdapterManagerTest, TestLocalUserIdData_001, testing::ext::TestSize.Level1)
{
    std::string key = "UTTEST_LOCAL_USER_ID_KEY";
    std::string value = "UTTEST_LOCAL_USER_ID_VALUE";

    auto ret = KVAdapterManager::GetInstance().PutLocalUserIdData(key, value);
    EXPECT_EQ(ret, DM_OK);

    std::string valueOut;
    ret = KVAdapterManager::GetInstance().GetLocalUserIdData(key, valueOut);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(valueOut, value);

    std::string valueUpdate = "UTTEST_LOCAL_USER_ID_VALUE_UPDATE";
    ret = KVAdapterManager::GetInstance().PutLocalUserIdData(key, valueUpdate);
    EXPECT_EQ(ret, DM_OK);

    ret = KVAdapterManager::GetInstance().GetLocalUserIdData(key, valueOut);
    EXPECT_EQ(ret, DM_OK);
    EXPECT_EQ(valueOut, valueUpdate);
}
} // namespace DistributedHardware
} // namespace OHOS
