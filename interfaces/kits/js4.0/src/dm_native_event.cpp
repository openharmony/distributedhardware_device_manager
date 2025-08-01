/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "dm_native_event.h"

#include "dm_log.h"

using namespace OHOS::DistributedHardware;

DmNativeEvent::DmNativeEvent(napi_env env, napi_value thisVar)
{
    env_ = env;
    thisVarRef_ = nullptr;
    napi_create_reference(env, thisVar, 1, &thisVarRef_);
}

DmNativeEvent::~DmNativeEvent()
{
    for (auto iter = eventMap_.begin(); iter != eventMap_.end(); iter++) {
        auto listener = iter->second;
        CHECK_NULL_VOID(listener);
        napi_delete_reference(env_, listener->handlerRef);
    }
    eventMap_.clear();
    napi_delete_reference(env_, thisVarRef_);
}

void DmNativeEvent::On(std::string &eventType, napi_value handler)
{
    LOGI("DmNativeEvent On in for event: %{public}s", eventType.c_str());
    std::lock_guard<std::mutex> autoLock(lock_);
    auto listener = std::make_shared<DmEventListener>();
    listener->eventType = eventType;
    napi_create_reference(env_, handler, 1, &listener->handlerRef);
    eventMap_[eventType] = listener;
}

void DmNativeEvent::Off(std::string &eventType)
{
    LOGI("DmNativeEvent Off in for event: %{public}s", eventType.c_str());
    napi_handle_scope scope = nullptr;
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed");
        return;
    }

    std::lock_guard<std::mutex> autoLock(lock_);
    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        LOGE("eventType %{public}s not find", eventType.c_str());
        napi_close_handle_scope(env_, scope);
        return;
    }
    auto listener = iter->second;
    napi_delete_reference(env_, listener->handlerRef);
    eventMap_.erase(eventType);
    napi_close_handle_scope(env_, scope);
}

void DmNativeEvent::OnEvent(const std::string &eventType, size_t argc, const napi_value *argv)
{
    LOGI("OnEvent for %{public}s", eventType.c_str());
    napi_handle_scope scope = nullptr;
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed");
        return;
    }

    std::lock_guard<std::mutex> autoLock(lock_);
    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        LOGE("eventType %{public}s not find", eventType.c_str());
        napi_close_handle_scope(env_, scope);
        return;
    }
    auto listener = iter->second;
    napi_value thisVar = nullptr;
    status = napi_get_reference_value(env_, thisVarRef_, &thisVar);
    if (status != napi_ok) {
        LOGE("napi_get_reference_value thisVar for %{public}s failed, status = %{public}d", eventType.c_str(), status);
        napi_close_handle_scope(env_, scope);
        return;
    }

    napi_value handler = nullptr;
    status = napi_get_reference_value(env_, listener->handlerRef, &handler);
    if (status != napi_ok) {
        LOGE("napi_get_reference_value handler for %{public}s failed, status = %{public}d", eventType.c_str(), status);
        napi_close_handle_scope(env_, scope);
        return;
    }

    napi_value callResult = nullptr;
    status = napi_call_function(env_, thisVar, handler, argc, argv, &callResult);
    if (status != napi_ok) {
        LOGE("napi_call_function for %{public}s failed, status = %{public}d", eventType.c_str(), status);
        napi_close_handle_scope(env_, scope);
        return;
    }
    napi_close_handle_scope(env_, scope);
}
