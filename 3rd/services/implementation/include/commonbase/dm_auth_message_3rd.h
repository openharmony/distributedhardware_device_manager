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

#ifndef OHOS_DM_AUTH_MESSAGE_3RD_H
#define OHOS_DM_AUTH_MESSAGE_3RD_H

namespace OHOS {
namespace DistributedHardware {

// Message type
enum DmCredMessageType {
    CRED_REQ_NEGOTIATE = 3010,
    CRED_RESP_NEGOTIATE = 3020,
    CRED_REQ_CREDENTIAL_AUTH_START = 3030,
    CRED_RESP_CREDENTIAL_AUTH_START = 3040,
    CRED_REQ_FINISH = 3050,
    CRED_RESP_FINISH = 3060
};

}  // namespace DistributedHardware
}  // namespace OHOS
#endif  // OHOS_DM_AUTH_MESSAGE_3RD_H
