/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_DEVICE_INFO_H
#define OHOS_DM_DEVICE_INFO_H

#include <cstdint>
#include <string>

#include "dm_app_image_info.h"

#define DM_MAX_DEVICE_ID_LEN (96)
#define DM_MAX_DEVICE_NAME_LEN (128)

namespace OHOS {
namespace DistributedHardware {
/**
 * @brief Device manager event notify.
 */
typedef enum DmNotifyEvent {
    /**
     * Device manager start event.
     */
    DM_NOTIFY_EVENT_START = 0,
    /**
     * Device manager on ready event.
     */
    DM_NOTIFY_EVENT_ONDEVICEREADY,
    // Add event here
    DM_NOTIFY_EVENT_BUTT,
} DmNotifyEvent;

/**
 * @brief Device Type definitions.
 */
typedef enum DmDeviceType {
    /**
     * Indicates an unknown device type.
     */
    DEVICE_TYPE_UNKNOWN = 0x00,
    /**
     * Indicates a smart camera.
     */
    DEVICE_TYPE_WIFI_CAMERA = 0x08,
    /**
     * Indicates a smart speaker.
     */
    DEVICE_TYPE_AUDIO = 0x0A,
    /**
     * Indicates a smart pc.
     */
    DEVICE_TYPE_PC = 0x0C,
    /**
     * Indicates a smart phone.
     */
    DEVICE_TYPE_PHONE = 0x0E,
    /**
     * Indicates a smart pad.
     */
    DEVICE_TYPE_PAD = 0x11,
    /**
     * Indicates a smart watch.
     */
    DEVICE_TYPE_WATCH = 0x6D,
    /**
     * Indicates a car.
     */
    DEVICE_TYPE_CAR = 0x83,
    /**
     * Indicates a smart TV.
     */
    DEVICE_TYPE_TV = 0x9C,
} DmDeviceType;

/**
 * @brief Device state change event definition.
 */
typedef enum DmDeviceState {
    /**
     * Device status is unknown.
     */
    DEVICE_STATE_UNKNOWN = -1,
    /**
     * Device online action, which indicates the device is physically online.
     */
    DEVICE_STATE_ONLINE = 0,
    /**
     * Device ready action, which indicates the information between devices has
     * been synchronized in the Distributed Data Service (DDS) module,
     * and the device is ready for running distributed services.
     */
    DEVICE_INFO_READY = 1,
    /**
     * Device offline action, which Indicates the device is physically offline.
     */
    DEVICE_STATE_OFFLINE = 2,
    /**
     * Device change action, which Indicates the device is physically change.
     */
    DEVICE_INFO_CHANGED = 3,
} DmDeviceState;

/**
 * @brief Device authentication form.
 */
typedef enum DmAuthForm {
    /**
     * Device Auth invalid.
     */
    INVALID_TYPE = -1,
    /**
     * Peer To Peer Device auth.
     */
    PEER_TO_PEER = 0,
    /**
     * Identical Account Device auth.
     */
    IDENTICAL_ACCOUNT = 1,
    /**
     * Across Account Device auth.
     */
    ACROSS_ACCOUNT = 2,
} DmAuthForm;

/**
 * @brief Device Information.
 */
typedef struct DmDeviceInfo {
    /**
     * Device Id of the device.
     */
    char deviceId[DM_MAX_DEVICE_ID_LEN];
    /**
     * Device name of the device.
     */
    char deviceName[DM_MAX_DEVICE_NAME_LEN];
    /**
     * Device type of the device.
     */
    uint16_t deviceTypeId;
    /**
     * NetworkId of the device.
     */
    char networkId[DM_MAX_DEVICE_ID_LEN];
    /**
     * The distance of discovered device, in centimeter(cm).
     */
    int32_t range;
    /**
     * NetworkType of the device.
     */
    char networkType[DM_MAX_DEVICE_ID_LEN];
    /**
     * Device authentication form.
     */
    DmAuthForm authForm;
} DmDeviceInfo;

/**
 * @brief Device Basic Information.
 */
typedef struct
DmDeviceBasicInfo {
    /**
     * Device Id of the device.
     */
    char deviceId[DM_MAX_DEVICE_ID_LEN];
    /**
     * Device name of the device.
     */
    char deviceName[DM_MAX_DEVICE_NAME_LEN];
    /**
     * Device type of the device.
     */
    uint16_t deviceTypeId;
    /**
     * NetworkId of the device.
     */
    char networkId[DM_MAX_DEVICE_ID_LEN];
    char networkType[DM_MAX_DEVICE_ID_LEN];
} DmDeviceBasicInfo;

/**
 * @brief Device Authentication param.
 */
typedef struct DmAuthParam {
    /**
     * the token used for this authentication.
     */
    std::string authToken;
    /**
     * the package name used for this authentication.
     */
    std::string packageName;
    /**
     * the app name used for this authentication.
     */
    std::string appName;
    /**
     * the app description used for this authentication.
     */
    std::string appDescription;
    /**
     * the auth type used for this authentication.
     */
    int32_t authType;
    /**
     * the business used for this authentication.
     */
    int32_t business;
    /**
     * the pin code used for this authentication.
     */
    int32_t pincode;
    /**
     * the direction used for this authentication.
     */
    int32_t direction;
    /**
     * the pin token used for this authentication.
     */
    int32_t pinToken;
    /**
     * the app image info used for this authentication.
     */
    DmAppImageInfo imageinfo;
} DmAuthParam;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DEVICE_INFO_H
