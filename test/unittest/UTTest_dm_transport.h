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

#ifndef OHOS_UTTEST_DM_TRANSPORT_H
#define OHOS_UTTEST_DM_TRANSPORT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "dm_comm_tool.h"
#include "dm_transport.h"

namespace OHOS {
namespace DistributedHardware {
class DMTransportTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<DMCommTool> dmCommTool;
    std::shared_ptr<DMCommTool> dmCommToolPtr = std::make_shared<DMCommTool>();
    std::shared_ptr<DMTransport> dmTransport_ = std::make_shared<DMTransport>(dmCommToolPtr);

    int32_t ERR_DM_FAILED = 96929744;
    int32_t ERR_DM_INPUT_PARA_INVALID = 96929749;
    uint32_t MAX_SEND_MSG_LENGTH = 4 * 1024 * 1024;
    int32_t DM_OK = 0;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_UTTEST_DM_TRANSPORT_H