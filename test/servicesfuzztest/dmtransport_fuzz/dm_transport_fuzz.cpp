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

#include "dm_transport.h"
#include "dm_transport_fuzz.h"
#include "dm_comm_tool.h"


namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<DMCommTool> dmCommToolPtr_ = std::make_shared<DMCommTool>();
std::shared_ptr<DMTransport> dmTransPortPtr_ = std::make_shared<DMTransport>(dmCommToolPtr_);

void DmTransPortFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t socketId = fdp.ConsumeIntegral<int32_t>();
    std::string rmtNetworkId(reinterpret_cast<const char*>(data), size);
    std::string dmPkgName(reinterpret_cast<const char*>(data), size);
    std::string peerSocketName(reinterpret_cast<const char*>(data), size);
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSocketName.c_str()),
        .networkId = const_cast<char*>(rmtNetworkId.c_str()),
        .pkgName = const_cast<char*>(dmPkgName.c_str()),
        .dataType = DATA_TYPE_BYTES
    };
    dmTransPortPtr_->OnSocketOpened(socketId, info);
    dmTransPortPtr_->OnSocketOpened(socketId, info);
    ShutdownReason reason;
    dmTransPortPtr_->OnSocketClosed(socketId, reason);

    void *data = nullptr;
    uint32_t dataLen = 0;
    dmTransPortPtr_->OnSocketClosed(socketId, data, dataLen);
}

void DmTransPortFirstFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DmTransPortFuzzTest(data, size);
    OHOS::DistributedHardware::DmTransPortFirstFuzzTest(data, size);
    return 0;
}
