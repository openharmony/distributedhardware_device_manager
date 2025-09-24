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
#ifndef OHOS_ANI_UTILS_H
#define OHOS_ANI_UTILS_H

#include <memory>
#include <map>
#include <string>
#include <vector>
#include "json_object.h"
#include "taihe/runtime.hpp"
#include "ohos.distributedDeviceManager.proj.hpp"
#include "ohos.distributedDeviceManager.impl.hpp"

namespace ani_utils {

int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, std::string &result,
    bool optional = false);
int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, bool &result,
    bool optional = false);
int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, int32_t &result,
    bool optional = false);
int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, uint32_t &result,
    bool optional = false);
int32_t AniGetProperty(const ani_env *env, ani_object ani_obj, const char *property, ani_object &result,
    bool optional = false);

class AniObjectUtils {
public:
    template<typename T>
    static ani_status Wrap(ani_env *env, ani_object object, T *nativePtr, const char *propName = "nativePtr")
    {
        return env->Object_SetFieldByName_Long(object, propName, reinterpret_cast<ani_long>(nativePtr));
    }

    template<typename T>
    static T* Unwrap(ani_env *env, ani_object object, const char *propName = "nativePtr")
    {
        ani_long nativePtr;
        if (ANI_OK != env->Object_GetFieldByName_Long(object, propName, &nativePtr)) {
            return nullptr;
        }
        return reinterpret_cast<T*>(nativePtr);
    }
};

class AniStringUtils {
public:
    static std::string ToStd(const ani_env *env, ani_string ani_str);
    static ani_string ToAni(const ani_env *env, const std::string& str);
};

class UnionAccessor {
public:
    UnionAccessor(ani_env *env, ani_object &obj) : env_(env), obj_(obj)
    {
    }

    bool IsInstanceOf(const std::string& cls_name)
    {
        ani_class cls;
        env_->FindClass(cls_name.c_str(), &cls);

        ani_boolean ret;
        env_->Object_InstanceOf(obj_, cls, &ret);
        return ret;
    }

    template<typename T>
    bool IsInstanceOfType();

    template<typename T>
    bool TryConvert(T &value);

    template<typename... Types>
    bool TryConvertVariant(std::variant<Types...> &value)
    {
        return GetNativeValue<decltype(value), Types...>(value);
    }

    template<typename T>
    bool GetNativeValue(T &value)
    {
        return false;
    }

    template<typename T, typename First, typename... Types>
    bool GetNativeValue(T &value)
    {
        First cValue;
        auto ret = TryConvert(cValue);
        if (ret == true) {
            value = cValue;
            return ret;
        }
        return GetNativeValue<T, Types...>(value);
    }

    template<typename T>
    bool TryConvertArray(std::vector<T> &value);

private:
    ani_env *env_;
    ani_object obj_;
};

bool AniGetMapItem(ani_env *env, ::taihe::map_view<::taihe::string, uintptr_t> const& taiheMap,
    const char* key, std::string& value);
bool AniGetMapItem(ani_env *env, ::taihe::map_view<::taihe::string, uintptr_t> const& taiheMap,
    const char* key, int32_t& value);

} //namespace ani_utils
#endif

