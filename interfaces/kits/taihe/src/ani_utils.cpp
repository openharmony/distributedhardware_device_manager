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

const ani_size MAX_REASONABLE_STRING_SIZE = 1024 * 1024; // 1MB

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
    if (ani_field_value < 0) {
        return ANI_INVALID_ARGS;
    }
    result = static_cast<uint32_t>(ani_field_value);
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

    if (strSize > MAX_REASONABLE_STRING_SIZE) {
        LOGE("String size too large: %{public}zu", strSize);
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

ani_status AniCreateInt(ani_env* env, int32_t value, ani_object& result)
{
    ani_status state;
    ani_class intClass;
    if ((state = env->FindClass("std.core.Int", &intClass)) != ANI_OK) {
        LOGE("FindClass std/core/Int failed, %{public}d", state);
        return state;
    }
    ani_method intClassCtor;
    if ((state = env->Class_FindMethod(intClass, "<ctor>", "i:", &intClassCtor)) != ANI_OK) {
        LOGE("Class_FindMethod Int ctor failed, %{public}d", state);
        return state;
    }
    ani_int aniValue = value;
    if ((state = env->Object_New(intClass, intClassCtor, &result, aniValue)) != ANI_OK) {
        LOGE("New Int object failed, %{public}d", state);
    }
    if (state != ANI_OK) {
        result = nullptr;
    }
    return state;
}

ani_string AniCreateString(ani_env *env, const std::string &para)
{
    if (env == nullptr) {
        return {};
    }
    ani_string ani_string_result = nullptr;
    if (env->String_NewUTF8(para.c_str(), para.size(), &ani_string_result) != ANI_OK) {
        return {};
    }
    return ani_string_result;
}

ani_method AniGetMethod(ani_env *env, ani_class cls, const char* methodName, const char* signature)
{
    if (env == nullptr) {
        return nullptr;
    }
    ani_method retMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, methodName, signature, &retMethod)) {
        return nullptr;
    }
    return retMethod;
}

ani_class AniGetClass(ani_env *env, const char* className)
{
    if (env == nullptr) {
        return nullptr;
    }
    ani_class cls {};
    if (ANI_OK != env->FindClass(className, &cls)) {
        return nullptr;
    }
    return cls;
}

ani_method AniGetClassMethod(ani_env *env, const char* className, const char* methodName, const char* signature)
{
    ani_class retClass = AniGetClass(env, className);
    if (retClass == nullptr) {
        return nullptr;
    }
    ani_method retMethod {};
    if (ANI_OK != env->Class_FindMethod(retClass, methodName, signature, &retMethod)) {
        return nullptr;
    }
    return retMethod;
}

ani_object AniCreatEmptyRecord(ani_env* env, ani_method& setMethod)
{
    ani_method constructor = ani_utils::AniGetClassMethod(env, "escompat.Record", "<ctor>", ":");
    ani_method mapSetMethod = ani_utils::AniGetClassMethod(env, "escompat.Record", "$_set", nullptr);
    if (constructor == nullptr || mapSetMethod == nullptr) {
        LOGE("AniGetClassMethod escompat.Record find method failed");
        return nullptr;
    }
    ani_object ani_record_result = nullptr;
    if (ANI_OK != env->Object_New(ani_utils::AniGetClass(env, "escompat.Record"), constructor, &ani_record_result)) {
        LOGE("escompat.Record Object_New failed");
        return nullptr;
    }
    setMethod = mapSetMethod;
    return ani_record_result;
}

ani_array AniCreateEmptyAniArray(ani_env *env, uint32_t size)
{
    if (env == nullptr) {
        LOGE("create ani array env is null");
        return nullptr;
    }
    ani_ref undefinedRef = nullptr;
    if (ANI_OK != env->GetUndefined(&undefinedRef)) {
        LOGE("GetUndefined Failed.");
        return nullptr;
    }
    ani_array resultArray;
    env->Array_New(size, undefinedRef, &resultArray);
    return resultArray;
}

ani_object AniCreateArray(ani_env *env, const std::vector<ani_object> &objectArray)
{
    ani_array array = AniCreateEmptyAniArray(env, objectArray.size());
    if (array == nullptr) {
        LOGE("Create array failed");
        return nullptr;
    }
    ani_size index = 0;
    for (auto &aniItem : objectArray) {
        if (ANI_OK != env->Array_Set(array, index, aniItem)) {
            LOGE("Set array failed, index=%{public}zu", index);
            return nullptr;
        }
        index++;
    }
    return array;
}

bool AniMapSet(ani_env *env, ani_object map, ani_method mapSetMethod, const char* key, ani_ref value)
{
    if (env == nullptr || map == nullptr || mapSetMethod == nullptr
        || key == nullptr || key[0] == 0 || value == nullptr) {
        return false;
    }
    ani_ref keyref = AniCreateString(env, std::string(key));
    if (keyref == nullptr) {
        LOGE("AniCreateString failed");
        return false;
    }
    if (ANI_OK != env->Object_CallMethod_Void(map, mapSetMethod, keyref, value)) {
        LOGE("Object_CallMethod_Void failed");
        return false;
    }
    return true;
}

bool AniMapSet(ani_env *env, ani_object map, ani_method mapSetMethod, const char* key, const std::string &valueStr)
{
    if (valueStr.empty()) {
        return false;
    }
    ani_ref valueRef = AniCreateString(env, valueStr);
    if (valueRef == nullptr) {
        LOGE("AniCreateString failed");
        return false;
    }
    return AniMapSet(env, map, mapSetMethod, key, valueRef);
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
bool UnionAccessor::TryConvert<int>(int &value)
{
    if (!IsInstanceOfType<int>()) {
        return false;
    }

    ani_int aniValue;
    auto ret = env_->Object_CallMethodByName_Int(obj_, "toInt", nullptr, &aniValue);
    if (ret != ANI_OK) {
        LOGE("toInt failed");
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
    auto ret = env_->Object_CallMethodByName_Double(obj_, "toDouble", nullptr, &aniValue);
    if (ret != ANI_OK) {
        LOGE("toDouble failed");
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
    auto ret = env_->Object_CallMethodByName_Boolean(obj_, "toBoolean", nullptr, &aniValue);
    if (ret != ANI_OK) {
        LOGE("toBoolean failed");
        return false;
    }
    value = static_cast<bool>(aniValue);
    LOGD("convert bool ok.");
    return true;
}

bool AniGetMapItem(ani_env *env, const ::taihe::map_view<::taihe::string, uintptr_t> &taiheMap,
    const char* key, std::string& value)
{
    if (env == nullptr) {
        return false;
    }
    std::string stdKey(key);
    auto iter = taiheMap.find_item(stdKey);
    if (iter == taiheMap.end()) {
        LOGE("find_item %{public}s failed", stdKey.c_str());
        return false;
    }
    if (iter->second == 0) {
        LOGE("Null pointer value for key: %{public}s", stdKey.c_str());
        return false;
    }
    ani_object aniobj = reinterpret_cast<ani_object>(iter->second);
    UnionAccessor access(env, aniobj);
    bool result = access.TryConvert(value);
    return result;
}

bool AniGetMapItem(ani_env *env, const ::taihe::map_view<::taihe::string, uintptr_t> &taiheMap,
    const char* key, int32_t& value)
{
    if (env == nullptr) {
        return false;
    }
    std::string stdKey(key);
    auto iter = taiheMap.find_item(stdKey);
    if (iter == taiheMap.end()) {
        LOGE("find_item %{public}s failed", stdKey.c_str());
        return false;
    }
    if (iter->second == 0) {
        LOGE("Null pointer value for key: %{public}s", stdKey.c_str());
        return false;
    }
    ani_object aniobj = reinterpret_cast<ani_object>(iter->second);
    UnionAccessor access(env, aniobj);
    bool result = access.TryConvert(value);
    return result;
}

void AniExecuteFunc(ani_vm* vm, const std::function<void(ani_env*)> func)
{
    LOGI("AniExecutePromise");
    if (vm == nullptr || !func) {
        LOGE("AniExecutePromise, vm error");
        return;
    }
    ani_env *currentEnv = nullptr;
    ani_status aniResult = vm->GetEnv(ANI_VERSION_1, &currentEnv);
    if (ANI_OK == aniResult && currentEnv != nullptr) {
        LOGI("AniExecutePromise, env exist");
        func(currentEnv);
        return;
    }

    ani_env* newEnv = nullptr;
    ani_options aniArgs { 0, nullptr };
    aniResult = vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &newEnv);
    if (ANI_OK != aniResult || newEnv == nullptr) {
        LOGE("AniExecutePromise, AttachCurrentThread error");
        return;
    }
    func(newEnv);
    aniResult = vm->DetachCurrentThread();
    if (ANI_OK != aniResult) {
        LOGE("AniExecutePromise, DetachCurrentThread error");
        return;
    }
}

} //namespace ani_utils