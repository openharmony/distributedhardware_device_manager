/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef UTTEST_DM_AUTH_ATTEST_COMMON_H
#define UTTEST_DM_AUTH_ATTEST_COMMON_H

#include <gtest/gtest.h>
#include <string>
#include <securec.h>
#include "hichain_auth_connector_mock.h"
#include "softbus_session_mock.h"
#include "dm_auth_state_machine_mock.h"
#include "dm_auth_attest_common.h"

namespace OHOS {
namespace DistributedHardware {


class DmAuthAttestCommonTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

}
}
#endif