/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UTTest_device_manager_service_impl_first.h"
#include "softbus_error_code.h"
#include "common_event_support.h"
#include "deviceprofile_connector.h"
#include "distributed_device_profile_client.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::DistributedDeviceProfile;
namespace OHOS {
namespace DistributedHardware {
void DeviceManagerServiceImplFirstTest::SetUp()
{
    if (deviceManagerServiceImpl_ == nullptr) {
        deviceManagerServiceImpl_ = std::make_shared<DeviceManagerServiceImpl>();
    }
    deviceManagerServiceImpl_->Initialize(listener_);
}

void DeviceManagerServiceImplFirstTest::TearDown()
{
}

void DeviceManagerServiceImplFirstTest::SetUpTestCase()
{
    DmDeviceProfileConnector::dmDeviceProfileConnector = deviceProfileConnectorMock_;
    DmMultipleUserConnector::dmMultipleUserConnector = multipleUserConnectorMock_;
}

void DeviceManagerServiceImplFirstTest::TearDownTestCase()
{
    DmDeviceProfileConnector::dmDeviceProfileConnector = nullptr;
    deviceProfileConnectorMock_ = nullptr;
    DmMultipleUserConnector::dmMultipleUserConnector = nullptr;
    multipleUserConnectorMock_ = nullptr;
}

namespace {
HWTEST_F(DeviceManagerServiceImplFirstTest, GetDeviceIdAndUserId_101, testing::ext::TestSize.Level0)
{
    int32_t userId = 1;
    std::string accountId = "accountId";
    auto ret = deviceManagerServiceImpl_->GetDeviceIdAndUserId(userId, accountId);
    EXPECT_TRUE(ret.empty());

    ret = deviceManagerServiceImpl_->GetDeviceIdAndUserId(userId);
    EXPECT_TRUE(ret.empty());

    std::string localUdid = "deviceId";
    int32_t localUserId = 123456;
    std::string peerUdid = "remoteUdid";
    int32_t peerUserId = 1;
    EXPECT_CALL(*deviceProfileConnectorMock_, DeleteAclForAccountLogOut(_, _, _, _)).WillOnce(Return(true));
    if (deviceManagerServiceImpl_->softbusConnector_ == nullptr) {
        deviceManagerServiceImpl_->Initialize(listener_);
    }

    if (deviceManagerServiceImpl_->deviceStateMgr_ == nullptr) {
        deviceManagerServiceImpl_->Initialize(listener_);
    }
    deviceManagerServiceImpl_->HandleIdentAccountLogout(localUdid, localUserId, peerUdid, peerUserId);

    std::vector<uint32_t> foregroundUserIds;
    std::vector<uint32_t> backgroundUserIds;
    std::string remoteUdid = "deviceId";
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_)).WillOnce(Return(ERR_DM_FAILED));
    deviceManagerServiceImpl_->HandleSyncUserIdEvent(foregroundUserIds, backgroundUserIds, remoteUdid, false);

    std::vector<int32_t> localUserIds;
    localUserIds.push_back(101);
    localUserIds.push_back(102);
    EXPECT_CALL(*multipleUserConnectorMock_, GetForegroundUserIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(localUserIds), Return(DM_OK)));
    deviceManagerServiceImpl_->HandleSyncUserIdEvent(foregroundUserIds, backgroundUserIds, remoteUdid, false);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
