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

#ifndef OHOS_DM_NATIVE_DEVICEMANAGER_JS_FAIL_TO_SUPPORT_H
#define OHOS_DM_NATIVE_DEVICEMANAGER_JS_FAIL_TO_SUPPORT_H

#include <memory>
#include <string>
#include <mutex>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "dm_native_event.h"
#define DM_NAPI_BUF_LENGTH (256)

struct AsyncCallbackInfoLite {
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;

    char bundleName[DM_NAPI_BUF_LENGTH] = {0};
    size_t bundleNameLen = 0;

    napi_ref callback = nullptr;
    int32_t status = -1;
    int32_t ret = 0;
};

class DeviceManagerNapiFailToSupport : public DmNativeEvent {
public:
    explicit DeviceManagerNapiFailToSupport(napi_env env, napi_value thisVar);
    ~DeviceManagerNapiFailToSupport() override;
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value CreateDeviceManager(napi_env env, napi_callback_info info);
    static napi_value ReleaseDeviceManager(napi_env env, napi_callback_info info);
    static napi_value SetUserOperationSync(napi_env env, napi_callback_info info);
    static napi_value GetTrustedDeviceListSync(napi_env env, napi_callback_info info);
    static napi_value GetTrustedDeviceList(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceInfoSync(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceInfo(napi_env env, napi_callback_info info);
    static napi_value GetDeviceInfo(napi_env env, napi_callback_info info);
    static napi_value UnAuthenticateDevice(napi_env env, napi_callback_info info);
    static napi_value StartDeviceDiscoverSync(napi_env env, napi_callback_info info);
    static napi_value StopDeviceDiscoverSync(napi_env env, napi_callback_info info);
    static napi_value PublishDeviceDiscoverySync(napi_env env, napi_callback_info info);
    static napi_value UnPublishDeviceDiscoverySync(napi_env env, napi_callback_info info);
    static napi_value AuthenticateDevice(napi_env env, napi_callback_info info);
    static napi_value RequestCredential(napi_env env, napi_callback_info info);
    static napi_value ImportCredential(napi_env env, napi_callback_info info);
    static napi_value DeleteCredential(napi_env env, napi_callback_info info);
    static napi_value JsOn(napi_env env, napi_callback_info info);
    static napi_value JsOff(napi_env env, napi_callback_info info);
    static DeviceManagerNapiFailToSupport *GetDeviceManagerNapi(std::string &bundleName);
    static int32_t RegisterCredentialCallback(napi_env env, const std::string &pkgName);

private:
    napi_env env_;
    static thread_local napi_ref sConstructor_;
    std::string bundleName_;
};
#endif // OHOS_DM_NATIVE_DEVICEMANAGER_JS_FAIL_TO_SUPPORT_H
