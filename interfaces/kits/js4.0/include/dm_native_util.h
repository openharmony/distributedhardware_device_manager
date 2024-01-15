/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_NATIVE_UTIL_H
#define OHOS_DM_NATIVE_UTIL_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "dm_device_info.h"
#include "napi/native_api.h"

namespace OHOS {
namespace DistributedHardware {

enum DMBussinessErrorCode {
    // Permission verify failed.
    ERR_NO_PERMISSION = 201,
    //The caller is not a system application.
    ERR_NOT_SYSTEM_APP = 202,
    // Input parameter error.
    ERR_INVALID_PARAMS = 401,
    // Failed to execute the function.
    DM_ERR_FAILED = 11600101,
    // Failed to obtain the service.
    DM_ERR_OBTAIN_SERVICE = 11600102,
    // Authentication invalid.
    DM_ERR_AUTHENTICALTION_INVALID = 11600103,
    // Discovery invalid.
    DM_ERR_DISCOVERY_INVALID = 11600104,
    // Publish invalid.
    DM_ERR_PUBLISH_INVALID = 11600105,
};

void DeviceBasicInfoToJsArray(const napi_env &env,
                                    const std::vector<DmDeviceBasicInfo> &vecDevInfo,
                                    const int32_t idx, napi_value &arrayResult);
bool DmAuthParamDetection(const DmAuthParam &authParam);
void DmAuthParamToJsAuthParam(const napi_env &env, const DmAuthParam &authParam,
                                         napi_value &paramResult);
void SetValueInt32(const napi_env &env, const std::string &fieldStr, const int32_t intValue,
                              napi_value &result);
void SetValueUtf8String(const napi_env &env, const std::string &fieldStr, const std::string &str,
                                   napi_value &result);
void JsObjectToString(const napi_env &env, const napi_value &object, const std::string &fieldStr, char *dest,
                                 const int32_t destLen);
void JsObjectToBool(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                               bool &fieldRef);
void JsObjectToInt(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                              int32_t &fieldRef);
std::string JsObjectToString(const napi_env &env, const napi_value &param);
void JsToDmPublishInfo(const napi_env &env, const napi_value &object,
                                  DmPublishInfo &info);
void JsToBindParam(const napi_env &env, const napi_value &object, std::string &bindParam, int32_t &bindType,
        bool &isMetaType);
void JsToDmAuthInfo(const napi_env &env, const napi_value &object, std::string &extra);
void JsToJsonObject(const napi_env &env, const napi_value &object, const std::string &fieldStr,
                               nlohmann::json &jsonObj);
void JsToDmDiscoveryExtra(const napi_env &env, const napi_value &object, std::string &extra);
bool JsToDiscoverTargetType(napi_env env, const napi_value &object, int32_t &discoverTargetType);
napi_value CreateBusinessError(napi_env env, int32_t errCode, bool isAsync = true);
bool CheckArgsVal(napi_env env, bool assertion, const std::string &param, const std::string &msg);
bool CheckArgsCount(napi_env env, bool assertion, const std::string &message);
bool CheckArgsType(napi_env env, bool assertion, const std::string &paramName, const std::string &type);
bool IsSystemApp();
std::string GetDeviceTypeById(DmDeviceType type);
bool IsFunctionType(napi_env env, napi_value value);
void InsertMapParames(nlohmann::json &bindParamObj, std::map<std::string, std::string> &bindParamMap);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_NATIVE_UTIL_H
