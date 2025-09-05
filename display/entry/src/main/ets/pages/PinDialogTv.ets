/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
import display from '@ohos.display';
import Constant from '../common/constant';
import i18n from '@ohos.i18n';
import { KeyCode } from '@ohos.multimodalInput.keyCode';

let dmClass: deviceManager.DeviceManager | null;
let TAG = '[DeviceManagerUI:PinDialog]==>';
let metaType: number = 11;
const ACTION_CANCEL_PINCODE_DISPLAY: number = 3;
const MSG_CANCEL_PIN_CODE_SHOW: number = 2;

@CustomDialog
struct PinCustomDialog {
  @State pinCode: string = '';
  @State buttonWidth: number = 1;
  @State buttonHeight: number = 1;
  @State codeNum: number = 0;
  @State pinCodeArr: Array<string> = [];
  @State btnColor: ResourceColor = Color.Transparent;
  @State isPC: boolean = false;
  @State mLocalWidth: number = 0;
  controller?: CustomDialogController

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

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear execute PinCustomDialog');
    try {
      this.mLocalWidth = display.getDefaultDisplaySync().width;
    } catch (err) {
      console.error('Failed to get display width:', err);
      this.mLocalWidth = 0;
    }
    if (AppStorage.get('metaType') != null) {
      metaType = AppStorage.get('metaType') as number;
    }
    this.isPC = Constant.isPC();
    this.pinCode = AppStorage.get('pinCode') as string;
    this.pinCodeArr = this.pinCode.split('');
  }

  setUserOperation(operation: number, param: string) {
    console.log(TAG + 'setUserOperation: ' + operation);
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null');
      return;
    }
    try {
      dmClass.setUserOperation(operation, param);
    } catch (error) {
      console.log(TAG + 'dmClass setUserOperation failed');
    }
  }

  private isTibetanLanguages(): boolean {
    console.info(`${TAG} isTibetanLanguages in`);
    let locale = new Intl.Locale(i18n.System.getSystemLanguage()).toString();
    console.info(`${TAG} isTibetanLanguages: ${locale}`);
    return Constant.TIBETAN_LANGUAGES.includes(locale);
  }

  build() {
    GridRow({
      columns: { xs: 4, sm: 8, md: 24 },
      gutter: { x: '4vp' },
      breakpoints: { value: ['600vp', '840vp'] }
    }) {
      GridCol({ span: { xs: 4, sm: 4, md: 6 }, offset: { sm: 2, md: 9 } }) {
        Column() {
          Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
            Text($r('app.string.dm_connect_code'))
              .fontSize('32fp')
              .fontColor('#E5FFFFFF')
              .fontWeight(FontWeight.Medium)
              .lineHeight(this.isTibetanLanguages() ? '32.5vp' : 0)
              .margin({
                left: '32vp',
                right: '32vp'
              })
          }
          .margin({ bottom: '32vp', top: '32vp' })

          Row() {
            Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
              ForEach(this.pinCodeArr, (item: string, index: number) => {
                Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
                  Text(item)
                    .fontSize('40fp')
                    .fontColor('#E5FFFFFF')
                    .fontWeight(FontWeight.Medium)
                }.width('32vp')
                .height('100%')
                .margin(index === this.pinCodeArr.length - 1 ? {} : { right: '22vp' })
              })
            }
            .height('53vp')
            .accessibilityText('[n1]' + this.pinCode + '[n0]')
          }
          .margin({ bottom: '32vp' })

          Flex({ justifyContent: FlexAlign.Center }) {
            Button($r('app.string.dm_cancel'))
              .fontSize('24vp')
              .fontWeight(FontWeight.Medium)
              .fontColor('#E5FFFFFF')
              .width('100%')
              .height('54vp')
              .backgroundColor(this.btnColor)
              .scale({x: this.buttonWidth, y: this.buttonHeight})
              .border({
                width: '1vp',
                color: '#26ffffff'
              })
              .defaultFocus(true)
              .onKeyEvent((event?: KeyEvent) => {
                if (event && event?.keyCode === KeyCode.KEYCODE_HOME && event?.type === KeyType.Down) {
                  console.log(TAG + 'onKeyEvent eventType: ' + event?.type)
                  return;
                }
                if (event && event?.keyCode === KeyCode.KEYCODE_HOME && event?.type === KeyType.Up) {
                  console.log(TAG + 'onKeyEvent eventType: ' + event?.type)
                  if (this.controller) {
                    this.controller.close();
                  }
                  this.cancel();
                  this.setUserOperation(ACTION_CANCEL_PINCODE_DISPLAY);
                }
              })
              .onClick(() => {
                if (this.controller) {
                  this.controller.close();
                }
                this.cancel();
                this.setUserOperation(ACTION_CANCEL_PINCODE_DISPLAY);
              })
              .onHover((isHover?: boolean, event?: HoverEvent): void => {
                if (isHover) {
                  this.btnColor = '#F1F3F5';
                  this.buttonWidth = 1.05;
                  this.buttonHeight = 1.05;
                } else {
                  this.btnColor = $r('sys.color.comp_background_tertiary');
                  this.buttonWidth = 1;
                  this.buttonHeight = 1;
                }
              })
              .stateStyles({
                pressed: {
                  .backgroundColor('#26FFFFFF')
                },
                focused: {
                  .scale({x: 1.05, y: 1.05})
                  .backgroundColor('#4DFFFFFF')
                },
                normal: {
                  .backgroundColor($r('sys.color.comp_background_tertiary'))
                }
              })
          }.margin({
            left: '32vp',
            right: '32vp',
            bottom: '32vp' })
        }
        .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
        .shadow(ShadowStyle.OUTER_FLOATING_SM)
        .borderRadius('32vp')
        .border({
          width: '1vp',
          color: '#26ffffff',
          radius: '32vp'
        })
        .width(px2vp(this.mLocalWidth) * 0.36)
        .margin({ left: $r('sys.float.ohos_id_dialog_margin_start'), right: $r('sys.float.ohos_id_dialog_margin_end') })
      }
    }
    .constraintSize({ maxHeight: '80%' })
  }
}

@Entry
@Component
struct dialogPlusPage {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: PinCustomDialog(),
    cancel: this.onCancel,
    autoCancel: false,
    onWillDismiss: ()=>{
      this.onWillDismiss()
    },
    alignment: DialogAlignment.Center,
    customStyle: true,
    maskColor: '#CC000000'
  });

  onCancel() {
    this.destruction();
  }

  onWillDismiss() {
    console.log(TAG + 'onWillDismiss: ' + ACTION_CANCEL_PINCODE_DISPLAY)
    const param = {
      'META_TYPE': metaType
    };
    const jsonStr = JSON.stringify(param);
    this.setUserOperation(ACTION_CANCEL_PINCODE_DISPLAY, jsonStr);
    this.destruction();
  }

  aboutToAppear() {
    this.initStatue();
    console.log(TAG + 'aboutToAppear execute')
  }

  aboutToDisappear() {
    console.log(TAG + 'aboutToDisappear executed')
    if (dmClass != null) {
      try {
        dmClass.off('uiStateChange')
        try {
          dmClass.release();
        } catch (err) {
          let e: BusinessError = err as BusinessError;
          console.error(TAG + 'release device manager errCode:' + e.code + ',errMessage:' + e.message);
        }
      } catch (error) {
        console.log(TAG + 'dmClass release failed')
      }
      dmClass = null
    }
  }

  initStatue() {
    if (dmClass) {
      console.log(TAG + 'deviceManager exist');
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

  setUserOperation(operation: number, param: string) {
    console.log(TAG + 'setUserOperation: ' + operation)
    if (dmClass == null) {
      console.log(TAG + 'setUserOperation: ' + 'dmClass null')
      return;
    }
    try {
      dmClass.setUserOperation(operation, param);
    } catch (error) {
      console.log(TAG + 'dmClass setUserOperation failed')
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

  build() {
    Column(this.dialogController.open())
  }
}