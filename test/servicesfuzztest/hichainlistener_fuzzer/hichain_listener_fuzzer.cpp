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
void FromJsonFuzzTest(FuzzedDataProvider &fdp)
{
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

void FromJsonInfoFuzzTest(FuzzedDataProvider &fdp)
{
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

void OnHichainDeviceUnBoundFuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();

    std::string peerUdid = fdp.ConsumeRandomLengthString();
    std::string groupInfo = fdp.ConsumeRandomLengthString();
    hiChainListener->OnHichainDeviceUnBound(peerUdid.c_str(), groupInfo.c_str());

    hiChainListener->OnHichainDeviceUnBound(nullptr, groupInfo.c_str());

    uint32_t outMaxLength = 65537;
    std::string longPeerUdid(outMaxLength, 'a');
    std::string longGroupInfo(outMaxLength, 'b');
    hiChainListener->OnHichainDeviceUnBound(longPeerUdid.c_str(), longGroupInfo.c_str());
}

void OnCredentialDeletedFuzzTest(FuzzedDataProvider &fdp)
{
    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();

    std::string credId = fdp.ConsumeRandomLengthString();
    std::string credInfo = fdp.ConsumeRandomLengthString();
    hiChainListener->OnCredentialDeleted(credId.c_str(), credInfo.c_str());

    hiChainListener->OnCredentialDeleted(nullptr, credInfo.c_str());

    uint32_t outMaxLength = 65537;
    std::string longCredId(outMaxLength, 'a');
    std::string longCredInfo(outMaxLength, 'b');
    hiChainListener->OnCredentialDeleted(longCredId.c_str(), longCredInfo.c_str());
}

void DeleteAllGroupFuzzTest(FuzzedDataProvider &fdp)
{
    std::string localUdid = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    size_t minUserIdCount = 1;
    size_t maxUserIdCount = 10;
    size_t userIdCount = fdp.ConsumeIntegralInRange<size_t>(minUserIdCount, maxUserIdCount);
    std::vector<int32_t> backgroundUserIds;
    for (size_t i = 0; i < userIdCount; ++i) {
        backgroundUserIds.push_back(fdp.ConsumeIntegral<int32_t>());
    }

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->DeleteAllGroup(localUdid, backgroundUserIds);
}

void GetRelatedGroupsFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::vector<GroupsInfo> groupList;
    GroupsInfo groupsInfo;
    groupsInfo.groupName = fdp.ConsumeRandomLengthString();
    groupsInfo.groupId = fdp.ConsumeRandomLengthString();
    groupList.push_back(groupsInfo);

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->GetRelatedGroups(userId, deviceId, groupList);
}

void GetRelatedGroupsExtFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::vector<GroupsInfo> groupList;
    GroupsInfo groupsInfo;
    groupsInfo.groupName = fdp.ConsumeRandomLengthString();
    groupsInfo.groupId = fdp.ConsumeRandomLengthString();
    groupList.push_back(groupsInfo);

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->GetRelatedGroupsExt(userId, deviceId, groupList);
}

void DeleteCredentialFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t osAccountId = fdp.ConsumeIntegral<int32_t>();
    std::string credId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->DeleteCredential(osAccountId, credId);
    hiChainListener->credManager_ = nullptr;
    hiChainListener->DeleteCredential(osAccountId, credId);
}

void DeleteGroupExtFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string groupId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->DeleteGroupExt(userId, groupId);
}

void DeleteGroupFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string groupId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);

    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->DeleteGroup(userId, groupId);
}

void GetRelatedGroupsCommonFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string deviceId = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::string pkgName = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    std::vector<GroupsInfo> groupList;
    GroupsInfo groupsInfo;
    groupsInfo.groupName = fdp.ConsumeRandomLengthString();
    groupsInfo.groupId = fdp.ConsumeRandomLengthString();
    groupList.push_back(groupsInfo);
    std::shared_ptr<HichainListener> hiChainListener = std::make_shared<HichainListener>();
    hiChainListener->GetRelatedGroupsCommon(userId, deviceId, pkgName.c_str(), groupList);
    userId = -1;
    hiChainListener->GetRelatedGroupsCommon(userId, deviceId, pkgName.c_str(), groupList);
}

void ModelHichainDeviceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::FromJsonFuzzTest(fdp);
    OHOS::DistributedHardware::FromJsonInfoFuzzTest(fdp);
    OHOS::DistributedHardware::OnHichainDeviceUnBoundFuzzTest(fdp);
    OHOS::DistributedHardware::OnCredentialDeletedFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteAllGroupFuzzTest(fdp);
    OHOS::DistributedHardware::GetRelatedGroupsFuzzTest(fdp);
    OHOS::DistributedHardware::GetRelatedGroupsExtFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteCredentialFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteGroupExtFuzzTest(fdp);
    OHOS::DistributedHardware::DeleteGroupFuzzTest(fdp);
    OHOS::DistributedHardware::GetRelatedGroupsCommonFuzzTest(fdp);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::ModelHichainDeviceFuzzTest(data, size);
    return 0;
}
