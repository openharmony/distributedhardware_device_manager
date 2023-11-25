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

#ifndef OHOS_DM_MULTIPLE_USER_CONNECTOR_H
#define OHOS_DM_MULTIPLE_USER_CONNECTOR_H

#include <cstdint>
#include <string>
namespace OHOS {
namespace DistributedHardware {
class MultipleUserConnector {
public:
    /**
     * @tc.name: MultipleUserConnector::GetCurrentAccountUserID
     * @tc.desc: Get Current Account UserID of the Multiple User Connector
     * @tc.type: FUNC
     */
    static int32_t GetCurrentAccountUserID(void);

    /**
     * @tc.name: MultipleUserConnector::SetSwitchOldUserId
     * @tc.desc: Set Switch Old UserId of the Multiple User Connector
     * @tc.type: FUNC
     */
    static void SetSwitchOldUserId(int32_t userId);

    /**
     * @tc.name: MultipleUserConnector::GetSwitchOldUserId
     * @tc.desc: Get Switc hOld UserId of the Multiple User Connector
     * @tc.type: FUNC
     */
    static int32_t GetSwitchOldUserId(void);

    /**
     * @tc.name: MultipleUserConnector::GetOhosAccountId
     * @tc.desc: Get Current AccountId of the Multiple User Connector
     * @tc.type: FUNC
     */
    static std::string GetOhosAccountId(void);
 
    /**
     * @tc.name: MultipleUserConnector::SetSwitchOldAccountId
     * @tc.desc: Set Switch Old UserId of the Multiple User Connector
     * @tc.type: FUNC
     */
    static void SetSwitchOldAccountId(std::string accountId);

    /**
     * @tc.name: MultipleUserConnector::GetSwitchOldAccountId
     * @tc.desc: Get Switc hOld UserId of the Multiple User Connector
     * @tc.type: FUNC
     */
    static std::string GetSwitchOldAccountId(void);
private:
    static int32_t oldUserId_;
    static std::string accountId_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_MULTIPLE_USER_CONNECTOR_H
