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
#ifndef OHOS_UTTEST_DM_DISTRIBUTED_HARDWARE_LOAD_H
#define OHOS_UTTEST_DM_DISTRIBUTED_HARDWARE_LOAD_H

#include <gtest/gtest.h>
#include <refbase.h>

#include "dm_constants.h"
#include "dm_crypto.h"
#include "dm_distributed_hardware_load.h"

namespace OHOS {
namespace DistributedHardware {
class DmDistributedHardwareLoadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class IRemoteObjectTest : public IRemoteObject {
public:
    int32_t GetObjectRefCount() override
    {
        return DM_OK;
    }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        (void)code;
        (void)data;
        (void)reply;
        (void)option;
        return DM_OK;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        (void)recipient;
        return true;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        (void)recipient;
        return true;
    }
    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        (void)fd;
        (void)args;
        return DM_OK;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
