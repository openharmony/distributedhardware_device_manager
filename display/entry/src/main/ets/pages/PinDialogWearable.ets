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
import { BusinessError } from '@ohos.base';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:PinDialog]==>';
let metaType: number = 11;
const ACTION_CANCEL_PINCODE_DISPLAY: number = 3;
const MSG_CANCEL_PIN_CODE_SHOW: number = 2;

@Entry
@Component
struct PinDialog {
  @State pinCode: string = '';
  @State pinCodeArr: Array<string> = [];
  @State btnColor: ResourceColor = Color.Transparent;
  @State isUserOperate: boolean = false;

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear execute PinCustomDialog');
    if (AppStorage.get('metaType') != null) {
      metaType = AppStorage.get('metaType') as number;
    }
    // 获取pinCode
    this.pinCode = AppStorage.get('pinCode') as string;
    this.pinCodeArr = this.pinCode.split('');
    this.initStatue();
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear executed');
    if (dmClass != null) {
      try {
        dmClass.off('uiStateChange');
        try {
          dmClass.release();
        } catch (err) {
          let e: BusinessError = err as BusinessError;
          console.error(TAG + 'release device manager errCode:' + e.code + ',errMessage:' + e.message);
        }
      } catch (error) {
        console.log(TAG + 'dmClass release failed');
      }
      dmClass = null;
    }
  }

  cancel() {
    console.log(TAG + 'destruction()');
    try {
      console.log(TAG + 'pin dialog terminateSelf');
      let session = AppStorage.get<UIExtensionContentSession>('pinSession');
      if (session) {
        session.terminateSelf();
      }
    } catch (err) {
      console.log(TAG + 'dialog cancel failed: ' + JSON.stringify(err));
    }
  }

  onPageHide() {
    console.log('onPageHide');
    if (this.isUserOperate) {
      console.log('user operate');
      return;
    }
    this.cancel();
  }

  initStatue() {
    if (dmClass) {
      console.log(TAG + 'deviceManager exist');
      return;
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.pin',
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
          if (msg === MSG_CANCEL_PIN_CODE_SHOW) {
            this.destruction();
          }
        })
      });
  }

  setUserOperation(operation: number, param: string) {
    console.log(TAG + 'setUserOperation: ' + operation);
    if (dmClass === null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null');
      return;
    }
    try {
      this.isUserOperate = true;
      dmClass.setUserOperation(operation, param);
    } catch (error) {
      console.log(TAG + 'dmClass setUserOperation failed');
    }
  }

  destruction() {
    console.log(TAG + 'destruction()');
    try {
      console.log(TAG + 'pin dialog terminateSelf');
      let session = AppStorage.get<UIExtensionContentSession>('pinSession');
      if (session) {
        session.terminateSelf();
      }
    } catch (err) {
      console.log(TAG + 'dialog cancel failed: ' + JSON.stringify(err));
    }
  }

  @Builder
  ConnectionCode() {
    Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
      Text($r('app.string.dm_connect_code'))
        .fontSize($r('sys.float.ohos_id_text_size_sub_title1'))
        .fontColor('#FFFFFF')
        .fontWeight(FontWeight.Bold)
        .margin({
          left: 24,
          right: 24
        })
        .offset({ y: 10})
    }
    .height(45)
    .margin({ bottom: 40 })
  }

  @Builder
  PinCode() {
    Row() {
      Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
        ForEach(this.pinCodeArr, (item: string) => {
          Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
            Text(item)
              .fontSize($r('sys.float.ohos_id_text_size_headline5'))
              .fontColor('#FFFFFF')
              .fontWeight(FontWeight.Medium)
          }.width('13%')
          .height('100%')
        })
      }.height(48)
    }
    .margin({ bottom: 34 })
  }

  @Builder
  ButtonCancel() {
    Flex({ justifyContent: FlexAlign.Center }) {
      Shape() {
        Ellipse()
          .width('100%')
          .height(60)
          .fill('#1F71FF')
        Column() {
          Button($r('app.string.dm_cancel'))
            .fontSize($r('sys.float.ohos_id_text_size_sub_title1'))
            .fontColor('#FFFFFF')
            .backgroundColor(Color.Transparent)
            .onClick(() => {
              const param = {
                'META_TYPE': metaType
              };
              const jsonStr = JSON.stringify(param);
              this.setUserOperation(ACTION_CANCEL_PINCODE_DISPLAY, jsonStr);
              this.destruction();
            })
            .offset({ y: -5 })
        }
      }
      .align(Alignment.Center)
      .offset({ y: 10})
    }
  }

  build() {
    Column() {
      this.ConnectionCode();
      this.PinCode();
      this.ButtonCancel();
    }
    .backgroundColor(Color.Black)
    .width('100%')
    .height('100%')
  }
}
