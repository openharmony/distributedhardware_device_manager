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


#ifndef DM_LOG_H
#define DM_LOG_H

#include "hilog/log.h"
#include "dm_constants.h"

#undef LOG_TAG
#define LOG_TAG "DHDM"

#define LOGD(fmt, ...) HILOG_DEBUG(LOG_CORE, \
    "[%{public}s][%{public}s]:" fmt, DH_LOG_TAG, __FUNCTION__, ##__VA_ARGS__)

#define LOGI(fmt, ...) HILOG_INFO(LOG_CORE, \
    "[%{public}s][%{public}s]:" fmt, DH_LOG_TAG, __FUNCTION__, ##__VA_ARGS__)

#define LOGW(fmt, ...) HILOG_WARN(LOG_CORE, \
    "[%{public}s][%{public}s]:" fmt, DH_LOG_TAG, __FUNCTION__, ##__VA_ARGS__)

#define LOGE(fmt, ...) HILOG_ERROR(LOG_CORE, \
    "[%{public}s][%{public}s]:" fmt, DH_LOG_TAG, __FUNCTION__, ##__VA_ARGS__)

#define CHECK_NULL_VOID(ptr)                    \
    do {                                        \
        if ((ptr) == NULL) {                    \
            LOGE("Address pointer is null");    \
            return;                             \
        }                                       \
    } while (0)

#define CHECK_NULL_RETURN(ptr, ret)             \
    do {                                        \
        if ((ptr) == NULL) {                    \
            LOGE("Address pointer is null");    \
            return (ret);                       \
        }                                       \
    } while (0)

#define CHECK_SIZE_VOID(sz)                     \
    do {                                        \
        if ((sz) >= DM_MAX_CONTAINER_SIZE) {    \
            LOGE("container size is more than max size"); \
            return;                             \
        }                                       \
    } while (0)

#define CHECK_SIZE_RETURN(sz, ret)              \
    do {                                        \
        if ((sz) >= DM_MAX_CONTAINER_SIZE) {    \
            LOGE("container size is more than max size"); \
            return (ret);                       \
        }                                       \
    } while (0)

#define CHECK_EMPTY_RETURN(is_empty, ret)       \
    do {                                        \
        if ((is_empty)) {                       \
            LOGE("container is empty");         \
            return (ret);                       \
        }                                       \
    } while (0)

#endif
