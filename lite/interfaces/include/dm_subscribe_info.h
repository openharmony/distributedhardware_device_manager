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


#ifndef DM_SUBSCRIBE_INFO_H
#define DM_SUBSCRIBE_INFO_H

#include <stdbool.h>
#include <stdint.h>

#define DM_MAX_DEVICE_CAPABILITY_LEN 65

typedef enum DM_DiscoverMode {
    DM_DISCOVER_MODE_PASSIVE = 0x55,
    DM_DISCOVER_MODE_ACTIVE = 0xAA
} DM_DiscoverMode;

typedef enum DM_ExchangeMedium {
    DM_AUTO = 0,
    DM_BLE = 1,
    DM_COAP = 2,
    DM_USB = 3,
    DM_MEDIUM_BUTT
} DM_ExchangeMedium;

typedef enum DM_ExchangeFreq {
    DM_LOW = 0,
    DM_MID = 1,
    DM_HIGH = 2,
    DM_SUPER_HIGH = 3,
    DM_EXTREME_HIGH = 4,
    DM_FREQ_BUTT
} DM_ExchangeFreq;

typedef struct DmSubscribeInfo {
    uint16_t subscribeId;
    DM_DiscoverMode mode;
    DM_ExchangeMedium medium;
    DM_ExchangeFreq freq;
    bool isSameAccount;
    bool isWakeRemote;
    char capability[DM_MAX_DEVICE_CAPABILITY_LEN];
} DmSubscribeInfo;

#endif
