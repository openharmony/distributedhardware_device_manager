/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "dm_native_event.h"
#include "jsi.h"
#include "device_manager_log.h"
#include "js_async_work.h"

using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace ACELite {

std::map<std::string, std::shared_ptr<DmEventListener>> DmNativeEvent::eventMap_;

DmNativeEvent::DmNativeEvent()
{   
   DMLOG(DM_LOG_INFO, "DmNativeEvent ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");    
}

DmNativeEvent::DmNativeEvent(JSIValue thisVar)
{   
   DMLOG(DM_LOG_INFO, "DmNativeEvent ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");    
}

DmNativeEvent::~DmNativeEvent()
{
    // for (auto iter = eventMap_.begin(); iter != eventMap_.end(); iter++) {
    //     auto listener = iter->second;
    //     JSI::ReleaseValue(listener->handlerRef);
    // }
    // eventMap_.clear();
    // JSI::ReleaseValue(thisVarRef_);
    DMLOG(DM_LOG_INFO, "~DmNativeEvent ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"); 
}


void DmNativeEvent::On(std::string &eventType, JSIValue handle, JSIValue thisVal)
{
    DMLOG(DM_LOG_INFO, "DmNativeEvent On in for event: %s", eventType.c_str());                 
    auto listener= std::make_shared<DmEventListener>();
    
    listener->eventType = eventType;
    listener->handlerRef = JSI::AcquireValue(handle);
    
    listener->thisVarRef_ = JSI::AcquireValue(thisVal);
    eventMap_[eventType] = listener;	
}

void DmNativeEvent::Off(std::string &eventType)
{
    DMLOG(DM_LOG_INFO, "DmNativeEvent Off in for event: %s", eventType.c_str());
    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        DMLOG(DM_LOG_ERROR, "eventType %s not find", eventType.c_str());
        return;
    }
    auto listener = iter->second;
    JSI::ReleaseValue(listener->handlerRef);
     
    JSI::ReleaseValue(listener->thisVarRef_);
    eventMap_.erase(eventType);
}

void DmNativeEvent::OnEvent(const std::string &eventType, uint8_t argsSize, const JSIValue *data)
{
    DMLOG(DM_LOG_INFO, "OnEvent for %s", eventType.c_str());

    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        DMLOG(DM_LOG_ERROR, "eventType %s not find", eventType.c_str());
        return;
    }
    auto listener = iter->second;
    if (!JSI::ValueIsFunction(listener->handlerRef)){
         DMLOG(DM_LOG_INFO, "OnEvent for %s handlerRef is null", eventType.c_str());
         return;
    }
    
    FuncParams* params = new FuncParams();
    params->handlerRef = listener->handlerRef;
    params->thisVarRef_ = listener->thisVarRef_; 
    params->args = data;
    params->argsSize = argsSize;
    
    DMLOG(DM_LOG_INFO, "OnEventAsyncWorkFunc for %s in", eventType.c_str());
    JsAsyncWork::DispatchAsyncWork(OnEventAsyncWorkFunc, reinterpret_cast<void *>(params));
    
}

void DmNativeEvent::OnEventAsyncWorkFunc(void *data)
{
    DMLOG(DM_LOG_INFO, "OnEventAsyncWorkFunc in ");
    FuncParams* params = reinterpret_cast<FuncParams *>(data);
    JSI::CallFunction(params->handlerRef, params->thisVarRef_, params->args, params->argsSize);	
}

}
}