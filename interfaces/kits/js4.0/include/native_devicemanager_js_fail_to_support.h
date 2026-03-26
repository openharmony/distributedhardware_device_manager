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

class DeviceManagerNapiFailToSupport : public DmNativeEvent {
public:
    explicit DeviceManagerNapiFailToSupport(napi_env env, napi_value thisVar);
    ~DeviceManagerNapiFailToSupport() override;
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value CreateDeviceManager(napi_env env, napi_callback_info info);
    static napi_value ReleaseDeviceManager(napi_env env, napi_callback_info info);
    static napi_value SetUserOperationSync(napi_env env, napi_callback_info info);
    static napi_value GetAvailableDeviceListSync(napi_env env, napi_callback_info info);
    static napi_value GetAvailableDeviceList(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceNetworkId(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceId(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceName(napi_env env, napi_callback_info info);
    static napi_value GetLocalDeviceType(napi_env env, napi_callback_info info);
    static napi_value GetDeviceName(napi_env env, napi_callback_info info);
    static napi_value GetDeviceType(napi_env env, napi_callback_info info);
    static napi_value StartDeviceDiscover(napi_env env, napi_callback_info info);
    static napi_value StopDeviceDiscover(napi_env env, napi_callback_info info);
    static napi_value BindTarget(napi_env env, napi_callback_info info);
    static napi_value UnBindTarget(napi_env env, napi_callback_info info);
    static napi_value JsOn(napi_env env, napi_callback_info info);
    static napi_value JsOff(napi_env env, napi_callback_info info);
    static napi_value JsGetDeviceProfileInfoList(napi_env env, napi_callback_info info);
    static napi_value JsGetDeviceIconInfo(napi_env env, napi_callback_info info);
    static napi_value JsPutDeviceProfileInfoList(napi_env env, napi_callback_info info);
    static napi_value JsGetLocalDisplayDeviceName(napi_env env, napi_callback_info info);
    static napi_value JsSetLocalDeviceName(napi_env env, napi_callback_info info);
    static napi_value JsSetRemoteDeviceName(napi_env env, napi_callback_info info);
    static napi_value JsRestoreLocalDeviceName(napi_env env, napi_callback_info info);
    static napi_value GetIdentificationByDeviceIds(napi_env env, napi_callback_info info);
    static napi_value JsGetDeviceNetworkIdList(napi_env env, napi_callback_info info);
    static napi_value SetHeartbeatPolicy(napi_env env, napi_callback_info info);
private:
    napi_env env_;
    static thread_local napi_ref sConstructor_;
    std::string bundleName_;
};
#endif // OHOS_DM_NATIVE_DEVICEMANAGER_JS_FAIL_TO_SUPPORT_H
