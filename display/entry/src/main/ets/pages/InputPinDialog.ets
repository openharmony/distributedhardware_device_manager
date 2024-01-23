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
import Constants from '../common/constant';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession'
import mediaquery from '@ohos.mediaquery';
import deviceInfo from '@ohos.deviceInfo';

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:InputPinDialog]==>'
const ACTION_CANCEL_PINCODE_INPUT: number = 4
const ACTION_DONE_PINCODE_INPUT: number = 5
const MSG_PIN_CODE_ERROR: number = 0
const MSG_CANCEL_PIN_CODE_INPUT: number = 3
const MSG_DOING_AUTH: number = 4

@CustomDialog
struct InputCustomDialog {
  @State password: string = '';
  @State passwordCircle: string[] = ['', '', '', '', '', ''];
  @State connect: string = '连接'
  @State messageTips: string = '请输入对端设备上显示的连接码'
  @State isTimes: number = 3
  @State errorTips: string = '连接码错误，还可以尝试' + this.isTimes + '次';
  @State errorTipsVisible: Visibility = Visibility.None
  @State cancelBtn: string = '取消'
  @State heightNum: number = 600;
  listener: mediaquery.MediaQueryListener = mediaquery.matchMediaSync('(orientation: landscape)');
  controller?: CustomDialogController

  onPortrait(mediaQueryResult: mediaquery.MediaQueryResult) {
    if (mediaQueryResult.matches as boolean) {
      this.heightNum = 300;
    } else {
      this.heightNum = 800;
    }
  }

  aboutToAppear() {
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.input',
      (err: Error, dm: deviceManager.DeviceManager) => {
      if (err) {
        console.log('createDeviceManager err:' + JSON.stringify(err) + '  --fail:' + '${dm}')
        return
      }
      dmClass = dm
      dmClass.on('uiStateChange', (data: Record<string, string>) => {
        console.log('uiStateChange executed, dialog closed' + JSON.stringify(data))
        let tmpStr: Record<string, number> = JSON.parse(data.param)
        let msg: number = tmpStr.uiStateMsg as number
        if (msg === MSG_DOING_AUTH) {
          this.errorTips = '正在认证中，请稍候……'
          this.errorTipsVisible = Visibility.Visible
          return
        }
        if (msg === MSG_CANCEL_PIN_CODE_INPUT) {
          this.destruction()
          return
        }
        if (msg === MSG_PIN_CODE_ERROR) {
          this.isTimes--
          this.errorTips = '连接码错误，还可以尝试' + this.isTimes + '次'
          this.password = ''
          this.errorTipsVisible = Visibility.Visible
        }
      })
    });
    this.listener.on('change', (mediaQueryResult: mediaquery.MediaQueryResult) => {
      this.onPortrait(mediaQueryResult);
    })
  }

  cancel() {
    console.log('cancle')
    if (dmClass) {
      console.log('deviceManager exist')
    } else {
      console.log('createDeviceManager is null')
      return
    }
    console.log('cancle' + ACTION_CANCEL_PINCODE_INPUT)
    this.setUserOperation(ACTION_CANCEL_PINCODE_INPUT, 'extra')
    this.destruction()
  }

  confirm() {
    console.log('confirm')
    if (this.password == null || this.password == '') {
      return;
    }
    if (dmClass) {
      console.log('deviceManager exist')
    } else {
      console.log('createDeviceManager is null')
      return
    }
    console.log('confirm' + JSON.stringify(ACTION_DONE_PINCODE_INPUT))
    this.setUserOperation(ACTION_DONE_PINCODE_INPUT, this.password)
  }

  setUserOperation(operation: number, extra: string) {
    console.log('setUserOperation: ' + operation + 'password' + extra)
    if (dmClass == null) {
      console.log('setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, extra)
    } catch (error) {
      console.log('dmClass setUserOperation failed')
    }
  }

  destruction() {
    let session = AppStorage.get<UIExtensionContentSession>('inputSession');
    if (session) {
      session.terminateSelf();
    }
  }

  isNumberSix(str: string): boolean {
    console.info(TAG + 'isNumber6 in');
    const reg: RegExp = new RegExp('^[0-9]{6}$');
    return reg.test(str);
  }

  passwordOnChange(value: string) {
    console.info(TAG + 'passwordOnChange in');
    this.password = value;
    if (this.isNumberSix(this.password)) {
      this.confirm();
    }
  }

  build() {
    Column() {
      Column() {
        Text(this.connect)
          .fontSize(20)
          .fontWeight(FontWeight.Medium)
          .margin({ top: 12, bottom: 8 })
          .width('100%')
          .textAlign(TextAlign.Start)

        Text(this.messageTips)
          .fontSize(14)
          .fontWeight(FontWeight.Regular)
          .margin({ bottom: 8 })
          .width('100%')
      }
      .margin({ left: 24 })

      Stack() {
        List() {
          ListItem() {
            Flex({ justifyContent: FlexAlign.Center }) {
              ForEach(this.passwordCircle, (item: string) => {
                Column()
                  .width(12)
                  .height(12)
                  .backgroundColor(item === '' ? 'white' : 'black')
                  .border({ width: 2, color: 'black', radius: 12})
                  .margin({ right: 24, top: 2})
              })
            }
          }
          .margin({ left: 24, top: 24, bottom: 24})
        }
        TextInput({ placeholder: '', text: this.password})
          .defaultFocus(true)
          .type(8)
          .height(60)
          .width(200)
          .opacity(0)
          .fontColor(('rgba(0,0,0,0)'))
          .backgroundColor(('rgba(0,0,0,0)'))
          .caretColor(('rgba(0,0,0,0)'))
          .maxLength(6)
          .margin({ bottom: 8 })
          .onChange((value: string) => {
            this.password = value;
            if (value.length > 6) {
              return;
            }
            let length = value.length;
            for (let i = 0; i < 6; i++) {
              if (i < length) {
                this.passwordCircle[i] = value[i];
              } else {
                this.passwordCircle[i] = '';
              }
            }
            let gThis = this;
            setTimeout(()=> {
              gThis.passwordOnChange(value);
            }, 1)
            console.log(TAG + 'this.password: ' + this.password);
          })
      }

      Text(this.errorTips)
          .fontSize(14)
          .fontWeight(FontWeight.Medium)
          .fontColor('#FA2A2D')
          .visibility(this.errorTipsVisible)

      Flex({ justifyContent: FlexAlign.Center }) {
        Button(this.cancelBtn)
          .height(40)
          .fontSize(16)
          .onClick(() => {
            if (this.controller) {
              this.controller.close();
            }
            this.cancel();
          })
          .backgroundColor(Color.Transparent)
          .fontColor('#007DFF')
      }.margin({ bottom: 10 })
    }.constraintSize({ maxHeight: `${this.heightNum}`})
  }
}

@Entry
@Component
struct dialogPlusPage {
  @State connect: string = '连接'
  @State messageTips: string = '请输入对端设备上显示的连接码';
  @State isTimes: number = 3;
  @State errorTips: string = '连接码错误，还可以尝试' + this.isTimes + '次';
  @State errorTipsVisible: number = Visibility.Hidden;
  dialogController: CustomDialogController = new CustomDialogController({
    builder: InputCustomDialog(),
    cancel: this.onCancel,
    autoCancel: false,
    alignment: (deviceInfo.deviceType === 'phone' || deviceInfo.deviceType === 'default') ?
      DialogAlignment.Bottom : DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    gridCount: 4,
    customStyle: false
  });

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear aboutToAppear')
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear aboutToDisappear')
    if (dmClass != null) {
      try {
        dmClass.off('uiStateChange');
        dmClass.release();
      } catch (error) {
        console.log('dmClass release failed');
      }
      dmClass = null
    }
  }

  onDone() {
    console.log('Done')
  }

  onCancel() {
    console.log('cancle')
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