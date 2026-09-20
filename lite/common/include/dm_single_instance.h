/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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


#ifndef OHOS_DM_SINGLE_INSTANCE_H
#define OHOS_DM_SINGLE_INSTANCE_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus

#define DM_DECLARE_SINGLE_INSTANCE(className) \
public:                                               \
    static __typeof__(className)* GetInstance();                  \
    static void DestroyInstance();                    \
private:                                              \
    className();                                     \
    ~className();                                    \
    className(const className&) = delete;             \
    className& operator=(const (className)&) = delete; \
    static __typeof__(className)* instance_

#define DM_DEFINE_INSTANCE_GET(className) \
    __typeof__(className)* className::GetInstance() { \
        if (instance_ == nullptr) { instance_ = new className(); } \
        return instance_; \
    }

#define DM_DEFINE_INSTANCE_DESTROY(className) \
    __typeof__(className)* className::instance_ = nullptr; \
    void className::DestroyInstance() { \
        if (instance_ != nullptr) { delete instance_; instance_ = nullptr; } \
    }

#define DM_DEFINE_SINGLE_INSTANCE(className) \
DM_DEFINE_INSTANCE_GET(className) \
DM_DEFINE_INSTANCE_DESTROY(className)

#else

#define DM_DECLARE_SINGLE_INSTANCE(className) \
    __typeof__(className)* className##GetInstance(void);  \
    void className##DestroyInstance(void); \
    static __typeof__(className)* className##Instance_

#define DM_DEFINE_INSTANCE_GET_C(className) \
    __typeof__(className)* className##GetInstance(void) { \
        if (className##Instance_ == NULL) { \
            className##Instance_ = (className*)calloc(1, sizeof(className)); \
        } \
        return className##Instance_; \
    }

#define DM_DEFINE_INSTANCE_DESTROY_C(className) \
    static __typeof__(className)* className##Instance_ = NULL; \
    void className##DestroyInstance(void) { \
        if (className##Instance_ != NULL) { free(className##Instance_); className##Instance_ = NULL; } \
    }

#define DM_DEFINE_SINGLE_INSTANCE(className) \
DM_DEFINE_INSTANCE_DESTROY_C(className) \
DM_DEFINE_INSTANCE_GET_C(className)

#endif

#define DM_IMPLEMENT_SINGLE_INSTANCE DM_DEFINE_SINGLE_INSTANCE

#endif // OHOS_DM_SINGLE_INSTANCE_H
