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
import deviceManager from '@ohos.distributedHardware.deviceManager';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
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
const CAST_PKG_NAME: string = 'CastEngineService';

@Entry
@Component
struct Index {
  @State peerAppOperation: string = '';
  @State peerCustomDescription: string = '';
  @State peerDeviceName: string = '';
  @State peerDeviceType: number = 0;
  @State secondsNum: number = 30;
  @State times: number = 0;
  @State isAvailableType: boolean = false;
  @State title: string = '';
  @State isUserOperate: boolean = false;

  initStatue() {
    if (dmClass) {
      console.log(TAG + 'deviceManager exist');
      return;
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.confirm',
      (err: Error, dm: deviceManager.DeviceManager) => {
        if (err) {
          console.log('createDeviceManager err:' + JSON.stringify(err) + ' --fail:' + JSON.stringify(dm));
          return;
        }
        dmClass = dm;
        dmClass.on('uiStateChange', (data: Record<string, string>) => {
          console.log('uiStateChange executed, dialog closed' + JSON.stringify(data));
          let tmpStr: Record<string, number> = JSON.parse(data.param);
          let msg: number = tmpStr.uiStateMsg as number;
          if (msg === MSG_CANCEL_CONFIRM_SHOW) {
            this.destruction();
            return;
          }
        });
      });
  }

  getImages(peerdeviceType: number): Resource {
    console.info('peerdeviceType is ' + peerdeviceType);
    if (peerdeviceType === deviceManager.DeviceType.SPEAKER) {
      this.isAvailableType = true;
      return $r('sys.symbol.soundai_fill');
    } else if (peerdeviceType === deviceManager.DeviceType.PHONE) {
      this.isAvailableType = true;
      return $r('sys.symbol.phone_fill_1');
    } else if (peerdeviceType === deviceManager.DeviceType.TABLET) {
      this.isAvailableType = true;
      return $r('sys.symbol.pad_fill');
    } else if (peerdeviceType === deviceManager.DeviceType.WEARABLE) {
      this.isAvailableType = true;
      return $r('sys.symbol.earphone_case_16896');
    } else if (peerdeviceType === deviceManager.DeviceType.CAR) {
      this.isAvailableType = true;
      return $r('sys.symbol.car_fill');
    } else if (peerdeviceType === deviceManager.DeviceType.TV) {
      this.isAvailableType = true;
      return $r('sys.symbol.smartscreen_fill');
    } else if (peerdeviceType === DEVICE_TYPE_PC) {
      this.isAvailableType = true;
      return $r('sys.symbol.matebook_fill');
    } else if (peerdeviceType === DEVICE_TYPE_2IN1) {
      this.isAvailableType = true;
      return $r('sys.symbol.matebook_fill');
    } else {
      this.isAvailableType = false;
      return $r('sys.symbol.unknown_device_fill');
    }
  }

  onPageShow() {
    console.log('onPageShow');
    this.initStatue();
  }

  onPageHide() {
    console.log('onPageHide');
    if (this.isUserOperate) {
      console.log('user operate');
      return;
    }
    this.onCancel();
  }

  destruction() {
    if (dmClass != null) {
      try {
        dmClass.release();
        dmClass = null;
      } catch (error) {
        console.log('dmClass release failed');
      }
    }
    let session = AppStorage.get<UIExtensionContentSession>('ConfirmSession');
    if (session) {
      session.terminateSelf();
    }
  }

  setUserOperation(operation: number) {
    console.log(TAG + 'setUserOperation: ' + operation);
    if (dmClass === null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null');
      return;
    }
    try {
      this.isUserOperate = true;
      dmClass.setUserOperation(operation, 'extra');
    } catch (error) {
      console.log(TAG + 'dmClass setUserOperation failed');
    }
  }

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear execute PinCustomDialog');
    let context = getContext() as common.UIAbilityContext;

    if (AppStorage.get('deviceName') != null) {
      this.peerDeviceName = AppStorage.get('deviceName') as string;
      console.log('peerDeviceName is ' + this.peerDeviceName);
    }
    let hostPkgLabel: string = AppStorage.get('hostPkgLabel') as string;
    if (hostPkgLabel === CAST_PKG_NAME) {
      this.title =
        context.resourceManager.getStringSync($r('app.string.dm_confirm_title_cast').id, this.peerDeviceName);
    } else if (hostPkgLabel != null) {
      this.title = context.resourceManager.getStringSync($r('app.string.dm_confirm_title_hap').id, hostPkgLabel,
        this.peerDeviceName);
      this.peerCustomDescription = context.resourceManager.getStringSync($r('app.string.dm_confirm_intention').id);
    } else {
      let titleFirst: string =
        context.resourceManager.getStringSync($r('app.string.dm_connect_device').id, this.peerDeviceName);
      this.title =
        context.resourceManager.getStringSync($r('app.string.dm_is_trust_device').id, titleFirst);
      this.peerCustomDescription = context.resourceManager.getStringSync($r('app.string.dm_confirm_intention').id);
    }

    if (AppStorage.get('deviceType') != null) {
      this.peerDeviceType = AppStorage.get('deviceType') as number;
      console.log('peerDeviceType is ' + this.peerDeviceType);
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
    }, 1000);
  }

  onAllowOnce() {
    console.log('allow once');
    if (dmClass === null) {
      console.log('createDeviceManager is null');
      return;
    }

    console.log('allow once' + ACTION_ALLOW_AUTH_ONCE);
    this.setUserOperation(ACTION_ALLOW_AUTH_ONCE);
    this.destruction();
  }

  onAllowAlways() {
    console.log('allow always');
    if (dmClass === null) {
      console.log('createDeviceManager is null');
      return;
    }

    console.log('allow always' + ACTION_ALLOW_AUTH_ALWAYS);
    this.setUserOperation(ACTION_ALLOW_AUTH_ALWAYS);
    this.destruction();
  }

  onCancel() {
    console.log('cancel');
    if (dmClass === null) {
      console.log('createDeviceManager is null');
      return;
    }

    console.log('cancel' + ACTION_CANCEL_AUTH);
    this.setUserOperation(ACTION_CANCEL_AUTH);
    this.destruction();
  }

  @Builder
  Symbol() {
    Shape() {
      Circle()
        .width(52)
        .height(52)
        .fill($r('sys.color.ohos_id_color_activated'))
      Column() {
        SymbolGlyph(this.getImages(this.peerDeviceType))
          .fontSize('36vp')
          .renderingStrategy(SymbolRenderingStrategy.MULTIPLE_OPACITY)
          .fontColor([$r('sys.color.ohos_id_color_primary_contrary')])
          .offset({ x: 8, y: 8 })
      }
    }
    .visibility(this.isAvailableType ? Visibility.Visible : Visibility.None)
    .margin({ bottom: 12, top: 6 })
  }

  @Builder
  Title() {
    Column() {
      Text(this.title)
        .textAlign(TextAlign.Center)
        .fontSize($r('sys.float.ohos_id_text_size_body2'))
        .fontWeight(FontWeight.Regular)
        .fontColor('#FFFFFF')
        .width('auto')
      Text(this.peerCustomDescription)
        .textAlign(TextAlign.Center)
        .fontColor('#99FFFFFF')
        .fontWeight(FontWeight.Regular)
        .fontSize($r('sys.float.ohos_id_text_size_body3'))
        .width('auto')
        .margin({ top: 2 })
        .visibility(this.peerCustomDescription === '' ? Visibility.None : Visibility.Visible)
    }.margin({
      top: this.isAvailableType ? 0 : 6,
      bottom: 12, left: 26, right: 26 })
  }

  @Builder
  Buttons() {
    Column() {
      Button($r('app.string.dm_allow_always'))
        .margin({ bottom: 12 })
        .onClick(() => {
          this.onAllowAlways();
        })
        .fontColor('#FFFFFF')
        .fontSize($r('sys.float.ohos_id_text_size_button2'))
        .height(40)
        .width('100%')
        .backgroundColor('#1F71FF')
      Button($r('app.string.dm_allow_this_time'))
        .margin({ bottom: 12 })
        .onClick(() => {
          this.onAllowOnce();
        })
        .fontColor('#5EA1FF')
        .fontSize($r('sys.float.ohos_id_text_size_button2'))
        .height(40)
        .width('100%')
        .backgroundColor('#405ea1ff')
      Button($r('app.plural.dm_not_allow', this.secondsNum, this.secondsNum))
        .onClick(() => {
          this.onCancel();
        })
        .fontColor('#5EA1FF')
        .fontSize($r('sys.float.ohos_id_text_size_button2'))
        .height(40)
        .width('100%')
        .backgroundColor('#405ea1ff')
    }
    .margin({ left: 50, right: 50, bottom: 36})
  }

  build() {
    Scroll() {
      Column() {
        this.Symbol();
        this.Title();
        this.Buttons();
      }
    }
    .backgroundColor(Color.Black)
    .height('100%')
   .width('100%')
  }
}
