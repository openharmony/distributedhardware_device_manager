/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <uv.h>
#include "native_devicemanager_js.h"
#include "device_manager.h"
#include "dm_constants.h"
#include "dm_log.h"
#include "device_manager_callback.h"
#include "dm_app_image_info.h"
#include "dm_device_info.h"
#include "dm_native_event.h"
#include "dm_subscribe_info.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "nlohmann/json.hpp"
#include "native_devicemanager_js_fuzzer.h"

namespace OHOS {
namespace DistributedHardware {
void NativeDeviceManagerStaticFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    napi_env env = *(reinterpret_cast<const napi_env*>(data));
    napi_callback_info info = *(reinterpret_cast<const napi_callback_info*>(data));
    std::string buldleName(reinterpret_cast<const char*>(data), size);
    DeviceManagerNapi *deviceManagerNapiPtr = DeviceManagerNapi::GetDeviceManagerNapi(buldleName);
    if (deviceManagerNapiPtr == nullptr) {
        return;
    }
    
    napi_value result = deviceManagerNapiPtr->CreateDeviceManager(env, info);
    result = deviceManagerNapiPtr->GetTrustedDeviceListSync(env, info);
    result = deviceManagerNapiPtr->AuthenticateDevice(env, info);
    result = deviceManagerNapiPtr->VerifyAuthInfo(env, info);
    result = deviceManagerNapiPtr->JsOn(env, info);
    result = deviceManagerNapiPtr->JsOff(env, info);
    result = deviceManagerNapiPtr->ReleaseDeviceManager(env, info);
}

void NativeDeviceManagerOnFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }
    std::string buldleName(reinterpret_cast<const char*>(data), size);
    std::string token(reinterpret_cast<const char*>(data), size);
    int32_t reason = *(reinterpret_cast<const int32_t*>(data));
    uint16_t subscribeId = *(reinterpret_cast<const uint16_t*>(data));
    DmNapiDevStateChangeAction action = *(reinterpret_cast<const DmNapiDevStateChangeAction*>(data));
    DmDeviceInfo deviceInfo = *(reinterpret_cast<const DmDeviceInfo*>(data));

    uv_work_t *work = new (std::nothrow) uv_work_t;
    std::unique_ptr<DmNapiStateJsCallback> jsCallback_;
    jsCallback_ = std::make_unique<DmNapiStateJsCallback>(buldleName, 0, 0, deviceInfo);
    work->data = reinterpret_cast<void *>(jsCallback_.get());
    DmNapiStateJsCallback *callback = reinterpret_cast<DmNapiStateJsCallback *>(work->data);
    delete work;

    DeviceManagerNapi *deviceManagerNapiPtr = DeviceManagerNapi::GetDeviceManagerNapi(buldleName);
    if (deviceManagerNapiPtr == nullptr) {
        return;
    }
    deviceManagerNapiPtr->OnDeviceStateChange(action, callback->deviceInfo_);
    deviceManagerNapiPtr->OnDeviceFound(subscribeId, callback->deviceInfo_);
    deviceManagerNapiPtr->OnDiscoveryFailed(subscribeId, reason);
    deviceManagerNapiPtr->OnAuthResult(buldleName, token, reason, reason);
    deviceManagerNapiPtr->OnVerifyResult(buldleName, reason, reason);
    deviceManagerNapiPtr->OnDmfaCall(buldleName);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::NativeDeviceManagerStaticFuzzTest(data, size);
    OHOS::DistributedHardware::NativeDeviceManagerOnFuzzTest(data, size);
    return 0;
}