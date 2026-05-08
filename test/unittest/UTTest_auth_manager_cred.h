/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef UTTEST_AUTH_MANAGER_CRED_H
#define UTTEST_AUTH_MANAGER_CRED_H

#include <gtest/gtest.h>
#include "auth_manager_cred.h"

namespace OHOS {
namespace DistributedHardware {

class AuthManagerCredTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
    std::shared_ptr<SoftbusConnector3rd> softbusConnector;
    std::shared_ptr<IDeviceManagerServiceListener3rd> listener;
    std::shared_ptr<HiChainAuthConnector3rd> hiChainAuthConnector;
    std::shared_ptr<AuthManagerCred> authManagerCred;
    std::shared_ptr<AuthSrcManagerCred> authSrcManagerCred;
    std::shared_ptr<AuthSinkManagerCred> authSinkManagerCred;
    std::shared_ptr<DmAuthCredContext> context;
};

}
}
#endif