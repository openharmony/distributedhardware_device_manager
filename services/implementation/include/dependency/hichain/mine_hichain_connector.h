/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_MINE_HICHAIN_CONNECTOR_H
#define OHOS_MINE_HICHAIN_CONNECTOR_H

#include <cstdint>
#include <string>

#include "device_auth.h"
#include "json_object.h"
#include "dm_single_instance.h"
#include "softbus_connector.h"

namespace OHOS {
namespace DistributedHardware {
class MineHiChainConnector {
public:
    static void onFinish(int64_t requestId, int operationCode, const char *returnData);
    static void onError(int64_t requestId, int operationCode, int errorCode, const char *errorReturn);

public:
    MineHiChainConnector();
    ~MineHiChainConnector();

    /**
     * @tc.name: MineHiChainConnector::Init
     * @tc.desc: Init HiChain of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t Init(void);

    /**
     * @tc.name: MineHiChainConnector::UnInit
     * @tc.desc: UnInit HiChain of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t UnInit(void);

    /**
     * @tc.name: MineHiChainConnector::DeleteCredentialAndGroup
     * @tc.desc: Delete Credential And Group HiChain of the HiChain Connector
     * @tc.type: FUNC
     */

    int32_t DeleteCredentialAndGroup(void);

    /**
     * @tc.name: MineHiChainConnector::CreateGroup
     * @tc.desc: Create Group of the HiChain Connector
     * @tc.type: FUNC
     */
    int32_t CreateGroup(const std::string &reqJsonStr);

    /**
     * @tc.name: MineHiChainConnector::RequestCredential
     * @tc.desc: Request Credential HiChain Callback of the HiChain Connector
     * @tc.type: FUNC
     */
    int RequestCredential(std::string &returnJsonStr);

    /**
     * @tc.name: MineHiChainConnector::CheckCredential
     * @tc.desc: Check Credential HiChain Callback of the HiChain Connector
     * @tc.type: FUNC
     */
    int CheckCredential(std::string reqJsonStr, std::string &returnJsonStr);

    /**
     * @tc.name: MineHiChainConnector::ImportCredential
     * @tc.desc: Import Credential HiChain Callback of the HiChain Connector
     * @tc.type: FUNC
     */
    int ImportCredential(std::string reqJsonStr, std::string &returnJsonStr);

    /**
     * @tc.name: MineHiChainConnector::DeleteCredential
     * @tc.desc: Delete Credential HiChain Callback of the HiChain Connector
     * @tc.type: FUNC
     */
    int DeleteCredential(std::string reqJsonStr, std::string &returnJsonStr);

private:
    static bool IsCredentialExist(void);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_HICHAIN_CONNECTOR_H
