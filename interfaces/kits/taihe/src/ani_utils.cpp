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
#define LOG_TAG "AniUtils"
#include "ani_utils.h"
#include "dm_device_info.h"
#include "dm_log.h"
#include "dm_constants.h"
#include "dm_anonymous.h"

namespace ani_utils {

int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, std::string &result, bool optional)
{
    if (env == nullptr || ani_obj == nullptr || property == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_object object = nullptr;
    int32_t status = AniGetProperty(env, ani_obj, property, object, optional);
    if (status == ANI_OK && object != nullptr) {
        result = AniStringUtils::ToStd(env, reinterpret_cast<ani_string>(object));
    }
    return status;
}

int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, bool &result, bool optional)
{
    if (env == nullptr || ani_obj == nullptr || property == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_boolean ani_field_value;
    ani_status status = const_cast<ani_env*>(env)->Object_GetPropertyByName_Boolean(
        ani_obj, property, reinterpret_cast<ani_boolean*>(&ani_field_value));
    if (status != ANI_OK) {
        if (optional) {
            status = ANI_OK;
        }
        return status;
    }
    result = (bool)ani_field_value;
    return ANI_OK;
}

int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, int32_t &result, bool optional)
{
    if (env == nullptr || ani_obj == nullptr || property == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_int ani_field_value;
    ani_status status = const_cast<ani_env*>(env)->Object_GetPropertyByName_Int(
        ani_obj, property, reinterpret_cast<ani_int*>(&ani_field_value));
    if (status != ANI_OK) {
        if (optional) {
            status = ANI_OK;
        }
        return status;
    }
    result = (int32_t)ani_field_value;
    return ANI_OK;
}

int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, uint32_t &result, bool optional)
{
    if (env == nullptr || ani_obj == nullptr || property == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_int ani_field_value;
    ani_status status = const_cast<ani_env*>(env)->Object_GetPropertyByName_Int(
        ani_obj, property, reinterpret_cast<ani_int*>(&ani_field_value));
    if (status != ANI_OK) {
        if (optional) {
            status = ANI_OK;
        }
        return status;
    }
    result = (uint32_t)ani_field_value;
    return ANI_OK;
}

int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, ani_object &result, bool optional)
{
    if (env == nullptr || ani_obj == nullptr || property == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_status status = const_cast<ani_env*>(env)->Object_GetPropertyByName_Ref(ani_obj, property,
        reinterpret_cast<ani_ref*>(&result));
    if (status != ANI_OK) {
        if (optional) {
            status = ANI_OK;
        }
        return status;
    }
    return ANI_OK;
}

std::string AniStringUtils::ToStd(const ani_env *env, ani_string ani_str)
{
    if (env == nullptr) {
        return std::string();
    }
    ani_size strSize = 0;
    auto status = const_cast<ani_env*>(env)->String_GetUTF8Size(ani_str, &strSize);
    if (ANI_OK != status) {
        LOGI("String_GetUTF8Size failed");
        return std::string();
    }

    std::vector<char> buffer(strSize + 1);
    char *utf8Buffer = buffer.data();

    ani_size bytesWritten = 0;
    status = const_cast<ani_env*>(env)->String_GetUTF8(ani_str, utf8Buffer, strSize + 1, &bytesWritten);
    if (ANI_OK != status) {
        LOGI("String_GetUTF8Size failed");
        return std::string();
    }

    utf8Buffer[bytesWritten] = '\0';
    std::string content = std::string(utf8Buffer);
    return content;
}

ani_string AniStringUtils::ToAni(const ani_env *env, const std::string& str)
{
    if (env == nullptr) {
        return nullptr;
    }
    ani_string aniStr = nullptr;
    if (ANI_OK != const_cast<ani_env*>(env)->String_NewUTF8(str.data(), str.size(), &aniStr)) {
        LOGI("Unsupported ANI_VERSION_1");
        return nullptr;
    }
    return aniStr;
}

template<>
bool UnionAccessor::IsInstanceOfType<bool>()
{
    return IsInstanceOf("std.core.Boolean");
}

template<>
bool UnionAccessor::IsInstanceOfType<int>()
{
    return IsInstanceOf("std.core.Int");
}

template<>
bool UnionAccessor::IsInstanceOfType<double>()
{
    return IsInstanceOf("std.core.Double");
}

template<>
bool UnionAccessor::IsInstanceOfType<std::string>()
{
    return IsInstanceOf("std.core.String");
}

template<>
bool UnionAccessor::TryConvertArray<ani_ref>(std::vector<ani_ref> &value)
{
    ani_double length;
    if (ANI_OK != env_->Object_GetPropertyByName_Double(obj_, "length", &length)) {
        LOGE("Object_GetPropertyByName_Double length failed");
        return false;
    }
    for (int i = 0; i < int(length); i++) {
        ani_ref ref;
        if (ANI_OK != env_->Object_CallMethodByName_Ref(obj_, "$_get", "i:C{std.core.Object}", &ref, (ani_int)i)) {
            LOGE("Object_GetPropertyByName_Ref failed");
            return false;
        }
        value.push_back(ref);
    }
    LOGD("convert ref array ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvertArray<bool>(std::vector<bool> &value)
{
    ani_double length;
    if (ANI_OK != env_->Object_GetPropertyByName_Double(obj_, "length", &length)) {
        LOGE("Object_GetPropertyByName_Double length failed");
        return false;
    }
    for (int i = 0; i < int(length); i++) {
        ani_ref ref;
        if (ANI_OK != env_->Object_CallMethodByName_Ref(obj_, "$_get", "i:C{std.core.Object}", &ref, (ani_int)i)) {
            LOGE("Object_GetPropertyByName_Ref failed");
            return false;
        }
        ani_boolean val;
        if (ANI_OK != env_->Object_CallMethodByName_Boolean(static_cast<ani_object>(ref), "unboxed", nullptr, &val)) {
            LOGE("Object_CallMethodByName_Boolean unbox failed");
            return false;
        }
        value.push_back(static_cast<bool>(val));
    }
    LOGD("convert bool array ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvertArray<int>(std::vector<int> &value)
{
    ani_double length;
    if (ANI_OK != env_->Object_GetPropertyByName_Double(obj_, "length", &length)) {
        LOGE("Object_GetPropertyByName_Double length failed");
        return false;
    }
    for (int i = 0; i < int(length); i++) {
        ani_ref ref;
        if (ANI_OK != env_->Object_CallMethodByName_Ref(obj_, "$_get", "i:C{std.core.Object}", &ref, (ani_int)i)) {
            LOGE("Object_GetPropertyByName_Ref failed");
            return false;
        }
        ani_int intValue;
        if (ANI_OK != env_->Object_CallMethodByName_Int(static_cast<ani_object>(ref), "unboxed", nullptr, &intValue)) {
            LOGE("Object_CallMethodByName_Int unbox failed");
            return false;
        }
        value.push_back(static_cast<int>(intValue));
    }
    LOGD("convert int array ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvertArray<double>(std::vector<double> &value)
{
    ani_double length;
    if (ANI_OK != env_->Object_GetPropertyByName_Double(obj_, "length", &length)) {
        LOGE("Object_GetPropertyByName_Double length failed");
        return false;
    }
    for (int i = 0; i < int(length); i++) {
        ani_ref ref;
        if (ANI_OK != env_->Object_CallMethodByName_Ref(obj_, "$_get", "i:C{std.core.Object}", &ref, (ani_int)i)) {
            LOGE("Object_GetPropertyByName_Ref failed");
            return false;
        }
        ani_double val;
        if (ANI_OK != env_->Object_CallMethodByName_Double(static_cast<ani_object>(ref), "unboxed", nullptr, &val)) {
            LOGE("Object_CallMethodByName_Double unbox failed");
            return false;
        }
        value.push_back(static_cast<double>(val));
    }
    LOGD("convert double array ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvertArray<uint8_t>(std::vector<uint8_t> &value)
{
    ani_ref buffer;
    if (ANI_OK != env_->Object_GetFieldByName_Ref(obj_, "buffer", &buffer)) {
        LOGE("Object_GetFieldByName_Ref failed");
        return false;
    }
    void* data;
    size_t size;
    if (ANI_OK != env_->ArrayBuffer_GetInfo(static_cast<ani_arraybuffer>(buffer), &data, &size)) {
        LOGE("ArrayBuffer_GetInfo failed");
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        value.push_back(static_cast<uint8_t*>(data)[i]);
    }
    LOGD("convert uint8 array ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvertArray<float>(std::vector<float> &value)
{
    ani_ref buffer;
    if (ANI_OK != env_->Object_GetFieldByName_Ref(obj_, "buffer", &buffer)) {
        LOGE("Object_GetFieldByName_Ref failed");
        return false;
    }
    void* data;
    size_t size;
    if (ANI_OK != env_->ArrayBuffer_GetInfo(static_cast<ani_arraybuffer>(buffer), &data, &size)) {
        LOGE("ArrayBuffer_GetInfo failed");
        return false;
    }
    auto count = size / sizeof(float);
    for (size_t i = 0; i < count; i++) {
        value.push_back(static_cast<uint8_t*>(data)[i]);
    }
    LOGD("convert float array ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvertArray<std::string>(std::vector<std::string> &value)
{
    ani_double length;
    if (ANI_OK != env_->Object_GetPropertyByName_Double(obj_, "length", &length)) {
        LOGE("Object_GetPropertyByName_Double length failed");
        return false;
    }

    for (int i = 0; i < int(length); i++) {
        ani_ref ref;
        if (ANI_OK != env_->Object_CallMethodByName_Ref(obj_, "$_get", "i:C{std.core.Object}", &ref, (ani_int)i)) {
            LOGE("Object_CallMethodByName_Ref failed");
            return false;
        }
        value.push_back(AniStringUtils::ToStd(env_, static_cast<ani_string>(ref)));
    }
    LOGD("convert string array ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvert<int>(int &value)
{
    if (!IsInstanceOfType<int>()) {
        return false;
    }

    ani_int aniValue;
    auto ret = env_->Object_CallMethodByName_Int(obj_, "unboxed", nullptr, &aniValue);
    if (ret != ANI_OK) {
        return false;
    }
    value = static_cast<int>(aniValue);
    LOGD("convert int ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvert<std::monostate>(std::monostate &value)
{
    ani_boolean isNull = false;
    auto status = env_->Reference_IsNull(static_cast<ani_ref>(obj_), &isNull);
    if (ANI_OK == status) {
        if (isNull) {
            value = std::monostate();
            LOGD("convert null ok.");
            return true;
        }
    }
    return false;
}

template<>
bool UnionAccessor::TryConvert<int64_t>(int64_t &value)
{
    return false;
}

template<>
bool UnionAccessor::TryConvert<double>(double &value)
{
    if (!IsInstanceOfType<double>()) {
        return false;
    }

    ani_double aniValue;
    auto ret = env_->Object_CallMethodByName_Double(obj_, "unboxed", nullptr, &aniValue);
    if (ret != ANI_OK) {
        return false;
    }
    value = static_cast<double>(aniValue);
    LOGD("convert double ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvert<std::string>(std::string &value)
{
    if (!IsInstanceOfType<std::string>()) {
        return false;
    }

    value = AniStringUtils::ToStd(env_, static_cast<ani_string>(obj_));
    LOGD("convert string ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvert<bool>(bool &value)
{
    if (!IsInstanceOfType<bool>()) {
        return false;
    }

    ani_boolean aniValue;
    auto ret = env_->Object_CallMethodByName_Boolean(obj_, "unboxed", nullptr, &aniValue);
    if (ret != ANI_OK) {
        return false;
    }
    value = static_cast<bool>(aniValue);
    LOGD("convert bool ok.");
    return true;
}

template<>
bool UnionAccessor::TryConvert<std::vector<uint8_t>>(std::vector<uint8_t> &value)
{
    if (!IsInstanceOf("escompat.Uint8Array")) {
        return false;
    }
    return TryConvertArray(value);
}

template<>
bool UnionAccessor::TryConvert<std::vector<float>>(std::vector<float> &value)
{
    if (!IsInstanceOf("escompat.Float32Array")) {
        return false;
    }
    return TryConvertArray(value);
}

template<>
bool UnionAccessor::TryConvert<std::vector<ani_ref>>(std::vector<ani_ref> &value)
{
    if (!IsInstanceOf("escompat.Array")) {
        return false;
    }
    return TryConvertArray(value);
}

bool AniGetMapItem(ani_env *env, ::taihe::map_view<::taihe::string, uintptr_t> const& taiheMap,
    const char* key, std::string& value)
{
    std::string stdKey(key);
    auto iter = taiheMap.find_item(stdKey);
    if (iter == taiheMap.end()) {
        LOGE("find_item %{public}s failed", stdKey.c_str());
        return false;
    }
    ani_object aniobj = reinterpret_cast<ani_object>(iter->second);
    UnionAccessor access(env, aniobj);
    bool result = access.TryConvert(value);
    return result;
}

bool AniGetMapItem(ani_env *env, ::taihe::map_view<::taihe::string, uintptr_t> const& taiheMap,
    const char* key, int32_t& value)
{
    std::string stdKey(key);
    auto iter = taiheMap.find_item(stdKey);
    if (iter == taiheMap.end()) {
        LOGE("find_item %{public}s failed", stdKey.c_str());
        return false;
    }
    ani_object aniobj = reinterpret_cast<ani_object>(iter->second);
    UnionAccessor access(env, aniobj);
    bool result = access.TryConvert(value);
    return result;
}

} //namespace ani_utils