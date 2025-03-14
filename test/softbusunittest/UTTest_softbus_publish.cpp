/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "UTTest_softbus_publish.h"

#include "dm_anonymous.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "json_object.h"
#include "softbus_connector.h"
#include "softbus_error_code.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusPublishTest::SetUp()
{
}
void SoftbusPublishTest::TearDown()
{
}
void SoftbusPublishTest::SetUpTestCase()
{
}
void SoftbusPublishTest::TearDownTestCase()
{
}

namespace {
HWTEST_F(SoftbusPublishTest, PublishSoftbusLNN_001, testing::ext::TestSize.Level0)
{
    SoftbusPublish spftbusPublish;
    int32_t ret = spftbusPublish.PublishSoftbusLNN();
    int publishId = 1;
    SoftbusPublish::OnSoftbusPublishResult(publishId, PUBLISH_LNN_SUCCESS);
    EXPECT_EQ(ret, ERR_DM_PUBLISH_FAILED);
}

HWTEST_F(SoftbusPublishTest, StopPublishSoftbusLNN_001, testing::ext::TestSize.Level0)
{
    SoftbusPublish spftbusPublish;
    int publishId = 2;
    int32_t ret = spftbusPublish.StopPublishSoftbusLNN(publishId);
    EXPECT_EQ(ret, ERR_DM_STOP_PUBLISH_LNN_FAILED);
}
} // namespace
} // namespace DistributedHardware
} // namespace OHOS
