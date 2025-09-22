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

#include "dm_transport_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <unistd.h>
#include <unordered_map>

#include "dm_comm_tool.h"
#include "dm_transport.h"

namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<DMCommTool> dmCommToolPtr_ = std::make_shared<DMCommTool>();
std::shared_ptr<DMTransport> dmTransPortPtr_ = std::make_shared<DMTransport>(dmCommToolPtr_);

void DmTransPortFuzzTest(FuzzedDataProvider &fdp)
{
    int32_t socketId = fdp.ConsumeIntegral<int32_t>();
    std::string rmtNetworkId = fdp.ConsumeRandomLengthString();
    std::string dmPkgName = fdp.ConsumeRandomLengthString();
    std::string peerSocketName = fdp.ConsumeRandomLengthString();
    PeerSocketInfo info = { .name = const_cast<char*>(peerSocketName.c_str()),
        .networkId = const_cast<char*>(rmtNetworkId.c_str()),
        .pkgName = const_cast<char*>(dmPkgName.c_str()),
        .dataType = DATA_TYPE_BYTES };
    dmTransPortPtr_->OnSocketOpened(socketId, info);
    dmTransPortPtr_->OnSocketOpened(socketId, info);
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_LNN_CHANGED;
    dmTransPortPtr_->OnSocketClosed(socketId, reason);

    void* dataInfo = nullptr;
    uint32_t dataLen = fdp.ConsumeIntegral<uint32_t>();
    dmTransPortPtr_->OnBytesReceived(socketId, dataInfo, dataLen);

    std::string dataStr = fdp.ConsumeRandomLengthString();
    dataInfo = reinterpret_cast<void*>(dataStr.data());
    dataLen = static_cast<uint32_t>(dataStr.length());
    dmTransPortPtr_->OnBytesReceived(socketId, dataInfo, dataLen);

    std::string payload = fdp.ConsumeRandomLengthString();
    dmTransPortPtr_->HandleReceiveMessage(socketId, payload);
}

void DmTransPortFirstFuzzTest(FuzzedDataProvider &fdp)
{
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
    std::string payload = fdp.ConsumeRandomLengthString();
    int32_t socketId = fdp.ConsumeIntegral<int32_t>();
    std::string rmtNetworkId = fdp.ConsumeRandomLengthString();
    std::string dmPkgName = fdp.ConsumeRandomLengthString();
    std::string peerSocketName = fdp.ConsumeRandomLengthString();
    PeerSocketInfo info = { .name = const_cast<char*>(peerSocketName.c_str()),
        .networkId = const_cast<char*>(rmtNetworkId.c_str()),
        .pkgName = const_cast<char*>(dmPkgName.c_str()),
        .dataType = DATA_TYPE_BYTES };
    dmTransPortPtr_->OnSocketOpened(socketId, info);
    dmTransPortPtr_->HandleReceiveMessage(socketId, payload);
    dmTransPortPtr_->CreateClientSocket(rmtNetworkId);
    dmTransPortPtr_->IsDeviceSessionOpened(rmtNetworkId, socketId);
    dmTransPortPtr_->StartSocket(rmtNetworkId, socketId);
    dmTransPortPtr_->StopSocket(rmtNetworkId);
    dmTransPortPtr_->Send(rmtNetworkId, payload, socketId);
    dmTransPortPtr_->StartSocket(rmtNetworkId, socketId);
    dmTransPortPtr_->StopSocket(rmtNetworkId);
    dmTransPortPtr_->Send(rmtNetworkId, payload, socketId);
    dmTransPortPtr_->IsDeviceSessionOpened(rmtNetworkId, socketId);
    dmTransPortPtr_->ClearDeviceSocketOpened(rmtNetworkId, socketId);
    dmTransPortPtr_->UnInit();
    dmTransPortPtr_->IsDeviceSessionOpened(rmtNetworkId, socketId);
    std::string remoteDevId = fdp.ConsumeRandomLengthString();
    dmTransPortPtr_->ClearDeviceSocketOpened(remoteDevId, socketId);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::DistributedHardware::DmTransPortFuzzTest(fdp);
    OHOS::DistributedHardware::DmTransPortFirstFuzzTest(fdp);
    return 0;
}
