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

#ifndef OHOS_DM_IPC_NOTIFY_PINHOLDER_EVENT_REQ_H
#define OHOS_DM_IPC_NOTIFY_PINHOLDER_EVENT_REQ_H

#include "ipc_req.h"

namespace OHOS {
namespace DistributedHardware {
class IpcNotifyPinHolderEventReq : public IpcReq {
    DECLARE_IPC_MODEL(IpcNotifyPinHolderEventReq);

public:
    int32_t GetPinHolderEvent() const
    {
        return pinHolderEvent_;
    }

    void SetPinHolderEvent(const int32_t event)
    {
        pinHolderEvent_ = event;
    }

    int32_t GetResult() const
    {
        return result_;
    }

    void SetResult(const int32_t result)
    {
        result_ = result;
    }

    const std::string &GetContent() const
    {
        return content_;
    }

    void SetContent(const std::string &content)
    {
        content_ = content;
    }

private:
    int32_t pinHolderEvent_;
    int32_t result_;
    std::string content_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_IPC_NOTIFY_PINHOLDER_EVENT_REQ_H
