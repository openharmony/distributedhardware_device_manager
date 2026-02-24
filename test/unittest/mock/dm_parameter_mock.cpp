/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "dm_parameter_mock.h"

#include "gtest/gtest.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

DmParameterMock::DmParameterMock()
{
}
DmParameterMock::~DmParameterMock()
{
}

extern "C" {
int GetParameter(const char *key, const char *def, char *value, unsigned int len)
{
    return DmParameter::dmParameterMock->GetParameter(key, def, value, len);
}

int SetParameter(const char *key, const char *value)
{
    return DmParameter::dmParameterMock->SetParameter(key, value);
}

int GetDevUdid(char *udid, int size)
{
    return DmParameter::dmParameterMock->GetDevUdid(udid, size);
}
}
} // namespace DistributedHardware
} // namespace OHOS