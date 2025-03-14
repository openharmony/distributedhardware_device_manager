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

#ifndef OHOS_UTTEST_DM_COMM_TOOL_H
#define OHOS_UTTEST_DM_COMM_TOOL_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "dm_comm_tool.h"
#include "dm_transport_mock.h"
#include "dm_softbus_cache_mock.h"

namespace OHOS {
namespace DistributedHardware {
class DMCommToolTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline  std::shared_ptr<DMTransportMock> dmTransportMock_ = std::make_shared<DMTransportMock>();
    static inline  std::shared_ptr<SoftbusCacheMock> softbusCacheMock_ = std::make_shared<SoftbusCacheMock>();

protected:
    std::shared_ptr<DMCommTool> dmCommTool;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_UTTEST_DM_COMM_TOOL_H