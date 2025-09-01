/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_APP_MANAGER_MOCK_H
#define OHOS_APP_MANAGER_MOCK_H

#include <string>
#include <gmock/gmock.h>

#include "app_manager.h"

namespace OHOS {
namespace DistributedHardware {
class DmAppManager {
public:
    virtual ~DmAppManager() = default;
public:
    virtual std::string GetAppId() = 0;
    virtual bool IsSystemSA() = 0;
    virtual int32_t GetNativeTokenIdByName(std::string &processName, int64_t &tokenId) = 0;
    virtual int32_t GetHapTokenIdByName(int32_t userId, std::string &bundleName, int32_t instIndex,
        int64_t &tokenId) = 0;
    virtual int32_t GetAppIdByPkgName(const std::string &pkgName, std::string &appId, const int32_t userId) = 0;
    virtual int32_t GetBundleNameForSelf(std::string &bundleName) = 0;
public:
    static inline std::shared_ptr<DmAppManager> dmAppManager = nullptr;
};

class AppManagerMock : public DmAppManager {
public:
    MOCK_METHOD(std::string, GetAppId, ());
    MOCK_METHOD(bool, IsSystemSA, ());
    MOCK_METHOD(int32_t, GetNativeTokenIdByName, (std::string &, int64_t &));
    MOCK_METHOD(int32_t, GetHapTokenIdByName, (int32_t, std::string &, int32_t, int64_t &));
    MOCK_METHOD(int32_t, GetAppIdByPkgName, (const std::string &, std::string &, int32_t));
    MOCK_METHOD(int32_t, GetBundleNameForSelf, (std::string &));
};
}
}
#endif
