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

#ifndef OHOS_DM_AUTH_PINCODE_MESSAGE_PROCESSOR_3RD_H
#define OHOS_DM_AUTH_PINCODE_MESSAGE_PROCESSOR_3RD_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "dm_auth_info_3rd.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
struct DmAuthPincodeContext;
struct DmPincodeAccess;

// Message type
enum DmPincodeMessageType {
    AUTH_PINCODE_REQ_NEGOTIATE = 1010,
    AUTH_PINCODE_RESP_NEGOTIATE = 1020,
    AUTH_PINCODE_REQ_PIN_AUTH_START = 1030,
    AUTH_PINCODE_RESP_PIN_AUTH_START = 1040,
    AUTH_PINCODE_REQ_PIN_AUTH_MSG_NEGOTIATE = 1031,
    AUTH_PINCODE_RESP_PIN_AUTH_MSG_NEGOTIATE = 1041,
    AUTH_PINCODE_REQ_FINISH = 1050,
    AUTH_PINCODE_RESP_FINISH = 1060
};

class DmAuthPincodeMessageProcessor3rd {
public:
    using CreateMessageFuncPtr =
        int32_t (DmAuthPincodeMessageProcessor3rd::*)(std::shared_ptr<DmAuthPincodeContext>, JsonObject &jsonObject);
    using ParseMessageFuncPtr =
        int32_t (DmAuthPincodeMessageProcessor3rd::*)(const JsonObject &, std::shared_ptr<DmAuthPincodeContext>);
    DmAuthPincodeMessageProcessor3rd();
    ~DmAuthPincodeMessageProcessor3rd();
    // Parse the message, and save the parsed information to the context
    int32_t ParseMessage(std::shared_ptr<DmAuthPincodeContext> context, const std::string &message);
    // Create a message, construct the corresponding message based on msgType
    std::string CreateMessage(DmPincodeMessageType msgType, std::shared_ptr<DmAuthPincodeContext> context);

    // Create and send a message
    void CreateAndSendMsg(DmPincodeMessageType msgType, std::shared_ptr<DmAuthPincodeContext> context);
private:
    // construct function implementation
    void ConstructCreateMessageFuncMap();
    void ConstructParseMessageFuncMap();
    int32_t ParseMessage1010(const JsonObject &jsonObject, std::shared_ptr<DmAuthPincodeContext> context);
    int32_t ParseMessage1020(const JsonObject &jsonObject, std::shared_ptr<DmAuthPincodeContext> context);
    int32_t ParseMessage1030(const JsonObject &jsonObject, std::shared_ptr<DmAuthPincodeContext> context);
    int32_t ParseMessage1040(const JsonObject &jsonObject, std::shared_ptr<DmAuthPincodeContext> context);
    int32_t ParseMessage1031(const JsonObject &jsonObject, std::shared_ptr<DmAuthPincodeContext> context);
    int32_t ParseMessage1041(const JsonObject &jsonObject, std::shared_ptr<DmAuthPincodeContext> context);
    int32_t ParseMessage1050(const JsonObject &jsonObject, std::shared_ptr<DmAuthPincodeContext> context);
    int32_t ParseMessage1060(const JsonObject &jsonObject, std::shared_ptr<DmAuthPincodeContext> context);

    int32_t CreateMessage1010(std::shared_ptr<DmAuthPincodeContext> context, JsonObject &jsonObject);
    int32_t CreateMessage1020(std::shared_ptr<DmAuthPincodeContext> context, JsonObject &jsonObject);
    int32_t CreateMessage1030(std::shared_ptr<DmAuthPincodeContext> context, JsonObject &jsonObject);
    int32_t CreateMessage1040(std::shared_ptr<DmAuthPincodeContext> context, JsonObject &jsonObject);
    int32_t CreateMessage1031(std::shared_ptr<DmAuthPincodeContext> context, JsonObject &jsonObject);
    int32_t CreateMessage1041(std::shared_ptr<DmAuthPincodeContext> context, JsonObject &jsonObject);
    int32_t CreateMessage1050(std::shared_ptr<DmAuthPincodeContext> context, JsonObject &jsonObject);
    int32_t CreateMessage1060(std::shared_ptr<DmAuthPincodeContext> context, JsonObject &jsonObject);

    bool IsMessageValid(const JsonItemObject &jsonObject);
private:
    std::unordered_map<DmPincodeMessageType, CreateMessageFuncPtr> createMessageFuncMap_;
    std::unordered_map<DmPincodeMessageType, ParseMessageFuncPtr> parseMessageFuncMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_PINCODE_MESSAGE_PROCESSOR_3RD_H
