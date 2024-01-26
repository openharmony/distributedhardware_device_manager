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
  @State message: string = '是否信任此设备？';
  @State allowOnce: string = '临时信任';
  @State allowAlways: string = '始终信任';
  @State peerAppOperation: string = '想要连接本机。';
  @State peerCustomDescription: string = '';
  @State peerDeviceName: string = '';
  @State peerDeviceType: number = 0;
  @State secondsNum: number = 30;
  @State times: number = 0;
  @State selectIndex: number = -1;
  @State isAvailableType: boolean = false;
  @state btnColor: ResourceColor = Color.Transparent;
  controller?: CustomDialogController

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear execute PinCustomDialog')
    if (AppStorage.get('deviceName') != null) {
      this.peerDeviceName = AppStorage.get('deviceName') as string
      console.log('peerDeviceName is ' + this.peerDeviceName)
    }

    if (AppStorage.get('deviceType') != null) {
      this.peerDeviceType = AppStorage.get('deviceType') as number
      console.log('peerDeviceType is ' + this.peerDeviceType)
    }

    if (AppStorage.get('appOperationStr') != null) {
      this.peerAppOperation = AppStorage.get('appOperationStr') as string
      console.log('peerAppOperation is ' + this.peerAppOperation)
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
    Column() {
      Image(this.getImages(this.peerDeviceType))
        .width(24)
        .height(24)
        .margin({ bottom: 16, top: 24 })
        .visibility(this.isAvailableType == false ? Visibility.None : Visibility.Visible)
      Column() {
        Text(this.peerDeviceName + this.peerAppOperation + this.message)
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
      }.margin({ bottom: 16, left: 24, right: 24 })

      Column() {
        Button(this.allowAlways)
          .margin({ bottom: 4 })
          .onClick(() => {
            this.onAllowAlways();
          })
          .onTouch((event?: TouchEvent | undefined) => {
            if (event != undefined && event.type === TouchType.Down) {
              this.selectIndex = 0;
            }
            if (event != undefined && event.type === TouchType.Up) {
              this.selectIndex = -1;
            }
          })
          .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
          .height(40)
          .width((deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? 250 : '100%')
          .backgroundColor(this.btnColor)
          .onHover((isHover?: boolean, event?: HoverEvent): void => {
            if (isHover) {
              this.btnColor = $r('sys.color.ohos_id_color_hover');
            } else {
              this.btnColor = (deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent;
            }
          })
          .stateStyles({
            pressed: {
              .backgroundColor($r('sys.color.ohos_id_color_click_effect'))
            },
            normal: {
              .backgroundColor((deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent)
            }
          })
        Button(this.allowOnce)
          .margin({ bottom: 4 })
          .onClick(() => {
            this.onAllowOnce();
          })
          .onTouch((event?: TouchEvent | undefined) => {
            if (event != undefined && event.type === TouchType.Down) {
              this.selectIndex = 1;
            }
            if (event != undefined && event.type === TouchType.Up) {
              this.selectIndex = -1;
            }
          })
          .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
          .height(40)
          .width((deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? 250 : '100%')
          .backgroundColor(this.btnColor)
          .onHover((isHover?: boolean, event?: HoverEvent): void => {
            if (isHover) {
              this.btnColor = $r('sys.color.ohos_id_color_hover');
            } else {
              this.btnColor = (deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent;
            }
          })
          .stateStyles({
            pressed: {
              .backgroundColor($r('sys.color.ohos_id_color_click_effect'))
            },
            normal: {
              .backgroundColor((deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent)
            }
          })
        Button('不信任(' + this.secondsNum + ' 秒)')
          .margin({ left: 16, right: 16 })
          .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
          .onClick(() => {
            this.onCancel();
          })
          .onTouch((event?: TouchEvent | undefined) => {
            if (event != undefined && event.type === TouchType.Down) {
              this.selectIndex = 2;
            }
            if (event != undefined && event.type === TouchType.Up) {
              this.selectIndex = -1;
            }
          })
          .height(40)
          .width((deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? 250 : '100%')
          .backgroundColor(this.btnColor)
          .onHover((isHover?: boolean, event?: HoverEvent): void => {
            if (isHover) {
              this.btnColor = $r('sys.color.ohos_id_color_hover');
            } else {
              this.btnColor = (deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent;
            }
          })
          .stateStyles({
            pressed: {
              .backgroundColor($r('sys.color.ohos_id_color_click_effect'))
            },
            normal: {
              .backgroundColor((deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? $r('sys.color.ohos_id_color_button_normal') : Color.Transparent)
            }
          })
      }
      .margin({
        left: 16,
        right: 16,
        bottom: deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc' ? 24 : 8
      })
    }
    .width('100%')
    .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
    .backgroundColor($r('sys.color.ohos_id_color_mask_thin'))
    .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
  }
}

@Entry
@Component
struct dialogPlusPage {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: ConfirmCustomDialog(),
    autoCancel: false,
    alignment: (deviceInfo.deviceType === 'phone' || deviceInfo.deviceType === 'default') ?
    DialogAlignment.Bottom : DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    gridCount: (deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'pc') ? 3 : 4,
    customStyle: false,
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
    GridRow({
      columns: { xs: 4, sm: 4, md: 8 },
      gutter: { x: 4 },
      breakpoints: { value: ['360vp', '400vp'] }
    }) {
      GridCol({ span: { xs: 4, sm: 4, md: 4 } }) {
        Column(this.dialogController.open())
      }
    }
  }
}