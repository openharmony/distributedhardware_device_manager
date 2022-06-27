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
private:
    static int32_t oldUserId_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_MULTIPLE_USER_CONNECTOR_H
