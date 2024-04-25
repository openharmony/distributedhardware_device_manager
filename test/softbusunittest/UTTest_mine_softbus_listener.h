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
#ifndef OHOS_UTTEST_MINE_SOFTBUS_LISTENER_H
#define OHOS_UTTEST_MINE_SOFTBUS_LISTENER_H

#include <gtest/gtest.h>
#include <refbase.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "softbus_adapter.h"
#include "softbus_bus_center.h"
#include "softbus_listener.h"
#include "dm_device_info.h"
#include "dm_subscribe_info.h"
#include "inner_session.h"
#include "mine_softbus_listener.h"
#include "session.h"

namespace OHOS {
namespace DistributedHardware {
class MineSoftbusListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

class ISoftbusDiscoveringCallbackTest : public ISoftbusDiscoveringCallback {
public:
    virtual ~ISoftbusDiscoveringCallbackTest()
    {
    }
    void OnDeviceFound(const std::string &pkgName, const DmDeviceInfo &info, bool isOnline) override
    {
        (void)pkgName;
        (void)info;
        (void)isOnline;
    }
    void OnDiscoveringResult(const std::string &pkgName, int32_t subscribeId, int32_t result) override
    {
        (void)pkgName;
        (void)subscribeId;
        (void)result;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
