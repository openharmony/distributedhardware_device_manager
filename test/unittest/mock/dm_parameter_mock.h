/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#ifndef OHOS_DM_PARAMETER_MOCK_H
#define OHOS_DM_PARAMETER_MOCK_H

#include <string>
#include <gmock/gmock.h>

namespace OHOS {
namespace DistributedHardware {
class DmParameter {
public:
    virtual ~DmParameter() = default;
    virtual int GetParameter(const char *key, const char *def, char *value, unsigned int len) = 0;
    virtual int SetParameter(const char *key, const char *value) = 0;
    virtual int GetDevUdid(char *udid, int size) = 0;
public:
    static inline std::shared_ptr<DmParameter> dmParameterMock = nullptr;
};

class DmParameterMock : public DmParameter {
public:
    DmParameterMock();
    ~DmParameterMock() override;
    MOCK_METHOD(int, GetParameter, (const char *, const char *, char *, unsigned int));
    MOCK_METHOD(int, SetParameter, (const char *, const char *));
    MOCK_METHOD(int, GetDevUdid, (char *, int));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_PARAMETER_MOCK_H
