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

/**
 * @addtogroup DeviceManager
 * @{
 *
 * @brief Provides APIs to obtain information about trusted devices and local devices.
 *
 * @since 20
 */

/**
 * @file oh_device_manager.h
 *
 * @brief Provides APIs of the DeviceManager module.
 *
 * @kit DistributedServiceKit
 * @library libdevicemanager_ndk.so
 * @syscap SystemCapability.DistributedHardware.DeviceManager
 *
 * @since 20
 */

#ifndef OH_DEVICE_MANAGER_H
#define OH_DEVICE_MANAGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get local device name.
 *
 * @param localDeviceName This is an output parameter. It indicates the address pointer of the localDeviceName.
 *                        You need to manually release space resources after using.
 * @param len This is an output parameter. Legnth of the localDeviceName.
 * @return Returns teh status code of the execution. For detail, see {@link DeviceManager_ErrorCode}.
 *         Returns {@link ERR_OK}, The operation is successful.
 *         Returns {@link DM_ERR_FAILED}, Failed to execute the function.
 *         Returns {@link DM_ERR_OBTAIN_SERVICE}, Failed to obtain the service.
 *         Returns {@link DM_ERR_OBTAIN_BUNDLE_NAME}, Failed to obtain the bundleName.
 * @since 20
 */
int32_t OH_DeviceManager_GetLocalDeviceName(char **localDeviceName, unsigned int &len);

#ifdef __cplusplus
};
#endif

/** @} */
#endif