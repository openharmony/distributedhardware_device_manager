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
#include "hichain_listener_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "hichain_listener.h"

namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 64;
void FromJsonFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 2 + MAX_STRING_LENGTH;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    JsonObject jsonObject;
    if (fdp.ConsumeBool()) {
        jsonObject[FIELD_GROUP_TYPE] = fdp.ConsumeIntegral<int32_t>();
    }
    if (fdp.ConsumeBool()) {
        jsonObject[FIELD_USER_ID] = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    }
    if (fdp.ConsumeBool()) {
        jsonObject[FIELD_OS_ACCOUNT_ID] = fdp.ConsumeIntegral<int32_t>();
    }

    GroupInformation groupInfo;
    FromJson(jsonObject, groupInfo);
}

void FromJsonInfoFuzzTest(const uint8_t* data, size_t size)
{
    size_t requiredSize = sizeof(int32_t) * 2 + MAX_STRING_LENGTH * 4;
    if ((data == nullptr) || (size < requiredSize)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    JsonObject jsonObject;
    jsonObject[FIELD_GROUP_NAME] = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    jsonObject[FIELD_GROUP_ID] = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    jsonObject[FIELD_GROUP_OWNER] = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    jsonObject[FIELD_GROUP_TYPE] = fdp.ConsumeIntegral<int32_t>();
    jsonObject[FIELD_GROUP_VISIBILITY] = fdp.ConsumeIntegral<int32_t>();
    jsonObject[FIELD_USER_ID] = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    GroupsInfo groupInfo;
    FromJson(jsonObject, groupInfo);
}

void RegisterDataChangeCbFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->deviceGroupManager_ = nullptr;
    hiChainListener->RegisterDataChangeCb();
}

void RegisterCredentialCbFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->credManager_ = nullptr;
    hiChainListener->RegisterCredentialCb();
}

void OnHichainDeviceUnBoundFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();

    uint32_t maxLength = 65536;
    std::string peerUdid = fdp.ConsumeRandomLengthString(size > maxLength ? maxLength : size);
    std::string groupInfo = fdp.ConsumeRandomLengthString(size > maxLength ? maxLength : size);
    hiChainListener->OnHichainDeviceUnBound(peerUdid.c_str(), groupInfo.c_str());

    hiChainListener->OnHichainDeviceUnBound(nullptr, groupInfo.c_str());

    uint32_t outMaxLength = 65537;
    std::string longPeerUdid(outMaxLength, 'a');
    std::string longGroupInfo(outMaxLength, 'b');
    hiChainListener->OnHichainDeviceUnBound(longPeerUdid.c_str(), longGroupInfo.c_str());
}

void OnCredentialDeletedFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();

    uint32_t maxLength = 65536;
    std::string credId = fdp.ConsumeRandomLengthString(size > maxLength ? maxLength : size);
    std::string credInfo = fdp.ConsumeRandomLengthString(size > maxLength ? maxLength : size);
    hiChainListener->OnCredentialDeleted(credId.c_str(), credInfo.c_str());

    hiChainListener->OnCredentialDeleted(nullptr, credInfo.c_str());

    uint32_t outMaxLength = 65537;
    std::string longCredId(outMaxLength, 'a');
    std::string longCredInfo(outMaxLength, 'b');
    hiChainListener->OnCredentialDeleted(longCredId.c_str(), longCredInfo.c_str());
}

void DeleteAllGroupFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(size_t) + MAX_STRING_LENGTH)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    std::string localUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    size_t minUserIdCount = 0;
    size_t maxUserIdCount = 10;
    size_t userIdCount = fdp.ConsumeIntegralInRange<size_t>(minUserIdCount, maxUserIdCount);
    std::vector<int32_t> backgroundUserIds;
    for (size_t i = 0; i < userIdCount; ++i) {
        backgroundUserIds.push_back(fdp.ConsumeIntegral<int32_t>());
    }

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->DeleteAllGroup(localUdid, backgroundUserIds);
}

void GetRelatedGroupsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + MAX_STRING_LENGTH)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::vector<GroupsInfo> groupList;

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->GetRelatedGroups(userId, deviceId, groupList);
}

void GetRelatedGroupsExtFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + MAX_STRING_LENGTH)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::vector<GroupsInfo> groupList;

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->GetRelatedGroupsExt(userId, deviceId, groupList);
}

void DeleteCredentialFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t) + MAX_STRING_LENGTH)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);

    int32_t osAccountId = fdp.ConsumeIntegral<int32_t>();
    std::string credId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->credManager_ = nullptr;
    hiChainListener->DeleteCredential(osAccountId, credId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::FromJsonFuzzTest(data, size);
    OHOS::DistributedHardware::FromJsonInfoFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterDataChangeCbFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterCredentialCbFuzzTest(data, size);
    OHOS::DistributedHardware::OnHichainDeviceUnBoundFuzzTest(data, size);
    OHOS::DistributedHardware::OnCredentialDeletedFuzzTest(data, size);
    OHOS::DistributedHardware::DeleteAllGroupFuzzTest(data, size);
    OHOS::DistributedHardware::GetRelatedGroupsFuzzTest(data, size);
    OHOS::DistributedHardware::GetRelatedGroupsExtFuzzTest(data, size);
    OHOS::DistributedHardware::DeleteCredentialFuzzTest(data, size);
    return 0;
}
