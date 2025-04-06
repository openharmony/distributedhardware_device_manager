/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "UTTest_dm_deviceprofile_connector.h"

#include "dm_constants.h"
#include "dm_device_info.h"
#include "deviceprofile_connector.h"
#include "dp_inited_callback_stub.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void DeviceProfileConnectorTest::SetUp()
{
}

void DeviceProfileConnectorTest::TearDown()
{
}

void DeviceProfileConnectorTest::SetUpTestCase()
{
    multipleUserConnectorMock_ = std::make_shared<MultipleUserConnectorMock>();
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
    cryptoMock_ = std::make_shared<CryptoMock>();
    DmCrypto::dmCrypto = cryptoMock_;
}

void DeviceProfileConnectorTest::TearDownTestCase()
{
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
    DmCrypto::dmCrypto = nullptr;
    cryptoMock_ = nullptr;
}

class MockDpInitedCallback : public DistributedDeviceProfile::DpInitedCallbackStub {
public:
    MockDpInitedCallback() {}
    ~MockDpInitedCallback() {}
    int32_t OnDpInited()
    {
        return DM_OK;
    }
};

void AddAccessControlProfileFirst(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 1;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "123456";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile profileFifth;
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

void AddAccessControlProfileFirst(DistributedDeviceProfile::AccessControlProfile& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 1;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "123456";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    accessControlProfiles.SetAccessControlId(accesserId);
    accessControlProfiles.SetAccesserId(accesserId);
    accessControlProfiles.SetAccesseeId(accesserId);
    accessControlProfiles.SetTrustDeviceId(trustDeviceId);
    accessControlProfiles.SetBindType(bindType);
    accessControlProfiles.SetAuthenticationType(authenticationType);
    accessControlProfiles.SetDeviceIdType(deviceIdType);
    accessControlProfiles.SetStatus(status);
    accessControlProfiles.SetBindLevel(bindLevel);
    accessControlProfiles.SetAccesser(accesser);
    accessControlProfiles.SetAccessee(accessee);
}

void AddAccessControlProfileSecond(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "localDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile profileFifth;
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

void AddAccessControlProfileThird(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile profileFifth;
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

void AddAccessControlProfileForth(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 2;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "localDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile profileFifth;
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

void AddAccessControlProfileFifth(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 2;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile profileFifth;
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

void AddAccessControlProfileSix(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 2;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile profileFifth;
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

void AddAccessControlProfile001(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "remoteDeviceId";
    std::string deviceIdEe = "localDeviceId";
    std::string trustDeviceId = "localDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile profileFifth;
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

void AddAccessControlProfile002(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 3;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "localDeviceId";
    std::string deviceIdEe = "remoteDeviceId";
    std::string trustDeviceId = "localDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName2");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName1");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    DistributedDeviceProfile::AccessControlProfile profileFifth;
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

void AddAccessControlProfileSeven(DistributedDeviceProfile::AccessControlProfile& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 256;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 2;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "localDeviceId";
    std::string deviceIdEe = "remoteDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(userId);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    accessControlProfiles.SetAccessControlId(accesserId);
    accessControlProfiles.SetAccesserId(accesserId);
    accessControlProfiles.SetAccesseeId(accesserId);
    accessControlProfiles.SetTrustDeviceId(trustDeviceId);
    accessControlProfiles.SetBindType(bindType);
    accessControlProfiles.SetAuthenticationType(authenticationType);
    accessControlProfiles.SetDeviceIdType(deviceIdType);
    accessControlProfiles.SetStatus(status);
    accessControlProfiles.SetBindLevel(bindLevel);
    accessControlProfiles.SetAccesser(accesser);
    accessControlProfiles.SetAccessee(accessee);
}

void AddAccessControlProfileEight(DistributedDeviceProfile::AccessControlProfile& accessControlProfiles)
{
    int32_t userId = 123456;
    int32_t bindType = 1;
    int32_t deviceIdType = 1;
    uint32_t bindLevel = 2;
    uint32_t status = 1;
    uint32_t authenticationType = 2;
    uint32_t accesserId = 1;
    uint32_t tokenId = 1001;

    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    std::string deviceIdEr = "localDeviceId";
    std::string deviceIdEe = "remoteDeviceId";
    std::string trustDeviceId = "remoteDeviceId";

    DistributedDeviceProfile::Accesser accesser;
    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(deviceIdEr);
    accesser.SetAccesserUserId(userId);
    accesser.SetAccesserAccountId(oldAccountId);
    accesser.SetAccesserTokenId(tokenId);
    accesser.SetAccesserBundleName("bundleName1");
    accesser.SetAccesserHapSignature("uph1");
    accesser.SetAccesserBindLevel(bindLevel);

    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeId(accesserId);
    accessee.SetAccesseeDeviceId(deviceIdEe);
    accessee.SetAccesseeUserId(0);
    accessee.SetAccesseeAccountId(newAccountId);
    accessee.SetAccesseeTokenId(tokenId);
    accessee.SetAccesseeBundleName("bundleName2");
    accessee.SetAccesseeHapSignature("uph1");
    accessee.SetAccesseeBindLevel(bindLevel);

    accessControlProfiles.SetAccessControlId(accesserId);
    accessControlProfiles.SetAccesserId(accesserId);
    accessControlProfiles.SetAccesseeId(accesserId);
    accessControlProfiles.SetTrustDeviceId(trustDeviceId);
    accessControlProfiles.SetBindType(bindType);
    accessControlProfiles.SetAuthenticationType(authenticationType);
    accessControlProfiles.SetDeviceIdType(deviceIdType);
    accessControlProfiles.SetStatus(status);
    accessControlProfiles.SetBindLevel(bindLevel);
    accessControlProfiles.SetAccesser(accesser);
    accessControlProfiles.SetAccessee(accessee);
}

void GetAccessControlProfiles(std::vector<DistributedDeviceProfile::AccessControlProfile>& accessControlProfiles)
{
    AddAccessControlProfileFirst(accessControlProfiles);
    AddAccessControlProfileSecond(accessControlProfiles);
    AddAccessControlProfileThird(accessControlProfiles);
    AddAccessControlProfileForth(accessControlProfiles);
    AddAccessControlProfileFifth(accessControlProfiles);
    AddAccessControlProfileSix(accessControlProfiles);
}
HWTEST_F(DeviceProfileConnectorTest, GetAccessControlProfile_001, testing::ext::TestSize.Level1)
{
    auto ret = DeviceProfileConnector::GetInstance().GetAccessControlProfile();
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, GetAppTrustDeviceList_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string deviceId = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetAppTrustDeviceList_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetAppTrustDeviceList_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "deviceId_003";
    auto ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetDeviceAclParam_001, testing::ext::TestSize.Level1)
{
    DmDiscoveryInfo discoveryInfo;
    bool isonline = true;
    int32_t authForm = 0;
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).Times(::testing::AtLeast(38)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceProfileConnector::GetInstance().GetDeviceAclParam(discoveryInfo, isonline, authForm);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_IDENTICAL_ACCOUNT);
    DmDiscoveryInfo discoveryInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, IDENTICAL_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(DEVICE);
    DmDiscoveryInfo discoveryInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, PEER_TO_PEER);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_003, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(APP);
    profiles.accesser_.SetAccesserBundleName("ohos_test");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "ohos_test";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, PEER_TO_PEER);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_004, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("ohos_test");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "ohos_test";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, PEER_TO_PEER);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_005, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(DEVICE);
    DmDiscoveryInfo discoveryInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, ACROSS_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_006, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(APP);
    profiles.accesser_.SetAccesserBundleName("pkgName");
    profiles.accesser_.SetAccesserDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "pkgName";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, ACROSS_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_007, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(APP);
    profiles.accessee_.SetAccesseeBundleName("pkgName");
    profiles.accessee_.SetAccesseeDeviceId("localDeviceId");
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.pkgname = "pkgName";
    discoveryInfo.localDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, ACROSS_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDmAuthForm_008, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    uint32_t invalidType = 10;
    profiles.SetBindType(invalidType);
    DmDiscoveryInfo discoveryInfo;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleDmAuthForm(profiles, discoveryInfo);
    EXPECT_EQ(ret, INVALID_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, CheckBindType_001, testing::ext::TestSize.Level1)
{
    std::string trustDeviceId = "trustDeviceId";
    std::string requestDeviceId = "requestDeviceId";
    uint32_t ret = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    EXPECT_EQ(ret, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, CheckBindType_002, testing::ext::TestSize.Level1)
{
    std::string trustDeviceId = "deviceId";
    std::string requestDeviceId = "requestDeviceId";
    uint32_t ret = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    EXPECT_NE(ret, IDENTICAL_ACCOUNT_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, CheckBindType_003, testing::ext::TestSize.Level1)
{
    std::string trustDeviceId = "deviceId";
    std::string requestDeviceId = "deviceId";
    uint32_t ret = DeviceProfileConnector::GetInstance().CheckBindType(trustDeviceId, requestDeviceId);
    EXPECT_NE(ret, IDENTICAL_ACCOUNT_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, GetBindTypeByPkgName_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string requestDeviceId;
    std::string trustUdid;
    auto ret = DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetBindTypeByPkgName_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string requestDeviceId;
    std::string trustUdid = "123456";
    auto ret = DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    EXPECT_EQ(ret.empty(), true);

    requestDeviceId = "remoteDeviceId";
    trustUdid = "localDeviceId";
    ret = DeviceProfileConnector::GetInstance().GetBindTypeByPkgName(pkgName, requestDeviceId, trustUdid);
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, CompareBindType_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId("deviceId");
    profile.SetStatus(INACTIVE);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles.push_back(profile);
    std::string pkgName;
    std::vector<int32_t> sinkBindType;
    std::string localDeviceId;
    std::string targetDeviceId = "targetDeviceId";
    auto ret = DeviceProfileConnector::GetInstance().CompareBindType(profiles, pkgName, sinkBindType, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, CompareBindType_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId("targetDeviceId");
    profile.SetStatus(ACTIVE);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles.push_back(profile);
    std::string pkgName;
    std::vector<int32_t> sinkBindType;
    std::string localDeviceId;
    std::string targetDeviceId = "targetDeviceId";
    auto ret = DeviceProfileConnector::GetInstance().CompareBindType(profiles, pkgName, sinkBindType, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, CompareBindType_003, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    profile.SetTrustDeviceId("targetDeviceId");
    profile.SetStatus(INACTIVE);
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles.push_back(profile);
    std::string pkgName;
    std::vector<int32_t> sinkBindType;
    std::string localDeviceId;
    std::string targetDeviceId = "targetDeviceId";
    auto ret = DeviceProfileConnector::GetInstance().CompareBindType(profiles, pkgName, sinkBindType, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, SyncAclByBindType_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::vector<int32_t> bindTypeVec;
    std::string localDeviceId;
    std::string targetDeviceId;
    auto ret = DeviceProfileConnector::GetInstance().SyncAclByBindType(pkgName, bindTypeVec, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, SyncAclByBindType_002, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::vector<int32_t> bindTypeVec;
    std::string localDeviceId;
    std::string targetDeviceId = "123456";
    bindTypeVec.push_back(IDENTICAL_ACCOUNT_TYPE);
    bindTypeVec.push_back(DEVICE_PEER_TO_PEER_TYPE);
    auto ret = DeviceProfileConnector::GetInstance().SyncAclByBindType(pkgName, bindTypeVec, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, SyncAclByBindType_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::vector<int32_t> bindTypeVec;
    std::string localDeviceId = "deviceId";
    std::string targetDeviceId = "remoteDeviceId";
    bindTypeVec.push_back(DEVICE_ACROSS_ACCOUNT_TYPE);
    bindTypeVec.push_back(APP_ACROSS_ACCOUNT_TYPE);
    auto ret = DeviceProfileConnector::GetInstance().SyncAclByBindType(pkgName, bindTypeVec, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), false);
}

HWTEST_F(DeviceProfileConnectorTest, SyncAclByBindType_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::vector<int32_t> bindTypeVec;
    std::string localDeviceId = "deviceId";
    std::string targetDeviceId = "remoteDeviceId";
    bindTypeVec.push_back(DEVICE_PEER_TO_PEER_TYPE);
    bindTypeVec.push_back(IDENTICAL_ACCOUNT_TYPE);
    auto ret = DeviceProfileConnector::GetInstance().SyncAclByBindType(pkgName, bindTypeVec, localDeviceId,
        targetDeviceId);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetProcessInfoFromAclByUserId_001, testing::ext::TestSize.Level1)
{
    std::string localDeviceId = "localDeviceId";
    std::string targetDeviceId = "targetDeviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId, targetDeviceId, 100);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetProcessInfoFromAclByUserId_002, testing::ext::TestSize.Level1)
{
    std::string localDeviceId = "123456";
    std::string targetDeviceId = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId, targetDeviceId, 100);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, GetProcessInfoFromAclByUserId_003, testing::ext::TestSize.Level1)
{
    std::string localDeviceId = "deviceId";
    std::string targetDeviceId = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId, targetDeviceId, 100);
    EXPECT_EQ(ret.empty(), true);
}

HWTEST_F(DeviceProfileConnectorTest, PutAccessControlList_001, testing::ext::TestSize.Level1)
{
    DmAclInfo aclInfo;
    DmAccesser dmAccesser;
    DmAccessee dmAccessee;
    int32_t userId = 123456;
    std::string localDeviceId = "deviceId";
    std::vector<std::string> peerUdids;
    std::multimap<std::string, int32_t> peerUserIdMap;
    DeviceProfileConnector::GetInstance().DeleteAclForUserRemoved(localDeviceId, userId, peerUdids, peerUserIdMap);
    int32_t ret = DeviceProfileConnector::GetInstance().PutAccessControlList(aclInfo, dmAccesser, dmAccessee);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAccessControlList_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    std::string oldAccountId;
    std::string newAccountId;
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, oldAccountId, newAccountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAccessControlList_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 123456;
    std::string oldAccountId = "oldAccountId";
    std::string newAccountId = "newAccountId";
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, oldAccountId, newAccountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAccessControlList_003, testing::ext::TestSize.Level1)
{
    int32_t userId = 123456;
    std::string oldAccountId = "accountId";
    std::string newAccountId = "newAccountId";
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, oldAccountId, newAccountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAccessControlList_004, testing::ext::TestSize.Level1)
{
    int32_t userId = 123456;
    std::string oldAccountId = "accountId";
    std::string newAccountId = "accountId";
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAccessControlList(userId, oldAccountId, newAccountId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_003, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(DEVICE);
    std::string targetDeviceId = "targetDeviceId";
    std::string localDeviceId = "localDeviceId";
    uint32_t index = 0;
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles,
        localDeviceId, sinkBindType, bindTypeIndex, index, targetDeviceId);
    EXPECT_EQ(sinkBindType, vector<int32_t>({DEVICE_PEER_TO_PEER_TYPE}));
    EXPECT_EQ(bindTypeIndex, vector<int32_t>({0}));
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_004, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(DEVICE);
    std::string targetDeviceId = "targetDeviceId";
    std::string localDeviceId = "localDeviceId";
    uint32_t index = 0;
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles,
        localDeviceId, sinkBindType, bindTypeIndex, index, targetDeviceId);
    EXPECT_EQ(sinkBindType, vector<int32_t>({DEVICE_ACROSS_ACCOUNT_TYPE}));
    EXPECT_EQ(bindTypeIndex, vector<int32_t>({0}));
}

HWTEST_F(DeviceProfileConnectorTest, CheckIdenticalAccount_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 0;
    std::string accountId;
    bool ret = DeviceProfileConnector::GetInstance().CheckIdenticalAccount(userId, accountId);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDevIdInAclForDevBind_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string deviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDevIdInAclForDevBind_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "123456";
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDevIdInAclForDevBind_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "deviceId";
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSinkDevIdInAclForDevBind_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string deviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSinkDevIdInAclForDevBind_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "123456";
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSinkDevIdInAclForDevBind_003, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "deviceId";
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, CheckDevIdInAclForDevBind_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string deviceId;
    bool ret = DeviceProfileConnector::GetInstance().CheckDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteTimeOutAcl_001, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    int32_t peerUserId = -1;
    uint32_t ret = DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(deviceId, peerUserId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetTrustNumber_001, testing::ext::TestSize.Level1)
{
    std::string deviceId;
    int32_t ret = DeviceProfileConnector::GetInstance().GetTrustNumber(deviceId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, IsSameAccount_001, testing::ext::TestSize.Level1)
{
    std::string udid = "udid";
    int32_t ret = DeviceProfileConnector::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorTest, IsSameAccount_002, testing::ext::TestSize.Level1)
{
    std::string udid = "deviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, GetAuthForm_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    std::string trustDev = "";
    std::string reqDev = "";
    int32_t ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, INVALIED_TYPE);
    profile.SetBindType(DM_IDENTICAL_ACCOUNT);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, IDENTICAL_ACCOUNT_TYPE);
    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(DEVICE);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, DEVICE_PEER_TO_PEER_TYPE);
    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(DEVICE);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, DEVICE_ACROSS_ACCOUNT_TYPE);
    profile.SetBindLevel(APP);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, APP_ACROSS_ACCOUNT_TYPE);
    profile.SetBindType(INVALIED_TYPE);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, INVALIED_TYPE);

    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(APP);
    ret = DeviceProfileConnector::GetInstance().GetAuthForm(profile, trustDev, reqDev);
    EXPECT_EQ(ret, APP_PEER_TO_PEER_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, GetBindLevel_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string localUdid = "localDeviceId";
    std::string udid = "remoteDeviceId";
    uint64_t tokenId = 0;
    int32_t bindLevel = INVALIED_TYPE;

    bindLevel = DeviceProfileConnector::GetInstance()
        .GetBindLevel(pkgName, localUdid, udid, tokenId);

    EXPECT_EQ(bindLevel, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateBindType_001, testing::ext::TestSize.Level1)
{
    std::string udid = "deviceId";
    int32_t bindType = DEVICE;
    std::map<std::string, int32_t> deviceMap;
    deviceMap[udid] = APP;
    DeviceProfileConnector::GetInstance().UpdateBindType(udid, bindType, deviceMap);
    EXPECT_EQ(deviceMap[udid], DEVICE);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateBindType_002, testing::ext::TestSize.Level1)
{
    std::string udid = "deviceId";
    int32_t bindType = DEVICE;
    std::map<std::string, int32_t> deviceMap;
    DeviceProfileConnector::GetInstance().UpdateBindType(udid, bindType, deviceMap);
    EXPECT_EQ(deviceMap[udid], DEVICE);
}

HWTEST_F(DeviceProfileConnectorTest, HandleAccountLogoutEvent_001, testing::ext::TestSize.Level1)
{
    int32_t remoteUserId = 0;
    int32_t bindType = DM_INVALIED_BINDTYPE;
    std::string remoteAccountHash = "remoteAccountHash";
    std::string remoteUdid = "1";
    std::string localUdid = "localDeviceId";

    bindType = DeviceProfileConnector::GetInstance().HandleAccountLogoutEvent(remoteUserId,
        remoteAccountHash, remoteUdid, localUdid);
    EXPECT_EQ(bindType, DM_INVALIED_BINDTYPE);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDevUnBindEvent_001, testing::ext::TestSize.Level1)
{
    int32_t remoteUserId = 0;
    std::string remoteUdid = "remoteDeviceId";
    std::string localUdid = "localDeviceId";
    int32_t bindType = DM_INVALIED_BINDTYPE;

    bindType = DeviceProfileConnector::GetInstance().HandleDevUnBindEvent(remoteUserId, remoteUdid, localUdid);
    EXPECT_EQ(bindType, DM_INVALIED_BINDTYPE);
}

HWTEST_F(DeviceProfileConnectorTest, HandleAppUnBindEvent_001, testing::ext::TestSize.Level1)
{
    int32_t remoteUserId = 0;
    int32_t tokenId = 0;
    std::string remoteUdid = "remoteDeviceId";
    std::string localUdid = "localDeviceId";
    std::string pkgName = "";
    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId,
        remoteUdid, tokenId, localUdid);
    EXPECT_NE(offlineParam.leftAclNumber, 0);

    int32_t peerTokenId = 1;
    offlineParam = DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid, tokenId,
        localUdid, peerTokenId);
    EXPECT_EQ(offlineParam.leftAclNumber, 0);
}

HWTEST_F(DeviceProfileConnectorTest, SingleUserProcess_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    DmAccessCaller caller;
    DmAccessCallee callee;
    int32_t ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, false);
    profile.SetBindType(DM_IDENTICAL_ACCOUNT);
    profile.accessee_.SetAccesseeBundleName("pkgName");
    profile.accessee_.SetAccesseeDeviceId("localDeviceId");
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(DEVICE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindLevel(APP);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindLevel(SERVICE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(DEVICE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindLevel(APP);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindLevel(SERVICE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    profile.SetBindType(INVALIED_TYPE);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DeviceProfileConnectorTest, GetAccessControlProfileByUserId_001, testing::ext::TestSize.Level1)
{
    int32_t userId = DEVICE;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    profiles = DeviceProfileConnector::GetInstance().GetAccessControlProfileByUserId(userId);
    EXPECT_GE(profiles.size(), 0);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAppBindLevel_001, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    std::string pkgName = "bundleName1";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string localUdid = "remoteDeviceId";
    std::string remoteUdid = "localDeviceId";
    GetAccessControlProfiles(profiles);
    DeviceProfileConnector::GetInstance().DeleteAppBindLevel(offlineParam, pkgName, profiles, localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, APP);

    pkgName = "bundleName2";
    localUdid = "localDeviceId";
    remoteUdid="remoteDeviceId";
    DeviceProfileConnector::GetInstance().DeleteAppBindLevel(offlineParam, pkgName, profiles, localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, APP);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAppBindLevel_003, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    std::string pkgName = "bundleName2";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string localUdid = "localDeviceId";
    std::string remoteUdid="remoteDeviceId";
    std::string extra = "bundleName1";
    GetAccessControlProfiles(profiles);
    DeviceProfileConnector::GetInstance().DeleteAppBindLevel(offlineParam,
        pkgName, profiles, localUdid, remoteUdid, extra);
    EXPECT_EQ(offlineParam.bindType, APP);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteDeviceBindLevel_001, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string localUdid = "remoteDeviceId";
    std::string remoteUdid="localDeviceId";
    GetAccessControlProfiles(profiles);
    DeviceProfileConnector::GetInstance().DeleteDeviceBindLevel(offlineParam, profiles, localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, DEVICE);

    localUdid = "localDeviceId";
    remoteUdid="remoteDeviceId";
    DeviceProfileConnector::GetInstance().DeleteDeviceBindLevel(offlineParam, profiles, localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, DEVICE);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteServiceBindLevel_001, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    std::string pkgName = "bundleName1";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string localUdid = "remoteDeviceId";
    std::string remoteUdid="localDeviceId";
    GetAccessControlProfiles(profiles);
    DeviceProfileConnector::GetInstance().DeleteServiceBindLevel(offlineParam, pkgName, profiles,
        localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, SERVICE);

    pkgName = "bundleName2";
    localUdid = "localDeviceId";
    remoteUdid="remoteDeviceId";
    DeviceProfileConnector::GetInstance().DeleteServiceBindLevel(offlineParam, pkgName, profiles,
        localUdid, remoteUdid);
    EXPECT_EQ(offlineParam.bindType, SERVICE);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDevIdInAclForDevBind_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "localDeviceId";
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteTimeOutAcl_002, testing::ext::TestSize.Level1)
{
    std::string deviceId = "remoteDeviceId";
    int32_t peerUserId = 0;
    uint32_t ret = DeviceProfileConnector::GetInstance().DeleteTimeOutAcl(deviceId, peerUserId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetTrustNumber_002, testing::ext::TestSize.Level1)
{
    std::string deviceId = "remoteDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().GetTrustNumber(deviceId);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, IsSameAccount_003, testing::ext::TestSize.Level1)
{
    std::string udid = "123456";
    int32_t ret = DeviceProfileConnector::GetInstance().IsSameAccount(udid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckAccessControl_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    std::string srcUdid;
    DmAccessCallee callee;
    std::string sinkUdid;
    int32_t ret = DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    srcUdid = "123456";
    sinkUdid = "123456";
    ret = DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckIsSameAccount_001, testing::ext::TestSize.Level1)
{
    DmAccessCaller caller;
    std::string srcUdid;
    DmAccessCallee callee;
    std::string sinkUdid;
    int32_t ret = DeviceProfileConnector::GetInstance().CheckIsSameAccount(caller, srcUdid, callee, sinkUdid);
    EXPECT_EQ(ret, ERR_DM_FAILED);

    srcUdid = "123456";
    sinkUdid = "123456";
    ret = DeviceProfileConnector::GetInstance().CheckIsSameAccount(caller, srcUdid, callee, sinkUdid);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, HandleAccountLogoutEvent_002, testing::ext::TestSize.Level1)
{
    int32_t remoteUserId = 0;
    int32_t bindType = DM_INVALIED_BINDTYPE;
    std::string remoteAccountHash = "remoteAccountHash";
    std::string remoteUdid = "123456";
    std::string localUdid = "localDeviceId";

    bindType = DeviceProfileConnector::GetInstance().HandleAccountLogoutEvent(remoteUserId,
        remoteAccountHash, remoteUdid, localUdid);
    EXPECT_NE(bindType, DM_IDENTICAL_ACCOUNT);

    localUdid = "remoteDeviceId";
    remoteUdid = "localDeviceId";
    bindType = DeviceProfileConnector::GetInstance().HandleAccountLogoutEvent(remoteUserId,
        remoteAccountHash, remoteUdid, localUdid);
    EXPECT_EQ(bindType, DM_IDENTICAL_ACCOUNT);

    int32_t remoteId = 456;
    remoteUserId = remoteId;
    bindType = DeviceProfileConnector::GetInstance().HandleAccountLogoutEvent(remoteUserId,
        remoteAccountHash, remoteUdid, localUdid);
    EXPECT_EQ(bindType, DM_IDENTICAL_ACCOUNT);
}

HWTEST_F(DeviceProfileConnectorTest, HandleDevUnBindEvent_002, testing::ext::TestSize.Level1)
{
    int32_t remoteUserId = 0;
    std::string remoteUdid;
    std::string localUdid = "localDeviceId";
    int32_t bindType = DM_INVALIED_BINDTYPE;
    bindType = DeviceProfileConnector::GetInstance().HandleDevUnBindEvent(remoteUserId, remoteUdid, localUdid);
    EXPECT_EQ(bindType, DM_INVALIED_BINDTYPE);

    remoteUdid = "123456";
    bindType = DeviceProfileConnector::GetInstance().HandleDevUnBindEvent(remoteUserId, remoteUdid, localUdid);
    EXPECT_EQ(bindType, DM_INVALIED_BINDTYPE);

    remoteUdid = "localDeviceId";
    remoteUserId = 1234;
    bindType = DeviceProfileConnector::GetInstance().HandleDevUnBindEvent(remoteUserId, remoteUdid, localUdid);
    EXPECT_EQ(bindType, DM_IDENTICAL_ACCOUNT);

    remoteUserId = 456;
    bindType = DeviceProfileConnector::GetInstance().HandleDevUnBindEvent(remoteUserId, remoteUdid, localUdid);
    EXPECT_EQ(bindType, 3);
}

HWTEST_F(DeviceProfileConnectorTest, GetAllAccessControlProfile_001, testing::ext::TestSize.Level1)
{
    int64_t accessControlId = 1;
    DeviceProfileConnector::GetInstance().DeleteAccessControlById(accessControlId);
    auto ret = DeviceProfileConnector::GetInstance().GetAllAccessControlProfile();
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, GetAclProfileByDeviceIdAndUserId_001, testing::ext::TestSize.Level1)
{
    std::string deviceId = "deviceId";
    int32_t userId = 123456;
    auto ret = DeviceProfileConnector::GetInstance().GetAclProfileByDeviceIdAndUserId(deviceId, userId
    );
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAclForAccountLogOut_001, testing::ext::TestSize.Level1)
{
    std::string localUdid;
    int32_t localUserId = 444;
    std::string peerUdid = "deviceId";
    int32_t peerUserId = 555;
    bool ret = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, localUserId,
        peerUdid, peerUserId);
    EXPECT_FALSE(ret);

    localUdid = "deviceId";
    localUserId = 123456;
    peerUdid = "deviceId";
    peerUserId = 456;
    ret = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, localUserId, peerUdid, peerUserId);
    EXPECT_FALSE(ret);

    localUdid = "deviceId";
    localUserId = 123456;
    peerUdid = "deviceId";
    peerUserId = 123456;
    ret = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, localUserId, peerUdid, peerUserId);
    EXPECT_TRUE(ret);

    localUdid = "localDeviceId";
    localUserId = 123456;
    peerUdid = "remoteDeviceId";
    peerUserId = 123456;
    ret = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, localUserId, peerUdid, peerUserId);
    EXPECT_FALSE(ret);

    localUdid = "remoteDeviceId";
    localUserId = 1234;
    peerUdid = "localDeviceId";
    peerUserId = 1234;
    ret = DeviceProfileConnector::GetInstance().DeleteAclForAccountLogOut(localUdid, localUserId, peerUdid, peerUserId);
    EXPECT_TRUE(ret);
}

HWTEST_F(DeviceProfileConnectorTest, GetProcessInfoFromAclByUserId_005, testing::ext::TestSize.Level1)
{
    std::string localDeviceId = "deviceId";
    std::string targetDeviceId = "deviceId";
    int32_t userId = 123456;
    auto ret = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId,
        targetDeviceId, userId);
    EXPECT_FALSE(ret.empty());

    localDeviceId = "remoteDeviceId";
    targetDeviceId = "localDeviceId";
    userId = 456;
    ret = DeviceProfileConnector::GetInstance().GetProcessInfoFromAclByUserId(localDeviceId,
        targetDeviceId, userId);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, GetDeviceIdAndBindLevel_001, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> userIds;
    int32_t userId = 123456;
    int32_t localId = 456;
    userIds.push_back(userId);
    userIds.push_back(localId);
    std::string localUdid = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndBindLevel(userIds, localUdid);
    EXPECT_FALSE(ret.empty());

    localUdid = "localDeviceId";
    ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndBindLevel(userIds, localUdid);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, GetDeviceIdAndUserId_001, testing::ext::TestSize.Level1)
{
    int32_t userId = 123456;
    std::string accountId;
    std::string localUdid = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndUserId(userId, accountId, localUdid);
    EXPECT_FALSE(ret.empty());

    localUdid = "localDeviceId";
    userId = 456;
    ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndUserId(userId, accountId, localUdid);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, HandleUserSwitched_001, testing::ext::TestSize.Level1)
{
    int32_t currentUserId = 0;
    std::string localUdid = "deviceId";
    int32_t beforeUserId = 123456;
    std::vector<std::string> deviceVec;
    int32_t ret = DeviceProfileConnector::GetInstance().HandleUserSwitched(localUdid, deviceVec, currentUserId,
        beforeUserId);
    EXPECT_EQ(ret, DM_OK);

    beforeUserId = 1234;
    currentUserId = 123456;
    ret = DeviceProfileConnector::GetInstance().HandleUserSwitched(localUdid, deviceVec, currentUserId, beforeUserId);
    EXPECT_EQ(ret, DM_OK);

    std::vector<int32_t> remoteUserIds;
    remoteUserIds.push_back(currentUserId);
    std::string remoteUdid = "deviceId";
    std::vector<int32_t> localUserIds;
    localUserIds.push_back(currentUserId);
    DeviceProfileConnector::GetInstance().HandleSyncForegroundUserIdEvent(remoteUserIds, remoteUdid,
        localUserIds, localUdid);

    localUdid = "remoteDeviceId";
    remoteUdid = "localDeviceId";
    int32_t localdeviceId = 456;
    remoteUserIds.push_back(localdeviceId);
    localUserIds.push_back(localdeviceId);
    DeviceProfileConnector::GetInstance().HandleSyncForegroundUserIdEvent(remoteUserIds, remoteUdid,
        localUserIds, localUdid);
}

HWTEST_F(DeviceProfileConnectorTest, GetOfflineProcessInfo_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "deviceId";
    std::vector<int32_t> localUserIds;
    std::string remoteUdid = "deviceId";
    std::vector<int32_t> remoteUserIds;
    int32_t userId = 123456;
    remoteUserIds.push_back(userId);
    localUserIds.push_back(userId);
    auto ret = DeviceProfileConnector::GetInstance().GetOfflineProcessInfo(localUdid, localUserIds, remoteUdid,
        remoteUserIds);
    EXPECT_FALSE(ret.empty());

    localUdid = "remoteDeviceId";
    remoteUdid = "localDeviceId";
    int32_t localdeviceId = 456;
    remoteUserIds.push_back(localdeviceId);
    localUserIds.push_back(localdeviceId);
    ret = DeviceProfileConnector::GetInstance().GetOfflineProcessInfo(localUdid, localUserIds, remoteUdid,
        remoteUserIds);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, GetUserIdAndBindLevel_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "deviceId";
    std::string peerUdid = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetUserIdAndBindLevel(localUdid, peerUdid);
    EXPECT_FALSE(ret.empty());

    localUdid = "remoteDeviceId";
    peerUdid = "localDeviceId";
    ret = DeviceProfileConnector::GetInstance().GetUserIdAndBindLevel(localUdid, peerUdid);
    EXPECT_FALSE(ret.empty());

    localUdid = "remoteDeviceIdee";
    peerUdid = "localDeviceIder";
    ret = DeviceProfileConnector::GetInstance().GetUserIdAndBindLevel(localUdid, peerUdid);
    EXPECT_FALSE(ret.empty());

    int32_t userId = 123456;
    std::vector<int32_t> localUserIds;
    localUserIds.push_back(userId);
    std::string remoteUdid = "deviceId";
    std::vector<int32_t> remoteFrontUserIds;
    remoteFrontUserIds.push_back(userId);
    std::vector<int32_t> remoteBackUserIds;
    remoteBackUserIds.push_back(userId);
    DeviceProfileConnector::GetInstance().UpdateACL(localUdid, localUserIds, remoteUdid, remoteFrontUserIds,
        remoteBackUserIds);

    localUdid = "remoteDeviceId";
    remoteUdid = "localDeviceId";
    userId = 456;
    std::vector<int32_t> remoteUserIds;
    remoteUserIds.push_back(userId);
    localUserIds.push_back(userId);
    DeviceProfileConnector::GetInstance().HandleSyncBackgroundUserIdEvent(remoteUserIds, remoteUdid, localUserIds,
        localUdid);
}

HWTEST_F(DeviceProfileConnectorTest, GetDeviceIdAndUserId_002, testing::ext::TestSize.Level1)
{
    std::string localUdid = "deviceId";
    int32_t localUserId = 123456;
    auto ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndUserId(localUdid, localUserId);
    EXPECT_FALSE(ret.empty());

    localUdid = "localDeviceId";
    localUserId = 456;
    ret = DeviceProfileConnector::GetInstance().GetDeviceIdAndUserId(localUdid, localUserId);
    EXPECT_FALSE(ret.empty());

    std::vector<int32_t> remoteUserIds;
    remoteUserIds.push_back(localUserId);
    std::string remoteUdid = "deviceId";
    std::vector<int32_t> localUserIds;
    localUserIds.push_back(localUserId);
    DeviceProfileConnector::GetInstance().HandleSyncBackgroundUserIdEvent(remoteUserIds, remoteUdid, localUserIds,
        localUdid);

    ProcessInfo process;
    process.userId = 1;
    process.pkgName = "pkgName";
    ProcessInfo process1;
    process1.userId = 1;
    process1.pkgName = "pkgName";
    EXPECT_TRUE(process == process1);

    process.userId = 0;
    EXPECT_TRUE(process < process1);

    DmNotifyKey dmNotifyKey;
    dmNotifyKey.processUserId = 1;
    dmNotifyKey.processPkgName = "proName";
    dmNotifyKey.notifyUserId = 0;
    dmNotifyKey.udid = "udid";

    DmNotifyKey dmNotifyKey1;
    dmNotifyKey1.processUserId = 1;
    dmNotifyKey1.processPkgName = "proName";
    dmNotifyKey1.notifyUserId = 0;
    dmNotifyKey1.udid = "udid";
    EXPECT_TRUE(dmNotifyKey == dmNotifyKey1);

    dmNotifyKey1.processUserId = 2;
    EXPECT_TRUE(dmNotifyKey < dmNotifyKey1);
}

HWTEST_F(DeviceProfileConnectorTest, GetAppTrustDeviceList_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "deviceId";

    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(123456));
    auto ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), false);

    deviceId = "remoteDeviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(1234));
    ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), false);

    deviceId = "remoteDeviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(1234));
    ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), false);

    deviceId = "remoteDeviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(1234));
    ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), false);

    deviceId = "remoteDeviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(12345));
    ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), false);

    deviceId = "remoteDeviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(12345));
    ret = DeviceProfileConnector::GetInstance().GetAppTrustDeviceList(pkgName, deviceId);
    EXPECT_EQ(ret.empty(), false);

    std::string udid;
    DeviceProfileConnector::GetInstance().DeleteAccessControlList(udid);

    udid = "deviceId";
    DeviceProfileConnector::GetInstance().DeleteAccessControlList(udid);
}

HWTEST_F(DeviceProfileConnectorTest, GetDevIdAndUserIdByActHash_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "deviceId";
    std::string peerUdid = "deviceId";
    int32_t peerUserId = 123456;
    std::string peerAccountHash = "";
    EXPECT_CALL(*cryptoMock_, GetAccountIdHash(_, _)).WillOnce(Return(ERR_DM_FAILED));
    auto ret = DeviceProfileConnector::GetInstance().GetDevIdAndUserIdByActHash(localUdid, peerUdid,
        peerUserId, peerAccountHash);
    EXPECT_TRUE(ret.empty());

    EXPECT_CALL(*cryptoMock_, GetAccountIdHash(_, _)).WillOnce(Return(DM_OK)).WillOnce(Return(ERR_DM_FAILED));
    ret = DeviceProfileConnector::GetInstance().GetDevIdAndUserIdByActHash(localUdid, peerUdid,
        peerUserId, peerAccountHash);
    EXPECT_TRUE(ret.empty());

    EXPECT_CALL(*cryptoMock_, GetAccountIdHash(_, _)).Times(::testing::AtLeast(40)).WillOnce(Return(DM_OK));
    ret = DeviceProfileConnector::GetInstance().GetDevIdAndUserIdByActHash(localUdid, peerUdid,
        peerUserId, peerAccountHash);
    EXPECT_FALSE(ret.empty());

    localUdid = "remoteDeviceId";
    peerUdid = "localDeviceId";
    peerUserId = 456;
    EXPECT_CALL(*cryptoMock_, GetAccountIdHash(_, _)).Times(::testing::AtLeast(40)).WillOnce(Return(DM_OK));
    ret = DeviceProfileConnector::GetInstance().GetDevIdAndUserIdByActHash(localUdid, peerUdid,
        peerUserId, peerAccountHash);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, GetDeviceAclParam_002, testing::ext::TestSize.Level1)
{
    DmDiscoveryInfo discoveryInfo;
    discoveryInfo.remoteDeviceIdHash = "";
    discoveryInfo.localDeviceId = "deviceId";
    discoveryInfo.userId = 123456;
    discoveryInfo.pkgname = "";
    bool isonline = true;
    int32_t authForm = 0;
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).Times(::testing::AtLeast(38)).WillOnce(Return(DM_OK));
    int32_t ret = DeviceProfileConnector::GetInstance().GetDeviceAclParam(discoveryInfo, isonline, authForm);
    EXPECT_EQ(ret, DM_OK);

    discoveryInfo.pkgname = "bundleName";
    discoveryInfo.localDeviceId = "deviceId";
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).Times(::testing::AtLeast(38)).WillOnce(Return(DM_OK));
    ret = DeviceProfileConnector::GetInstance().GetDeviceAclParam(discoveryInfo, isonline, authForm);
    EXPECT_EQ(ret, DM_OK);

    discoveryInfo.localDeviceId = "trustDeviceId";
    EXPECT_CALL(*cryptoMock_, GetUdidHash(_, _)).Times(::testing::AtLeast(38)).WillOnce(Return(DM_OK));
    ret = DeviceProfileConnector::GetInstance().GetDeviceAclParam(discoveryInfo, isonline, authForm);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSinkDevIdInAclForDevBind_004, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "localDeviceId";
    bool ret = DeviceProfileConnector::GetInstance().CheckSinkDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DeviceProfileConnectorTest, CheckSrcDevIdInAclForDevBind_005, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string deviceId = "123456";
    bool ret = DeviceProfileConnector::GetInstance().CheckSrcDevIdInAclForDevBind(pkgName, deviceId);
    EXPECT_FALSE(ret);

    DistributedDeviceProfile::AccessControlProfile profiles;
    std::string remoteUdid = "remoteDeviceId";
    std::vector<int32_t> remoteFrontUserIds;
    std::vector<int32_t> remoteBackUserIds;
    AddAccessControlProfileFirst(profiles);
    DeviceProfileConnector::GetInstance().DeleteSigTrustACL(profiles, remoteUdid,
        remoteFrontUserIds, remoteBackUserIds);

    remoteUdid = "localDeviceId";
    AddAccessControlProfileSeven(profiles);
    DeviceProfileConnector::GetInstance().DeleteSigTrustACL(profiles, remoteUdid,
        remoteFrontUserIds, remoteBackUserIds);

    AddAccessControlProfileEight(profiles);
    std::string localUdid = "localDeviceId";
    std::vector<int32_t> localUserIds;
    int32_t userId = 123456;
    localUserIds.push_back(userId);
    remoteUdid =  "remoteDeviceId";
    remoteFrontUserIds.push_back(0);
    remoteFrontUserIds.push_back(userId);
    DeviceProfileConnector::GetInstance().UpdatePeerUserId(profiles, localUdid, localUserIds, remoteUdid,
        remoteFrontUserIds);
}

HWTEST_F(DeviceProfileConnectorTest, CheckAccessControl_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 123456;
    DmAccessCaller caller;
    caller.userId = userId;
    std::string srcUdid = "deviceId";
    DmAccessCallee callee;
    callee.userId = userId;
    std::string sinkUdid = "deviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
    EXPECT_EQ(ret, DM_OK);

    srcUdid = "remoteDeviceId";
    sinkUdid = "localDeviceId";
    caller.userId = 456;
    callee.userId = 456;
    ret = DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
    EXPECT_EQ(ret, DM_OK);

    callee.userId = 0;
    ret = DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckIsSameAccount_002, testing::ext::TestSize.Level1)
{
    int32_t userId = 456;
    DmAccessCaller caller;
    caller.userId = userId;
    std::string srcUdid = "localDeviceId";
    DmAccessCallee callee;
    callee.userId = userId;
    std::string sinkUdid = "remoteDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().CheckIsSameAccount(caller, srcUdid, callee, sinkUdid);
    EXPECT_EQ(ret, DM_OK);

    callee.userId = 0;
    ret = DeviceProfileConnector::GetInstance().CheckAccessControl(caller, srcUdid, callee, sinkUdid);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, HandleAppUnBindEvent_002, testing::ext::TestSize.Level1)
{
    int32_t remoteUserId = 456;
    int32_t tokenId = 1001;
    std::string remoteUdid = "localDeviceId";
    std::string localUdid = "remoteDeviceId";
    std::string pkgName = "";
    DmOfflineParam offlineParam;
    offlineParam = DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid, tokenId,
        localUdid);
    EXPECT_NE(offlineParam.leftAclNumber, 0);

    int32_t peerTokenId = 1001;
    offlineParam = DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid, tokenId,
        localUdid, peerTokenId);
    EXPECT_EQ(offlineParam.leftAclNumber, 0);

    tokenId = 1002;
    peerTokenId = tokenId;
    offlineParam = DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid, tokenId,
        localUdid);
    EXPECT_EQ(offlineParam.leftAclNumber, 0);

    offlineParam = DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId, remoteUdid, tokenId,
        localUdid, peerTokenId);
    EXPECT_EQ(offlineParam.leftAclNumber, 0);
}

HWTEST_F(DeviceProfileConnectorTest, HandleAppUnBindEvent_003, testing::ext::TestSize.Level1)
{
    int32_t remoteUserId = 0;
    int32_t tokenId = 0;
    std::string remoteUdid = "remoteDeviceId";
    std::string localUdid = "localDeviceId";
    std::string pkgName = "";
    int32_t peerTokenId = 1001;
    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId,
        remoteUdid, tokenId, localUdid, peerTokenId);
    EXPECT_NE(offlineParam.leftAclNumber, 0);
}

HWTEST_F(DeviceProfileConnectorTest, HandleAppUnBindEvent_004, testing::ext::TestSize.Level1)
{
    int32_t remoteUserId = 123456;
    int32_t tokenId = 1001;
    std::string remoteUdid = "remoteDeviceId";
    std::string localUdid = "localDeviceId";
    std::string pkgName = "";
    int32_t peerTokenId = 1001;
    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance().HandleAppUnBindEvent(remoteUserId,
        remoteUdid, tokenId, localUdid, peerTokenId);
    EXPECT_NE(offlineParam.leftAclNumber, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetTokenIdByNameAndDeviceId_002, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string pkgName = "bundleName2";
    std::string requestDeviceId = "localDeviceId";
    GetAccessControlProfiles(profiles);
    auto ret = DeviceProfileConnector::GetInstance().GetTokenIdByNameAndDeviceId(pkgName, requestDeviceId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetTokenIdByNameAndDeviceId_003, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string pkgName = "bundleName8";
    std::string requestDeviceId = "localDeviceId9";
    GetAccessControlProfiles(profiles);
    auto ret = DeviceProfileConnector::GetInstance().GetTokenIdByNameAndDeviceId(pkgName, requestDeviceId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetTokenIdByNameAndDeviceId_004, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string pkgName = "bundleName2";
    std::string requestDeviceId = "localDeviceId9";
    GetAccessControlProfiles(profiles);
    auto ret = DeviceProfileConnector::GetInstance().GetTokenIdByNameAndDeviceId(pkgName, requestDeviceId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetTokenIdByNameAndDeviceId_005, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string pkgName;
    std::string requestDeviceId;
    GetAccessControlProfiles(profiles);
    auto ret = DeviceProfileConnector::GetInstance().GetTokenIdByNameAndDeviceId(pkgName, requestDeviceId);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetAclProfileByUserId_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "deviceId";
    int32_t userId = 123456;
    std::string remoteUdid  = "deviceId";
    auto ret = DeviceProfileConnector::GetInstance().GetAclProfileByUserId(localUdid, userId, remoteUdid);
    EXPECT_FALSE(ret.empty());

    localUdid = "remoteDeviceId";
    remoteUdid = "localDeviceId";
    userId = 456;
    ret = DeviceProfileConnector::GetInstance().GetAclProfileByUserId(localUdid, userId, remoteUdid);
    EXPECT_FALSE(ret.empty());
}

HWTEST_F(DeviceProfileConnectorTest, PutAllTrustedDevices_001, testing::ext::TestSize.Level1)
{
    std::vector<DistributedDeviceProfile::TrustedDeviceInfo> deviceInfos;
    int32_t ret = DeviceProfileConnector::GetInstance().PutAllTrustedDevices(deviceInfos);
    EXPECT_NE(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, CheckBindType_004, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "localDeviceId";
    std::string localUdid = "remoteDeviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(0));
    uint32_t ret = DeviceProfileConnector::GetInstance().CheckBindType(peerUdid, localUdid);
    EXPECT_EQ(ret, IDENTICAL_ACCOUNT_TYPE);

    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(1234));
    ret = DeviceProfileConnector::GetInstance().CheckBindType(peerUdid, localUdid);
    EXPECT_EQ(ret, 5);
}

HWTEST_F(DeviceProfileConnectorTest, GetTokenIdByNameAndDeviceId_001, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string requestDeviceId = "remoteDeviceId";
    uint64_t ret = DeviceProfileConnector::GetInstance().GetTokenIdByNameAndDeviceId(pkgName, requestDeviceId);
    EXPECT_NE(ret, 0);

    requestDeviceId = "localDeviceId";
    ret = DeviceProfileConnector::GetInstance().GetTokenIdByNameAndDeviceId(pkgName, requestDeviceId);
    EXPECT_NE(ret, 0);
}

HWTEST_F(DeviceProfileConnectorTest, GetParamBindTypeVec_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    std::string requestDeviceId = "requestDeviceId";
    std::vector<int32_t> bindTypeVec;
    std::string trustUdid = "localUdid";
    profiles.SetBindType(DM_IDENTICAL_ACCOUNT);
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, requestDeviceId, bindTypeVec, trustUdid);
    EXPECT_TRUE(bindTypeVec.empty());

    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(1);
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserDeviceId(trustUdid);
    accessee.SetAccesseeDeviceId(requestDeviceId);
    profiles.SetAccesser(accesser);
    profiles.SetAccessee(accessee);
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, requestDeviceId, bindTypeVec, trustUdid);
    EXPECT_FALSE(bindTypeVec.empty());

    bindTypeVec.clear();
    profiles.SetBindLevel(3);
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, requestDeviceId, bindTypeVec, trustUdid);
    EXPECT_FALSE(bindTypeVec.empty());

    bindTypeVec.clear();
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, requestDeviceId, bindTypeVec, trustUdid);
    EXPECT_FALSE(bindTypeVec.empty());

    bindTypeVec.clear();
    profiles.SetBindLevel(1);
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, requestDeviceId, bindTypeVec, trustUdid);
    EXPECT_FALSE(bindTypeVec.empty());

    bindTypeVec.clear();
    profiles.SetBindType(DM_IDENTICAL_ACCOUNT);
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, requestDeviceId, bindTypeVec, trustUdid);
    EXPECT_FALSE(bindTypeVec.empty());

    bindTypeVec.clear();
    profiles.SetBindType(DM_INVALIED_BINDTYPE);
    DeviceProfileConnector::GetInstance().GetParamBindTypeVec(profiles, requestDeviceId, bindTypeVec, trustUdid);
    EXPECT_TRUE(bindTypeVec.empty());
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    std::string localDeviceId = "localDeviceId";
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    uint32_t index = 1;
    std::string targetDeviceId = "remoteDeviceId";
    profiles.SetBindType(DM_IDENTICAL_ACCOUNT);
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, localDeviceId, sinkBindType, bindTypeIndex, index,
        targetDeviceId);
    EXPECT_FALSE(sinkBindType.empty());

    profiles.SetBindType(DM_POINT_TO_POINT);
    profiles.SetBindLevel(1);
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, localDeviceId, sinkBindType, bindTypeIndex, index,
        targetDeviceId);
    EXPECT_FALSE(sinkBindType.empty());

    profiles.SetBindLevel(3);
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserDeviceId(targetDeviceId);
    accessee.SetAccesseeDeviceId(localDeviceId);
    profiles.SetAccesser(accesser);
    profiles.SetAccessee(accessee);
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, localDeviceId, sinkBindType, bindTypeIndex, index,
        targetDeviceId);
    EXPECT_FALSE(sinkBindType.empty());

    accesser.SetAccesserDeviceId(localDeviceId);
    accessee.SetAccesseeDeviceId(targetDeviceId);
    profiles.SetAccesser(accesser);
    profiles.SetAccessee(accessee);
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, localDeviceId, sinkBindType, bindTypeIndex, index,
        targetDeviceId);
    EXPECT_FALSE(sinkBindType.empty());
}

HWTEST_F(DeviceProfileConnectorTest, ProcessBindType_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profiles;
    std::string localDeviceId = "localDeviceId";
    std::vector<int32_t> sinkBindType;
    std::vector<int32_t> bindTypeIndex;
    uint32_t index = 1;
    std::string targetDeviceId = "remoteDeviceId";
    profiles.SetBindType(DM_ACROSS_ACCOUNT);
    profiles.SetBindLevel(1);
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, localDeviceId, sinkBindType, bindTypeIndex, index,
        targetDeviceId);
    EXPECT_FALSE(sinkBindType.empty());

    profiles.SetBindLevel(3);
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserDeviceId(targetDeviceId);
    accessee.SetAccesseeDeviceId(localDeviceId);
    profiles.SetAccesser(accesser);
    profiles.SetAccessee(accessee);
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, localDeviceId, sinkBindType, bindTypeIndex, index,
        targetDeviceId);
    EXPECT_FALSE(sinkBindType.empty());

    accesser.SetAccesserDeviceId(localDeviceId);
    accessee.SetAccesseeDeviceId(targetDeviceId);
    profiles.SetAccesser(accesser);
    profiles.SetAccessee(accessee);
    DeviceProfileConnector::GetInstance().ProcessBindType(profiles, localDeviceId, sinkBindType, bindTypeIndex, index,
        targetDeviceId);
    EXPECT_FALSE(sinkBindType.empty());
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAclForRemoteUserRemoved_001, testing::ext::TestSize.Level1)
{
    std::string peerUdid = "deviceId";
    int32_t peerUserId = 123456;
    std::vector<int32_t> userIds;
    DeviceProfileConnector::GetInstance().DeleteAclForRemoteUserRemoved(peerUdid, peerUserId, userIds);
    EXPECT_FALSE(userIds.empty());

    peerUdid = "remoteDeviceId";
    peerUserId = 1234;
    DeviceProfileConnector::GetInstance().DeleteAclForRemoteUserRemoved(peerUdid, peerUserId, userIds);
    EXPECT_FALSE(userIds.empty());
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAccessControlList_001, testing::ext::TestSize.Level1)
{
    std::string pkgName;
    std::string localDeviceId = "remoteDeviceId";
    std::string remoteDeviceId;
    int32_t bindLevel = 3;
    std::string extra;
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(456));
    DmOfflineParam offlineParam = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(offlineParam.leftAclNumber, 0);

    extra = "extra";
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(456));
    offlineParam = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(offlineParam.leftAclNumber, 0);

    bindLevel = 2;
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(456));
    offlineParam = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(offlineParam.leftAclNumber, 0);

    bindLevel = 1;
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(456));
    offlineParam = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(offlineParam.leftAclNumber, 0);

    bindLevel = 5;
    offlineParam = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(offlineParam.bindType, INVALIED_TYPE);

    bindLevel = 2;
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(-1));
    offlineParam = DeviceProfileConnector::GetInstance().DeleteAccessControlList(pkgName, localDeviceId,
        remoteDeviceId, bindLevel, extra);
    EXPECT_EQ(offlineParam.bindType, INVALIED_TYPE);
}

HWTEST_F(DeviceProfileConnectorTest, DeleteAppBindLevel_002, testing::ext::TestSize.Level1)
{
    DmOfflineParam offlineParam;
    std::string pkgName = "bundleName1";
    std::vector<DistributedDeviceProfile::AccessControlProfile> profiles;
    std::string localUdid = "remoteDeviceId";
    std::string remoteUdid = "localDeviceId";
    std::string extra;
    AddAccessControlProfileSix(profiles);
    DeviceProfileConnector::GetInstance().DeleteAppBindLevel(offlineParam, pkgName, profiles, localUdid, remoteUdid,
        extra);
    EXPECT_EQ(offlineParam.leftAclNumber, 0);

    AddAccessControlProfile001(profiles);
    extra = "bundleName2";
    DeviceProfileConnector::GetInstance().DeleteAppBindLevel(offlineParam, pkgName, profiles, localUdid, remoteUdid,
        extra);
    EXPECT_EQ(offlineParam.bindType, APP);

    AddAccessControlProfile002(profiles);
    extra = "bundleName2";
    DeviceProfileConnector::GetInstance().DeleteAppBindLevel(offlineParam, pkgName, profiles, localUdid, remoteUdid,
        extra);
    EXPECT_EQ(offlineParam.bindType, APP);
}

HWTEST_F(DeviceProfileConnectorTest, GetBindLevel_002, testing::ext::TestSize.Level1)
{
    std::string pkgName = "bundleName";
    std::string localUdid = "remoteDeviceId";
    std::string udid = "localDeviceId";
    uint64_t tokenId = 0;
    int32_t bindLevel = INVALIED_TYPE;

    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(456));
    bindLevel = DeviceProfileConnector::GetInstance().GetBindLevel(pkgName, localUdid, udid, tokenId);
    EXPECT_EQ(bindLevel, APP);

    localUdid = "deviceId";
    udid = "deviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetFirstForegroundUserId()).WillOnce(Return(123456));
    bindLevel = DeviceProfileConnector::GetInstance().GetBindLevel(pkgName, localUdid, udid, tokenId);
    EXPECT_EQ(bindLevel, DEVICE);

    int32_t bindType = 256;
    std::string peerUdid = "123456";
    localUdid = "localDeviceId";
    int32_t localUserId = 1234;
    std::string localAccountId = "";
    DeviceProfileConnector::GetInstance().HandleDeviceUnBind(bindType, peerUdid, localUdid, localUserId,
        localAccountId);
}

HWTEST_F(DeviceProfileConnectorTest, SingleUserProcess_002, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    DmAccessCaller caller;
    DmAccessCallee callee;
    profile.SetBindType(DM_POINT_TO_POINT);
    profile.SetBindLevel(APP);
    caller.pkgName = "bundleName";
    DistributedDeviceProfile::Accessee accessee;
    accessee.SetAccesseeBundleName("bundleName");
    profile.SetAccessee(accessee);
    int32_t ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
    

    profile.SetBindType(DM_ACROSS_ACCOUNT);
    profile.SetBindLevel(APP);
    accessee.SetAccesseeBundleName("bundleName");
    profile.SetAccessee(accessee);
    ret = DeviceProfileConnector::GetInstance().SingleUserProcess(profile, caller, callee);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DeviceProfileConnectorTest, CheckDeviceInfoPermission_001, testing::ext::TestSize.Level1)
{
    std::string localUdid = "localUdid";
    std::string peerDeviceId = "localDeviceId";
    int32_t ret = DeviceProfileConnector::GetInstance().CheckDeviceInfoPermission(localUdid, peerDeviceId);
    EXPECT_EQ(ret, DM_OK);

    localUdid = "localDeviceId";
    peerDeviceId = "remoteDeviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetTokenIdAndForegroundUserId(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(1001), SetArgReferee<1>(456), Return()));
    EXPECT_CALL(*multipleUserConnectorMock_, GetOhosAccountIdByUserId(_)).WillOnce(Return(""));
    ret = DeviceProfileConnector::GetInstance().CheckDeviceInfoPermission(localUdid, peerDeviceId);
    EXPECT_EQ(ret, DM_OK);

    localUdid = "remoteDeviceId";
    peerDeviceId = "localDeviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetTokenIdAndForegroundUserId(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(1001), SetArgReferee<1>(456), Return()));
    EXPECT_CALL(*multipleUserConnectorMock_, GetOhosAccountIdByUserId(_)).WillOnce(Return("newAccountId"));
    ret = DeviceProfileConnector::GetInstance().CheckDeviceInfoPermission(localUdid, peerDeviceId);
    EXPECT_EQ(ret, DM_OK);
}

HWTEST_F(DeviceProfileConnectorTest, UpdateAclDeviceName_001, testing::ext::TestSize.Level1)
{
    std::string udid = "localDeviceId";
    std::string newDeviceName = "deviceName";
    int32_t ret = DeviceProfileConnector::GetInstance().UpdateAclDeviceName(udid, newDeviceName);
    EXPECT_EQ(ret, DM_OK);

    udid = "remoteDeviceId";
    newDeviceName = "deviceNameInfo";
    ret = DeviceProfileConnector::GetInstance().UpdateAclDeviceName(udid, newDeviceName);
    EXPECT_EQ(ret, DM_OK);

    udid = "UDID";
    ret = DeviceProfileConnector::GetInstance().UpdateAclDeviceName(udid, newDeviceName);
    EXPECT_EQ(ret, ERR_DM_FAILED);
}

HWTEST_F(DeviceProfileConnectorTest, CheckAppLevelAccess_001, testing::ext::TestSize.Level1)
{
    DistributedDeviceProfile::AccessControlProfile profile;
    int32_t acerTokenId = 1001;
    int32_t aceeTokenId = 1002;
    DmAccessCaller caller;
    DmAccessCallee callee;
    DistributedDeviceProfile::Accesser accesser;
    DistributedDeviceProfile::Accessee accessee;
    accesser.SetAccesserTokenId(acerTokenId);
    accessee.SetAccesseeTokenId(aceeTokenId);
    profile.SetAccesser(accesser);
    profile.SetAccessee(accessee);
    caller.tokenId = acerTokenId;
    callee.tokenId = aceeTokenId;
    bool ret = DeviceProfileConnector::GetInstance().CheckAppLevelAccess(profile, caller, callee);
    EXPECT_TRUE(ret);

    caller.tokenId = aceeTokenId;
    callee.tokenId = acerTokenId;
    ret = DeviceProfileConnector::GetInstance().CheckAppLevelAccess(profile, caller, callee);
    EXPECT_TRUE(ret);

    caller.tokenId = 1;
    callee.tokenId = 1;
    ret = DeviceProfileConnector::GetInstance().CheckAppLevelAccess(profile, caller, callee);
    EXPECT_FALSE(ret);
}
} // namespace DistributedHardware
} // namespace OHOS
