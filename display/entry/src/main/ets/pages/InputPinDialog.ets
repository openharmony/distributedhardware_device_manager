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
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession'
import mediaquery from '@ohos.mediaquery';
import deviceInfo from '@ohos.deviceInfo';
import inputMethod from '@ohos.inputMethod';
import Constant from '../common/constant';

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
  @State isTimes: number = 3;
  @State errorTips: Resource = $r('app.plural.dm_incorrect_code', this.isTimes, this.isTimes);
  @State errorTipsVisible: Visibility = Visibility.None;
  @State heightNum: number = 600;
  @State targetDeviceName: string = '';
  @State isPC: boolean = false;
  @State btnColor: ResourceColor = Color.Transparent;
  listener: mediaquery.MediaQueryListener = mediaquery.matchMediaSync('(orientation: landscape)');
  controller?: CustomDialogController;

  onPortrait(mediaQueryResult: mediaquery.MediaQueryResult) {
    if (mediaQueryResult.matches as boolean) {
      this.heightNum = 300;
    } else {
      this.heightNum = 800;
    }
  }

  aboutToAppear() {
    this.isPC = Constant.isPC();
    if (AppStorage.get('targetDeviceName') != null) {
      this.targetDeviceName = AppStorage.get('targetDeviceName') as string;
      console.log('targetDeviceName is ' + this.targetDeviceName);
    }
    deviceManager.createDeviceManager('com.ohos.devicemanagerui.input',
      (err: Error, dm: deviceManager.DeviceManager) => {
      if (err) {
        console.log('createDeviceManager err:' + JSON.stringify(err) + '  --fail:' + '${dm}');
        return;
      }
      dmClass = dm;
      dmClass.on('uiStateChange', (data: Record<string, string>) => {
        console.log('uiStateChange executed, dialog closed' + JSON.stringify(data));
        let tmpStr: Record<string, number> = JSON.parse(data.param);
        let msg: number = tmpStr.uiStateMsg as number;
        if (msg === MSG_DOING_AUTH) {
          this.errorTips = $r('app.string.dm_authenticating');
          this.errorTipsVisible = Visibility.Visible;
          return;
        }
        if (msg === MSG_CANCEL_PIN_CODE_INPUT) {
          this.destruction();
          return;
        }
        if (msg === MSG_PIN_CODE_ERROR) {
          this.isTimes--;
          this.errorTips = $r('app.plural.dm_incorrect_code', this.isTimes, this.isTimes);
          this.password = '';
          this.errorTipsVisible = Visibility.Visible;
          this.passwordCircle = ['', '', '', '', '', ''];
        }
      })
    });
    this.listener.on('change', (mediaQueryResult: mediaquery.MediaQueryResult) => {
      this.onPortrait(mediaQueryResult);
    });
  }

  cancel() {
    console.log('cancle');
    if (dmClass) {
      console.log('deviceManager exist');
    } else {
      console.log('createDeviceManager is null');
      return;
    }
    console.log('cancle' + ACTION_CANCEL_PINCODE_INPUT);
    this.setUserOperation(ACTION_CANCEL_PINCODE_INPUT, 'extra');
    this.destruction();
  }

  confirm() {
    console.log('confirm');
    if (this.password == null || this.password == '') {
      return;
    }
    if (dmClass) {
      console.log('deviceManager exist');
    } else {
      console.log('createDeviceManager is null');
      return;
    }
    console.log('confirm' + JSON.stringify(ACTION_DONE_PINCODE_INPUT));
    this.setUserOperation(ACTION_DONE_PINCODE_INPUT, this.password);
  }

  setUserOperation(operation: number, extra: string) {
    console.log('setUserOperation: ' + operation + 'password' + extra);
    if (dmClass == null) {
      console.log('setUserOperation: ' + 'dmClass null');
      return;
    }
    try {
      dmClass.setUserOperation(operation, extra);
    } catch (error) {
      console.log('dmClass setUserOperation failed');
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
    if (this.isNumberSix(value)) {
      this.confirm();
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
          Column() {
            Text($r('app.string.dm_connect', this.targetDeviceName))
              .fontSize($r('sys.float.ohos_id_text_size_dialog_tittle'))
              .fontWeight(FontWeight.Bold)
              .fontColor($r('sys.color.ohos_id_color_text_primary'))
              .margin({ top: 12, bottom: 3 })
              .width('100%')
              .textAlign(TextAlign.Start)
              .maxLines(2)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .minFontSize(12)
              .maxFontSize($r('sys.float.ohos_id_text_size_dialog_tittle'))
              .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)

            Text($r('app.string.dm_enter_connect_code'))
              .fontSize($r('sys.float.ohos_id_text_size_body2'))
              .fontWeight(FontWeight.Regular)
              .fontColor($r('sys.color.ohos_id_color_text_secondary'))
              .margin({ bottom: 8 })
              .width('100%')
              .maxLines(2)
              .textOverflow({ overflow: TextOverflow.Ellipsis })
              .minFontSize(12)
              .maxFontSize($r('sys.float.ohos_id_text_size_body2'))
              .heightAdaptivePolicy(TextHeightAdaptivePolicy.LAYOUT_CONSTRAINT_FIRST)
          }
          .margin({ left: 24, right: 24 })
          .height(72)
          .justifyContent(FlexAlign.Center)

          Stack() {
            List() {
              ListItem() {
                Flex({ justifyContent: FlexAlign.Center }) {
                  ForEach(this.passwordCircle, (item:string) => {
                    Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
                      Text(item)
                        .fontSize($r('sys.float.ohos_id_text_size_headline7'))
                        .fontColor($r('sys.color.ohos_id_color_text_primary'))
                        .fontWeight(FontWeight.Medium)
                        .height(26)
                    }.width('10%')
                    .height('100%')
                    .visibility(item === '' ? Visibility.None : Visibility.Visible)
                  })
                  ForEach(this.passwordCircle, (item: string) => {
                    Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
                      Column()
                        .width(12)
                        .height(12)
                        .border({ width: 2, color: $r('sys.color.ohos_id_color_primary'), radius: 12})
                    }.width('10%')
                    .height('100%')
                    .visibility(item === '' ? Visibility.Visible : Visibility.None)
                  })
                }
              }
            }
            TextInput({ placeholder: '', text: this.password})
              .defaultFocus(true)
              .type(8)
              .height(60)
              .opacity(0)
              .fontColor(('rgba(0,0,0,0)'))
              .backgroundColor(('rgba(0,0,0,0)'))
              .caretColor(('rgba(0,0,0,0)'))
              .maxLength(6)
              .margin({ bottom: 8 })
              .width('100%')
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
                }, 50)
                console.log(TAG + 'this.password: ' + this.password);
              })
          }.height(48)
          .margin({ top: 12, bottom: 16})

          Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
            Text(this.errorTips)
                .fontSize($r('sys.float.ohos_id_text_size_body2'))
                .fontWeight(FontWeight.Medium)
                .fontColor($r('sys.color.ohos_id_color_warning'))
          }.visibility(this.errorTipsVisible)
          .margin({ bottom: 16,
                    left: $r('sys.float.ohos_id_corner_radius_dialog'),
                    right: $r('sys.float.ohos_id_corner_radius_dialog') })

          Flex({ justifyContent: FlexAlign.Center }) {
            Button($r('app.string.dm_cancel'))
              .height(40)
              .fontSize($r('sys.float.ohos_id_text_size_button1'))
              .onClick(() => {
                let inputMethodController = inputMethod.getController();
                inputMethodController.hideTextInput();
                if (this.controller) {
                  this.controller.close();
                }
                this.cancel();
              })
              .width('100%')
              .backgroundColor(this.btnColor)
              .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
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
          }.margin({
            left: this.isPC ? 72 : 16,
            right: this.isPC ? 72 : 16,
            bottom: this.isPC ? 24 : 16 })
        }.constraintSize({ maxHeight: `${this.heightNum}`})
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
    builder: InputCustomDialog(),
    autoCancel: false,
    alignment: Constant.isPhone() ? DialogAlignment.Bottom : DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    customStyle: true,
    maskColor: $r('sys.color.ohos_id_color_mask_thin')
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

  build() {
    Column(this.dialogController.open())
  }
}