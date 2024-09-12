/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "distributed_device_profile_client.h"

namespace OHOS {
namespace DistributedDeviceProfile {
void AddAccessControlProfileFirst(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = -1;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 0;
    uint32_t status = 0;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "accountId";
    std::string newAccountId = "accountId";
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "123456";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceId);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceId);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileFirst;
    profileFirst.SetAccessControlId(accesserId);
    profileFirst.SetAccesserId(accesserId);
    profileFirst.SetAccesseeId(accesserId);
    profileFirst.SetTrustDeviceId(trustDeviceId);
    profileFirst.SetBindType(bindType);
    profileFirst.SetAuthenticationType(authenticationType);
    profileFirst.SetDeviceIdType(deviceIdType);
    profileFirst.SetStatus(status);
    profileFirst.SetBindLevel(bindLevel);
    profileFirst.SetAccesser(accesser);
    profileFirst.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFirst);
}

void AddAccessControlProfileSecond(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 1;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 1;
    uint32_t status = 1;
    uint32_t authenticationType = 1;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "accountId";
    std::string newAccountId = "accountId";
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "123456";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceId);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceId);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileSecond;
    profileSecond.SetAccessControlId(accesserId);
    profileSecond.SetAccesserId(accesserId);
    profileSecond.SetAccesseeId(accesserId);
    trustDeviceId = deviceId;
    profileSecond.SetTrustDeviceId(trustDeviceId);
    profileSecond.SetBindType(bindType);
    profileSecond.SetAuthenticationType(authenticationType);
    profileSecond.SetDeviceIdType(deviceIdType);
    profileSecond.SetStatus(status);
    profileSecond.SetBindLevel(bindLevel);
    profileSecond.SetAccesser(accesser);
    profileSecond.SetAccessee(accessee);
    accessControlProfiles.push_back(profileSecond);
}

void AddAccessControlProfileThird(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 2;
    uint32_t authenticationType = 1;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "accountId";
    std::string newAccountId = "accountId";
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "123456";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceId);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceId);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileThird;
    profileThird.SetAccessControlId(accesserId);
    profileThird.SetAccesserId(accesserId);
    profileThird.SetAccesseeId(accesserId);
    profileThird.SetTrustDeviceId(trustDeviceId);
    profileThird.SetBindType(bindType);
    profileThird.SetAuthenticationType(authenticationType);
    profileThird.SetDeviceIdType(deviceIdType);
    profileThird.SetStatus(status);
    profileThird.SetBindLevel(bindLevel);
    profileThird.SetAccesser(accesser);
    profileThird.SetAccessee(accessee);
    accessControlProfiles.push_back(profileThird);
}

void AddAccessControlprofileFourth(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 1282;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 3;
    uint32_t authenticationType = 1;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "accountId";
    std::string newAccountId = "accountId";
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "123456";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceId);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceId);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileFourth;
    profileFourth.SetAccessControlId(accesserId);
    profileFourth.SetAccesserId(accesserId);
    profileFourth.SetAccesseeId(accesserId);
    profileFourth.SetTrustDeviceId(trustDeviceId);
    profileFourth.SetBindType(bindType);
    profileFourth.SetAuthenticationType(authenticationType);
    profileFourth.SetDeviceIdType(deviceIdType);
    profileFourth.SetStatus(status);
    profileFourth.SetBindLevel(bindLevel);
    profileFourth.SetAccesser(accesser);
    profileFourth.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFourth);
}

void AddAccessControlProfileFifth(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 1282;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 1;
    uint32_t status = 3;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "accountId";
    std::string newAccountId = "accountId";
    std::string deviceId = "deviceId";
    std::string trustDeviceId = "deviceId";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceId);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceId);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileFifth;
    profileFifth.SetAccessControlId(accesserId);
    profileFifth.SetAccesserId(accesserId);
    profileFifth.SetAccesseeId(accesserId);
    profileFifth.SetTrustDeviceId(trustDeviceId);
    profileFifth.SetBindType(bindType);
    profileFifth.SetAuthenticationType(authenticationType);
    profileFifth.SetDeviceIdType(deviceIdType);
    profileFifth.SetStatus(status);
    profileFifth.SetBindLevel(bindLevel);
    profileFifth.SetAccesser(accesser);
    profileFifth.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFifth);
}

void AddAccessControlProfileSixth(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 2;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 3;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "localDeviceId";
    std::string deviceIdEe = "remoteDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileFifth;
    profileFifth.SetAccessControlId(accesserId);
    profileFifth.SetAccesserId(accesserId);
    profileFifth.SetAccesseeId(accesserId);
    profileFifth.SetTrustDeviceId(trustDeviceId);
    profileFifth.SetBindType(bindType);
    profileFifth.SetAuthenticationType(authenticationType);
    profileFifth.SetDeviceIdType(deviceIdType);
    profileFifth.SetStatus(status);
    profileFifth.SetBindLevel(bindLevel);
    profileFifth.SetAccesser(accesser);
    profileFifth.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFifth);
}

void AddAccessControlProfileSeventh(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 2;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 3;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(oldAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileFifth;
    profileFifth.SetAccessControlId(accesserId);
    profileFifth.SetAccesserId(accesserId);
    profileFifth.SetAccesseeId(accesserId);
    profileFifth.SetTrustDeviceId(trustDeviceId);
    profileFifth.SetBindType(bindType);
    profileFifth.SetAuthenticationType(authenticationType);
    profileFifth.SetDeviceIdType(deviceIdType);
    profileFifth.SetStatus(status);
    profileFifth.SetBindLevel(bindLevel);
    profileFifth.SetAccesser(accesser);
    profileFifth.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFifth);
}

void AddAccessControlProfileEighth(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 1282;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 2;
    uint32_t status = 3;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "localDeviceId";
    std::string deviceIdEe = "remoteDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileFifth;
    profileFifth.SetAccessControlId(accesserId);
    profileFifth.SetAccesserId(accesserId);
    profileFifth.SetAccesseeId(accesserId);
    profileFifth.SetTrustDeviceId(trustDeviceId);
    profileFifth.SetBindType(bindType);
    profileFifth.SetAuthenticationType(authenticationType);
    profileFifth.SetDeviceIdType(deviceIdType);
    profileFifth.SetStatus(status);
    profileFifth.SetBindLevel(bindLevel);
    profileFifth.SetAccesser(accesser);
    profileFifth.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFifth);
}

void AddAccessControlProfileNinth(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 1282;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 2;
    uint32_t status = 3;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileFifth;
    profileFifth.SetAccessControlId(accesserId);
    profileFifth.SetAccesserId(accesserId);
    profileFifth.SetAccesseeId(accesserId);
    profileFifth.SetTrustDeviceId(trustDeviceId);
    profileFifth.SetBindType(bindType);
    profileFifth.SetAuthenticationType(authenticationType);
    profileFifth.SetDeviceIdType(deviceIdType);
    profileFifth.SetStatus(status);
    profileFifth.SetBindLevel(bindLevel);
    profileFifth.SetAccesser(accesser);
    profileFifth.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFifth);
}

void AddAccessControlProfileTenth(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 1282;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 1;
    uint32_t status = 3;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "localDeviceId";
    std::string deviceIdEe = "remoteDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileFifth;
    profileFifth.SetAccessControlId(accesserId);
    profileFifth.SetAccesserId(accesserId);
    profileFifth.SetAccesseeId(accesserId);
    profileFifth.SetTrustDeviceId(trustDeviceId);
    profileFifth.SetBindType(bindType);
    profileFifth.SetAuthenticationType(authenticationType);
    profileFifth.SetDeviceIdType(deviceIdType);
    profileFifth.SetStatus(status);
    profileFifth.SetBindLevel(bindLevel);
    profileFifth.SetAccesser(accesser);
    profileFifth.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFifth);
}

void AddAccessControlProfileEleventh(std::vector<AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 1282;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 1;
    uint32_t status = 3;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    AccessControlProfile profileFifth;
    profileFifth.SetAccessControlId(accesserId);
    profileFifth.SetAccesserId(accesserId);
    profileFifth.SetAccesseeId(accesserId);
    profileFifth.SetTrustDeviceId(trustDeviceId);
    profileFifth.SetBindType(bindType);
    profileFifth.SetAuthenticationType(authenticationType);
    profileFifth.SetDeviceIdType(deviceIdType);
    profileFifth.SetStatus(status);
    profileFifth.SetBindLevel(bindLevel);
    profileFifth.SetAccesser(accesser);
    profileFifth.SetAccessee(accessee);
    accessControlProfiles.push_back(profileFifth);
}

int32_t DistributedDeviceProfileClient::PutAccessControlProfile(const AccessControlProfile& accessControlProfile)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::UpdateAccessControlProfile(const AccessControlProfile& accessControlProfile)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::GetTrustDeviceProfile(const std::string& deviceId,
    TrustDeviceProfile& trustDeviceProfile)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::GetAllTrustDeviceProfile(std::vector<TrustDeviceProfile>& trustDeviceProfiles)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::GetAccessControlProfile(std::map<std::string, std::string> params,
    std::vector<AccessControlProfile>& accessControlProfiles)
{
    AddAccessControlProfileFirst(accessControlProfiles);
    AddAccessControlProfileSecond(accessControlProfiles);
    AddAccessControlProfileThird(accessControlProfiles);
    AddAccessControlprofileFourth(accessControlProfiles);
    AddAccessControlProfileFifth(accessControlProfiles);
    AddAccessControlProfileSixth(accessControlProfiles);
    AddAccessControlProfileSeventh(accessControlProfiles);
    AddAccessControlProfileEighth(accessControlProfiles);
    AddAccessControlProfileNinth(accessControlProfiles);
    AddAccessControlProfileTenth(accessControlProfiles);
    AddAccessControlProfileEleventh(accessControlProfiles);
    return 0;
}

int32_t DistributedDeviceProfileClient::GetAllAccessControlProfile(
    std::vector<AccessControlProfile>& accessControlProfiles)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::DeleteAccessControlProfile(int32_t accessControlId)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::PutServiceProfile(const ServiceProfile& serviceProfile)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::PutServiceProfileBatch(const std::vector<ServiceProfile>& serviceProfiles)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::PutCharacteristicProfile(const CharacteristicProfile& characteristicProfile)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::PutCharacteristicProfileBatch(
    const std::vector<CharacteristicProfile>& characteristicProfiles)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::GetDeviceProfile(const std::string& deviceId, DeviceProfile& deviceProfile)
{
    if (deviceId != "") {
        return 0;
    }
    return -1;
}

int32_t DistributedDeviceProfileClient::GetServiceProfile(const std::string& deviceId, const std::string& serviceName,
    ServiceProfile& serviceProfile)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::GetCharacteristicProfile(const std::string& deviceId,
    const std::string& serviceName, const std::string& characteristicId, CharacteristicProfile& characteristicProfile)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::DeleteServiceProfile(const std::string& deviceId,
    const std::string& serviceName)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::DeleteCharacteristicProfile(const std::string& deviceId,
    const std::string& serviceName, const std::string& characteristicKey)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::SubscribeDeviceProfile(const SubscribeInfo& subscribeInfo)
{
    return 0;
}

int32_t DistributedDeviceProfileClient::UnSubscribeDeviceProfile(const SubscribeInfo& subscribeInfo)
{
    return 0;
}
}
}