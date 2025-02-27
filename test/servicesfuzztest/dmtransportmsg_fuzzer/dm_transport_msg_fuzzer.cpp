/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <fuzzer/FuzzedDataProvider.h>

#include "dm_transport_msg.h"
#include "dm_transport_msg_fuzzer.h"
#include "dm_comm_tool.h"
#include "cJSON.h"


namespace OHOS {
namespace DistributedHardware {

namespace {
    constexpr uint32_t FORUSERID = 10;
    constexpr uint32_t BACKUSERID = 20;
    constexpr uint32_t USERID = 30;
    constexpr uint32_t USERFIRSTID = 40;
}

void DmTransPortMsgFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t code = fdp.ConsumeIntegral<int32_t>();
    std::string msg(reinterpret_cast<const char*>(data), size);
    std::string remoteUdid(reinterpret_cast<const char*>(data), size);
    const char* jsonString = R"({
        "MsgType": "0",
        "userId": "12345",
        "accountId": "a******3",
        "peerUdids": ["u******1", "u******2"],
        "peerUdid": "p******d",
        "accountName": "t******t",
        "syncUserIdFlag": 1,
        "userIds": [
            {"type": 1, "userId": 111},
            {"type": 0, "userId": 222}
        ]
    })";
    cJSON *jsonObject = nullptr;
    std::vector<uint32_t> foregroundUserIds;
    foregroundUserIds.push_back(FORUSERID);
    std::vector<uint32_t> backgroundUserIds;
    backgroundUserIds.push_back(BACKUSERID);
    UserIdsMsg userIdsMsg(foregroundUserIds, backgroundUserIds);
    CommMsg commMsg(code, msg);
    std::vector<uint32_t> userIds;
    userIds.push_back(USERID);
    userIds.push_back(USERFIRSTID);
    NotifyUserIds notifyUserIds(remoteUdid, userIds);
    ToJson(jsonObject, userIdsMsg);
    FromJson(jsonObject, userIdsMsg);
    ToJson(jsonObject, commMsg);
    FromJson(jsonObject, commMsg);
    ToJson(jsonObject, notifyUserIds);
    FromJson(jsonObject, notifyUserIds);
    jsonObject = cJSON_Parse(jsonString);
    ToJson(jsonObject, userIdsMsg);
    FromJson(jsonObject, userIdsMsg);
    ToJson(jsonObject, commMsg);
    FromJson(jsonObject, commMsg);
    GetCommMsgString(commMsg);
    ToJson(jsonObject, notifyUserIds);
    FromJson(jsonObject, notifyUserIds);
    notifyUserIds.ToString();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmTransPortMsgFuzzTest(data, size);
    return 0;
}
