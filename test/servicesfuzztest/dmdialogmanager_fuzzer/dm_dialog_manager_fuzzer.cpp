/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#include "dm_dialog_manager_fuzzer.h"
#include "dm_dialog_manager.h"

namespace OHOS {
namespace DistributedHardware {
constexpr size_t MAX_STRING_LENGTH = 1024;

void ShowServiceBindConfirmDialogFuzzTest(FuzzedDataProvider &fdp)
{
    std::string param = fdp.ConsumeRandomLengthString(MAX_STRING_LENGTH);
    DmDialogManager::GetInstance().ShowServiceBindConfirmDialog(param);
}

}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    const size_t minSize = sizeof(int32_t);
    if ((data == nullptr) || (size < minSize)) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    /* Run your code on data */
    OHOS::DistributedHardware::ShowServiceBindConfirmDialogFuzzTest(fdp);
    return 0;
}
