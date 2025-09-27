/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_OHOS_DM_CONFIG_MANAGER_H
#define OHOS_OHOS_DM_CONFIG_MANAGER_H

#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "authentication.h"
#include "crypto_adapter.h"
#include "decision_adapter.h"
#include "dm_single_instance.h"
#if !defined(__LITEOS_M__)
#include "ffrt.h"
#endif

namespace OHOS {
namespace DistributedHardware {
typedef struct {
    std::string name;
    std::string type;
    std::string version;
    std::string funcName;
    std::string soName;
    std::string soPath;
} AdapterSoLoadInfo;

typedef struct {
    int32_t authType;
    std::string name;
    std::string type;
    std::string version;
    std::string funcName;
    std::string soName;
    std::string soPath;
} AuthSoLoadInfo;

class DmConfigManager final {
    DM_DECLARE_SINGLE_INSTANCE_BASE(DmConfigManager);

public:
    ~DmConfigManager();

    /**
     * @tc.name: DmConfigManager::GetAllAuthType
     * @tc.desc: Get All AuthType of the DeviceManager Config Manager
     * @tc.type: FUNC
     */
    void GetAllAuthType(std::vector<std::string> &allAuthType);
    std::shared_ptr<ICryptoAdapter> GetCryptoAdapter(const std::string &soName);

    /**
     * @tc.name: DmConfigManager::GetAuthAdapter
     * @tc.desc: Get Auth Adapter of the DeviceManager Config Manager
     * @tc.type: FUNC
     */
    void GetAuthAdapter(std::map<int32_t, std::shared_ptr<IAuthentication>> &authAdapter);

private:
    DmConfigManager();
    void ParseAdapterConfig();
    void ParseAuthConfig();

private:
#if !defined(__LITEOS_M__)
    ffrt::mutex cryptoAdapterMutex_;
#endif
    std::map<int32_t, AuthSoLoadInfo> soAuthLoadInfo_;
    std::map<std::string, AdapterSoLoadInfo> soAdapterLoadInfo_;
    std::map<std::string, std::shared_ptr<ICryptoAdapter>> cryptoAdapterPtr_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_OHOS_DM_CONFIG_MANAGER_H
