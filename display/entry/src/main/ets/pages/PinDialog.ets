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
import { BusinessError } from '@ohos.base';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import deviceInfo from '@ohos.deviceInfo';

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:PinDialog]==>';
const ACTION_CANCEL_PINCODE_DISPLAY: number = 3
const MSG_CANCEL_PIN_CODE_SHOW: number = 2

@CustomDialog
struct PinCustomDialog {
  @State messageTitle: string = '连接码';
  @State pinCode: string = '';
  @State cancel_str: string = '取消';
  @State isShow: boolean = true;
  @State displayWidth:number = 0;
  @State displayHeight:number = 0;
  @State topRect_height:number=0;
  @State pinCodeArr:Array<string> = [];

  controller?: CustomDialogController

  cancel() {
    console.log(TAG+'destruction()');
    try {
      console.log(TAG+'pin dialog terminateSelf');
      let session = AppStorage.get<UIExtensionContentSession>('pinSession');
      if (session) {
        session.terminateSelf();
      }
    } catch(err) {
      console.log(TAG+'dialog cancel failed: ' + JSON.stringify(err));
    }
  }

  aboutToAppear() {
    console.log(TAG+'aboutToAppear execute PinCustomDialog')
    // 获取pinCode
    this.pinCode = AppStorage.get('pinCode') as string;
    this.pinCodeArr = this.pinCode.split('');
    console.log(TAG+'this.pinCodeArr'+this.pinCodeArr);
  }

  setUserOperation(operation: number) {
    console.log(TAG+'setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log(TAG+'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, 'extra');
    } catch (error) {
      console.log(TAG+'dmClass setUserOperation failed')
    }
  }

  build() {
    Column() {
      Flex({direction:FlexDirection.Row,alignItems:ItemAlign.Center,justifyContent:FlexAlign.Start}){
        Text(this.messageTitle)
          .fontSize(20)
          .fontColor('#182431')
          .fontWeight(FontWeight.Medium)
          .margin({
            top:14,
            bottom:14,
            left:24,
            right:24
          })
      }
      .height(56)
      .margin({bottom:24})

      Row(){
        Flex({direction:FlexDirection.Row,alignItems:ItemAlign.Center,justifyContent:FlexAlign.Center}){
          ForEach(this.pinCodeArr,(item:string,index)=>{
            Text(item)
              .fontSize(24)
              .fontColor('#1e1e1e')
              .fontWeight(FontWeight.Medium)
              .height(26)
              .margin({right:24})
          })
        }
      }
      .margin({bottom:24})

      Row(){
        Button(this.cancel_str)
          .fontSize(16)
          .fontColor("#007DFF")
          .onClick(() => {
            if (this.controller) {
              this.controller.close();
            }
            this.cancel();
            this.setUserOperation(ACTION_CANCEL_PINCODE_DISPLAY)
          })
          .height(40)
          .margin({left: 16, right: 16})
          .backgroundColor(Color.Transparent)
      }
    }
    .constraintSize({maxHeight: `${300}`})
  }
}


@Entry
@Component
struct dialogPlusPage {
  @State cancel: string = '取消';
  @State isShow: boolean = true;

  dialogController: CustomDialogController = new CustomDialogController({
    builder: PinCustomDialog(),
    cancel: this.onCancel,
    autoCancel: false,
    alignment: (deviceInfo.deviceType === 'phone' || deviceInfo.deviceType === 'default') ?
      DialogAlignment.Bottom : DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    gridCount: 4,
    customStyle: false
  });

  onCancel() {
    this.destruction();
  }

  aboutToAppear() {
    this.initStatue();
    console.log(TAG+'aboutToAppear execute')
  }

  aboutToDisappear() {
    console.log(TAG+ 'aboutToDisappear executed')
    if (dmClass != null) {
      try {
        dmClass.off('uiStateChange')
        try {
          dmClass.release();
        } catch (err) {
          let e: BusinessError = err as BusinessError;
          console.error(TAG+'release device manager errCode:' + e.code + ',errMessage:' + e.message);
        }
      } catch (error) {
        console.log(TAG+'dmClass release failed')
      }
      dmClass = null
    }
  }

  initStatue() {
    if (dmClass) {
      console.log(TAG+'deviceManager exist');
      return;
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.pin',
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
        if (msg === MSG_CANCEL_PIN_CODE_SHOW) {
          this.destruction()
        }
      })
    });
  }

  setUserOperation(operation: number) {
    console.log(TAG+'setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log(TAG+'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, 'extra');
    } catch (error) {
      console.log(TAG+'dmClass setUserOperation failed')
    }
  }

  destruction() {
    console.log(TAG+'destruction()');
    try {
      console.log(TAG+'pin dialog terminateSelf');
      let session = AppStorage.get<UIExtensionContentSession>('pinSession');
      if (session) {
        session.terminateSelf();
      }
    } catch(err) {
      console.log(TAG+'dialog cancel failed: ' + JSON.stringify(err));
    }
  }

  build() {
    GridRow({
      columns: { xs: 4, sm: 4, md: 8 },
      gutter: { x: 4 },
      breakpoints: { value: ['360vp', '400vp']}
    }) {
      GridCol({ span: { xs: 4, sm: 4, md: 4 } }) {
        Column(this.dialogController.open())
      }
    }
  }
}