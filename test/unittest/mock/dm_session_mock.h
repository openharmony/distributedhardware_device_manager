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
#ifndef OHOS_DM_SESSION_MOCK_H
#define OHOS_DM_SESSION_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "session.h"

namespace OHOS {
namespace DistributedHardware {
class DmSession {
public:
    virtual ~DmSession() = default;
public:
    virtual int CreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener *listener) = 0;
    virtual int RemoveSessionServer(const char *pkgName, const char *sessionName) = 0;
public:
    static inline std::shared_ptr<DmSession> dmSessionMock = nullptr;
};

class DmSessionMock : public DmSession {
public:
    DmSessionMock();
    ~DmSessionMock() override;
    MOCK_METHOD(int, CreateSessionServer, (const char *, const char *, const ISessionListener *));
    MOCK_METHOD(int, RemoveSessionServer, (const char *, const char *));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_SESSION_MOCK_H
