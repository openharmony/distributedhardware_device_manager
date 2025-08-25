/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "generate_encrypted_uuid_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include "device_manager_impl.h"

namespace OHOS {
namespace DistributedHardware {

void GenerateEncryptedUuidFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string pkgName = fdp.ConsumeRandomLengthString();
    std::string uuid = fdp.ConsumeRandomLengthString();
    std::string appId = fdp.ConsumeRandomLengthString();
    std::string encryptedUuid = fdp.ConsumeRandomLengthString();

    DeviceManagerImpl::GetInstance().ipcClientProxy_ =
        std::make_shared<IpcClientProxy>(std::make_shared<IpcClientManager>());
    DeviceManagerImpl::GetInstance().GenerateEncryptedUuid(pkgName, uuid, appId, encryptedUuid);
    DeviceManagerImpl::GetInstance().GetEncryptedUuidByNetworkId(pkgName, uuid, appId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::GenerateEncryptedUuidFuzzTest(data, size);

    return 0;
}
