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


#ifndef DM_RANDOM_H
#define DM_RANDOM_H

#include <stdint.h>
#include <stdbool.h>
#include "dm_container.h"

#ifndef DM_EXPORT
#define DM_EXPORT __attribute__((visibility("default")))
#endif

DM_EXPORT int32_t DmGenRandInt(int32_t randMin, int32_t randMax);
DM_EXPORT int64_t DmGenRandLongLong(int64_t randMin, int64_t randMax);
DM_EXPORT uint64_t DmGenRandUnLongLong(uint64_t randMin, uint64_t randMax);
DM_EXPORT uint16_t DmGenRandUint(uint16_t randMin, uint16_t randMax);
DM_EXPORT uint16_t DmGenUniqueRandUint(DmSet_int* randUint16Set);

#endif
