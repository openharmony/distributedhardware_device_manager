/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
import deviceManager from '@ohos.distributedHardware.deviceManager';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import deviceInfo from '@ohos.deviceInfo';
import Constant from '../common/constant';
import common from '@ohos.app.ability.common';

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:ConfirmDialog]==>';
const ACTION_ALLOW_AUTH_ONCE: number = 0;
const ACTION_CANCEL_AUTH: number = 1;
const ACTION_AUTH_CONFIRM_TIMEOUT: number = 2;
const ACTION_ALLOW_AUTH_ALWAYS: number = 6;
const MSG_CANCEL_CONFIRM_SHOW: number = 5;
const DEVICE_TYPE_2IN1: number = 0xA2F;
const DEVICE_TYPE_PC: number = 0x0C;

@CustomDialog
struct ConfirmCustomDialog {
  @State peerAppOperation: string = '';
  @State peerCustomDescription: string = '';
  @State peerDeviceName: string = '';
  @State peerDeviceType: number = 0;
  @State secondsNum: number = 30;
  @State times: number = 0;
  @State isAvailableType: boolean = false;
  @State btnColor: ResourceColor = Color.Transparent;
  @State titleFirst: string = '';
  controller?: CustomDialogController;
  isPC: boolean = false;

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear execute PinCustomDialog')
    let context = getContext() as common.UIAbilityContext;
    if (AppStorage.get('deviceName') != null) {
      this.peerDeviceName = AppStorage.get('deviceName') as string
      console.log('peerDeviceName is ' + this.peerDeviceName)
    }
    this.titleFirst =
      context.resourceManager.getStringSync($r('app.string.dm_connect_device').id, this.peerDeviceName);

    if (AppStorage.get('deviceType') != null) {
      this.peerDeviceType = AppStorage.get('deviceType') as number
      console.log('peerDeviceType is ' + this.peerDeviceType)
    }

    if (AppStorage.get('appOperationStr') != null) {
      this.peerAppOperation = AppStorage.get('appOperationStr') as string
      console.log('peerAppOperation is ' + this.peerAppOperation)
      this.titleFirst = this.peerDeviceName + ' ' + this.peerAppOperation;
    }

    if (AppStorage.get('customDescriptionStr') != null) {
      this.peerCustomDescription = AppStorage.get('customDescriptionStr') as string
      console.log('peerCustomDescription is ' + this.peerCustomDescription)
    }
    this.times = setInterval(() => {
      console.info('devicemanagerui confirm dialog run seconds:' + this.secondsNum);
      this.secondsNum--;
      if (this.secondsNum === 0) {
        clearInterval(this.times);
        this.times = 0;
        this.setUserOperation(ACTION_AUTH_CONFIRM_TIMEOUT);
        this.destruction();
        console.info('click cancel times run out');
      }
    }, 1000)
    console.log(TAG + 'deviceInfo.deviceType:' + deviceInfo.deviceType);
    this.isPC = Constant.isPC();
  }

  onAllowOnce() {
    console.log('allow once')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log('allow once' + ACTION_ALLOW_AUTH_ONCE)
    this.setUserOperation(ACTION_ALLOW_AUTH_ONCE)
    this.destruction()
  }

  onAllowAlways() {
    console.log('allow always')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log('allow always' + ACTION_ALLOW_AUTH_ALWAYS)
    this.setUserOperation(ACTION_ALLOW_AUTH_ALWAYS)
    this.destruction()
  }

  onCancel() {
    console.log('cancel')
    if (dmClass == null) {
      console.log('createDeviceManager is null')
      return
    }

    console.log('cancel' + ACTION_CANCEL_AUTH)
    this.setUserOperation(ACTION_CANCEL_AUTH)
    this.destruction()
  }

  setUserOperation(operation: number) {
    console.log(TAG + 'setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, 'extra');
    } catch (error) {
      console.log(TAG + 'dmClass setUserOperation failed')
    }
  }

  destruction() {
    if (dmClass != null) {
      try {
        dmClass.release();
        dmClass = null;
      } catch (error) {
        console.log('dmClass release failed')
      }
    }
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  getImages(peerdeviceType: number): Resource {
    console.info('peerdeviceType is ' + peerdeviceType)
    if (peerdeviceType == deviceManager.DeviceType.SPEAKER) {
      this.isAvailableType = true
      return $r('app.media.ic_device_soundx')
    } else if (peerdeviceType == deviceManager.DeviceType.PHONE) {
      this.isAvailableType = true
      return $r('app.media.ic_public_devices_phone')
    } else if (peerdeviceType == deviceManager.DeviceType.TABLET) {
      this.isAvailableType = true
      return $r('app.media.ic_device_pad')
    } else if (peerdeviceType == deviceManager.DeviceType.WEARABLE) {
      this.isAvailableType = true
      return $r('app.media.ic_device_watch')
    } else if (peerdeviceType == deviceManager.DeviceType.CAR) {
      this.isAvailableType = true
      return $r('app.media.ic_public_car')
    } else if (peerdeviceType == deviceManager.DeviceType.TV) {
      this.isAvailableType = true
      return $r('app.media.ic_device_smartscreen')
    } else if (peerdeviceType == DEVICE_TYPE_PC) {
      this.isAvailableType = true
      return $r('app.media.ic_device_matebook')
    } else if (peerdeviceType == DEVICE_TYPE_2IN1) {
      this.isAvailableType = true
      return $r('app.media.ic_device_matebook')
    } else {
      this.isAvailableType = false
      return $r('app.media.icon')
    }
  }

  build() {
    GridRow({
      columns: { xs: 4, sm: 8, md: this.isPC ? 24 : 12 },
      gutter: { x: 4 },
      breakpoints: { value: ['520vp', '840vp'] }
    }) {
      GridCol({ span: { xs: 4, sm: 4, md: this.isPC ? 6 : 4 }, offset: { sm: 2, md: this.isPC ? 9 : 4 } }) {
        Column() {
          Image(this.getImages(this.peerDeviceType))
            .width(24)
            .height(24)
            .margin({ bottom: 16, top: 24 })
            .visibility(this.isAvailableType == false ? Visibility.None : Visibility.Visible)
          Column() {
            Text($r('app.string.dm_is_trust_device', this.titleFirst))
              .textAlign(TextAlign.Start)
              .fontColor($r('sys.color.ohos_id_color_text_primary'))
              .fontWeight(FontWeight.Medium)
              .fontSize($r('sys.float.ohos_id_text_size_body1'))
              .maxLines(2)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .width('100%')
            Text(this.peerCustomDescription)
              .textAlign(TextAlign.Start)
              .fontColor($r('sys.color.ohos_id_color_text_secondary'))
              .fontWeight(FontWeight.Regular)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .fontSize($r('sys.float.ohos_id_text_size_body2'))
              .maxLines(3)
              .constraintSize({ maxHeight: 45 })
              .width('100%')
              .margin({ top: 2 })
              .visibility(this.peerCustomDescription === '' ? Visibility.None : Visibility.Visible)
          }.margin({
            top: this.isAvailableType ? 0 : 24,
            bottom: 16, left: 24, right: 24 })

          Column() {
            Button($r('app.string.dm_allow_always'))
              .margin({ bottom: 4 })
              .onClick(() => {
                this.onAllowAlways();
              })
              .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
              .height(40)
              .width(this.isPC ? 250 : '100%')
              .backgroundColor(this.btnColor)
              .onHover((isHover?: boolean, event?: HoverEvent): void => {
                if (isHover) {
                  this.btnColor = $r('sys.color.ohos_id_color_hover');
                } else {
                  this.btnColor = this.isPC ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent;
                }
              })
              .stateStyles({
                pressed: {
                  .backgroundColor($r('sys.color.ohos_id_color_click_effect'))
                },
                normal: {
                  .backgroundColor(this.isPC ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent)
                }
              })
            Button($r('app.string.dm_allow_temp'))
              .margin({ bottom: 4 })
              .onClick(() => {
                this.onAllowOnce();
              })
              .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
              .height(40)
              .width(this.isPC ? 250 : '100%')
              .backgroundColor(this.btnColor)
              .onHover((isHover?: boolean, event?: HoverEvent): void => {
                if (isHover) {
                  this.btnColor = $r('sys.color.ohos_id_color_hover');
                } else {
                  this.btnColor = this.isPC ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent;
                }
              })
              .stateStyles({
                pressed: {
                  .backgroundColor($r('sys.color.ohos_id_color_click_effect'))
                },
                normal: {
                  .backgroundColor(this.isPC ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent)
                }
              })
            Button($r('app.plural.dm_not_allow', this.secondsNum, this.secondsNum))
              .margin({ left: 16, right: 16 })
              .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
              .onClick(() => {
                this.onCancel();
              })
              .height(40)
              .width(this.isPC ? 250 : '100%')
              .backgroundColor(this.btnColor)
              .onHover((isHover?: boolean, event?: HoverEvent): void => {
                if (isHover) {
                  this.btnColor = $r('sys.color.ohos_id_color_hover');
                } else {
                  this.btnColor = this.isPC ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent;
                }
              })
              .stateStyles({
                pressed: {
                  .backgroundColor($r('sys.color.ohos_id_color_click_effect'))
                },
                normal: {
                  .backgroundColor(this.isPC ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent)
                }
              })
          }
          .margin({
            left: 16,
            right: 16,
            bottom: this.isPC ? 24 : 8
          })
        }
        .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
        .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
        .margin({ left: $r('sys.float.ohos_id_dialog_margin_start'), right: $r('sys.float.ohos_id_dialog_margin_end') })
      }
    }
  }
}

@Entry
@Component
struct dialogPlusPage {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: ConfirmCustomDialog(),
    autoCancel: false,
    alignment: Constant.isPhone() ? DialogAlignment.Bottom : DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    customStyle: true,
    maskColor: $r('sys.color.ohos_id_color_mask_thin')
  });

  initStatue() {
    if (dmClass) {
      console.log(TAG + 'deviceManager exist')
      return
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.confirm',
      (err: Error, dm: deviceManager.DeviceManager) => {
        if (err) {
          console.log('createDeviceManager err:' + JSON.stringify(err) + ' --fail:' + JSON.stringify(dm))
          return
        }
        dmClass = dm
        dmClass.on('uiStateChange', (data: Record<string, string>) => {
          console.log('uiStateChange executed, dialog closed' + JSON.stringify(data))
          let tmpStr: Record<string, number> = JSON.parse(data.param)
          let msg: number = tmpStr.uiStateMsg as number
          if (msg === MSG_CANCEL_CONFIRM_SHOW) {
            this.destruction()
            return
          }
        })
      })
  }

  onPageShow() {
    console.log('onPageShow')
    this.initStatue()
  }

  destruction() {
    if (dmClass != null) {
      try {
        dmClass.release();
        dmClass = null;
      } catch (error) {
        console.log('dmClass release failed')
      }
    }
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  build() {
    Column(this.dialogController.open())
  }
}