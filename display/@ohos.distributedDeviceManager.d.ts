/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import type { AsyncCallback } from './@ohos.base';
import type { Callback } from './@ohos.base';

/**
 * Providers interfaces to create a {@link deviceManager} instances.
 *
 * @namespace distributedDeviceManager
 * @syscap SystemCapability.DistributedHardware.DeviceManager
 * @since 10
 */

declare namespace distributedDeviceManager {
  
  /**
   * Basic description information of a distributed device.
   * @interface DeviceBasicInfo
   * @syscap SystemCapability.DistributedHardware.DeviceManager
   * @since 10
   */
  interface DeviceBasicInfo {
    /**
     * Device id.
     * @since 10
     */
    deviceId: string;

    /**
     * Device name.
     * @since 10
     */
    deviceName: string;

    /**
     * Device type.
     * @since 10
     */
    deviceType: number;

    /**
     * Device network id.
     * @since 10
     */
    networkId?: string;
  }

  /**
   * The status of the nearby devices.
   * @enum { DeviceStatusChange }
   * @since 10
   * @syscap SystemCapability.DistributedHardware.DeviceManager
   */
  enum DeviceStatusChange {
    /**
     * Unknown status.
     * @since 10
     */
    UNKNOWN = 0,

    /**
     * Available status.
     * @since 10
     */
    AVAILABLE = 1,

    /**
     * Unavailable status.
     * @since 10
     */
    UNAVAILABLE = 2,
  }

  /**
   * Device bind parameter. The input parameter of the function {@link bindTarget}.
   * @interface BindParam
   * @syscap SystemCapability.DistributedHardware.DeviceManager
   * @since 10
   */
  interface BindParam {
    /**
     * Bind type, 1 for pin code.
     * @since 10
     */
    bindType: number;

    /**
     * Extra information.
     * Such as:
     * targetPkgName - The name of binding target.
     * appName - The name of application.
     * appOperation - The operation of application.
     * @since 10
     */
    extraInfo: { [key: string]: Object };
  }

  /**
   * Creates an {@code DeviceManager} instance.
   *
   * <p>To manage devices, you must first call this method to obtain a {@code DeviceManager} instance and then
   * use this instance to call other device management methods.
   *
   * @since 10
   * @param bundleName Indicates the bundle name of the application.
   * @param callback Indicates the callback to be invoked upon {@code DeviceManager} instance creation.
   * @throws { BusinessError } 401 - Input parameter error.
   */
  function createDeviceManager(bundleName: string, callback: AsyncCallback<DeviceManager>): void;

  /**
   * Provides methods for managing devices.
   *
   * @interface DeviceManager
   * @syscap SystemCapability.DistributedHardware.DeviceManager
   * @since 10
   */
  interface DeviceManager {
    /**
     * Releases the {@code DeviceManager} instance that is no longer used.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     */
    releaseDeviceManager(): void;

    /**
     * Get a list of available devices. This interface query all authorized and connectable devices.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @returns Returns a list of available devices.
     */
    getAvailableDeviceListSync(): Array<DeviceBasicInfo>;

    /**
     * Get a list of available devices. This interface query all authorized and connectable devices.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param callback Indicates the callback to be invoked upon getAvailableDeviceList
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @returns Returns a list of available devices.
     */
    getAvailableDeviceList(callback: AsyncCallback<Array<DeviceBasicInfo>>): void;

    /**
     * Get a list of available devices. This interface query all authorized and connectable devices.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @returns Returns a list of available devices.
     */
    getAvailableDeviceList(): Promise<Array<DeviceBasicInfo>>;

    /**
     * Get the network id of the local device.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @returns Returns local device network id.
     */
    getLocalDeviceNetworkIdSync(): string;

    /**
     * Get the device name of the local device.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @returns Returns local device name.
     */
    getLocalDeviceNameSync(): string;

    /**
     * Get the device type of the local device.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @returns Returns local device type.
     */
    getLocalDeviceTypeSync(): number;

    /**
     * Get the device id of the local device.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @returns Returns local device type.
     */
    getLocalDeviceIdSync(): string;

    /**
     * Get the device name by network id.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { string } networkId - Device network id.
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @returns Returns device name.
     */
    getDeviceNameSync(networkId: string): string;

    /**
     * Get the device type by network id.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { string } networkId - Device network id.
     * @throws { BusinessError } 201 - User permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @returns Returns device type.
     */
    getDeviceTypeSync(networkId: string): number;

    /**
     * Start to discover target.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param subscribeId subscribe id to discovery device
     * @param filterOptions filterOptions to filter discovery device
     * @throws { BusinessError } 401 - Input parameter error.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 11600104 - Discovery invalid.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     */
    startDiscovering(subscribeId: number, filterOptions?: string): void;

    /**
     * Stop discovering target.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param subscribeId service subscribe id
     * @throws { BusinessError } 401 - Input parameter error.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     */
    stopDiscovering(subscribeId: number): void;

    /**
     * Bind the specified target.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param deviceId id of device to bind
     * @param bindParam parameters of device to bind
     * @param callback indicates the callback to be invoked upon bindDevice
     * @throws { BusinessError } 401 - Input parameter error.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     * @throws { BusinessError } 11600103 - Bind invalid.
     */
    bindTarget(deviceId: string, bindParam: BindParam, callback: AsyncCallback<{deviceId: string}>): void;

    /**
     * Unbind the specified target.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param deviceId id of device to unbind
     * @throws { BusinessError } 401 - Input parameter error.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 11600101 - Failed to execute the function.
     */
    unbindTarget(deviceId: string): void;

    /**
     * The reply of ui operation from pin-code window, this interface can only be used by pin-code-hap of devicemanager.
     *
     * @permission ohos.permission.ACCESS_SERVICE_DM
     * @since 10
     * @param action The reply action of user operation.
     * @param params Indicates the input param of the user.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 202 - The caller is not a system application.
     * @throws { BusinessError } 401 - Input parameter error.
     * @systemapi this method can be used only by system applications.
     */
    replyUiAction(action: number, params: string): void;

    /**
     * Register a device status callback so that the application can be notified upon device status changes based on
     * the application bundle name.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'deviceStatusChange' } type Device status change.
     * @param { Callback<{ action: DeviceStatusChange, device: DeviceBasicInfo }> } callback
     *          Indicates the device status callback to register.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    on(type: 'deviceStatusChange', callback: Callback<{ action: DeviceStatusChange, device: DeviceBasicInfo }>): void;

    /**
     * UnRegister device status callback based on the application bundle name.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'deviceStatusChange' } type Device status change.
     * @param { Callback<{ action: DeviceStatusChange, device: DeviceBasicInfo }> } callback
     *          Indicates the device status callback to unregister.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    off(type: 'deviceStatusChange', callback?: Callback<{ action: DeviceStatusChange, device: DeviceBasicInfo }>): void;

    /**
     * Register a device discovery result callback so that the application can be notified when discovery success.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'discoverSuccess' } type Successfully discovered device.
     * @param { Callback<{ subscribeId: number, device: DeviceBasicInfo }> } callback
     *          Indicates the device discovery callback to register.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    on(type: 'discoverSuccess', callback: Callback<{ subscribeId: number, device: DeviceBasicInfo }>): void;

    /**
     * UnRegister the device discovery result callback.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'discoverSuccess' } type Successfully discovered device.
     * @param { Callback<{ subscribeId: number, device: DeviceBasicInfo }> } callback
     *          Indicates the device discovery callback to unregister.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    off(type: 'discoverSuccess', callback?: Callback<{ subscribeId: number, device: DeviceBasicInfo }>): void;

    /**
     * Register a device name change callback so that the application can be notified when discovery success.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'deviceNameChange' } type Changed device name.
     * @param { Callback<{ deviceName: string }> } callback Indicates the device name change callback to register.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    on(type: 'deviceNameChange', callback: Callback<{ deviceName: string }>): void;

    /**
     * UnRegister the device name change result callback.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'deviceNameChange' } type Changed device name.
     * @param { Callback<{ deviceName: string }> } callback Indicates the device name change callback to unregister.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    off(type: 'deviceNameChange', callback?: Callback<{ deviceName: string }>): void;

    /**
     * Register a device discovery result callback so that the application can be notified when discover failed.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'discoverFail' } type Discovery Device Failure.
     * @param { Callback<{ subscribeId: number, reason: number }> } callback
     *          Indicates the device found result callback to register.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    on(type: 'discoverFail', callback: Callback<{ subscribeId: number, reason: number }>): void;

    /**
     * UnRegister the device discovery result callback.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'discoverFail' } type Discovery Device Failure.
     * @param { Callback<{ subscribeId: number, reason: number }> } callback
     *          Indicates the device found result callback to unregister.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    off(type: 'discoverFail', callback?: Callback<{ subscribeId: number, reason: number }>): void;

    /**
     * Register a serviceError callback so that the application can be notified when devicemanager service died
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'serviceDie' } type Service death.
     * @param { function } callback Indicates the service error callback to register.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    on(type: 'serviceDie', callback: () => void): void;

    /**
     * UnRegister the service error callback.
     *
     * @permission ohos.permission.DISTRIBUTED_DATASYNC
     * @since 10
     * @param { 'serviceDie' } type Service death.
     * @param { function } callback Indicates the service error callback to unregister.
     * @throws { BusinessError } 201 - Permission verify failed.
     * @throws { BusinessError } 401 - Input parameter error.
     */
    off(type: 'serviceDie', callback?: () => void): void;

    /**
     * Register a callback from deviceManager service so that the devicemanager ui can be notified when ui statue
     * changes.
     *
     * @permission ohos.permission.ACCESS_SERVICE_DM
     * @param { 'replyResult' } type Ui reply result to register.
     * @param { Callback<{ param: string }> } callback Indicates the devicemanager ui state to register.
     * @throws { BusinessError } 401 - Input parameter error.
     * @throws { BusinessError } 202 - The caller is not a system application.
     * @syscap SystemCapability.DistributedHardware.DeviceManager
     * @systemapi this method can be used only by system applications.
     * @since 10
     */
    on(type: 'replyResult', callback: Callback<{ param: string }>): void;

    /**
      * Unregister uiStatueChange, this interface can only be used by devicemanager ui.
      *ui state change
      *
      * @permission ohos.permission.ACCESS_SERVICE_DM
      * @param { 'replyResult' } type Ui reply result to unregister.
      * @param { Callback<{ param: string }> } callback Indicates the devicemanager ui state to unregister.
      * @throws { BusinessError } 401 - Input parameter error.
      * @throws { BusinessError } 202 - The caller is not a system application. 
      * @syscap SystemCapability.DistributedHardware.DeviceManager
      * @systemapi this method can be used only by system applications.
      * @since 10
      */
    off(type: 'replyResult', callback?: Callback<{ param: string }>): void;
  }
}

export default distributedDeviceManager;
