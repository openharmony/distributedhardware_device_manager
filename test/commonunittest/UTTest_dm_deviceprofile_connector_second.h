/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_UTTEST_DM_DEVICEPROFILE_CONNECTOR_SECOND_H
#define OHOS_UTTEST_DM_DEVICEPROFILE_CONNECTOR_SECOND_H

#include <memory>
#include <gtest/gtest.h>

#include "access_control_profile.h"
#include "deviceprofile_connector.h"
#include "distributed_device_profile_client_mock.h"
#include "deviceprofile_connector_mock.h"
#include "multiple_user_connector_mock.h"

namespace OHOS {
namespace DistributedHardware {
class DeviceProfileConnectorSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    DeviceProfileConnector connector;

    static inline std::shared_ptr<DistributedDeviceProfile::DistributedDeviceProfileClientMock>
        distributedDeviceProfileClientMock_ =
        std::make_shared<DistributedDeviceProfile::DistributedDeviceProfileClientMock>();
    static inline std::shared_ptr<MultipleUserConnectorMock> multipleUserConnectorMock_ =
        std::make_shared<MultipleUserConnectorMock>();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_UTTEST_DM_DEVICEPROFILE_CONNECTOR_SECOND_H
