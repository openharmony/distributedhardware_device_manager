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

void DmTransPortMsgFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t code = fdp.ConsumeIntegral<int32_t>();
    std::string msg = fdp.ConsumeRandomLengthString();
    std::string remoteUdid = fdp.ConsumeRandomLengthString();
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
    backgroundUserIds.push_back(fdp.ConsumeIntegral<uint32_t>());
    UserIdsMsg userIdsMsg(foregroundUserIds, backgroundUserIds, true);
    CommMsg commMsg(code, msg);
    std::vector<uint32_t> userIds;
    userIds.push_back(USERID);
    userIds.push_back(USERFIRSTID);
    userIds.push_back(fdp.ConsumeIntegral<uint32_t>());
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
    cJSON_Delete(jsonObject);
}

void DmTransPortMsgFirstFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t code = fdp.ConsumeIntegral<int32_t>();
    std::string msg = fdp.ConsumeRandomLengthString();
    std::string remoteUdid = fdp.ConsumeRandomLengthString();
    const char* jsonString = R"({
        "accountId": "a******3",
        "userId": 123,
        "tokenId": 123456,
        "extra": "extra",
        "udid": "p******d",
    })";
    cJSON* jsonObject = nullptr;
    LogoutAccountMsg accountInfo;
    accountInfo.accountId = fdp.ConsumeRandomLengthString();
    ToJson(jsonObject, accountInfo);
    FromJson(jsonObject, accountInfo);
    UninstAppMsg uninstAppMsg;
    uninstAppMsg.userId_ = fdp.ConsumeIntegral<int32_t>();
    ToJson(jsonObject, uninstAppMsg);
    FromJson(jsonObject, uninstAppMsg);
    UnBindAppMsg unBindAppMsg;
    unBindAppMsg.extra_ = fdp.ConsumeRandomLengthString();
    ToJson(jsonObject, unBindAppMsg);
    FromJson(jsonObject, unBindAppMsg);

    jsonObject = cJSON_Parse(jsonString);
    ToJson(jsonObject, accountInfo);
    FromJson(jsonObject, accountInfo);
    ToJson(jsonObject, uninstAppMsg);
    FromJson(jsonObject, uninstAppMsg);
    ToJson(jsonObject, unBindAppMsg);
    FromJson(jsonObject, unBindAppMsg);
    cJSON_Delete(jsonObject);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::DmTransPortMsgFuzzTest(fdp);
    OHOS::DistributedHardware::DmTransPortMsgFirstFuzzTest(fdp);
    return 0;
}
