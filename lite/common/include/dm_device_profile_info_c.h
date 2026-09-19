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


#ifndef DM_DEVICE_PROFILE_INFO_C_H
#define DM_DEVICE_PROFILE_INFO_C_H

#include <stdint.h>
#include "dm_container.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    DmString deviceId;
    DmString deviceName;
    DmString deviceTypeName;
    DmString deviceTypeId;
    int32_t deviceStatus;
    int32_t deviceProfileType;
    DmString networkId;
    int32_t deviceRange;
    int32_t deviceScale;
    int32_t deviceReference;
} DmDeviceProfileInfo;

typedef struct {
    DmString deviceId;
    DmString deviceName;
    int32_t deviceProfileType;
} DmDeviceProfileInfoFilterOptions;

typedef struct {
    DmString deviceId;
    DmString iconUrl;
    int32_t iconWidth;
    int32_t iconHeight;
    int32_t iconDensity;
    int32_t iconSourceType;
    int32_t iconInfoSourceType;
} DmDeviceIconInfo;

typedef struct {
    DmString deviceId;
    int32_t iconWidth;
    int32_t iconHeight;
    int32_t iconDensity;
    int32_t iconSourceType;
    int32_t iconInfoSourceType;
} DmDeviceIconInfoFilterOptions;

typedef struct {
    DmString key;
    DmString value;
} NetworkIdQueryFilter;

typedef struct {
    DmString deviceId;
    DmString key;
    DmString value;
    int32_t status;
} DmServiceProfileInfo;

DM_VEC_DEFINE(DmServiceProfileInfo);

DM_VEC_DEFINE(DmDeviceProfileInfo);

#ifdef __cplusplus
}
#endif

#endif // DM_DEVICE_PROFILE_INFO_C_H
