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

#ifndef OHOS_DM_AUTH_MESSAGE_PROCESSOR_CRED_H
#define OHOS_DM_AUTH_MESSAGE_PROCESSOR_CRED_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "access_control_profile.h"
#include "dm_auth_message_3rd.h"
#include "crypto_mgr_3rd.h"
#include "dm_auth_info_3rd.h"
#include "json_object.h"

namespace OHOS {
namespace DistributedHardware {
struct DmAuthCredContext;
struct DmCredAccess;

class DmAuthMessageProcessorCred {
public:
    using CreateMessageFuncPtr =
        int32_t (DmAuthMessageProcessorCred::*)(std::shared_ptr<DmAuthCredContext>, JsonObject &jsonObject);
    using ParseMessageFuncPtr =
        int32_t (DmAuthMessageProcessorCred::*)(const JsonObject &, std::shared_ptr<DmAuthCredContext>);
    DmAuthMessageProcessorCred();
    ~DmAuthMessageProcessorCred();
    // Parse the message, and save the parsed information to the context
    int32_t ParseMessage(std::shared_ptr<DmAuthCredContext> context, const std::string &message);
    // Create a message, construct the corresponding message based on msgType
    std::string CreateMessage(DmCredMessageType msgType, std::shared_ptr<DmAuthCredContext> context);

    // Create and send a message
    void CreateAndSendMsg(DmCredMessageType msgType, std::shared_ptr<DmAuthCredContext> context);

private:
    // construct function implementation
    void ConstructCreateMessageFuncMap();
    void ConstructParseMessageFuncMap();
    std::unordered_map<DmCredMessageType, CreateMessageFuncPtr> createMessageFuncMap_;
    std::unordered_map<DmCredMessageType, ParseMessageFuncPtr> parseMessageFuncMap_;
    int32_t ParseMessage3010(const JsonObject &jsonObject, std::shared_ptr<DmAuthCredContext> context);
    int32_t ParseMessage3020(const JsonObject &jsonObject, std::shared_ptr<DmAuthCredContext> context);
    int32_t ParseMessage3030(const JsonObject &jsonObject, std::shared_ptr<DmAuthCredContext> context);
    int32_t ParseMessage3040(const JsonObject &jsonObject, std::shared_ptr<DmAuthCredContext> context);
    int32_t ParseMessage3050(const JsonObject &jsonObject, std::shared_ptr<DmAuthCredContext> context);
    int32_t ParseMessage3060(const JsonObject &jsonObject, std::shared_ptr<DmAuthCredContext> context);
    
    int32_t CreateMessage3010(std::shared_ptr<DmAuthCredContext> context, JsonObject &jsonObject);
    int32_t CreateMessage3020(std::shared_ptr<DmAuthCredContext> context, JsonObject &jsonObject);
    int32_t CreateMessage3030(std::shared_ptr<DmAuthCredContext> context, JsonObject &jsonObject);
    int32_t CreateMessage3040(std::shared_ptr<DmAuthCredContext> context, JsonObject &jsonObject);
    int32_t CreateMessage3050(std::shared_ptr<DmAuthCredContext> context, JsonObject &jsonObject);
    int32_t CreateMessage3060(std::shared_ptr<DmAuthCredContext> context, JsonObject &jsonObject);
    std::shared_ptr<CryptoMgr3rd> cryptoMgr_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_AUTH_MESSAGE_PROCESSOR_CRED_H
